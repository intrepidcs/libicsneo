#include "icsneo/communication/packet/iso9141packet.h"
#include <algorithm>

using namespace icsneo;

std::shared_ptr<ISO9141Message> HardwareISO9141Packet::Decoder::decodeToMessage(const std::vector<uint8_t>& bytestream) {
	const HardwareISO9141Packet* data = (const HardwareISO9141Packet*)bytestream.data();

	if(!mMsg) {
		mMsg = std::make_shared<ISO9141Message>();
		mGotPackets = 0;
	}

	mGotPackets++;

	const bool morePacketsComing = data->c3.frm == 0;
	const uint8_t bytesInCurrentMessage = data->c3.len;
	if(mMsg->data.size() + bytesInCurrentMessage > 500) {
		mMsg.reset();
		return std::shared_ptr<ISO9141Message>();
	}

	// This timestamp is raw off the device (in timestampResolution increments)
	// Decoder will fix as it has information about the timestampResolution increments
	mMsg->timestamp = data->timestamp.TS;

	auto* dataStart = data->data;
	if(mGotPackets == 1) {
		// Header
		if(bytesInCurrentMessage < 3) {
			mMsg.reset(); // We don't have the header for some reason
			return std::shared_ptr<ISO9141Message>();
		}

		std::copy(data->data, data->data + 3, mMsg->header.begin());
		dataStart += 3;
	}

	// Data
	mMsg->data.insert(mMsg->data.end(), dataStart, data->data + (bytesInCurrentMessage > 8 ? 8 : bytesInCurrentMessage));
	if(bytesInCurrentMessage > 8)
		mMsg->data.push_back(data->c1.d8);
	if(bytesInCurrentMessage > 9)
		mMsg->data.push_back(data->c2.d9);
	if(bytesInCurrentMessage > 10)
		mMsg->data.push_back(data->c2.d10);
	if(bytesInCurrentMessage > 11)
		mMsg->data.push_back(data->c3.d11);

	if(morePacketsComing)
		return std::shared_ptr<ISO9141Message>();

	mMsg->transmitted = data->c1.tx;
	mMsg->isInit = data->c3.init;
	mMsg->framingError = data->c1.options & 0x1;
	mMsg->overflowError = data->c1.options & 0x2;
	mMsg->parityError = data->c1.options & 0x4;
	mMsg->rxTimeoutError = data->c1.options & 0x8;
	mMsg->description = data->stats;

	auto ret = mMsg;
	mMsg.reset();
	return ret;
}