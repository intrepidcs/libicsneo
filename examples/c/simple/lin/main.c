// Signal to dynamically load the library
//#define ICSNEOC_DYNAMICLOAD

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

#ifdef _WIN32
	#define SLEEP(msecs) Sleep(msecs)
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
	#include <time.h>
	#define SLEEP(msecs) do { \
		struct timespec ts; \
		ts.tv_sec = msecs/1000; \
		ts.tv_nsec = msecs%1000*1000; \
		nanosleep(&ts, NULL); \
	} while (0)
#else
	#error "Platform unknown"
#endif

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
	for(size_t i = 0; i < numDevices; i++) {
		char productDescription[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION] = { 0 };
		size_t descriptionLength = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;

		// Updates productDescription and descriptionLength for each device
		if(icsneo_describeDevice(devices + i, productDescription, &descriptionLength)) {
			printf("[%zd] %s\tConnected: ", i + 1, productDescription);
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
			printf("Description for device %zd not available!\n", i + 1);
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

	for(size_t i = 0; i < numNewDevices; ++i) {
		devices[numDevices + i] = newDevices[i];
	}
	numDevices += numNewDevices;
	return numNewDevices;
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
			for(size_t i = 0; i < eventCount; ++i) {
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
			for(size_t i = 0; i < eventCount; ++i) {
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

	size_t selectedDeviceNum = 10;

	while(selectedDeviceNum > numDevices) {
		char deviceSelection = getCharInput(9, '1', '2', '3', '4', '5', '6', '7', '8', '9');
		if(deviceSelection < '0') {
			printf("Selected device out of range!\n");
			continue;
		}
		selectedDeviceNum = deviceSelection - '0';
		if(selectedDeviceNum > numDevices) {
			printf("Selected device out of range!\n");
		}
	}

	printf("\n");

	return devices + selectedDeviceNum - 1;
}

int main() {
	neoversion_t ver = icsneo_getVersion();
	printf("ICS icsneoc version %u.%u.%u\n\n", ver.major, ver.minor, ver.patch);
	// Find and attempt to open device
	size_t numNewDevices = scanNewDevices();
	if(numNewDevices == 1) {
		printf("1 new device found!\n");
	} else {
		printf("%d new devices found!\n", (int) numNewDevices);
	}
	printAllDevices();
	printf("\n");

	// Select a device and get its description
	if(numDevices == 0) {
		printf("No devices found! Please scan for new devices.\n\n");
		return 1;
	}
	selectedDevice = &devices[0];

	// Get the product description for the device
	char productDescription[ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION] = { 0 };
	size_t descriptionLength = ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION;
	icsneo_describeDevice(selectedDevice, productDescription, &descriptionLength);

	// Attempt to open the selected device
	{
		if(icsneo_openDevice(selectedDevice)) {
			printf("%s successfully opened!\n\n", productDescription);
		} else {
			printf("%s failed to open!\n\n", productDescription);
			printLastError();
			printf("\n");
		}
	}
	// Attempt to go online
	{
		if(icsneo_goOnline(selectedDevice)) {
			printf("%s successfully went online!\n\n", productDescription);
		} else {
			printf("%s failed to go online!\n\n", productDescription);
			printLastError();
			printf("\n");
		}
	}
	// Attempt to enable message polling
	{
		if(icsneo_enableMessagePolling(selectedDevice)) {
			printf("Successfully enabled message polling for %s!\n\n", productDescription);
		} else {
			printf("Failed to enable message polling for %s!\n\n", productDescription);
			printLastError();
			printf("\n");
		}
	}
	// Send message LIN
	{
		// Start generating sample msg
		uint8_t sendMessageData[8];
		sendMessageData[0] = 0x33;
		sendMessageData[1] = 0x44;
		sendMessageData[2] = 0x55;
		sendMessageData[3] = 0x66;
		sendMessageData[4] = 0x77;
		sendMessageData[5] = 0x88;
		sendMessageData[6] = 0x88;

		neomessage_lin_t msg = {0};

		msg.header[0] = 0x11; //protected ID
		msg.header[1] = 0x11;
		msg.header[2] = 0x22;
		msg.length = 10;
		msg.netid = ICSNEO_NETID_LIN;
		msg.data = sendMessageData;
		msg.linStatus.txCommander = 1;
		msg.linStatus.txChecksumEnhanced = 1;
		msg.type = ICSNEO_NETWORK_TYPE_LIN;
		msg.checksum = 0x88;

		// Attempt to transmit the sample msg
		if(icsneo_transmit(selectedDevice, (const neomessage_t*) &msg)) {
			printf("Message transmit successful!\n\n");
		} else {
			printf("Failed to transmit message to %s!\n\n", productDescription);
			printLastError();
			printf("\n");
		}
	}
	// Wait for a moment
	SLEEP(1000);
	// Get messages
	{
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
						case ICSNEO_NETWORK_TYPE_LIN: {
							neomessage_lin_t* linMsg = (neomessage_lin_t*)frame;
							size_t frameLen = linMsg->length;
							size_t dataLen = (frameLen > 2) ? (frameLen - 2) : 0;
							size_t numberBytesHeader = (dataLen > 1) ? 3 : 1;
							size_t numberBytesData = frameLen - numberBytesHeader;
							if(linMsg->netid == ICSNEO_NETID_LIN) {
								printf("LIN 1 | ID: 0x%02x [%zu] ", linMsg->header[0], dataLen);
							}
							else if (linMsg->netid == ICSNEO_NETID_LIN2) {
								printf("LIN 2 | ID: 0x%02x [%zu] ", linMsg->header[0], dataLen);
							}

							for(size_t i = 0; i < dataLen; ++i) {
								if (i < 2) {
									printf("%02x ", linMsg->header[i+1]);
								} else {
									printf("%02x ", linMsg->data[i-2]);
								}
							}
							printf("| Checksum: 0x%02x\n", linMsg->checksum);
							break;
						}
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
	// Attempt to disable message polling
	{
		if(icsneo_disableMessagePolling(selectedDevice)) {
			printf("Successfully disabled message polling for %s!\n\n", productDescription);
		} else {
			printf("Failed to disable message polling limit for %s!\n\n", productDescription);
			printLastError();
			printf("\n");
		}
	}
	// Attempt to go offline
	{
		if(icsneo_goOffline(selectedDevice)) {
			printf("%s successfully went offline!\n\n", productDescription);
		} else {
			printf("%s failed to go offline!\n\n", productDescription);
			printLastError();
			printf("\n");
		}
	}
	// Attempt to close the device
	{
		if(icsneo_closeDevice(selectedDevice)) {
			numDevices--;
			printf("Successfully closed %s!\n\n", productDescription);

			// Shifts everything after the removed device 1 index to the left
			bool startResizing = false;
			for(size_t i = 0; i < numDevices; ++i) {
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
	}
	//exit
	printf("Exiting program\n");
	return 0;
}
