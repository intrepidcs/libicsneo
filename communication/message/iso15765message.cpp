#include "icsneo/communication/message/iso15765message.h"

#include <cstring>

using namespace icsneo;

std::shared_ptr<J2534CommandMessage> J2534CommandMessage::decodeToMessage(const std::vector<uint8_t>& data) {
	if(data.size() < 2)
		return nullptr;

	auto msg = std::make_shared<J2534CommandMessage>((J2534Command)data[1]);
	if(data.size() > 2)
        msg->setArgumentData(std::vector<uint8_t>(data.begin() + 2, data.end()));

	return msg;
}

uint16_t J2534CommandMessage::read16LE(size_t o) const {
	return (uint16_t)((uint16_t)argumentData[o] | ((uint16_t)argumentData[o + 1] << 8));
}

void J2534CommandMessage::write16LE(size_t o, uint16_t v) {
	argumentData[o] = (uint8_t)(v & 0xFF);
	argumentData[o + 1] = (uint8_t)((v >> 8) & 0xFF);
}

uint16_t J2534CommandMessage::read16BE(size_t o) const {
	return (uint16_t)(((uint16_t)argumentData[o] << 8) | argumentData[o + 1]);
}

void J2534CommandMessage::write16BE(size_t o, uint16_t v) {
	argumentData[o] = (uint8_t)((v >> 8) & 0xFF);
	argumentData[o + 1] = (uint8_t)(v & 0xFF);
}

void J2534CommandMessage::writeBits16LE(size_t o, unsigned bitPos, unsigned bitCount, uint32_t value) {
	const uint16_t mask = (uint16_t)(((1u << bitCount) - 1u) << bitPos);
	uint16_t word = read16LE(o);
	word = (uint16_t)((word & ~mask) | (((value << bitPos) & mask)));
	write16LE(o, word);
}

uint32_t J2534CommandMessage::read32LE(size_t o) const {
	return  (uint32_t)argumentData[o]
		| ((uint32_t)argumentData[o + 1] << 8)
		| ((uint32_t)argumentData[o + 2] << 16)
		| ((uint32_t)argumentData[o + 3] << 24);
}

void J2534CommandMessage::write32LE(size_t o, uint32_t v) {
	argumentData[o] = (uint8_t)(v & 0xFF);
	argumentData[o + 1] = (uint8_t)((v >> 8) & 0xFF);
	argumentData[o + 2] = (uint8_t)((v >> 16) & 0xFF);
	argumentData[o + 3] = (uint8_t)((v >> 24) & 0xFF);
}

uint32_t J2534CommandMessage::read32BE(size_t o) const {
	return ((uint32_t)argumentData[o] << 24) | ((uint32_t)argumentData[o + 1] << 16)
		| ((uint32_t)argumentData[o + 2] << 8) | (uint32_t)argumentData[o + 3];
}

void J2534CommandMessage::write32BE(size_t o, uint32_t v) {
	argumentData[o] = (uint8_t)((v >> 24) & 0xFF);
	argumentData[o + 1] = (uint8_t)((v >> 16) & 0xFF);
	argumentData[o + 2] = (uint8_t)((v >> 8) & 0xFF);
	argumentData[o + 3] = (uint8_t)(v & 0xFF);
}

J2534_RxCanFilter J2534SetupCanRxFilteringMessage::getRxCanFilter() const {
	constexpr size_t filterSize = sizeof(J2534_RxCanFilter::fb);
	J2534_RxCanFilter filter = {};
	if(argumentData.size() >= (10 + filterSize))
	{
		filter.idx = (argumentData[2] << 8) | argumentData[3];
		filter.enabled = (argumentData[4] << 8) | argumentData[5];
		filter.filterCount = (argumentData[6] << 8) | argumentData[7];
		filter.fb = *((MessageFilterBytes*)(argumentData.data() + 8));
		filter.networkId = (argumentData[8 + filterSize] << 8) | argumentData[9 + filterSize];
	}
	return filter;
}

