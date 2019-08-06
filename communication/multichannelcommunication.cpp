#include "icsneo/communication/multichannelcommunication.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/communication/packetizer.h"
#include <iostream>
#include <iomanip>

using namespace icsneo;

void MultiChannelCommunication::spawnThreads() {
	mainChannelReadThread = std::thread(&MultiChannelCommunication::readTask, this);
}

void MultiChannelCommunication::joinThreads() {
	closing = true;
	if(mainChannelReadThread.joinable())
		mainChannelReadThread.join();
	closing = false;
}

bool MultiChannelCommunication::sendPacket(std::vector<uint8_t>& bytes) {
	bytes.insert(bytes.begin(), {(uint8_t)CommandType::HostPC_to_Vnet1, (uint8_t)bytes.size(), (uint8_t)(bytes.size() >> 8)});
	return rawWrite(bytes);
}

void MultiChannelCommunication::readTask() {
	bool readMore = true;
	std::deque<uint8_t> usbReadFifo;
	std::vector<uint8_t> readBytes;
	std::vector<uint8_t> payloadBytes;

	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	while(!closing) {
		if(readMore) {
			readBytes.clear();
			if(impl->readWait(readBytes)) {
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

					if(usbReadFifo.size() <= currentReadIndex + currentCommandLength) { // Come back we have more data
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
					
					if(packetizer->input(payloadBytes)) {
						for(auto& packet : packetizer->output()) {
							std::shared_ptr<Message> msg;
							if(!decoder->decode(msg, packet)) {
								report(APIEvent::Type::Unknown, APIEvent::Severity::Error); // TODO Use specific error
								continue;
							}

							for(auto& cb : messageCallbacks) { // We might have closed while reading or processing
								if(!closing) {
									// We want callbacks to be able to access errors
									EventManager::GetInstance().cancelErrorDowngradingOnCurrentThread();
									cb.second.callIfMatch(msg);
									EventManager::GetInstance().downgradeErrorsOnCurrentThread();
								}
							}
						}
					}

					state = PreprocessState::SearchForCommand;
			}
		}
		
	}
}