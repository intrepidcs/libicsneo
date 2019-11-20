#include "icsneo/device/extensions/flexray/controller.h"
#include "icsneo/device/device.h"

using namespace icsneo;

void FlexRay::Controller::_setStatus(std::shared_ptr<FlexRayControlMessage> msg) {
	std::lock_guard<std::mutex> lk(statusLock);
	status = msg;
}

std::shared_ptr<FlexRayControlMessage> FlexRay::Controller::getStatus() const {
	std::lock_guard<std::mutex> lk(statusLock);
	return status;
}

std::pair<const FlexRay::Cluster::Configuration&, const FlexRay::Controller::Configuration&> FlexRay::Controller::getConfiguration() const {
	return { clusterConfig, controllerConfig };
}

void FlexRay::Controller::setConfiguration(Cluster::Configuration clConfig, Controller::Configuration coConfig) {
	configDirty = true;
	clusterConfig = clConfig;
	controllerConfig = coConfig;
}

void FlexRay::Controller::addMessageBuffer(MessageBuffer buffer) {
	configDirty = true;
	messageBuffers.emplace_back(std::make_shared<MessageBuffer>(buffer));
}

void FlexRay::Controller::clearMessageBuffers() {
	configDirty = true;
	messageBuffers.clear();
}

bool FlexRay::Controller::wakeup(std::chrono::milliseconds timeout) {
	return setCurrentPOCCommand(FlexRay::POCCommand::Wakeup, true, timeout);
}

