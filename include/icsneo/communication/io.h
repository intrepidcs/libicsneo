#ifndef __ICSNEO_IO_H_
#define __ICSNEO_IO_H_

#ifdef __cplusplus

namespace icsneo {

enum class IO {
	EthernetActivation = 0, // The DoIP activation line, 0 is HiZ and 1 is pulled up to VBAT
	USBHostPower = 1,
	BackupPowerEnabled = 2, // The FIRE 2's backup super capacitor
	BackupPowerGood = 3, // Whether or not the FIRE 2's backup super capacitor is charged (read only)
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
} neoio_t;
#endif

#endif