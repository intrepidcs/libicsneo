
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <vector>
#include <fcntl.h>
#include <cstring>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <optional>
#include "icsneo/platform/tcp.h"

#ifdef _WIN32
#define WIN_INT(a) static_cast<int>(a)
#else
#define WIN_INT(a) a
#endif

#ifdef __APPLE__
#define APPLE_SIN_LEN(a) a.sin_len = sizeof(struct sockaddr_in);
#else
#define APPLE_SIN_LEN(a)
#endif

using namespace icsneo;

TCP::Socket::Socket(int domain, int type, int protocol, bool nonblocking) {
	#ifdef _WIN32
		class WSAState {
		public:
			WSAState() {
				WSADATA wsaData = {};
				valid = ::WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
			}
			~WSAState() {
				::WSACleanup();
			}
			explicit operator bool() const { return valid; }
		private:
			bool valid = false;
		};
		static const WSAState WSA_STATE;
		if(!WSA_STATE)
			return;
	#endif
	fd = ::socket(domain, type, protocol);
	if(nonblocking) {
		#ifdef _WIN32
			unsigned long param = 1;
			::ioctlsocket(fd, FIONBIO, &param);
		#else
			const int flags = fcntl(fd, F_GETFL, 0);
			::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		#endif
	}
}

TCP::Socket::~Socket() {
	#ifdef _WIN32
		::closesocket(fd);
	#else
		::close(fd);
	#endif
}

