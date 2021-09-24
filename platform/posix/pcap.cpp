#include "icsneo/platform/posix/pcap.h"
#include "icsneo/communication/network.h"
#include "icsneo/communication/communication.h"
#include "icsneo/communication/packetizer.h"
#include <codecvt>
#include <chrono>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#ifdef __linux__
#include <netpacket/packet.h>
#else
#include <net/if_dl.h>
#endif

using namespace icsneo;

static const uint8_t BROADCAST_MAC[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static const uint8_t ICS_UNSET_MAC[6] = { 0x00, 0xFC, 0x70, 0xFF, 0xFF, 0xFF };

std::vector<PCAP::NetworkInterface> PCAP::knownInterfaces;

std::vector<PCAP::PCAPFoundDevice> PCAP::FindAll() {
	static bool warned = false; // Only warn once for failure to open devices
	std::vector<PCAPFoundDevice> foundDevices;

	// First we ask PCAP to give us all of the devices
	pcap_if_t* alldevs;
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	bool success = false;
	// Calling pcap_findalldevs too quickly can cause various errors. Retry a few times in this case.
	for(auto retry = 0; retry < 10; retry++) {
		auto ret = pcap_findalldevs(&alldevs, errbuf);
		if(ret == 0) {
			success = true;
			break;
		}
	}

	if(!success) {
		EventManager::GetInstance().add(APIEvent::Type::PCAPCouldNotFindDevices, APIEvent::Severity::Error);
		return std::vector<PCAPFoundDevice>();
	}

	std::vector<NetworkInterface> interfaces;
	for(pcap_if_t* dev = alldevs; dev != nullptr; dev = dev->next) {
		if(dev->name == nullptr)
			continue;
		if(dev->addresses == nullptr) {
			//std::cout << dev->name << " has no addresses" << std::endl;
			continue;
		}
		NetworkInterface netif;
		netif.nameFromPCAP = dev->name;
		if(dev->description)
			netif.descriptionFromPCAP = dev->description;
		pcap_addr* currentAddress = dev->addresses;
		bool hasAddress = false;
		while(!hasAddress && currentAddress != nullptr) {
#ifdef __linux__
			if(currentAddress->addr && currentAddress->addr->sa_family == AF_PACKET) {
				struct sockaddr_ll* s = (struct sockaddr_ll*)currentAddress->addr;
				memcpy(netif.macAddress, s->sll_addr, sizeof(netif.macAddress));
				hasAddress = true;
				break;
			}
#else // macOS and likely other BSDs
			if(currentAddress->addr && currentAddress->addr->sa_family == AF_LINK) {
				struct sockaddr_dl* s = (struct sockaddr_dl*)currentAddress->addr;
				if(s->sdl_alen == 6 && s->sdl_alen + s->sdl_nlen < sizeof(s->sdl_data)) {
					const uint8_t* mac = (uint8_t*)(s->sdl_data) + s->sdl_nlen;
					memcpy(netif.macAddress, mac, sizeof(netif.macAddress));
					hasAddress = true;
					break;
				}
			}
#endif
			currentAddress = currentAddress->next;
		}

		if(!hasAddress)
			continue;

		interfaces.push_back(netif);
	}

	pcap_freealldevs(alldevs);

	for(auto& iface : interfaces) {
		bool exists = false;
		for(auto& known : knownInterfaces)
			if(memcmp(iface.macAddress, known.macAddress, sizeof(iface.macAddress)) == 0)
				exists = true;
		if(!exists)
			knownInterfaces.emplace_back(iface);
	}

	for(size_t i = 0; i < knownInterfaces.size(); i++) {
		auto& iface = knownInterfaces[i];
		// if(iface.fullName.length() == 0)
		// 	continue; // Win32 did not find this interface in the previous step

		errbuf[0] = '\0';
		iface.fp = pcap_open_live(iface.nameFromPCAP.c_str(), 65536, 1,
#ifdef __linux__ // -1 is required for instant reporting of new packets
			-1, // to_ms
#else // macOS gives BIOCSRTIMEOUT for -1 and no packets for 0
			1,
#endif
			errbuf);
		// TODO Handle warnings
		// if(strlen(errbuf) != 0) { // This means a warning
		// 	std::cout << "Warning for " << iface.nameFromPCAP << " " << errbuf << std::endl;
		// }

		if(iface.fp == nullptr) {
			if (!warned) {
				warned = true;
				EventManager::GetInstance().add(APIEvent::Type::PCAPCouldNotFindDevices, APIEvent::Severity::EventWarning);
				// std::cout << "pcap_open_live failed for " << iface.nameFromPCAP << " with " << errbuf << std::endl;
			}
			continue; // Could not open the interface
		}

		pcap_setnonblock(iface.fp, 1, errbuf);

		EthernetPacketizer::EthernetPacket requestPacket;
		memcpy(requestPacket.srcMAC, iface.macAddress, sizeof(requestPacket.srcMAC));
		requestPacket.payload.reserve(4);
		requestPacket.payload = {
			((1 << 4) | (uint8_t)Network::NetID::Main51), // Packet size of 1 on NETID_MAIN51
			(uint8_t)Command::RequestSerialNumber
		};
		requestPacket.payload.push_back(Packetizer::ICSChecksum(requestPacket.payload));
		requestPacket.payload.insert(requestPacket.payload.begin(), 0xAA);

		auto bs = requestPacket.getBytestream();
		pcap_sendpacket(iface.fp, bs.data(), (int)bs.size());

		auto timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(50);
		while(std::chrono::high_resolution_clock::now() <= timeout) { // Wait up to 5ms for the response
			struct pcap_pkthdr* header;
			const uint8_t* data;
			auto res = pcap_next_ex(iface.fp, &header, &data);
			if(res < 0) {
				if (!warned) {
					warned = true;
					EventManager::GetInstance().add(APIEvent::Type::PCAPCouldNotFindDevices, APIEvent::Severity::EventWarning);
					// std::cout << "pcapnextex failed with " << res << std::endl;
				}
				break;
			}
			if(res == 0)
				continue; // Keep waiting for that packet
			
			EthernetPacketizer::EthernetPacket packet(data, header->caplen);
			// Is this an ICS response packet (0xCAB2) from an ICS MAC, either to broadcast or directly to us?
			if(packet.etherType == 0xCAB2 && packet.srcMAC[0] == 0x00 && packet.srcMAC[1] == 0xFC && packet.srcMAC[2] == 0x70 && (
				memcmp(packet.destMAC, iface.macAddress, sizeof(packet.destMAC)) == 0 ||
				memcmp(packet.destMAC, BROADCAST_MAC, sizeof(packet.destMAC)) == 0 ||
				memcmp(packet.destMAC, ICS_UNSET_MAC, sizeof(packet.destMAC)) == 0
			)) {
				/* We have received a packet from a device. We don't know if this is the device we're
				 * looking for, we don't know if it's actually a response to our RequestSerialNumber
				 * or not, we just know we got something.
				 *
				 * Unlike most transport layers, we can't get the serial number here as we actually
				 * need to parse this message that has been returned. Some devices parse messages
				 * differently, so we need to use their communication layer. We could technically
				 * create a communication layer to parse the packet we have in `payload` here, but
				 * we'd need to be given a packetizer and decoder for the device. I'm intentionally
				 * avoiding passing that information down here for code quality's sake. Instead, pass
				 * the packet we received back up so the device can handle it.
				 */
				neodevice_handle_t handle = (neodevice_handle_t)((i << 24) | (packet.srcMAC[3] << 16) | (packet.srcMAC[4] << 8) | (packet.srcMAC[5]));
				PCAPFoundDevice* alreadyExists = nullptr;
				for(auto& dev : foundDevices)
					if(dev.device.handle == handle)
						alreadyExists = &dev;

				if(alreadyExists == nullptr) {
					PCAPFoundDevice foundDevice;
					foundDevice.device.handle = handle;
					foundDevice.discoveryPackets.push_back(std::move(packet.payload));
					foundDevices.push_back(foundDevice);
				} else {
					alreadyExists->discoveryPackets.push_back(std::move(packet.payload));
				}
			}
		}

		pcap_close(iface.fp);
		iface.fp = nullptr;
	}

	return foundDevices;
}

bool PCAP::IsHandleValid(neodevice_handle_t handle) {
	uint8_t netifIndex = (uint8_t)(handle >> 24);
	return (netifIndex < knownInterfaces.size());
}

PCAP::PCAP(device_eventhandler_t err, neodevice_t& forDevice) : Driver(err), device(forDevice), ethPacketizer(err) {
	if(IsHandleValid(device.handle)) {
		iface = knownInterfaces[(device.handle >> 24) & 0xFF];
		iface.fp = nullptr; // We're going to open our own connection to the interface. This should already be nullptr but just in case.

		deviceMAC[0] = 0x00;
		deviceMAC[1] = 0xFC;
		deviceMAC[2] = 0x70;
		deviceMAC[3] = (device.handle >> 16) & 0xFF;
		deviceMAC[4] = (device.handle >> 8) & 0xFF;
		deviceMAC[5] = device.handle & 0xFF;
		memcpy(ethPacketizer.deviceMAC, deviceMAC, 6);
		memcpy(ethPacketizer.hostMAC, iface.macAddress, 6);
	} else {
		openable = false;
	}
}

bool PCAP::open() {
	if(!openable)
		return false;

	if(isOpen())
		return false;

	// Open the interface
	iface.fp = pcap_open_live(iface.nameFromPCAP.c_str(), 65536, 1,
#ifdef __linux__ // -1 is required for instant reporting of new packets
		-1, // to_ms
#else // macOS gives BIOCSRTIMEOUT for -1 and no packets for 0
		1,
#endif
		errbuf);
	if(iface.fp == nullptr) {
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	pcap_setnonblock(iface.fp, 0, errbuf);
	pcap_set_immediate_mode(iface.fp, 1);

	// Create threads
	readThread = std::thread(&PCAP::readTask, this);
	writeThread = std::thread(&PCAP::writeTask, this);
	
	return true;
}

bool PCAP::isOpen() {
	return iface.fp != nullptr;
}

bool PCAP::close() {
	if(!isOpen())
		return false;

	closing = true; // Signal the threads that we are closing
	pcap_breakloop(iface.fp);
	pthread_cancel(readThread.native_handle());
	readThread.join();
	writeThread.join();
	closing = false;

	pcap_close(iface.fp);
	iface.fp = nullptr;

	uint8_t flush;
	WriteOperation flushop;
	while(readQueue.try_dequeue(flush)) {}
	while(writeQueue.try_dequeue(flushop)) {}

	return true;
}

void PCAP::readTask() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	while (!closing) {
		pcap_dispatch(iface.fp, -1, [](uint8_t* obj, const struct pcap_pkthdr* header, const uint8_t* data) {
			PCAP* driver = reinterpret_cast<PCAP*>(obj);
			if(driver->ethPacketizer.inputUp({data, data + header->caplen})) {
				const auto bytes = driver->ethPacketizer.outputUp();
				driver->readQueue.enqueue_bulk(bytes.data(), bytes.size());
			}
		}, (uint8_t*)this);
	}
}

void PCAP::writeTask() {
	WriteOperation writeOp;
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	while(!closing) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
			continue;

		// If we have a bunch of small packets to send, try to pack them into a packet
		// We use the average packet size to determine if we're likely to have enough room
		size_t bytesPushed = 0;
		size_t packetsPushed = 0;
		do {
			packetsPushed++;
			bytesPushed += writeOp.bytes.size();
			ethPacketizer.inputDown(std::move(writeOp.bytes));
		} while(bytesPushed < (EthernetPacketizer::MaxPacketLength - (bytesPushed / packetsPushed * 2)) && writeQueue.try_dequeue(writeOp));

		for(const auto& packet : ethPacketizer.outputDown()) {
			pcap_sendpacket(iface.fp, packet.data(), (int)packet.size());
		}
		// TODO Handle packet send errors
	}
}
