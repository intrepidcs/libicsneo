#include "icsneo/platform/windows/pcap.h"
#include "icsneo/communication/network.h"
#include "icsneo/communication/communication.h"
#include "icsneo/communication/packetizer.h"
#include <pcap.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#include <codecvt>
#include <chrono>
#include <iostream>

using namespace icsneo;

static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
static const uint8_t BROADCAST_MAC[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

std::vector<PCAP::NetworkInterface> PCAP::knownInterfaces;

std::vector<PCAP::PCAPFoundDevice> PCAP::FindAll() {
	std::vector<PCAPFoundDevice> foundDevices;
	PCAPDLL pcap;
	if(!pcap.ok()) {
		EventManager::GetInstance().add(APIEvent::Type::PCAPCouldNotStart, APIEvent::Severity::Error);
		return std::vector<PCAPFoundDevice>();
	}

	// First we ask WinPCAP to give us all of the devices
	pcap_if_t* alldevs;
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	bool success = false;
	// Calling pcap.findalldevs_ex too quickly can cause various errors. Retry a few times in this case.
	for(auto retry = 0; retry < 10; retry++) {
		auto ret = pcap.findalldevs_ex(PCAP_SRC_IF_STRING, nullptr, &alldevs, errbuf);
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
		NetworkInterface netif;
		netif.nameFromWinPCAP = dev->name;
		netif.descriptionFromWinPCAP = dev->description;
		interfaces.push_back(netif);
	}

	pcap.freealldevs(alldevs);

	// Now we're going to ask Win32 for the information as well
	ULONG size = 0;
	if(GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, nullptr, &size) != ERROR_BUFFER_OVERFLOW) {
		EventManager::GetInstance().add(APIEvent::Type::PCAPCouldNotFindDevices, APIEvent::Severity::Error);
		return std::vector<PCAPFoundDevice>();
	}
	std::vector<uint8_t> adapterAddressBuffer;
	adapterAddressBuffer.resize(size);
	if(GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, (IP_ADAPTER_ADDRESSES*)adapterAddressBuffer.data(), &size) != ERROR_SUCCESS) {
		EventManager::GetInstance().add(APIEvent::Type::PCAPCouldNotFindDevices, APIEvent::Severity::Error);
		return std::vector<PCAPFoundDevice>();
	}
	
	// aa->AdapterName constains a unique name of the interface like "{3B1D2791-435A-456F-8A7B-9CB0EEE5DAB3}"
	// interface.nameFromWinPCAP has "rpcap://\Device\NPF_{3B1D2791-435A-456F-8A7B-9CB0EEE5DAB3}"
	// We're comparing strings to match the Win32 info with the WinPCAP info
	for(IP_ADAPTER_ADDRESSES* aa = (IP_ADAPTER_ADDRESSES*)adapterAddressBuffer.data(); aa != nullptr; aa = aa->Next) {
		for(auto& interface : interfaces) {
			if(interface.nameFromWinPCAP.find(aa->AdapterName) == std::string::npos)
				continue; // This is not the interface that corresponds

			memcpy(interface.macAddress, aa->PhysicalAddress, sizeof(interface.macAddress));
			interface.nameFromWin32API = aa->AdapterName;
			interface.descriptionFromWin32API = converter.to_bytes(aa->Description);
			interface.friendlyNameFromWin32API = converter.to_bytes(aa->FriendlyName);
			if(interface.descriptionFromWin32API.find("LAN9512/LAN9514") != std::string::npos) {
				// This is an Ethernet EVB device
				interface.fullName = "Intrepid Ethernet EVB ( " + interface.friendlyNameFromWin32API + " : " + interface.descriptionFromWin32API + " )";
			} else {
				interface.fullName = interface.friendlyNameFromWin32API + " : " + interface.descriptionFromWin32API;
			}
		}
	}

	for(auto& interface : interfaces) {
		bool exists = false;
		for(auto& known : knownInterfaces)
			if(memcmp(interface.macAddress, known.macAddress, sizeof(interface.macAddress)) == 0)
				exists = true;
		if(!exists)
			knownInterfaces.emplace_back(interface);
	}

	constexpr auto openflags = (PCAP_OPENFLAG_PROMISCUOUS | PCAP_OPENFLAG_MAX_RESPONSIVENESS | PCAP_OPENFLAG_NOCAPTURE_LOCAL);
	for(size_t i = 0; i < knownInterfaces.size(); i++) {
		auto& interface = knownInterfaces[i];
		if(interface.fullName.length() == 0)
			continue; // Win32 did not find this interface in the previous step

		interface.fp = pcap.open(interface.nameFromWinPCAP.c_str(), 30, openflags, 1, nullptr, errbuf);

		if(interface.fp == nullptr)
			continue; // Could not open the interface

		EthernetPacket requestPacket;
		requestPacket.payload.reserve(4);
		requestPacket.payload = {
			((1 << 4) | (uint8_t)Network::NetID::Main51), // Packet size of 1 on NETID_MAIN51
			(uint8_t)Command::RequestSerialNumber
		};
		requestPacket.payload.push_back(Packetizer::ICSChecksum(requestPacket.payload));
		requestPacket.payload.insert(requestPacket.payload.begin(), 0xAA);

		auto bs = requestPacket.getBytestream();
		pcap.sendpacket(interface.fp, bs.data(), (int)bs.size());

		auto timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(5);
		while(std::chrono::high_resolution_clock::now() <= timeout) { // Wait up to 5ms for the response
			struct pcap_pkthdr* header;
			const uint8_t* data;
			auto res = pcap.next_ex(interface.fp, &header, &data);
			if(res < 0) {
				//std::cout << "pcapnextex failed with " << res << std::endl;
				break;
			}
			if(res == 0)
				continue; // Keep waiting for that packet
			
			EthernetPacket packet(data, header->caplen);
			// Is this an ICS response packet (0xCAB2) from an ICS MAC, either to broadcast or directly to us?
			if(packet.etherType == 0xCAB2 && packet.srcMAC[0] == 0x00 && packet.srcMAC[1] == 0xFC && packet.srcMAC[2] == 0x70 && (
				memcmp(packet.destMAC, interface.macAddress, sizeof(packet.destMAC)) == 0 ||
				memcmp(packet.destMAC, BROADCAST_MAC, sizeof(packet.destMAC)) == 0
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

		pcap.close(interface.fp);
		interface.fp = nullptr;
	}

	return foundDevices;
}

bool PCAP::IsHandleValid(neodevice_handle_t handle) {
	uint8_t netifIndex = (uint8_t)(handle >> 24);
	return (netifIndex < knownInterfaces.size());
}

PCAP::PCAP(const device_eventhandler_t& err, neodevice_t& forDevice) : ICommunication(err), device(forDevice) {
	if(IsHandleValid(device.handle)) {
		interface = knownInterfaces[(device.handle >> 24) & 0xFF];
		interface.fp = nullptr; // We're going to open our own connection to the interface. This should already be nullptr but just in case.

		deviceMAC[0] = 0x00;
		deviceMAC[1] = 0xFC;
		deviceMAC[2] = 0x70;
		deviceMAC[3] = (device.handle >> 16) & 0xFF;
		deviceMAC[4] = (device.handle >> 8) & 0xFF;
		deviceMAC[5] = device.handle & 0xFF;
	} else {
		openable = false;
	}
}

bool PCAP::open() {
	if(!openable) {
		report(APIEvent::Type::InvalidNeoDevice, APIEvent::Severity::Error);
		return false;
	}

	if(!pcap.ok()) {
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	if(isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::Error);
		return false;
	}	

	// Open the interface
	interface.fp = pcap.open(interface.nameFromWinPCAP.c_str(), 100, PCAP_OPENFLAG_PROMISCUOUS | PCAP_OPENFLAG_MAX_RESPONSIVENESS, 1, nullptr, errbuf);
	if(interface.fp == nullptr) {
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	// Create threads
	readThread = std::thread(&PCAP::readTask, this);
	writeThread = std::thread(&PCAP::writeTask, this);
	
	return true;
}

bool PCAP::isOpen() {
	return interface.fp != nullptr;
}

bool PCAP::close() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	closing = true; // Signal the threads that we are closing
	readThread.join();
	writeThread.join();
	closing = false;

	pcap.close(interface.fp);
	interface.fp = nullptr;

	uint8_t flush;
	WriteOperation flushop;
	while(readQueue.try_dequeue(flush)) {}
	while(writeQueue.try_dequeue(flushop)) {}

	return true;
}

void PCAP::readTask() {
	struct pcap_pkthdr* header;
	const uint8_t* data;
	while(!closing) {
		auto readBytes = pcap.next_ex(interface.fp, &header, &data);
		if(readBytes < 0) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			break;
		}
		if(readBytes == 0)
			continue; // Keep waiting for that packet

		EthernetPacket packet(data, header->caplen);

		if(packet.etherType != 0xCAB2)
			continue; // Not a packet to host

		if(memcmp(packet.destMAC, interface.macAddress, sizeof(packet.destMAC)) != 0 &&
			memcmp(packet.destMAC, BROADCAST_MAC, sizeof(packet.destMAC)) != 0)
			continue; // Packet is not addressed to us or broadcast

		if(memcmp(packet.srcMAC, deviceMAC, sizeof(deviceMAC)) != 0)
			continue; // Not a packet from the device we're concerned with

		readQueue.enqueue_bulk(packet.payload.data(), packet.payload.size());
	}
}

void PCAP::writeTask() {
	WriteOperation writeOp;
	uint16_t sequence = 0;
	EthernetPacket sendPacket;
	
	// Set MAC address of packet
	memcpy(sendPacket.srcMAC, interface.macAddress, sizeof(sendPacket.srcMAC));
	memcpy(sendPacket.destMAC, deviceMAC, sizeof(deviceMAC));

	while(!closing) {
		if(!writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(100)))
			continue;

		sendPacket.packetNumber = sequence++;
		sendPacket.payload = std::move(writeOp.bytes);
		auto bs = sendPacket.getBytestream();
		if(!closing)
			pcap.sendpacket(interface.fp, bs.data(), (int)bs.size());
		onWrite();
		// TODO Handle packet send errors
	}
}

