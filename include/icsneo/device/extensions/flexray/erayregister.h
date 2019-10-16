#ifndef __ERAYREGISTER_H_
#define __ERAYREGISTER_H_

#include <cstdint>

namespace icsneo {

enum class ERAYRegister : uint32_t {
	// Customer Registers : Not Part of ERAY, part of FUJITSU FlexRay  ASSP MB88121C
	VER = 0x0000,// Version Information Register  reset = 0440_79FF  Access = r
	CCNT = 0x0004,// Clock Control Register  reset = 0000_0000   Access =  r/w
	DBGS_DMAS = 0x0008,//  Debug Support Register / DMA Support Register reset = 0000_0000    Acess = r/w
	INT_REG = 0x000C,//  Interrupt Register  Register reset = 0000_0000    Acess = r/w
	LCK = 0x001C,// Lock Register  reset = 0000_0000  Acess = r/w

	// Interrupt-Related Registers
	EIR = 0x0020,// Error Interrupt Register  reset=0000_0000    Access = r/w
	SIR = 0x0024,// Status Interrupt Register  0000_0000    r/w
	EILS = 0x0028,// Error Interrupt Line Select Register  0000_0000    r/w
	SILS = 0x002C,// Status Interrupt Line Select Register  0303_FFFF    r/w
	EIES = 0x0030,// Error Interrupt Enable Register (set)  0000_0000    r/w
	EIER = 0x0034,// Error Interrupt Enable Register (reset)  0000_0000    r/w
	SIES = 0x0038,// Status Interrupt Enable Register (set)  0000_0000    r/w
	SIER = 0x003C,// Status Interrupt Enable Register (reset)  0000_0000    r/w
	ILE = 0x0040,// Interrupt Line Enable Register  0000_0000    r/w
	T0C = 0x0044,// Timer 0 Configuration Register 0  0000_0000    r/w
	T1C = 0x0048,// Timer 0 Configuration Register 1  0002_0000    r/w
	STPW1 = 0x004C,// Stop Watch Register 1  0000_0000    r/w
	STPW2 = 0x0050,// Stop Watch Register 2  0000_0000    r
	//0x0054�0x007C-  reserved(11)  0000_0000    r

	// System Universal Control (SUC)
	// The system universal control controls the following functions.
	//� Configuration
	//� Wakeup
	//� Startup
	//� Normal operation
	//� Passive operation
	//� Monitor mode
	SUCC1 = 0x0080,// SUC Configuration Register 1        0C40_1000  r/w
	SUCC2 = 0x0084,// SUC Configuration Register 2        0100_0504  r/w
	SUCC3 = 0x0088,// SUC Configuration Register 3        0000_0011  r/w

	// Network Management (NEM)
	// Sets the handling of the network management vector
	NEMC = 0x008C,// NEM Configuration Register    0000_0000    r/w

	PRTC1 = 0x0090,// PRT Configuration Register 1      084C_0633    r/w
	PRTC2 = 0x0094,// PRT Configuration Register 2      0F2D_0A0E    r/w
	MHDC = 0x0098,// MHD Configuration Register        0000_0000    r/w
	GTUC1 = 0x00A0,// GTU Configuration Register 1      0000_0280    r/w
	GTUC2 = 0x00A4,// GTU Configuration Register 2      0002_000A    r/w
	GTUC3 = 0x00A8,// GTU Configuration Register 3      0202_0000    r/w
	GTUC4 = 0x00AC,// GTU Configuration Register 4      0008_0007    r/w
	GTUC5 = 0x00B0,// GTU Configuration Register 5      0E00_0000    r/w
	GTUC6 = 0x00B4,// GTU Configuration Register 6      0002_0000    r/w
	GTUC7 = 0x00B8,// GTU Configuration Register 7      0002_0004    r/w
	GTUC8 = 0x00BC,// GTU Configuration Register 8      0000_0002    r/w
	GTUC9 = 0x00C0,// GTU Configuration Register 9      0000_0101    r/w
	GTUC10 = 0x00C4,// GTU Configuration Register 10     0002_0005    r/w
	GTUC11 = 0x00C8,// GTU Configuration Register 11     0000_0000    r/w

	CCSV = 0x0100,// CC Status Vector Register  0010_4000    r

	CCEV = 0x0104,// CC Error Vector Register  0000_0000    r
	//		0x0108 =-0x010C,  reserved(2)  0000_0000    r
	SCV = 0x0110,//Slot Counter Value Register  0000_0000    r
	MTCCV = 0x0114,//Macrotick and Cycle Counter Value Register  0000_0000    r
	RCV = 0x0118,//Rate Correction Value Register  0000_0000    r
	OCV = 0x011C,//Offset Correction Value Register  0000_0000    r
	SFS = 0x0120,//Sync Frame Status Register  0000_0000    r
	SWNIT = 0x0124,//Symbol Window and NIT Status Register  0000_0000    r
	ACS = 0x0128,//Aggregated Channel Status Register  0000_0000    r/w  -

	ESID_START = 0x0130,
	ESID_END = 0x0168,

	OSID_START = 0x0170,
	OSID_END = 0x01A8,

	NMV1 = 0x01B0,
	NMV2 = 0x01B4,
	NMV3 = 0x01B8,

	// Message Buffer Control Registers
	MRC = 0x0300,// Message RAM Configuration Register  0180_0000  r/w
	FRF = 0x0304,// FIFO Rejection Filter Register  0180_0000    r/w
	FRFM = 0x0308,// FIFO Rejection Filter Mask Register  0000_0000    r/w
	FCL = 0x030C,// FIFO Critical Level Register  0000_0080    r/w

	// Message Buffer Status Registers
	MHDS = 0x0310,// Message Handler Status Register  0000_0000  r/w
	LDTS = 0x0314,// Last Dynamic Transmit Slot Register  0000_0000  r
	FSR = 0x0318,// FIFO Status Register  0000_0000  r
	MHDF = 0x031C,// Message Handler Constraints Flags Register  0000_0000  r/w
	TXRQ1 = 0x0320,// Transmission Request Register 1  0000_0000  r
	TXRQ2 = 0x0324,// Transmission Request Register 2  0000_0000  r
	TXRQ3 = 0x0328,// Transmission Request Register 3  0000_0000  r
	TXRQ4 = 0x032C,// Transmission Request Register 4  0000_0000  r
	NDAT1 = 0x0330,// New Data Register 1  0000_0000  r
	NDAT2 = 0x0334,// New Data Register 2  0000_0000  r

	WRHS1 = 0x0500,// Write Header Section Register 1
	WRHS2 = 0x0504,// Write Header Section Register 2
	WRHS3 = 0x0508,// Write Header Section Register 3

	IBCM = 0x0510,// Input Buffer Command Mask Register
	IBCR = 0x0514,// Input Buffer Command Request Register

	OBCM = 0x710,
	OBCR = 0x714,
};

} // namespace icsneo

#endif // __ERAYREGISTER_H_