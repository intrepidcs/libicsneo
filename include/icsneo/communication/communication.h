#ifndef __COMMUNICATION_H_
#define __COMMUNICATION_H_

#ifdef __cplusplus

#include "icsneo/communication/driver.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/network.h"
#include "icsneo/communication/packet.h"
#include "icsneo/communication/message/callback/messagecallback.h"
#include "icsneo/communication/message/serialnumbermessage.h"
#include "icsneo/communication/message/logicaldiskinfomessage.h"
#include "icsneo/device/deviceversion.h"
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
		std::unique_ptr<Driver>&& driver,
		std::function<std::unique_ptr<Packetizer>()> makeConfiguredPacketizer,
		std::unique_ptr<Encoder>&& e,
		std::unique_ptr<Decoder>&& md) : makeConfiguredPacketizer(makeConfiguredPacketizer), encoder(std::move(e)), decoder(std::move(md)), report(report), driver(std::move(driver)) {
		packetizer = makeConfiguredPacketizer();
	}
	virtual ~Communication();

	bool open();
	bool close();
	bool isOpen();
	bool isDisconnected();
	virtual void spawnThreads();
	virtual void joinThreads();
	void modeChangeIncoming() { driver->modeChangeIncoming(); }
	void awaitModeChangeComplete() { driver->awaitModeChangeComplete(); }
	bool rawWrite(const std::vector<uint8_t>& bytes) { return driver->write(bytes); }
	virtual bool sendPacket(std::vector<uint8_t>& bytes);
	bool redirectRead(std::function<void(std::vector<uint8_t>&&)> redirectTo);
	void clearRedirectRead();

	void setWriteBlocks(bool blocks) { driver->writeBlocks = blocks; }

	virtual bool sendCommand(Command cmd, bool boolean) { return sendCommand(cmd, std::vector<uint8_t>({ (uint8_t)boolean })); }
	virtual bool sendCommand(Command cmd, std::vector<uint8_t> arguments = {});
	bool sendCommand(ExtendedCommand cmd, std::vector<uint8_t> arguments = {});
	bool getSettingsSync(std::vector<uint8_t>& data, std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	std::shared_ptr<SerialNumberMessage> getSerialNumberSync(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	optional< std::vector< optional<DeviceAppVersion> > > getVersionsSync(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	std::shared_ptr<LogicalDiskInfoMessage> getLogicalDiskInfoSync(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));

	int addMessageCallback(const MessageCallback& cb);
	bool removeMessageCallback(int id);
	std::shared_ptr<Message> waitForMessageSync(
		const std::shared_ptr<MessageFilter>& f = {},
		std::chrono::milliseconds timeout = std::chrono::milliseconds(50)) {
		return waitForMessageSync([](){ return true; }, f, timeout);
	}
	// onceWaitingDo is a way to avoid race conditions.
	// Return false to bail early, in case your initial command failed.
	std::shared_ptr<Message> waitForMessageSync(
		std::function<bool(void)> onceWaitingDo,
		const std::shared_ptr<MessageFilter>& f = {},
		std::chrono::milliseconds timeout = std::chrono::milliseconds(50));

	std::function<std::unique_ptr<Packetizer>()> makeConfiguredPacketizer;
	std::unique_ptr<Packetizer> packetizer;
	std::unique_ptr<Encoder> encoder;
	std::unique_ptr<Decoder> decoder;
	device_eventhandler_t report;

protected:
	std::unique_ptr<Driver> driver;
	static int messageCallbackIDCounter;
	std::mutex messageCallbacksLock;
	std::map<int, MessageCallback> messageCallbacks;
	std::atomic<bool> closing{false};
	std::atomic<bool> redirectingRead{false};
	std::function<void(std::vector<uint8_t>&&)> redirectionFn;
	std::mutex redirectingReadMutex; // Don't allow read to be disabled while in the redirectionFn

	void dispatchMessage(const std::shared_ptr<Message>& msg);
	void handleInput(Packetizer& p, std::vector<uint8_t>& readBytes);

private:
	std::thread readTaskThread;
	void readTask();
};

}

#endif // __cplusplus

#endif