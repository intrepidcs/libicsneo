#ifndef __ICSNEO_IO_H_
#define __ICSNEO_IO_H_

#include <icsneo/icsneoc2types.h>

typedef struct _neomiscio_t {
	size_t number;
	bool supportsDigitalIn;
	bool supportsDigitalOut;
	bool supportsAnalogIn;
#ifdef __cplusplus
	_neomiscio_t(size_t n, bool dIn = false, bool dOut = false, bool aIn = false)
		: number(n), supportsDigitalIn(dIn), supportsDigitalOut(dOut), supportsAnalogIn(aIn) {}
#endif
} neomiscio_t;

#ifdef __cplusplus

namespace icsneo {

using MiscIO = neomiscio_t;

enum class IO {
	EthernetActivation = icsneoc2_io_type_eth_activation, // The DoIP activation line, 0 is HiZ and 1 is pulled up to VBAT
	USBHostPower = icsneoc2_io_type_usb_host_power,
	BackupPowerEnabled = icsneoc2_io_type_backup_power_en, // The FIRE 2's backup super capacitor
	BackupPowerGood = icsneoc2_io_type_backup_power_good, // Whether or not the FIRE 2's backup super capacitor is charged (read only)
	Misc = icsneoc2_io_type_misc, // General purpose IO on the device
	EMisc = icsneoc2_io_type_emisc, // Extended general purpose IO on the device
};

// Note that the C API does a static cast between this and neoio_t so keep them in sync!

}

#endif // __cplusplus

#ifdef __ICSNEOC_H_
typedef enum _neoio_t {
	ICSNEO_IO_ETH_ACTIVATION = (0),
	ICSNEO_IO_USB_HOST_POWER = (1),
	ICSNEO_IO_BACKUP_POWER_EN = (2),
	ICSNEO_IO_BACKUP_POWER_GOOD = (3),
	ICSNEO_IO_MISC = (4),
	ICSNEO_IO_EMISC = (5),
} neoio_t;
#endif

#endif