bool FlexRay::Controller::configure(std::chrono::milliseconds timeout) {
	const auto initialTimeout = timeout;
	const auto functionBegin = std::chrono::steady_clock::now();
	const auto updateTimeout = [&initialTimeout, &functionBegin, &timeout]() {
		timeout = std::chrono::duration_cast<std::chrono::milliseconds>(initialTimeout - (std::chrono::steady_clock::now() - functionBegin));
	};

	auto statusPair = getCurrentPOCStatus(timeout);
	const auto& status = statusPair.second;
	if(!statusPair.first)
		return false;
	updateTimeout();

	if(status != POCStatus::Config) {
		if(!enterConfig(timeout))
			return false;
		updateTimeout();
	}

	if(!setCurrentPOCCommand(POCCommand::ClearRAMs, true, timeout))
		return false;
	const auto start = std::chrono::steady_clock::now();
	bool carbusy = isClearAllRAMBusy();
	while(carbusy && (std::chrono::steady_clock::now() - start) < timeout) {
		carbusy = isClearAllRAMBusy();
	}
	if(carbusy) // timeout
		return false;
	updateTimeout();

	std::vector<std::pair<ERAYRegister, uint32_t>> registerWrites;
	registerWrites.reserve(18);

	registerWrites.push_back({ ERAYRegister::SUCC1,
		((controllerConfig.KeySlotUsedForStartup & 0x1) << 8) | // TXST
		((controllerConfig.KeySlotUsedForSync & 0x1) << 9) | // TXSY
		((clusterConfig.ColdStartAttempts & 0x1f) << 11) | // CSA
		((controllerConfig.AllowPassiveToActiveCyclePairs & 0x1f) << 16) | // PTA
		((controllerConfig.WakeupOnChannelB & 0x1) << 21) | // WUCS
		((controllerConfig.KeySlotOnlyEnabled & 0x1) << 22) | // TSM
		((controllerConfig.AllowHaltDueToClock & 0x1) << 23) | // HCSE
		((controllerConfig.MTSOnA & 0x1) << 24) | // MTSA
		((controllerConfig.MTSOnB & 0x1) << 25) | // MTSB
		((controllerConfig.ChannelA & 0x1) << 26) | // CCHA
		((controllerConfig.ChannelB & 0x1) << 27) // CCHB
	});

	registerWrites.push_back({ ERAYRegister::SUCC2,
		((controllerConfig.ListenTimeout & 0x1fffff)) |
		((clusterConfig.ListenNoiseMacroticks - 1) << 24)
	});

	registerWrites.push_back({ ERAYRegister::SUCC3,
		(clusterConfig.MaxWithoutClockCorrectionPassive & 0xF) |
		((clusterConfig.MaxWithoutClockCorrectionFatal) << 4)
	});

	registerWrites.push_back({ ERAYRegister::NEMC,
		clusterConfig.NetworkManagementVectorLengthBytes
	});

	registerWrites.push_back({ ERAYRegister::PRTC1,
		(clusterConfig.TransmissionStartSequenceDurationBits & 0xF) |
		((clusterConfig.CASRxLowMax & 0x3F) << 4) |
		((clusterConfig.StrobePointPosition & 0x3) << 12) |
		((clusterConfig.Speed & 0x3) << 14) |
		((clusterConfig.WakeupRxWindowBits & 0x1ff) << 16) |
		((controllerConfig.WakeupPattern) << 26)
	});

	registerWrites.push_back({ ERAYRegister::PRTC2,
		(clusterConfig.WakeupRxIdleBits) |
		((clusterConfig.WakeupRxLowBits) << 8) |
		((clusterConfig.WakeupTxIdleBits) << 16) |
		((clusterConfig.WakeupTxActiveBits) << 24)
	});

	registerWrites.push_back({ ERAYRegister::MHDC,
		(clusterConfig.PayloadLengthOfStaticSlotInWords) |
		((controllerConfig.LatestTxMinislot) << 16)
	});

	registerWrites.push_back({ ERAYRegister::GTUC1,
		controllerConfig.MicroPerCycle
	});

	registerWrites.push_back({ ERAYRegister::GTUC2,
		(clusterConfig.SyncFrameIDCountMax << 16) |
		clusterConfig.MacroticksPerCycle
	});

	registerWrites.push_back({ ERAYRegister::GTUC3,
		(controllerConfig.MicroInitialOffsetA) |
		((controllerConfig.MicroInitialOffsetB) << 8) |
		((controllerConfig.MacroInitialOffsetA) << 16) |
		((controllerConfig.MacroInitialOffsetB) << 24)
	});

	registerWrites.push_back({ ERAYRegister::GTUC4,
		((clusterConfig.MacroticksPerCycle - clusterConfig.NetworkIdleTimeMacroticks - 1) & 0xFFFF) |
		((clusterConfig.OffsetCorrectionStartMacroticks - 1) << 16)
	});

	registerWrites.push_back({ ERAYRegister::GTUC5,
		controllerConfig.DelayCompensationAMicroticks |
		(controllerConfig.DelayCompensationBMicroticks << 8) |
		(controllerConfig.ClusterDriftDamping << 16) |
		(controllerConfig.DecodingCorrectionMicroticks << 24)
	});

	registerWrites.push_back({ ERAYRegister::GTUC6,
		controllerConfig.AcceptStartupRangeMicroticks |
		(controllerConfig.RateCorrectionOutMicroticks << 16)
	});

	registerWrites.push_back({ ERAYRegister::GTUC7,
		(clusterConfig.StaticSlotMacroticks) |
		(clusterConfig.NumberOfStaticSlots << 16)
	});

	registerWrites.push_back({ ERAYRegister::GTUC8,
		clusterConfig.MinislotDurationMacroticks |
		(clusterConfig.NumberOfMinislots << 16)
	});

	registerWrites.push_back({ ERAYRegister::GTUC9,
		clusterConfig.ActionPointOffset |
		(clusterConfig.MinislotActionPointOffsetMacroticks << 8) |
		(clusterConfig.DynamicSlotIdlePhaseMinislots << 16)
	});

	registerWrites.push_back({ ERAYRegister::GTUC10,
		controllerConfig.OffsetCorrectionOutMicroticks |
		(controllerConfig.RateCorrectionOutMicroticks << 16)
	});

	registerWrites.push_back({ ERAYRegister::GTUC11,
		controllerConfig.ExternOffsetCorrectionControl |
		(controllerConfig.ExternRateCorrectionControl << 8) |
		(controllerConfig.ExternOffsetCorrectionMicroticks << 16) |
		(controllerConfig.ExternRateCorrectionMicroticks << 24)
	});

	std::vector<std::shared_ptr<MessageBuffer>> staticTx;
	std::vector<std::shared_ptr<MessageBuffer>> dynamicTx;

	// Add key slot messages
	std::shared_ptr<MessageBuffer> first = std::make_shared<MessageBuffer>();
	bool firstIsInMessageBuffers = false;
	bool firstUsed = false;

	std::shared_ptr<MessageBuffer> second = std::make_shared<MessageBuffer>();
	bool secondIsInMessageBuffers = false;
	bool secondUsed = false;

	if(controllerConfig.KeySlotUsedForSync || controllerConfig.KeySlotOnlyEnabled) {
		first->isStartup = controllerConfig.KeySlotUsedForStartup;
		first->isSync = controllerConfig.KeySlotUsedForSync;
		first->isTransmit = true;
		first->channelA = true;
		first->channelB = !controllerConfig.TwoKeySlotMode && controllerConfig.ChannelB;
		first->frameID = controllerConfig.KeySlotID;
		first->frameLengthBytes = clusterConfig.PayloadLengthOfStaticSlotInWords * 2;
		first->baseCycle = 0;
		first->cycleRepetition = 1;
		first->continuousMode = true;
		staticTx.push_back(first);
		firstUsed = true;

		if(controllerConfig.TwoKeySlotMode) {
			second->isStartup = controllerConfig.KeySlotUsedForStartup;
			second->isSync = controllerConfig.KeySlotUsedForSync;
			second->isTransmit = true;
			second->channelB =true;
			second->frameID = controllerConfig.SecondKeySlotID;
			second->frameLengthBytes = clusterConfig.PayloadLengthOfStaticSlotInWords * 2;
			second->baseCycle = 0;
			second->cycleRepetition = 1;
			second->continuousMode = true;
			staticTx.push_back(second);
			secondUsed = true;
		}
	}

	for(auto& buf : messageBuffers) {
		if(!buf->isTransmit)
			continue; // Only transmit frames need to be written to the controller

		if(buf->frameID == controllerConfig.KeySlotID) {
			first = buf;
			staticTx[0] = buf;
			// Enforce keyslot rules
			first->isStartup = controllerConfig.KeySlotUsedForStartup;
			first->isSync = controllerConfig.KeySlotUsedForSync;
			first->isDynamic = false;
			// Suppress default buffer
			firstIsInMessageBuffers = true;
			continue;
		}
		else if(controllerConfig.TwoKeySlotMode && buf->frameID == controllerConfig.SecondKeySlotID) {
			second = buf;
			staticTx[1] = buf;
			buf->isDynamic = false;
			// Enforce keyslot rules
			second->isStartup = controllerConfig.KeySlotUsedForStartup;
			second->isSync = controllerConfig.KeySlotUsedForSync;
			second->isDynamic = false;
			// Suppress default buffer
			secondIsInMessageBuffers = true;
			continue;
		}

		if(buf->isDynamic)
			dynamicTx.push_back(buf);
		else
			staticTx.push_back(buf);
	}

	// If the user is using the default coldstart messages, they need to be added to the list for transmit
	if(firstUsed && !firstIsInMessageBuffers)
		messageBuffers.push_back(first);
	if(secondUsed && !secondIsInMessageBuffers)
		messageBuffers.push_back(second);

	int64_t totalBuffers = staticTx.size() + dynamicTx.size();
	if(totalBuffers > 128) // TODO warn
		totalBuffers = 128;

	registerWrites.push_back({ ERAYRegister::MRC,
		(uint8_t(staticTx.size())) | // FDB[7:0] message buffers exclusively for the static segment
		// FFB[7:0] set to 0x80, No message buffer assigned to the FIFO
		(0x80 << 8) |
		(uint8_t(totalBuffers - 1) << 16) |
		(controllerConfig.TwoKeySlotMode << 26)
	});

	for(const auto& regpair : registerWrites) {
		if(!writeRegister(regpair.first, regpair.second, true, timeout))
			return false;
		updateTimeout();
	}

	uint16_t dataPointer = (totalBuffers + 1) * 4;
	for(auto i = 0; i < totalBuffers; i++) {
		MessageBuffer& buf = *(i < staticTx.size() ? staticTx[i] : dynamicTx[i - staticTx.size()]);

		if(buf.frameID == 0)
			buf.frameID = i | (1 << 10);

		uint32_t hs1 = (
			(buf.frameID) |
			(CalculateCycleFilter(buf.baseCycle, buf.cycleRepetition) << 16) |
			((buf.channelA & 0x1) << 24) |
			((buf.channelB & 0x1) << 25) |
			((buf.isTransmit & 0x1) << 26) | // CFG
			((buf.isNMFrame & 0x1) << 27) | // PPIT
			((!buf.continuousMode & 0x1) << 28) | // TXM
			((0 & 0x1) << 29) // MBI, disabled for now but we might want confirmations in the future
		);

		uint32_t hs2 = (
			CalculateHCRC(buf) |
			(((buf.frameLengthBytes + 1) / 2) << 16)
		);

		uint32_t hs3 = dataPointer;
		buf._dataPointer = dataPointer;
		buf._id = i;
		dataPointer += buf.frameLengthBytes / 4;
		dataPointer += dataPointer % 4; // must be a 4 byte boundary

		if(!writeRegister(ERAYRegister::WRHS1, hs1, true, timeout))
			return false;
		updateTimeout();

		if(!writeRegister(ERAYRegister::WRHS2, hs2, true, timeout))
			return false;
		updateTimeout();

		if(!writeRegister(ERAYRegister::WRHS3, hs3, true, timeout))
			return false;
		updateTimeout();

		if(!writeRegister(ERAYRegister::IBCM, 1, true, timeout))
			return false;
		const auto ibcmstart = std::chrono::steady_clock::now();
		bool ibcmbusy = isInputBufferHostBusy();
		while(ibcmbusy && (std::chrono::steady_clock::now() - ibcmstart) < timeout) {
			ibcmbusy = isInputBufferHostBusy();
		}
		if(ibcmbusy) // timeout
			return false;
		updateTimeout();

		if(!writeRegister(ERAYRegister::IBCR, i, true, timeout))
			return false;
		updateTimeout();
	}

	configDirty = false;
	return true;
}

