#ifndef __FLEXRAYCONTROLLER_H_
#define __FLEXRAYCONTROLLER_H_

#include <cstdint>
#include <memory>
#include <chrono>
#include <mutex>
#include "icsneo/communication/message/flexray/control/flexraycontrolmessage.h"
#include "icsneo/device/extensions/flexray/erayregister.h"
#include "icsneo/device/extensions/flexray/poccommand.h"

namespace icsneo {

class Device;

namespace FlexRay {

class Controller {
public:
	Controller(Device& device, uint8_t index) : device(device), index(index) {}
	std::shared_ptr<FlexRayControlMessage> getStatus() const;
	void _setStatus(std::shared_ptr<FlexRayControlMessage> msg);

	bool getAllowColdstart() const { return allowColdstart; }
	void setAllowColdstart(bool enable) { allowColdstart = enable; }

	bool getWakeupBeforeStart() const { return wakeupBeforeStart; }
	void setWakeupBeforeStart(bool enable) { wakeupBeforeStart = enable; }

	void getReady();
	void start();

private:
	bool isPOCBusy() const { return readRegisterOr(ERAYRegister::SUCC1, 0x00000080) & 0x00000080; }
	std::pair<bool, FlexRay::POCCommand> getCurrentPOCCommand(std::chrono::milliseconds timeout = std::chrono::milliseconds(50)) const;
	bool setCurrentPOCCommand(
		FlexRay::POCCommand cmd,
		bool checkForSuccess = true,
		std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
	bool wasCommandSuccessful(std::chrono::milliseconds timeout = std::chrono::milliseconds(50)) const;

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
	mutable std::mutex statusLock;
	mutable std::mutex readRegisterLock;
	std::shared_ptr<FlexRayControlMessage> status;
	bool allowColdstart = false;
	bool wakeupBeforeStart = false;
};

} // namespace FlexRay

} // namespace icsneo

#endif // __FLEXRAYCONTROLLER_H_