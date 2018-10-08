#include "device/include/idevicesettings.h"
#include "communication/message/filter/include/main51messagefilter.h"
#include <cstring>

using namespace icsneo;

uint16_t IDeviceSettings::CalculateGSChecksum(const std::vector<uint8_t>& settings) {
	uint16_t gs_crc = 0;
	const uint16_t* p = (const uint16_t*)settings.data();
	size_t words = settings.size();
	if(words % 2 == 1)
		return 0xFFFF; // Somehow settings is not word aligned
	words /= 2;

	while(words--) {
		uint16_t temp = *p;

		for (int i = 0; i < 16; i++) {
			bool iBit = temp & 1;
			
			int iCrcNxt;
			//CRCNXT = NXTBIT EXOR CRC_RG(15);
			if (gs_crc & (1 << 15))
				iCrcNxt = iBit ^ 1;
			else
				iCrcNxt = iBit;
			iCrcNxt = iCrcNxt & 0x01;


			// CRC_RG(15:1) = CRC_RG(14:0); // shift left by
			gs_crc = gs_crc << 1;
			gs_crc = gs_crc & 0xFFFE;// clear first bit

			if (iCrcNxt)//CRC_RG(14:0) = CRC_RG(14:0) EXOR (4599hex);
				gs_crc = gs_crc ^ 0xa001;

			temp >>= 1;
		}

		p++;
	}
	return gs_crc;
}

void IDeviceSettings::refresh(bool ignoreChecksum) {
	std::vector<uint8_t> rxSettings;
	bool ret = com->getSettingsSync(rxSettings);
	if(ret) {
		if(rxSettings.size() < 6) // We need to at least have the header of GLOBAL_SETTINGS
			return;

		constexpr size_t gs_size = 3 * sizeof(uint16_t);
		size_t rxLen = rxSettings.size() - gs_size;

		uint16_t gs_version = rxSettings[0] | (rxSettings[1] << 8);
		uint16_t gs_len = rxSettings[2] | (rxSettings[3] << 8);
		uint16_t gs_chksum = rxSettings[4] | (rxSettings[5] << 8);
		rxSettings.erase(rxSettings.begin(), rxSettings.begin() + gs_size);

		if(gs_version != 5) {
			std::cout << "gs_version was " << gs_version << " instead of 5.\nPlease update your firmware." << std::endl;
			return;
		}

		if(rxLen != gs_len) {
			std::cout << "rxLen was " << rxLen << " and gs_len was " << gs_len << " while reading settings" << std::endl;
			return;
		}

		if(!ignoreChecksum && gs_chksum != CalculateGSChecksum(rxSettings)) {
			std::cout << "Checksum mismatch while reading settings" << std::endl;
			return;
		}

		settings = std::move(rxSettings);
		settingsLoaded = true;

		if(settings.size() != structSize) {
			std::cout << "Settings size was " << settings.size() << " bytes but it should be " << structSize << " bytes for this device" << std::endl;
			settingsLoaded = false;
		}
	}
}

bool IDeviceSettings::apply(bool temporary) {
	std::vector<uint8_t> bytestream;
	bytestream.resize(7 + structSize);
	bytestream[0] = 0x00;
	bytestream[1] = GS_VERSION;
	bytestream[2] = GS_VERSION >> 8;
	bytestream[3] = (uint8_t)structSize;
	bytestream[4] = (uint8_t)(structSize >> 8);
	uint16_t gs_checksum = CalculateGSChecksum(settings);
	bytestream[5] = (uint8_t)gs_checksum;
	bytestream[6] = (uint8_t)(gs_checksum >> 8);
	memcpy(bytestream.data() + 7, getRawStructurePointer(), structSize);

	com->sendCommand(Command::SetSettings, bytestream);
	std::shared_ptr<Message> msg = com->waitForMessageSync(std::make_shared<Main51MessageFilter>(Command::SetSettings), std::chrono::milliseconds(1000));

	if(!msg || msg->data[0] != 1) { // We did not receive a response
		refresh(); // Attempt to get the settings from the device so we're up to date if possible
		return false;
	}

	refresh(true); // Refresh ignoring checksum
	// The device might modify the settings once they are applied, however in this case it does not update the checksum
	// We refresh to get these updates, update the checksum, and send it back so it's all in sync
	gs_checksum = CalculateGSChecksum(settings);
	bytestream[5] = (uint8_t)gs_checksum;
	bytestream[6] = (uint8_t)(gs_checksum >> 8);
	memcpy(bytestream.data() + 7, getRawStructurePointer(), structSize);

	com->sendCommand(Command::SetSettings, bytestream);
	msg = com->waitForMessageSync(std::make_shared<Main51MessageFilter>(Command::SetSettings), std::chrono::milliseconds(1000));
	if(!msg || msg->data[0] != 1) {
		refresh();
		return false;
	}

	if(!temporary) {
		com->sendCommand(Command::SaveSettings);
		msg = com->waitForMessageSync(std::make_shared<Main51MessageFilter>(Command::SaveSettings), std::chrono::milliseconds(5000));
	}
	
	refresh(); // Refresh our buffer with what the device has, whether we were successful or not

	return (msg && msg->data[0] == 1); // Device sends 0x01 for success
}

