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
#include <optional>
#include <unordered_map>
#include <set>
#include <chrono>
#include "icsneo/api/eventmanager.h"
#include "icsneo/api/lifetime.h"
#include "icsneo/api/periodic.h"
#include "icsneo/device/neodevice.h"
#include "icsneo/device/idevicesettings.h"
#include "icsneo/device/nullsettings.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/deviceversion.h"
#include "icsneo/device/founddevice.h"
#include "icsneo/device/coremini.h"
#include "icsneo/disk/diskreaddriver.h"
#include "icsneo/disk/diskwritedriver.h"
#include "icsneo/disk/nulldiskdriver.h"
#include "icsneo/disk/diskdetails.h"
#include "icsneo/communication/communication.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/communication/io.h"
#include "icsneo/communication/message/resetstatusmessage.h"
#include "icsneo/communication/message/wiviresponsemessage.h"
#include "icsneo/communication/message/scriptstatusmessage.h"
#include "icsneo/communication/message/supportedfeaturesmessage.h"
#include "icsneo/communication/message/genericbinarystatusmessage.h"
#include "icsneo/communication/message/hardwareinfo.h"
#include "icsneo/communication/message/extendeddatamessage.h"
#include "icsneo/communication/message/livedatamessage.h"
#include "icsneo/communication/message/tc10statusmessage.h"
#include "icsneo/communication/message/macsecmessage.h"
#include "icsneo/communication/packet/genericbinarystatuspacket.h"
#include "icsneo/communication/packet/livedatapacket.h"
#include "icsneo/device/extensions/flexray/controller.h"
#include "icsneo/communication/message/flexray/control/flexraycontrolmessage.h"
#include "icsneo/communication/message/ethphymessage.h"
#include "icsneo/third-party/concurrentqueue/concurrentqueue.h"
#include "icsneo/platform/nodiscard.h"
#include "icsneo/disk/vsa/vsa.h"
#include "icsneo/disk/vsa/vsaparser.h"
#include "icsneo/communication/message/versionmessage.h"
#include "icsneo/communication/message/gptpstatusmessage.h"


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

// Devices which are discernable by a serial range
#define ICSNEO_FINDABLE_DEVICE_BY_SERIAL_RANGE(className, type, serialLow, serialHigh) \
	static constexpr const char* SERIAL_RANGE_LOW = serialLow; \
	static constexpr const char* SERIAL_RANGE_HIGH = serialHigh; \
	ICSNEO_FINDABLE_DEVICE_BASE(className, type)

namespace icsneo {

class DeviceExtension;

typedef uint64_t MemoryAddress;

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

	using OpenStatusHandler = std::function<Device::OpenDirective(OpenStatusType type, const std::string& status, std::optional<double> progress)>;

	bool open(OpenFlags flags = {}, OpenStatusHandler handler =
		[](OpenStatusType, const std::string&, std::optional<double>) { return Device::OpenDirective::Continue; });
	virtual bool close();
	virtual bool isOnline() const { return online; }
	virtual bool isOpen() const { return com->isOpen(); }
	virtual bool isDisconnected() const { return com->isDisconnected(); }
	virtual bool goOnline();
	virtual bool goOffline();
	virtual bool enableLogData();
	virtual bool disableLogData();

	enum class PreloadReturn : uint8_t
	{
		Pending,
		Ok,
		OKEncrypted,
		NoScript,
	};



	int8_t prepareScriptLoad();
	bool startScript(Disk::MemoryType memType = Disk::MemoryType::SD);
	bool stopScript();
	bool clearScript(Disk::MemoryType memType = Disk::MemoryType::SD);
	bool uploadCoremini(std::istream& stream, Disk::MemoryType memType = Disk::MemoryType::SD);
	std::optional<CoreminiHeader> readCoreminiHeader(Disk::MemoryType memType = Disk::MemoryType::SD);

	bool eraseScriptMemory(Disk::MemoryType memType, uint64_t amount);

	virtual std::optional<MemoryAddress> getCoreminiStartAddressFlash() const {
		return std::nullopt;
	}

