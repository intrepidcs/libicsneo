#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <vector>
#include <memory>
#include <utility>
#include <cstring>
#include "icsneo/api/eventmanager.h"
#include "icsneo/device/neodevice.h"
#include "icsneo/device/idevicesettings.h"
#include "icsneo/device/nullsettings.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/device/extensions/deviceextension.h"
#include "icsneo/communication/communication.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/encoder.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/communication/message/resetstatusmessage.h"
#include "icsneo/device/extensions/flexray/controller.h"
#include "icsneo/communication/message/flexray/control/flexraycontrolmessage.h"
#include "icsneo/third-party/concurrentqueue/concurrentqueue.h"

namespace icsneo {

class Device {
public:
	virtual ~Device() {
		if(isMessagePollingEnabled())
			disableMessagePolling();
		close();
	}

	static std::string SerialNumToString(uint32_t serial);
	static uint32_t SerialStringToNum(const std::string& serial);
	static bool SerialStringIsNumeric(const std::string& serial);

	uint16_t getTimestampResolution() const;
	DeviceType getType() const { return DeviceType(data.type); }
	uint16_t getProductId() const { return productId; }
	std::string getSerial() const { return data.serial; }
	uint32_t getSerialNumber() const { return Device::SerialStringToNum(getSerial()); }
	const neodevice_t& getNeoDevice() const { return data; }
	std::string describe() const;
	friend std::ostream& operator<<(std::ostream& os, const Device& device) {
		os << device.describe();
		return os;
	}

	virtual bool open();
	virtual bool close();
	virtual bool isOnline() const { return online; }
	virtual bool isOpen() const { return com->isOpen(); }
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

	virtual std::shared_ptr<FlexRay::Controller> getFlexRayControllerByNetwork(const Network&) const { return nullptr; }

	const device_eventhandler_t& getEventHandler() const { return report; }

	std::shared_ptr<Communication> com;
	std::unique_ptr<IDeviceSettings> settings;

protected:
	uint16_t productId = 0;
	bool online = false;
	int messagePollingCallbackID = 0;
	int internalHandlerCallbackID = 0;
	device_eventhandler_t report;

	// START Initialization Functions
	Device(neodevice_t neodevice = { 0 }) {
		data = neodevice;
		data.device = this;
	}
	
	template<typename Transport, typename Settings = NullSettings>
	void initialize() {
		report = makeEventHandler();
		auto transport = makeTransport<Transport>();
		setupTransport(*transport);
		auto packetizer = makePacketizer();
		setupPacketizer(*packetizer);
		auto encoder = makeEncoder(packetizer);
		setupEncoder(*encoder);
		auto decoder = makeDecoder();
		setupDecoder(*decoder);
		com = makeCommunication(std::move(transport), packetizer, std::move(encoder), std::move(decoder));
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

	template<typename Transport>
	std::unique_ptr<ICommunication> makeTransport() { return std::unique_ptr<ICommunication>(new Transport(report, getWritableNeoDevice())); }
	virtual void setupTransport(ICommunication&) {}

	virtual std::shared_ptr<Packetizer> makePacketizer() { return std::make_shared<Packetizer>(report); }
	virtual void setupPacketizer(Packetizer&) {}

	virtual std::unique_ptr<Encoder> makeEncoder(std::shared_ptr<Packetizer> p) { return std::unique_ptr<Encoder>(new Encoder(report, p)); }
	virtual void setupEncoder(Encoder&) {}

	virtual std::unique_ptr<Decoder> makeDecoder() { return std::unique_ptr<Decoder>(new Decoder(report)); }
	virtual void setupDecoder(Decoder&) {}

	virtual std::shared_ptr<Communication> makeCommunication(
		std::unique_ptr<ICommunication> t,
		std::shared_ptr<Packetizer> p,
		std::unique_ptr<Encoder> e,
		std::unique_ptr<Decoder> d) { return std::make_shared<Communication>(report, std::move(t), p, std::move(e), std::move(d)); }
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

	neodevice_t& getWritableNeoDevice() { return data; }

private:
	neodevice_t data;
	std::shared_ptr<ResetStatusMessage> latestResetStatus;

	mutable std::mutex extensionsLock;
	std::vector<std::shared_ptr<DeviceExtension>> extensions;
	void forEachExtension(std::function<void(const std::shared_ptr<DeviceExtension>&)> fn);

	std::vector<Network> supportedTXNetworks;
	std::vector<Network> supportedRXNetworks;
	
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
};

}

#endif