void J2534SetupCanRxFilteringMessage::setRxCanFilter(const J2534_RxCanFilter& filter) {
	argumentData.resize(10 + sizeof(filter.fb));
	argumentData[2] = (filter.idx >> 8) & 0xFF;
	argumentData[3] = filter.idx & 0xFF;
	argumentData[4] = (filter.enabled >> 8) & 0xFF;
	argumentData[5] = filter.enabled & 0xFF;
	argumentData[6] = (filter.filterCount >> 8) & 0xFF;
	argumentData[7] = filter.filterCount & 0xFF;
    *((MessageFilterBytes*)(argumentData.data() + 8)) = filter.fb;
	argumentData[8 + sizeof(filter.fb)] = (filter.networkId >> 8) & 0xFF;
	argumentData[9 + sizeof(filter.fb)] = filter.networkId & 0xFF;
}


J2534SetupIso15765FlowControlMessage::J2534SetupIso15765FlowControlMessage()
	: J2534CommandMessage(J2534Command::SetupIso15765RxFilter) {
	argumentData.resize(MessageSize);
	// Bytes [0..1] were populated by the base constructor. Everything else starts at zero
	// (matches the previous behavior which zeroed the flt region and left the rest of the
	// caller-supplied struct as-is; typical callers value-initialized their struct too).
	for(size_t i = 2; i < MessageSize; ++i)
		argumentData[i] = 0;
}

void J2534SetupIso15765FlowControlMessage::rebuildFilterBytes(void) {
	// Zero the 28-byte flt region.
	for(size_t i = 0; i < FltSize; ++i)
		argumentData[FltOffset + i] = 0;

	const uint32_t id      = read32LE(IdOffset);
	const uint32_t idMask  = read32LE(IdMaskOffset);
	const bool is29Bit     = getFlagBit(FlagId29BitEnable);
	const bool extAddr     = getFlagBit(FlagExtAddressEnable);
	const uint8_t extAddrByte = argumentData[ExtAddrOffset];

	// CoreMiniMessageFilterBytes has an interleaved layout:
	//   uiFilterMask0 at flt+0,  uiFilterID0 at flt+2
	//   uiFilterMask1 at flt+4,  uiFilterID1 at flt+6
	//   uiFilterMask2 at flt+8,  uiFilterID2 at flt+10
	//   datalink.uiFilterMask3 at flt+12, datalink.uiFilterID3 at flt+14
	//
	// CoreMiniMsgBitsCAN bit layout within each 16-bit unit:
	//   unit0: IDE(bit0), SRR(bit1), SID(bits2-12), NETWORKINDEX(bits13-15)
	//   unit1: EID(bits0-11), TXMSG(bit12), ...
	//   unit2: DLC(bits0-3), ..., RTR(bit9), EID2(bits10-15)
	const size_t MaskUnit0  = FltOffset + 0;
	const size_t MaskUnit1  = FltOffset + 4;
	const size_t MaskUnit2  = FltOffset + 8;
	const size_t ValueUnit0 = FltOffset + 2;
	const size_t ValueUnit1 = FltOffset + 6;
	const size_t ValueUnit2 = FltOffset + 10;

	// filter arbid: pMask->IDE = 1
	writeBits16LE(MaskUnit0, /*bit*/0, /*width*/1, 1);

	if(is29Bit) {
		// pValue->IDE = 1; pValue->SID = (id >> 18) & 0x7FF
		writeBits16LE(ValueUnit0, 0, 1, 1);
		writeBits16LE(ValueUnit0, 2, 11, (id >> 18) & 0x7FF);
		// pValue->EID = (id >> 6) & 0xFFF
		writeBits16LE(ValueUnit1, 0, 12, (id >> 6) & 0xFFF);
		// pValue->EID2 = id & 0x3F
		writeBits16LE(ValueUnit2, 10, 6, id & 0x3F);
		// pMask->SID = (id_mask >> 18) & 0x7FF
		writeBits16LE(MaskUnit0, 2, 11, (idMask >> 18) & 0x7FF);
		// pMask->EID = (id_mask >> 6) & 0xFFF  (overlaps pValue->{IDE,SID} storage)
		writeBits16LE(MaskUnit1, 0, 12, (idMask >> 6) & 0xFFF);
		// pMask->EID2 = id_mask & 0x3F         (overlaps pValue->EID storage)
		writeBits16LE(MaskUnit2, 10, 6, idMask & 0x3F);
	} else {
		// pValue->IDE = 0 (already zero); pValue->SID = id
		writeBits16LE(ValueUnit0, 2, 11, id & 0x7FF);
		// pMask->SID = id_mask
		writeBits16LE(MaskUnit0, 2, 11, idMask & 0x7FF);
	}

	// Extended-address filter: writes into the datalink.uiFilterID3 / uiFilterMask3 bytes.
	//   uiFilterMask3 = flt[12..13] = wire[37..38]
	//   uiFilterID3   = flt[14..15] = wire[39..40]
	if(extAddr) {
		const size_t kValueBytes = FltOffset + 14; // uiFilterID3
		const size_t kMaskBytes  = FltOffset + 12; // uiFilterMask3
		argumentData[kValueBytes + 0] = extAddrByte;
		argumentData[kMaskBytes  + 0] = 0xFF;
		argumentData[kValueBytes + 1] = 0;
		argumentData[kMaskBytes  + 1] = 0;
	}
}

