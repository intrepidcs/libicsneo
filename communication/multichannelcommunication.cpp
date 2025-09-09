#include "icsneo/communication/multichannelcommunication.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/message/neoreadmemorysdmessage.h"

using namespace icsneo;

MultiChannelCommunication::MultiChannelCommunication(device_eventhandler_t err, std::unique_ptr<Driver> com,
	std::function<std::unique_ptr<Packetizer>()> makeConfiguredPacketizer, std::unique_ptr<Encoder> e,
	std::unique_ptr<Decoder> md, size_t vnetCount) :
	Communication(err, std::move(com), makeConfiguredPacketizer, std::move(e), std::move(md)), numVnets(vnetCount), packetRB(2048) {
	vnetThreads.resize(numVnets);
	vnetQueues.resize(numVnets);
}

void MultiChannelCommunication::spawnThreads() {
	for(size_t i = 0; i < numVnets; i++) {
		while(vnetQueues[i].pop()) {} // Ensure the queue is empty
		vnetThreads[i] = std::thread(&MultiChannelCommunication::vnetReadTask, this, i);
	}
	hidReadThread = std::thread(&MultiChannelCommunication::hidReadTask, this);
}

void MultiChannelCommunication::joinThreads() {
	closing = true;
	if(hidReadThread.joinable())
		hidReadThread.join();
	for(auto& thread : vnetThreads) {
		if(thread.joinable())
			thread.join();
	}
	closing = false;
}

bool MultiChannelCommunication::sendPacket(std::vector<uint8_t>& bytes) {
	bytes.insert(bytes.begin(), {(uint8_t)CommandType::HostPC_to_Vnet1, (uint8_t)bytes.size(), (uint8_t)(bytes.size() >> 8)});
	return rawWrite(bytes);
}

void MultiChannelCommunication::hidReadTask() {
	bool readMore = true;
	bool gotPacket = false; // Have we got the first valid packet (don't flag errors otherwise)
	std::deque<uint8_t> usbReadFifo;
	std::vector<uint8_t> readBytes;
	std::vector<uint8_t> payloadBytes;

	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	while(!closing) {
		if(readMore) {
			readBytes.clear();
			if(driver->readWait(readBytes)) {
				readMore = false;
				usbReadFifo.insert(usbReadFifo.end(), std::make_move_iterator(readBytes.begin()), std::make_move_iterator(readBytes.end()));
			}
		} else {
			switch(state) {
				case PreprocessState::SearchForCommand:
					if(usbReadFifo.size() < 1) {
						readMore = true;
						continue;
					}

					currentCommandType = (CommandType)usbReadFifo[0];

					if(!CommandTypeIsValid(currentCommandType)) {
						// Device to host bytes discarded
						if(gotPacket)
							EventManager::GetInstance().add(APIEvent(APIEvent::Type::FailedToRead, APIEvent::Severity::Error));
						usbReadFifo.pop_front();
						continue;
					}

					currentReadIndex = 1;

					if(CommandTypeHasAddress(currentCommandType)) {
						state = PreprocessState::ParseAddress;
						continue; // No commands which define an address also define a length, so we can just continue from there
					}

					currentCommandLength = CommandTypeDefinesLength(currentCommandType);
					if(currentCommandLength == 0) {
						state = PreprocessState::ParseLength;
						continue;
					}

					state = PreprocessState::GetData;
					continue;
				case PreprocessState::ParseAddress:
					// The address is represented by a 4 byte little endian
					// Don't care about it yet
					currentReadIndex += 4;
					// Intentionally fall through
				case PreprocessState::ParseLength:
					state = PreprocessState::ParseLength; // Set state in case we've fallen through, but later need to go around again

					if(usbReadFifo.size() < currentReadIndex + 2) { // Come back we have more data
						readMore = true;
						continue;
					}

					// The length is represented by a 2 byte little endian
					currentCommandLength = usbReadFifo[currentReadIndex++];
					currentCommandLength |= usbReadFifo[currentReadIndex++] << 8;
					// Intentionally fall through
				case PreprocessState::GetData:
					state = PreprocessState::GetData; // Set state in case we've fallen through, but later need to go around again

					if(usbReadFifo.size() < currentReadIndex + currentCommandLength) { // Come back when we have more data
						readMore = true;
						continue;
					}

					for(size_t i = 0; i < currentReadIndex; i++)
						usbReadFifo.pop_front();

					payloadBytes.clear();
					payloadBytes.resize(currentCommandLength);
					for(auto i = 0; i < currentCommandLength; i++) {
						payloadBytes[i] = usbReadFifo[0];
						usbReadFifo.pop_front();
					}

					moodycamel::BlockingReaderWriterQueue< std::vector<uint8_t> >* currentQueue = nullptr;
					switch(currentCommandType) {
						case CommandType::Vnet1_to_HostPC:
							currentQueue = &vnetQueues[0];
							break;
						case CommandType::Vnet2_to_HostPC:
							if(numVnets >= 2)
								currentQueue = &vnetQueues[1];
							break;
						case CommandType::Vnet3_to_HostPC:
							if(numVnets >= 3)
								currentQueue = &vnetQueues[2];
							break;
						case CommandType::SDCC1_to_HostPC: {
							auto msg = std::make_shared<NeoReadMemorySDMessage>();
							std::swap(msg->data, payloadBytes);
							dispatchMessage(msg);
							break;
						}
					}

					if(currentQueue == nullptr) {
						state = PreprocessState::SearchForCommand;
						break;
					}

                    {
                        std::unique_lock lk(ringBufMutex);
                        if(!packetRB.write(std::move(payloadBytes)) && gotPacket)
                            EventManager::GetInstance().add(APIEvent(APIEvent::Type::FailedToRead, APIEvent::Severity::Error));
                        payloadBytes.clear();
                    }
                    ringBufCV.notify_all();
					gotPacket = true;
					state = PreprocessState::SearchForCommand;
					break;
			}
		}
	}
}

void MultiChannelCommunication::vnetReadTask(size_t vnetIndex) {
	std::vector<uint8_t> payloadBytes;
	std::unique_ptr<Packetizer> packetizerLifetime;
	Packetizer* vnetPacketizer;
	if(vnetIndex == 0)
		vnetPacketizer = packetizer.get();
	else {
		packetizerLifetime = makeConfiguredPacketizer();
		vnetPacketizer = packetizerLifetime.get();
	}

	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	while(!closing) {
        std::unique_lock lk(ringBufMutex);
        ringBufCV.wait(lk);
        if(vnetPacketizer->input(packetRB)) {
            for(const auto& packet : vnetPacketizer->output()) {
                std::shared_ptr<Message> msg;
                if(!decoder->decode(msg, packet))
                    continue;

                dispatchMessage(msg);
            }
        }
	}
}