bool FlexRay::Controller::getReady(std::chrono::milliseconds timeout) {
	const auto initialTimeout = timeout;
	const auto functionBegin = std::chrono::steady_clock::now();
	const auto updateTimeout = [&initialTimeout, &functionBegin, &timeout]() {
		timeout = std::chrono::duration_cast<std::chrono::milliseconds>(initialTimeout - (std::chrono::steady_clock::now() - functionBegin));
	};

	// Initial sanity check that we have communication with the controller
	auto endian = readRegister(ERAYRegister::ENDN, timeout);
	if (!endian.first || endian.second != 0x87654321)
		return false;
	updateTimeout();

	auto statusPair = getCurrentPOCStatus(timeout);
	const auto& status = statusPair.second;
	if(!statusPair.first)
		return false;
	updateTimeout();

	if(status == POCStatus::Ready && !configDirty) {
		// Already in the desired state
		if(allowColdstart && !setCurrentPOCCommand(FlexRay::POCCommand::AllowColdstart, true, timeout))
			return false;
		return true;
	}

	if(status != POCStatus::Config) {
		// Must enter config before continuing
		if(!enterConfig(timeout))
			return false;
		updateTimeout();

		// Reconfigure if necessary
		if(configDirty && !configure(timeout))
			return false;
		updateTimeout();
	}

	// Enter the READY state
	if(!lockConfiguration(timeout))
		return false;
	updateTimeout();

	// Signal that we'd like to coldstart, if necessary
	if(allowColdstart && !setCurrentPOCCommand(FlexRay::POCCommand::AllowColdstart, true, timeout))
		return false;
	return true;
}

