// libicsneo A2B example
// Example must be ran with rada2b as slave on TDM4 32 bit channel size and one ADI master node
// Options:
// -h, --help   Display help message.
// -e, --example [EXAMPLE_NUM] Example to run.
// Example usage: ./libicsneocpp-a2b.exe --example 1
// Example usage: ./libicsneocpp-a2b.exe -h

#include <iostream>
#include <fstream>
#include <icsneo/icsneocpp.h>
#include <icsneo/device/tree/rada2b/rada2bsettings.h>
#include <icsneo/communication/message/callback/streamoutput/a2bdecoder.h>
#include <string>

static constexpr size_t numFramesInWave = 48;

std::string makeWave() {
	icsneo::WaveFileHeader header = icsneo::WaveFileHeader(1, 48000, 24);
	std::vector<uint8_t> sineWaveSamples = {
		0x00,	0x2B,	0x98,	0x08,	0x25,	0x01,	0x10,	0xD3,	0xEF,	0x18,	0x40,	0xA3,	0x20,	0x33,	0x4C, 	0x26,
		0xCE,	0xCA,	0x2D,	0x5B,	0x41,	0x32,	0xB9,	0x3C,	0x37,	0x6E,	0x50,	0x3B,	0x29,	0x18,	0x3D,	0xC2,
		0x96,	0x3F,	0x86,	0xD3,	0x3F,	0xEC,	0x35,	0x3F,	0x85,	0xA7,	0x3D,	0xC4,	0x19,	0x3B,	0x28,	0xC9,
		0x37,	0x6B,	0x5A,	0x32,	0xC1,	0xC4,	0x2D,	0x4A,	0xEE,	0x26,	0xE8,	0xB1,	0x20,	0x0E,	0xCF,	0x18,
		0x6F,	0xAA,	0x10,	0x9C,	0x17,	0x08,	0x53,	0x35,	0x00,	0x01,	0xFD,	0xF7,	0xA9,	0x29,	0xEF,	0x66,
		0x87,	0xE7,	0x8F,	0x37,	0xDF,	0xF0,	0x8A,	0xD9,	0x19,	0xB3,	0xD2,	0xB1,	0x3E,	0xCD,	0x42,	0xE9,
		0xC8,	0x8F,	0xE0,	0xC4,	0xDB,	0x39,	0xC2,	0x39,	0x78,	0xC0,	0x7A,	0x8A,	0xC0,	0x16,	0x38,	0xC0,
		0x74,	0x18,	0xC2,	0x44,	0xBF,	0xC4,	0xCE,	0x26,	0xC8,	0x9A,	0x99,	0xCD,	0x3F,	0x53,	0xD2,	0xA8,
		0xBD,	0xD9,	0x32,	0xF5,	0xDF,	0xC2,	0x68,	0xE7,	0xD5,	0xFD,	0xEF,	0x02,	0x15,	0xF8,	0x3B,	0x33,
	};

	std::vector<uint8_t> sineWave;
	sineWave.reserve(sineWaveSamples.size() + sizeof(header));

	sineWave.insert(sineWave.begin(), (uint8_t*)&header, (uint8_t*)(&header) + sizeof(header));
	std::copy(sineWaveSamples.begin(), sineWaveSamples.end(), std::back_inserter(sineWave));

	return std::string(sineWave.begin(), sineWave.end());
}

