#ifndef __SOCKET_H_
#define __SOCKET_H_

#ifdef __cplusplus

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#endif

#include <string>
#include <chrono>

namespace icsneo {

#ifdef _WIN32
class WSA {
public:
	WSA() {
		// TODO: add error checking
		WSAStartup(MAKEWORD(2, 2), &wsaData);
	}
	~WSA() {
		WSACleanup();
	}
private:
	WSADATA wsaData;
};
#endif

class Address {
public:
	Address() = default;
	Address(const char* ip, uint16_t port)
		: _ip(ip), _port(port)
	{
		_sockaddr.sin_family = AF_INET;
		inet_pton(AF_INET, ip, &_sockaddr.sin_addr);
		_sockaddr.sin_port = htons(port);
	}
	Address(sockaddr_in& sockaddr)
		: _sockaddr(sockaddr)
	{
		char cip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &sockaddr.sin_addr, cip, sizeof(cip));
		_ip = cip;
		_port = ntohs(sockaddr.sin_port);
	}
	const std::string& ip() const { return _ip; }
	const uint16_t& port() const { return _port; }
	const sockaddr_in& sockaddr() const { return _sockaddr; }
private:
	std::string _ip;
	uint16_t _port;
	sockaddr_in _sockaddr;
};

class Socket {
public:
	#ifdef _WIN32
	using SocketHandleType = SOCKET;
	#else
	using SocketHandleType = int;
	#endif
	
	template<class... Args>
	Socket(Args&&... args) {
		#ifdef _WIN32
		static WSA wsa;
		#endif
		mFD = socket(std::forward<Args>(args)...);
	}
	
	~Socket() {
		#ifdef _WIN32
		closesocket(mFD);
		#else
		close(mFD);
		#endif
	}
	
	bool set_reuse(bool value) {
		int ival = value;
		return ::setsockopt(mFD, SOL_SOCKET, SO_REUSEADDR, (const char*)&ival, sizeof(ival)) != -1;
	}
	
	bool set_nonblocking() {
		#ifdef _WIN32
		u_long nonblock = 1;
		return ioctlsocket(mFD, FIONBIO, &nonblock) != SOCKET_ERROR;
		#else
		return fcntl(mFD, F_SETFL, fcntl(mFD, F_GETFL, 0) | O_NONBLOCK) != -1;
		#endif
	}

	bool connect(const Address& to) {
		return ::connect(mFD, (sockaddr*)&to.sockaddr(), sizeof(sockaddr_in)) != -1;
	}
	
	bool bind(const Address& at) {
		return ::bind(mFD, (sockaddr*)&at.sockaddr(), sizeof(sockaddr_in)) != -1;
	}
	
	bool poll(const std::chrono::milliseconds& timeout, bool& in) {
		#ifdef _WIN32
		WSAPOLLFD pfd;
		pfd.fd = mFD;
		pfd.events = POLLIN;
		if (::WSAPoll(&pfd, 1, static_cast<int>(timeout.count())) == SOCKET_ERROR) {
			return false;
		}
		in = pfd.revents & POLLIN;
		return true;
		#else
		struct pollfd pfd;
		pfd.fd = mFD;
		pfd.events = POLLIN;
		pfd.revents = 0;
		if (::poll(&pfd, 1, static_cast<int>(timeout.count())) == -1) {
			return false;
		}
		in = pfd.revents & POLLIN;
		return true;
		#endif
	}
	
	bool sendto(const void* buffer, size_t size, const Address& to) {
		size_t totalSent = 0;
		do {
			const auto sent = ::sendto(mFD, (const char*)buffer, (int)size, 0, (sockaddr*)&to.sockaddr(), sizeof(sockaddr_in));
			if (sent == -1) {
				return false;
			}
			totalSent += sent;
		} while (totalSent < size);
		return true;
	}
	
	bool send(const void* buffer, size_t size) {
		auto sent = ::send(mFD, (const char*)buffer, (int)size, 0);
		if(sent == -1) {
			return false;
		}
		return (size_t)sent == size;
	}
	
	bool recvfrom(void* buffer, size_t& size, Address& from) {
		sockaddr_in addr;
		socklen_t addLen = sizeof(addr);
		const auto read = ::recvfrom(mFD, (char*)buffer, (int)size, 0, (sockaddr*)&addr, &addLen);
		if (read == -1) {
			return false;
		}
		size = read;
		from = Address(addr);
		return true;
	}
	
	bool recv(void* buffer, size_t& size) {
		const auto read = ::recv(mFD, (char*)buffer, (int)size, 0);
		if (read == -1) {
			return false;
		}
		size = read;
		return true;
	}

	template<typename REQ, typename RES>
	bool transceive(REQ&& request, RES&& response, const std::chrono::milliseconds& timeout) {
		if(!send(request.data(), request.size())) {
			return false;
		}
		bool hasData;
		if(!poll(timeout, hasData)) {
			return false;
		}
		if(!hasData) {
			return false;
		}
		size_t responseSize = response.size();
		if(!recv(response.data(), responseSize)) {
			return false;
		}
		response.resize(responseSize);
		return true;
	}
	
	bool address(Address& address) const {
		sockaddr_in sin;
		socklen_t len = sizeof(sin);
		getsockname(mFD, (sockaddr*)&sin, &len);
		address = Address(sin);
		return true;
	}
	
	bool join_multicast(const std::string& interfaceIP, const std::string& multicastIP) {
		ip_mreq mreq;
		inet_pton(AF_INET, interfaceIP.c_str(), &mreq.imr_interface);
		inet_pton(AF_INET, multicastIP.c_str(), &mreq.imr_multiaddr);
		return setsockopt(mFD, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char*)&mreq, sizeof(mreq)) == 0;
	}
	
	operator bool() const { return mFD != -1; }
	operator SocketHandleType() const { return mFD; }
private:
	SocketHandleType mFD;
};

} // namespace icsneo

#endif // __cplusplus

#endif // __SOCKET_H_