PCAP::EthernetPacket::EthernetPacket(const std::vector<uint8_t>& bytestream) {
	loadBytestream(bytestream);
}

PCAP::EthernetPacket::EthernetPacket(const uint8_t* data, size_t size) {
	std::vector<uint8_t> bs(size);
	for(size_t i = 0; i < size; i++)
		bs[i] = data[i];
	loadBytestream(bs);
}

int PCAP::EthernetPacket::loadBytestream(const std::vector<uint8_t>& bytestream) {
	errorWhileDecodingFromBytestream = 0;
	for(size_t i = 0; i < 6; i++)
		destMAC[i] = bytestream[i];
	for(size_t i = 0; i < 6; i++)
		srcMAC[i] = bytestream[i + 6];
	etherType = (bytestream[12] << 8) | bytestream[13];
	icsEthernetHeader = (bytestream[14] << 24) | (bytestream[15] << 16) | (bytestream[16] << 8) | bytestream[17];
	uint16_t payloadSize = bytestream[18] | (bytestream[19] << 8);
	packetNumber = bytestream[20] | (bytestream[21] << 8);
	uint16_t packetInfo = bytestream[22] | (bytestream[23] << 8);
	firstPiece = packetInfo & 1;
	lastPiece = (packetInfo >> 1) & 1;
	bufferHalfFull = (packetInfo >> 2) & 2;
	payload = std::vector<uint8_t>(bytestream.begin() + 24, bytestream.end());
	size_t payloadActualSize = payload.size();
	if(payloadActualSize < payloadSize)
		errorWhileDecodingFromBytestream = 1;
	payload.resize(payloadSize);
	return errorWhileDecodingFromBytestream;
}

