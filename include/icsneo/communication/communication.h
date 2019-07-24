#ifndef __COMMUNICATION_H_
#define __COMMUNICATION_H_

#include "icsneo/communication/icommunication.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/network.h"
#include "icsneo/communication/packet.h"
#include "icsneo/communication/message/callback/messagecallback.h"
#include "icsneo/communication/message/serialnumbermessage.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/decoder.h"
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <queue>
#include <map>

namespace icsneo {

class Communication {
public:
	Communication(
		device_eventhandler_t report,
		std::unique_ptr<ICommunication> com,
		std::shared_ptr<Packetizer> p,
		std::unique_ptr<Encoder> e,
		std::unique_ptr<Decoder> md) : packetizer(p), encoder(std::move(e)), decoder(std::move(md)), report(report), impl(std::move(com)) {}
	virtual ~Communication() { close(); }

	bool open();
	bool close();
	bool isOpen();
	virtual void spawnThreads();
	virtual void joinThreads();
	bool rawWrite(const std::vector<uint8_t>& bytes) { return impl->write(bytes); }
	virtual bool sendPacket(std::vector<uint8_t>& bytes);

	void setWriteBlocks(bool blocks) { impl->writeBlocks = blocks; }

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

	std::shared_ptr<Packetizer> packetizer; // Ownership is shared with the encoder
	std::unique_ptr<Encoder> encoder;
	std::unique_ptr<Decoder> decoder;
	device_eventhandler_t report;

protected:
	std::unique_ptr<ICommunication> impl;
	static int messageCallbackIDCounter;
	std::mutex messageCallbacksLock;
	std::map<int, MessageCallback> messageCallbacks;
	std::atomic<bool> closing{false};

private:
	std::thread readTaskThread;
	void readTask();
};

}

#endif