// Example 0: TX
void example0(std::shared_ptr<icsneo::Device>& rada2b) { 
	std::cout << "Transmitting a sine wave..." << std::endl;

	// Create sine tone in wave format
	std::string waveString = makeWave();

	// Audio map to map which channel in wave to stream on a2b bus.
	icsneo::A2BAudioChannelMap a2bmap(4);
	
	
	a2bmap.set(
		2, // Channel on a2b bus
		icsneo::A2BMessage::A2BDirection::Downstream, // Direction
		0 // Channel in wave file
	);

	a2bmap.setAll(0);

	icsneo::A2BDecoder decoder(
		std::make_unique<std::istringstream>(waveString), // Wave file stream
		false, // True when using 16 bit samples
		a2bmap
	);


	double elapsedTime = 0.0;
	// Play roughly 5 seconds of sine tone.
	while(elapsedTime < 5.0) {

		decoder.outputAll(rada2b); // Output entire wave file

		elapsedTime += ((double)numFramesInWave) * 1.0/48000.0;

		decoder.stream->clear();
		decoder.stream->seekg(0, std::ios::beg);
		// Also outputs entire wave file
		while(decoder && elapsedTime < 5.0) {
			auto msg = decoder.decode();
			rada2b->transmit(msg);
			elapsedTime += ((double)msg->getNumFrames())*1.0/48000.0;
		}

		decoder.stream->clear();
		decoder.stream->seekg(0, std::ios::beg);
	}

}

// Example 1: RX
void example1(std::shared_ptr<icsneo::Device>& rada2b) {
	std::cout << "Receiving 5 seconds of audio data..." << std::endl;

	// Add WAV output message callback
	// Saves samples to "out.wav"
	auto handler = rada2b->addMessageCallback(std::make_shared<icsneo::A2BWAVOutput>("out.wav", 48000));

	// Sleep this thread for 5 seconds, message callback still runs
	std::this_thread::sleep_for(std::chrono::seconds(5));

	// Remove callback
	rada2b->removeMessageCallback(handler);
}


// Example 2: RADA2B settings 
void example2(std::shared_ptr<icsneo::Device>& rada2b) {
	uint32_t tdm;
	{
		// Get device settings
		auto* settings = rada2b->settings.get();
		auto* rada2bSettings = (icsneo::RADA2BSettings*)settings;
		
		// Check if monitor mode is enabled
		auto type = rada2bSettings->getNodeType(icsneo::RADA2BSettings::RADA2BDevice::Monitor);
		if(type == icsneo::RADA2BSettings::NodeType::Monitor) {
			std::cout << "Device is in monitor mode" << std::endl;
		}
		else {
			std::cout << "Device is not in monitor mode" << std::endl;
		}

		// Get current tdm mode
		tdm = rada2bSettings->getTDMModeInt(icsneo::RADA2BSettings::RADA2BDevice::Node);
		
		std::cout << "Current tdm mode: " << tdm << std::endl;
		// Set node type to master node.
		rada2bSettings->setNodeType(icsneo::RADA2BSettings::RADA2BDevice::Node, icsneo::RADA2BSettings::NodeType::Master);

		// Set TDM mode to TDM8
		rada2bSettings->setTDMMode(icsneo::RADA2BSettings::RADA2BDevice::Node, icsneo::RADA2BSettings::TDMMode::TDM8);
		
		// Apply local settings to device
		rada2bSettings->apply();
	}
}

// Example 3: A2BMessage API
void example3() {
	icsneo::A2BMessage msg = icsneo::A2BMessage(4, false, 2048); // Create new A2BMessage

	msg[0][0] = 60; // Set sample using operator[][]
	msg[0][3] = 60; // Frame 0, channel 2 upstream
	msg[6][2] = 32; // Frame 6, channel 1 downstream

	// Equivalent to last line
	msg.setSample(icsneo::A2BMessage::A2BDirection::Downstream, 1, 6, 32);

	// Get sample
	std::cout << "Channel 1 downstream sample for frame 6: " << msg.getSample(icsneo::A2BMessage::A2BDirection::Downstream, 1, 6).value() << std::endl; 

	// Get number of frames
	auto numFrames = msg.getNumFrames();
	std::cout << "Num frames: " << numFrames << std::endl;

	icsneo::A2BPCMSample sample1 = 40;
	icsneo::A2BPCMSample sample2 = 60;
	msg.fill(sample1); // Fill whole message with sample 40 

	msg.fillFrame(sample2, numFrames/2); // Fill frame numFrames/2 with sample2

	// Print msg sample contents
	std::cout << "A2B message contents:" << std::endl;
	for(size_t y = 0; y < numFrames; y++) {
		for(size_t x = 0; x < ((size_t)(msg.getNumChannels())*2); x++) { // Num channels including upstream and downstream
			std::cout << msg[y][x] << " ";
		}
		std::cout << std::endl;
	}

	// Set and get bits
	msg.setSyncFrameBit(true);

	std::cout << "Was received from monitor: " << msg.isMonitorMsg() << std::endl;
}


