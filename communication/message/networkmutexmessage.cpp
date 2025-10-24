#include "icsneo/communication/message/networkmutexmessage.h"
#include "icsneo/communication/icspb.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/message/extendedresponsemessage.h"


using namespace icsneo;

std::shared_ptr<NetworkMutexMessage> NetworkMutexMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	NetworkMutexMessage decoded;
	commands::network::v1::NetworkMutex msg;

	if(!protoapi::processResponse(bytestream.data(), bytestream.size(), msg)) {
		return nullptr;
	}

	if(msg.has_client_id()) {
		decoded.owner_id.emplace(msg.client_id());
	}
	if(msg.has_type()) {
		decoded.type.emplace(static_cast<NetworkMutexType>(msg.type()));
	}
	if(msg.has_priority()) {
		decoded.priority.emplace(msg.priority());
	}
	if(msg.has_ttl()){
		decoded.ttlMs.emplace(msg.ttl());
	}
	if(msg.has_event()){
		decoded.event.emplace(static_cast<NetworkMutexEvent>(msg.event()));
	}
	for(int i = 0 ; i < msg.network_ids_size(); ++i){
		decoded.networks.emplace(static_cast<Network::NetID>(msg.network_ids(i)));
	}
	return std::make_shared<NetworkMutexMessage>(decoded);
}

std::vector<uint8_t> NetworkMutexMessage::EncodeArgumentsForLock(uint32_t client_id, NetworkMutexType type, uint32_t priority, uint32_t ttlMs, const std::set<Network::NetID>& networks, const device_eventhandler_t& /* report */) {
	commands::network::v1::NetworkMutex msg;
	for(auto&& network_id : networks) {
		msg.add_network_ids(static_cast<commands::network::v1::NetworkId>(network_id));
	}
	msg.set_client_id(client_id);
	msg.set_priority(priority);
	msg.set_ttl(ttlMs);
	msg.set_type(static_cast<commands::network::v1::MutexType>(type));

	return protoapi::getPayload(protoapi::Command::PUT, msg);
}

std::vector<uint8_t> NetworkMutexMessage::EncodeArgumentsForLockAll(uint32_t client_id, NetworkMutexType type, uint32_t priority, uint32_t ttlMs, const device_eventhandler_t& /* report */) {
	commands::network::v1::NetworkMutex msg;
	msg.set_client_id(client_id);
	msg.set_priority(priority);
	msg.set_ttl(ttlMs);
	msg.set_type(static_cast<commands::network::v1::MutexType>(type));
	msg.set_global(true);

	return protoapi::getPayload(protoapi::Command::PUT, msg);
}

std::vector<uint8_t> NetworkMutexMessage::EncodeArgumentsForUnlock(uint32_t client_id, const std::set<Network::NetID>& networks, const device_eventhandler_t& /* report */) {
	commands::network::v1::NetworkMutex msg;
	msg.Clear();
	for(auto&& network_id : networks)
	{
		msg.add_network_ids(static_cast<commands::network::v1::NetworkId>(network_id));
	}
	msg.set_client_id(client_id);
	msg.set_release(true);

	return protoapi::getPayload(protoapi::Command::PUT, msg);
}

std::vector<uint8_t> NetworkMutexMessage::EncodeArgumentsForUnlockAll(uint32_t client_id, const device_eventhandler_t& /* report */) {
	commands::network::v1::NetworkMutex msg;
	msg.Clear();
	msg.set_client_id(client_id);
	msg.set_release(true);
	msg.set_global(true);

	return protoapi::getPayload(protoapi::Command::PUT, msg);
}

std::vector<uint8_t> NetworkMutexMessage::EncodeArgumentsForStatus(Network::NetID network, const device_eventhandler_t& /* report */) {
	commands::network::v1::NetworkMutex msg;
	msg.add_network_ids(static_cast<commands::network::v1::NetworkId>(network));
	return protoapi::getPayload(protoapi::Command::GET, msg);
}
