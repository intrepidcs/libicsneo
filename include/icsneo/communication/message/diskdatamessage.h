#ifndef __DISKDATAMESSAGE_H_
#define __DISKDATAMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

namespace icsneo {

class DiskDataMessage : public InternalMessage {
public:
	DiskDataMessage(std::vector<uint8_t>&& d) : InternalMessage(_icsneo_netid_t::icsneo_netid_disk_data) {
		data = std::move(d);
	}
};

}

#endif // __cplusplus

#endif