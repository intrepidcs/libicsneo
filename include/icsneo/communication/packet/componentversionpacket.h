#ifndef __COMPONENTVERSIONPACKET_H__
#define __COMPONENTVERSIONPACKET_H__

#ifdef __cplusplus

#include <cstdint>
#include <memory>
#include <vector>

namespace icsneo {

class ComponentVersionsMessage;

struct ComponentVersionPacket {
    static std::shared_ptr<ComponentVersionsMessage> DecodeToMessage(const std::vector<uint8_t>& bytes);
};

}

#endif // __cplusplus

#endif // __DEVICECOMPONENTVERSIONPACKET_H__