bool FlexRay::Controller::start(std::chrono::milliseconds timeout) {
	const auto initialTimeout = timeout;
	const auto functionBegin = std::chrono::steady_clock::now();
	const auto updateTimeout = [&initialTimeout, &functionBegin, &timeout]() {
		timeout = std::chrono::duration_cast<std::chrono::milliseconds>(initialTimeout - (std::chrono::steady_clock::now() - functionBegin));
	};

	// First make sure we're ready to start (configured/ready state)
	if(!getReady(timeout))
		return false;
	updateTimeout();

	// Wakeup the network if necessary
	if(wakeupBeforeStart && !wakeup(timeout))
		return false;
	updateTimeout();

	// And finally run
	if(!setCurrentPOCCommand(FlexRay::POCCommand::Run, false, timeout))
		return false;
	return true;
}

bool FlexRay::Controller::transmit(const std::shared_ptr<FlexRayMessage>& frmsg) {
	bool success = false;

	for(const auto& buf : messageBuffers) {
		if(!buf->isTransmit)
			continue;

		if(frmsg->slotid != buf->frameID)
			continue;

		if(CalculateCycleFilter(frmsg->cycle, frmsg->cycleRepetition) != CalculateCycleFilter(buf->baseCycle, buf->cycleRepetition))
			continue;

		FlexRay::Channel bufChannel = FlexRay::Channel::None;
		if(buf->channelA && buf->channelB)
			bufChannel = FlexRay::Channel::AB;
		else if(buf->channelA)
			bufChannel = FlexRay::Channel::A;
		else if(buf->channelB)
			bufChannel = FlexRay::Channel::B;
		
		if(frmsg->channel != bufChannel)
			continue;

		// This is a message buffer we want to fill
		if(!device.com->sendCommand(Command::FlexRayControl, FlexRayControlMessage::BuildWriteMessageBufferArgs(index, buf->_id, frmsg->data, buf->frameLengthBytes)))
			continue;

		success = true;
	}

	return success;
}

