#ifndef __DISKDATAMESSAGE_H_
#define __DISKDATAMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

namespace icsneo {

class DiskDataMessage : public RawMessage {
public:
	DiskDataMessage(std::vector<uint8_t>&& d) : RawMessage(Network::NetID::DiskData) {
		data = std::move(d);
	}
};

}

#endif // __cplusplus

#endif