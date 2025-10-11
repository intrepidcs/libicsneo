#ifndef __PCAP_WINDOWS_H_
#define __PCAP_WINDOWS_H_

#ifdef __cplusplus

#include "icsneo/platform/windows/internal/pcapdll.h"
#include "icsneo/device/neodevice.h"
#include "icsneo/communication/driver.h"
#include "icsneo/device/founddevice.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/communication/ethernetpacketizer.h"
#include <string>
#include <vector> 
namespace icsneo {

class PCAP : public Driver {
public:
	static void Find(std::vector<FoundDevice>& foundDevices);
	static std::string GetEthDevSerialFromMacAddress(uint8_t product, uint16_t macSerial);
	static bool IsHandleValid(neodevice_handle_t handle);

	PCAP(const device_eventhandler_t& err, neodevice_t& forDevice);
	bool open() override;
	bool isOpen() override;
	bool close() override;
	bool isEthernet() const override { return true; }
	driver_finder_t getFinder() override { return PCAP::Find; }

private:
	const PCAPDLL& pcap;
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	neodevice_t& device;
	uint8_t deviceMAC[6];
	bool openable = true;
	EthernetPacketizer ethPacketizer;
	
	std::thread readThread, writeThread;
	std::thread transmitThread;
	pcap_send_queue* transmitQueue = nullptr;
	std::condition_variable transmitQueueCV;
	std::mutex transmitQueueMutex;
	
	void readTask();
	void writeTask();
	void transmitTask();

	class NetworkInterface {
	public:
		uint8_t uuid;
		uint8_t macAddress[6];
		std::string nameFromWinPCAP;
		std::string nameFromWin32API;
		std::string descriptionFromWinPCAP;
		std::string descriptionFromWin32API;
		std::string friendlyNameFromWin32API;
		std::string fullName;
		pcap_t* fp = nullptr;
		pcap_stat stats;
	};
	static std::vector<NetworkInterface> knownInterfaces;
	NetworkInterface iface;
};

}

#endif // __cplusplus

#endif