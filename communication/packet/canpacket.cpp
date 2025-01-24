#include "icsneo/communication/packet/canpacket.h"
#include "icsneo/communication/message/canerrormessage.h"

using namespace icsneo;

std::optional<uint8_t> icsneo::CAN_DLCToLength(uint8_t length, bool fd) {
	if (length <= 8)
		return length;

	if (fd) {
		switch(length) {
			case 0x9:
				return uint8_t(12);
			case 0xa:
				return uint8_t(16);
			case 0xb:
				return uint8_t(20);
			case 0xc:
				return uint8_t(24);
			case 0xd:
				return uint8_t(32);
			case 0xe:
				return uint8_t(48);
			case 0xf:
				return uint8_t(64);
		}
	}

	return std::nullopt;
}

std::optional<uint8_t> icsneo::CAN_LengthToDLC(size_t dataLength, bool fd)
{
	if (dataLength <= 8)
		return uint8_t(dataLength);

	if (fd) {
		if (dataLength <= 12)
			return uint8_t(0x9);
		else if (dataLength <= 16)
			return uint8_t(0xA);
		else if (dataLength <= 20)
			return uint8_t(0xB);
		else if (dataLength <= 24)
			return uint8_t(0xC);
		else if (dataLength <= 32)
			return uint8_t(0xD);
		else if (dataLength <= 48)
			return uint8_t(0xE);
		else if (dataLength <= 64)
			return uint8_t(0xF);
	}

	return std::nullopt;
}
std::shared_ptr<Message> HardwareCANPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	const HardwareCANPacket* data = (const HardwareCANPacket*)bytestream.data();
	const HardwareCANErrorPacket* errPacket = (const HardwareCANErrorPacket*)bytestream.data();
	if(errPacket->ERROR_INDICATOR) {
		auto msg = std::make_shared<CANErrorMessage>();
		msg->receiveErrorCount = errPacket->REC;
		msg->transmitErrorCount = errPacket->TEC;
		msg->errorWarn = HardwareCANErrorPacket::GetErrorWarn(errPacket->flags);
		msg->errorPassive = HardwareCANErrorPacket::GetErrorPassive(errPacket->flags);
		msg->busOff = HardwareCANErrorPacket::GetBusOff(errPacket->flags);
		msg->errorCode = (CANErrorCode)errPacket->error_code;
		msg->dataErrorCode = (CANErrorCode)errPacket->brs_data_error_code;
		// This timestamp is raw off the device (in timestampResolution increments)
		// Decoder will fix as it has information about the timestampResolution increments
		msg->timestamp = data->timestamp.TS;

		return msg;

	} else { // CAN Frame
		auto msg = std::make_shared<CANMessage>();

		// Arb ID
		if(data->header.IDE) { // Extended 29-bit ID
			msg->arbid = (data->header.SID & 0x7ff) << 18;
			msg->arbid |= (data->eid.EID & 0xfff) << 6;
			msg->arbid |= (data->dlc.EID2 & 0x3f);
			msg->isExtended = true;
		} else { // Standard 11-bit ID
			msg->arbid = data->header.SID;
		}

		// This timestamp is raw off the device (in timestampResolution increments)
		// Decoder will fix as it has information about the timestampResolution increments
		msg->timestamp = data->timestamp.TS;

		// DLC
		uint8_t length = data->dlc.DLC;
		msg->dlcOnWire = length; // This will hold the real DLC on wire 0x0 - 0xF
		if(data->header.EDL && data->timestamp.IsExtended) { // CAN FD
			msg->isCANFD = true;
			msg->baudrateSwitch = data->header.BRS; // CAN FD Baudrate Switch
			msg->errorStateIndicator = data->header.ESI;
			const std::optional<uint8_t> lenFromDLC = CAN_DLCToLength(length, true);
			if (lenFromDLC)
				length = *lenFromDLC;
		} else if(length > 8) { // This is a standard CAN frame with a length of more than 8
			// Yes, this is possible. On the wire, the length field is a nibble, and we do want to return an accurate value
			// We don't want to overread our buffer, though, so make sure we cap the length
			length = 8;
		}

		// Data
		// The first 8 bytes are always in the standard place
		if((data->dlc.RTR && data->header.IDE) || (!data->header.IDE && data->header.SRR)) { // Remote Request Frame
			msg->data.resize(length); // This data will be all zeros, but the length will be set
			msg->isRemote = true;
		} else {
			msg->data.reserve(length);
			msg->data.insert(msg->data.end(), data->data, data->data + (length > 8 ? 8 : length));
			if(length > 8) { // If there are more than 8 bytes, they come at the end of the message
				// Messages with extra data are formatted as message, then uint16_t netid, then uint16_t length, then extra data
				const auto extraDataStart = bytestream.begin() + sizeof(HardwareCANPacket) + 2 + 2;
				msg->data.insert(msg->data.end(), extraDataStart, extraDataStart + (length - 8));
			}
		}

		msg->transmitted = data->eid.TXMSG;
		msg->error = data->eid.TXAborted || data->eid.TXError || data->eid.TXLostArb;
		msg->description = data->stats;

		return msg;
	}
}

