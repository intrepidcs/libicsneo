#ifndef __ALLMACADDRESSESMESSAGE_H_
#define __ALLMACADDRESSESMESSAGE_H_

#define ICSNEO_MAC_ADDRESS_LEN 6
#define ICSNEO_MAX_MAC_COUNT 32

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <array>
namespace icsneo {

static constexpr uint16_t MaxMACAddressCount = ICSNEO_MAX_MAC_COUNT;
static constexpr uint8_t MACAddressLength = ICSNEO_MAC_ADDRESS_LEN;

using MACAddress = std::array<uint8_t, MACAddressLength>;

class AllMACAddressesMessage : public Message {
public:
	static std::shared_ptr<AllMACAddressesMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);

	AllMACAddressesMessage() : Message(Type::AllMACAddresses) {}

	std::unordered_map<Network::NetID, MACAddress> addresses;
};

} // namespace icsneo

#endif // __cplusplus

#endif // __ALLMACADDRESSESMESSAGE_H_
