#include <windows.h>
#include <winsock2.h>

#include "icsneo/platform/windows/pcap.h"
#include "icsneo/communication/network.h"
#include "icsneo/communication/communication.h"
#include "icsneo/communication/ethernetpacketizer.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/communication/ringbuffer.h"
#include <pcap.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#include <codecvt>
#include <chrono>
#include <iostream>
#include <locale>

using namespace icsneo;

static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

std::vector<PCAP::NetworkInterface> PCAP::knownInterfaces;

void PCAP::Find(std::vector<FoundDevice>& found) {
	const PCAPDLL& pcap = PCAPDLL::getInstance();
	if(!pcap.ok()) {
		EventManager::GetInstance().add(APIEvent::Type::PCAPCouldNotStart, APIEvent::Severity::Error);
		return;
	}

	// First we ask WinPCAP to give us all of the devices
	pcap_if_t* alldevs;
	char errbuf[PCAP_ERRBUF_SIZE] = { 0 };
	bool success = false;
	// Calling pcap.findalldevs_ex too quickly can cause various errors. Retry a few times in this case.
	for(auto retry = 0; retry < 10; retry++) {
		auto ret = pcap.findalldevs_ex((char*)PCAP_SRC_IF_STRING, nullptr, &alldevs, errbuf);
		if(ret == 0) {
			success = true;
			break;
		}
	}

	if(!success) {
		EventManager::GetInstance().add(APIEvent::Type::PCAPCouldNotFindDevices, APIEvent::Severity::Error);
		return;
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
		return;
	}
	std::vector<uint8_t> adapterAddressBuffer;
	adapterAddressBuffer.resize(size);
	if(GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, (IP_ADAPTER_ADDRESSES*)adapterAddressBuffer.data(), &size) != ERROR_SUCCESS) {
		EventManager::GetInstance().add(APIEvent::Type::PCAPCouldNotFindDevices, APIEvent::Severity::Error);
		return;
	}
	
	// aa->AdapterName constains a unique name of the interface like "{3B1D2791-435A-456F-8A7B-9CB0EEE5DAB3}"
	// iface.nameFromWinPCAP has "rpcap://\Device\NPF_{3B1D2791-435A-456F-8A7B-9CB0EEE5DAB3}"
	// We're comparing strings to match the Win32 info with the WinPCAP info
	for(IP_ADAPTER_ADDRESSES* aa = (IP_ADAPTER_ADDRESSES*)adapterAddressBuffer.data(); aa != nullptr; aa = aa->Next) {
		for(auto& iface : interfaces) {
			if(iface.nameFromWinPCAP.find(aa->AdapterName) == std::string::npos)
				continue; // This is not the interface that corresponds

			memcpy(iface.macAddress, aa->PhysicalAddress, sizeof(iface.macAddress));
			iface.nameFromWin32API = aa->AdapterName;
			iface.descriptionFromWin32API = converter.to_bytes(aa->Description);
			iface.friendlyNameFromWin32API = converter.to_bytes(aa->FriendlyName);
			if(iface.descriptionFromWin32API.find("LAN9512/LAN9514") != std::string::npos) {
				// This is an Ethernet EVB device
				iface.fullName = "Intrepid Ethernet EVB ( " + iface.friendlyNameFromWin32API + " : " + iface.descriptionFromWin32API + " )";
			} else {
				iface.fullName = iface.friendlyNameFromWin32API + " : " + iface.descriptionFromWin32API;
			}
		}
	}

	for(auto& iface : interfaces) {
		bool exists = false;
		for(auto& known : knownInterfaces)
			if(memcmp(iface.macAddress, known.macAddress, sizeof(iface.macAddress)) == 0)
				exists = true;
		if(!exists)
			knownInterfaces.emplace_back(iface);
	}

	constexpr auto openflags = (PCAP_OPENFLAG_MAX_RESPONSIVENESS | PCAP_OPENFLAG_NOCAPTURE_LOCAL);
	for(size_t i = 0; i < knownInterfaces.size(); i++) {
		auto& iface = knownInterfaces[i];
		if(iface.fullName.length() == 0)
			continue; // Win32 did not find this interface in the previous step

		iface.fp = pcap.open(iface.nameFromWinPCAP.c_str(), 1518, openflags, 1, nullptr, errbuf);

		if(iface.fp == nullptr)
			continue; // Could not open the interface

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
		pcap.sendpacket(iface.fp, bs.data(), (int)bs.size());

		auto timeout = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(250);
		constexpr const size_t TempBufferSize = 4096;
		RingBuffer tempBuffer(TempBufferSize);
		while(std::chrono::high_resolution_clock::now() <= timeout) { // Wait up to 5ms for the response
			struct pcap_pkthdr* header;
			const uint8_t* data;
			auto res = pcap.next_ex(iface.fp, &header, &data);
			if(res < 0) {
				//std::cout << "pcapnextex failed with " << res << std::endl;
				break;
			}
			if(res == 0)
				continue; // Keep waiting for that packet
			
			EthernetPacketizer ethPacketizer([](APIEvent::Type, APIEvent::Severity) {});
			memcpy(ethPacketizer.hostMAC, iface.macAddress, sizeof(ethPacketizer.hostMAC));
			ethPacketizer.allowInPacketsFromAnyMAC = true;
			if(!ethPacketizer.inputUp({ data, data + header->caplen }))
				continue; // This packet is not for us

			Packetizer packetizer([](APIEvent::Type, APIEvent::Severity) {});
			tempBuffer.write(ethPacketizer.outputUp());
			if(!packetizer.input(tempBuffer))
				continue; // This packet was not well formed

			EthernetPacketizer::EthernetPacket decoded(data, header->caplen);
			for(const auto& packet : packetizer.output()) {
				Decoder decoder([](APIEvent::Type, APIEvent::Severity) {});
				std::shared_ptr<Message> message;
				if(!decoder.decode(message, packet))
					continue;

				const auto serial = std::dynamic_pointer_cast<SerialNumberMessage>(message);
				if(!serial || serial->deviceSerial.size() != 6)
					continue;

				FoundDevice foundDevice;
				foundDevice.handle = (neodevice_handle_t)((i << 24) | (decoded.srcMAC[3] << 16) | (decoded.srcMAC[4] << 8) | (decoded.srcMAC[5]));
				foundDevice.productId = decoded.srcMAC[2];
				memcpy(foundDevice.serial, serial->deviceSerial.c_str(), sizeof(foundDevice.serial) - 1);
				foundDevice.serial[sizeof(foundDevice.serial) - 1] = '\0';

				if(std::any_of(found.begin(), found.end(), [&](const auto& found) { return ::strncmp(foundDevice.serial, found.serial, sizeof(foundDevice.serial)) == 0; }))
					continue;

				foundDevice.makeDriver = [](const device_eventhandler_t& reportFn, neodevice_t& device) {
					return std::unique_ptr<Driver>(new PCAP(reportFn, device));
				};

				found.push_back(foundDevice);
			}
		}

		pcap.close(iface.fp);
		iface.fp = nullptr;
	}
}

