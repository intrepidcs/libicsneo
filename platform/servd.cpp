#include "icsneo/platform/servd.h"

#include <string_view>

using namespace icsneo;

#define SERVD_VERSION 1

static const Address SERVD_ADDRESS = Address("127.0.0.1", 26741);
static const std::string SERVD_VERSION_STR = std::to_string(SERVD_VERSION);

bool Servd::Enabled() {
	char* enabled = std::getenv("LIBICSNEO_USE_SERVD");
	return enabled ? enabled[0] == '1' : false;
}

std::vector<std::string> split(const std::string_view& str, char delim = ' ')
{
	std::vector<std::string> ret;
	size_t tail = 0;
	size_t head = 0;
	while (head < str.size()) {
		if (str[head] == delim) {
			ret.emplace_back(&str[tail], head - tail);
			tail = head + 1;
		}
		++head;
	}
	ret.emplace_back(&str[tail], head - tail);
	return ret;
}

void Servd::Find(std::vector<FoundDevice>& found) {
	Socket socket;
	if(!socket.set_nonblocking()) {
		EventManager::GetInstance().add(APIEvent::Type::ServdNonblockError, APIEvent::Severity::Error);
		return;
	}
	if(!socket.bind(Address("127.0.0.1", 0))) {
		EventManager::GetInstance().add(APIEvent::Type::ServdBindError, APIEvent::Severity::Error);
		return;
	}
	std::string response;

	response.resize(512);
	const std::string version_request = SERVD_VERSION_STR + " version";
	if(!socket.transceive(SERVD_ADDRESS, version_request, response, std::chrono::milliseconds(5000))) {
		EventManager::GetInstance().add(APIEvent::Type::ServdTransceiveError, APIEvent::Severity::Error);
		return;
	}

	if(std::stoll(response) < SERVD_VERSION) {
		EventManager::GetInstance().add(APIEvent::Type::ServdOutdatedError, APIEvent::Severity::Error);
		return;
	}

	response.resize(512);
	const std::string find_request = SERVD_VERSION_STR + " find";
	if(!socket.transceive(SERVD_ADDRESS, find_request, response, std::chrono::milliseconds(5000))) {
		EventManager::GetInstance().add(APIEvent::Type::ServdTransceiveError, APIEvent::Severity::Error);
		return;
	}
	const auto lines = split(response, '\n');
	for(auto&& line : lines) {
		const auto cols = split(line, ' ');
		if(cols.size() < 2) {
			EventManager::GetInstance().add(APIEvent::Type::ServdInvalidResponseError, APIEvent::Severity::Error);
			continue;
		}
		const auto& serial = cols[0];
		std::unordered_set<std::string> drivers;
		for (size_t i = 1; i < cols.size(); ++i) {
			drivers.emplace(cols[i]);
		}
		auto& newFound = found.emplace_back();
		std::copy(serial.begin(), serial.end(), newFound.serial);
		newFound.makeDriver = [=](device_eventhandler_t err, neodevice_t& forDevice) {
			return std::make_unique<Servd>(err, forDevice, drivers);
		};
	}
}

Servd::Servd(const device_eventhandler_t& err, neodevice_t& forDevice, const std::unordered_set<std::string>& availableDrivers) :
	Driver(err), device(forDevice) {
	messageSocket.set_nonblocking();
	messageSocket.bind(Address("127.0.0.1", 0));
	if(availableDrivers.count("dxx")) {
		driver = "dxx"; // prefer USB over Ethernet
	} else if(availableDrivers.count("cab")) {
		driver = "cab"; // prefer CAB over TCP
	} else if(availableDrivers.count("tcp")) {
		driver = "tcp";
	} else if(availableDrivers.count("vcp")) {
		driver = "vcp";
	} else {
		// just take the first driver
		driver = *availableDrivers.begin();
	}
}

Servd::~Servd() {
	close();
}

bool Servd::open() {
	const std::string request = SERVD_VERSION_STR + " open " + std::string(device.serial) + " " + driver;
	std::string response;
	response.resize(512);
	if(!messageSocket.transceive(SERVD_ADDRESS, request, response, std::chrono::milliseconds(5000))) {
		EventManager::GetInstance().add(APIEvent::Type::ServdTransceiveError, APIEvent::Severity::Error);
		return false;
	}
	const auto tokens = split(response);
	if(tokens.size() != 4) {
		EventManager::GetInstance().add(APIEvent::Type::ServdInvalidResponseError, APIEvent::Severity::Error);
		return false;
	}
	aliveThread = std::thread(&Servd::alive, this);
	readThread = std::thread(&Servd::read, this, Address{tokens[2].c_str(), (uint16_t)std::stol(tokens[3].c_str())});
	writeThread = std::thread(&Servd::write, this, Address{tokens[0].c_str(), (uint16_t)std::stol(tokens[1].c_str())});
	opened = true;
	return true;
}

bool Servd::isOpen() {
	return opened;
}