bool FlexRay::Controller::halt(std::chrono::milliseconds timeout) {
	return setCurrentPOCCommand(POCCommand::Halt, true, timeout);
}

bool FlexRay::Controller::freeze(std::chrono::milliseconds timeout) {
	return setCurrentPOCCommand(POCCommand::Freeze, true, timeout);
}

bool FlexRay::Controller::triggerMTS(std::chrono::milliseconds timeout) {
	// triggerMTS will do nothing unless either MTSOnA or MTSOnB (or both) are set at configure time
	return setCurrentPOCCommand(POCCommand::SendMTS, true, timeout);
}

std::pair<bool, FlexRay::POCCommand> FlexRay::Controller::getCurrentPOCCommand(std::chrono::milliseconds timeout) const {
	const auto val = readRegister(ERAYRegister::SUCC1, timeout);
	return {val.first, FlexRay::POCCommand(val.second & 0x0000000F)};
}

bool FlexRay::Controller::setCurrentPOCCommand(FlexRay::POCCommand cmd, bool checkForSuccess, std::chrono::milliseconds timeout) {
	const auto beforeWrite = std::chrono::steady_clock::now();
	if(!writeRegister(ERAYRegister::SUCC1, uint32_t(cmd), 0xF, true, timeout))
		return false;
	if(!checkForSuccess)
		return true;
	
	const auto writeDuration = std::chrono::steady_clock::now() - beforeWrite;
	timeout = std::chrono::duration_cast<std::chrono::milliseconds>(timeout - writeDuration);

	return wasCommandSuccessful(timeout);
}

