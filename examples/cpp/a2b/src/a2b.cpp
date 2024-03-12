/**
 * libicsneo A2B example
 * 
 * Example were made to be ran with RAD-A2B as main node on TDM4 16 bit channel size and one additional sub node (either an ADI board or an additional RAD-A2B).
 * Follow the specific hardware instructions per example to ensure expected output. Be sure to configure the A2B network before running these examples, especially
 * ones which Tx or Rx audio.
 * 
 * Options:
 * -h, --help   Display help message.
 * -e, --example [EXAMPLE_NUM] Example to run.
 * Example usage: ./libicsneocpp-a2b.exe --example 1
 * Example usage: ./libicsneocpp-a2b.exe -h
*/

#include <iostream>
#include <fstream>
#include <icsneo/icsneocpp.h>
#include <icsneo/device/tree/rada2b/rada2bsettings.h>
#include <icsneo/communication/message/callback/streamoutput/a2bwavoutput.h>
#include <string>
#include <math.h>

std::string makeWAV() {
	icsneo::WAVHeader header = icsneo::WAVHeader(1, 48000, 16);
	std::vector<uint8_t> sineWAVSamples = {
		0xFF, 0x3F, 0x81, 0x5A, 0xD9, 0x6E, 0xA2, 0x7B, 0xFF, 0x7F, 0xA2, 0x7B, 0xD9, 0x6E, 0x81, 0x5A,
		0xFF, 0x3F, 0x20, 0x21, 0x00, 0x00, 0xE0, 0xDE, 0x01, 0xC0, 0x7F, 0xA5, 0x27, 0x91, 0x5E, 0x84,
		0x01, 0x80, 0x5E, 0x84, 0x27, 0x91, 0x7F, 0xA5, 0x01, 0xC0, 0xE0, 0xDe, 0x00, 0x00, 0x20, 0x21
	};

	std::vector<uint8_t> sineWAV;
	sineWAV.reserve(sineWAVSamples.size() + sizeof(header));

	sineWAV.insert(sineWAV.begin(), (uint8_t*)&header, (uint8_t*)(&header) + sizeof(header));
	std::copy(sineWAVSamples.begin(), sineWAVSamples.end(), std::back_inserter(sineWAV));

	return std::string(sineWAV.begin(), sineWAV.end());
}

/**
 * Example 0: TX
*/
void example0(const std::shared_ptr<icsneo::Device>& rada2b) { 
	std::cout << "Transmitting a sine tone..." << std::endl;

	// Create sine tone in wav format
	std::string wavString = makeWAV();

	std::istringstream sineWAV(wavString);
	double elapsedTime = 0.0;
	
	// Create a IWAVStream object which represents a WAV data stream
	// the IWAVStream object here is initialized with an outside std::ostream, 
	// so it holds a reference pointer to this stream.
	icsneo::IWAVStream wavStream(sineWAV);

	// Create a channel map which maps each message channel to a channel in the input WAV file
	icsneo::ChannelMap channelMap;

	// Here we will just set every message channel to channel 0 in the WAV file
	// We have 8 channels since this is TDM4 and we include both upstream and downstream
	// see docs for specific message channel labeling information

	for(uint8_t messageChannel = 0; messageChannel < 8; messageChannel++) {
		channelMap[messageChannel] = 0;
	}

	// Play roughly 5 seconds of sine tone.
	while(elapsedTime < 5.0) {
		while(elapsedTime < 5.0) {
			// If WAVStream is invalid (at EOF) break out of loop
			if(!wavStream) {
				break;
			}

			// Creates a new message with the maximum amount of allocated frames
			auto msg = std::make_shared<icsneo::A2BMessage>(
				icsneo::A2BMessage::TDMMode::TDM4, /* TDM mode of the message, we use TDM4 for this whole example*/
				true /* true if we want 16 bit channels in the message, false for 32 bit. This should match the RAD-A2B device setting */
			);
			msg->txmsg = true;
			msg->network = icsneo::Network(icsneo::Network::NetID::A2B2);

			// Load the WAV audio data into the desired channel, break if we fail to load
			if(!msg->loadAudioBuffer(wavStream, channelMap)) {
				break;
			}

			// Transmit the message
			if(!rada2b->transmit(msg)) {
				std::cout << "Failed to transmit." << std::endl;
				break;
			}
			elapsedTime += (static_cast<double>(msg->getNumFrames()))*1.0/48000.0;
		}

		// Reset the WAV stream
		wavStream.reset();
	}
}

