// Signal to dynamically load the library
#define ICSNEOC_DYNAMICLOAD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

// Get the PRIu64 macro for timestamps
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

// Include icsneo/icsneoc.h to access library functions
#include "icsneo/icsneoc.h"

size_t msgLimit = 50000;
size_t numDevices = 0;
neodevice_t devices[99];
const neodevice_t* selectedDevice = NULL;

/**
 * \brief Prints all current known devices to output in the following format:
 * [num] DeviceType SerialNum    Connected: Yes/No    Online: Yes/No    Msg Polling: On/Off
 *
 * If any devices could not be described due to an error, they will appear in the following format:
 * Description for device num not available!
 */
void printAllDevices() {
	if(numDevices == 0) {
		printf("No devices found! Please scan for new devices.\n");
	}
	for(int i = 0; i < numDevices; i++) {
		char productDescription[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION] = { 0 };
		size_t descriptionLength = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;

		// Updates productDescription and descriptionLength for each device
		if(icsneo_describeDevice(devices + i, productDescription, &descriptionLength)) {
			printf("[%d] %s\tConnected: ", i + 1, productDescription);
			if(icsneo_isOpen(devices + i)) {
				printf("Yes\t");
			} else printf("No\t");

			printf("Online: ");
			if(icsneo_isOnline(devices + i)) {
				printf("Yes\t");
			} else printf("No\t");

			printf("Msg Polling: ");
			if(icsneo_isMessagePollingEnabled(devices + i)) {
				printf("On\n");
			} else printf("Off\n");

		} else {
			printf("Description for device %d not available!\n", i + 1);
		}
	}
}

/**
 * \brief Scans for any new devices, adding them to devices and updating numDevices accordingly
 * A total of 99 devices may be stored at once
 */
size_t scanNewDevices() {
	neodevice_t newDevices[99];
	size_t numNewDevices = 99;
	icsneo_findAllDevices(newDevices, &numNewDevices);

	for(int i = 0; i < numNewDevices; ++i) {
		devices[numDevices + i] = newDevices[i];
	}
	numDevices += numNewDevices;
	return numNewDevices;
}

// Prints the main menu options to output
void printMainMenu() {
	printf("Press the letter next to the function you want to use:\n");
	printf("A - List all devices\n");
	printf("B - Scan for new devices\n");
	printf("C - Open/close\n");
	printf("D - Go online/offline\n");
	printf("E - Enable/disable message polling\n");
	printf("F - Get messages\n");
	printf("G - Send message\n");
	printf("H - Get events\n");
	printf("I - Set HS CAN to 250K\n");
	printf("J - Set HS CAN to 500K\n");
	printf("L - Set Digital IO\n");
	printf("M - Set HS CAN termination\n");
	printf("X - Exit\n");
}

void printLastError() {
	neoevent_t error;
	if(icsneo_getLastError(&error))
		printf("Error 0x%u: %s\n", error.eventNumber, error.description);
	else
		printf("No errors found!\n");
}

/**
 * \brief Gets all current API events and prints them to output
 * Flushes the API event cache, meaning future calls (barring any new events) will not detect any further API events
 */
void printAPIEvents() {
	neoevent_t events[99];
	size_t eventCount = 99;
	if(icsneo_getEvents(events, &eventCount)) {
		if(eventCount == 1) {
			printf("1 API event found!\n");
			printf("Event 0x%u: %s\n", events[0].eventNumber, events[0].description);
		} else {
			printf("%d API events found!\n", (int) eventCount);
			for(int i = 0; i < eventCount; ++i) {
				printf("Event 0x%u: %s\n", events[i].eventNumber, events[i].description);
			}
		}
	} else {
		printf("Failed to get API events!\n");
	}
}

/**
 * \brief Gets all current device events and prints them to output. If no device events were found, printAPIEvents() is called
 * Flushes the device event cache, meaning future calls (barring any new events) will not detect any further device events for this device
 */
