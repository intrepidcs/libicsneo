#ifndef __COMMUNICATION_H_
#define __COMMUNICATION_H_

#include "communication/include/icommunication.h"
#include "communication/include/command.h"
#include "communication/include/network.h"
#include "communication/include/packet.h"
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <queue>
#include <map>

namespace icsneo {

class Communication {
public:
	static uint8_t ICSChecksum(const std::vector<uint8_t>& data);

	Communication(std::shared_ptr<ICommunication> com, std::shared_ptr<Packetizer> p, std::shared_ptr<MessageDecoder> md) : impl(com), packetizer(p), decoder(md) {}
	virtual ~Communication() { close(); }

	bool open();
	bool close();
	virtual void spawnThreads();
	virtual void joinThreads();
	bool rawWrite(const std::vector<uint8_t>& bytes) { return impl->write(bytes); }
	std::vector<uint8_t>& packetWrap(std::vector<uint8_t>& data, bool addChecksum = true);
	bool sendPacket(std::vector<uint8_t>& bytes);

	virtual bool sendCommand(Command cmd, bool boolean) { return sendCommand(cmd, std::vector<uint8_t>({ (uint8_t)boolean })); }
	virtual bool sendCommand(Command cmd, std::vector<uint8_t> arguments = {});
	bool getSettingsSync(std::vector<uint8_t>& data, std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	bool getSerialNumberSync(std::string& serial, std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	
	int addMessageCallback(const MessageCallback& cb);
	bool removeMessageCallback(int id);
	std::shared_ptr<Message> waitForMessageSync(MessageFilter f = MessageFilter(), std::chrono::milliseconds timeout = std::chrono::milliseconds(50));

	void setAlign16Bit(bool enable) { align16bit = enable; }

	std::shared_ptr<Packetizer> packetizer;
	std::shared_ptr<MessageDecoder> decoder;

protected:
	std::shared_ptr<ICommunication> impl;
	static int messageCallbackIDCounter;
	std::map<int, MessageCallback> messageCallbacks;
	std::atomic<bool> closing{false};

private:
	bool isOpen = false;
	bool align16bit = true; // Not needed for Gigalog, Galaxy, etc and newer

	std::thread readTaskThread;
	void readTask();
};

};

#endif