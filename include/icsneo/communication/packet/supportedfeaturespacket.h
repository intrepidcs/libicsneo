#ifndef __SUPPORTEDFEATURESPACKET_H_
#define __SUPPORTEDFEATURESPACKET_H_

#ifdef __cplusplus

#include <cstdint>
#include <memory>
#include <vector>

namespace icsneo {

class SupportedFeaturesMessage;

struct SupportedFeaturesPacket {
	static std::shared_ptr<SupportedFeaturesMessage> DecodeToMessage(const std::vector<uint8_t>& bytes);
};

}

#endif // __cplusplus

#endif // __SUPPORTEDFEATURESPACKET_H_