void printDeviceEvents(neodevice_t* device) {
	neoevent_t events[99];
	size_t eventCount = 99;
	if(icsneo_getDeviceEvents(selectedDevice, events, &eventCount)) {
		if(eventCount == 1) {
			printf("1 device event found!\n");
			printf("Event 0x%x: %s\n", events[0].eventNumber, events[0].description);
		} else {
			printf("%d device events found!\n", (int) eventCount);
			for(int i = 0; i < eventCount; ++i) {
				printf("Event 0x%x: %s\n", events[i].eventNumber, events[i].description);
			}
		}
	}
}

/**
 * \brief Used to check character inputs for correctness (if they are found in an expected list)
 * \param[in] numArgs the number of possible options for the expected character
 * \param[in] ... the possible options for the expected character
 * \returns the entered character
 *
 * This function repeatedly prompts the user for input until a matching input is entered
 * Example usage: char input = getCharInput(5, 'F', 'u', 'b', 'a', 'r');
 */
char getCharInput(int numArgs, ...) {
	// 99 chars shold be more than enough to catch any typos
	char input[99];
	bool found = false;

	va_list vaList;
	va_start(vaList, numArgs);

	char* list = (char*) calloc(numArgs, sizeof(char));
	for(int i = 0; i < numArgs; ++i) {
		*(list + i) = va_arg(vaList, int);
	}

	va_end(vaList);

	while(!found) {
		fgets(input, 99, stdin);
		if(strlen(input) == 2) {
			for(int i = 0; i < numArgs; ++i) {
				if(input[0] == *(list + i)) {
					found = true;
					break;
				}
			}
		}

		if(!found) {
			printf("Input did not match expected options. Please try again.\n");
		}
	}

	free(list);

	return input[0];
}

/**
 * \brief Prompts the user to select a device from the list of currently known devices
 * \returns a pointer to the device in devices[] selected by the user
 * Requires an input from 1-9, so a maximum of 9 devices are supported
 */
const neodevice_t* selectDevice() {
	printf("Please select a device:\n");
	printAllDevices();
	printf("\n");

	int selectedDeviceNum = 10;

	while(selectedDeviceNum > numDevices) {
		char deviceSelection = getCharInput(9, '1', '2', '3', '4', '5', '6', '7', '8', '9');
		selectedDeviceNum = deviceSelection - '0';
		if(selectedDeviceNum > numDevices) {
			printf("Selected device out of range!\n");
		}
	}

	printf("\n");

	return devices + selectedDeviceNum - 1;
}

