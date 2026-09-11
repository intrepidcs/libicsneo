#ifndef __ISO_15765MESSAGE_H_
#define __ISO_15765MESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/main51message.h"
#include "icsneo/communication/command.h"
#include "icsneo/J2534.h"

#include <memory>
#include <optional>
#include <vector>

namespace icsneo {

class J2534CommandMessage : public Main51Message
{
public:
	enum class J2534Command : uint8_t
	{
		Enable = 0,	//RED_CMD_J2534_EXTENSION_ENABLE,
		SetupIso15765RxFilter = 1, //RED_CMD_J2534_EXTENSION_SETUP_ISO15_RX_FILTER,
		SetupIso15765TxMessageProperties = 2, //RED_CMD_J2534_EXTENSION_SETUP_ISO15_TX_MESSAGE_PROPERTIES,
		SetupIso15765TxMessageDataBytes = 3, //RED_CMD_J2534_EXTENSION_SETUP_ISO15_TX_MESSAGE_DATABYTES,
		SetupCanRxFilter = 4, //RED_CMD_J2534_EXTENSION_SETUP_CAN_RX_FILTER,
		SetupClearCanRxFilters = 5, //RED_CMD_J2534_EXTENSION_SETUP_CLEAR_CAN_RX_FILTERS,
		EnableFiltering = 6, //RED_CMD_J2534_EXTENSION_ENABLE_FILTERING,
	};

	J2534CommandMessage(const J2534Command j2534Command, const std::vector<uint8_t>& arguments = {})
		: Main51Message() {
		command = Command::J2534Command;
		argumentData.push_back((uint8_t)j2534Command);
		argumentData.push_back(0);
		setArgumentData(arguments);
	}
	virtual ~J2534CommandMessage(void) = default;

    J2534Command getJ2534Command(void) const { return (J2534Command)argumentData[0]; }
    void setJ2534Command(const J2534Command& j2534Command) { argumentData[0] = (uint8_t)j2534Command; }

    const std::vector<uint8_t>& getArgumentData(void) const { return argumentData; }
    void setArgumentData(const std::vector<uint8_t>& arguments) { argumentData.insert(argumentData.end(), arguments.begin(), arguments.end()); }

	static std::shared_ptr<J2534CommandMessage> decodeToMessage(const std::vector<uint8_t>& data);

protected:
    std::vector<uint8_t> argumentData;

	uint16_t read16LE(size_t o) const;
	void write16LE(size_t o, uint16_t v);
	uint16_t read16BE(size_t o) const;
	void write16BE(size_t o, uint16_t v);
	void writeBits16LE(size_t o, unsigned bitPos, unsigned bitCount, uint32_t value);
	uint32_t read32LE(size_t o) const;
	void write32LE(size_t o, uint32_t v);
	uint32_t read32BE(size_t o) const;
	void write32BE(size_t o, uint32_t v);
};

class J2534EnableMessage : public J2534CommandMessage {
public:
	J2534EnableMessage(const bool enable)
		: J2534CommandMessage(J2534Command::Enable, { (uint8_t)enable }) {
	}
	bool getEnable(void) const { return (bool)argumentData[2]; }
    void setEnable(bool enable) { argumentData[2] = (uint8_t)enable; }
};

class J2534EnableFilteringMessage : public J2534CommandMessage {
public:
	J2534EnableFilteringMessage(const bool enable)
		: J2534CommandMessage(J2534Command::EnableFiltering, { (uint8_t)enable }) {
	}
	bool getEnable(void) const { return (bool)argumentData[2]; }
	void setEnable(bool enable) { argumentData[2] = (uint8_t)enable; }
};

class J2534SetupCanRxFilteringMessage : public J2534CommandMessage {
public:
	J2534SetupCanRxFilteringMessage(const J2534_RxCanFilter& filter)
		: J2534CommandMessage(J2534Command::SetupCanRxFilter) {
        setRxCanFilter(filter);
	}

	J2534_RxCanFilter getRxCanFilter(void) const;

	void setRxCanFilter(const J2534_RxCanFilter& filter);
};

class J2534SetupIso15765FlowControlMessage : public J2534CommandMessage
{
public:
	J2534SetupIso15765FlowControlMessage();

