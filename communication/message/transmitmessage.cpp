#include "icsneo/communication/message/transmitmessage.h"

// packet defs
#include "icsneo/communication/packet/ethernetpacket.h"
#include "icsneo/communication/packet/canpacket.h"
#include "icsneo/communication/packet/linpacket.h"

using namespace icsneo;

// copied.. TODO
static std::optional<uint8_t> CAN_LengthToDLC(size_t dataLength, bool fd) {
	if(dataLength <= 8)
		return uint8_t(dataLength);

	if(fd) {
		if(dataLength <= 12)
			return uint8_t(0x9);
		if(dataLength <= 16)
			return uint8_t(0xA);
		if(dataLength <= 20)
			return uint8_t(0xB);
		if(dataLength <= 24)
			return uint8_t(0xC);
		if(dataLength <= 32)
			return uint8_t(0xD);
		if(dataLength <= 48)
			return uint8_t(0xE);
		if(dataLength <= 64)
			return uint8_t(0xF);
	}

	return std::nullopt;
}

static std::vector<uint8_t> EncodeFromMessageEthernet(std::shared_ptr<Frame> frame, const device_eventhandler_t& report) {
	auto ethmsg = std::dynamic_pointer_cast<EthernetMessage>(frame);
	if(!ethmsg) {
		report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
		return {};
	}
	std::vector<uint8_t> encoded;
	size_t messageLen = ethmsg->data.size();
	encoded.resize(sizeof(TransmitMessage) + messageLen);

	TransmitMessage* const msg = (TransmitMessage*)encoded.data();
	HardwareEthernetPacket* const ethpacket = (HardwareEthernetPacket*)(msg->commonHeader);
	uint8_t* const payload = encoded.data() + sizeof(TransmitMessage);

	ethpacket->header.ENABLE_PADDING = ethmsg->noPadding ? 0 : 1;
	ethpacket->header.FCS_OVERRIDE = ethmsg->fcs ? 1 : 0;
	ethpacket->eid.txlen = static_cast<uint16_t>(messageLen);
	ethpacket->Length = static_cast<uint16_t>(messageLen);
	ethpacket->stats = ethmsg->description;
	ethpacket->NetworkID = static_cast<uint16_t>(ethmsg->network.getNetID());
	std::copy(ethmsg->data.begin(), ethmsg->data.end(), payload);
	return encoded;
}

static std::vector<uint8_t> EncodeFromMessageCAN(std::shared_ptr<Frame> frame, const device_eventhandler_t& report) {
	auto canmsg = std::dynamic_pointer_cast<CANMessage>(frame);
	if(!canmsg) {
		report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
		return {};
	}
	if(canmsg->isCANFD && canmsg->isRemote) {
		report(APIEvent::Type::RTRNotSupported, APIEvent::Severity::Error);
		return {}; // RTR frames can not be used with CAN FD
	}
	std::vector<uint8_t> encoded;
	size_t messageLen = canmsg->data.size();
	size_t extraLen = 0;
	if(messageLen > 8) {
		extraLen = messageLen - 8;
	}
	encoded.resize(sizeof(TransmitMessage) + extraLen);

	TransmitMessage* const msg = (TransmitMessage*)encoded.data();
	HardwareCANPacket* const canpacket = (HardwareCANPacket*)(msg->commonHeader);
	uint8_t* const extra_payload = encoded.data() + sizeof(TransmitMessage);

	const size_t dataSize = canmsg->data.size();
	std::optional<uint8_t> dlc = CAN_LengthToDLC(dataSize, canmsg->isCANFD);
	if(!dlc.has_value()) {
		report(APIEvent::Type::MessageMaxLengthExceeded, APIEvent::Severity::Error);
		return {}; // Too much data for the protocol
	}
	// arb id
	if(canmsg->isExtended) {
		canpacket->header.IDE = 1;
		canpacket->header.SID = (canmsg->arbid >> 18) & 0x7FF;
		canpacket->eid.EID = (canmsg->arbid >> 6) & 0xfff;
		canpacket->dlc.EID2 = canmsg->arbid & 0x3f;
	} else {
		canpacket->header.IDE = 0;
		canpacket->header.SID = canmsg->arbid & 0x7FF;
	}

	// DLC
	canpacket->dlc.DLC = dlc.value();

	// FDF/BRS or remote frames
	if(canmsg->isCANFD) {
		canpacket->header.EDL = 1;
		canpacket->header.BRS = canmsg->baudrateSwitch ? 1 : 0;
		canpacket->header.ESI = canmsg->errorStateIndicator ? 1 : 0;
		canpacket->dlc.RTR = 0;

	} else {
		canpacket->header.EDL = 0;
		canpacket->header.BRS = 0;
		canpacket->header.ESI = 0;
		canpacket->dlc.RTR = canmsg->isRemote ? 1 : 0;
	}
	// network
	canpacket->NetworkID = static_cast<uint16_t>(canmsg->network.getNetID());
	canpacket->Length = static_cast<uint16_t>(extraLen);
	// description id
	canpacket->stats = canmsg->description;
	// first 8 bytes
	std::copy(canmsg->data.begin(), canmsg->data.begin() + (messageLen > 8 ? 8 : messageLen), canpacket->data);
	// extra bytes
	if(extraLen > 0) {
		// copy extra data after the can packet
		std::copy(canmsg->data.begin() + 8, canmsg->data.end(), extra_payload);
	}
	return encoded;
}

static std::vector<uint8_t> EncodeFromMessageLIN(std::shared_ptr<Frame> /* frame */, const device_eventhandler_t& report) {
	// TODO
	report(APIEvent::Type::UnsupportedTXNetwork, APIEvent::Severity::Error);
	return {};
}

std::vector<uint8_t> TransmitMessage::EncodeFromMessage(std::shared_ptr<Frame> frame, uint32_t client_id, const device_eventhandler_t& report) {
	std::vector<uint8_t> result;
	switch(frame->network.getType()) {
		case Network::Type::Ethernet:
		case Network::Type::AutomotiveEthernet:
			result = EncodeFromMessageEthernet(frame, report);
			break;
		case Network::Type::Internal:
		case Network::Type::CAN:
			result = EncodeFromMessageCAN(frame, report);
			break;
		case Network::Type::LIN:
			result = EncodeFromMessageLIN(frame, report);
			break;
		default:
			report(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::Error);
			return result;
	}
	// common fields
	TransmitMessage* const msg = (TransmitMessage*)result.data();
	msg->options.clientId = client_id;
	msg->options.networkId = static_cast<uint32_t>(frame->network.getNetID());
	msg->options.reserved[0] = 0;
	msg->options.reserved[1] = 0;
	msg->options.reserved[2] = 0;

	return result;
}