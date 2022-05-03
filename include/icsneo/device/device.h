#ifndef __DEVICE_H__
#define __DEVICE_H__

#ifdef __cplusplus

#include <vector>
#include <memory>
#include <utility>
#include <cstring>
#include <cstdint>
#include <atomic>
#include <type_traits>
#include "icsneo/api/eventmanager.h"
#include "icsneo/api/lifetime.h"
#include "icsneo/device/neodevice.h"
#include "icsneo/device/idevicesettings.h"
#include "icsneo/device/nullsettings.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/deviceversion.h"
#include "icsneo/device/founddevice.h"
#include "icsneo/disk/diskreaddriver.h"
#include "icsneo/disk/diskwritedriver.h"
#include "icsneo/disk/nulldiskdriver.h"
#include "icsneo/communication/communication.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/communication/io.h"
#include "icsneo/communication/message/resetstatusmessage.h"
#include "icsneo/communication/message/wiviresponsemessage.h"
#include "icsneo/device/extensions/flexray/controller.h"
#include "icsneo/communication/message/flexray/control/flexraycontrolmessage.h"
#include "icsneo/communication/message/ethphymessage.h"
#include "icsneo/third-party/concurrentqueue/concurrentqueue.h"
#include "icsneo/platform/optional.h"
#include "icsneo/platform/nodiscard.h"

#define ICSNEO_FINDABLE_DEVICE_BASE(className, type) \
	static constexpr DeviceType::Enum DEVICE_TYPE = type; \
	className(const FoundDevice& dev) : className(neodevice_t(dev, DEVICE_TYPE), dev.makeDriver) {}

// Devices which are discernable by the first two characters of their serial
#define ICSNEO_FINDABLE_DEVICE(className, type, serialStart) \
	static constexpr const char* SERIAL_START = serialStart; \
	ICSNEO_FINDABLE_DEVICE_BASE(className, type)

// Devices which are discernable by their USB PID
#define ICSNEO_FINDABLE_DEVICE_BY_PID(className, type, pid) \
	static constexpr const uint16_t PRODUCT_ID = pid; \
	ICSNEO_FINDABLE_DEVICE_BASE(className, type)

namespace icsneo {

class DeviceExtension;

class Device {
public:
	virtual ~Device();

	static std::string SerialNumToString(uint32_t serial);
	static uint32_t SerialStringToNum(const std::string& serial);
	static bool SerialStringIsNumeric(const std::string& serial);

	uint16_t getTimestampResolution() const;
	DeviceType getType() const { return DeviceType(data.type); }
	std::string getSerial() const { return data.serial; }
	uint32_t getSerialNumber() const { return Device::SerialStringToNum(getSerial()); }
	const neodevice_t& getNeoDevice() const { return data; }
	virtual std::string getProductName() const { return getType().getGenericProductName(); }
	std::string describe() const;
	friend std::ostream& operator<<(std::ostream& os, const Device& device) {
		os << device.describe();
		return os;
	}

	class OpenFlags {
	public:
		enum Enum {
			/**
			 * Even if the firmware does not match the current firmware version,
			 * the device will not be updated.
			 * 
			 * Note: The device may still be flashed if the device has no firmware
			 * 
			 * This has no effect if the DFU extension is not present
			 */
			SuppressAutoUpdate = 1 << 0,

			/**
			 * Force reflash the device.
			 * 
			 * This has no effect if the DFU extension is not present
			 */
			ForceReflash = 1 << 1
		};
		using EnumType = std::underlying_type<Enum>::type;

		OpenFlags(Enum e = Enum(0)) : val(e) {}
		EnumType operator&(Enum e) const { return EnumType(val) & EnumType(e); }

	private:
		const Enum val;
	};

	enum class OpenDirective {
		Continue,
		Cancel,
		Skip
	};

	enum class OpenStatusType {
		QuestionContinueSkipCancel,
		QuestionContinueCancel,
		Progress
	};

	using OpenStatusHandler = std::function<Device::OpenDirective(OpenStatusType type, const std::string& status, optional<double> progress)>;

	bool open(OpenFlags flags = {}, OpenStatusHandler handler =
		[](OpenStatusType, const std::string&, optional<double>) { return Device::OpenDirective::Continue; });
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