std::vector<uint8_t> PCAP::EthernetPacket::getBytestream() const {
	size_t payloadSize = payload.size();
	std::vector<uint8_t> bytestream;
	bytestream.reserve(6 + 6 + 2 + 4 + 2 + 2 + 2 + payloadSize);
	for(size_t i = 0; i < 6; i++)
		bytestream.push_back(destMAC[i]);
	for(size_t i = 0; i < 6; i++)
		bytestream.push_back(srcMAC[i]);
	// EtherType should be put into the bytestream as big endian
	bytestream.push_back((uint8_t)(etherType >> 8));
	bytestream.push_back((uint8_t)(etherType));
	// Our Ethernet header should be put into the bytestream as big endian
	bytestream.push_back((uint8_t)(icsEthernetHeader >> 24));
	bytestream.push_back((uint8_t)(icsEthernetHeader >> 16));
	bytestream.push_back((uint8_t)(icsEthernetHeader >> 8));
	bytestream.push_back((uint8_t)(icsEthernetHeader));
	// The payload size comes next, it's little endian
	bytestream.push_back((uint8_t)(payloadSize));
	bytestream.push_back((uint8_t)(payloadSize >> 8));
	// Packet number is little endian
	bytestream.push_back((uint8_t)(packetNumber));
	bytestream.push_back((uint8_t)(packetNumber >> 8));
	// Packet info gets assembled into a bitfield
	uint16_t packetInfo = 0;
	packetInfo |= firstPiece & 1;
	packetInfo |= (lastPiece & 1) << 1;
	packetInfo |= (bufferHalfFull & 1) << 2;
	bytestream.push_back((uint8_t)(packetInfo));
	bytestream.push_back((uint8_t)(packetInfo >> 8));
	bytestream.insert(bytestream.end(), payload.begin(), payload.end());
	return bytestream;
}