bool PCAP::IsHandleValid(neodevice_handle_t handle) {
	uint8_t netifIndex = (uint8_t)(handle >> 24);
	return (netifIndex < knownInterfaces.size());
}

PCAP::PCAP(const device_eventhandler_t& err, neodevice_t& forDevice) : Driver(err), device(forDevice), pcap(PCAPDLL::getInstance()), ethPacketizer(err) {
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
	iface.fp = pcap.open(iface.nameFromWinPCAP.c_str(), 65536, PCAP_OPENFLAG_MAX_RESPONSIVENESS | PCAP_OPENFLAG_NOCAPTURE_LOCAL, 50, nullptr, errbuf);
	if(iface.fp == nullptr) {
		report(APIEvent::Type::DriverFailedToOpen, APIEvent::Severity::Error);
		return false;
	}

	// Create threads
	readThread = std::thread(&PCAP::readTask, this);
	writeThread = std::thread(&PCAP::writeTask, this);
	transmitThread = std::thread(&PCAP::transmitTask, this);
	
	return true;
}

bool PCAP::isOpen() {
	return iface.fp != nullptr;
}

bool PCAP::close() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	closing = true; // Signal the threads that we are closing
	readThread.join();
	writeThread.join();
	transmitThread.join();
	closing = false;

	pcap.close(iface.fp);
	iface.fp = nullptr;

	WriteOperation flushop;
	readBuffer.clear();
	while(writeQueue.try_dequeue(flushop)) {}
	transmitQueue = nullptr;

	return true;
}