uint16_t J2534SetupIso15765FlowControlMessage::getIdx(void) const { return read16LE(IdxOffset); }
void J2534SetupIso15765FlowControlMessage::setIdx(uint16_t idx) { write16LE(IdxOffset, idx); }

uint16_t J2534SetupIso15765FlowControlMessage::getCoreMiniId(void) const { return read16LE(CoreMiniOffset); }
void J2534SetupIso15765FlowControlMessage::setCoreMiniId(uint16_t coreMiniId) { write16LE(CoreMiniOffset, coreMiniId); }

uint8_t J2534SetupIso15765FlowControlMessage::getPadding(void) const { return argumentData[PaddingOffset]; }
void J2534SetupIso15765FlowControlMessage::setPadding(uint8_t padding) { argumentData[PaddingOffset] = padding; }

uint32_t J2534SetupIso15765FlowControlMessage::getId(void) const { return read32LE(IdOffset); }
void J2534SetupIso15765FlowControlMessage::setId(uint32_t id) { write32LE(IdOffset, id); rebuildFilterBytes(); }

uint32_t J2534SetupIso15765FlowControlMessage::getIdMask(void) const { return read32LE(IdMaskOffset); }
void J2534SetupIso15765FlowControlMessage::setIdMask(uint32_t idMask) { write32LE(IdMaskOffset, idMask); rebuildFilterBytes(); }

uint32_t J2534SetupIso15765FlowControlMessage::getFcId(void) const { return read32LE(FcIdOffset); }
void J2534SetupIso15765FlowControlMessage::setFcId(uint32_t fcId) { write32LE(FcIdOffset, fcId); }

uint8_t J2534SetupIso15765FlowControlMessage::getFlowControlExtendedAddress(void) const { return argumentData[FcExtAddrOffset]; }
void J2534SetupIso15765FlowControlMessage::setFlowControlExtendedAddress(uint8_t addr) { argumentData[FcExtAddrOffset] = addr; }

uint8_t J2534SetupIso15765FlowControlMessage::getExtendedAddress(void) const { return argumentData[ExtAddrOffset]; }
void J2534SetupIso15765FlowControlMessage::setExtendedAddress(uint8_t addr) { argumentData[ExtAddrOffset] = addr; rebuildFilterBytes(); }

uint8_t J2534SetupIso15765FlowControlMessage::getBlockSize(void) const { return argumentData[BlockSizeOffset]; }
void J2534SetupIso15765FlowControlMessage::setBlockSize(uint8_t blockSize) { argumentData[BlockSizeOffset] = blockSize; }