	bool transmit(std::shared_ptr<Frame> frame);
	bool transmit(std::vector<std::shared_ptr<Frame>> frames);

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
	 * Read from the logical disk in this device, starting from byte `pos`
	 * and reading up to `amount` bytes.
	 * 
	 * The number of bytes read will be returned in case of success.
	 * 
	 * If the number of bytes read is less than the amount requested,
	 * an error will be set in icsneo::GetLastError() explaining why.
	 * Likely, either the end of the logical disk has been reached, or
	 * the timeout was reached while the read had only partially completed.
	 *
	 * Upon failure, icsneo::nullopt will be returned and an error will be
	 * set in icsneo::GetLastError().
	 */
	optional<uint64_t> readLogicalDisk(uint64_t pos, uint8_t* into, uint64_t amount,
		std::chrono::milliseconds timeout = Disk::DefaultTimeout);

	/**
	 * Write to the logical disk in this device, starting from byte `pos`
	 * and writing up to `amount` bytes.
	 * 
	 * The number of bytes written will be returned in case of success.
	 * 
	 * If the number of bytes written is less than the amount requested,
	 * an error will be set in icsneo::GetLastError() explaining why.
	 * Likely, either the end of the logical disk has been reached, or
	 * the timeout was reached while the write had only partially completed.
	 *
	 * Upon failure, icsneo::nullopt will be returned and an error will be
	 * set in icsneo::GetLastError().
	 */
	optional<uint64_t> writeLogicalDisk(uint64_t pos, const uint8_t* from, uint64_t amount,
		std::chrono::milliseconds timeout = Disk::DefaultTimeout);

	/**
	 * Check if the logical disk is connected. This means the disk is inserted,
	 * and if required (for instance for multi-card configurations), configured
	 * properly.
	 * 
	 * This method is synchronous and contacts the device for the latest status.
	 * 
	 * `icsneo::nullopt` will be returned if the device does not respond in a
	 * timely manner.
	 */
	optional<bool> isLogicalDiskConnected();

	/**
	 * Get the size of the connected logical disk in bytes.
	 * 
	 * This method is synchronous and contacts the device for the latest status.
	 * 
	 * `icsneo::nullopt` will be returned if the device does not respond in a
	 * timely manner, or if the disk is disconnected/improperly configured.
	 */
	optional<uint64_t> getLogicalDiskSize();

	/**
	 * Get the offset to the VSA filesystem within the logical disk, represented
	 * in bytes.
	 * 
	 * This method is synchronous and consacts the device for the latest status
	 * if necessary.
	 * 
	 * `icsneo::nullopt` will be returned if the device does not respond in a
	 * timely manner, or if the disk is disconnected/improperly configured.
	 */
	optional<uint64_t> getVSAOffsetInLogicalDisk();

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

	typedef std::function< void(uint32_t startSector, uint32_t endSector) > NewCaptureCallback;

	/**
	 * Add a callback which will be called for all new captures.
	 *
	 * This is invalid for devices which are not running the Wireless neoVI stack.
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addNewCaptureCallback(NewCaptureCallback cb);

	typedef std::function< void(uint16_t connectionTimeoutMinutes) > SleepRequestedCallback;

	/**
	 * Add a callback which will be called when a Wireless neoVI device is
	 * ready for sleep, pending any uploads we might want to complete first.
	 *
	 * Call Device::allowSleep() once ready to signal that status to the device.
	 * 
	 * Check Device::isSleepRequested() to check if the sleep request was interrupted.
	 * In that case, the sleep requested callbacks will be called again.
	 *
	 * This is invalid for devices which are not running the Wireless neoVI stack.
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addSleepRequestedCallback(SleepRequestedCallback cb);

	/**
	 * Check whether sleep has been requested by a VSSAL Wireless neoVI script.
	 */
	optional<bool> isSleepRequested() const;

	/**
	 * Signal to a running VSSAL Wireless neoVI script that we are ready for
	 * sleep.
	 *
	 * If @param remoteWakeup is specified, the modem will be kept running in sleep
	 * mode, where supported.
	 *
	 * This is invalid for devices which are not running the Wireless neoVI stack.
	 */
	bool allowSleep(bool remoteWakeup = false);

	virtual std::vector<std::shared_ptr<FlexRay::Controller>> getFlexRayControllers() const { return {}; }

	void addExtension(std::shared_ptr<DeviceExtension>&& extension);

	/**
	 * For use by extensions only.
	 */
	NODISCARD("If the Lifetime is not held, disconnects will be immediately unsuppressed")
	Lifetime suppressDisconnects();

