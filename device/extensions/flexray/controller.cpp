#include "icsneo/device/extensions/flexray/controller.h"
#include "icsneo/device/device.h"

using namespace icsneo;

std::shared_ptr<FlexRayControlMessage> FlexRay::Controller::getStatus() const {
	std::lock_guard<std::mutex> lk(statusLock);
	return status;
}

void FlexRay::Controller::_setStatus(std::shared_ptr<FlexRayControlMessage> msg) {
	std::lock_guard<std::mutex> lk(statusLock);
	status = msg;
}

void FlexRay::Controller::getReady() {
	
}

void FlexRay::Controller::start() {
	if(true) // TODO something
		getReady();
	if(wakeupBeforeStart)
		setCurrentPOCCommand(FlexRay::POCCommand::Wakeup);
	if(allowColdstart)
		setCurrentPOCCommand(FlexRay::POCCommand::AllowColdstart);
	setCurrentPOCCommand(FlexRay::POCCommand::Run);
}

std::pair<bool, FlexRay::POCCommand> FlexRay::Controller::getCurrentPOCCommand(std::chrono::milliseconds timeout) const {
	const auto val = readRegister(ERAYRegister::SUCC1, timeout);
	return {val.first, FlexRay::POCCommand(val.second & 0x0000000F)};
}

bool FlexRay::Controller::setCurrentPOCCommand(FlexRay::POCCommand cmd, bool checkForSuccess, std::chrono::milliseconds timeout) {
	const auto beforeWrite = std::chrono::steady_clock::now();

	if(!writeRegister(ERAYRegister::SUCC1, uint32_t(cmd), true, timeout))
		return false;
	if(!checkForSuccess)
		return true;
	
	const auto writeDuration = std::chrono::steady_clock::now() - beforeWrite;
	timeout = std::chrono::duration_cast<std::chrono::milliseconds>(writeDuration - timeout);
	if(timeout.count() <= 0)
		return false; // Out of time!

	return wasCommandSuccessful(timeout);
}

bool FlexRay::Controller::wasCommandSuccessful(std::chrono::milliseconds timeout) const {
	const auto val = getCurrentPOCCommand(timeout);
	return val.first && val.second != FlexRay::POCCommand::CommandNotAccepted;
}

std::pair<bool, uint32_t> FlexRay::Controller::readRegister(ERAYRegister reg, std::chrono::milliseconds timeout) const {
	std::lock_guard<std::mutex> lk(readRegisterLock);
	device.com->sendCommand(Command::FlexRayControl, FlexRayControlMessage::BuildReadCCRegsArgs(index, uint16_t(reg)));
	std::shared_ptr<FlexRayControlMessage> resp;
	const auto start = std::chrono::steady_clock::now();
	while(!resp && (std::chrono::steady_clock::now() - start) < timeout) {
		auto msg = device.com->waitForMessageSync(MessageFilter(icsneo::Network::NetID::FlexRayControl), timeout);
		if(auto frmsg = std::dynamic_pointer_cast<FlexRayControlMessage>(msg)) {
			if(frmsg->decoded && frmsg->controller == index && frmsg->opcode == FlexRay::Opcode::ReadCCRegs)
				resp = frmsg;
		}
	}
	if(resp)
		return {true, resp->registers[0]};
	else
		return {false, 0};
}

uint32_t FlexRay::Controller::readRegisterOr(ERAYRegister reg, uint32_t defaultValue, std::chrono::milliseconds timeout) const {
	auto ret = readRegister(reg, timeout);
	return ret.first ? ret.second : defaultValue;
}

bool FlexRay::Controller::writeRegister(
	ERAYRegister reg,
	uint32_t value,
	uint32_t mask,
	bool waitForPOCReady,
	std::chrono::milliseconds timeout) {

	if(mask != 0xffffffff) {
		const auto beforeRead = std::chrono::steady_clock::now();
		auto pair = readRegister(reg, timeout);
		if(!pair.first)
			return false; // Couldn't read, so we don't want to try to write anything
		auto readDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beforeRead);
		timeout = readDuration - timeout;
		if(timeout.count() <= 0)
			return false; // Out of time!
		pair.second &= ~mask;
		pair.second |= value & mask;
		value = pair.second;
	}
	return writeRegister(reg, value, waitForPOCReady, timeout);
}

bool FlexRay::Controller::writeRegister(
	ERAYRegister reg,
	uint32_t value,
	bool waitForPOCReady,
	std::chrono::milliseconds timeout) {

	if(waitForPOCReady) {
		const auto start = std::chrono::steady_clock::now();
		bool pocBusy = isPOCBusy();
		while(pocBusy && (std::chrono::steady_clock::now() - start) < timeout) {
			pocBusy = isPOCBusy();
		}
		if(pocBusy) // timeout
			return false;
	}

	device.com->sendCommand(Command::FlexRayControl, FlexRayControlMessage::BuildWriteCCRegArgs(index, uint16_t(reg), value));
	return true; // Does the device send anything back to tell us this actually happened?
}