void TCP::Find(std::vector<FoundDevice>& found) {
	static const auto MDNS_PORT = htons((unsigned short)5353);
	static const auto MDNS_IP = htonl((((uint32_t)224U) << 24U) | ((uint32_t)251U));

	class IFAddresses {
	public:
		#ifdef _WIN32
			typedef IP_ADAPTER_ADDRESSES* InterfaceHandle;
		#else
			typedef ifaddrs* InterfaceHandle;
		#endif

		class Interface {
		public:
			Interface(InterfaceHandle handle) : handle(handle) {}
			Interface next() {
				#ifdef _WIN32
					return Interface(handle->Next);
				#else
					return Interface(handle->ifa_next);
				#endif
			}
			unsigned flags() {
				#ifdef _WIN32
					return handle->Flags;
				#else
					return handle->ifa_flags;
				#endif
			}
			explicit operator bool() {
				return handle;
			}
			bool validType() {
				#ifdef _WIN32
					return
						handle &&
						(handle->TunnelType != TUNNEL_TYPE_TEREDO) &&
						(handle->OperStatus == IfOperStatusUp) &&
						(address()->sa_family == AF_INET);
				#else
					return
						handle &&
						handle->ifa_addr &&
						(flags() & IFF_UP) &&
						(flags() & IFF_MULTICAST) &&
						!(flags() & IFF_LOOPBACK) &&
						!(flags() & IFF_POINTOPOINT) &&
						(handle->ifa_addr->sa_family == AF_INET) &&
						(((sockaddr_in*)address())->sin_addr.s_addr != htonl(INADDR_LOOPBACK));
				#endif

			}
			InterfaceHandle operator->() {
				return handle;
			}
			sockaddr* address() const {
				#ifdef _WIN32
					return handle->FirstUnicastAddress->Address.lpSockaddr;
				#else
					return handle->ifa_addr;
				#endif
			}
			std::string_view name() const {
				#ifdef _WIN32
					return handle->AdapterName;
				#else
					return handle->ifa_name;
				#endif
			}
		private:
			InterfaceHandle handle;
		};

		IFAddresses() {
			#ifdef _WIN32
				unsigned long ret;
				unsigned long size = 15'000;
				do {
					storage.resize(size);
					ret = ::GetAdaptersAddresses(AF_INET, 0, NULL, (InterfaceHandle)storage.data(), &size);
				} while (ret == ERROR_BUFFER_OVERFLOW);
				front = (InterfaceHandle)storage.data();
			#else
				::getifaddrs(&front);
			#endif
		}
		~IFAddresses() {
			#ifdef _WIN32
			#else
				::freeifaddrs(front);
			#endif
		}

		explicit operator bool() {
			return front;
		}
		Interface begin() const {
			return Interface(front);
		}
	private:
		#ifdef _WIN32
			std::vector<uint8_t> storage;
		#endif
		InterfaceHandle front;
	};

	IFAddresses interfaces;
	if(!interfaces) {
		EventManager::GetInstance().add(APIEvent::Type::GetIfAddrsError, APIEvent::Severity::EventWarning);
		return;
	}

	for(auto intf = interfaces.begin(); intf; intf = intf.next()) {
		if(!intf.validType())
			continue;
		Socket socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(!socket) {
			EventManager::GetInstance().add(APIEvent::Type::SocketFailedToOpen, APIEvent::Severity::EventWarning);
			return;
		}

		{
			unsigned int reuse = 1;
			if(::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
				EventManager::GetInstance().add(APIEvent::Type::ErrorSettingSocketOption, APIEvent::Severity::EventWarning);
				continue;
			}
			#ifndef _WIN32
				if(::setsockopt(socket, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0) {
					EventManager::GetInstance().add(APIEvent::Type::ErrorSettingSocketOption, APIEvent::Severity::EventWarning);
					continue;
				}
				#ifndef __APPLE__
					if(::setsockopt(socket, SOL_SOCKET, SO_BINDTODEVICE, intf.name().data(), intf.name().size()) < 0) {
						EventManager::GetInstance().add(APIEvent::Type::ErrorSettingSocketOption, APIEvent::Severity::EventWarning);
						continue;
					}
				#endif
			#endif
		}
		auto ifAddrIn = (sockaddr_in*)intf.address();
		ifAddrIn->sin_port = MDNS_PORT;

		{
			sockaddr_in addr = *ifAddrIn;
			APPLE_SIN_LEN(addr);
			::setsockopt(socket, IPPROTO_IP, IP_MULTICAST_IF, (const char*)&addr.sin_addr, sizeof(addr.sin_addr));
			#ifndef _WIN32
				addr.sin_addr.s_addr = INADDR_ANY;
			#endif
			if(::bind(socket, (sockaddr*)&addr, sizeof(addr)) == -1) {
				EventManager::GetInstance().add(APIEvent::Type::FailedToBind, APIEvent::Severity::EventWarning);
				continue;
			}
		}

		{
			ip_mreq req = {};
			req.imr_multiaddr.s_addr = htonl((((uint32_t)224U) << 24U) | ((uint32_t)251U));
			req.imr_interface = ifAddrIn->sin_addr;
			::setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&req, sizeof(req));
		}

		std::array<uint8_t, 35> query = {
			0x00, 0x00, /* id */
			0x00, 0x00, /* flags */
			0x00, 0x01, /* query count */
			0x00, 0x00, /* answer count */
			0x00, 0x00, /* auth count */
			0x00, 0x00, /* additional count */
			0x06, '_', 'n', 'e', 'o', 'v', 'i', 0x04, '_', 't', 'c', 'p', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00,
			0x00, 0x0c, /* type*/
			0x00, 0x01 /* class */
		};
		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = MDNS_IP;
		addr.sin_port = MDNS_PORT;
		APPLE_SIN_LEN(addr);
		if(::sendto(socket, (char*)query.data(), WIN_INT(query.size()), 0, (sockaddr*)&addr, sizeof(addr)) < 0) {
			EventManager::GetInstance().add(APIEvent::Type::SendToError, APIEvent::Severity::EventWarning);
			continue;
		}

		timeval timeout = {};
		timeout.tv_usec = 50000;
		fd_set readfs;
		FD_ZERO(&readfs);
		int nfds = WIN_INT(socket) + 1;
		FD_SET(socket, &readfs);
		while(true) {
			static constexpr size_t bufferLen = 2048;
			uint8_t buffer[bufferLen];
			::select(nfds, &readfs, 0, 0, &timeout); // timeout is intentially not reset, we want timeout.tv_usec _total_
			const auto recvRet = ::recv(socket, (char*)buffer, bufferLen, 0);
			static constexpr auto headerLength = 12;
			if(recvRet < headerLength) {
				break;
			}
			uint8_t* bufferEnd = buffer + recvRet;
			const auto flags = ntohs(*(uint16_t*)(buffer + 2));
			const bool isResponse = flags >> 15;
			if(!isResponse)
				continue;
			const auto answerCount = ntohs(*(uint16_t*)(buffer + 6));
			const auto additionalCount = ntohs(*(uint16_t*)(buffer + 10));
			if(answerCount < 1 || additionalCount < 1) {
				continue;
			}
			std::function<std::optional<size_t>(uint8_t*,std::vector<std::string_view>&)> parseStrings = [&](uint8_t* start, std::vector<std::string_view>& strings) -> std::optional<size_t> {
				uint8_t* origStart = start;
				for(size_t i = 0; i < 10 /* infinite loop prevention */; ++i) {
					if(start >= bufferEnd)
						return std::nullopt;
					static constexpr uint8_t isCompressed = 0xC0;
					if(*start & isCompressed) {
						if(start + 2 > bufferEnd)
							return std::nullopt;
						uint16_t offset = ntohs(*(uint16_t*)start) & 0x3FFF;
						if(buffer + offset > bufferEnd)
							return std::nullopt;
						parseStrings(buffer + offset, strings);
						start += 2;
						return start - origStart;
					} else if(*start == 0) {
						return start + 1 - origStart;
					} else {
						if(start + 1 /* skip the length */ + *start > bufferEnd - 1 /* every string ends with '\0' */)
							return std::nullopt;
						strings.emplace_back((char*)(start + 1), *start);
						start += 1 + *start;
					}
				}
				return std::nullopt;
			};
			struct Record {
				enum class Type {
					PTR = 0x000C,
					SRV = 0x0021,
					A = 0x0001,
				};
				std::vector<std::string_view> name;
				Type type;
				uint8_t* data;
				uint16_t dataLength;
			};
			const auto parseRecord = [&](uint8_t* start, Record& parsedRecord) -> std::optional<size_t> {
				uint8_t* origStart = start;
				const auto nameLength = parseStrings(start, parsedRecord.name);
				if(!nameLength)
					return std::nullopt;
				if(start + *nameLength + 10 /* type + flags + TTL + data length */ > bufferEnd)
					return std::nullopt;
				start += *nameLength;
				parsedRecord.type = Record::Type(ntohs(*(uint16_t*)start));
				start += 8; /* type + flags + TTL */
				parsedRecord.dataLength = ntohs(*(uint16_t*)start);
				start += 2;
				parsedRecord.data = start;
				start += parsedRecord.dataLength;
				return start - origStart;
			};

			const auto parseRecords = [&](uint8_t* start, std::vector<Record>& parsed) -> std::optional<size_t> {
				uint8_t* origStart = start;
				for(auto& record : parsed) {
					const auto recordLength = parseRecord(start, record);
					if(!recordLength || start + *recordLength > bufferEnd)
						return std::nullopt;
					start += *recordLength;
				}
				return start - origStart;
			};
			std::vector<Record> answers(answerCount);
			const auto answersLength = parseRecords(buffer + headerLength, answers);
			if(!answersLength)
				continue;

			std::vector<Record> additional(additionalCount);
			const auto additionalLength = parseRecords(buffer + headerLength + *answersLength, additional);
			if(!additionalLength)
				continue;
			FoundDevice foundDevice;

			const auto fillSerial = [&]() -> bool {
				for(const auto& record : answers) {
					if(record.name.size() == 3 && record.name[0] == "_neovi" && record.name[1] == "_tcp" && record.name[2] == "local") {
						constexpr size_t deviceSerialBufferLength = sizeof(foundDevice.serial);
						std::copy(record.data + 1 /* length field */, record.data + deviceSerialBufferLength, foundDevice.serial);
						foundDevice.serial[deviceSerialBufferLength - 1] = '\0';
						return true;
					}
				}
				return false;
			};

			if(!fillSerial())
				continue;

			std::optional<uint32_t> devIP;
			std::optional<uint16_t> devPort;
			const auto fillDevEndpoint = [&]() -> bool {
				for(const auto& record : additional) {
					if(record.name.size() == 4 && record.name[1] == "_neovi" && record.name[2] == "_tcp" && record.name[3] == "local") {
						if(record.type == Record::Type::A) {
							if(record.dataLength != 4)
								return false;
							devIP.emplace(ntohl(*(uint32_t*)record.data));
						} else if(record.type == Record::Type::SRV) {
							if(record.dataLength != 8 /* priority + weight + port */)
								return false;
							devPort.emplace(ntohs(*(uint16_t*)(record.data + 4)));
						}
					}
				}
				return devIP && devPort;
			};

			if(!fillDevEndpoint())
				continue;

			NetworkInterface on = {
				std::string(intf.name()),
				ntohl(ifAddrIn->sin_addr.s_addr)
			};

			foundDevice.makeDriver = [=](const device_eventhandler_t& eh, neodevice_t&) {
				return std::unique_ptr<Driver>(new TCP(eh, on, *devIP, *devPort));
			};
			found.push_back(std::move(foundDevice));
		}
	}
}

