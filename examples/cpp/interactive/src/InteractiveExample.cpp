#include <iostream>
#include <string>
#include <ctype.h>
#include <vector>
#include <map>

// Include icsneo/icsneocpp.h to access library functions
#include "icsneo/icsneocpp.h"

/**
 * \brief Prints all current known devices to output in the following format:
 * [num] DeviceType SerialNum    Connected: Yes/No    Online: Yes/No    Msg Polling: On/Off
 *
 * If any devices could not be described due to an error, they will appear in the following format:
 * Description for device num not available!
 */
void printAllDevices(const std::vector<std::shared_ptr<icsneo::Device>>& devices) {
	if(devices.size() == 0) {
		std::cout << "No devices found! Please scan for new devices." << std::endl;
	}

	int index = 1;
	for(auto device : devices) {
		std::cout << "[" << index << "] " << device->describe() << "\tConnected: " << (device->isOpen() ? "Yes\t" : "No\t");
		std::cout << "Online: " << (device->isOnline() ? "Yes\t" : "No\t");
		std::cout << "Msg Polling: " << (device->isMessagePollingEnabled() ? "On" : "Off") << std::endl;
		index++;
	}
}

// Prints the main menu options to output
void printMainMenu() {
	std::cout << "Press the letter next to the function you want to use:" << std::endl;
	std::cout << "A - List all devices" << std::endl;
	std::cout << "B - Find all devices" << std::endl;
	std::cout << "C - Open/close" << std::endl;
	std::cout << "D - Go online/offline" << std::endl;
	std::cout << "E - Enable/disable message polling" << std::endl;
	std::cout << "F - Get messages" << std::endl;
	std::cout << "G - Send messages" << std::endl;
	std::cout << "H - Get events" << std::endl;
	std::cout << "I - Set HS CAN to 250K" << std::endl;
	std::cout << "J - Set LSFT CAN to 250K" << std::endl;
	std::cout << "K - Add/Remove a message callback" << std::endl;
	std::cout << "L - Set Digital IO" << std::endl;
	std::cout << "M - Set HS CAN termination" << std::endl;
	std::cout << "X - Exit" << std::endl;
}

/**
 * \brief Gets all current API events (info and warning level) and prints them to output
 * Flushes all current API events, meaning future calls (barring any new events) will not detect any further API events
 */
void printAPIEvents() {
	// Match all events
	auto events = icsneo::GetEvents(icsneo::EventFilter());

	if(events.size() == 1) {
		std::cout << "1 API event found!" << std::endl;
	} else {
		std::cout << events.size() << " API events found!" << std::endl;
	}

	for(auto event : events) {
		std::cout << event << std::endl;
	}
}

/**
 * \brief Gets all current API warnings and prints them to output
 * Flushes all current API warnings, meaning future calls (barring any new warnings) will not detect any further API warnings
 */
void printAPIWarnings() {
	// Match all warning events, regardless of device
	auto warnings = icsneo::GetEvents(icsneo::EventFilter(nullptr, icsneo::APIEvent::Severity::EventWarning));

	if(warnings.size() == 1) {
		std::cout << "1 API warning found!" << std::endl;
	} else {
		std::cout << warnings.size() << " API warnings found!" << std::endl;
	}

	for(auto warning : warnings) {
		std::cout << warning << std::endl;
	}
}

/**
 * \brief Gets all current device events and prints them to output.
 * Flushes all current device events, meaning future calls (barring any new events) will not detect any further device events for this device
 */
void printDeviceEvents(std::shared_ptr<icsneo::Device> device) {
	// Match all events for the specified device
	auto events = icsneo::GetEvents(icsneo::EventFilter(device.get()));

	if(events.size() == 1) {
		std::cout << "1 device event found!" << std::endl;
	} else {
		std::cout << events.size() << " device events found!" << std::endl;
	}

	for(auto event : events) {
		std::cout << event << std::endl;
	}
}

/**
 * \brief Gets all current device warnings and prints them to output.
 * Flushes all current device warnings, meaning future calls (barring any new warnings) will not detect any further device warnings for this device
 */
