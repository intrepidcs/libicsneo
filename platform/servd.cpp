#include "icsneo/platform/servd.h"

#include <string_view>

#include <cstdlib>

using namespace icsneo;

#define SERVD_VERSION 2

static const Address SERVD_ADDRESS = Address("127.0.0.1", 26741);
static const std::string SERVD_VERSION_STR = std::to_string(SERVD_VERSION);

bool Servd::Enabled() {
	#ifdef _MSC_VER
	#pragma warning(push)
	#pragma warning(disable : 4996)
	#endif
	char* enabled = std::getenv("LIBICSNEO_USE_SERVD");
	#ifdef _MSC_VER
	#pragma warning(pop)
	#endif
	return enabled ? enabled[0] == '1' : false;
}

std::vector<std::string> split(const std::string_view& str, char delim = ' ') {
	if(str.empty())
		return {};
	std::vector<std::string> ret;
	size_t tail = 0;
	size_t head = 0;
	while(head < str.size()) {
		if(str[head] == delim) {
			ret.emplace_back(&str[tail], head - tail);
			tail = head + 1;
		}
		++head;
	}
	ret.emplace_back(&str[tail], head - tail);
	return ret;
}

void Servd::Find(std::vector<FoundDevice>& found) {
	Socket socket(AF_INET, SOCK_DGRAM, 0);
	socket.connect(SERVD_ADDRESS);
	if(!socket.set_nonblocking()) {
		EventManager::GetInstance().add(APIEvent::Type::ServdNonblockError, APIEvent::Severity::Error);
		return;
	}
	std::string response;

	response.resize(512);
	const std::string version_request = SERVD_VERSION_STR + " version";
	if(!socket.transceive(version_request, response, std::chrono::milliseconds(5000))) {
		EventManager::GetInstance().add(APIEvent::Type::ServdTransceiveError, APIEvent::Severity::Error);
		return;
	}

	if(std::stoll(response) < SERVD_VERSION) {
		EventManager::GetInstance().add(APIEvent::Type::ServdOutdatedError, APIEvent::Severity::Error);
		return;
	}

	response.resize(512);
	const std::string find_request = SERVD_VERSION_STR + " find";
	if(!socket.transceive(find_request, response, std::chrono::milliseconds(5000))) {
		EventManager::GetInstance().add(APIEvent::Type::ServdTransceiveError, APIEvent::Severity::Error);
		return;
	}
	const auto lines = split(response, '\n');
	for(auto&& line : lines) {
		const auto cols = split(line, ' ');
		if(cols.size() < 3) {
			EventManager::GetInstance().add(APIEvent::Type::ServdInvalidResponseError, APIEvent::Severity::Error);
			continue;
		}
		const auto& serial = cols[0];
		const auto& ip = cols[1];
		uint16_t port = 0;
		try {
			port = static_cast<uint16_t>(std::stoi(cols[2]));
		} catch (const std::exception&) {
			EventManager::GetInstance().add(APIEvent::Type::ServdInvalidResponseError, APIEvent::Severity::Error);
			continue;
		}
		Address address(ip.c_str(), port);
		auto& newFound = found.emplace_back();
		std::copy(serial.begin(), serial.end(), newFound.serial);
		newFound.makeDriver = [=](device_eventhandler_t err, neodevice_t& forDevice) {
			return std::make_unique<Servd>(err, forDevice, address);
		};
	}
}

Servd::Servd(const device_eventhandler_t& err, neodevice_t& forDevice, const Address& address) :
	Driver(err), device(forDevice), messageSocket(AF_INET, SOCK_DGRAM, 0) {
	messageSocket.connect(address);
	messageSocket.set_nonblocking();
}

Servd::~Servd() {
	close();
}

bool Servd::open() {
	const std::string request = SERVD_VERSION_STR + " open";
	std::string response;
	response.resize(512);
	if(!messageSocket.transceive(request, response, std::chrono::milliseconds(5000))) {
		EventManager::GetInstance().add(APIEvent::Type::ServdTransceiveError, APIEvent::Severity::Error);
		return false;
	}
	const auto tokens = split(response);
	if(tokens.size() != 2) {
		EventManager::GetInstance().add(APIEvent::Type::ServdInvalidResponseError, APIEvent::Severity::Error);
		return false;
	}
	dataSocket = std::make_unique<Socket>(AF_INET, SOCK_STREAM, 0);
	const auto& ip = tokens[0];
	uint16_t port = 0;
	try {
		port = static_cast<uint16_t>(std::stoi(tokens[1]));
	} catch (const std::exception&) {
		EventManager::GetInstance().add(APIEvent::Type::ServdInvalidResponseError, APIEvent::Severity::Error);
		return false;
	}
	Address address(ip.c_str(), port);
	dataSocket->connect(address);
	readThread = std::thread(&Servd::read, this);
	writeThread = std::thread(&Servd::write, this);
	opened = true;
	return true;
}