int main() {

	// Attempt to initialize the library and access its functions
	// This call searches for icsneoc.dll according to the standard dynamic-link library search order
	int ret = icsneo_init();
	if(ret == 1) {
		printf("The library was already initialized!\n");
		return ret;
	}

	if(ret == 2) {
		printf("The library could not be found!\n");
		return ret;
	}

	if(ret == 3) {
		printf("The library is missing functions!\n");
		return ret;
	}

	neoversion_t ver = icsneo_getVersion();
	printf("ICS icsneoc.dll version %u.%u.%u\n\n", ver.major, ver.minor, ver.patch);

	while(true) {
		printMainMenu();
		printf("\n");
		char input = getCharInput(26, 'A', 'a', 'B', 'b', 'C', 'c', 'D', 'd', 'E', 'e', 'F', 'f', 'G', 'g', 'H', 'h', 'I', 'i', 'J', 'j', 'L', 'l', 'M', 'm', 'X', 'x');
		printf("\n");
		switch(input) {
		// List current devices
		case 'A':
		case 'a':
			printAllDevices();
			printf("\n");
			break;
		// Scan for new devices
		case 'B':
		case 'b':
		{
			size_t numNewDevices = scanNewDevices();
			if(numNewDevices == 1) {
				printf("1 new device found!\n");
			} else {
				printf("%d new devices found!\n", (int) numNewDevices);
			}
			printAllDevices();
			printf("\n");
			break;
		}
		// Open/close a device
		case 'C':
		case 'c':
		{
			// Select a device and get its description
			if(numDevices == 0) {
				printf("No devices found! Please scan for new devices.\n\n");
				break;
			}
			selectedDevice = selectDevice();

			// Get the product description for the device
			char productDescription[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION] = { 0 };
			size_t descriptionLength = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;
			icsneo_describeDevice(selectedDevice, productDescription, &descriptionLength);

			printf("Would you like to open or close %s?\n", productDescription);
			printf("[1] Open\n[2] Close\n[3] Cancel\n\n");

			char input = getCharInput(3, '1', '2', '3');
			printf("\n");

			switch(input) {
			case '1':
				// Attempt to open the selected device
				if(icsneo_openDevice(selectedDevice)) {
					printf("%s successfully opened!\n\n", productDescription);
				} else {
					printf("%s failed to open!\n\n", productDescription);
					printLastError();
					printf("\n");
				}
				break;
			case '2':
				// Attempt to close the device
				if(icsneo_closeDevice(selectedDevice)) {
					numDevices--;
					printf("Successfully closed %s!\n\n", productDescription);

					// Shifts everything after the removed device 1 index to the left
					bool startResizing = false;
					for(int i = 0; i < numDevices; ++i) {
						if(selectedDevice == devices + i)
							startResizing = true;
						if(startResizing)
							devices[i] = devices[i + 1];
					}

					selectedDevice = NULL;
				} else {
					printf("Failed to close %s!\n\n", productDescription);
					printLastError();
					printf("\n");
				}

				break;
			default:
				printf("Canceling!\n\n");
			}
		}
		break;
		// Go online/offline
		case 'D':
		case 'd':
		{
			// Select a device and get its description
			if(numDevices == 0) {
				printf("No devices found! Please scan for new devices.\n\n");
				break;
			}
			selectedDevice = selectDevice();

			// Get the product description for the device
			char productDescription[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION] = { 0 };
			size_t descriptionLength = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;
			icsneo_describeDevice(selectedDevice, productDescription, &descriptionLength);

			printf("Would you like to have %s go online or offline?\n", productDescription);
			printf("[1] Online\n[2] Offline\n[3] Cancel\n\n");

			char input = getCharInput(3, '1', '2', '3');
			printf("\n");

			switch(input) {
			case '1':
				// Attempt to go online
				if(icsneo_goOnline(selectedDevice)) {
					printf("%s successfully went online!\n\n", productDescription);
				} else {
					printf("%s failed to go online!\n\n", productDescription);
					printLastError();
					printf("\n");
				}
				break;
			case '2':
				// Attempt to go offline
				if(icsneo_goOffline(selectedDevice)) {
					printf("%s successfully went offline!\n\n", productDescription);
				} else {
					printf("%s failed to go offline!\n\n", productDescription);
					printLastError();
					printf("\n");
				}
				break;
			default:
				printf("Canceling!\n\n");
			}
		}
		break;
		// Enable/disable message polling
		case 'E':
		case 'e':
		{
			// Select a device and get its description
			if(numDevices == 0) {
				printf("No devices found! Please scan for new devices.\n\n");
				break;
			}
			selectedDevice = selectDevice();

			// Get the product description for the device
			char productDescription[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION] = { 0 };
			size_t descriptionLength = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;
			icsneo_describeDevice(selectedDevice, productDescription, &descriptionLength);

			printf("Would you like to enable or disable message polling for %s?\n", productDescription);
			printf("[1] Enable\n[2] Disable\n[3] Cancel\n\n");

			char input = getCharInput(3, '1', '2', '3');
			printf("\n");

			switch(input) {
			case '1':
				// Attempt to enable message polling
				if(icsneo_enableMessagePolling(selectedDevice)) {
					printf("Successfully enabled message polling for %s!\n\n", productDescription);
				} else {
					printf("Failed to enable message polling for %s!\n\n", productDescription);
					printLastError();
					printf("\n");
				}

				// Manually setting the polling message limit as done below is optional
				// It will default to 20k if not set
				// Attempt to set the polling message limit
				if(icsneo_setPollingMessageLimit(selectedDevice, msgLimit)) {
					printf("Successfully set message polling limit for %s!\n\n", productDescription);
				} else {
					printf("Failed to set polling message limit for %s!\n\n", productDescription);
					printLastError();
					printf("\n");
				}
				break;
			case '2':
				// Attempt to disable message polling
				if(icsneo_disableMessagePolling(selectedDevice)) {
					printf("Successfully disabled message polling for %s!\n\n", productDescription);
				} else {
					printf("Failed to disable message polling limit for %s!\n\n", productDescription);
					printLastError();
					printf("\n");
				}
				break;
			default:
				printf("Canceling!\n\n");
			}
		}
		break;
		// Get messages
		case 'F':
		case 'f':
		{
			// Select a device and get its description
			if(numDevices == 0) {
				printf("No devices found! Please scan for new devices.\n\n");
				break;
			}
			selectedDevice = selectDevice();

			// Get the product description for the device
			char productDescription[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION] = { 0 };
			size_t descriptionLength = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;
			icsneo_describeDevice(selectedDevice, productDescription, &descriptionLength);

			// Prepare the array of neomessage_t ptrs for reading in the messages
			neomessage_t* msgs = (neomessage_t*) malloc(msgLimit * sizeof(neomessage_t));

			// Get messages
			size_t msgCount = msgLimit;

			// Attempt to get messages
			if(!icsneo_getMessages(selectedDevice, msgs, &msgCount, (uint64_t) 0)) {
				printf("Failed to get messages for %s!\n\n", productDescription);
				printLastError();
				free(msgs);
				printf("\n");
				break;
			}

			if(msgCount == 1) {
				printf("1 message received from %s!\n", productDescription);
			} else {
				printf("%d messages received from %s!\n", (int) msgCount, productDescription);
			}

			// Print out the received messages
			for(size_t i = 0; i < msgCount; i++) {
				const neomessage_t* msg = &msgs[i];
				switch(msg->messageType) {
					case ICSNEO_MESSAGE_TYPE_FRAME: {
						const neomessage_frame_t* frame = (neomessage_frame_t*)msg;
						switch(frame->type) {
							case ICSNEO_NETWORK_TYPE_CAN: {
								neomessage_can_t* canMsg = (neomessage_can_t*)frame;
								printf("\t0x%03x [%zu] ", canMsg->arbid, canMsg->length);
								for(size_t i = 0; i < canMsg->length; i++) {
									printf("%02x ", canMsg->data[i]);
								}
								if(canMsg->status.transmitMessage)
									printf("TX%s %04x ", canMsg->status.globalError ? " ERR" : "", canMsg->description);
								printf("(%"PRIu64")\n", canMsg->timestamp);
								break;
							}
							default:
								printf("\tMessage on netid %d with length %zu\n", frame->netid, frame->length);
								break;
						}
						break;
					}
					case ICSNEO_MESSAGE_TYPE_CAN_ERROR_COUNT: {
						const neomessage_can_error_t* cec = (neomessage_can_error_t*)msg;
						printf("\tCAN error counts changed, TEC=%d, REC=%d%s", cec->transmitErrorCount, cec->receiveErrorCount,
							cec->status.canBusOff ? " (Bus Off)" : "");
						break;
					}
				}
			}
			printf("\n");

			free(msgs);
		}

		break;
		// Send message
		case 'G':
		case 'g':
		{
			// Select a device and get its description
			if(numDevices == 0) {
				printf("No devices found! Please scan for new devices.\n\n");
				break;
			}
			selectedDevice = selectDevice();

			// Get the product description for the device
			char productDescription[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION] = { 0 };
			size_t descriptionLength = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;
			icsneo_describeDevice(selectedDevice, productDescription, &descriptionLength);

			// Start generating sample msg
			uint8_t sendMessageData[6];
			sendMessageData[0] = 0xaa;
			sendMessageData[1] = 0xbb;
			sendMessageData[2] = 0xcc;
			sendMessageData[3] = 0xdd;
			sendMessageData[4] = 0xee;
			sendMessageData[5] = 0xff;

			neomessage_can_t msg = {0};
			msg.arbid = 0x120;
			msg.length = 6;
			msg.netid = ICSNEO_NETID_HSCAN;
			msg.data = sendMessageData;
			msg.description = 0x1c5; // Random description to match the receipt
			msg.status.canfdFDF = false;
			msg.status.extendedFrame = false;
			msg.status.canfdBRS = false;
			// End generating sample msg

			// Attempt to transmit the sample msg
			if(icsneo_transmit(selectedDevice, (const neomessage_t*) &msg)) {
				printf("Message transmit successful!\n\n");
			} else {
				printf("Failed to transmit message to %s!\n\n", productDescription);
				printLastError();
				printf("\n");
			}
		}
		break;
		// Get events
		case 'H':
		case 'h':
		{
			// All API events
			printAPIEvents();
			printf("\n");
		}
		break;
		// Set HS CAN to 250k
		case 'I':
		case 'i':
		{
			// Select a device and get its description
			if(numDevices == 0) {
				printf("No devices found! Please scan for new devices.\n\n");
				break;
			}
			selectedDevice = selectDevice();

			// Get the product description for the device
			char productDescription[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION] = { 0 };
			size_t descriptionLength = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;
			icsneo_describeDevice(selectedDevice, productDescription, &descriptionLength);

			// Attempt to set baudrate and apply settings
			if(icsneo_setBaudrate(selectedDevice, ICSNEO_NETID_HSCAN, 250000) && icsneo_settingsApply(selectedDevice)) {
				printf("Successfully set HS CAN baudrate for %s to 250k!\n\n", productDescription);
			} else {
				printf("Failed to set HS CAN for %s to 250k!\n\n", productDescription);
				printLastError();
				printf("\n");
			}
		}
		break;
		// Set HS CAN to 500k
		case 'J':
		case 'j':
		{
			// Select a device and get its description
			if(numDevices == 0) {
				printf("No devices found! Please scan for new devices.\n\n");
				break;
			}
			selectedDevice = selectDevice();

			// Get the product description for the device
			char productDescription[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION] = { 0 };
			size_t descriptionLength = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;
			icsneo_describeDevice(selectedDevice, productDescription, &descriptionLength);

			// Attempt to set baudrate and apply settings
			if(icsneo_setBaudrate(selectedDevice, ICSNEO_NETID_HSCAN, 500000) && icsneo_settingsApply(selectedDevice)) {
				printf("Successfully set HS CAN baudrate for %s to 500k!\n\n", productDescription);
			} else {
				printf("Failed to set HS CAN for %s to 500k!\n\n", productDescription);
				printLastError();
				printf("\n");
			}
		}
		break;
		// Set Digital IO
		case 'L':
		case 'l':
		{
			// Select a device and get its description
			if(numDevices == 0) {
				printf("No devices found! Please scan for new devices.\n\n");
				break;
			}
			selectedDevice = selectDevice();

			printf("Select from the following:\n");

			printf("[1] Ethernet (DoIP) Activation Line");
			bool val;
			bool haveVal = icsneo_getDigitalIO(selectedDevice, ICSNEO_IO_ETH_ACTIVATION, 1, &val);
			if(!haveVal) {
				neoevent_t event;
				bool got = icsneo_getLastError(&event);
				printf(": Unknown (%s)\n", got ? event.description : "No error");
			} else {
				if(val)
					printf(": Enabled\n");
				else
					printf(": Disabled\n");
			}

			printf("[2] USB Host Power");
			haveVal = icsneo_getDigitalIO(selectedDevice, ICSNEO_IO_USB_HOST_POWER, 1, &val);
			if(!haveVal) {
				neoevent_t event;
				bool got = icsneo_getLastError(&event);
				printf(": Unknown (%s)\n", got ? event.description : "No error");
			} else {
				if(val)
					printf(": Enabled\n");
				else
					printf(": Disabled\n");
			}

			printf("[3] Backup Power");
			haveVal = icsneo_getDigitalIO(selectedDevice, ICSNEO_IO_BACKUP_POWER_EN, 1, &val);
			if(!haveVal) {
				neoevent_t event;
				bool got = icsneo_getLastError(&event);
				printf(": Unknown (%s)\n", got ? event.description : "No error");
			} else {
				if(val)
					printf(": Enabled ");
				else
					printf(": Disabled ");

				haveVal = icsneo_getDigitalIO(selectedDevice, ICSNEO_IO_BACKUP_POWER_GOOD, 1, &val);
				if(!haveVal) {
					neoevent_t event;
					bool got = icsneo_getLastError(&event);
					printf("with unknown status (%s)\n", got ? event.description : "No error");
				} else {
					if(val)
						printf("and Charged\n");
					else
						printf("and Not Charged\n");
				}
			}

			printf("[4] Cancel\n\n");
			char selection = getCharInput(4, '1', '2', '3', '4');
			printf("\n");

			if(selection == '4') {
				printf("Canceling!\n\n");
				break;
			}

			printf("[0] Disable\n[1] Enable\n[2] Cancel\n\n");
			char selection2 = getCharInput(3, '0', '1', '2');
			printf("\n");

			if(selection2 == '2') {
				printf("Canceling!\n\n");
				break;
			}

			const bool set = selection2 == '1';
			neoio_t type;
			switch (selection)
			{
			case '1': type = ICSNEO_IO_ETH_ACTIVATION; break;
			case '2': type = ICSNEO_IO_USB_HOST_POWER; break;
			case '3': type = ICSNEO_IO_BACKUP_POWER_EN; break;
			};
			if(icsneo_setDigitalIO(selectedDevice, type, 1, set)) {
				printf("OK!\n\n");
			} else {
				neoevent_t event;
				bool got = icsneo_getLastError(&event);
				printf("Failure! (%s)\n\n", got ? event.description : "No error");
			}
		}
		break;
		// Set HS CAN termination
		case 'M':
		case 'm':
		{
			// Select a device and get its description
			if(numDevices == 0) {
				printf("No devices found! Please scan for new devices.\n\n");
				break;
			}
			selectedDevice = selectDevice();

			// Get the product description for the device
			char productDescription[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION] = { 0 };
			size_t descriptionLength = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;
			icsneo_describeDevice(selectedDevice, productDescription, &descriptionLength);

			printf("Termination is ");
			const bool val = icsneo_isTerminationEnabledFor(selectedDevice, ICSNEO_NETID_HSCAN);
			neoevent_t err = { 0 };
			if(icsneo_getLastError(&err)) {
				printf("not available at this time: %s\n\n", err.description);
				break;
			}
			printf(val ? "currently enabled\n" : "currently disabled\n");

			printf("[0] Disable\n[1] Enable\n[2] Cancel\n\n");
			char selection2 = getCharInput(3, '0', '1', '2');
			printf("\n");

			if(selection2 == '2') {
				printf("Canceling!\n\n");
				break;
			}

			// Attempt to set baudrate and apply settings
			if(icsneo_setTerminationFor(selectedDevice, ICSNEO_NETID_HSCAN, selection2 == '1') && icsneo_settingsApply(selectedDevice)) {
				printf("Successfully set HS CAN termination for %s!\n\n", productDescription);
			} else {
				printf("Failed to set HS CAN termination for %s!\n\n", productDescription);
				printLastError();
				printf("\n");
			}
		}
		break;
		// Exit
		case 'X':
		case 'x':
			printf("Exiting program\n");
			return !icsneo_close();
		default:
			printf("Unexpected input, exiting!\n");
			return 1;
		}
	}

	return 0;
}