	uint16_t getIdx(void) const;
	void setIdx(uint16_t idx);
	uint16_t getCoreMiniId(void) const;
	void setCoreMiniId(uint16_t coreMiniId);
	uint8_t getPadding(void) const;
	void setPadding(uint8_t padding);
	uint32_t getId(void) const;
	void setId(uint32_t id);
	uint32_t getIdMask(void) const;
	void setIdMask(uint32_t idMask);
	uint32_t getFcId(void) const;
	void setFcId(uint32_t fcId);
	uint8_t getFlowControlExtendedAddress(void) const;
	void setFlowControlExtendedAddress(uint8_t addr);
	uint8_t getExtendedAddress(void) const;
	void setExtendedAddress(uint8_t addr);
	uint8_t getBlockSize(void) const;
	void setBlockSize(uint8_t blockSize);
	uint8_t getStMin(void) const;
	void setStMin(uint8_t stMin);
	uint16_t getCfTimeout(void) const;
	void setCfTimeout(uint16_t cfTimeout);
	uint32_t getFlags(void) const;
	void setFlags(uint32_t flags);

	bool getEnable(void) const;
	void setEnable(bool enable);
	bool getIs29BitEnabled(void) const;
	void setIs29BitEnabled(bool enable);
	bool getIsFc29BitEnabled(void) const;
	void setIsFc29BitEnabled(bool enable);
	bool getExtAddressEnabled(void) const;
	void setExtAddressEnabled(bool enable);
	bool getFcExtAddressEnabled(void) const;
	void setFcExtAddressEnabled(bool enable);
	bool getFlowControlTransmissionEnabled(void) const;
	void setFlowControlTransmissionEnabled(bool enable);
	bool getPaddingEnabled(void) const;
	void setPaddingEnabled(bool enable);
	bool getIsCanFd(void) const;
	void setIsCanFd(bool enable);
	bool getIsBrsEnabled(void) const;
	void setIsBrsEnabled(bool enable);

private:
	// The legacy wire format was the raw memory of the (now-removed) J2534_RxFlowControlRequest
	// struct copied into mArgumentData starting at offset 1 (overwriting the header pad byte).
	// The struct was compiled with `#pragma pack(2)` on MSVC little-endian. All multi-byte
	// scalars are therefore stored little-endian; the "flt" region reproduces the exact
	// byte pattern produced by the old MessageBitsCAN bitfield writes.
	//
	// Byte layout (mArgumentData indices):
	//   [0]      J2534 command header byte (set by base class)
	//   [1]	  padding
	//   [2..3]   idx (u16 LE)
	//   [4..5]   iCoreMiniID (u16 LE)
	//   [6]      padding (u8)
	//   [7]      structure pad byte (unused)
	//   [8..11]  id (u32 LE)
	//   [12..15] id_mask (u32 LE)
	//   [16..19] fc_id (u32 LE)
	//   [20]     flowControlExtendedAddress (u8)
	//   [21]     extendedAddress (u8)
	//   [22]     blockSize (u8)
	//   [23]     stMin (u8)
	//   [24..25] cf_timeout (u16 LE)
	//   [26..53] MessageFilterBytes flt (28 bytes) -- see RebuildFilterBytes for details
	//   [54..57] flags (u32 LE) -- see kFcFlag* below
	//
	// Total wire size: 58 bytes.
	static const size_t MessageSize = 58;
	static const size_t IdxOffset = 2;
	static const size_t CoreMiniOffset = 4;
	static const size_t PaddingOffset = 6;
	static const size_t IdOffset = 8;
	static const size_t IdMaskOffset = 12;
	static const size_t FcIdOffset = 16;
	static const size_t FcExtAddrOffset = 20;
	static const size_t ExtAddrOffset = 21;
	static const size_t BlockSizeOffset = 22;
	static const size_t StMinOffset = 23;
	static const size_t CfTimeoutOffset = 24;
	static const size_t FltOffset = 26; // 28 bytes
	static const size_t FltSize = 28;
	static const size_t FlagsOffset = 54;

