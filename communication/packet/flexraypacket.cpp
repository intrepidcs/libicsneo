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

	if(data->tss_length_12_5ns == 0xffff) {// Flag value meaning this is a symbol
		msg->symbol = FlexRay::Symbol::Unknown; // We can't know the symbol yet because this will depend on the baudrate
		// Eventually we'll have to get this from the framelen
	} else {
		msg->tsslen = data->tss_length_12_5ns * 12.5e-9;
		msg->channelB = data->statusBits.bits.chb;
		
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
				msg->nullFrame = data->null_frame;
				msg->sync = data->sync;
				msg->startup = data->startup;
				msg->id = data->id;
				if(int64_t(numBytes) != int64_t(data->Length) - 4) {
					
				} else {
					// This is an error, probably need to flag it
				}
			}
		}
	}
	return msg;
}

bool HardwareFlexRayPacket::EncodeFromMessage(const FlexRayMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report) {
	return false;
}