void printDeviceWarnings(std::shared_ptr<icsneo::Device> device) {
	// Match all warning events for the specified device
	auto events = icsneo::GetEvents(icsneo::EventFilter(device.get(), icsneo::APIEvent::Severity::EventWarning));

	if(events.size() == 1) {
		std::cout << "1 device warning found!" << std::endl;
	} else {
		std::cout << events.size() << " device warnings found!" << std::endl;
	}

	for(auto event : events) {
		std::cout << event << std::endl;
	}
}

/**
 * \brief Used to check character inputs for correctness (if they are found in an expected list)
 * \param[in] numArgs the number of possible options for the expected character
 * \param[in] ... the possible options for the expected character
 * \returns the entered character
 *
 * This function repeatedly prompts the user for input until a matching input is entered
 * Example usage:
 * char input = getCharInput(std::vector<char> {'F', 'u', 'b', 'a', 'r'});
 */
char getCharInput(std::vector<char> allowed) {
	bool found = false;
	std::string input;

	while(!found) {
		std::cin >> input;

		if(input.length() == 1) {
			for(char compare : allowed) {
				if(compare == input.c_str()[0]) {
					found = true;
					break;
				}
			}
		}

		if(!found) {
			std::cout << "Input did not match expected options. Please try again." << std::endl;
		}
	}

	return input.c_str()[0];
}

/**
 * \brief Prompts the user to select a device from the list of currently known devices
 * \returns a pointer to the device in devices[] selected by the user
 * Requires an input from 1-9, so a maximum of 9 devices are supported
 */
std::shared_ptr<icsneo::Device> selectDevice(const std::vector<std::shared_ptr<icsneo::Device>>& from) {
	printf("Please select a device:\n");
	printAllDevices(from);
	printf("\n");

	int selectedDeviceNum = 10;

	while((size_t) selectedDeviceNum > from.size()) {
		char deviceSelection = getCharInput(std::vector<char> {'1', '2', '3', '4', '5', '6', '7', '8', '9'});
		selectedDeviceNum = deviceSelection - '0';
		if((size_t) selectedDeviceNum > from.size()) {
			std::cout << "Selected device out of range!" << std::endl;
		}
	}

	std::cout << std::endl;

	return from.at(selectedDeviceNum - 1);
}

