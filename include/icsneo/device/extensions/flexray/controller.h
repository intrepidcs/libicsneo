#ifndef __FLEXRAYCONTROLLER_H_
#define __FLEXRAYCONTROLLER_H_

#ifdef __cplusplus

#include <cstdint>
#include <memory>
#include <chrono>
#include <mutex>
#include "icsneo/communication/message/flexray/control/flexraycontrolmessage.h"
#include "icsneo/communication/message/flexray/flexraymessage.h"
#include "icsneo/device/extensions/flexray/messagebuffer.h"
#include "icsneo/device/extensions/flexray/erayregister.h"
#include "icsneo/device/extensions/flexray/poccommand.h"
#include "icsneo/device/extensions/flexray/pocstatus.h"
#include "icsneo/device/extensions/flexray/cluster.h"

#define INIT(x) = x

namespace icsneo {

class Device;

namespace FlexRay {

class Controller {
public:
	Controller(Device& device, uint8_t index, Network net) : device(device), index(index), network(net) {}
	void _setStatus(std::shared_ptr<FlexRayControlMessage> msg);

	// Begin Public Interface
	std::shared_ptr<FlexRayControlMessage> getStatus() const;

	const Network& getNetwork() const { return network; }

	struct Configuration; // Forward declaration
	std::pair<const Cluster::Configuration&, const Controller::Configuration&> getConfiguration() const;
	void setConfiguration(Cluster::Configuration clConfig, Controller::Configuration coConfig);

	bool getStartWhenGoingOnline() const { return startWhenGoingOnline; }
	void setStartWhenGoingOnline(bool enable) { startWhenGoingOnline = enable; }

	bool getAllowColdstart() const { return allowColdstart; }
	void setAllowColdstart(bool enable) { allowColdstart = enable; }

	bool getWakeupBeforeStart() const { return wakeupBeforeStart; }
	void setWakeupBeforeStart(bool enable) { wakeupBeforeStart = enable; }

	void addMessageBuffer(MessageBuffer buffer);
	void clearMessageBuffers();

	bool wakeup(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	bool configure(std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));
	bool getReady(std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));
	bool start(std::chrono::milliseconds timeout = std::chrono::milliseconds(2000));

	bool transmit(const std::shared_ptr<FlexRayMessage>& frmsg);

