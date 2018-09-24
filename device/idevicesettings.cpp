#include "device/include/idevicesettings.h"

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

void IDeviceSettings::refresh() {
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

		if(gs_chksum != CalculateGSChecksum(rxSettings)) {
			std::cout << "Checksum mismatch while reading settings" << std::endl;
			return;
		}
		
		settings = std::move(rxSettings);
		settingsLoaded = true;
	}
}