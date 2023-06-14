/*
NOTE: This example is written for a RED 2 device. To use another device type,
change the union lookup type in the pSettings structure to match your device!
e.g. pSettings.Settings.red2 -> pSettings.Settings.fire3
*/

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
	int numDevices = 10;
	NeoDevice devices[10];
	void* hObject;  // holds a handle to the neoVI object
	int iRetVal = 0;
	int deviceTypes = 0;
	int iResult = 0;
	SDeviceSettings pSettings;

	iRetVal = icsneoFindNeoDevices(deviceTypes, devices, &numDevices);
	if(iRetVal) {
		// Attempt to open the selected device, enable message polling, and go online
		iRetVal = icsneoOpenNeoDevice(&devices[0], &hObject, NULL, 1, 0);
		if(iRetVal) {
			puts("Device found and opened!\n");
		} else {
			puts("Device found but failed to open!\n");
		}
	} else {
		puts("No new devices found!\n");
	}

	// Get device settings
	{
		iRetVal = icsneoValidateHObject(hObject);
		iRetVal = icsneoGetDeviceSettings(hObject, &pSettings, sizeof(SRed2Settings), 0);
		if(iRetVal) {
			puts("Settings read successfully\n");
			printf("Baudrate: %u\n", pSettings.Settings.red2.lin1.Baudrate);
			printf("Master resistor: %s \n", (pSettings.Settings.red2.lin1.MasterResistor == 0 ? "true" : "false"));
		} else {
			puts("Settings not read!\n");
		}
	}
	SLEEP(1000);
	// set device settings
	{
		pSettings.Settings.red2.lin1.Baudrate = 19200;
		pSettings.Settings.red2.lin1.MasterResistor = 1;
		iRetVal = icsneoSetDeviceSettings(hObject, &pSettings, sizeof(pSettings), 1, 0);
		if(iRetVal) {
			puts("Settings set successfully\n");
			iRetVal = icsneoGetDeviceSettings(hObject, &pSettings, sizeof(SRed2Settings), 0);
			printf("Baudrate: %u\n", pSettings.Settings.red2.lin1.Baudrate);
			printf("Master resistor: %s\n", (pSettings.Settings.red2.lin1.MasterResistor == 0 ? "true" : "false"));
		} else {
			puts("Settings not set!\n");
		}
	}
	int iNumberOfErrors = 0;
	// Attempt to close the device
	{
		// Close Communication
		iResult = icsneoClosePort(hObject, &iNumberOfErrors);
	}
	puts("Exiting program\n");
	return iResult;
}