bool Servd::isOpen() {
	return opened;
}

bool Servd::close() {
	setIsClosing(true);
	if(readThread.joinable()) {
		readThread.join();
	}
	if(writeThread.joinable()) {
		writeThread.join();
	}
	if(isOpen()) {
		Address localAddress;
		dataSocket->address(localAddress);
		const std::string request = SERVD_VERSION_STR + " close " + localAddress.ip() + " " + std::to_string(localAddress.port());
		std::string response;
		response.resize(1);
		if(!messageSocket.transceive(request, response, std::chrono::milliseconds(5000))) {
			EventManager::GetInstance().add(APIEvent::Type::ServdTransceiveError, APIEvent::Severity::Error);
			return false;
		}
		dataSocket.reset();
	}
	opened = false;
	setIsClosing(false);
	return true;
}

bool Servd::enableCommunication(bool enable, bool& sendMsg) {
	const std::string serialString(device.serial);
	{
		const std::string request = SERVD_VERSION_STR + " lock com 1000";
		std::string response;
		response.resize(1);
		bool locked = false;
		const auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(1);
		do {
			if(!messageSocket.transceive(request, response, std::chrono::milliseconds(5000))) {
				return false;
			}
			locked = response == "1" ? true : false;
			if(locked) {
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		} while (std::chrono::steady_clock::now() < timeout);
		if(!locked) {
			EventManager::GetInstance().add(APIEvent::Type::ServdLockError, APIEvent::Severity::Error);
			return false;
		}
	}
	uint64_t com = 0;
	{
		const std::string request = SERVD_VERSION_STR + " load com";
		std::string response;
		response.resize(20);
		if(!messageSocket.transceive(request, response, std::chrono::milliseconds(5000))) {
			EventManager::GetInstance().add(APIEvent::Type::ServdTransceiveError, APIEvent::Severity::Error);
			return false;
		}
		com = response.empty() ? 0 : std::stoll(response);
	}
	sendMsg = false;
	if(enable) {
		sendMsg = true; // always send enable com, we want the status message(s)
	} else {
		if(com == 0) {
			sendMsg = true; // we're the only client, safe to send disable
		} else if(com == 1 && comEnabled) {
			sendMsg = true; // we're the last client and we have com enabled, disable
		}
	}
	if(comEnabled != enable) {
		com += enable ? 1 : -1;
		const std::string request = SERVD_VERSION_STR + " store com " + std::to_string(com);
		std::string response;
		response.resize(1);
		if(!messageSocket.transceive(request, response, std::chrono::milliseconds(5000))) {
			EventManager::GetInstance().add(APIEvent::Type::ServdSendError, APIEvent::Severity::Error);
			return false;
		}
	}
	comEnabled = enable;
	{
		const std::string request = SERVD_VERSION_STR + " unlock com";
		std::string response;
		response.resize(1);
		if(!messageSocket.transceive(request, response, std::chrono::milliseconds(5000))) {
			EventManager::GetInstance().add(APIEvent::Type::ServdSendError, APIEvent::Severity::Error);
			return false;
		}
	}
	return true;
}

void Servd::read() {
	std::vector<uint8_t> buf(2 * 1024 * 1024);
	while(!isDisconnected() && !isClosing()) {
		bool hasData;
		if(!dataSocket->poll(std::chrono::milliseconds(100), hasData)) {
			EventManager::GetInstance().add(APIEvent::Type::ServdPollError, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		if(!hasData) {
			continue;
		}
		size_t bufSize = buf.size();
		if(!dataSocket->recv(buf.data(), bufSize)) {
			EventManager::GetInstance().add(APIEvent::Type::ServdRecvError, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		pushRx(buf.data(), bufSize);
	}
}

void Servd::write() {
	WriteOperation writeOp;
	while(!isDisconnected() && !isClosing()) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100))) {
			continue;
		}
		if(!isClosing()) {
			if(!dataSocket->send(writeOp.bytes.data(), writeOp.bytes.size())) {
				EventManager::GetInstance().add(APIEvent::Type::ServdSendError, APIEvent::Severity::Error);
				setIsDisconnected(true);
				return;
			}
		}
	}
}