TCP::TCP(const device_eventhandler_t& err, NetworkInterface on, uint32_t dstIP, uint16_t dstPort) :
	Driver(err), interfaceDescription(on), dstIP(dstIP), dstPort(dstPort) {
}

bool TCP::open() {
	if(socket) {
		report(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::Error);
		return false;
	}

	auto partiallyOpenSocket = std::make_unique<Socket>(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	#if !defined(_WIN32) && !defined(__APPLE__)
		if(::setsockopt(*partiallyOpenSocket, SOL_SOCKET, SO_BINDTODEVICE, interfaceDescription.name.c_str(), interfaceDescription.name.size()) < 0) {
			report(APIEvent::Type::ErrorSettingSocketOption, APIEvent::Severity::Error);
			return false;
		}
	#endif

	{
		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(interfaceDescription.ip);
		APPLE_SIN_LEN(addr);
		if(::bind(*partiallyOpenSocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
			report(APIEvent::Type::FailedToBind, APIEvent::Severity::Error);
			return false;
		}
	}

	{
		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(dstIP);
		addr.sin_port = htons(dstPort);
		APPLE_SIN_LEN(addr);

		// the socket is non-blocking so it's expected that the first connect will fail
		if(::connect(*partiallyOpenSocket, (sockaddr*)&addr, sizeof(addr)) == 0) {
			report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
			return false;
		}
		#ifdef _WIN32
			if(::WSAGetLastError() != WSAEWOULDBLOCK) {
				report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
				return false;
			}
		#else
			if(errno != EINPROGRESS) {
				report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
				return false;
			}
		#endif

		timeval timeout = {};
		timeout.tv_sec = 1;
		fd_set writefs;
		FD_ZERO(&writefs);
		int nfds = WIN_INT(*partiallyOpenSocket) + 1;
		FD_SET(*partiallyOpenSocket, &writefs);
		::select(nfds, 0, &writefs, 0, &timeout);

		if(::connect(*partiallyOpenSocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
			#ifdef _WIN32
				if(::WSAGetLastError() != WSAEISCONN) {
					report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
					return false;
				}
			#elif defined(__APPLE__)
				if(errno != EISCONN) {
					report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
					return false;
				}
			#else
				report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
				return false;
			#endif
		}
	}

	socket = std::move(partiallyOpenSocket);
	readThread = std::thread(&TCP::readTask, this);
	writeThread = std::thread(&TCP::writeTask, this);
	return true;
}

bool TCP::isOpen() {
	return socket ? true : false;
}

bool TCP::close() {
	if(!isOpen() && !isDisconnected()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	closing = true;
	disconnected = false;

	if(readThread.joinable())
		readThread.join();
	if(writeThread.joinable())
		writeThread.join();

	WriteOperation flushop;
	readBuffer.pop(readBuffer.size());
	while(writeQueue.try_dequeue(flushop)) {}

	socket.reset();
	closing = false;

	return true;
}

void TCP::readTask() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	const int nfds = WIN_INT(*socket) + 1;
	fd_set readfs;
	FD_ZERO(&readfs);
	FD_SET(*socket, &readfs);
	timeval timeout;

	constexpr size_t READ_BUFFER_SIZE = 2048;
	uint8_t readbuf[READ_BUFFER_SIZE];
	while(!closing) {
		if(const auto received = ::recv(*socket, (char*)readbuf, READ_BUFFER_SIZE, 0); received > 0) {
			writeToReadBuffer(readbuf, received);
		} else {
			timeout.tv_sec = 0;
			timeout.tv_usec = 50'000;
			::select(nfds, &readfs, 0, 0, &timeout);
		}
	}
}

void TCP::writeTask() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	const int nfds = WIN_INT(*socket) + 1;
	fd_set writefs;
	FD_ZERO(&writefs);
	FD_SET(*socket, &writefs);
	timeval timeout;

	WriteOperation writeOp;
	while(!closing) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
			continue;

		while(!closing) {
			if(::send(*socket, (char*)writeOp.bytes.data(), WIN_INT(writeOp.bytes.size()), 0) > 0)
				break;
			timeout.tv_sec = 0;
			timeout.tv_usec = 100'000;
			::select(nfds, 0, &writefs, 0, &timeout);
		}
	}
}