/**
 * Example 1: RX
*/
void example1(const std::shared_ptr<icsneo::Device>& rada2b) {
	std::cout << "Receiving 5 seconds of audio data..." << std::endl;

	// Add WAV output message callback
	// Saves samples to "out.wav"
	auto handler = rada2b->addMessageCallback(
		std::make_shared<icsneo::A2BWAVOutput>(
			"audio16bit.wav", /* output file name */
			icsneo::ChannelMap( /** channel mapping which maps our output WAV channels to the message channels from incoming messages */
				{ /* See docs for specific A2B channel indexing information */
					{static_cast<uint8_t>(3u), static_cast<uint8_t>(0u)}, /* Map output WAV channel 3 to channel 0 downstream of the A2B network/A2BMessage */
					{static_cast<uint8_t>(2u), static_cast<uint8_t>(1u)}, /* Map output WAV channel 2 to channel 0 upstream of the A2B network/A2BMessage */
					{static_cast<uint8_t>(1u), static_cast<uint8_t>(2u)}, /* Map output WAV channel 1 to channel 1 downstream of the A2B network/A2BMessage */
					{static_cast<uint8_t>(0u), static_cast<uint8_t>(3u)} /* Map output WAV channel 0 to channel 1 upstream of the A2B network/A2BMessage */
				}
			), 
			icsneo::PCMType::L16, /* store samples with 16 bit resolution*/
			2u, /* Number of channels in the output WAV file */
			48000 /* Sample rate of WAV file */
		)
	);

	// Sleep this thread for 5 seconds, message callback still runs
	std::this_thread::sleep_for(std::chrono::seconds(5));

	// Make sure you send 16 bit audio data on the above message channels in the channel map
	// to the RAD-A2B main node through a microphone or a different modem.
	// You can configure the message channels by changing the stream config in the A2B schematic

	// Remove callback
	rada2b->removeMessageCallback(handler);
}

/**
 * Example 2: RAD-A2B settings
*/
void example2(const std::shared_ptr<icsneo::Device>& rada2b) {
	uint8_t numChannels;
	{
		// Get device settings
		auto* settings = rada2b->settings.get();
		auto* rada2bSettings = static_cast<icsneo::RADA2BSettings*>(settings);
		
		// Check if monitor mode is enabled
		auto type = rada2bSettings->getNodeType(icsneo::RADA2BSettings::RADA2BDevice::Monitor);
		if(type == icsneo::RADA2BSettings::NodeType::Monitor) {
			std::cout << "Device is in monitor mode" << std::endl;
		}
		else {
			std::cout << "Device is not in monitor mode" << std::endl;
		}

		// Get current tdm mode
		numChannels = rada2bSettings->getNumChannels(icsneo::RADA2BSettings::RADA2BDevice::Node);
		
		std::cout << "Current num channels: " << static_cast<uint32_t>(numChannels) << std::endl;
		// Set node type to master node.
		rada2bSettings->setNodeType(icsneo::RADA2BSettings::RADA2BDevice::Node, icsneo::RADA2BSettings::NodeType::Master);

		// Set TDM mode to TDM8
		rada2bSettings->setTDMMode(icsneo::RADA2BSettings::RADA2BDevice::Node, icsneo::RADA2BSettings::TDMMode::TDM4);
		
		// Apply local settings to device
		rada2bSettings->apply();
	}
}

