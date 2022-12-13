#include "icsneo/communication/socket.h"
#include "icsneo/api/event.h"
#include "icsneo/api/eventmanager.h"

namespace icsneo {

bool SocketBase::open() {
	#ifdef _WIN32
	WSADATA wsaData;
	if(::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		EventManager::GetInstance().add(APIEvent::Type::SocketFailedToOpen, APIEvent::Severity::Error);
		return false;
	}
	#endif

	if((sockFileDescriptor = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		#ifdef _WIN32
		::WSACleanup();
		#endif
		EventManager::GetInstance().add(APIEvent::Type::SocketFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	sockIsOpen = true;
	return true;
}

bool SocketBase::close() {
	#ifdef _WIN32
	if(::closesocket(sockFileDescriptor) < 0) {
		// should probably check for WSAEWOULDBLOCK as ::closesocket must be repeated to close in that case
		#ifdef ICSNEO_ENABLE_DEVICE_SHARING
		EventManager::GetInstance().add(APIEvent::Type::SocketFailedToClose, APIEvent::Severity::Error);
		#endif
		return false;
	}
	::WSACleanup();
	#else
	// ignore ENOTCONN from ::shutdown as the peer may have already forcibly closed its socket (e.g. a crash)
	if( ((::shutdown(sockFileDescriptor, SHUT_RDWR) < 0) && (ENOTCONN != errno)) ||
		((::close(sockFileDescriptor) < 0) && (EBADF == errno)) )
	{
		#ifdef ICSNEO_ENABLE_DEVICE_SHARING
		EventManager::GetInstance().add(APIEvent::Type::SocketFailedToClose, APIEvent::Severity::Error);
		#endif
		return false;
	}
	#endif

	sockIsOpen = false;
	sockIsConnected = false;
	return true;
}

bool SocketBase::connect() {
	sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if( (!isOpen() && !open()) ||
	    (::inet_pton(addr.sin_family, "127.0.0.1", &addr.sin_addr) <= 0) ||
	    (::connect(sockFileDescriptor, (sockaddr*)&addr, sizeof(addr)) < 0) )
	{
		EventManager::GetInstance().add(APIEvent::Type::SocketFailedToConnect, APIEvent::Severity::Error);
		return false;
	}

	#ifdef _WIN32
	DWORD tv = 5000u; // 5 second receive timeout but in windows
	#else
	struct timeval tv;
	tv.tv_sec = 5u; // 5 second receive timeout
	tv.tv_usec = 0;
	setIgnoreSIGPIPE();
	#endif

	// Set the 5 second timeout from above in the socket options
	::setsockopt(sockFileDescriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
	::setsockopt(sockFileDescriptor, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

	sockIsConnected = true;
	return true;
}

bool SocketBase::isOpen() {
	return sockIsOpen;
}

bool SocketBase::isConnected() {
	return sockIsConnected;
}

bool SocketBase::read(void* output, std::size_t length) {
	if(!(isOpen() && isConnected()))
		return false;

	#ifdef _WIN32
	return ::recv(sockFileDescriptor, (char*)output, (int)length, 0) > 0;
	#else
	if(::read(sockFileDescriptor, output, length) <= 0) {
		EventManager::GetInstance().add(APIEvent::Type::SocketFailedToRead, APIEvent::Severity::Error);
		return false;
	}
	return true;
	#endif
}

bool SocketBase::write(const void* input, std::size_t length) {
	if(!(isOpen() && isConnected()))
		return false;

	#ifdef _WIN32
	if(::send(sockFileDescriptor, (char*)input, (int)length, 0) < 0) {
		switch(WSAGetLastError()) {
			case WSAETIMEDOUT:
			case WSAENOTCONN:
			case WSAESHUTDOWN:
			case WSAECONNRESET:
			case WSAECONNABORTED:
			{
				sockIsOpen = false;
				break;
			}
			default:
			{
				EventManager::GetInstance().add(APIEvent::Type::SocketFailedToWrite, APIEvent::Severity::Error);
				break;
			}
		}
		return false;
	}
	#else
	if(::write(sockFileDescriptor, input, length) < 0) {
		switch(errno) {
			case EPIPE:
			case ETIMEDOUT:
			{
				sockIsOpen = false;
				break;
			}
			default:
			{
				EventManager::GetInstance().add(APIEvent::Type::SocketFailedToWrite, APIEvent::Severity::Error);
				break;
			}
		}
		return false;
	}
	#endif
	return true;
}

bool SocketBase::readString(std::string& str) {
	size_t length;
	if(!read(&length, sizeof(length)))
		return false;
	str.resize(length);
	if(!read(str.data(), length))
		return false;
	return true;
}

bool SocketBase::writeString(const std::string& str) {
	size_t length = str.size();
	if(!write(&length, sizeof(length)))
		return false;
	if(!write(str.data(), length))
		return false;
	return true;
}

ActiveSocket::ActiveSocket(SocketFileDescriptor sockFD) {
	sockFileDescriptor = sockFD;
	sockIsOpen = true;
	sockIsConnected = true;
}

ActiveSocket::ActiveSocket(Protocol protocol, uint16_t port) {
	this->protocol = protocol;
	this->port = port;
}

ActiveSocket::~ActiveSocket() {
	if(isOpen())
		close();
}

LockedSocket::LockedSocket(SocketBase& base, std::unique_lock<std::mutex>&& l) :
	SocketBase(base), lock(std::move(l)) {
}

LockedSocket lockSocket() {
	static ActiveSocket socket(SocketBase::Protocol::TCP, RPC_PORT);
	if(!socket.isOpen())
		socket.open();
	if(!socket.isConnected())
		socket.connect();
	static std::mutex lock;
	return LockedSocket(socket, std::unique_lock<std::mutex>(lock));
}

void SocketBase::setIgnoreSIGPIPE() {
	#ifndef _WIN32
	struct sigaction sa{};
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	::sigaction(SIGPIPE, &sa, NULL);
	#endif
}

Acceptor::Acceptor(Protocol protocol, uint16_t port)
	: ActiveSocket(protocol, port) {
}

bool Acceptor::initialize() {
	if(open() && bind() && listen()) {
		isValid = true;
		return true;
	}
	return false;
}

std::shared_ptr<ActiveSocket> Acceptor::accept()
{
	if(!isValid)
		return nullptr;
	const SocketFileDescriptor acceptFd = ::accept(sockFileDescriptor, (sockaddr*)NULL, NULL);
	if(acceptFd < 0)
		return nullptr;
	return std::make_shared<ActiveSocket>(acceptFd);
}

bool Acceptor::bind()
{
	sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	if( (::inet_pton(addr.sin_family, "127.0.0.1", &addr.sin_addr) <= 0) ||
	    (::bind(sockFileDescriptor, (sockaddr*)&addr, sizeof(addr)) < 0) )
	{
		EventManager::GetInstance().add(APIEvent::Type::SocketAcceptorFailedToBind, APIEvent::Severity::Error);
		return false;
	}
	return true;
}

bool Acceptor::listen()
{
	if(::listen(sockFileDescriptor, UINT8_MAX) < 0) {
		EventManager::GetInstance().add(APIEvent::Type::SocketAcceptorFailedToListen, APIEvent::Severity::Error);
		return false;
	}
	return true;
}

} //namespace icsneo