	virtual std::optional<MemoryAddress> getCoreminiStartAddressSD() const {
		return std::nullopt;
	}

	bool supportsCoreminiScript() const {
		return (getCoreminiStartAddressFlash()) || (getCoreminiStartAddressSD());
	}

	std::optional<MemoryAddress> getCoreminiStartAddress(Disk::MemoryType memType) const {
		switch(memType) {
			case Disk::MemoryType::Flash:
				return getCoreminiStartAddressFlash();
			case Disk::MemoryType::SD:
				return getCoreminiStartAddressSD();
			default:
				break;
		}
		return std::nullopt;
	}

	virtual bool supportsEraseMemory() const {
		return false;
	}


	// Message polling related functions


	bool enableMessagePolling(std::optional<MessageFilter> filter = std::nullopt);
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

	int addMessageCallback(const std::shared_ptr<MessageCallback>& cb) { return com->addMessageCallback(cb); }
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

	std::shared_ptr<HardwareInfo> getHardwareInfo(std::chrono::milliseconds timeout = std::chrono::milliseconds(100));


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
	std::optional<uint64_t> readLogicalDisk(uint64_t pos, uint8_t* into, uint64_t amount,
		std::chrono::milliseconds timeout = Disk::DefaultTimeout, Disk::MemoryType memType = Disk::MemoryType::SD);

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
	std::optional<uint64_t> writeLogicalDisk(uint64_t pos, const uint8_t* from, uint64_t amount,
		std::chrono::milliseconds timeout = Disk::DefaultTimeout, Disk::MemoryType memType = Disk::MemoryType::SD);

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
	std::optional<bool> isLogicalDiskConnected();

	/**
	 * Get the size of the connected logical disk in bytes.
	 * 
	 * This method is synchronous and contacts the device for the latest status.
	 * 
	 * `icsneo::nullopt` will be returned if the device does not respond in a
	 * timely manner, or if the disk is disconnected/improperly configured.
	 */
	std::optional<uint64_t> getLogicalDiskSize();

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
	std::optional<uint64_t> getVSAOffsetInLogicalDisk();

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
	std::optional<bool> getDigitalIO(IO type, size_t number = 1);

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
	std::optional<double> getAnalogIO(IO type, size_t number = 1);

	struct WiVIUpload
	{
		bool cellular;
		bool wifi;
		bool isPrePost;
		bool isPreTime;
		uint32_t preTriggerSize;
		uint16_t priority;
		uint16_t captureIndex;
		uint32_t startSector;
		uint32_t endSector;
	};
	typedef std::function< void(WiVIUpload upload) > NewCaptureCallback;

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
	std::optional<bool> isSleepRequested() const;

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

	enum class ScriptStatus {
		CoreMiniRunning = 0,
		SectorOverflow = 1,
		RemainingSectors = 2,
		LastSector = 3,
		ReadBinSize = 4,
		MinSector = 5,
		MaxSector = 6,
		CurrentSector = 7,
		CoreMiniCreateTime = 8,
		FileChecksum = 9,
		CoreMiniVersion = 10,
		CoreMiniHeaderSize = 11,
		DiagnosticErrorCode = 12,
		DiagnosticErrorCodeCount = 13,
		MaxCoreMiniSize = 14,
		Logging = 15,
		IsEncrypted = 16,
	};

	enum RootDirectoryEntryFlags : uint8_t {
		IsPrePost = 1,
		PrePostTriggered = (1 << 1),
		UploadPriority = (1 << 2) | (1 << 3),
		CellularEnabled = (1 << 4),
		WiFiEnabled = (1 << 5),
		Uploaded = (1 << 6),
		Unused = (1 << 7)
	};

	typedef std::function< void(uint64_t value) > ScriptStatusCallback;

	/**
	 * Get all current script status values
	 */
	std::shared_ptr<ScriptStatusMessage> getScriptStatus() const;

