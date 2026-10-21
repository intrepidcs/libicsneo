#ifndef __BOOTLOADER_RADMULTICHIPCOMMUNICATION_H_
#define __BOOTLOADER_RADMULTICHIPCOMMUNICATION_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/communication/radcommunication.h"
#include "icsneo/device/extensions/bootloader/communication/application.h"

#include <cstdint>
#include <memory>

namespace icsneo {

namespace Bootloader {

class RADMultiChipCommunication : public RADCommunication {
private:
	std::unique_ptr<Application> standardComm;
	const bool doAuthChecks;
	const uint8_t slot;
	bool authenticate() override;
public:
	RADMultiChipCommunication(icsneo::Device& dev, const ChipInfo& chipToFlash, bool auth, uint8_t slot = UINT8_MAX);

	bool flash(const FWRepo& fw, const icsneo::Device::OpenStatusHandler& handler) override;
	bool open() override;
	bool close() override;
	bool enterApplication() override;
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_RADMULTICHIPCOMMUNICATION_H_
