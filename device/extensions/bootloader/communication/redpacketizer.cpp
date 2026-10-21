#include "icsneo/device/extensions/bootloader/communication/redpacketizer.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"

#include <algorithm>

namespace icsneo {

namespace Bootloader {

bool REDPacketizer::input(icsneo::RingBuffer& bytes) {
	while(bytes.size() > 0) {
		switch(state) {
		case HEADER:
			if(bytes[0] == Magic) {
				working.emplace_back();
				state = CMD;
			} else {
				Log::Trace(__func__, "Got bootloader unknown byte: " + std::to_string(bytes[0]));
			}
			bytes.pop_front();
			break;
		case CMD:
			working.back().resp = Response(bytes[0]);
			if(working.back().resp != Response::Accepted && working.back().resp != Response::Rejected) {
				Log::Trace(__func__, "Got bootloader unknown response: " + std::to_string(bytes[0]));
				working.pop_back();
				state = HEADER;
			} else {
				state = LENGTH_LOWER;
			}
			bytes.pop_front();
			break;
		case LENGTH_LOWER:
			working.back().len = bytes[0];
			state = LENGTH_UPPER;
			bytes.pop_front();
			break;
		case LENGTH_UPPER:
			working.back().len |= uint16_t(bytes[0]) << 8;
			if(working.back().len == 0) {
				working.back().done = true;
				state = HEADER;
			} else {
				state = DATA;
			}
			bytes.pop_front();
			break;
		case DATA: {
			// The RingBuffer is not contiguous, so copy out with read() rather than taking a pointer
			auto& frame = working.back();
			const size_t dataToGo = frame.len - frame.data.size();
			const size_t toCopy = std::min(bytes.size(), dataToGo);
			const size_t oldSize = frame.data.size();
			frame.data.resize(oldSize + toCopy);
			bytes.read(frame.data.data() + oldSize, 0, toCopy);
			bytes.pop(toCopy);
			if(toCopy == dataToGo) {
				frame.done = true;
				state = HEADER;
			}
			break;
		}
		}
	}

	return std::any_of(working.begin(), working.end(), [](const RxFrame& rx) { return rx.done; });
}

std::vector<REDPacketizer::RxFrame> REDPacketizer::output() {
	// Hand back the completed frames, anything still in progress stays in working
	std::vector<RxFrame> done;
	std::vector<RxFrame> pending;
	for(auto& rx : working) {
		if(rx.done)
			done.push_back(std::move(rx));
		else
			pending.push_back(std::move(rx));
	}
	working = std::move(pending);
	return done;
}

} // namespace Bootloader

} // namespace icsneo