	/**
	 * Add a callback to be called when VSSAL script running state changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addCoreMiniRunningCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::CoreMiniRunning, std::move(cb)); }

	/**
	 * Add a callback to be called when the VSSAL script encryption mode changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addEncryptedModeCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::IsEncrypted, std::move(cb)); }

	/**
	 * Add a callback to be called when the number of times a sector was dropped due to lack of space
	 * in firmware's filesystem buffer changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addSectorOverflowsCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::SectorOverflow, std::move(cb)); }

	/**
	 * Add a callback to be called when number of sectors of space left in firmware's local file system buffer changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addNumberRemainingSectorsCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::RemainingSectors, std::move(cb)); }

	/**
	 * Add a callback to be called when last sector that was written to changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addLastSectorCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::LastSector, std::move(cb)); }

	/**
	 * Add a callback to be called when the size of the ReadBin changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addReadBinSizeCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::ReadBinSize, std::move(cb)); }

	/**
	 * Add a callback to be called when the first sector address of logged data changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addMinSectorCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::MinSector, std::move(cb)); }

	/**
	 * Add a callback to be called when the last sector address of logged data changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addMaxSectorCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::MaxSector, std::move(cb)); }

	/**
	 * Add a callback to be called when the sector that is about to be written changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addCurrentSectorCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::CurrentSector, std::move(cb)); }

	/**
	 * Add a callback to be called when the VSSAL script create time changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addCoreMiniCreateTimeCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::CoreMiniCreateTime, std::move(cb)); }

	/**
	 * Add a callback to be called when the VSSAL script checksum changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addFileChecksumCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::FileChecksum, std::move(cb)); }

	/**
	 * Add a callback to be called when the VSSAL script version changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addCoreMiniVersionCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::CoreMiniVersion, std::move(cb)); }

	/**
	 * Add a callback to be called when the VSSAL script header size changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addCoreMiniHeaderSizeCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::CoreMiniHeaderSize, std::move(cb)); }

	/**
	 * Add a callback to be called when the firmware diagnostic error code changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addDiagnosticErrorCodeCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::DiagnosticErrorCode, std::move(cb)); }

	/**
	 * Add a callback to be called when the firmware diagnostic error code count changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addDiagnosticErrorCodeCountCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::DiagnosticErrorCodeCount, std::move(cb)); }

	/**
	 * Add a callback to be called when the maximum size a VSSAL script can be changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addMaxCoreMiniSizeCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::MaxCoreMiniSize, std::move(cb)); }

	/**
	 * Add a callback to be called when the device logging state changes
	 */
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addLoggingCallback(ScriptStatusCallback cb) { return addScriptStatusCallback(ScriptStatus::Logging, std::move(cb)); }

	typedef std::function<void(void)> VINAvailableCallback;
	NODISCARD("If the Lifetime is not held, the callback will be immediately removed")
	Lifetime addVINAvailableCallback(VINAvailableCallback cb);
	std::optional<bool> isVINEnabled() const;
	std::optional<std::string> getVIN() const;

	virtual std::vector<std::shared_ptr<FlexRay::Controller>> getFlexRayControllers() const { return {}; }

	void addExtension(std::shared_ptr<DeviceExtension>&& extension);

	/**
	 * For use by extensions only.
	 */
	NODISCARD("If the Lifetime is not held, disconnects will be immediately unsuppressed")
	Lifetime suppressDisconnects();

	bool refreshComponentVersions();
	/**
	 * For use by extensions only. A more stable API will be provided in the future.
	 */
	const std::vector<std::optional<DeviceAppVersion>>& getVersions() const { return versions; }
	const std::vector<ComponentVersion>& getComponentVersions() const { return componentVersions; }

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

	//RTC declarations
	std::optional<std::chrono::time_point<std::chrono::system_clock>> getRTC();
	bool setRTC(const std::chrono::time_point<std::chrono::system_clock>& time);

	// Get a bitfield from the device representing supported networks and features.
	std::optional<std::set<SupportedFeature>> getSupportedFeatures();

	/**
	 * Returns true if this device supports the Wireless neoVI featureset
	 */
	virtual bool supportsWiVI() const { return false; }

	// Returns true if this device supports Live Data subscription
	virtual bool supportsLiveData() const { return false; }

