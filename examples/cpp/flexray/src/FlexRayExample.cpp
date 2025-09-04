#include <iostream>
#include <random>

#include "icsneo/icsneocpp.h"
#include "icsneo/device/extensions/flexray/cluster.h"

/* Modifiable Configuration Variables*/

static size_t NumMessages = 300;
static size_t FramePayloadSize = 134;
static uint16_t SlotOne = 1;
static uint16_t SlotTwo = 3;
static bool SuppressRxTxDebugMessages = true;

static const icsneo::FlexRay::Controller::Configuration DefaultControllerConfig {
    160, // uint16_t AcceptStartupRangeMicroticks INIT(0); // pdAcceptedStartupRange
    true, // bool AllowHaltDueToClock INIT(false); // pAllowHaltDueToClock

    15, // uint8_t AllowPassiveToActiveCyclePairs INIT(0); // pAllowPassiveToActive

    2, // uint8_t ClusterDriftDamping INIT(0); // pClusterDriftDamping

    true, // bool ChannelA INIT(false); // pChannels
    true, // bool ChannelB INIT(false); // pChannels

    56, // uint8_t DecodingCorrectionMicroticks INIT(0); // pDecodingCorrection
    28, // uint8_t DelayCompensationAMicroticks INIT(0); // pDelayCompensation[A]
    28, // uint8_t DelayCompensationBMicroticks INIT(0); // pDelayCompensation[B]
    0, // uint8_t ExternOffsetCorrectionControl INIT(0); // pExternOffsetControl
    0, // uint8_t ExternRateCorrectionControl INIT(0); // pExternRateControl
    0, // uint8_t ExternOffsetCorrectionMicroticks INIT(0); // pExternOffsetCorrection
    0, // uint8_t ExternRateCorrectionMicroticks INIT(0); // pExternRateCorrection

    0, // uint16_t KeySlotID INIT(0);

    false, // bool KeySlotOnlyEnabled INIT(false); // pSingleSlotEnabled (TSM)
    true, // bool KeySlotUsedForStartup INIT(false); // pKeySlotUsedForStartup (TXST)
    true, // bool KeySlotUsedForSync INIT(false); // pKeySlotUsedForSync (TXSY)

    226, // uint16_t LatestTxMinislot INIT(0); // pLatestTx
    401202, // uint32_t ListenTimeout INIT(0); // pdListenTimeout

    7, // uint8_t MacroInitialOffsetA INIT(0); // pMacroInitialOffset[A],  Valid 2..72
    7, // uint8_t MacroInitialOffsetB INIT(0); // pMacroInitialOffset[B], Valid 2..72

    36, // uint8_t MicroInitialOffsetA INIT(0); // pMicroInitialOffset[A],  Valid 0..240
    36, // uint8_t MicroInitialOffsetB INIT(0); // pMicroInitialOffset[B], Valid 0..240

    200000, // uint32_t MicroPerCycle INIT(0); // pMicroPerCycle

    false, // bool MTSOnA INIT(false);
    false, // bool MTSOnB INIT(false);

    189, // uint16_t OffsetCorrectionOutMicroticks INIT(0); // pOffsetCorrectionOut
    601, // uint16_t RateCorrectionOutMicroticks INIT(0); // pdMaxDrift and pRateCorrectionOut

    0, // uint16_t SecondKeySlotID INIT(0);
    false, // bool TwoKeySlotMode INIT(false);

    55, // uint8_t WakeupPattern INIT(0);
    false, // bool WakeupOnChannelB INIT(false); // pWakeupChannel
};

namespace Cluster = icsneo::FlexRay::Cluster;

static const Cluster::Configuration DefaultClusterConfig {
    Cluster::SpeedType::FLEXRAY_BAUDRATE_10M, // neoflexray_speed_t Speed INIT(FLEXRAY_BAUDRATE_10M /* = 0 */); // gdSampleClockPeriod, pSamplesPerMicrotick, Baud Rate Prescaler
	Cluster::SPPType::FLEXRAY_SPP_5, // neoflexray_spp_t StrobePointPosition INIT(FLEXRAY_SPP_5 /* = 0 */);

	4, // uint8_t ActionPointOffset INIT(0); // gdActionPointOffset
	64, // uint8_t CASRxLowMax INIT(0); // gdCASRxLowMax 
	8, // uint8_t ColdStartAttempts INIT(0); // gColdStartAttempts
	5000, // uint16_t CycleDurationMicroSec INIT(0); // gdCycle
	1, // uint8_t DynamicSlotIdlePhaseMinislots INIT(0); // gdDynamicSlotIdlePhase, In Minislot increments [0..2]
	4, // uint8_t ListenNoiseMacroticks INIT(0); // gListenNoise
	5000, // uint16_t MacroticksPerCycle INIT(0); // gMacroPerCycle
	1, // uint8_t MacrotickDurationMicroSec INIT(0); // gdMacroTick
	2, // uint8_t MaxWithoutClockCorrectionFatal INIT(0); // gMaxWithoutClockCorrectionFatal
	2, // uint8_t MaxWithoutClockCorrectionPassive INIT(0); // gMaxWithoutClockCorrectionPassive
	4, // uint8_t MinislotActionPointOffsetMacroticks INIT(0); // gdMinislotActionPointOffset
	10, // uint32_t MinislotDurationMacroticks INIT(0); // gdMinislot
    40, // uint32_t NetworkIdleTimeMacroticks INIT(0); // gdNIT
	1, // uint8_t NetworkManagementVectorLengthBytes INIT(0); // gNetworkManagementVectorLength
	0, // uint32_t NumberOfMinislots INIT(0); // gNumberOfMinislots
	32, // uint16_t NumberOfStaticSlots INIT(0); // gdNumberOfStaticSlots
	4991, // uint32_t OffsetCorrectionStartMacroticks INIT(0); // gOffsetCorrectionStart
	67, // uint8_t PayloadLengthOfStaticSlotInWords INIT(0); // gPayloadLengthStatic

	155, // uint16_t StaticSlotMacroticks INIT(0); // gdStaticSlot
	0, // uint32_t SymbolWindowMacroticks INIT(0); // gdSymbolWindow
	0, // uint32_t SymbolWindowActionPointOffsetMacroticks INIT(0);
	15, // uint16_t SyncFrameIDCountMax INIT(0); // gSyncNodeMax

	11, // uint8_t TransmissionStartSequenceDurationBits INIT(0); // gdTSSTransmitter
	40, // uint8_t WakeupRxIdleBits INIT(0); // gdWakeupSymbolRxIdle
	40, // uint8_t WakeupRxLowBits INIT(0); // gdWakeupSymbolRxLow
	301, // uint16_t WakeupRxWindowBits INIT(0); // gdWakeupSymbolRxWindow
	60, // uint8_t WakeupTxActiveBits INIT(0); // gdWakeupSymbolTxLow (active = low here)
	180, // uint8_t WakeupTxIdleBits INIT(0); // gdWakeupSymbolTxIdle
};

