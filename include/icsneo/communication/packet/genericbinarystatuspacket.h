#ifndef __GENERICBINARYSTATUSPACKET_H__
#define __GENERICBINARYSTATUSPACKET_H__

#ifdef __cplusplus

#include <cstdint>
#include <memory>
#include <vector>

namespace icsneo {

class GenericBinaryStatusMessage;

struct GenericBinaryStatusPacket {
    static std::shared_ptr<GenericBinaryStatusMessage> DecodeToMessage(const std::vector<uint8_t>& bytes);
    static std::vector<uint8_t> EncodeArguments(uint16_t binaryIndex);
};

}

#endif // __cplusplus

#endif // __GENERICBINARYSTATUSPACKET_H__