	std::optional<EthPhyMessage> sendEthPhyMsg(const EthPhyMessage& message, std::chrono::milliseconds timeout = std::chrono::milliseconds(50));


	/**
	 * Set the flags of the root directory entry specified at given address
	 *
	 * Will not allow changes of IsPrePost and PrePostTriggered flags and will produce a warning
	 * if there is an attempt to do so
	 *
	 * @param mask Flags to set, with each bit representing a different entry flag @RootDirectoryEntryFlags
	 * @param values The values in which to set each flag, each bit corresponding to the flag in the same position
	 * @param collectionEntryByteAddress The position of the root directory entry in which to set these flags
	 * @return Success or failure
	 */
	std::optional<bool> SetRootDirectoryEntryFlags(uint8_t mask, uint8_t values, uint32_t collectionEntryByteAddress);

	std::shared_ptr<Communication> com;
	std::shared_ptr<IDeviceSettings> settings;

	std::optional<size_t> getGenericBinarySize(uint16_t binaryIndex);
	bool readBinaryFile(std::ostream& stream, uint16_t binaryIndex);
	bool writeBinaryFile(const std::vector<uint8_t>& in, uint16_t binaryIndex);
	bool subscribeLiveData(std::shared_ptr<LiveDataCommandMessage> message);
	bool unsubscribeLiveData(const LiveDataHandle& handle);
	bool clearAllLiveData();
	bool setValueLiveData(std::shared_ptr<LiveDataSetValueMessage> message);
	
	enum class DiskFormatDirective : uint8_t {
		Continue,
		Stop
	};

	using DiskFormatProgress = std::function<DiskFormatDirective(uint64_t sectorsFormatted, uint64_t sectorsTotal)>;
	bool formatDisk(
		const DiskDetails& config,
		const DiskFormatProgress& handler = {},
		std::chrono::milliseconds interval = std::chrono::milliseconds(500) /** Send updates at an interval of 500ms */
	);
	bool forceDiskConfigUpdate(const DiskDetails& config); // Forces a disk layout and enables change without formatting
	std::shared_ptr<DiskDetails> getDiskDetails(std::chrono::milliseconds timeout = std::chrono::milliseconds(100));
	virtual size_t getDiskCount() const { return 0; }
	bool supportsDiskFormatting() const { return getDiskCount() != 0; }

	// VSA Read functions

	/**
	 * Read VSA message records from disk and dispatch the messages via Communication object. Default behavior excludes
	 * records older than the current CoreMini script and performs a full disk dump of other records. The CoreMini script is
	 * also stopped by default.
	 *
	 * @param extractionSettings Contains filters and other advanced settings for extraction process
	 *
	 * @return Returns false if there were failures during the read or parse processes or issues with record formatting, else true
	 */
	bool readVSA(const VSAExtractionSettings& extractionSettings = VSAExtractionSettings());

	/**
	 * Determines important metadata about VSA record storage on the disk. Terminates at first failed attempt to retrieve information
	 *
	 * @param metadata The metadata object to store the probed information into
	 * @param extractionSettings The settings for this extraction of VSA data
	 *
	 * @return True if all metadata information was successfully found
	 */
	bool probeVSA(VSAMetadata& metadata, const VSAExtractionSettings& extractionSettings);

	/**
	 * Find the first VSA record chronologically from ring buffer in the VSA log file on disk
	 *
	 * @param firstOffset Variable used to pass out offset of first record on the disk
	 * @param firstRecord Variable used to pass out the first record in the buffer
	 * @param extractionSettings The settings for this extraction of VSA data
	 * @param optMetadata Metadata about the current state of the VSA log file
	 * (Must include valid CoreMini timestamp, disk size, and isOverlapped values)
	 *
	 * @return True if the first record was found successfully
	 */
	bool findFirstVSARecord(uint64_t& firstOffset, std::shared_ptr<VSA>& firstRecord,
		const VSAExtractionSettings& extractionSettings = VSAExtractionSettings(), std::optional<VSAMetadata> optMetadata = std::nullopt);