static std::condition_variable cv;
static std::mutex msgRcvdMutex;

void configureFlexRayDevice(std::shared_ptr<icsneo::Device> device) {
    auto controllers = device->getFlexRayControllers();
    for (uint16_t i = 0; i < controllers.size(); i++) {
        auto controller = controllers[i];
        icsneo::FlexRay::Cluster::Configuration clusterConfig = DefaultClusterConfig;
        icsneo::FlexRay::Controller::Configuration controllerConfig = DefaultControllerConfig;
        controller->setAllowColdstart(true);
        controllerConfig.KeySlotID = (i == 0) ? SlotOne : SlotTwo;
        controller->setConfiguration(clusterConfig, controllerConfig);
        controller->setStartWhenGoingOnline(true);
    }
}

std::vector<std::shared_ptr<icsneo::FlexRayMessage>> makeDummyFlexRayMessages(size_t msgCount, size_t msgLen) {
    std::vector<std::shared_ptr<icsneo::FlexRayMessage>> messages;
    messages.reserve(msgCount);
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<unsigned short> dist(0, 255);
    for (size_t i = 0; i < msgCount; i++) {
        auto msg = std::make_shared<icsneo::FlexRayMessage>();
        msg->network = icsneo::Network::NetID::FLEXRAY_01;
        msg->cycleRepetition = 1;
        msg->channel = icsneo::FlexRay::Channel::AB;
        msg->data.reserve(msgLen);
        for (size_t j = 0; j < msgLen; j++) {
            msg->data.push_back(static_cast<uint8_t>(dist(engine)));
        }
        msg->slotid = (i % 2 == 0) ? SlotOne : SlotTwo;
        messages.push_back(msg);
    }
    return messages;
}

int main() {
    auto devices = icsneo::FindAllDevices();
    std::shared_ptr<icsneo::Device> flexrayDevice = nullptr;
    for (auto&& device : devices) {
        if (device->getExtension("FlexRay")) {
            flexrayDevice = device;
        }
    }

    if (flexrayDevice) {
        std::cout << "Found device " << flexrayDevice->getProductName() << " " << flexrayDevice->getSerial() << " which supports FlexRay." << std::endl;
        auto&& controllers = flexrayDevice->getFlexRayControllers();
        if (controllers.size() > 0) {
            std::vector<std::shared_ptr<icsneo::FlexRayMessage>> messages;
            size_t currentMessage = 0;
            auto filter = std::make_shared<icsneo::MessageFilter>(icsneo::Network::NetID::FLEXRAY_01);
            auto rxCallback = [&messages, &currentMessage](std::shared_ptr<icsneo::Message> message) -> void {
                std::unique_lock lk(msgRcvdMutex);
                auto frmsg = std::static_pointer_cast<icsneo::FlexRayMessage>(message);
                if (currentMessage < messages.size() && frmsg->data.size() > 1 && frmsg->data == messages[currentMessage]->data) {
                    if (!SuppressRxTxDebugMessages) {
                        std::cout << "Message Number " << currentMessage + 1 << " received." << std::endl;
                    }
                    currentMessage++;
                    lk.unlock();
                    cv.notify_all();
                }
            };

            auto cb = std::make_shared<icsneo::MessageCallback>(rxCallback, filter);
            auto cbId = flexrayDevice->addMessageCallback(cb);
            messages = makeDummyFlexRayMessages(NumMessages, FramePayloadSize);
            configureFlexRayDevice(flexrayDevice);
            if (!flexrayDevice->open()) {
                std::cerr << "Device open failed." << std::endl;
                return -1;
            }
            if (!flexrayDevice->goOnline()) {
                std::cerr << "Device go online failed." << std::endl;
                return -1;
            }
            std::cout << "Transmitting " << NumMessages << " FlexRay frames." << std::endl;
            for (auto msg : messages) {
                std::unique_lock lk(msgRcvdMutex);
                if (!flexrayDevice->transmit(msg)) {
                    std::cerr << "Failed to transmit message." << std::endl;
                    return -1;
                } else if (!SuppressRxTxDebugMessages) {
                    std::cout << "Message Number " << currentMessage + 1 << " transmitted." << std::endl;
                }

                cv.wait(lk);
                lk.unlock();
            }
            std::cout << "All transmitted frames received." << std::endl;
            flexrayDevice->removeMessageCallback(cbId);
        }
    } else {
        std::cerr << "Unable to find a device which supports FlexRay." << std::endl;
        return -1;
    }

    std::cout << "Finished." << std::endl;

    return 0;
}