bool IDeviceSettings::applyDefaults(bool temporary) {
	com->sendCommand(Command::SetDefaultSettings);
	std::shared_ptr<Message> msg = com->waitForMessageSync(std::make_shared<Main51MessageFilter>(Command::SetDefaultSettings), std::chrono::milliseconds(1000));
	if(!msg || msg->data[0] != 1) {
		refresh();
		return false;
	}

	refresh(true); // Refresh ignoring checksum
	// The device might modify the settings once they are applied, however in this case it does not update the checksum
	// We refresh to get these updates, update the checksum, and send it back so it's all in sync
	std::vector<uint8_t> bytestream;
	bytestream.resize(7 + structSize);
	bytestream[0] = 0x00;
	bytestream[1] = GS_VERSION;
	bytestream[2] = GS_VERSION >> 8;
	bytestream[3] = (uint8_t)structSize;
	bytestream[4] = (uint8_t)(structSize >> 8);
	uint16_t gs_checksum = CalculateGSChecksum(settings);
	bytestream[5] = (uint8_t)gs_checksum;
	bytestream[6] = (uint8_t)(gs_checksum >> 8);
	memcpy(bytestream.data() + 7, getRawStructurePointer(), structSize);

	com->sendCommand(Command::SetSettings, bytestream);
	msg = com->waitForMessageSync(std::make_shared<Main51MessageFilter>(Command::SetSettings), std::chrono::milliseconds(1000));
	if(!msg || msg->data[0] != 1) {
		refresh();
		return false;
	}

	if(!temporary) {
		com->sendCommand(Command::SaveSettings);
		msg = com->waitForMessageSync(std::make_shared<Main51MessageFilter>(Command::SaveSettings), std::chrono::milliseconds(5000));
	}
	
	refresh(); // Refresh our buffer with what the device has, whether we were successful or not

	return (msg && msg->data[0] == 1); // Device sends 0x01 for success
}

bool IDeviceSettings::setBaudrateFor(Network net, uint32_t baudrate) {
	switch(net.getType()) {
		case Network::Type::CAN: {
			CAN_SETTINGS* cfg = getCANSettingsFor(net);
			if(cfg == nullptr)
				return false;
				
			uint8_t newBaud = getEnumValueForBaudrate(baudrate);
			if(newBaud == 0xFF)
				return false;
			cfg->Baudrate = newBaud;
			cfg->auto_baud = false;
			cfg->SetBaudrate = AUTO; // Device will use the baudrate value to set the TQ values
			return true;
		}
		default:
			return false;
	}
}

template<typename T> bool IDeviceSettings::setStructure(const T& newStructure) {
	if(sizeof(T) != structSize)
		return false; // The wrong structure was passed in for the current device
	
	if(settings.size() != structSize)
		settings.resize(structSize);
	
	memcpy(settings.data(), &newStructure, structSize);
	return true;
}

uint8_t IDeviceSettings::getEnumValueForBaudrate(uint32_t baudrate) {
	switch(baudrate) {
		case 20000:
			return BPS20;
		case 33000:
			return BPS33;
		case 50000:
			return BPS50;
		case 62000:
			return BPS62;
		case 83000:
			return BPS83;
		case 100000:
			return BPS100;
		case 125000:
			return BPS125;
		case 250000:
			return BPS250;
		case 500000:
			return BPS500;
		case 800000:
			return BPS800;
		case 1000000:
			return BPS1000;
		case 666000:
			return BPS666;
		case 2000000:
			return BPS2000;
		case 4000000:
			return BPS4000;
		case 5000000:
			return CAN_BPS5000;
		case 6667000:
			return CAN_BPS6667;
		case 8000000:
			return CAN_BPS8000;
		case 10000000:
			return CAN_BPS10000;
		default:
			return 0xFF;
	}
}