	/**
	 * Find the last record chronologically from ring buffer in the VSA log file with a valid timestamp
	 *
	 * @param lastOffset Variable used to pass out the offset of the last record on the disk
	 * @param lastRecord Variable used to pass out the last record with a valid timestamp
	 * @param extractionSettings The settings for this extraction of VSA data
	 * @param optMetadata Metadata about the current state of the VSA log file
	 * (Must include valid CoreMini timestamp, disk size, and isOverlapped values)
	 *
	 * @return True if the last record was found successfully
	 */
	bool findLastVSARecord(uint64_t& lastOffset, std::shared_ptr<VSA>& lastRecord,
		const VSAExtractionSettings& extractionSettings = VSAExtractionSettings(), std::optional<VSAMetadata> optMetadata = std::nullopt);

	/**
	 * Find the closest VSA record to the desired time_point
	 *
	 * @param point The desired time_point of the record
	 * @param vsaOffset Variable used to pass out offset of record closest to the desired time_point
	 * @param record Variable used to pass out the record closest to the desired time_point
	 * @param extractionSettings Settings for this extraction of VSA data
	 * @param optMetadata Optional param to include metadata about the VSA log file on disk
	 *
	 * @return Pair containing the location of the record closest to the desired time_point (in bytes from the beginning of VSA log file) and the record itself
	 */
	bool findVSAOffsetFromTimepoint(
		ICSClock::time_point point, uint64_t& vsaOffset, std::shared_ptr<VSA>& record, const VSAExtractionSettings& extractionSettings = VSAExtractionSettings(),
		std::optional<VSAMetadata> optMetadata = std::nullopt);

	/**
	 * Parse VSA message records with the given filter and dispatch them with this device's com channel
	 *
	 * @param metadata Important information about the VSA logfile (including first record location)
	 * @param extractionSettings Settings for this extraction of VSA data
	 * @param filter Struct used to determine which bytes to read and to filter out undesired VSA records
	 *
	 * @return True if there were no failures reading from disk, parsing VSA records, or dispatching VSA records
	 */
	bool parseVSA(
		VSAMetadata& metadata, const VSAExtractionSettings& extractionSettings = VSAExtractionSettings(),
		const VSAMessageReadFilter& filter = VSAMessageReadFilter());

	/**
	 * Wrapper function for Device::readLogicalDisk(pos, into, amount, ...) that handles the VSA record ring buffer.
	 * Handles pos that is before the VSA::RecordStartOffset or is larger than the diskSize.
	 * Sets amount to maximum size of ring buffer if given amount is too large.
	 *
	 * @param pos Position to start read from in relation to VSA file start
	 * @param into The buffer to read bytes into from the disk
	 * @param amount The number of bytes to read into the buffer
	 * @param metadata Optional metadata param (used to determine disk size and if disk is overlapped)
	 *
	 * @return Returns value of return from readLogicalDisk with the given inputs
	 */
	std::optional<uint64_t> vsaReadLogicalDisk(
		uint64_t pos, uint8_t* into, uint64_t amount, std::optional<VSAMetadata> metadata = std::nullopt
	);

	virtual bool isOnlineSupported() const { return true; }

	virtual bool supportsComponentVersions() const { 
		return !getComponentVersions().empty();
	}

	virtual bool supportsTC10() const { return false; }
	
	virtual bool supportsGPTP() const { return false; }

	bool requestTC10Wake(Network::NetID network);

	bool requestTC10Sleep(Network::NetID network);

	std::optional<TC10StatusMessage> getTC10Status(Network::NetID network);
	std::optional<GPTPStatus> getGPTPStatus(std::chrono::milliseconds timeout = std::chrono::milliseconds(100));

	/* MACsec support */
	virtual bool writeMACsecConfig(const MACsecMessage& message, uint16_t binaryIndex);

	std::shared_ptr<DeviceExtension> getExtension(const std::string& name) const;

protected:
	bool online = false;
	int messagePollingCallbackID = 0;
	int internalHandlerCallbackID = 0;
	device_eventhandler_t report;