	/**
	 * For use by extensions only. A more stable API will be provided in the future.
	 */
	const std::vector<optional<DeviceAppVersion>>& getVersions() const { return versions; }

	/**
	 * Some alternate communication protocols do not support DFU
	 */
	virtual bool currentDriverSupportsDFU() const { return true; }

	const device_eventhandler_t& getEventHandler() const { return report; }

	/**
	 * Tell whether the current device supports reading and writing
	 * Ethernet PHY registers through MDIO.
	 */
	virtual bool getEthPhyRegControlSupported() const { return false; }

	/**
	 * Returns true if this device supports the Wireless neoVI featureset
	 */
	virtual bool supportsWiVI() const { return false; }

	optional<EthPhyMessage> sendEthPhyMsg(const EthPhyMessage& message, std::chrono::milliseconds timeout = std::chrono::milliseconds(50));

	std::shared_ptr<Communication> com;
	std::unique_ptr<IDeviceSettings> settings;

protected:
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
	Device(neodevice_t neodevice) : data(neodevice) {
		data.device = this;
	}
	
	template<typename Settings = NullSettings, typename DiskRead = Disk::NullDriver, typename DiskWrite = Disk::NullDriver>
	void initialize(const driver_factory_t& makeDriver) {
		report = makeEventHandler();
		auto encoder = makeEncoder();
		setupEncoder(*encoder);
		auto decoder = makeDecoder();
		setupDecoder(*decoder);
		com = makeCommunication(
			makeDriver(report, getWritableNeoDevice()),
			std::bind(&Device::makeConfiguredPacketizer, this),
			std::move(encoder),
			std::move(decoder)
		);
		setupCommunication(*com);
		settings = makeSettings<Settings>(com);
		setupSettings(*settings);
		diskReadDriver = std::unique_ptr<DiskRead>(new DiskRead());
		diskWriteDriver = std::unique_ptr<DiskWrite>(new DiskWrite());
		setupSupportedRXNetworks(supportedRXNetworks);
		setupSupportedTXNetworks(supportedTXNetworks);
		setupExtensions();
	}

	virtual device_eventhandler_t makeEventHandler() {
		return [this](APIEvent::Type type, APIEvent::Severity severity) {
			EventManager::GetInstance().add(type, severity, this);
		};
	}

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
	virtual void setupCommunication(Communication& communication) {
		communication.packetizer = communication.makeConfiguredPacketizer();
	}

	template<typename Settings>
	std::unique_ptr<IDeviceSettings> makeSettings(std::shared_ptr<Communication> com) {
		return std::unique_ptr<IDeviceSettings>(new Settings(com));
	}
	virtual void setupSettings(IDeviceSettings&) {}

	virtual void setupSupportedRXNetworks(std::vector<Network>&) {}
	virtual void setupSupportedTXNetworks(std::vector<Network>&) {}

	virtual void setupExtensions() {}

	// Hook for devices such as FIRE which need to inject traffic before RequestSerialNumber
	// Return false to bail
	virtual bool afterCommunicationOpen() { return true; }

	virtual bool requiresVehiclePower() const { return true; }

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

	virtual void handleDeviceStatus(const std::shared_ptr<RawMessage>&) {}

	neodevice_t& getWritableNeoDevice() { return data; }

private:
	neodevice_t data;
	std::shared_ptr<ResetStatusMessage> latestResetStatus;
	std::vector<optional<DeviceAppVersion>> versions;

	mutable std::mutex diskLock;
	std::unique_ptr<Disk::ReadDriver> diskReadDriver;
	std::unique_ptr<Disk::WriteDriver> diskWriteDriver;

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

	bool firmwareUpdateSupported();

	APIEvent::Type getCommunicationNotEstablishedError();
	
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
	std::mutex heartbeatMutex;
	std::thread heartbeatThread;

	// Wireless neoVI Stack
	std::atomic<bool> stopWiVIThread{false};
	std::condition_variable stopWiVIcv;
	mutable std::mutex wiviMutex;
	std::thread wiviThread;
	std::atomic<bool> wiviSleepRequested{false};
	std::vector<NewCaptureCallback> newCaptureCallbacks;
	std::vector< std::pair<SleepRequestedCallback, bool /* notified */> > sleepRequestedCallbacks;
	void wiviThreadBody();
	void stopWiVIThreadIfNecessary(std::unique_lock<std::mutex> lk);
};

}

#endif // __cplusplus

#endif