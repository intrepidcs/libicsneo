#ifndef __ETHPHYMESSAGE_H__
#define __ETHPHYMESSAGE_H__

#ifdef __cplusplus

#include "icsneo/communication/packet/ethphyregpacket.h"
#include "icsneo/communication/message/message.h"
#include "icsneo/communication/packet.h"
#include "icsneo/api/eventmanager.h"
#include <vector>
#include <memory>

namespace icsneo {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // nameless struct/union
#endif

struct PhyMessage {
	bool Enabled;
	bool WriteEnable;
	bool Clause45Enable;
	uint8_t version;
	union {
		Clause22Message clause22;
		Clause45Message clause45;
	};
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

//Internal message which provides an interface with device ethernet PHY registers,
//with Clause22 and Clause45 message support
class EthPhyMessage : public Message {
public:
	EthPhyMessage() : Message(Message::Type::EthernetPhyRegister) {}
	bool appendPhyMessage(bool writeEnable, bool clause45, uint8_t phyAddrOrPort, uint8_t pageOrDevice, uint16_t regAddr, uint16_t regVal = 0x0000u, bool enabled = true);
	bool appendPhyMessage(std::shared_ptr<PhyMessage> message);
	size_t getMessageCount() const;

	std::vector<std::shared_ptr<PhyMessage>> messages;
};

}

#endif // __cplusplus

#endif