	std::mutex ioMutex;
	std::optional<bool> ethActivationStatus;
	std::optional<bool> usbHostPowerStatus;
	std::optional<bool> backupPowerEnabled;
	std::optional<bool> backupPowerGood;
	std::array<std::optional<bool>, 6> miscDigital;
	std::array<std::optional<double>, 2> miscAnalog;

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
			[this]() -> std::unique_ptr<Packetizer> {
				auto packetizer = makePacketizer();
				setupPacketizer(*packetizer);
				return packetizer;
			},
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
	std::shared_ptr<IDeviceSettings> makeSettings(std::shared_ptr<Communication> comm) {
		return std::make_shared<Settings>(comm);
	}
	virtual void setupSettings(IDeviceSettings&) {}

	virtual void setupSupportedRXNetworks(std::vector<Network>&) {}
	virtual void setupSupportedTXNetworks(std::vector<Network>&) {}

	virtual void setupExtensions() {}

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
	// Hook for devices such as FIRE which need to inject traffic before RequestSerialNumber
	// Return false to bail
	virtual bool afterCommunicationOpen() { return true; }

	virtual bool requiresVehiclePower() const { return true; }


	void handleInternalMessage(std::shared_ptr<Message> message);

	virtual void handleDeviceStatus(const std::shared_ptr<RawMessage>&) {}

	neodevice_t& getWritableNeoDevice() { return data; }

	enum class LEDState : uint8_t {
		Offline = 0x04,
		CoreMiniRunning = 0x08, // This should override "offline" if the CoreMini is running
		Online = 0x10
	};
	LEDState ledState;
	void updateLEDState();
private:
	neodevice_t data;
	std::shared_ptr<ResetStatusMessage> latestResetStatus;
	std::vector<std::optional<DeviceAppVersion>> versions;
	std::vector<ComponentVersion> componentVersions;

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
	
	size_t pollingMessageLimit = 20000;
	moodycamel::BlockingConcurrentQueue<std::shared_ptr<Message>> pollingContainer;
	void enforcePollingMessageLimit();

	std::atomic<bool> stopHeartbeatThread{false};
	std::mutex heartbeatMutex;
	std::thread heartbeatThread;

	std::mutex diskMutex;

	// Wireless neoVI Stack
	std::atomic<bool> stopWiVIThread{false};
	std::condition_variable stopWiVIcv;
	mutable std::mutex wiviMutex;
	std::thread wiviThread;
	std::atomic<bool> wiviSleepRequested{false};
	std::vector<NewCaptureCallback> newCaptureCallbacks;
	std::vector< std::pair<SleepRequestedCallback, bool /* notified */> > sleepRequestedCallbacks;
	std::vector<std::pair<VINAvailableCallback, bool /* notified */>> vinAvailableCallbacks;
	void wiviThreadBody();
	void stopWiVIThreadIfNecessary(std::unique_lock<std::mutex> lk);

	//Script status
	std::atomic<bool> stopScriptStatusThread{false};
	std::condition_variable stopScriptStatusCv;
	mutable std::mutex scriptStatusMutex;
	std::thread scriptStatusThread;
	std::unordered_map<ScriptStatus, std::vector<ScriptStatusCallback>> scriptStatusCallbacks;
	std::unordered_map<ScriptStatus, uint64_t> scriptStatusValues;
	Lifetime addScriptStatusCallback(ScriptStatus, ScriptStatusCallback);
	bool updateScriptStatusValue(ScriptStatus, uint64_t newValue);
	void notifyScriptStatusCallback(ScriptStatus, uint64_t);
	void scriptStatusThreadBody();
	void stopScriptStatusThreadIfNecessary(std::unique_lock<std::mutex> lk);

	// VSA Read functions

	/**
	 * Read the timestamp from disk of the VSA record stored at pos. If the timestamp is unparsable, attempt to read from
	 * previous records up to minPos
	 *
	 * @param parser The parser that is used to create a VSA record from the given buffer
	 * @param buffer Vector of bytes that stores a sector from the disk
	 * @param pos The location that the buffer was read from
	 * @param minPos The leftmost (minimum) offset from the beginning of the VSA log file to attempt to read from
	 * @param optMetadata Optional param to include metadata about the VSA log file on disk
	 *
	 * @return The timestamp of the first valid record found at or before the given position
	 */
	std::optional<uint64_t> getVSATimestampOrBefore(VSAParser& parser, std::vector<uint8_t>& buffer, uint64_t pos, uint64_t minPos,
		std::optional<VSAMetadata> optMetadata = std::nullopt);