bool FlexRay::Controller::wasCommandSuccessful(std::chrono::milliseconds timeout) const {
	const auto start = std::chrono::steady_clock::now();
	bool pocBusy = isPOCBusy();
	while(pocBusy && (std::chrono::steady_clock::now() - start) < timeout) {
		pocBusy = isPOCBusy();
	}
	if(pocBusy) // timeout
		return false;
	timeout = std::chrono::duration_cast<std::chrono::milliseconds>(timeout - (std::chrono::steady_clock::now() - start));

	const auto val = getCurrentPOCCommand(timeout);
	return val.first && val.second != FlexRay::POCCommand::CommandNotAccepted;
}

std::pair<bool, FlexRay::POCStatus> FlexRay::Controller::getCurrentPOCStatus(std::chrono::milliseconds timeout) const {
	auto regpair = readRegister(ERAYRegister::CCSV, timeout);
	return { regpair.first, FlexRay::POCStatus(regpair.second & 0x3F) };
}

bool FlexRay::Controller::lockConfiguration(std::chrono::milliseconds timeout) {
	// This is not anything super special, just the way to get the ERAY out of POC:config
	// See the ERAY Users Manaual section 4.3.1

	auto beforeWrite = std::chrono::steady_clock::now();
	if(!writeRegister(ERAYRegister::LCK, 0xCE, true, timeout))
		return false;
	timeout = std::chrono::duration_cast<std::chrono::milliseconds>(timeout - (std::chrono::steady_clock::now() - beforeWrite));
	if(timeout.count() <= 0)
		return false; // Out of time!

	beforeWrite = std::chrono::steady_clock::now();
	if(!writeRegister(ERAYRegister::LCK, 0x31, true, timeout))
		return false;
	timeout = std::chrono::duration_cast<std::chrono::milliseconds>(timeout - (std::chrono::steady_clock::now() - beforeWrite));

	return setCurrentPOCCommand(POCCommand::Ready, true, timeout);
}

bool FlexRay::Controller::enterConfig(std::chrono::milliseconds timeout) {
	const auto initialTimeout = timeout;
	const auto functionBegin = std::chrono::steady_clock::now();
	const auto updateTimeout = [&initialTimeout, &functionBegin, &timeout]() {
		timeout = std::chrono::duration_cast<std::chrono::milliseconds>(initialTimeout - (std::chrono::steady_clock::now() - functionBegin));
	};

	auto statusPair = getCurrentPOCStatus(timeout);
	const auto& status = statusPair.second;
	if(!statusPair.first)
		return false;
	updateTimeout();

	if(status != FlexRay::POCStatus::Ready &&
		status != FlexRay::POCStatus::Config &&
		status != FlexRay::POCStatus::DefaultConfig &&
		status != FlexRay::POCStatus::Halt) {
		if(!setCurrentPOCCommand(FlexRay::POCCommand::Freeze, true, timeout))
			return false;
		updateTimeout();
	}

	// If we're halted, we first go into DEFAULT_CONFIG before entering CONFIG
	// Unintuitively, this enters DEFAULT_CONFIG
	if(!setCurrentPOCCommand(FlexRay::POCCommand::Config, true, timeout))
		return false;
	updateTimeout();

	// Now this enters CONFIG
	return setCurrentPOCCommand(FlexRay::POCCommand::Config, true, timeout);
}