bool HardwareCANPacket::EncodeFromMessage(const CANMessage& message, std::vector<uint8_t>& result, const device_eventhandler_t& report) {
	if(message.isCANFD && message.isRemote) {
		report(APIEvent::Type::RTRNotSupported, APIEvent::Severity::Error);
		return false; // RTR frames can not be used with CAN FD
	}

	const size_t dataSize = message.data.size();
	std::optional<uint8_t> dlc = CAN_LengthToDLC(dataSize, message.isCANFD);
	if (!dlc.has_value()) {
		report(APIEvent::Type::MessageMaxLengthExceeded, APIEvent::Severity::Error);
		return false; // Too much data for the protocol
	}

	if (message.dlcOnWire != 0) {
		if(message.dlcOnWire > 0xf) {
			// The DLC is only a nibble
			// It is actually possible to transmit a standard CAN frame with a DLC > 8
			// While it is invalid, most controllers will still pass along the received
			// frame and 8 bytes of data, so it may be desirable to test behavior with
			// these frames. We let you do it if you set `message.dlcOnWire` for transmit.
			report(APIEvent::Type::MessageMaxLengthExceeded, APIEvent::Severity::Error);
			return false;
		}

		if (message.dlcOnWire < *dlc) {
			report(APIEvent::Type::MessageMaxLengthExceeded, APIEvent::Severity::Error);
			return false;
		}

		if (message.dlcOnWire > *dlc)
			dlc = message.dlcOnWire;
	}

	// The only way this fails is if we're transmitting a DLC > 8 on standard CAN
	const uint8_t paddedLength = CAN_DLCToLength(*dlc, message.isCANFD).value_or(8);
	const uint8_t paddingBytes = uint8_t(paddedLength - dataSize);

	// Pre-allocate as much memory as we will possibly need for speed
	result.reserve(16 + dataSize + paddingBytes);

	result.push_back(0 /* byte count here later */ << 4 | (uint8_t(message.network.getNetID()) & 0xF));

	// Two bytes for Description ID, big endian
	result.insert(result.end(), { uint8_t(message.description >> 8), uint8_t(message.description) });

	// Next 2-4 bytes are ArbID
	if(message.isExtended) {
		if(message.arbid >= 0x20000000) {// Extended messages use 29-bit arb IDs
			report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
			return false;
		}

		result.insert(result.end(), {
			(uint8_t)(message.arbid >> 21),
			(uint8_t)(((((message.arbid & 0x001C0000) >> 13) & 0xFF) + (((message.arbid & 0x00030000) >> 16) & 0xFF)) | 8),
			(uint8_t)(message.arbid >> 8),
			(uint8_t)message.arbid
		});
	} else {
		if(message.arbid >= 0x800) {// Standard messages use 11-bit arb IDs
			report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
			return false;
		}

		result.insert(result.end(), {
			(uint8_t)(message.arbid >> 3),
			(uint8_t)((message.arbid & 0x7) << 5)
		});
	}

	// Status and DLC bits
	if(message.isCANFD) {
		result.push_back(0x0F); // FD Frame
		uint8_t fdStatusByte = *dlc;
		if(message.baudrateSwitch)
			fdStatusByte |= 0x80; // BRS status bit
		// The firmware does not yet support transmitting ESI
		result.push_back(fdStatusByte);
	} else {
		// TODO Support high voltage wakeup, bitwise-or in 0x8 here to enable
		uint8_t statusNibble = message.isRemote ? 0x4 : 0x0;
		result.push_back((statusNibble << 4) | *dlc);
	}

	// Now finally the payload
	result.insert(result.end(), message.data.begin(), message.data.end());
	result.resize(result.size() + paddingBytes);

	// Fill in the length byte from earlier
	result[0] |= result.size() << 4;

	return true;
}