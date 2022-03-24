#ifndef __PCAP_POSIX_H_
#define __PCAP_POSIX_H_

#ifdef __cplusplus

#include "icsneo/device/neodevice.h"
#include "icsneo/device/founddevice.h"
#include "icsneo/communication/driver.h"
#include "icsneo/communication/ethernetpacketizer.h"
#include "icsneo/api/eventmanager.h"
#include <string>
#include <pcap.h>

namespace icsneo {

class PCAP : public Driver {
public:
	static void Find(std::vector<FoundDevice>& foundDevices);
	static std::string GetEthDevSerialFromMacAddress(uint8_t product, uint16_t macSerial);
	static bool IsHandleValid(neodevice_handle_t handle);

	PCAP(device_eventhandler_t err, neodevice_t& forDevice);
	bool open() override;
	bool isOpen() override;
	bool close() override;
	bool isEthernet() const override { return true; }
private:
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	neodevice_t& device;
	uint8_t deviceMAC[6];
	bool openable = true;
	EthernetPacketizer ethPacketizer;
	void readTask() override;
	void writeTask() override;

	class NetworkInterface {
	public:
		uint8_t uuid;
		uint8_t macAddress[6];
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