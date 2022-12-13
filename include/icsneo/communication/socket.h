#ifndef __SOCKET_H_
#define __SOCKET_H_

#ifdef __cplusplus

#include <vector>
#include <optional>
#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>

#ifdef _WIN32
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET SocketFileDescriptor;
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <cerrno>
#include <string.h>
typedef int SocketFileDescriptor;
#endif

namespace icsneo {

enum class RPC {
	DEVICE_FINDER_FIND_ALL,
	DEVICE_FINDER_GET_SUPORTED_DEVICES,
	DEVICE_OPEN,
	DEVICE_GO_ONLINE,
	DEVICE_GO_OFFLINE,
	DEVICE_CLOSE,
	DEVICE_LOCK,
	DEVICE_UNLOCK,
	SDIO_OPEN,
	SDIO_CLOSE,
	GET_EVENTS,
	GET_LAST_ERROR,
	GET_EVENT_COUNT,
	DISCARD_EVENTS,
	SET_EVENT_LIMIT,
	GET_EVENT_LIMIT
};

static constexpr uint16_t RPC_PORT = 54949;

class SocketBase {
public:
	enum class Protocol {
		TCP = SOCK_STREAM,
	};

	bool open();
	bool close();
	bool connect();
	bool isOpen();
	bool isConnected();
	bool read(void* output, std::size_t length);
	bool write(const void* input, std::size_t length);
	bool writeString(const std::string& str);
	bool readString(std::string& str);
	
	template<typename... Ts>
	bool writeTyped(Ts... input) {
		return (... && write(&input, sizeof(input)));
	}
	
	template<typename... Ts>
	bool readTyped(Ts&... output) {
		return (... && read(&output, sizeof(output)));
	}

protected:
	Protocol protocol;
	uint16_t port;
	SocketFileDescriptor sockFileDescriptor;
	bool sockIsOpen = false;
	bool sockIsConnected = false;
	
	void setIgnoreSIGPIPE();
};

// RAII Socket
class ActiveSocket : public SocketBase {
public:
	ActiveSocket(SocketFileDescriptor sockFD);
	ActiveSocket(Protocol protocol, uint16_t port);
	~ActiveSocket();
};

// RAII Socket IO 
class LockedSocket : public SocketBase {
public:
	LockedSocket(SocketBase& socket, std::unique_lock<std::mutex>&& lock);
private:
	std::unique_lock<std::mutex> lock;
};

class Acceptor : public ActiveSocket {
public:
	Acceptor(Protocol protocol, uint16_t port);
	bool initialize();
	std::shared_ptr<ActiveSocket> accept();

private:
	bool isValid = false;
	bool bind();
	bool listen();
};

LockedSocket lockSocket();

}

#endif // __cplusplus

#endif
