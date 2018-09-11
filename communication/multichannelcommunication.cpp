#include "communication/include/multichannelcommunication.h"
#include "communication/include/messagedecoder.h"
#include <iostream>
#include <iomanip>

using namespace icsneo;

void MultiChannelCommunication::spawnThreads() {
	mainChannelReadThread = std::thread(&MultiChannelCommunication::readTask, this);
}

void MultiChannelCommunication::joinThreads() {
	if(mainChannelReadThread.joinable())
		mainChannelReadThread.join();
}

bool MultiChannelCommunication::sendCommand(Communication::Command cmd, std::vector<uint8_t> arguments) {
	std::vector<uint8_t> bytes;
	bytes.push_back((uint8_t)cmd);
	for(auto& b : arguments)
		bytes.push_back(b);
	bytes.insert(bytes.begin(), 0xB | ((uint8_t)bytes.size() << 4));
	bytes = Communication::packetWrap(bytes);
	bytes.insert(bytes.begin(), {(uint8_t)CommandType::HostPC_to_Vnet1, (uint8_t)bytes.size(), (uint8_t)(bytes.size() >> 8)});
	return rawWrite(bytes);
}

void MultiChannelCommunication::readTask() {
	bool readMore = true;
	std::deque<uint8_t> usbReadFifo;
	std::vector<uint8_t> readBytes;
	std::vector<uint8_t> payloadBytes;
	MessageDecoder decoder;

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
						std::cout << "cnv" << std::hex << (int)currentCommandType << ' ' << std::dec;
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

					//std::cout << std::dec << "Got a multichannel message! Size: " << currentCommandLength << std::hex << std::setfill('0') << std::setw(2) << " Cmd: 0x" << (int)currentCommandType << std::endl;
					for(auto i = 0; i < currentReadIndex; i++)
						usbReadFifo.pop_front();

					payloadBytes.clear();
					payloadBytes.reserve(currentCommandLength);
					for(auto i = 0; i < currentCommandLength; i++) {
						//std::cout << (int)usbReadFifo[0] << ' ';
						payloadBytes.push_back(usbReadFifo[0]);
						// if(i % 16 == 15)
						// 	std::cout << std::endl;
						usbReadFifo.pop_front();
					}
					//std::cout << std::dec << std::endl;
					
					if(decoder.input(payloadBytes)) {
						for(auto& msg : decoder.output()) {
							for(auto& cb : messageCallbacks) {
								if(!closing) { // We might have closed while reading or processing
									cb.second.callIfMatch(msg);
								}
							}
						}
					}

					state = PreprocessState::SearchForCommand;
			}
		}
		
	}
}