uint8_t J2534SetupIso15765FlowControlMessage::getStMin(void) const { return argumentData[StMinOffset]; }
void J2534SetupIso15765FlowControlMessage::setStMin(uint8_t stMin) { argumentData[StMinOffset] = stMin; }

uint16_t J2534SetupIso15765FlowControlMessage::getCfTimeout(void) const { return read16LE(CfTimeoutOffset); }
void J2534SetupIso15765FlowControlMessage::setCfTimeout(uint16_t cfTimeout) { write16LE(CfTimeoutOffset, cfTimeout); }

uint32_t J2534SetupIso15765FlowControlMessage::getFlags(void) const { return read32LE(FlagsOffset); }
void J2534SetupIso15765FlowControlMessage::setFlags(uint32_t flags) { write32LE(FlagsOffset, flags); rebuildFilterBytes(); }

bool J2534SetupIso15765FlowControlMessage::getEnable(void) const { return getFlagBit(FlagEnable); }
void J2534SetupIso15765FlowControlMessage::setEnable(bool enable) { setFlagBit(FlagEnable, enable); }

bool J2534SetupIso15765FlowControlMessage::getIs29BitEnabled(void) const { return getFlagBit(FlagId29BitEnable); }
void J2534SetupIso15765FlowControlMessage::setIs29BitEnabled(bool enable) { setFlagBit(FlagId29BitEnable, enable); rebuildFilterBytes(); }

bool J2534SetupIso15765FlowControlMessage::getIsFc29BitEnabled(void) const { return getFlagBit(FlagFcId29BitEnable); }
void J2534SetupIso15765FlowControlMessage::setIsFc29BitEnabled(bool enable) { setFlagBit(FlagFcId29BitEnable, enable); }

bool J2534SetupIso15765FlowControlMessage::getExtAddressEnabled(void) const { return getFlagBit(FlagExtAddressEnable); }
void J2534SetupIso15765FlowControlMessage::setExtAddressEnabled(bool enable) { setFlagBit(FlagExtAddressEnable, enable); rebuildFilterBytes(); }

bool J2534SetupIso15765FlowControlMessage::getFcExtAddressEnabled(void) const { return getFlagBit(FlagFcExtAddressEnable); }
void J2534SetupIso15765FlowControlMessage::setFcExtAddressEnabled(bool enable) { setFlagBit(FlagFcExtAddressEnable, enable); }

bool J2534SetupIso15765FlowControlMessage::getFlowControlTransmissionEnabled(void) const { return getFlagBit(FlagEnableFlowControlTransmit); }
void J2534SetupIso15765FlowControlMessage::setFlowControlTransmissionEnabled(bool enable) { setFlagBit(FlagEnableFlowControlTransmit, enable); }

bool J2534SetupIso15765FlowControlMessage::getPaddingEnabled(void) const { return getFlagBit(FlagPaddingEnable); }
void J2534SetupIso15765FlowControlMessage::setPaddingEnabled(bool enable) { setFlagBit(FlagPaddingEnable, enable); }

bool J2534SetupIso15765FlowControlMessage::getIsCanFd(void) const { return getFlagBit(FlagIsCanFd); }
void J2534SetupIso15765FlowControlMessage::setIsCanFd(bool enable) { setFlagBit(FlagIsCanFd, enable); }

bool J2534SetupIso15765FlowControlMessage::getIsBrsEnabled(void) const { return getFlagBit(FlagIsBrsEnabled); }
void J2534SetupIso15765FlowControlMessage::setIsBrsEnabled(bool enable) { setFlagBit(FlagIsBrsEnabled, enable); }

void J2534SetupIso15765FlowControlMessage::setFlagBit(uint32_t mask, bool enable) {
	uint32_t f = read32LE(FlagsOffset);
	if(enable) f |= mask;
	else        f &= ~mask;
	write32LE(FlagsOffset, f);
}

bool J2534SetupIso15765FlowControlMessage::getFlagBit(uint32_t mask) const {
	return (read32LE(FlagsOffset) & mask) != 0;
}

