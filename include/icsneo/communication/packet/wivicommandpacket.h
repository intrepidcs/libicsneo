#ifndef __WIVICOMMANDPACKET_H_
#define __WIVICOMMANDPACKET_H_

#ifdef __cplusplus

#include "icsneo/communication/packet.h"
#include <cstdint>
#include <vector>
#include <memory>

#pragma pack(push,2)
#pragma warning(push)
#pragma warning(disable: 4201) // nonstandard extension used: nameless struct/union
#pragma warning(disable: 4200) // nonstandard extension used: zero-sized array in struct/union

namespace icsneo {

union CoreMiniFixedPointValue {
	CoreMiniFixedPointValue() {}
	CoreMiniFixedPointValue(int32_t integralValue) { ValueInt32 = integralValue; }

	struct {
		union {
			uint32_t ValueFractionPart;
			struct {
				int16_t ValueInt16FractionLSB;
				int16_t ValueInt16FractionMSB;
			};
		};
		union {
			int32_t ValueInt32;
			struct {
				int16_t ValueInt16;
				int16_t ValueInt16PartMSB;
			};
			struct{
				uint8_t ValueInt8;
				uint8_t ValueInt8PartMSB;
				int16_t ValueInt8Part16MSB;
			};
		};
	};
	int64_t ValueLarge;
};

namespace WiVI {

class ResponseMessage; // Forward declaration to avoid cyclic includes

enum class Command : uint16_t {
	GetAll = 0x0010,
	ClearUploads = 0x0011,
	SetSignal = 0x0012,
	GetSignal = 0x0013,
	Result = 0x0014,
	GetPhysicalSignal = 0x0015,
};

enum class SignalType : uint16_t { // enumCoreMiniValueMiscValueType
	WokeUpOnSMS = 0x0067,
	AvailableDiskSpaceKB = 0x0069,
	ManualTrigger = 0x006c,
	SleepRequest = 0x006d,
	ConnectionTimeout = 0x006e,
	TimeSinceLastMessageMs = 0x006f,
	UploadsPending = 0x0077,
};

struct Upload {
	uint32_t startSector;
	uint32_t endSector;
	struct {
		uint16_t pending : 1;
		uint16_t started : 1;
		uint16_t reserved : 14;
	} flags;
};

struct CaptureInfo {
	uint16_t captureBlockIndex; // What capture block is this for
	struct {
		uint16_t isPrePost : 1;
		uint16_t isPreTime : 1; // Only valid if the capture block's bPrePostExtract is set
		uint16_t uploadOverCellular : 1;
		uint16_t uploadOverWiFi : 1;
		uint16_t uploadStackSize : 3; // 0 => size of 1
		uint16_t uploadOverflow : 1;
		uint16_t uploadPriority : 4;
		uint16_t reserved : 4;
	} flags;
	// Only valid if the capture block's bPrePostExtract is set
	uint32_t preTriggerSize;
	Upload uploadStage;
	Upload uploadStack[2];
};

struct CommandPacket {
	static std::shared_ptr<WiVI::ResponseMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);

	struct Header {
		WiVI::Command cmd;
		uint16_t length;
	};

	struct Result {
		Header header;
		WiVI::Command responseTo;
		uint16_t result;
	};

	struct GetSignal {
		static std::vector<uint8_t> Encode(WiVI::SignalType type);

		Header header;
		WiVI::SignalType type;
	};

	struct SetSignal {
		static std::vector<uint8_t> Encode(WiVI::SignalType type, CoreMiniFixedPointValue value);

		Header header;
		WiVI::SignalType type;
		CoreMiniFixedPointValue value;
	};
};

} // namespace WiVI

} // namespace icsneo

#pragma warning(pop)
#pragma pack(pop)

#endif // __cplusplus

#endif