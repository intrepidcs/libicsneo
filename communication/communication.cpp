#include "communication/include/communication.h"
#include <chrono>
#include <iostream>
#include <queue>
#include <iomanip>
#include <cstring>
#include "communication/include/messagedecoder.h"
#include "communication/include/packetizer.h"

using namespace icsneo;

int Communication::messageCallbackIDCounter = 1;

uint8_t Communication::ICSChecksum(const std::vector<uint8_t>& data) {
	uint32_t checksum = 0;
	for(auto i = 0; i < data.size(); i++)
		checksum += data[i];
	checksum = ~checksum;
	checksum++;
	return (uint8_t)checksum;
}

std::vector<uint8_t>& Communication::packetWrap(std::vector<uint8_t>& data, bool addChecksum) {
	if(addChecksum)
		data.push_back(ICSChecksum(data));
	data.insert(data.begin(), 0xAA);
	if(align16bit && data.size() % 2 == 1)
		data.push_back('A');
	return data;
}

bool Communication::open() {
	if(isOpen)
		return true;

	spawnThreads();
	isOpen = true;
	return impl->open();
}

void Communication::spawnThreads() {
	readTaskThread = std::thread(&Communication::readTask, this);
}

void Communication::joinThreads() {
	if(readTaskThread.joinable())
		readTaskThread.join();
}

bool Communication::close() {
	if(!isOpen)
		return false;

	isOpen = false;
	closing = true;
	joinThreads();

	return impl->close();
}

bool Communication::sendPacket(std::vector<uint8_t>& bytes) {
	return impl->write(Communication::packetWrap(bytes));
}

bool Communication::sendCommand(Communication::Command cmd, std::vector<uint8_t> arguments) {
	std::vector<uint8_t> bytes;
	bytes.push_back((uint8_t)cmd);
	for(auto& b : arguments)
		bytes.push_back(b);
	bytes.insert(bytes.begin(), 0xB | ((uint8_t)bytes.size() << 4));
	return sendPacket(bytes);
}

int Communication::addMessageCallback(const MessageCallback& cb) {
	messageCallbacks.insert(std::make_pair(messageCallbackIDCounter, cb));
	return messageCallbackIDCounter++;
}

bool Communication::removeMessageCallback(int id) {
	try {
		messageCallbacks.erase(id);
		return true;
	} catch(...) {
		return false;
	}
}

void Communication::readTask() {
	std::vector<uint8_t> readBytes;
	Packetizer packetizer;
	MessageDecoder decoder;

	while(!closing) {
		readBytes.clear();
		if(impl->readWait(readBytes)) {
			if(packetizer.input(readBytes)) {
				for(auto& packet : packetizer.output()) {
					auto msg = decoder.decodePacket(packet);
					for(auto& cb : messageCallbacks) { // We might have closed while reading or processing
						if(!closing) {
							cb.second.callIfMatch(msg);
						}
					}
				}
			}
		}
	}
}
