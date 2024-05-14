#ifndef __HARDWAREINFOPACKET_H__
#define __HARDWAREINFOPACKET_H__

#ifdef __cplusplus

#include <cstdint>
#include <memory>
#include <vector>

namespace icsneo {

class HardwareInfo;

struct HardwareInfoPacket {		
	static std::shared_ptr<HardwareInfo> DecodeToMessage(const std::vector<uint8_t>& bytes);
};

}

#endif // __cplusplus

#endif // __DEVICECOMPONENTVERSIONPACKET_H__
