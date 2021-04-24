#ifndef __DEVICE_H__
#define __DEVICE_H__

#ifdef __cplusplus

#include <vector>
#include <memory>
#include <utility>
#include <cstring>
#include <atomic>
#include "icsneo/api/eventmanager.h"
#include "icsneo/api/lifetime.h"
#include "icsneo/device/neodevice.h"
#include "icsneo/device/idevicesettings.h"
#include "icsneo/device/nullsettings.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/extensions/deviceextension.h"
#include "icsneo/communication/communication.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/communication/io.h"
#include "icsneo/communication/message/resetstatusmessage.h"
#include "icsneo/device/extensions/flexray/controller.h"
#include "icsneo/communication/message/flexray/control/flexraycontrolmessage.h"
#include "icsneo/third-party/concurrentqueue/concurrentqueue.h"
#include "icsneo/platform/optional.h"
#include "icsneo/platform/nodiscard.h"

namespace icsneo {

class Device {
public:
	virtual ~Device();

	static std::string SerialNumToString(uint32_t serial);
	static uint32_t SerialStringToNum(const std::string& serial);
	static bool SerialStringIsNumeric(const std::string& serial);

	uint16_t getTimestampResolution() const;
	DeviceType getType() const { return DeviceType(data.type); }
	uint16_t getProductId() const { return productId; }
	std::string getSerial() const { return data.serial; }
	uint32_t getSerialNumber() const { return Device::SerialStringToNum(getSerial()); }
	const neodevice_t& getNeoDevice() const { return data; }
	virtual std::string getProductName() const { return getType().getGenericProductName(); }
	std::string describe() const;
	friend std::ostream& operator<<(std::ostream& os, const Device& device) {
		os << device.describe();
		return os;
	}

	virtual bool open();
	virtual bool close();
	virtual bool isOnline() const { return online; }
	virtual bool isOpen() const { return com->isOpen(); }
	virtual bool isDisconnected() const { return com->isDisconnected(); }
	virtual bool goOnline();
	virtual bool goOffline();