	// Bit positions inside the u32 LE flags word (matches the legacy bitfield layout).
	static const uint32_t FlagEnable = 1u << 0;
	static const uint32_t FlagId29BitEnable = 1u << 1;
	static const uint32_t FlagFcId29BitEnable = 1u << 2;
	static const uint32_t FlagExtAddressEnable = 1u << 3;
	static const uint32_t FlagFcExtAddressEnable = 1u << 4;
	static const uint32_t FlagEnableFlowControlTransmit = 1u << 5;
	static const uint32_t FlagPaddingEnable = 1u << 6;
	static const uint32_t FlagIsCanFd = 1u << 7;
	static const uint32_t FlagIsBrsEnabled = 1u << 8;

	// Rebuilds the filter bytes region of mArgumentData based on the current id / id_mask /
	// id_29_bit_enable / ext_address_enable / extendedAddress state. Preserves the legacy
	// (pre-existing) wire format bit-for-bit.
	void rebuildFilterBytes(void);

	void setFlagBit(uint32_t mask, bool enable);

	bool getFlagBit(uint32_t mask) const;
};

class Iso15765TxSetupMessage : public J2534CommandMessage {
public:
	Iso15765TxSetupMessage()
		: J2534CommandMessage(J2534Command::SetupIso15765TxMessageProperties) {
		argumentData.resize(36);
	}

	uint16_t getIdx(void) const;
	void setIdx(uint16_t idx);
	uint16_t getCoreMiniId(void) const;
	void setCoreMiniId(uint16_t coreMiniId);
	uint32_t getMessageLength(void) const;
	void setMessageLength(uint32_t messageLength);
	uint8_t getPadding(void) const;
	void setPadding(uint8_t padding);
	uint8_t getTxDl(void) const;
	void setTxDl(uint8_t txDl);
	uint32_t getId(void) const;
	void setId(uint32_t id);
	uint32_t getFcId(void) const;
	void setFcId(uint32_t fcId);
	uint32_t getFcIdMask(void) const;
	void setFcIdMask(uint32_t fcIdMask);
	uint8_t getFlowControlExtendedAddress(void) const;
	void setFlowControlExtendedAddress(uint8_t addr);
	uint8_t getExtendedAddress(void) const;
	void setExtendedAddress(uint8_t addr);
	uint16_t getFsTimeout(void) const;
	void setFsTimeout(uint16_t timeout);
	uint16_t getFsWait(void) const;
	void setFsWait(uint16_t wait);
	uint32_t getFlags(void) const;
	void setFlags(uint32_t flags);
	uint8_t getStMin(void) const;
	void setStMin(uint8_t stMin);
	uint8_t getBlockSize(void) const;
	void setBlockSize(uint8_t blockSize);

	bool getIs29BitEnabled(void) const;
	void setIs29BitEnabled(bool enable);
	bool getIsFc29BitEnabled(void) const;
	void setIsFc29BitEnabled(bool enable);
	bool getExtAddressEnabled(void) const;
	void setExtAddressEnabled(bool enable);
	bool getFcExtAddressEnabled(void) const;
	void setFcExtAddressEnabled(bool enable);
	bool getOverrideStMin(void) const;
	void setOverrideStMin(bool enable);
	bool getOverrideBlockSize(void) const;
	void setOverrideBlockSize(bool enable);
	bool getPaddingEnabled(void) const;
	void setPaddingEnabled(bool enable);
	bool getIsCanFd(void) const;
	void setIsCanFd(bool enable);
	bool getIsBrsEnabled(void) const;
	void setIsBrsEnabled(bool enable);

private:
	// Matches J2534_Iso15TxRequest (#pragma pack(2), little-endian) copied after the
	// 2-byte J2534 command header, as sent by vspy3 J2534_Transaction.
	//   [0..1]   J2534 command header (managed by base class)
	//   [2..3]   idx           (u16 LE)
	//   [4..5]   iCoreMiniID   (u16 LE)
	//   [6..9]   messageLength (u32 LE)
	//   [10]     padding
	//   [11]     tx_dl
	//   [12..15] id            (u32 LE)
	//   [16..19] fc_id         (u32 LE)
	//   [20..23] fc_id_mask    (u32 LE)
	//   [24]     flowControlExtendedAddress
	//   [25]     extendedAddress
	//   [26..27] fs_timeout    (u16 LE)
	//   [28..29] fs_wait       (u16 LE)
	//   [30..33] flags         (u32 LE)
	//   [34]     stMin
	//   [35]     blockSize