uint16_t FlexRay::Controller::CalculateHCRC(const MessageBuffer& buf) {
	uint16_t ret = 0x1A;

	auto addBit = [&ret](uint8_t bit) {
		bit = bit ? 1 : 0;

		int crcNxt; //CRCNXT = NXTBIT EXOR CRC_RG(14);
		if (ret & (1<<10))
			crcNxt = bit ^ 1;
		else
			crcNxt = bit ^ 0;
		crcNxt &= 0x01;

		// CRC_RG(14:1) = CRC_RG(13:0); // shift left by
		ret <<= 1;
		ret &= 0x7FE; // clear first bit

		if (crcNxt) //CRC_RG(14:0) = CRC_RG(14:0) EXOR (4599hex);
			ret ^= 0x385;
	};

	addBit(buf.isStartup);
	addBit(buf.isSync);
	for(auto i = 0; i < 11; i++)
		addBit(buf.frameID & (1 << (10 - i)));
	for(auto i = 0; i < 7; i++)
		addBit(((buf.frameLengthBytes + 1) / 2) & (1 << (6 - i)));

	return ret;
}

uint16_t FlexRay::Controller::CalculateCycleFilter(uint8_t baseCycle, uint8_t cycleRepetition) {
	uint8_t cycleRepCode = 0;
	switch(cycleRepetition) {
		case 1: cycleRepCode = 0b1; break;
		case 2: cycleRepCode = 0b10; break;
		case 4: cycleRepCode = 0b100; break;
		case 8: cycleRepCode = 0b1000; break;
		case 16: cycleRepCode = 0b10000; break;
		case 32: cycleRepCode = 0b100000; break;
		case 64: cycleRepCode = 0b1000000; break;
	}
	return (cycleRepCode | baseCycle);
}

std::pair<bool, uint32_t> FlexRay::Controller::readRegister(ERAYRegister reg, std::chrono::milliseconds timeout) const {
	if(timeout.count() <= 0)
		return {false, 0}; // Out of time!

	std::lock_guard<std::mutex> lk(readRegisterLock);
	device.com->sendCommand(Command::FlexRayControl, FlexRayControlMessage::BuildReadCCRegsArgs(index, uint16_t(reg)));
	std::shared_ptr<FlexRayControlMessage> resp;
	const auto start = std::chrono::steady_clock::now();
	while((std::chrono::steady_clock::now() - start) < timeout) {
		auto msg = device.com->waitForMessageSync(MessageFilter(icsneo::Network::NetID::FlexRayControl), timeout / 4);
		if(auto frmsg = std::dynamic_pointer_cast<FlexRayControlMessage>(msg)) {
			if(frmsg->decoded && frmsg->controller == index && frmsg->opcode == FlexRay::Opcode::ReadCCRegs)
				resp = frmsg;
		}
		if(resp)
			break;
		device.com->sendCommand(Command::FlexRayControl, FlexRayControlMessage::BuildReadCCRegsArgs(index, uint16_t(reg)));
	}
	if(resp && !resp->registers.empty())
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

	if(waitForPOCReady) {
		const auto start = std::chrono::steady_clock::now();
		bool pocBusy = isPOCBusy();
		while(pocBusy && (std::chrono::steady_clock::now() - start) < timeout) {
			pocBusy = isPOCBusy();
		}
		if(pocBusy) // timeout
			return false;
	}

	if(mask != 0xffffffff) {
		const auto beforeRead = std::chrono::steady_clock::now();
		auto pair = readRegister(reg, timeout);
		if(!pair.first)
			return false; // Couldn't read, so we don't want to try to write anything
		auto readDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beforeRead);
		timeout -= readDuration;
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
	if(timeout.count() <= 0)
		return false; // Out of time!

	if(waitForPOCReady) {
		const auto start = std::chrono::steady_clock::now();
		bool pocBusy = isPOCBusy();
		while(pocBusy && (std::chrono::steady_clock::now() - start) < timeout) {
			pocBusy = isPOCBusy();
		}
		if(pocBusy) // timeout
			return false;
	}

	if(!device.com->sendCommand(Command::FlexRayControl, FlexRayControlMessage::BuildWriteCCRegArgs(index, uint16_t(reg), value)))
		return false;
	return true; // The device does not confirm the the command, if it did we'd put that here
}