bool Servd::close() {
	setIsClosing(true);
	if(aliveThread.joinable()) {
		aliveThread.join();
	}
	if(readThread.joinable()) {
		readThread.join();
	}
	if(writeThread.joinable()) {
		writeThread.join();
	}
	if(isOpen()) {
		const std::string request = SERVD_VERSION_STR + " close " + std::string(device.serial);
		messageSocket.sendto(request.data(), request.size(), SERVD_ADDRESS);
	}
	opened = false;
	return true;
}

bool Servd::enableCommunication(bool enable, bool& sendMsg) {
	const std::string serialString(device.serial);
	{
		const std::string request = SERVD_VERSION_STR + " lock " + serialString + " com 1000";
		std::string response;
		response.resize(1);
		bool locked = false;
		const auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(1);
		do {
			if(!messageSocket.transceive(SERVD_ADDRESS, request, response, std::chrono::milliseconds(5000))) {
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
		const std::string request = SERVD_VERSION_STR + " load " + serialString + " com";
		std::string response;
		response.resize(20);
		if(!messageSocket.transceive(SERVD_ADDRESS, request, response, std::chrono::milliseconds(5000))) {
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
		const std::string request = SERVD_VERSION_STR + " store " + serialString  + " com " + std::to_string(com);
		if(!messageSocket.sendto(request.data(), request.size(), SERVD_ADDRESS)) {
			EventManager::GetInstance().add(APIEvent::Type::ServdSendError, APIEvent::Severity::Error);
			return false;
		}
	}
	comEnabled = enable;
	{
		const std::string request = SERVD_VERSION_STR + " unlock " + serialString + " com";
		if(!messageSocket.sendto(request.data(), request.size(), SERVD_ADDRESS)) {
			EventManager::GetInstance().add(APIEvent::Type::ServdSendError, APIEvent::Severity::Error);
			return false;
		}
	}
	return true;
}

void Servd::alive() {
	Socket socket;
	socket.set_nonblocking();
	socket.bind(Address("127.0.0.1", 0));
	const std::string statusRequest = SERVD_VERSION_STR + " status " + std::string(device.serial);
	std::string statusResponse;
	statusResponse.resize(8);
	while(!isDisconnected() && !isClosing()) {
		if(!socket.sendto(statusRequest.data(), statusRequest.size(), {"127.0.0.1", 26741})) {
			EventManager::GetInstance().add(APIEvent::Type::ServdSendError, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		bool hasData;
		if(!socket.poll(std::chrono::milliseconds(2000), hasData)) {
			EventManager::GetInstance().add(APIEvent::Type::ServdPollError, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		if(!hasData) {
			EventManager::GetInstance().add(APIEvent::Type::ServdNoDataError, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		size_t statusResponseSize = statusResponse.size();
		if(!socket.recv(statusResponse.data(), statusResponseSize)) {
			EventManager::GetInstance().add(APIEvent::Type::ServdRecvError, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		statusResponse.resize(statusResponseSize);
		if(statusRequest == "closed") {
			EventManager::GetInstance().add(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		if(statusResponse != "open") {
			EventManager::GetInstance().add(APIEvent::Type::ServdInvalidResponseError, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void Servd::read(Address&& address) {
	Socket socket;
	socket.set_nonblocking();
	socket.set_reuse(true);
	#ifdef _WIN32
	if(!socket.bind(Address("127.0.0.1", address.port()))) {
		EventManager::GetInstance().add(APIEvent::Type::ServdBindError, APIEvent::Severity::Error);
		setIsDisconnected(true);
		return;
	}
	#else
	if(!socket.bind(Address(address.ip().c_str(), address.port()))) {
		EventManager::GetInstance().add(APIEvent::Type::ServdBindError, APIEvent::Severity::Error);
		setIsDisconnected(true);
		return;
	}
	#endif
	if(!socket.join_multicast("127.0.0.1", address.ip())) {
		EventManager::GetInstance().add(APIEvent::Type::ServdJoinMulticastError, APIEvent::Severity::Error);
		setIsDisconnected(true);
		return;
	}

	std::vector<uint8_t> buf(65535);
	while(!isDisconnected() && !isClosing()) {
		bool hasData;
		if(!socket.poll(std::chrono::milliseconds(100), hasData)) {
			EventManager::GetInstance().add(APIEvent::Type::ServdPollError, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		if(!hasData) {
			continue;
		}
		size_t bufSize = buf.size();
		if(!socket.recv(buf.data(), bufSize)) {
			EventManager::GetInstance().add(APIEvent::Type::ServdRecvError, APIEvent::Severity::Error);
			setIsDisconnected(true);
			return;
		}
		pushRx(buf.data(), bufSize);
	}
}

void Servd::write(Address&& address) {
	Socket socket;
	socket.bind(Address("127.0.0.1", 0));
	WriteOperation writeOp;
	while(!isDisconnected() && !isClosing()) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100))) {
			continue;
		}
		if(!isClosing()) {
			if(!socket.sendto(writeOp.bytes.data(), writeOp.bytes.size(), address)) {
				EventManager::GetInstance().add(APIEvent::Type::ServdSendError, APIEvent::Severity::Error);
				setIsDisconnected(true);
				return;
			}
		}
	}
}
