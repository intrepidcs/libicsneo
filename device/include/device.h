#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <vector>
#include <memory>
#include <cstring>
#include "device/include/neodevice.h"
#include "device/include/idevicesettings.h"
#include "device/include/devicetype.h"
#include "communication/include/communication.h"
#include "communication/include/packetizer.h"
#include "communication/include/encoder.h"
#include "communication/include/decoder.h"
#include "third-party/concurrentqueue/concurrentqueue.h"

namespace icsneo {

class Device {
public:
	static constexpr const char* SERIAL_FIND_ON_OPEN = "xxxxxx";

	Device(neodevice_t neodevice = { 0 }) {
		data = neodevice;
		data.device = this;
	}
	virtual ~Device() {
		disableMessagePolling();
		close();
	}

	static std::string SerialNumToString(uint32_t serial);
	static uint32_t SerialStringToNum(const std::string& serial);
	static bool SerialStringIsNumeric(const std::string& serial);

	DeviceType getType() const { return DeviceType(data.type); }
	uint16_t getProductId() const { return productId; }
	std::string getSerial() const { return data.serial; }
	uint32_t getSerialNumber() const { return Device::SerialStringToNum(getSerial()); }
	const neodevice_t& getNeoDevice() const { return data; }

	virtual bool open();
	virtual bool close();
	virtual bool isOnline() const { return online; }
	virtual bool goOnline();
	virtual bool goOffline();

	// Message polling related functions
	void enableMessagePolling();
	bool disableMessagePolling();
	std::vector<std::shared_ptr<Message>> getMessages();
	bool getMessages(std::vector<std::shared_ptr<Message>>& container, size_t limit = 0);
	size_t getCurrentMessageCount() { return pollingContainer.size_approx(); }
	size_t getPollingMessageLimit() { return pollingMessageLimit; }
	void setPollingMessageLimit(size_t newSize) {
		pollingMessageLimit = newSize;
		enforcePollingMessageLimit();
	}

	std::unique_ptr<IDeviceSettings> settings;

protected:
	uint16_t productId = 0;
	bool online = false;
	int messagePollingCallbackID = 0;
	std::shared_ptr<Communication> com;

	neodevice_t& getWritableNeoDevice() { return data; }

private:
	neodevice_t data;
	
	size_t pollingMessageLimit = 20000;
	moodycamel::ConcurrentQueue<std::shared_ptr<Message>> pollingContainer;
	void enforcePollingMessageLimit();
};

}

#endif