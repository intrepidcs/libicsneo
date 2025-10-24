#ifndef COMMUNICATION_PROTO_API_IMPL_H
#define COMMUNICATION_PROTO_API_IMPL_H

#include <cstdint>

#ifdef _WIN32
#pragma warning(push, 0)
#endif
#include <common/v1/proto_header.pb.h>
#include <commands/generic/v1/client_id.pb.h>
#include <commands/network/v1/mutex.pb.h>
#ifdef _WIN32
#pragma warning(pop)
#endif

namespace icsneo {
namespace protoapi {

enum class Command : uint8_t {
	Unspecified = 0,
	GET = 1,
	PUT = 2,
	PATCH = 3,
};

enum class Id : uint32_t {
	Unspecified = 0,
	MfgConfig = 1,
	CpuId = 2,
	NetworkMutex = 3,
	ClientId = 4,
};

template <typename T>
struct IDLookup {
	constexpr static common::v1::ProtoId value = common::v1::ProtoId::PROTO_ID_UNSPECIFIED;
};

template <>
struct IDLookup<commands::network::v1::NetworkMutex> {
	constexpr static common::v1::ProtoId value = common::v1::ProtoId::PROTO_ID_NETWORK_MUTEX;
};

template <>
struct IDLookup<commands::generic::v1::ClientId> {
	constexpr static common::v1::ProtoId value = common::v1::ProtoId::PROTO_ID_GET_CLIENT_ID;
};

inline uint8_t* writeVarint32(uint32_t value, uint8_t* target) {
	return google::protobuf::io::CodedOutputStream::WriteVarint32ToArray(value, target);
}

inline const uint8_t* readVarint32(uint32_t& value, const uint8_t* source) {
	value = 0;
	do {
		value <<= 7;
		value += *source & 0x7F;
	} while(*source++ & 0x80);
	return source;
}

template <typename T>
std::vector<uint8_t> getPayload(Command command, const T& msg) {
	std::vector<uint8_t> payload;
	constexpr uint32_t proto_version = 1;
	common::v1::ProtoHeader hdr;
	hdr.set_version(proto_version);
	hdr.set_api_command(static_cast<common::v1::ApiCommand>(command));
	hdr.set_auth_method(common::v1::AuthMethod::PROTO_API_AUTH_NONE); // TODO
	hdr.set_proto_id(IDLookup<T>::value);

	static constexpr size_t varint_size_max = 5;
	size_t header_size = hdr.ByteSizeLong();
	size_t msg_size = msg.ByteSizeLong();
	size_t payload_size_max = header_size + msg_size + (varint_size_max * 2); // TODO: auth
	payload.resize(payload_size_max);

	uint8_t* target = payload.data();

	target = writeVarint32(static_cast<uint32_t>(header_size), target);
	hdr.SerializeToArray(target, static_cast<int>(header_size));
	target += header_size;
	if(msg_size) {
		target = writeVarint32(static_cast<uint32_t>(msg_size), target);
		msg.SerializeToArray(target, static_cast<int>(msg_size));
		target += msg_size;
	}
	payload.resize(target - payload.data());
	return payload;
}

template <typename T>
bool processResponse(const uint8_t* payload, size_t /* payload_length */, T& msg) {
	// TODO: we should probably check payload_length throughout to not go past the end of the supplied buffer

	// header
	common::v1::ProtoHeader hdr;
	uint32_t length;
	const uint8_t* serialized = readVarint32(length, payload);
	if(!hdr.ParseFromArray(serialized, length)) {
		return false;
	}
	// message payload
	serialized = readVarint32(length, serialized + length);
	if(!msg.ParseFromArray(serialized, length)) {
		return false;
	}
	return true;
}

inline Id getProtoId(const uint8_t* payload, size_t /* payload_length */) {
	// TODO: we should probably check this throughout to not go past the end of the supplied buffer

	// header
	common::v1::ProtoHeader hdr;
	uint32_t length;
	const uint8_t* serialized = readVarint32(length, payload);
	if(!hdr.ParseFromArray(serialized, length)) {
		return Id::Unspecified;
	}
	return static_cast<Id>(hdr.proto_id());
}

} // namespace protoapi
} // namespace icsneo

#endif