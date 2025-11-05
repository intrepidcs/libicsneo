#include <iostream>
#include "icsneo/icsneocpp.h"

int main(int, char**) {
	// Hash calculated by encrypting 16 zero bytes with AES ecb using the SAK
	// The device uses this hash to authenticate the key
	std::array<uint8_t, 16> sak = {0x01u, 0x02u, 0x03u, 0x04u, 0x01u, 0x02u, 0x03u, 0x04u, 0x01u, 0x02u, 0x03u, 0x04u, 0x01u, 0x02u, 0x03u, 0x04u};
	std::array<uint8_t, 16> hash = {0xDAu, 0x80u, 0xF2u, 0x20u, 0x8Bu, 0x59u, 0x88u, 0x12u, 0x94u, 0x4Eu, 0xEA, 0xB0, 0x52u, 0xDEu, 0xDEu, 0x66u};

	auto devices = icsneo::FindAllDevices();

	if(devices.size() == 0) {
		std::cout << "No device found" << std::endl;
		return -1;
	}

	std::shared_ptr<icsneo::Device> device = devices[0];
	
	if(!device->open()) {
		std::cout << "Failed to open device" << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return -1;
	}
	
	icsneo::MACsecConfig cfg(device->getType());

	if(!cfg) {
		std::cout << "Failed to initialize config" << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return -1;
	}

	// Fill out secure association information for each rx/tx port
	icsneo::MACsecRxSa rxSa;
	std::copy(sak.begin(), sak.end(), rxSa.sak.begin());
	std::copy(hash.begin(), hash.end(), rxSa.hashKey.begin());

	icsneo::MACsecTxSa txSa;
	std::copy(sak.begin(), sak.end(), txSa.sak.begin());
	std::copy(hash.begin(), hash.end(), txSa.hashKey.begin());

	// Add the secure associations to the config
	int rxSaHandle = cfg.addRxSa(rxSa);
	int txSaHandle = cfg.addTxSa(txSa);

	// Verify secure associations were configured properly
	if(rxSaHandle < 0 || txSaHandle < 0) {
		std::cout << "Failed to verify secure associations" << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return -1;
	}

	// Fill out security entity information for each rx/tx port
	icsneo::MACsecRxSecY rxSecY;
	rxSecY.cipher = icsneo::MACsecCipherSuite::GcmAes128;
	rxSecY.sci = 0x1122334455660001ull;

	icsneo::MACsecTxSecY txSecY;
	txSecY.cipher = icsneo::MACsecCipherSuite::GcmAes128;
	txSecY.sci = 0x1122334455660001ull;

	// Add security entites to the config
	int rxSecYHandle = cfg.addRxSecY(rxSecY, static_cast<uint8_t>(rxSaHandle));
	int txSecYHandle = cfg.addTxSecY(txSecY, static_cast<uint8_t>(txSaHandle));	

	// Verify security entities were configured properly
	if(rxSecYHandle < 0 || txSecYHandle < 0) {
		std::cout << "Failed to verify security entities" << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return -1;
	}

	// Enable communication directions
	cfg.setRxEnable(true);
	cfg.setTxEnable(true);

	// Write config to the device
	if(!device->writeMACsecConfig(cfg)) {
		std::cout << "Failed to write MACsec config" << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return -1;
	}
	
	device->close();
	return 0;
}