void PCAP::readTask() {
	struct pcap_pkthdr* header;
	const uint8_t* data;
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	while(!closing) {
		auto readBytes = pcap.next_ex(iface.fp, &header, &data);
		if(readBytes < 0) {
			report(APIEvent::Type::FailedToRead, APIEvent::Severity::Error);
			break;
		}
		if(readBytes == 0)
			continue; // Keep waiting for that packet

		if(ethPacketizer.inputUp({data, data + header->caplen})) {
			const auto bytes = ethPacketizer.outputUp();
			writeToReadBuffer(bytes.data(), bytes.size());
		}
	}
}

void PCAP::writeTask() {
	WriteOperation writeOp;
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	pcap_send_queue* queue1 = pcap.sendqueue_alloc(128000);
	pcap_send_queue* queue2 = pcap.sendqueue_alloc(128000);
	pcap_send_queue* queue = queue1;

	while(!closing) {
		// Potentially, we added frames to a second queue faster than the other thread was able to hand the first
		// off to the kernel. In that case, wait for a minimal amount of time before checking whether we can
		// transmit it again.
		if(writeQueue.wait_dequeue_timed(writeOp, std::chrono::milliseconds(queue->len ? 1 : 100))) {
			unsigned int i = 0;
			do {
				ethPacketizer.inputDown(std::move(writeOp.bytes));
				if(i++ >= (queue->maxlen - queue->len) / 1518 / 3)
					break; // Not safe to try to fit any more packets in this queue, let it transmit and come around again
			} while(writeQueue.try_dequeue(writeOp));

			for(const auto& data : ethPacketizer.outputDown()) {
				pcap_pkthdr header = {};
				header.caplen = header.len = bpf_u_int32(data.size());
				if(pcap.sendqueue_queue(queue, &header, data.data()) != 0)
					report(APIEvent::Type::FailedToWrite, APIEvent::Severity::EventWarning);
			}
		}
		
		std::unique_lock<std::mutex> lk(transmitQueueMutex);
		// Check if we want to transmit our current queue
		// If we're not currently transmitting a queue, let this one transmit immediately for good latency
		// If our queue is full and we're transmitting the other, we can't accept any more packets out of the writeQueue
		// In that case we're putting as many packets into the driver as possible, so wait for it to be free
		// This puts the backpressure on the writeQueue
		if(queue->len && (!transmitQueue || queue->len + (1518*2) >= queue->maxlen)) {
			if(transmitQueue) // Need to wait for the queue to become available
				transmitQueueCV.wait(lk, [this] { return !transmitQueue; });
			
			// Time to swap
			transmitQueue = queue;
			lk.unlock();
			transmitQueueCV.notify_one();

			// Set up our next queue
			if(queue == queue1) {
				pcap.sendqueue_destroy(queue2);
				queue = queue2 = pcap.sendqueue_alloc(128000);
			} else {
				pcap.sendqueue_destroy(queue1);
				queue = queue1 = pcap.sendqueue_alloc(128000);
			}
		}
	}

	pcap.sendqueue_destroy(queue1);
	pcap.sendqueue_destroy(queue2);
}

void PCAP::transmitTask() {
	while(!closing) {
		std::unique_lock<std::mutex> lk(transmitQueueMutex);
		if(transmitQueueCV.wait_for(lk, std::chrono::milliseconds(100), [this] { return !!transmitQueue; }) && !closing && transmitQueue) {
			pcap_send_queue* current = transmitQueue;
			lk.unlock();
			pcap.sendqueue_transmit(iface.fp, current, 0);
			{
				std::lock_guard<std::mutex> lk2(transmitQueueMutex);
				transmitQueue = nullptr;
			}
			transmitQueueCV.notify_one();
		}
	}
}
