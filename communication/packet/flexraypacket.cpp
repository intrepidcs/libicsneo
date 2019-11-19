#include "icsneo/communication/packet/flexraypacket.h"

using namespace icsneo;

std::shared_ptr<FlexRayMessage> HardwareFlexRayPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	const HardwareFlexRayPacket* data = (const HardwareFlexRayPacket*)bytestream.data();
	if(!data->timestamp.IsExtended) // We can only process "extended" frames here
		return nullptr;

	auto msg = std::make_shared<FlexRayMessage>();

	// This timestamp is raw off the device (in timestampResolution increments)
	// Decoder will fix as it has information about the timestampResolution increments
	msg->timestamp = data->timestamp.TS;

	// Always get the frame length, even for a symbol
	msg->framelen = data->frame_length_12_5ns * 12.5e-9;

	msg->channel = data->statusBits.bits.chb ? icsneo::FlexRay::Channel::B : icsneo::FlexRay::Channel::A;

	if(data->tss_length_12_5ns == 0xffff) {// Flag value meaning this is a symbol
		// These values are only for 10Mbit
		// That's the only baudrate supported for now
		if (data->frame_length_12_5ns > 480)
			msg->symbol = FlexRay::Symbol::Wakeup;
		else if (data->frame_length_12_5ns > 264)
			msg->symbol = FlexRay::Symbol::CAS;
		else
			msg->symbol = FlexRay::Symbol::Unknown;
	} else {
		msg->tsslen = data->tss_length_12_5ns * 12.5e-9;

		if(data->statusBits.bits.bytesRxed >= 5) {
			if(data->statusBits.bits.hcrc_error)
				msg->headerCRCStatus = FlexRay::CRCStatus::Error;
		} else {
			msg->headerCRCStatus = FlexRay::CRCStatus::NoCRC;
		}

		uint32_t numBytes = data->payload_len * 2;
		if(int64_t(numBytes) >= int64_t(data->Length) - 4) {
			if(data->statusBits.bits.fcrc_error)
				msg->crcStatus = FlexRay::CRCStatus::Error;
		} else {
			msg->crcStatus = FlexRay::CRCStatus::NoCRC;
		}

		if(data->statusBits.bits.bytesRxed >= 5) { // Received entire header
			msg->headerCRC = (data->hdr_crc_10 << 10) | data->hdr_crc_9_0;
			if(msg->headerCRCStatus != FlexRay::CRCStatus::Error) {
				msg->reserved0was1 = data->reserved_0;
				msg->payloadPreamble = data->payload_preamble;
				msg->nullFrame = !data->null_frame;
				msg->sync = data->sync;
				msg->startup = data->startup;
				msg->slotid = data->slotid;
				msg->cycle = data->cycle;
				msg->dynamic = data->statusBits.bits.dynamic;
				if(int64_t(numBytes) != int64_t(data->Length) - 4) {
					// This is an error, probably need to flag it
				} else {
					const uint8_t* dataStart = (const uint8_t*)(data) - 4 + sizeof(HardwareFlexRayPacket);
					msg->data = std::vector<uint8_t>(dataStart, dataStart + numBytes);
				}
			}
		}
	}
	return msg;
}

bool HardwareFlexRayPacket::EncodeFromMessage(const FlexRayMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report) {
	(void)message;
	(void)bytestream;
	(void)report;
	return false;
}