/**
 * Example 3: Packaging and transmitting sine tone using A2BMessage API
*/
void example3(const std::shared_ptr<icsneo::Device>& rada2b) {
	std::cout << "Transmitting a 1000 hz sine tone." << std::endl;

	float deltaTime = static_cast<float>(1.0/48000.0);
	float elapsedTime = 0.0;
	float twoPI = static_cast<float>(2.0*atan(1.0)*4.0);
	float frequency = 1000;
	float amplitude = static_cast<float>((1 << 15) - 1);
	size_t tdm = 4;
	size_t bytesPerSample = 2;

	size_t numFrames = 2048 / (2 * tdm * bytesPerSample);


	// Play for roughly 5 seconds
	while(elapsedTime < 5.0) {
		// Allocate message
		std::shared_ptr<icsneo::A2BMessage> a2bmsgPtr = std::make_shared<icsneo::A2BMessage>(numFrames, icsneo::A2BMessage::TDMMode::TDM4, true);

		icsneo::A2BMessage& a2bmsg = *a2bmsgPtr.get();
		a2bmsg.network = icsneo::Network(icsneo::Network::NetID::A2B2);
		a2bmsg.txmsg = true;

		for(size_t frame = 0; frame < a2bmsg.getNumFrames(); frame++) {
		
			// Sine tone sample, amplitude 1000, frequency 1000 hz

			float contSample = amplitude*sin(twoPI*frequency*elapsedTime);
			icsneo::PCMSample sample = static_cast<icsneo::PCMSample>(contSample);

			// Send this sine wave sample downstream on channels 0, 1, and 2
			a2bmsg.setChannelSample(
				icsneo::A2BMessage::Direction::Downstream,
				0,
				frame,
				sample,
				icsneo::PCMType::L16
			);

			a2bmsg.setChannelSample(
				icsneo::A2BMessage::Direction::Downstream,
				1,
				frame,
				sample,
				icsneo::PCMType::L16
			);

			a2bmsg.setChannelSample(
				icsneo::A2BMessage::Direction::Downstream,
				2,
				frame,
				sample,
				icsneo::PCMType::L16
			);

			elapsedTime+=deltaTime;
		}

		// Transmit message to device

		if(!rada2b->transmit(a2bmsgPtr)) {
			std::cout << "Failed to transmit." << std::endl;
		}
	}


}

