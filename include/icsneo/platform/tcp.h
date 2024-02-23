#ifndef __TCP_H_
#define __TCP_H_

#ifdef __cplusplus

#include <memory>

#include "icsneo/communication/driver.h"
#include "icsneo/device/founddevice.h"

namespace icsneo {

class TCP : public Driver {
public:
	static void Find(std::vector<FoundDevice>& foundDevices);

	struct NetworkInterface {
		const std::string name;
		const uint32_t ip;
	};

	TCP(const device_eventhandler_t& err, NetworkInterface on, uint32_t dstIP, uint16_t dstPort);
	~TCP() override { if(isOpen()) close(); }
	bool open() override;
	bool isOpen() override;
	bool close() override;
	bool isEthernet() const override { return true; }
private:
	#ifdef _WIN32
		typedef size_t SocketFileDescriptor;
	#else
		typedef int SocketFileDescriptor;
	#endif

	class Socket {
	public:
		Socket(int domain, int type, int protocol, bool nonblocking = true);
		~Socket();
		explicit operator bool() const { return fd != -1; }
		operator SocketFileDescriptor() const { return fd; }
	private:
		SocketFileDescriptor fd;
	};
	
	NetworkInterface interfaceDescription;
	uint32_t dstIP;
	uint16_t dstPort;
	std::unique_ptr<Socket> socket;
	void readTask() override;
	void writeTask() override;
};

}

#endif // __cplusplus

#endif