	/**
	 * Read the timestamp from disk of the VSA record stored at pos. If the timestamp is unparsable, attempt to read from
	 * previous records up to maxPos
	 *
	 * @param parser The parser that is used to create a VSA record from the given buffer
	 * @param buffer Vector of bytes that stores a sector that was previously read from the disk
	 * @param pos The location that data in the buffer was read from
	 * @param maxPos The rightmost (maximum) offset from the beginning of the VSA log file to attempt to read from
	 * @param optMetadata Optional param to include metadata about the VSA log file on disk
	 *
	 * @return The timestamp of the first valid record found at or after the given position
	 */
	std::optional<uint64_t> getVSATimestampOrAfter(VSAParser& parser, std::vector<uint8_t>& buffer, uint64_t pos, uint64_t maxPos,
		std::optional<VSAMetadata> optMetadata = std::nullopt);

	/**
	 * Iterate over VSA records and dispatch the messages contained within them. For extended message records, we concatenate the payloads of all of
	 * the records together before dispatching. Dispatch is performed by Communication::dispatchMessage(...).
	 *
	 * @param parser The parser that holds the VSAMessage records to be dispatched
	 *
	 * @return True if dispatching of records is successful without unhandled issues from record parse, else false
	 */
	bool dispatchVSAMessages(VSAParser& parser);

	/**
	 * Determine if the ring buffer for VSA records has filled entirely and looped to the beginning.
	 *
	 * @param optMetadata Optional metadata param with partial information about current state of VSA log file
	 * (Must contain valid disk size and CoreMini timestamp)
	 *
	 * @return True if the buffer has looped; Returns std::nullopt if unable to determine
	 */
	std::optional<bool> isVSAOverlapped(std::optional<VSAMetadata> optMetadata = std::nullopt);

	/**
	 * Find the first extended message record in the sequence of the given extended message record by backtracking in the disk.
	 * Results are returned by reference (not through the return value)
	 *
	 * @param record The extended message record whose sequence for which to find the first record
	 * @param pos The position of the given record in the VSA log file (in bytes)
	 * @param parser Used to parse indices and sequence numbers from the extended message records
	 * @param metadata Optional param to include metadata about the VSA log file on disk
	 *
	 * @return True if the first extended message record in the sequence was successfully found
	 */
	bool findFirstExtendedVSAFromConsecutive(std::shared_ptr<VSAExtendedMessage>& record, uint64_t& pos, 
		VSAParser& parser, std::optional<VSAMetadata> metadata = std::nullopt);

	/**
	 * Find the first record before the given position that contains a valid timestamp. Results are returned by reference.
	 * 
	 * @param record The record from which to backtrack in the VSA buffer
	 * @param pos The position of the given record in the VSA log file (in bytes)
	 * @param parser Used to parse records from the VSA buffer
	 * 
	 * @return True if a record with valid timestamp is found within a set number of reads
	*/
	bool findPreviousRecordWithTimestamp(std::shared_ptr<VSA>& record, uint64_t& pos, VSAParser& parser);

	/**
	 * Get the creation timestamp of the CoreMini script from VSA log file
	 *
	 * @return Timestamp in 25 nanosecond ticks since January 1, 2007
	 */
	std::optional<uint64_t> getCoreMiniScriptTimestamp();

	/**
	 * Get the size of the VSA file storage system from the CoreMini script
	 * 
	 * @return The size of the vsa log files on the disk
	 */
	std::optional<uint64_t> getVSADiskSize();

	bool enableNetworkCommunication(bool enable, uint32_t timeout = 0);

	// Keeponline (keepalive for online)
	std::unique_ptr<Periodic> keeponline;
};

}

#endif // __cplusplus

#endif