	static const size_t SetupIdxOffset = 2;
	static const size_t SetupCoreMiniOffset = 4;
	static const size_t SetupMsgLenOffset = 6;
	static const size_t SetupPaddingOffset = 10;
	static const size_t SetupTxDlOffset = 11;
	static const size_t SetupIdOffset = 12;
	static const size_t SetupFcIdOffset = 16;
	static const size_t SetupFcIdMaskOffset = 20;
	static const size_t SetupFcExtAddrOffset = 24;
	static const size_t SetupExtAddrOffset = 25;
	static const size_t SetupFsTimeoutOffset = 26;
	static const size_t SetupFsWaitOffset = 28;
	static const size_t SetupFlagsOffset = 30;
	static const size_t SetupStMinOffset = 34;
	static const size_t SetupBlockSizeOffset = 35;

	// Flag bit positions within the 32-bit flags word.
	static const uint32_t FlagId29BitEnable = 1u << 0;
	static const uint32_t FlagFcId29BitEnable = 1u << 1;
	static const uint32_t FlagExtAddressEnable = 1u << 2;
	static const uint32_t FlagFcExtAddressEnable = 1u << 3;
	static const uint32_t FlagOverrideStMin = 1u << 4;
	static const uint32_t FlagOverrideBlockSize = 1u << 5;
	static const uint32_t FlagPaddingEnable = 1u << 6;
	static const uint32_t FlagIsCanFd = 1u << 7;
	static const uint32_t FlagIsBrsEnabled = 1u << 8;

	bool getFlagBit(uint32_t mask) const;
	void setFlagBit(uint32_t mask, bool enable);
};

class Iso15765TxDataMessage : public J2534CommandMessage {
public:
	static constexpr uint16_t MaxDataLength = 500;

	Iso15765TxDataMessage()
		: J2534CommandMessage(J2534Command::SetupIso15765TxMessageDataBytes) {
		argumentData.resize(10);
	}

	uint16_t getIdx(void) const;
	void setIdx(uint16_t idx);
	uint32_t getOffset(void) const;
	void setOffset(uint32_t offset);
	uint16_t getLen(void) const;
	const uint8_t* getData(void) const;
	void setData(const uint8_t* sourceData, uint16_t len);

private:
	// Matches J2534_Iso15TxDataRequest (#pragma pack(2), little-endian) after the
	// 2-byte J2534 command header.
	//   [0..1]  J2534 command header (managed by base class)
	//   [2..3]  idx    (u16 LE)
	//   [4..7]  offset (u32 LE)
	//   [8..9]  len    (u16 LE)
	//   [10..]  data
	static const size_t DataIdxOffset = 2;
	static const size_t DataOffsetOffset = 4;
	static const size_t DataLenOffset = 8;
	static const size_t DataPayloadOffset = 10;
};

class Iso15765MessageArgs {
public:
	struct ArbId {
		bool Is29Bit;
		uint32_t Id;

        ArbId(uint32_t id = 0, bool is29Bit = false)
            : Is29Bit(is29Bit)
            , Id(id) {
        }
	};

	Iso15765MessageArgs(void)
		: networkId(0)
        , txIndex(0)
        , arbId({ false, 0 })
        , flowControlArbId({ false, 0 })
        , flowControlArbIdMask(0)
        , isCanFd(false)
        , isBrsEnabled(false)
        , tx_DL(8)
        , fs_Timeout(0)
        , fs_WaitTimeout(0)
        , cf_Timeout(0)
        , isFlowControlEnabled(false) {
	}

	uint16_t getNetworkId(void) const { return networkId; }
	void setNetworkId(uint16_t netId) { networkId = netId; }
	uint8_t getTxIndex(void) const { return txIndex; }
	void setTxIndex(uint8_t idx) { txIndex = idx; }