/**
 * Example 4: Retrieving A2B bus status using I2C messaages.
*/
void example4(const std::shared_ptr<icsneo::Device>& rada2b) {
	std::shared_ptr<icsneo::I2CMessage> msg = std::make_shared<icsneo::I2CMessage>();
	std::shared_ptr<icsneo::MessageFilter> msgFilter = std::make_shared<icsneo::MessageFilter>(icsneo::Network::NetID::I2C2);

	msg->network = icsneo::Network(icsneo::Network::NetID::I2C2);
	msg->controlBytes.resize(1);
	msg->controlBytes[0] = static_cast<uint8_t>(0x17u); // Register address for A2B INTTYPE
	msg->dataBytes.resize(1, 0);
	msg->direction = icsneo::I2CMessage::Direction::Read;
	msg->deviceMode = icsneo::I2CMessage::DeviceMode::Controller;
	msg->address = static_cast<uint16_t>(0x68); // A2B master node address.
	msg->isTXMsg = true;
	
	auto handler = rada2b->addMessageCallback(std::make_shared<icsneo::MessageCallback>(
		[] (std::shared_ptr<icsneo::Message> newMsg) {

			if(newMsg->type == icsneo::Message::Type::Frame) {
				const auto& frame = std::dynamic_pointer_cast<icsneo::Frame>(newMsg);
				if(frame && frame->network.getNetID() == icsneo::Network::NetID::I2C2) {
					const auto& i2cMessage = std::dynamic_pointer_cast<icsneo::I2CMessage>(frame);

					if(!i2cMessage) {
						return;
					}

					if(i2cMessage->controlBytes.size() == 1 && i2cMessage->direction == icsneo::I2CMessage::Direction::Read) {
						if(i2cMessage->controlBytes[0] == 0x17u) {
							if(i2cMessage->dataBytes.size() == 1) {
								std::cout << "Current A2B bus status INTTYPE code: " << static_cast<int>(i2cMessage->dataBytes[0]) << '\n'; 
							}
						} else if(i2cMessage->controlBytes[0] == 0x03u) {
							if(i2cMessage->dataBytes.size() == 1) {
								std::cout << "A2B_PRODUCT register: " << static_cast<int>(i2cMessage->dataBytes[0]) << std::endl;
							}
						} else if(i2cMessage->controlBytes[0] == 0x02u) {
							if(i2cMessage->dataBytes.size() == 1) {
								std::cout << "A2B_VENDOR register: " << static_cast<int>(i2cMessage->dataBytes[0]) << std::endl;
							}
						} else if(i2cMessage->controlBytes[0] == 0x1C) {
							if(i2cMessage->dataBytes.size() == 1) {
								std::cout << "A2B_INTMSK1 register value: " << static_cast<int>(i2cMessage->dataBytes[0]) << std::endl;
							}
						}
					}
				}
			}
		}
	, msgFilter));
	if(!rada2b->transmit(msg)) {
		std::cout << "Failed to transmit." << std::endl;
	}
	msg->controlBytes[0] = 0x03; // Address for A2B_PRODUCT register

	if(!rada2b->transmit(msg)) {
		std::cout << "Failed to transmit." << std::endl;
	}

	msg->controlBytes[0] = 0x02; // Address for A2B_VENDOR register
	if(!rada2b->transmit(msg)) {
		std::cout << "Failed to transmit." << std::endl;
	}

	msg->controlBytes[0] = 0x1C ; // Address for A2B_INTMSK1 register
	msg->dataBytes[0] = 0x11;
	msg->direction = icsneo::I2CMessage::Direction::Write;
	// Write register
	if(!rada2b->transmit(msg)) {
		std::cout << "Failed to transmit" << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	msg->direction = icsneo::I2CMessage::Direction::Read;

	// Read register
	if(!rada2b->transmit(msg)) {
		std::cout << "Failed to transmit." << std::endl;
	}
	
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	rada2b->removeMessageCallback(handler);
}

/**
 * Example 5: Reading A2B sequence chart .puml file
*/
void example5(const std::shared_ptr<icsneo::Device>& rada2b) {
	// The A2B sequence chart is located at binary index 0
	constexpr uint16_t a2bSequenceChartIndex = 0;

	// Create a ostream object to capture sequence chart data
	std::ofstream a2bSequenceChart("a2b_sequence_chart.puml", std::ios::out | std::ios::binary);
	
	if(!rada2b->readBinaryFile(a2bSequenceChart, a2bSequenceChartIndex)) {
		std::cout << "Failed to read A2B sequence chart" << std::endl;
	}
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
	std::cout << "2\tSet RAD-A2B settings" << std::endl;
	std::cout << "3\tPackaging and transmitting sine wav using A2BMessage API" << std::endl;
	std::cout << "4\tRead/write I2C registers on A2B board" << std::endl;
	std::cout << "5\tReading out A2B sequence chart .puml file" << std::endl;
}

int main(int argc, char** argv) {
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

	if(option < 0 || option > 5) {
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
		std::cerr << "Could not find RAD-A2B." << std::endl;
		return EXIT_FAILURE;
	}

	std::shared_ptr<icsneo::Device> rada2b = *it;
	if(!rada2b->open()) {
		std::cout << "Failed to open RAD-A2B." << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return EXIT_FAILURE;
	} 
	else {
		std::cout << "Opened RAD-A2B." << std::endl;
	}

	if(!rada2b->goOnline()) {
		std::cout << "Failed to go online with RAD-A2B." << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return EXIT_FAILURE;
	}
	else {
		std::cout << "RAD-A2B online." << std::endl;
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
			example3(rada2b);
			break;
		case 4:
			example4(rada2b);
			break;
		case 5:
			example5(rada2b);
			break;
		default:
			break;
	}

	rada2b->goOffline();
	rada2b->close();

	return EXIT_SUCCESS;
}