int main() {
	std::cout << "Running libicsneo " << icsneo::GetVersion() << std::endl << std::endl;

	size_t msgLimit = 50000;
	std::vector<std::shared_ptr<icsneo::Device>> devices;
	std::map<std::shared_ptr<icsneo::Device>, std::vector<int>> callbacks;
	std::shared_ptr<icsneo::Device> selectedDevice;

	while(true) {
		printMainMenu();
		std::cout << std::endl;
		char input = getCharInput(std::vector<char> {'A', 'a', 'B', 'b', 'C', 'c', 'D', 'd', 'E', 'e', 'F', 'f', 'G', 'g', 'H', 'h', 'I', 'i', 'J', 'j', 'K', 'k', 'L', 'l', 'M', 'm', 'X', 'x'});
		std::cout << std::endl;

		switch(input) {
		// List current devices
		case 'A':
		case 'a':
			printAllDevices(devices);
			std::cout << std::endl;
			break;
		// Find all devices
		case 'B':
		case 'b':
		{
			devices = icsneo::FindAllDevices();

			for(auto device : devices) {
				callbacks.insert({device, std::vector<int>()});
			}

			if(devices.size() == 1) {
				std::cout << "1 device found!" << std::endl;
			} else {
				std::cout << devices.size() << " devices found!" << std::endl;
			}
			printAllDevices(devices);
			std::cout << std::endl;
			break;
		}
		// Open/Close
		case 'C':
		case 'c':
		{
			// Select a device and get its description
			if(devices.size() == 0) {
				std::cout << "No devices found! Please scan for new devices." << std::endl << std::endl;
				break;
			}
			selectedDevice = selectDevice(devices);

			std::cout << "Would you like to open or close " << selectedDevice->describe() << "?" << std::endl;
			std::cout << "[1] Open" << std::endl << "[2] Close" << std::endl <<  "[3] Cancel" << std::endl << std::endl;
			char selection = getCharInput(std::vector<char> {'1', '2', '3'});
			std::cout << std::endl;
			
			switch(selection) {
			case '1':
				if(selectedDevice->open()) {
					std::cout << selectedDevice->describe() << " successfully opened!" << std::endl << std::endl;
				} else {
					std::cout << selectedDevice->describe() << " failed to open!" << std::endl << std::endl;
					std::cout << icsneo::GetLastError() << std::endl;
					std::cout << std::endl;
				}
				
				break;
			case '2':
				// Attempt to close the device
				if(selectedDevice->close()) {
					std::cout << "Successfully closed " << selectedDevice->describe() << "!" << std::endl << std::endl;
					selectedDevice = NULL;
				} else {
					std::cout << "Failed to close " << selectedDevice->describe() << "!" << std::endl << std::endl;
					std::cout << icsneo::GetLastError() << std::endl;;
					std::cout << std::endl;
				}
				
				break;
			default:
				std::cout << "Canceling!" << std::endl << std::endl;
				break;
			}
		}
		break;
		// Go online/offline
		case 'D':
		case 'd':
		{
			// Select a device and get its description
			if(devices.size() == 0) {
				std::cout << "No devices found! Please scan for new devices." << std::endl << std::endl;
				break;
			}
			selectedDevice = selectDevice(devices);

			std::cout << "Would you like to have " << selectedDevice->describe() << " go online or offline?" << std::endl;
			std::cout << "[1] Online" << std::endl << "[2] Offline" << std::endl << "[3] Cancel" << std::endl << std::endl;
			char selection = getCharInput(std::vector<char> {'1', '2', '3'});
			std::cout << std::endl;

			switch(selection) {
			case '1':
				// Attempt to have the selected device go online
				if(selectedDevice->goOnline()) {
					std::cout << selectedDevice->describe() << " successfully went online!" << std::endl << std::endl;
				} else {
					std::cout << selectedDevice->describe() << " failed to go online!" << std::endl << std::endl;
					std::cout << icsneo::GetLastError() << std::endl;;
					std::cout << std::endl;
				}
				break;
			case '2':
				// Attempt to go offline
				if(selectedDevice->goOffline()) {
					std::cout << selectedDevice->describe() << " successfully went offline!" << std::endl << std::endl;
				} else {
					std::cout << selectedDevice->describe() << " failed to go offline!" << std::endl << std::endl;
					std::cout << icsneo::GetLastError() << std::endl;;
					std::cout << std::endl;
				}
				break;
			default:
				std::cout << "Canceling!" << std::endl << std::endl;
				break;
			}
		}
		break;
		// Enable/disable message polling
		case 'E':
		case 'e':
		{
			// Select a device and get its description
			if(devices.size() == 0) {
				std::cout << "No devices found! Please scan for new devices." << std::endl << std::endl;
				break;
			}
			selectedDevice = selectDevice(devices);

			std::cout << "Would you like to enable or disable message polling for " << selectedDevice->describe() << "?" << std::endl;
			std::cout << "[1] Enable" << std::endl << "[2] Disable" << std::endl << "[3] Cancel" << std::endl << std::endl;
			char selection = getCharInput(std::vector<char> {'1', '2', '3'});
			std::cout << std::endl;

			switch(selection) {
			case '1':
				// Attempt to enable message polling
				if(selectedDevice->enableMessagePolling()) {
					std::cout << "Successfully enabled message polling for " << selectedDevice->describe() << "!" << std::endl << std::endl;
				} else {
					std::cout << "Failed to enable message polling for " << selectedDevice->describe() << "!" << std::endl << std::endl;
					std::cout << icsneo::GetLastError() << std::endl;;
					std::cout << std::endl;
				}
				// Manually setting the polling message limit as done below is optional
				// It will default to 20k if not set
				selectedDevice->setPollingMessageLimit(50000);
				if(selectedDevice->getPollingMessageLimit() == 50000) {
					std::cout << "Successfully set polling message limit for " << selectedDevice->describe() << "!" << std::endl << std::endl;
				} else {
					std::cout << "Failed to set polling message limit for " << selectedDevice->describe() << "!" << std::endl << std::endl;
					std::cout << icsneo::GetLastError() << std::endl;;
					std::cout << std::endl;
				}
				break;
			case '2':
				// Attempt to disable message polling
				if(selectedDevice->disableMessagePolling()) {
					std::cout << "Successfully disabled message polling for " << selectedDevice->describe() << "!" << std::endl;
				} else {
					std::cout << "Failed to disable message polling for " << selectedDevice->describe() << "!" << std::endl << std::endl;
					std::cout << icsneo::GetLastError() << std::endl;;
					std::cout << std::endl;
				}
				break;
			default:
				std::cout << "Canceling!" << std::endl << std::endl;
				break;
			}			
		}
		break;
		// Get messages
		case 'F':
		case 'f':
		{
			// Select a device and get its description
			if(devices.size() == 0) {
				std::cout << "No devices found! Please scan for new devices." << std::endl << std::endl;
				break;
			}
			selectedDevice = selectDevice(devices);

			std::vector<std::shared_ptr<icsneo::Message>> msgs;

			// Attempt to get messages, limiting the number of messages at once to 50,000
			// A third parameter of type std::chrono::milliseconds is also accepted if a timeout is desired
			if(!selectedDevice->getMessages(msgs, msgLimit)) {
				std::cout << "Failed to get messages for " << selectedDevice->describe() << "!" << std::endl << std::endl;
				std::cout << icsneo::GetLastError() << std::endl;;
				std::cout << std::endl;
				break;
			}

			if(msgs.size() == 1) {
				std::cout << "1 message received from " << selectedDevice->describe() << "!" << std::endl;
			} else {
				std::cout << msgs.size() << " messages received from " << selectedDevice->describe() << "!" << std::endl;
			}

			// Print out the received messages
			for(auto msg : msgs) {
				switch(msg->network.getType()) {
				case icsneo::Network::Type::CAN:
				{
					// A message of type CAN is guaranteed to be a CANMessage, so we can static cast safely
					auto canMsg = std::static_pointer_cast<icsneo::CANMessage>(msg);
					std::cout << "\t0x" << std::setfill('0') << std::setw(3) << std::hex << (int) canMsg->arbid << " [" << canMsg->data.size() << "] " << std::dec;
					
					for(auto data : canMsg->data) {
						std::cout << std::setfill('0') << std::setw(2) << std::hex << (int) data << " " << std::dec;
					}

					std::cout << canMsg->timestamp << std::endl;
					break;
				}
				default:
					if(msg->network.getNetID() != icsneo::Network::NetID::Device) {
						std::cout << "\tMessage on netid " << msg->network.GetNetIDString(msg->network.getNetID()) << " with length " << msg->data.size() << std::endl;
					}
					break;
				}
			}
						
			std::cout << std::endl;
		}

		break;
		// Send messages
		case 'G':
		case 'g':
		{
			// Select a device and get its description
			if(devices.size() == 0) {
				std::cout << "No devices found! Please scan for new devices." << std::endl << std::endl;
				break;
			}
			selectedDevice = selectDevice(devices);

			std::cout << "Transmitting a normal CAN frame..." << std::endl;
			auto msg = std::make_shared<icsneo::CANMessage>();
			msg->network = icsneo::Network::NetID::HSCAN;
			msg->arbid = 0x120;
			msg->data.insert(msg->data.end(), {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff});
			msg->isExtended = false;
			msg->isCANFD = false;

			// Attempt to transmit the sample msg
			if(selectedDevice->transmit(msg)) {
				std::cout << "Message transmit successful!" << std::endl;
			} else {
				std::cout << "Failed to transmit message to " << selectedDevice->describe() << "!" << std::endl << std::endl;
				std::cout << icsneo::GetLastError() << std::endl;;
			}
			
			/** Example of CAN FD
			std::cout << "Transmitting an extended CAN FD frame... " << std::endl;
			auto txMessage = std::make_shared<icsneo::CANMessage>();
			txMessage->network = icsneo::Network::NetID::HSCAN;
			txMessage->arbid = 0x1C5001C5;
			txMessage->data.insert(txMessage->data.end(), {0xaa, 0xbb, 0xcc});
			// The DLC will come from the length of the data vector
			txMessage->isExtended = true;
			txMessage->isCANFD = true;

			// Attempt to transmit the sample msg
			if(selectedDevice->transmit(txMessage)) {
				std::cout << "Extended CAN FD frame transmit successful!" << std::endl;
			} else {
				std::cout << "Failed to transmit extended CAN FD frame to " << selectedDevice->describe() << "!" << std::endl << std::endl;
				std::cout << icsneo::GetLastError() << std::endl;;
			}
			*/

			std::cout << std::endl;
		}
		break;
		// Get events
		case 'H':
		case 'h':
		{
			// Prints all events
			printAPIEvents();
			std::cout << std::endl;
		}
		break;
		// Set HS CAN to 250k
		case 'I':
		case 'i':
		{
			// Select a device and get its description
			if(devices.size() == 0) {
				std::cout << "No devices found! Please scan for new devices." << std::endl << std::endl;
				break;
			}
			selectedDevice = selectDevice(devices);

			// Attempt to set baudrate and apply settings
			if(selectedDevice->settings->setBaudrateFor(icsneo::Network::NetID::HSCAN, 250000) && selectedDevice->settings->apply()) {
				std::cout << "Successfully set HS CAN baudrate for " << selectedDevice->describe() << " to 250k!" << std::endl;
			} else {
				std::cout << "Failed to set HS CAN baudrate for " << selectedDevice->describe() << " to 250k!" << std::endl << std::endl;
				std::cout << icsneo::GetLastError() << std::endl;;
			}
			std::cout << std::endl;
		}
		break;
		// Set LSFT CAN to 250k
		case 'J':
		case 'j':
		{
			// Select a device and get its description
			if(devices.size() == 0) {
				std::cout << "No devices found! Please scan for new devices." << std::endl << std::endl;
				break;
			}
			selectedDevice = selectDevice(devices);

			// Attempt to set baudrate and apply settings
			if(selectedDevice->settings->setBaudrateFor(icsneo::Network::NetID::LSFTCAN, 250000) && selectedDevice->settings->apply()) {
				std::cout << "Successfully set LSFT CAN baudrate for " << selectedDevice->describe() << " to 250k!" << std::endl;
			} else {
				std::cout << "Failed to set LSFT CAN baudrate for " << selectedDevice->describe() << " to 250k!" << std::endl << std::endl;
				std::cout << icsneo::GetLastError() << std::endl;;
			}
			std::cout << std::endl;
		}
		break;
		// Add/Remove a message callback
		case 'K':
		case 'k':
		{
			// Select a device and get its description
			if(devices.size() == 0) {
				std::cout << "No devices found! Please scan for new devices." << std::endl << std::endl;
				break;
			}
			selectedDevice = selectDevice(devices);

			std::cout << "Would you like to add or remove a message callback for " << selectedDevice->describe() << "?" << std::endl;
			std::cout << "[1] Add" << std::endl << "[2] Remove" << std::endl <<  "[3] Cancel" << std::endl << std::endl;
			char selection = getCharInput(std::vector<char> {'1', '2', '3'});
			std::cout << std::endl;

			switch(selection) {
			case '1':
			{
				// Shameless copy-paste from get messages above, demonstrating a callback
				int callbackID = selectedDevice->addMessageCallback(icsneo::MessageCallback([](std::shared_ptr<icsneo::Message> msg){
					switch(msg->network.getType()) {
					case icsneo::Network::Type::CAN:
					{
						// A message of type CAN is guaranteed to be a CANMessage, so we can static cast safely
						auto canMsg = std::static_pointer_cast<icsneo::CANMessage>(msg);
						std::cout << "\t0x" << std::setfill('0') << std::setw(3) << std::hex << (int) canMsg->arbid << " [" << canMsg->data.size() << "] " << std::dec;
						
						for(auto data : canMsg->data) {
							std::cout << std::setfill('0') << std::setw(2) << std::hex << (int) data << " " << std::dec;
						}

						std::cout << canMsg->timestamp << std::endl;
						break;
					}
					default:
						if(msg->network.getNetID() != icsneo::Network::NetID::Device) {
							std::cout << "\tMessage on netid " << msg->network.GetNetIDString(msg->network.getNetID()) << " with length " << msg->data.size() << std::endl;
						}
						break;
					}
				}));

				if(callbackID != -1) {
					std::cout << "Successfully added message callback to " << selectedDevice->describe() << "!" << std::endl;
					callbacks.find(selectedDevice)->second.push_back(callbackID);
				} else {
					std::cout << "Failed to add message callback to " << selectedDevice->describe() << "!" << std::endl << std::endl;
					std::cout << icsneo::GetLastError() << std::endl;;
				}
			}
			break;
			case '2':
			{
				if(callbacks.find(selectedDevice)->second.size() == 0) {
					std::cout << "No callbacks found for " << selectedDevice->describe() << "!" << std::endl;
					break;
				} else {
					std::vector<char> allowed;
					
					std::cout << "Which id would you like to remove?" << std::endl;
					for(int id : callbacks.find(selectedDevice)->second) {
						allowed.push_back(static_cast<char>(id) + '0');
						std::cout << "[" << id << "]" << std::endl;
					}
					std::cout << std::endl;


					int removeID = getCharInput(allowed) - '0';
					std::cout << std::endl;

					if(selectedDevice->removeMessageCallback(removeID)) {
						std::cout << "Successfully removed callback id " << removeID << " from " << selectedDevice->describe() << "!" << std::endl;
					} else {
						std::cout << "Failed to remove message callback id " << removeID << " from " << selectedDevice->describe() << "!" << std::endl << std::endl;
						std::cout << icsneo::GetLastError() << std::endl;;
					}					
				}
			}
			break;
			default:
				std::cout << "Canceling!" << std::endl << std::endl;
				break;
			}
		}
		break;
		// Set Digital IO
		case 'L':
		case 'l':
		{
			// Select a device and get its description
			if(devices.size() == 0) {
				std::cout << "No devices found! Please scan for new devices." << std::endl << std::endl;
				break;
			}
			selectedDevice = selectDevice(devices);

			const auto ethAct = selectedDevice->getEthernetActivationLineCount();
			const auto usbHost = selectedDevice->getUSBHostPowerCount();
			const auto backup = selectedDevice->getBackupPowerSupported();

			if(ethAct == 0 && usbHost == 0 && !backup) {
				std::cout << "The selected device has no controllable digital IO." << std::endl << std::endl;
				break;
			}

			std::vector<char> options = { '1' };
			std::vector<std::string> names;
			std::map< char, std::pair<icsneo::IO, size_t> > types;

			std::cout << "Select from the following:" << std::endl;
			for(size_t i = 1; i <= ethAct; i++) {
				if(i > 1)
					options.push_back(options.back() + 1);
				names.push_back("Ethernet (DoIP) Activation Line");
				if(ethAct > 1) {
					names.back() += ' ';
					names.back() += i;
				}
				std::cout << '[' << options.back() << "] " << names.back();
				const auto val = selectedDevice->getDigitalIO(icsneo::IO::EthernetActivation, i);
				types[options.back()] = { icsneo::IO::EthernetActivation, i };
				if(val) {
					if(*val)
						std::cout << ": Enabled" << std::endl;
					else
						std::cout << ": Disabled" << std::endl;
				} else {
					std::cout << ": Unknown (" << icsneo::GetLastError() << ")" << std::endl;
				}
			}
			for(size_t i = 1; i <= usbHost; i++) {
				options.push_back(options.back() + 1);
				names.push_back("USB Host Power");
				if(usbHost > 1) {
					names.back() += ' ';
					names.back() += i;
				}
				std::cout << '[' << options.back() << "] " << names.back();
				const auto val = selectedDevice->getDigitalIO(icsneo::IO::USBHostPower, i);
				types[options.back()] = { icsneo::IO::USBHostPower, i };
				if(val) {
					if(*val)
						std::cout << ": Enabled" << std::endl;
					else
						std::cout << ": Disabled" << std::endl;
				} else {
					std::cout << ": Unknown (" << icsneo::GetLastError() << ")" << std::endl;
				}
			}
			if(backup) {
				options.push_back(options.back() + 1);
				names.push_back("Backup Power");
				std::cout << '[' << options.back() << "] " << names.back() << ": ";
				auto val = selectedDevice->getDigitalIO(icsneo::IO::BackupPowerEnabled);
				types[options.back()] = { icsneo::IO::BackupPowerEnabled, 1 };
				if(val) {
					if(*val)
						std::cout << "Enabled";
					else
						std::cout << "Disabled";

					val = selectedDevice->getDigitalIO(icsneo::IO::BackupPowerGood);
					if(val) {
						if(*val)
							std::cout << " and Charged" << std::endl;
						else
							std::cout << " and Not Charged" << std::endl;
					} else {
						std::cout << "but the status is unknown (" << icsneo::GetLastError() << ")" << std::endl;
					}
				} else {
					std::cout << "Unknown (" << icsneo::GetLastError() << ")" << std::endl;
				}
			}

			options.push_back(options.back() + 1);
			std::cout << '[' << options.back() << "] Cancel" << std::endl << std::endl;

			char selection = getCharInput(options);
			std::cout << std::endl;

			if(selection == options.back()) {
				std::cout << "Canceling!" << std::endl << std::endl;
				break;
			}

			std::cout << "[0] Disable\n[1] Enable\n[2] Cancel" << std::endl << std::endl;
			char selection2 = getCharInput({ '0', '1', '2' });
			std::cout << std::endl;

			if(selection2 == '2') {
				std::cout << "Canceling!" << std::endl << std::endl;
				break;
			}

			const bool val = selection2 == '1';
			if(selectedDevice->setDigitalIO(types[selection].first, types[selection].second, val))
				std::cout << "OK!" << std::endl << std::endl;
			else
				std::cout << "Failure! (" << icsneo::GetLastError() << ")" << std::endl << std::endl;
		}
		break;
		// Set HS CAN termination
		case 'M':
		case 'm':
		{
			// Select a device and get its description
			if(devices.size() == 0) {
				std::cout << "No devices found! Please scan for new devices." << std::endl << std::endl;
				break;
			}
			selectedDevice = selectDevice(devices);

			std::cout << "Termination is ";
			const auto val = selectedDevice->settings->isTerminationEnabledFor(icsneo::Network::NetID::HSCAN);
			if(!val.has_value()) {
				std::cout << "not available at this time: " << icsneo::GetLastError() << std::endl << std::endl;
				break;
			}
			std::cout << (*val ? "currently enabled" : "currently disabled") << std::endl;

			std::cout << "[0] Disable\n[1] Enable\n[2] Cancel" << std::endl << std::endl;
			char selection2 = getCharInput({ '0', '1', '2' });
			std::cout << std::endl;

			if(selection2 == '2') {
				std::cout << "Canceling!" << std::endl << std::endl;
				break;
			}

			// Attempt to set termination and apply settings
			if(selectedDevice->settings->setTerminationFor(icsneo::Network::NetID::HSCAN, selection2 == '1') && selectedDevice->settings->apply()) {
				std::cout << "Successfully set HS CAN termination for " << selectedDevice->describe() << std::endl;
			} else {
				std::cout << "Failed to set HS CAN termination for " << selectedDevice->describe() << std::endl;
				std::cout << icsneo::GetLastError() << std::endl;;
			}
			std::cout << std::endl;
		}
		break;
		// Exit
		case 'X':
		case 'x':
			printf("Exiting program\n");
			return 0;
		default:
			printf("Unexpected input, exiting!\n");
			return 1;
		}
	}

	return 0;
}