	const ArbId& getArbId(void) const { return arbId; }
	void setArbId(const ArbId& id) { arbId = id; }
	const ArbId& getFlowControlArbId(void) const { return flowControlArbId; }
	void setFlowControlArbId(const ArbId& id) { flowControlArbId = id; }
	uint32_t getFlowControlArbIdMask(void) const { return flowControlArbIdMask; }
	void setFlowControlArbIdMask(uint32_t mask) { flowControlArbIdMask = mask; }
	bool getIsCanFd(void) const { return isCanFd; }
	void setIsCanFd(bool value) { isCanFd = value; }
	bool getIsBrsEnabled(void) const { return isBrsEnabled; }
	void setIsBrsEnabled(bool value) { isBrsEnabled = value; }

	uint8_t getTxDl(void) const { return tx_DL; }
	void setTxDl(uint8_t txDl) { tx_DL = txDl; }
	std::optional<uint8_t> getStMin(void) const { return stMin; }
	void setStMin(std::optional<uint8_t> v) { stMin = v; }
	std::optional<uint8_t> getBlockSize(void) const { return blockSize; }
	void setBlockSize(std::optional<uint8_t> b) { blockSize = b; }
	std::optional<uint8_t> getFlowControlExtendedAddress(void) const { return flowControlExtendedAddress; }
	void setFlowControlExtendedAddress(std::optional<uint8_t> fcExtAddr) { flowControlExtendedAddress = fcExtAddr; }
	std::optional<uint8_t> getExtendedAddress(void) const { return extendedAddress; }
	void setExtendedAddress(std::optional<uint8_t> extAddr) { extendedAddress = extAddr; }
	std::optional<uint8_t> getPaddingValue(void) const { return paddingValue; }
	void setPaddingValue(std::optional<uint8_t> v) { paddingValue = v; }

	uint16_t getFsTimeout(void) const { return fs_Timeout; }
	void setFsTimeout(uint16_t timeout) { fs_Timeout = timeout; }
	uint16_t getFsWaitTimeout(void) const { return fs_WaitTimeout; }
	void setFsWaitTimeout(uint16_t timeout) { fs_WaitTimeout = timeout; }
	uint16_t getCfTimeout(void) const { return cf_Timeout; }
	void setCfTimeout(uint16_t timeout) { cf_Timeout = timeout; }

	const std::vector<uint8_t>& getData(void) const { return data; }
	void setData(std::vector<uint8_t>&& d) { data = std::move(d); }

	bool getIsFlowControlEnabled(void) const { return isFlowControlEnabled; }
	void setIsFlowControlEnabled(bool enabled) { isFlowControlEnabled = enabled; }

protected:

	uint16_t networkId;	// The netid of the message (determines which network to transmit on)
	uint8_t txIndex;		// identifier for this transmit message

	ArbId arbId;					// arbId of transmitted frames (CAN id to transmit to)
	ArbId flowControlArbId;		// flow control arb id filter value (response id from receiver)
	uint32_t flowControlArbIdMask; // The flow control arb id filter mask (response id from receiver)
	bool isCanFd;
	bool isBrsEnabled;

	uint8_t tx_DL;										// Maximum CAN(FD) protocol length for transmitted frames (Valid values are 8, 12, 16, 20, 24, 32, 48, 64)
	std::optional<uint8_t> stMin;						// Overrides the stMin that the receiver reports (Set to J2534's STMIN_TX if <= 0xFF)
	std::optional<uint8_t> blockSize;					// Overrides the block size that the receiver reports (Set to J2534's BS_TX if <= 0xFF)
	std::optional<uint8_t> flowControlExtendedAddress; // Expected Extended Address byte of response from receiver
	std::optional<uint8_t> extendedAddress;			// Extended Address byte of transmitter
	std::optional<uint8_t> paddingValue;				// The padding byte to use to fill the unused portion of transmitted CAN frames (single frame, first frame, consecutive frame)

	uint16_t fs_Timeout;		// max timeout (ms) for waiting on flow control response (Set this to N_BS_MAX's value if J2534)
	uint16_t fs_WaitTimeout;	// max timeout (ms) for waiting on flow control response after receiving flow control with flow status set to WAIT (Set this to N_BS_MAX's value if J2534)
	uint16_t cf_Timeout;		// max timeout (ms) for waiting on consecutive frame (Set this to N_CR_MAX's value in J2534)

	std::vector<uint8_t> data;

	bool isFlowControlEnabled;	// Enables Flow Control frame transmission (Rx message; from neoVI to ECU)
};


}

#endif // __cplusplus

#endif