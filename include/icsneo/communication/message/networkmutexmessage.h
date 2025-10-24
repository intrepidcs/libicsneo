#ifndef __NETWORKMUTEXMESSAGE_H_
#define __NETWORKMUTEXMESSAGE_H_

#include <cstdint>
#include <set>
#include "icsneo/communication/network.h"
#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/extendedresponsemessage.h"
#include "icsneo/api/eventmanager.h"

namespace icsneo {

enum class NetworkMutexType : uint8_t {
	Shared = 0,
	TxExclusive = 1,
	ExternalExclusive = 2,
	FullyExclusive = 3,
};

enum class NetworkMutexEvent : uint8_t {
	Released = 0,
	Expired = 1,
	Preempted = 2,
	Queued = 3,
	Acquired = 4,
};

class NetworkMutexMessage : public Message
{
public:
	NetworkMutexMessage() : Message(Message::Type::NetworkMutex) {}

	static std::shared_ptr<NetworkMutexMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	static std::vector<uint8_t> EncodeArgumentsForLock(uint32_t client_id, NetworkMutexType type, uint32_t priority, uint32_t ttlMs, const std::set<Network::NetID>& networks, const device_eventhandler_t& report);
	static std::vector<uint8_t> EncodeArgumentsForLockAll(uint32_t client_id, NetworkMutexType type, uint32_t priority, uint32_t ttlMs, const device_eventhandler_t& report);
	static std::vector<uint8_t> EncodeArgumentsForUnlock(uint32_t client_id, const std::set<Network::NetID>& networks, const device_eventhandler_t& report);
	static std::vector<uint8_t> EncodeArgumentsForUnlockAll(uint32_t client_id, const device_eventhandler_t& report);
	static std::vector<uint8_t> EncodeArgumentsForStatus(Network::NetID network, const device_eventhandler_t& report);

	static const char* GetNetworkMutexTypeString(NetworkMutexType type) {
		switch(type) {
			case icsneo::NetworkMutexType::Shared:
				return "Shared";
			case icsneo::NetworkMutexType::TxExclusive:
				return "TxExclusive";
			case icsneo::NetworkMutexType::ExternalExclusive:
				return "ExternalExclusive";
			case icsneo::NetworkMutexType::FullyExclusive:
				return "FullyExclusive";
			default:
				return "Unknown";
		}
	}

	static const char* GetNetworkMutexEventString(NetworkMutexEvent event) {
		switch(event) {
			case icsneo::NetworkMutexEvent::Acquired:
				return "Acquired";
			case icsneo::NetworkMutexEvent::Released:
				return "Released";
			case icsneo::NetworkMutexEvent::Preempted:
				return "Preempted";
			case icsneo::NetworkMutexEvent::Expired:
				return "Expired";
			case icsneo::NetworkMutexEvent::Queued:
				return "Queued";
			default:
				return "Unknown";
		}
	}

	std::optional<uint32_t> owner_id;
	std::optional<NetworkMutexType> type;
	std::optional<uint32_t> priority;
	std::optional<uint32_t> ttlMs;
	std::set<Network::NetID> networks;
	std::optional<NetworkMutexEvent> event;
};

} // namespace icsneo

#endif
