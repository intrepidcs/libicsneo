#ifndef __ENCODER_H_
#define __ENCODER_H_

#include "communication/message/include/message.h"
#include "communication/message/include/canmessage.h"
#include "communication/include/packet.h"
#include "communication/include/command.h"
#include "communication/include/network.h"
#include "communication/include/packetizer.h"
#include <queue>
#include <vector>
#include <memory>

#pragma pack(push, 1)

namespace icsneo {

class Encoder {
public:
	Encoder(std::shared_ptr<Packetizer> packetizerInstance) : packetizer(packetizerInstance) {}
	bool encode(std::vector<uint8_t>& result, const std::shared_ptr<Message>& message);
	bool encode(std::vector<uint8_t>& result, Command cmd, bool boolean) { return encode(result, cmd, std::vector<uint8_t>({ (uint8_t)boolean })); }
	bool encode(std::vector<uint8_t>& result, Command cmd, std::vector<uint8_t> arguments = {});

	bool supportCANFD = false;
private:
	std::shared_ptr<Packetizer> packetizer;
};

}

#pragma pack(pop)

#endif