	// Message polling related functions
	bool enableMessagePolling();
	bool disableMessagePolling();
	bool isMessagePollingEnabled() { return messagePollingCallbackID != 0; };
	std::pair<std::vector<std::shared_ptr<Message>>, bool> getMessages();
	bool getMessages(std::vector<std::shared_ptr<Message>>& container, size_t limit = 0, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
	size_t getCurrentMessageCount() { return pollingContainer.size_approx(); }
	size_t getPollingMessageLimit() { return pollingMessageLimit; }
	void setPollingMessageLimit(size_t newSize) {
		pollingMessageLimit = newSize;
		enforcePollingMessageLimit();
	}

	int addMessageCallback(const MessageCallback& cb) { return com->addMessageCallback(cb); }
	bool removeMessageCallback(int id) { return com->removeMessageCallback(id); }

	bool transmit(std::shared_ptr<Message> message);
	bool transmit(std::vector<std::shared_ptr<Message>> messages);

	void setWriteBlocks(bool blocks);

	const std::vector<Network>& getSupportedRXNetworks() const { return supportedRXNetworks; }
	const std::vector<Network>& getSupportedTXNetworks() const { return supportedTXNetworks; }
	virtual bool isSupportedRXNetwork(const Network& net) const {
		return std::find(supportedRXNetworks.begin(), supportedRXNetworks.end(), net) != supportedRXNetworks.end();
	}
	virtual bool isSupportedTXNetwork(const Network& net) const {
		return std::find(supportedTXNetworks.begin(), supportedTXNetworks.end(), net) != supportedTXNetworks.end();
	}

	virtual size_t getNetworkCountByType(Network::Type) const;
	virtual Network getNetworkByNumber(Network::Type, size_t) const;

	/**
	 * Retrieve the number of Ethernet (DoIP) Activation lines present
	 * on this device.
	 */
	virtual size_t getEthernetActivationLineCount() const { return 0; }

	/**
	 * Retrieve the number of power-controlled USB Host ports present
	 * on this device.
	 */
	virtual size_t getUSBHostPowerCount() const { return 0; }

	/**
	 * Tell whether the current device supports controlling a backup
	 * power source through the API.
	 */
	virtual bool getBackupPowerSupported() const { return false; }

	/**
	 * Retrieve the information about the misc IOs present on this
	 * device.
	 */
	virtual std::vector<MiscIO> getMiscIO() const { return {}; }

	/**
	 * Retrieve the information about the emisc IOs present on this
	 * device.
	 */
	virtual std::vector<MiscIO> getEMiscIO() const { return {}; }

	/**
	 * Get the value of a digital IO, returning an empty optional if the
	 * value is not present, the specified IO is not valid for this device,
	 * or if an error occurs.
	 * 
	 * The index number starts counting at 1 to keep the numbers in sync
	 * with the numbering on the device, and is set to 1 by default.
	 */
	optional<bool> getDigitalIO(IO type, size_t number = 1);

	/**
	 * Set a digital IO to either a 1, if value is true, or 0 otherwise.
	 *
	 * The index number starts counting at 1 to keep the numbers in sync
	 * with the numbering on the device.
	 */
	bool setDigitalIO(IO type, size_t number, bool value);

	/**
	 * Set the first digital IO of a given type to either a 1, if value
	 * is true, or 0 otherwise.
	 */
	bool setDigitalIO(IO type, bool value) { return setDigitalIO(type, 1, value); }

	/**
	 * Get the value of an analog IO, returning an empty optional if the
	 * value is not present, the specified IO is not valid for this device,
	 * or if an error occurs.
	 * 
	 * The index number starts counting at 1 to keep the numbers in sync
	 * with the numbering on the device, and is set to 1 by default.
	 */
	optional<double> getAnalogIO(IO type, size_t number = 1);

	virtual std::vector<std::shared_ptr<FlexRay::Controller>> getFlexRayControllers() const { return {}; }

	/**
	 * For use by extensions only.
	 */
	NODISCARD("If the Lifetime is not held, disconnects will be immediately unsuppressed")
	Lifetime suppressDisconnects();

	const device_eventhandler_t& getEventHandler() const { return report; }

	std::shared_ptr<Communication> com;
	std::unique_ptr<IDeviceSettings> settings;

protected:
	uint16_t productId = 0;
	bool online = false;
	int messagePollingCallbackID = 0;
	int internalHandlerCallbackID = 0;
	device_eventhandler_t report;

	std::mutex ioMutex;
	optional<bool> ethActivationStatus;
	optional<bool> usbHostPowerStatus;
	optional<bool> backupPowerEnabled;
	optional<bool> backupPowerGood;
	std::array<optional<bool>, 6> miscDigital;
	std::array<optional<double>, 2> miscAnalog;

	// START Initialization Functions
	Device(neodevice_t neodevice = { 0 }) {
		data = neodevice;
		data.device = this;
	}
	
	template<typename Driver, typename Settings = NullSettings>
	void initialize() {
		report = makeEventHandler();
		auto driver = makeDriver<Driver>();
		setupDriver(*driver);
		auto encoder = makeEncoder();
		setupEncoder(*encoder);
		auto decoder = makeDecoder();
		setupDecoder(*decoder);
		com = makeCommunication(std::move(driver), std::bind(&Device::makeConfiguredPacketizer, this), std::move(encoder), std::move(decoder));
		setupCommunication(*com);
		settings = makeSettings<Settings>(com);
		setupSettings(*settings);
		setupSupportedRXNetworks(supportedRXNetworks);
		setupSupportedTXNetworks(supportedTXNetworks);
		setupExtensions();
	}

	virtual device_eventhandler_t makeEventHandler() {
		return [this](APIEvent::Type type, APIEvent::Severity severity) {
			EventManager::GetInstance().add(type, severity, this);
		};
	}

	template<typename Driver>
	std::unique_ptr<Driver> makeDriver() { return std::unique_ptr<Driver>(new Driver(report, getWritableNeoDevice())); }
	virtual void setupDriver(Driver&) {}

	virtual std::unique_ptr<Packetizer> makePacketizer() { return std::unique_ptr<Packetizer>(new Packetizer(report)); }
	virtual void setupPacketizer(Packetizer&) {}
	std::unique_ptr<Packetizer> makeConfiguredPacketizer() {
		auto packetizer = makePacketizer();
		setupPacketizer(*packetizer);
		return packetizer;
	}

	virtual std::unique_ptr<Encoder> makeEncoder() { return std::unique_ptr<Encoder>(new Encoder(report)); }
	virtual void setupEncoder(Encoder&) {}

	virtual std::unique_ptr<Decoder> makeDecoder() { return std::unique_ptr<Decoder>(new Decoder(report)); }
	virtual void setupDecoder(Decoder&) {}

	virtual std::shared_ptr<Communication> makeCommunication(
		std::unique_ptr<Driver> t,
		std::function<std::unique_ptr<Packetizer>()> makeConfiguredPacketizer,
		std::unique_ptr<Encoder> e,
		std::unique_ptr<Decoder> d) { return std::make_shared<Communication>(report, std::move(t), makeConfiguredPacketizer, std::move(e), std::move(d)); }
	virtual void setupCommunication(Communication&) {}

	template<typename Settings>
	std::unique_ptr<IDeviceSettings> makeSettings(std::shared_ptr<Communication> com) {
		return std::unique_ptr<IDeviceSettings>(new Settings(com));
	}
	virtual void setupSettings(IDeviceSettings&) {}

	virtual void setupSupportedRXNetworks(std::vector<Network>&) {}
	virtual void setupSupportedTXNetworks(std::vector<Network>&) {}

	virtual void setupExtensions() {}
	void addExtension(std::shared_ptr<DeviceExtension>&& extension);

	// Hook for devices such as FIRE which need to inject traffic before RequestSerialNumber
	// Return false to bail
	virtual bool afterCommunicationOpen() { return true; }
	
	template<typename Extension>
	std::shared_ptr<Extension> getExtension() const {
		std::shared_ptr<Extension> ret;
		std::lock_guard<std::mutex> lk(extensionsLock);
		for(auto& ext : extensions) {
			if((ret = std::dynamic_pointer_cast<Extension>(ext)))
				break;
		}
		return ret;
	}
	// END Initialization Functions

	void handleInternalMessage(std::shared_ptr<Message> message);

	virtual void handleDeviceStatus(const std::shared_ptr<Message>& message) {}

	neodevice_t& getWritableNeoDevice() { return data; }

private:
	neodevice_t data;
	std::shared_ptr<ResetStatusMessage> latestResetStatus;

	mutable std::mutex extensionsLock;
	std::vector<std::shared_ptr<DeviceExtension>> extensions;
	void forEachExtension(std::function<bool(const std::shared_ptr<DeviceExtension>&)> fn);

	std::vector<Network> supportedTXNetworks;
	std::vector<Network> supportedRXNetworks;
	
	APIEvent::Type attemptToBeginCommunication();

	// Use heartbeatSuppressed instead when reading
	std::atomic<int> heartbeatSuppressedByUser{0};
	bool heartbeatSuppressed() const { return heartbeatSuppressedByUser > 0 || (settings && settings->applyingSettings); }

	void handleNeoVIMessage(std::shared_ptr<CANMessage> message);
	
	enum class LEDState : uint8_t {
		Offline = 0x04,
		CoreMiniRunning = 0x08, // This should override "offline" if the CoreMini is running
		Online = 0x10
	};
	LEDState ledState;
	void updateLEDState();
	
	size_t pollingMessageLimit = 20000;
	moodycamel::BlockingConcurrentQueue<std::shared_ptr<Message>> pollingContainer;
	void enforcePollingMessageLimit();

	std::atomic<bool> stopHeartbeatThread{false};
	std::thread heartbeatThread;
};

}

#endif // __cplusplus

#endif