#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#if defined _WIN32
	#include "icsneo/platform/windows.h"
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

// Get the PRIu64 macro for timestamps
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

// Include icsneo/icsneolegacy.h to access library functions
#include "icsneo/icsneolegacy.h"

int main() {
	int ver = icsneoGetDLLVersion();
	printf("ICS icsneolegacy.dll version %u\n\n", ver);
	// Find and attempt to open device
	//legacy open device
	int numDevices = 255;
	NeoDeviceEx devices[255] = {0};
	void* hObject = NULL;  // holds a handle to the neoVI object
	int iRetVal = 0;
	int iResult = 0;

	iRetVal = icsneoFindDevices(devices, &numDevices, NULL, 0, NULL, 0);
	if(iRetVal && numDevices < 0) {
		// Attempt to open the selected device, enable message polling, and go online
		iRetVal = icsneoOpenDevice(&devices[0], &hObject, NULL, 1, 0, NULL, 0);
		if(iRetVal) {
			printf("Device found and opened!\n");
		} else {
			printf("Device found but failed to open!\n");
		}
	} else {
		printf("No new devices found!\n");
	}

	// Send message LIN
	{
		//lin responder frame
		icsSpyMessageJ1850 msg1 = {0};
		int lNetworkID;
		msg1.Protocol = SPY_PROTOCOL_LIN;
		msg1.StatusBitField = 0;
		msg1.StatusBitField2 = 0;
		lNetworkID = NETID_LIN2;
		msg1.Header[0] = 0x11; //protected ID
		msg1.Header[1] = 0xaa;
		msg1.Header[2] = 0xbb;
		msg1.Data[0] = 0xcc;
		msg1.Data[1] = 0xdd;
		msg1.Data[2] = 0x11;
		msg1.Data[3] = 0x22;
		msg1.Data[4] = 0x33;
		msg1.Data[5] = 0x44;
		msg1.Data[6] = 0x44; //checksum 0x33 enhanced
		msg1.NumberBytesData = 7;
		msg1.NumberBytesHeader = 3;
		iRetVal = icsneoTxMessages(hObject, (icsSpyMessage*)&msg1, lNetworkID, 1);
		if(!iRetVal)
			printf("Device failed to transmit LIN responder update\n");
		else
			printf("Transmitted successfully!\n");

		icsSpyMessageJ1850 msg2 = {0};
		msg2.Protocol = SPY_PROTOCOL_LIN;
		msg2.StatusBitField = SPY_STATUS_INIT_MESSAGE;
		lNetworkID = NETID_LIN;
		msg2.Header[0] = 0x11; //protected ID
		msg2.NumberBytesData = 0;
		msg2.NumberBytesHeader = 1;
		iRetVal = icsneoTxMessages(hObject, (icsSpyMessage*)&msg2, lNetworkID, 1);
		if(!iRetVal)
			printf("Device failed to transmit LIN commander header\n");
		else
			printf("Transmitted successfully!\n");

		SLEEP(250);

		icsSpyMessageJ1850 msg3 = {0};
		msg3.Protocol = SPY_PROTOCOL_LIN;
		msg3.StatusBitField = SPY_STATUS_INIT_MESSAGE;
		msg3.StatusBitField2 = 0;
		lNetworkID = NETID_LIN;
		msg3.Header[0] = 0xe2; //protected ID
		msg3.Header[1] = 0x44;
		msg3.Header[2] = 0x33;
		msg3.Data[0] = 0x22;
		msg3.Data[1] = 0x11;
		msg3.Data[2] = 0x11;
		msg3.Data[3] = 0x22;
		msg3.Data[4] = 0x33;
		msg3.Data[5] = 0x44;
		msg3.Data[6] = 0xc7; //checksum
		msg3.NumberBytesData = 7;
		msg3.NumberBytesHeader = 3;
		iRetVal = icsneoTxMessages(hObject, (icsSpyMessage*)&msg3, lNetworkID, 1);
		if(!iRetVal)
			printf("Device failed to transmit LIN commander message\n");
		else
			printf("Transmitted successfully!\n");
	}
	SLEEP(1000);
	// Get messages
	{
		static icsSpyMessage rxMsg[30000];
		int numMessages = 0;
		int numErrors = 0;
		iRetVal = icsneoGetMessages(hObject, rxMsg, &numMessages, &numErrors);
		if(!iRetVal)
			printf("Get Messages failed!\n");
		else
			for(int idx = 0; idx < numMessages; ++idx)
			{
				if(rxMsg[idx].Protocol == SPY_PROTOCOL_LIN) {
					const icsSpyMessageJ1850* linMsg = (icsSpyMessageJ1850*)&rxMsg[idx];
					size_t frameLen = (linMsg->NumberBytesHeader + linMsg->NumberBytesData);
					size_t dataLen = (frameLen > 2) ? (frameLen - 2) : 0;
					if(linMsg->NetworkID == NETID_LIN) {
						printf("LIN 1 | ID: 0x%02x [%zu] ", linMsg->Header[0], dataLen);
					}
					else if (linMsg->NetworkID == NETID_LIN2) {
						printf("LIN 2 | ID: 0x%02x [%zu] ", linMsg->Header[0], dataLen);
					}

					for(size_t i = 0; i < dataLen; ++i) {
						if (i < 2) {
							printf("%02x ", linMsg->Header[i+1]);
						} else {
							printf("%02x ", linMsg->Data[i-2]);
						}
					}
					if(linMsg->NumberBytesData > 0)
						printf("| Checksum: 0x%02x\n", linMsg->Data[linMsg->NumberBytesData-1]);
					else
						printf("| Checksum: 0x%02x\n", linMsg->Header[linMsg->NumberBytesHeader-1]);
				}
			}
	}

	int iNumberOfErrors = 0;
	// Attempt to close the device
	{
		// Close Communication
		iResult = icsneoClosePort(hObject, &iNumberOfErrors);
	}
	printf("Exiting program\n");
	return iResult;
}
