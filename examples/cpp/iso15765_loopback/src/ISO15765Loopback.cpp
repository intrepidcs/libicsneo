#include <cctype>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "icsneo/icsneocpp.h"

namespace {

using namespace std::chrono_literals;

enum class Mode {
	None,
	Raw,
	SingleFrame,
	Both,
};

struct Options {
	Mode mode = Mode::None;
	std::string testerSerial;
	std::string ecuSerial;
	int64_t baud = 500000;
	bool keepBaud = false;
	uint32_t txId = 0x7E0;
	uint32_t fcId = 0x7E8;
	std::optional<size_t> payloadBytes;
	int listenMs = 2000;
	uint8_t padding = 0x00;
	uint16_t fsTimeoutMs = 75;
	uint16_t fsWaitMs = 150;
	uint16_t cfTimeoutMs = 150;
};

std::mutex gLogMutex;

void logLine(const std::string& line) {
	std::lock_guard<std::mutex> lk(gLogMutex);
	std::cout << line << std::endl;
}

bool iequals(const std::string& a, const std::string& b) {
	if(a.size() != b.size())
		return false;
	for(size_t i = 0; i < a.size(); i++) {
		if(std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
			return false;
	}
	return true;
}

std::string hexBytes(const std::vector<uint8_t>& data) {
	std::ostringstream oss;
	oss << std::hex << std::setfill('0');
	for(size_t i = 0; i < data.size(); i++) {
		if(i)
			oss << ' ';
		oss << std::setw(2) << static_cast<unsigned>(data[i]);
	}
	return oss.str();
}

std::string pciDescribe(const std::vector<uint8_t>& data) {
	if(data.empty())
		return {};
	const uint8_t pciType = static_cast<uint8_t>(data[0] >> 4);
	const uint8_t pciLow = static_cast<uint8_t>(data[0] & 0x0F);
	std::ostringstream oss;
	switch(pciType) {
		case 0x0:
			if(pciLow > 7 || data.size() < static_cast<size_t>(1 + pciLow))
				return {};
			oss << "SF len=" << static_cast<unsigned>(pciLow);
			break;
		case 0x1: {
			if(data.size() < 8)
				return {};
			const uint16_t len = static_cast<uint16_t>((pciLow << 8) | data[1]);
			if(len < 8)
				return {};
			oss << "FF len=" << len;
			break;
		}
		case 0x2:
			if(data.size() < 2)
				return {};
			oss << "CF SN=" << static_cast<unsigned>(pciLow);
			break;
		case 0x3: {
			if(data.size() < 3 || pciLow > 2)
				return {};
			static const char* fsName[] = { "CTS", "WAIT", "OVFLW" };
			oss << "FC FS=" << fsName[pciLow];
			oss << " BS=" << static_cast<unsigned>(data[1]);
			oss << " STmin=0x" << std::hex << std::setw(2) << std::setfill('0')
				<< static_cast<unsigned>(data[2]);
			break;
		}
		default:
			return {};
	}
	return oss.str();
}

void printUsage(const char* argv0) {
	std::cerr
		<< "Usage: " << argv0 << " --raw|--sf|--both [options]\n\n"
		<< "Two Intrepid devices with CAN1 wired together (CANH/CANL, terminated).\n"
		<< "Prints CAN1 frames with ISO-TP PCI decode (SF/FF/CF/FC).\n\n"
		<< "Modes:\n"
		<< "  --raw   Raw CAN ping-pong (cable check)\n"
		<< "  --sf    Firmware ISO-TP single-frame TX from tester\n"
		<< "  --both  Firmware ISO-TP TX on tester, firmware flow-control on ECU\n\n"
		<< "Devices:\n"
		<< "  --tester SERIAL   Tester (sender) serial\n"
		<< "  --ecu SERIAL      ECU / peer serial\n"
		<< "                    Optional if exactly two devices are found.\n\n"
		<< "Optional:\n"
		<< "  --baud N          CAN1 baud (default 500000, applied temporarily)\n"
		<< "  --keep-baud       Do not change device baud\n"
		<< "  --tx-id HEX       Tester ISO-TP CAN ID (default 0x7E0)\n"
		<< "  --fc-id HEX       Flow-control CAN ID (default 0x7E8)\n"
		<< "  --bytes N         ISO-TP payload size (sf default 5, both default 20)\n"
		<< "  --listen-ms N     Time to listen after the action (default 2000)\n";
}

uint32_t parseU32(const std::string& s) {
	return static_cast<uint32_t>(std::stoul(s, nullptr, 0));
}

bool parseArgs(int argc, char** argv, Options& opt) {
	for(int i = 1; i < argc; i++) {
		const std::string a = argv[i];
		auto need = [&](const char* name) -> std::string {
			if(i + 1 >= argc)
				throw std::runtime_error(std::string("missing value for ") + name);
			return argv[++i];
		};
		if(a == "--raw")
			opt.mode = Mode::Raw;
		else if(a == "--sf")
			opt.mode = Mode::SingleFrame;
		else if(a == "--both")
			opt.mode = Mode::Both;
		else if(a == "--tester")
			opt.testerSerial = need("--tester");
		else if(a == "--ecu")
			opt.ecuSerial = need("--ecu");
		else if(a == "--baud")
			opt.baud = static_cast<int64_t>(std::stoll(need("--baud")));
		else if(a == "--keep-baud")
			opt.keepBaud = true;
		else if(a == "--tx-id")
			opt.txId = parseU32(need("--tx-id"));
		else if(a == "--fc-id")
			opt.fcId = parseU32(need("--fc-id"));
		else if(a == "--bytes")
			opt.payloadBytes = static_cast<size_t>(std::stoul(need("--bytes")));
		else if(a == "--listen-ms")
			opt.listenMs = std::stoi(need("--listen-ms"));
		else if(a == "-h" || a == "--help") {
			printUsage(argv[0]);
			return false;
		} else {
			std::cerr << "Unknown argument: " << a << "\n";
			printUsage(argv[0]);
			return false;
		}
	}
	if(opt.mode == Mode::None) {
		printUsage(argv[0]);
		return false;
	}
	return true;
}

size_t defaultPayloadSize(Mode mode) {
	return mode == Mode::SingleFrame ? 5 : 20;
}

std::vector<uint8_t> makePayload(size_t n) {
	std::vector<uint8_t> data(n);
	for(size_t i = 0; i < n; i++)
		data[i] = static_cast<uint8_t>(i & 0xFF);
	return data;
}

bool isExtended(uint32_t id) {
	return id > 0x7FFu;
}

void printApiResult(const char* what, bool ok) {
	std::ostringstream oss;
	oss << "  " << what << ": " << (ok ? "OK" : "FAIL");
	if(!ok) {
		const auto err = icsneo::GetLastError();
		if(err.getType() != icsneo::APIEvent::Type::NoErrorFound)
			oss << " [" << err << "]";
	}
	logLine(oss.str());
}

bool transmitCan(icsneo::Device& device, uint32_t arbId, std::vector<uint8_t> data) {
	auto frame = std::make_shared<icsneo::CANMessage>();
	frame->network = icsneo::Network::NetID::DWCAN_01;
	frame->arbid = arbId;
	frame->data = std::move(data);
	frame->isExtended = isExtended(arbId);
	frame->isCANFD = false;
	const bool ok = device.transmit(frame);
	if(!ok)
		printApiResult("transmit CAN", ok);
	return ok;
}

void fillIso15765(icsneo::Iso15765MessageArgs& msg, const Options& opt, const std::vector<uint8_t>& payload, bool rx) {
	msg.setTxIndex(0);
	msg.setArbId(icsneo::Iso15765MessageArgs::ArbId(opt.txId, isExtended(opt.txId)));
	msg.setFlowControlArbId(icsneo::Iso15765MessageArgs::ArbId(opt.fcId, isExtended(opt.fcId)));
	msg.setFlowControlArbIdMask(isExtended(opt.txId) ? 0x1FFFFFFFu : 0x7FFu);
	msg.setIsCanFd(false);
	msg.setIsBrsEnabled(false);
	msg.setTxDl(8);
	msg.setPaddingValue(opt.padding);
	msg.setFsTimeout(opt.fsTimeoutMs);
	msg.setFsWaitTimeout(opt.fsWaitMs);
	msg.setCfTimeout(opt.cfTimeoutMs);
	msg.setIsFlowControlEnabled(rx);
	if(!payload.empty()) {
		auto copy = payload;
		msg.setData(std::move(copy));
	}
}

struct Sniffer {
	const char* tag = "";
	std::optional<uint64_t> t0;
	std::mutex t0Mutex;

	void onCan(const std::shared_ptr<icsneo::CANMessage>& can) {
		{
			std::lock_guard<std::mutex> lk(t0Mutex);
			if(!t0)
				t0 = can->timestamp;
		}

		double relMs = 0.0;
		{
			std::lock_guard<std::mutex> lk(t0Mutex);
			if(t0 && can->timestamp >= *t0)
				relMs = static_cast<double>(can->timestamp - *t0) / 1e6;
		}

		std::ostringstream oss;
		oss << "  [" << tag << (can->transmitted ? " TX" : " RX") << "] "
			<< std::fixed << std::setprecision(3) << relMs << " ms  "
			<< can->network << "  0x" << std::hex << std::uppercase
			<< std::setw(isExtended(can->arbid) ? 8 : 3) << std::setfill('0') << can->arbid
			<< std::dec << " [" << can->data.size() << "]  " << hexBytes(can->data);
		const auto pci = pciDescribe(can->data);
		if(!pci.empty())
			oss << "  " << pci;
		logLine(oss.str());
	}
};

void addCanSniffer(icsneo::Device& device, Sniffer& sniffer) {
	auto filter = std::make_shared<icsneo::MessageFilter>(icsneo::Network::NetID::DWCAN_01);
	device.addMessageCallback(std::make_shared<icsneo::MessageCallback>(filter, [&sniffer](std::shared_ptr<icsneo::Message> message) {
		if(message->type != icsneo::Message::Type::Frame)
			return;
		auto frame = std::static_pointer_cast<icsneo::Frame>(message);
		if(frame->network.getType() != icsneo::Network::Type::CAN)
			return;
		sniffer.onCan(std::static_pointer_cast<icsneo::CANMessage>(message));
	}));
}

bool openAndOnline(std::shared_ptr<icsneo::Device>& device, const Options& opt) {
	logLine(std::string("Opening ") + device->describe() + " ...");
	if(!device->open()) {
		printApiResult("open", false);
		return false;
	}
	if(!opt.keepBaud && device->settings) {
		if(!device->settings->setBaudrateFor(icsneo::Network::NetID::DWCAN_01, opt.baud))
			printApiResult("setBaudrateFor", false);
		else if(!device->settings->apply(true))
			printApiResult("settings->apply(temporary)", false);
	}
	if(!device->goOnline()) {
		printApiResult("goOnline", false);
		device->close();
		return false;
	}
	return true;
}

void closeDevice(const std::shared_ptr<icsneo::Device>& device) {
	if(!device)
		return;
	device->iso15765DisableAll();
	if(device->isOnline())
		device->goOffline();
	if(device->isOpen())
		device->close();
}

bool enableIso(icsneo::Device& device, const char* who) {
	const bool ok = device.iso15765Enable(icsneo::Network(icsneo::Network::NetID::DWCAN_01));
	printApiResult((std::string(who) + " ISO15765_Enable").c_str(), ok);
	return ok;
}

bool receiveIso(icsneo::Device& device, const Options& opt) {
	icsneo::Iso15765MessageArgs msg;
	fillIso15765(msg, opt, {}, true);
	const bool ok = device.iso15765SetupRxFlowControl(icsneo::Network(icsneo::Network::NetID::DWCAN_01), msg);
	printApiResult("ECU ISO15765_ReceiveMessage", ok);
	return ok;
}

bool transmitIso(icsneo::Device& device, const Options& opt, const std::vector<uint8_t>& payload) {
	icsneo::Iso15765MessageArgs msg;
	fillIso15765(msg, opt, payload, false);
	const bool ok = device.iso15765TransmitMessage(
		icsneo::Network(icsneo::Network::NetID::DWCAN_01), msg, std::chrono::milliseconds(0));
	printApiResult("tester ISO15765_TransmitMessage", ok);
	return ok;
}

std::shared_ptr<icsneo::Device> findBySerial(const std::vector<std::shared_ptr<icsneo::Device>>& devices, const std::string& serial) {
	for(const auto& d : devices) {
		if(iequals(d->getSerial(), serial))
			return d;
	}
	return nullptr;
}

} // namespace

int main(int argc, char** argv) {
	Options opt;
	try {
		if(!parseArgs(argc, argv, opt))
			return 1;
	} catch(const std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}

	std::cout << "libicsneo " << icsneo::GetVersion() << std::endl;

	auto devices = icsneo::FindAllDevices();
	std::cout << "Found " << devices.size() << " device(s)\n";
	for(const auto& d : devices)
		std::cout << "  " << d->describe() << " serial=" << d->getSerial() << "\n";

	if(devices.size() < 2) {
		std::cerr << "Need two devices connected.\n";
		return 1;
	}

	std::shared_ptr<icsneo::Device> tester;
	std::shared_ptr<icsneo::Device> ecu;
	if(opt.testerSerial.empty() && opt.ecuSerial.empty()) {
		if(devices.size() != 2) {
			std::cerr << "More than two devices found; pass --tester and --ecu serials.\n";
			return 1;
		}
		tester = devices[0];
		ecu = devices[1];
		std::cout << "No serials given; using first as tester, second as ECU.\n";
	} else {
		if(opt.testerSerial.empty() || opt.ecuSerial.empty()) {
			std::cerr << "Pass both --tester and --ecu, or neither.\n";
			return 1;
		}
		tester = findBySerial(devices, opt.testerSerial);
		ecu = findBySerial(devices, opt.ecuSerial);
		if(!tester || !ecu) {
			std::cerr << "Could not match tester/ECU serials.\n";
			return 1;
		}
	}

	const size_t payloadSize = opt.payloadBytes.value_or(defaultPayloadSize(opt.mode));
	const auto payload = makePayload(payloadSize);

	std::cout << "Tester: " << tester->describe() << "\n"
			  << "ECU:    " << ecu->describe() << "\n";
	if(opt.mode != Mode::Raw) {
		std::ostringstream hdr;
		hdr << "CAN1 TX 0x" << std::hex << std::uppercase << opt.txId
			<< "  FC 0x" << opt.fcId << std::dec
			<< "  payload " << payloadSize << " bytes";
		std::cout << hdr.str() << "\n";
	}
	std::cout << std::endl;

	if(!openAndOnline(tester, opt) || !openAndOnline(ecu, opt))
		return 1;

	Sniffer testerSniff;
	testerSniff.tag = "TESTER";
	Sniffer ecuSniff;
	ecuSniff.tag = "ECU   ";
	addCanSniffer(*tester, testerSniff);
	addCanSniffer(*ecu, ecuSniff);

	switch(opt.mode) {
		case Mode::Raw:
			logLine("Mode --raw: tester 0x123, then ECU 0x456");
			transmitCan(*tester, 0x123, { 0x11, 0x22, 0x33, 0x44 });
			std::this_thread::sleep_for(200ms);
			transmitCan(*ecu, 0x456, { 0x55, 0x66, 0x77, 0x88 });
			break;
		case Mode::SingleFrame:
			logLine("Mode --sf: firmware ISO-TP single frame from tester");
			if(payload.size() > 7)
				logLine("  warning: payload > 7 bytes will be a First Frame (no flow control in this mode)");
			enableIso(*tester, "tester");
			transmitIso(*tester, opt, payload);
			break;
		case Mode::Both:
			logLine("Mode --both: firmware TX on tester, firmware flow-control on ECU");
			enableIso(*tester, "tester");
			enableIso(*ecu, "ECU");
			receiveIso(*ecu, opt);
			std::this_thread::sleep_for(100ms);
			transmitIso(*tester, opt, payload);
			break;
		case Mode::None:
			break;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(opt.listenMs));

	closeDevice(tester);
	closeDevice(ecu);
	return 0;
}
