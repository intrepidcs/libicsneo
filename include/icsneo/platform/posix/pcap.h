#ifndef __PCAP_POSIX_H_
#define __PCAP_POSIX_H_

#ifdef __cplusplus

#include "icsneo/device/neodevice.h"
#include "icsneo/communication/driver.h"
#include "icsneo/communication/ethernetpacketizer.h"
#include "icsneo/api/eventmanager.h"
#include <string>
#include <pcap.h>

namespace icsneo {

class PCAP : public Driver {
public:
	class PCAPFoundDevice {
	public:
		neodevice_t device;
		std::vector<std::vector<uint8_t>> discoveryPackets;
	};

	static std::vector<PCAPFoundDevice> FindAll();
	static std::string GetEthDevSerialFromMacAddress(uint8_t product, uint16_t macSerial);
	static bool IsHandleValid(neodevice_handle_t handle);

	PCAP(device_eventhandler_t err, neodevice_t& forDevice);
	bool open();
	bool isOpen();
	bool close();
private:
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	neodevice_t& device;
	uint8_t deviceMAC[6];
	bool openable = true;
	EthernetPacketizer ethPacketizer;
	void readTask();
	void writeTask();

	class NetworkInterface {
	public:
		uint8_t uuid;
		uint8_t macAddress[8];
		std::string nameFromPCAP;
		std::string descriptionFromPCAP;
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