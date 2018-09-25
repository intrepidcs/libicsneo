#ifndef __COMMUNICATION_H_
#define __COMMUNICATION_H_

#include "communication/include/icommunication.h"
#include "communication/include/command.h"
#include "communication/include/network.h"
#include "communication/include/packet.h"
#include "communication/message/callback/include/messagecallback.h"
#include "communication/message/include/serialnumbermessage.h"
#include "communication/include/packetizer.h"
#include "communication/include/decoder.h"
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <queue>
#include <map>

namespace icsneo {

class Communication {
public:
	Communication(std::shared_ptr<ICommunication> com, std::shared_ptr<Packetizer> p, std::shared_ptr<Decoder> md) : impl(com), packetizer(p), decoder(md) {}
	virtual ~Communication() { close(); }

	bool open();
	bool close();
	virtual void spawnThreads();
	virtual void joinThreads();
	bool rawWrite(const std::vector<uint8_t>& bytes) { return impl->write(bytes); }
	bool sendPacket(std::vector<uint8_t>& bytes);

	virtual bool sendCommand(Command cmd, bool boolean) { return sendCommand(cmd, std::vector<uint8_t>({ (uint8_t)boolean })); }
	virtual bool sendCommand(Command cmd, std::vector<uint8_t> arguments = {});
	bool getSettingsSync(std::vector<uint8_t>& data, std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	std::shared_ptr<SerialNumberMessage> getSerialNumberSync(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	
	int addMessageCallback(const MessageCallback& cb);
	bool removeMessageCallback(int id);
	std::shared_ptr<Message> waitForMessageSync(MessageFilter f = MessageFilter(), std::chrono::milliseconds timeout = std::chrono::milliseconds(50)) {
		return waitForMessageSync(std::make_shared<MessageFilter>(f), timeout);
	}
	std::shared_ptr<Message> waitForMessageSync(std::shared_ptr<MessageFilter> f, std::chrono::milliseconds timeout = std::chrono::milliseconds(50));

	std::shared_ptr<Packetizer> packetizer;
	std::shared_ptr<Decoder> decoder;

protected:
	std::shared_ptr<ICommunication> impl;
	static int messageCallbackIDCounter;
	std::map<int, MessageCallback> messageCallbacks;
	std::atomic<bool> closing{false};

private:
	bool isOpen = false;

	std::thread readTaskThread;
	void readTask();
};

};

#endif