uint16_t Iso15765TxSetupMessage::getIdx(void) const { return read16LE(SetupIdxOffset); }
void Iso15765TxSetupMessage::setIdx(uint16_t idx) { write16LE(SetupIdxOffset, idx); }

uint16_t Iso15765TxSetupMessage::getCoreMiniId(void) const { return read16LE(SetupCoreMiniOffset); }
void Iso15765TxSetupMessage::setCoreMiniId(uint16_t coreMiniId) { write16LE(SetupCoreMiniOffset, coreMiniId); }

uint32_t Iso15765TxSetupMessage::getMessageLength(void) const { return read32LE(SetupMsgLenOffset); }
void Iso15765TxSetupMessage::setMessageLength(uint32_t messageLength) { write32LE(SetupMsgLenOffset, messageLength); }

uint8_t Iso15765TxSetupMessage::getPadding(void) const { return argumentData[SetupPaddingOffset]; }
void Iso15765TxSetupMessage::setPadding(uint8_t padding) { argumentData[SetupPaddingOffset] = padding; }

uint8_t Iso15765TxSetupMessage::getTxDl(void) const { return argumentData[SetupTxDlOffset]; }
void Iso15765TxSetupMessage::setTxDl(uint8_t txDl) { argumentData[SetupTxDlOffset] = txDl; }

uint32_t Iso15765TxSetupMessage::getId(void) const { return read32LE(SetupIdOffset); }
void Iso15765TxSetupMessage::setId(uint32_t id) { write32LE(SetupIdOffset, id); }

uint32_t Iso15765TxSetupMessage::getFcId(void) const { return read32LE(SetupFcIdOffset); }
void Iso15765TxSetupMessage::setFcId(uint32_t fcId) { write32LE(SetupFcIdOffset, fcId); }

uint32_t Iso15765TxSetupMessage::getFcIdMask(void) const { return read32LE(SetupFcIdMaskOffset); }
void Iso15765TxSetupMessage::setFcIdMask(uint32_t fcIdMask) { write32LE(SetupFcIdMaskOffset, fcIdMask); }

uint8_t Iso15765TxSetupMessage::getFlowControlExtendedAddress(void) const { return argumentData[SetupFcExtAddrOffset]; }
void Iso15765TxSetupMessage::setFlowControlExtendedAddress(uint8_t addr) { argumentData[SetupFcExtAddrOffset] = addr; }

uint8_t Iso15765TxSetupMessage::getExtendedAddress(void) const { return argumentData[SetupExtAddrOffset]; }
void Iso15765TxSetupMessage::setExtendedAddress(uint8_t addr) { argumentData[SetupExtAddrOffset] = addr; }

uint16_t Iso15765TxSetupMessage::getFsTimeout(void) const { return read16LE(SetupFsTimeoutOffset); }
void Iso15765TxSetupMessage::setFsTimeout(uint16_t timeout) { write16LE(SetupFsTimeoutOffset, timeout); }

uint16_t Iso15765TxSetupMessage::getFsWait(void) const { return read16LE(SetupFsWaitOffset); }
void Iso15765TxSetupMessage::setFsWait(uint16_t wait) { write16LE(SetupFsWaitOffset, wait); }

uint32_t Iso15765TxSetupMessage::getFlags(void) const { return read32LE(SetupFlagsOffset); }
void Iso15765TxSetupMessage::setFlags(uint32_t flags) { write32LE(SetupFlagsOffset, flags); }

uint8_t Iso15765TxSetupMessage::getStMin(void) const { return argumentData[SetupStMinOffset]; }
void Iso15765TxSetupMessage::setStMin(uint8_t stMin) { argumentData[SetupStMinOffset] = stMin; }

uint8_t Iso15765TxSetupMessage::getBlockSize(void) const { return argumentData[SetupBlockSizeOffset]; }
void Iso15765TxSetupMessage::setBlockSize(uint8_t blockSize) { argumentData[SetupBlockSizeOffset] = blockSize; }

