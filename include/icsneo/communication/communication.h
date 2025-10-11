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
#include "icsneo/communication/message/componentversionsmessage.h"
#include "icsneo/communication/message/extendedresponsemessage.h"
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
	// Note that the Packetizer is not created by the constructor,
	// and should be done once the Communication module is in place.
	Communication(
		device_eventhandler_t report,
		std::unique_ptr<Driver>&& driver,
		std::function<std::unique_ptr<Packetizer>()> makeConfiguredPacketizer,
		std::unique_ptr<Encoder>&& e,
		std::unique_ptr<Decoder>&& md) : makeConfiguredPacketizer(makeConfiguredPacketizer), encoder(std::move(e)), decoder(std::move(md)), driver(std::move(driver)), report(report) {}
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

	void pauseReads();
	void resumeReads();
	bool readsArePaused();

	void setWriteBlocks(bool blocks) { driver->writeBlocks = blocks; }

	virtual bool sendCommand(Command cmd, bool boolean) { return sendCommand(cmd, std::vector<uint8_t>({ (uint8_t)boolean })); }
	virtual bool sendCommand(Command cmd, std::vector<uint8_t> arguments = {});
	bool sendCommand(ExtendedCommand cmd, std::vector<uint8_t> arguments = {});
	bool getSettingsSync(std::vector<uint8_t>& data, std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	std::shared_ptr<SerialNumberMessage> getSerialNumberSync(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	std::optional< std::vector< std::optional<DeviceAppVersion> > > getVersionsSync(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	std::shared_ptr<LogicalDiskInfoMessage> getLogicalDiskInfoSync(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	std::optional< std::vector<ComponentVersion> > getComponentVersionsSync(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));

	int addMessageCallback(const std::shared_ptr<MessageCallback>& cb);
	bool removeMessageCallback(int id);
	std::shared_ptr<Message> waitForMessageSync(
		const std::shared_ptr<MessageFilter>& f = {},
		std::chrono::milliseconds timeout = std::chrono::milliseconds(500)) {
		return waitForMessageSync([](){ return true; }, f, timeout);
	}
	// onceWaitingDo is a way to avoid race conditions.
	// Return false to bail early, in case your initial command failed.
	std::shared_ptr<Message> waitForMessageSync(
		std::function<bool(void)> onceWaitingDo,
		const std::shared_ptr<MessageFilter>& f = {},
		std::chrono::milliseconds timeout = std::chrono::milliseconds(500));

	void dispatchMessage(const std::shared_ptr<Message>& msg);

	std::function<std::unique_ptr<Packetizer>()> makeConfiguredPacketizer;
	std::unique_ptr<Packetizer> packetizer;
	std::unique_ptr<Encoder> encoder;
	std::unique_ptr<Decoder> decoder;
	std::unique_ptr<Driver> driver;
	device_eventhandler_t report;

	size_t readTaskWakeLimit = 1;
	std::chrono::milliseconds readTaskWakeTimeout = std::chrono::milliseconds(1000);

protected:
	static int messageCallbackIDCounter;
	std::mutex messageCallbacksLock;
	std::map<int, std::shared_ptr<MessageCallback>> messageCallbacks;
	std::atomic<bool> closing{false};

	std::condition_variable pauseReadTaskCv;
	std::mutex pauseReadTaskMutex;
	std::atomic<bool> pauseReadTask = false;

	std::mutex syncMessageMutex;

	void handleInput(Packetizer& p);

private:
	std::thread readTaskThread;
	void readTask();
};

}

#endif // __cplusplus

#endif