void displayUsage() {
	std::cout << "libicsneo A2B example" << std::endl;
	std::cout << "Example must be ran with rada2b as slave on TDM4 32 bit channel size and one ADI master node" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "-h, --help\tDisplay help message." << std::endl;
	std::cout << "-e, --example [EXAMPLE_NUM]\tExample to run." << std::endl;
	std::cout << "Example usage: ./libicsneocpp-a2b.exe --example 1" << std::endl;
	std::cout << "Example usage: ./libicsneocpp-a2b.exe -h" << std::endl;
	std::cout << std::endl;
	std::cout << "Example options:" << std::endl;
	std::cout << "0\ttx" << std::endl;
	std::cout << "1\trx" << std::endl;
	std::cout << "2\trx split channel" << std::endl;
	std::cout << "3\tA2BMessage API" << std::endl;
}

int main(int argc, char **argv) {
	std::vector<std::string> arguments(argv, argv + argc);
	if(argc > 4 || argc == 1) {
		std::cerr << "Invalid usage." << std::endl;
		displayUsage();
		return EXIT_FAILURE;
	}

	if(std::any_of(arguments.begin(), arguments.end(), [](const std::string& arg) { return arg == "-h" || arg == "--help"; })) {
		displayUsage();
		return EXIT_SUCCESS;
	}
	
	if(arguments[1] != "-e" && arguments[1] != "--example") {
		std::cerr << "Invalid usage." << std::endl;
		displayUsage();
		return EXIT_FAILURE;
	}

	int option = atoi(arguments[2].c_str());

	if(option < 0 || option > 3) {
		std::cerr << "Invalid usage." << std::endl;
		displayUsage();
		return EXIT_FAILURE;
	}

	std::cout << icsneo::GetVersion() << std::endl;
	const auto& devices = icsneo::FindAllDevices();

	auto it = std::find_if(devices.begin(), devices.end(), [&](const auto& dev) {
			const auto& txNetworks = dev->getSupportedTXNetworks();
			const auto& rxNetworks = dev->getSupportedRXNetworks();

			if(std::none_of(txNetworks.begin(), txNetworks.end(), [](const icsneo::Network& net) { return net.getType() == icsneo::Network::Type::A2B; })) {
				return false;
			}

			if(std::none_of(rxNetworks.begin(), rxNetworks.end(), [](const icsneo::Network& net) { return net.getType() == icsneo::Network::Type::A2B; })) {
				return false;
			}

			return true;
		}
	);

	if(it == devices.end()) {
		std::cerr << "Could not find RADA2B." << std::endl;
		return EXIT_FAILURE;
	}

	std::shared_ptr<icsneo::Device> rada2b = *it;
	if(!rada2b->open()) {
		std::cout << "Failed to open RADA2B." << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return EXIT_FAILURE;
	} 
	else {
		std::cout << "Opened RADA2B." << std::endl;
	}

	if(!rada2b->goOnline()) {
		std::cout << "Failed to go online with RADA2B." << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return EXIT_FAILURE;
	}
	else {
		std::cout << "RADA2B online." << std::endl;
	}
	
	switch(option) {
		case 0:
			example0(rada2b);
			break;
		case 1:
			example1(rada2b);
			break;
		case 2:
			example2(rada2b);
			break;
		case 3:
			example3();
			break;
		default:
			break;
	}

	rada2b->goOffline();
	rada2b->close();

	return EXIT_SUCCESS;
}