	bool halt(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	bool freeze(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	bool triggerMTS(std::chrono::milliseconds timeout = std::chrono::milliseconds(200));
	// End Public Interface

private:
	bool isPOCBusy() const { return readRegisterOr(ERAYRegister::SUCC1, 0x00000080) & 0x00000080; }
	bool isClearAllRAMBusy() const { return readRegisterOr(ERAYRegister::MHDS, 0x00000080) & 0x00000080; }
	bool isInputBufferHostBusy() const { return readRegisterOr(ERAYRegister::IBCR, 0x00008000) & 0x00008000; }
	std::pair<bool, FlexRay::POCCommand> getCurrentPOCCommand(std::chrono::milliseconds timeout = std::chrono::milliseconds(50)) const;
	bool setCurrentPOCCommand(
		FlexRay::POCCommand cmd,
		bool checkForSuccess = true,
		std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	bool wasCommandSuccessful(std::chrono::milliseconds timeout = std::chrono::milliseconds(50)) const;

	std::pair<bool, FlexRay::POCStatus> getCurrentPOCStatus(std::chrono::milliseconds timeout = std::chrono::milliseconds(50)) const;

	bool lockConfiguration(std::chrono::milliseconds timeout = std::chrono::milliseconds(150));
	bool enterConfig(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));

	static uint16_t CalculateHCRC(const MessageBuffer& buf);
	static uint16_t CalculateCycleFilter(uint8_t baseCycle, uint8_t cycleRepetition);

	std::pair<bool, uint32_t> readRegister(
		ERAYRegister reg,
		std::chrono::milliseconds timeout = std::chrono::milliseconds(50)) const;
	uint32_t readRegisterOr(
		ERAYRegister reg,
		uint32_t defaultValue = 0,
		std::chrono::milliseconds timeout = std::chrono::milliseconds(50)) const;
	bool writeRegister(
		ERAYRegister reg,
		uint32_t value,
		uint32_t mask,
		bool waitForPOCReady = true,
		std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	bool writeRegister(
		ERAYRegister reg,
		uint32_t value,
		bool waitForPOCReady = true,
		std::chrono::milliseconds timeout = std::chrono::milliseconds(50));

	Device& device;
	uint8_t index;
	Network network;
	mutable std::mutex statusLock;
	mutable std::mutex readRegisterLock;
	std::shared_ptr<FlexRayControlMessage> status;
	bool startWhenGoingOnline = false;
	bool allowColdstart = false;
	bool wakeupBeforeStart = false;
	bool lastSeenRunning = false;

public:
#define neoflexray_controller_config_t icsneo::FlexRay::Controller::Configuration
	struct Configuration {

#else // __cplusplus

#include <stdbool.h>
#include <stdint.h>

#define INIT(x)

	typedef struct {
#endif // __cplusplus

		uint16_t AcceptStartupRangeMicroticks INIT(0);
		bool AllowHaltDueToClock INIT(false);

		uint8_t AllowPassiveToActiveCyclePairs INIT(0);

		uint8_t ClusterDriftDamping INIT(0);

		bool ChannelA INIT(false);
		bool ChannelB INIT(false);

		uint8_t DecodingCorrectionMicroticks INIT(0);
		uint8_t DelayCompensationAMicroticks INIT(0);
		uint8_t DelayCompensationBMicroticks INIT(0);
		uint8_t ExternOffsetCorrectionControl INIT(0);
		uint8_t ExternRateCorrectionControl INIT(0);
		uint8_t ExternOffsetCorrectionMicroticks INIT(0);
		uint8_t ExternRateCorrectionMicroticks INIT(0);

		bool ExternalSync INIT(false);
		bool UseExternalSync INIT(false);

		bool FallBackInternal INIT(false);
		bool UseFallBackInternal INIT(false);

		uint16_t KeySlotID INIT(0);

		bool KeySlotOnlyEnabled INIT(false); // pSingleSlotEnabled (TSM)
		bool KeySlotUsedForStartup INIT(false); // pKeySlotUsedForStartup (TXST)
		bool KeySlotUsedForSync INIT(false); // pKeySlotUsedForSync (TXSY)

		uint16_t LatestTxMinislot INIT(0);
		uint32_t ListenTimeout INIT(0);

		uint8_t MacroInitialOffsetA INIT(0); // Valid 2..72
		uint8_t MacroInitialOffsetB INIT(0); // Valid 2..72

		uint32_t MaximumDynamicPayloadLengthWords INIT(0);

		uint8_t MicroInitialOffsetA INIT(0); // Valid 0..240
		uint8_t MicroInitialOffsetB INIT(0); // Valid 0..240

		uint32_t MicroPerCycle INIT(0);

		double MicrotickDurationSec INIT(0);
		bool UseMicrotickDurationSec INIT(false);

		bool MTSOnA INIT(false);
		bool MTSOnB INIT(false);

		bool NMVectorEarlyUpdate INIT(false);
		bool UseNMVectorEarlyUpdate INIT(false);

		uint16_t OffsetCorrectionOutMicroticks INIT(0);
		uint16_t RateCorrectionOutMicroticks INIT(0); // pdMaxDrift and pRateCorrectionOut

		uint32_t SamplesPerMicrotick INIT(0);
		bool UseSamplesPerMicrotick INIT(false);

		uint16_t SecondKeySlotID INIT(0);
		bool TwoKeySlotMode INIT(false);

		uint8_t WakeupPattern INIT(0);
		bool WakeupOnChannelB INIT(false);

#ifndef __cplusplus
	} neoflexray_controller_config_t;
#else
	};

private:
	bool configDirty = false;
	Cluster::Configuration clusterConfig;
	Controller::Configuration controllerConfig;
	std::vector<std::shared_ptr<MessageBuffer>> messageBuffers;
};

} // namespace FlexRay

} // namespace icsneo
#endif // __cplusplus

#endif // __FLEXRAYCONTROLLER_H_