bool Iso15765TxSetupMessage::getIs29BitEnabled(void) const { return getFlagBit(FlagId29BitEnable); }
void Iso15765TxSetupMessage::setIs29BitEnabled(bool enable) { setFlagBit(FlagId29BitEnable, enable); }
bool Iso15765TxSetupMessage::getIsFc29BitEnabled(void) const { return getFlagBit(FlagFcId29BitEnable); }
void Iso15765TxSetupMessage::setIsFc29BitEnabled(bool enable) { setFlagBit(FlagFcId29BitEnable, enable); }
bool Iso15765TxSetupMessage::getExtAddressEnabled(void) const { return getFlagBit(FlagExtAddressEnable); }
void Iso15765TxSetupMessage::setExtAddressEnabled(bool enable) { setFlagBit(FlagExtAddressEnable, enable); }
bool Iso15765TxSetupMessage::getFcExtAddressEnabled(void) const { return getFlagBit(FlagFcExtAddressEnable); }
void Iso15765TxSetupMessage::setFcExtAddressEnabled(bool enable) { setFlagBit(FlagFcExtAddressEnable, enable); }
bool Iso15765TxSetupMessage::getOverrideStMin(void) const { return getFlagBit(FlagOverrideStMin); }
void Iso15765TxSetupMessage::setOverrideStMin(bool enable) { setFlagBit(FlagOverrideStMin, enable); }
bool Iso15765TxSetupMessage::getOverrideBlockSize(void) const { return getFlagBit(FlagOverrideBlockSize); }
void Iso15765TxSetupMessage::setOverrideBlockSize(bool enable) { setFlagBit(FlagOverrideBlockSize, enable); }
bool Iso15765TxSetupMessage::getPaddingEnabled(void) const { return getFlagBit(FlagPaddingEnable); }
void Iso15765TxSetupMessage::setPaddingEnabled(bool enable) { setFlagBit(FlagPaddingEnable, enable); }
bool Iso15765TxSetupMessage::getIsCanFd(void) const { return getFlagBit(FlagIsCanFd); }
void Iso15765TxSetupMessage::setIsCanFd(bool enable) { setFlagBit(FlagIsCanFd, enable); }
bool Iso15765TxSetupMessage::getIsBrsEnabled(void) const { return getFlagBit(FlagIsBrsEnabled); }
void Iso15765TxSetupMessage::setIsBrsEnabled(bool enable) { setFlagBit(FlagIsBrsEnabled, enable); }

bool Iso15765TxSetupMessage::getFlagBit(uint32_t mask) const {
	return (getFlags() & mask) != 0;
}

void Iso15765TxSetupMessage::setFlagBit(uint32_t mask, bool enable) {
	uint32_t f = getFlags();
	if(enable) f |= mask;
	else        f &= ~mask;
	setFlags(f);
}

uint16_t Iso15765TxDataMessage::getIdx(void) const { return read16LE(DataIdxOffset); }
void Iso15765TxDataMessage::setIdx(uint16_t idx) { write16LE(DataIdxOffset, idx); }

uint32_t Iso15765TxDataMessage::getOffset(void) const { return read32LE(DataOffsetOffset); }
void Iso15765TxDataMessage::setOffset(uint32_t offset) { write32LE(DataOffsetOffset, offset); }

uint16_t Iso15765TxDataMessage::getLen(void) const { return read16LE(DataLenOffset); }

const uint8_t* Iso15765TxDataMessage::getData(void) const { return argumentData.data() + DataPayloadOffset; }

void Iso15765TxDataMessage::setData(const uint8_t* sourceData, uint16_t len) {
    len = std::min(len, (uint16_t)MaxDataLength);
	argumentData.resize(DataPayloadOffset + len);
	write16LE(DataLenOffset, len);
	if(len > 0 && sourceData != nullptr)
		std::copy(sourceData, sourceData + len, argumentData.begin() + DataPayloadOffset);
}