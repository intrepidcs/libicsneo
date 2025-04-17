#ifndef __NETWORKID_H_
#define __NETWORKID_H_

#include <stdint.h>

typedef uint16_t neonetid_t;
typedef uint8_t neonettype_t;

#ifdef __cplusplus

#include <ostream>
#include <optional>
#include <tuple>

namespace icsneo {

class Network {
	static constexpr uint16_t OFFSET_PLASMA_SLAVE1 = 100;
	static constexpr uint16_t OFFSET_PLASMA_SLAVE2 = 200;
	static constexpr uint16_t COUNT_PLASMA_SLAVE = 51;
	static constexpr uint16_t OFFSET_PLASMA_SLAVE1_RANGE2 = 4608;
	static constexpr uint16_t OFFSET_PLASMA_SLAVE2_RANGE2 = 8704;
	static constexpr uint16_t OFFSET_PLASMA_SLAVE3_RANGE2 = 12800;

public:
	enum class NetID : neonetid_t {
		Device = 0,
		DWCAN_01 = 1, // previously HSCAN
		DWCAN_08 = 2, // previously MSCAN
		SWCAN_01 = 3, // previously SWCAN
		LSFTCAN_01 = 4, // previously LSFTCAN
		FordSCP = 5,
		J1708 = 6,
		Aux = 7,
		J1850VPW = 8,
		ISO9141_01 = 9, // previously ISO9141
		DiskData = 10,
		Main51 = 11,
		RED = 12,
		SCI = 13,
		ISO9141_02 = 14, // previously ISO9141_2
		ISO14230 = 15,
		LIN_01 = 16, // previously LIN
		AE_01 = 17, // previously OP_Ethernet1
		AE_02 = 18, // previously OP_Ethernet2
		AE_03 = 19, // previously OP_Ethernet3

		// START Device Command Returns
		// When we send a command, the device returns on one of these, depending on command
		RED_EXT_MEMORYREAD = 20,
		RED_INT_MEMORYREAD = 21,
		RED_DFLASH_READ = 22,
		NeoMemorySDRead = 23, // Response from NeoMemory (MemoryTypeSD)
		CAN_ERRBITS = 24,
		NeoMemoryWriteDone = 25,
		RED_WAVE_CAN1_LOGICAL = 26,
		RED_WAVE_CAN2_LOGICAL = 27,
		RED_WAVE_LIN1_LOGICAL = 28,
		RED_WAVE_LIN2_LOGICAL = 29,
		RED_WAVE_LIN1_ANALOG = 30,
		RED_WAVE_LIN2_ANALOG = 31,
		RED_WAVE_MISC_ANALOG = 32,
		RED_WAVE_MISCDIO2_LOGICAL = 33,
		RED_NETWORK_COM_ENABLE_EX = 34,
		RED_NEOVI_NETWORK = 35,
		RED_READ_BAUD_SETTINGS = 36,
		RED_OLDFORMAT = 37,
		RED_SCOPE_CAPTURE = 38,
		RED_HARDWARE_EXCEP = 39,
		RED_GET_RTC = 40,
		// END Device Command Returns

		ISO9141_03 = 41, // previously ISO9141_3
		DWCAN_02 = 42, // previously HSCAN2
		DWCAN_03 = 44, // previously HSCAN3
		AE_04 = 45, // previously OP_Ethernet4
		AE_05 = 46, // previously OP_Ethernet5
		ISO9141_04 = 47, // previously ISO9141_4
		LIN_02 = 48, // previously LIN2
		LIN_03 = 49, // previously LIN3
		LIN_04 = 50, // previously LIN4
		// MOST = 51, Old and unused
		RED_App_Error = 52,
		CGI = 53,
		Reset_Status = 54,
		FB_Status = 55,
		App_Signal_Status = 56,
		Read_Datalink_Cm_Tx_Msg = 57,
		Read_Datalink_Cm_Rx_Msg = 58,
		Logging_Overflow = 59,
		ReadSettings = 60,
		DWCAN_04 = 61, // previously HSCAN4
		DWCAN_05 = 62, // previously HSCAN5
		RS232 = 63,
		UART_01 = 64, // previously UART
		UART_02 = 65, // previously UART2
		UART_03 = 66, // previously UART3
		UART_04 = 67, // previously UART4
		SWCAN_02 = 68, // previously SWCAN2
		ETHERNET_DAQ = 69, // previously Ethernet_DAQ
		Data_To_Host = 70,
		TextAPI_To_Host = 71,
		SPI_01 = 72, // previously SPI1
		AE_06 = 73, // previously OP_Ethernet6
		Red_VBat = 74,
		AE_07 = 75, // previously OP_Ethernet7
		AE_08 = 76, // previously OP_Ethernet8
		AE_09 = 77, // previously OP_Ethernet9
		AE_10 = 78, // previously OP_Ethernet10
		AE_11 = 79, // previously OP_Ethernet11
		FLEXRAY_01A = 80, // previously FlexRay1a
		FLEXRAY_01B = 81, // previously FlexRay1b
		FLEXRAY_02A = 82, // previously FlexRay2a
		FLEXRAY_02B = 83, // previously FlexRay2b
		LIN_05 = 84, // previously LIN5
		FLEXRAY_01 = 85, // previously FlexRay
		FLEXRAY_02 = 86, // previously FlexRay2
		AE_12 = 87, // previously OP_Ethernet12
		I2C_01 = 88, // previously I2C
		MOST_25 = 90, // previously MOST25
		MOST_50 = 91, // previously MOST50
		MOST_150 = 92, // previously MOST150
		ETHERNET_01 = 93, // previously Ethernet
		GMFSA = 94,
		TCP = 95,
		DWCAN_06 = 96, // previously HSCAN6
		DWCAN_07 = 97, // previously HSCAN7
		LIN_06 = 98, // previously LIN6
		LSFTCAN_02 = 99, // previously LSFTCAN2
		LogicalDiskInfo = 187,
		WiVICommand = 221,
		ScriptStatus = 224,
		EthPHYControl = 239,
		ExtendedCommand = 240,
		ExtendedData = 242,
		FlexRayControl = 243,
		CoreMiniPreLoad = 244,
		HW_COM_Latency_Test = 512,
		DeviceStatus = 513,
		UDP = 514,
		ForwardedMessage = 516,
		I2C_02 = 517, // previously I2C2
		I2C_03 = 518, // previously I2C3
		I2C_04 = 519, // previously I2C4
		ETHERNET_02 = 520, // previously Ethernet2
		A2B_01 = 522, // previously A2B1
		A2B_02 = 523, // previously A2B2
		ETHERNET_03 = 524, // previously Ethernet3
		WBMS_01 = 532, // previously WBMS
		DWCAN_09 = 534, // previously DWCAN9
		DWCAN_10 = 535, // previously DWCAN10
		DWCAN_11 = 536, // previously DWCAN11
		DWCAN_12 = 537, // previously DWCAN12
		DWCAN_13 = 538, // previously DWCAN13
		DWCAN_14 = 539, // previously DWCAN14
		DWCAN_15 = 540, // previously DWCAN15
		DWCAN_16 = 541, // previously DWCAN16
		LIN_07 = 542, // previously LIN7
		LIN_08 = 543, // previously LIN8
		SPI_02 = 544, // previously SPI2
		MDIO_01 = 545, // previously MDIO1
		MDIO_02 = 546, // previously MDIO2
		MDIO_03 = 547, // previously MDIO3
		MDIO_04 = 548, // previously MDIO4
		MDIO_05 = 549, // previously MDIO5
		MDIO_06 = 550, // previously MDIO6
		MDIO_07 = 551, // previously MDIO7
		MDIO_08 = 552, // previously MDIO8
		AE_13 = 553, // previously OP_Ethernet13
		AE_14 = 554, // previously OP_Ethernet14
		AE_15 = 555, // previously OP_Ethernet15
		AE_16 = 556, // previously OP_Ethernet16
		SPI_03 = 557, // previously SPI3
		SPI_04 = 558, // previously SPI4
		SPI_05 = 559, // previously SPI5
		SPI_06 = 560, // previously SPI6
		SPI_07 = 561, // previously SPI7
		SPI_08 = 562, // previously SPI8
		LIN_09 = 563, // previously LIN9
		LIN_10 = 564, // previously LIN10
		LIN_11 = 565, // previously LIN11
		LIN_12 = 566, // previously LIN12
		LIN_13 = 567, // previously LIN13
		LIN_14 = 568, // previously LIN14
		LIN_15 = 569, // previously LIN15
		LIN_16 = 570, // previously LIN16
		Any = 0xfffe, // Never actually set as type, but used as flag for filtering
		Invalid = 0xffff
	};
	enum class Type : neonettype_t {
		Invalid = 0,
		Internal = 1, // Used for statuses that don't actually need to be transferred to the client application
		CAN = 2,
		LIN = 3,
		FlexRay = 4,
		MOST = 5,
		Ethernet = 6,
		LSFTCAN = 7,
		SWCAN = 8,
		ISO9141 = 9,
		I2C = 10,
		A2B = 11,
		SPI = 12,
		MDIO = 13,
		Any = 0xFE, // Never actually set as type, but used as flag for filtering
		Other = 0xFF
	};
	enum class CoreMini : uint8_t {
		DWCAN_01 = 0,
		DWCAN_08 = 1,
		LIN_01 = 2,
		LIN_02 = 3,
		Virtual = 4,
		DWCAN_02 = 5,
		LSFTCAN_01 = 6,
		SWCAN_01 = 7,
		DWCAN_03 = 8,
		CGI = 9,
		J1850VPW = 10,
		LIN_03 = 11,
		LIN_04 = 12,
		J1708 = 13,
		DWCAN_04 = 14,
		DWCAN_05 = 15,
		ISO9141_01 = 16,
		ISO9141_02 = 17,
		ISO9141_03 = 18,
		ISO9141_04 = 19,
		FLEXRAY_01A = 20,
		UART_01 = 21,
		UART_02 = 22,
		LIN_05 = 23,
		MOST_25 = 24,
		MOST_50 = 25,
		FLEXRAY_01B = 26,
		SWCAN_02 = 27,
		ETHERNET_DAQ = 28,
		ETHERNET_01 = 29,
		FLEXRAY_02A = 30,
		FLEXRAY_02B = 31,
		DWCAN_06 = 32,
		DWCAN_07 = 33,
		LIN_06 = 34,
		LSFTCAN_02 = 35,
		AE_01 = 36,
		AE_02 = 37,
		AE_03 = 38,
		AE_04 = 39,
		AE_05 = 40,
		AE_06 = 41,
		AE_07 = 42,
		AE_08 = 43,
		AE_09 = 44,
		AE_10 = 45,
		AE_11 = 46,
		AE_12 = 47,
		TCPVirtual = 48,
		UDPVirtual = 49,
		FLEXRAY_01 = 50,
		FLEXRAY_02 = 51,
		ForwardedMessage = 52,
		I2C_01 = 53,
		I2C_02 = 54,
		I2C_03 = 55,
		I2C_04 = 56,
		ETHERNET_02 = 57,
		A2B_01 = 58,
		A2B_02 = 59,
		ETHERNET_03 = 60,
		WBMS_01 = 61,
		DWCAN_09 = 62,
		DWCAN_10 = 63,
		DWCAN_11 = 64,
		DWCAN_12 = 65,
		DWCAN_13 = 66,
		DWCAN_14 = 67,
		DWCAN_15 = 68,
		DWCAN_16 = 69,
		LIN_07 = 70,
		LIN_08 = 71,
		SPI_01 = 72,
		SPI_02 = 73,
		MDIO_01 = 75,
		MDIO_02 = 76,
		MDIO_03 = 77,
		MDIO_04 = 78,
		MDIO_05 = 79,
		MDIO_06 = 80,
		MDIO_07 = 81,
		MDIO_08 = 82,
		AE_13 = 83,
		AE_14 = 84,
		AE_15 = 85,
		AE_16 = 86,
		SPI_03 = 87,
		SPI_04 = 88,
		SPI_05 = 89,
		SPI_06 = 90,
		SPI_07 = 91,
		SPI_08 = 92,
		LIN_09 = 93,
		LIN_10 = 94,
		LIN_11 = 95,
		LIN_12 = 96,
		LIN_13 = 97,
		LIN_14 = 98,
		LIN_15 = 99,
		LIN_16 = 100,
	};
	static const char* GetTypeString(Type type) {
		switch(type) {
		case Type::CAN:
			return "CAN";
		case Type::LIN:
			return "LIN";
		case Type::FlexRay:
			return "FlexRay";
		case Type::MOST:
			return "MOST";
		case Type::Other:
			return "Other";
		case Type::Internal:
			return "Internal";
		case Type::ISO9141:
			return "ISO 9141-2";
		case Type::Ethernet:
			return "Ethernet";
		case Type::LSFTCAN:
			return "Low Speed Fault Tolerant CAN";
		case Type::SWCAN:
			return "Single Wire CAN";
		case Type::I2C:
			return "I2C";
		case Type::A2B:
			return "A2B";
		case Type::SPI:
			return "SPI";
		case Type::MDIO:
			return "MDIO";
		case Type::Invalid:
		default:
			return "Invalid Type";
		}
	}

	enum class VnetId : uint8_t {
		None = 0,
		VNET_A = 1,
		VNET_B = 2,
	};

	/**
	 * So if you are passing in the offset from OFFSET_PLASMA_SLAVE1 or
	 * the offset from OFFSET_PLASMA_SLAVE2, return the vnet agnostic
	 * netid so caller can commonize handlers without caring about WHICH slave.
	 */
	static NetID OffsetToSimpleNetworkId(uint16_t offset) {
		switch(offset) {
		default:
		case 0:
			return NetID::Device;
		case 1:
			return NetID::DWCAN_01;
		case 2:
			return NetID::DWCAN_08;
		case 3:
			return NetID::SWCAN_01;
		case 4:
			return NetID::LSFTCAN_01;
		case 5:
			return NetID::FordSCP;
		case 6:
			return NetID::J1708;
		case 7:
			return NetID::Aux;
		case 8:
			return NetID::J1850VPW;
		case 9:
			return NetID::ISO9141_01;
		case 10:
			return NetID::DiskData;
		case 11:
			return NetID::Main51;
		case 12:
			return NetID::RED;
		case 13:
			return NetID::SCI;
		case 14:
			return NetID::ISO9141_02;
		case 15:
			return NetID::ISO14230;
		case 16:
			return NetID::LIN_01;
		case 17:
			return NetID::ISO9141_03;
		case 18:
			return NetID::DWCAN_02;
		case 19:
			return NetID::DWCAN_03;
		case 20:
			return NetID::ISO9141_04;
		case 21:
			return NetID::LIN_02;
		case 22:
			return NetID::LIN_03;
		case 23:
			return NetID::LIN_04;
		//case 24:
		//	return NetID::MOST;	// Deprecated
		case 25:
			return NetID::CGI;
		case 26:
			return NetID::I2C_01;
		case 27:
			return NetID::SPI_01;
		case 28:
			return NetID::FLEXRAY_01A;
		case 29:
			return NetID::MOST_25;
		case 30:
			return NetID::MOST_50;
		case 31:
			return NetID::MOST_150;
		case 32:
			return NetID::DWCAN_04;
		case 33:
			return NetID::DWCAN_05;
		case 34:
			return NetID::RS232;
		case 35:
			return NetID::UART_01;
		case 36:
			return NetID::UART_02;
		case 37:
			return NetID::UART_03;
		case 38:
			return NetID::UART_04;
		case 39:
			return NetID::SWCAN_02;
		case 40:
			return NetID::FLEXRAY_01B;
		case 41:
			return NetID::FLEXRAY_02A;
		case 42:
			return NetID::FLEXRAY_02B;
		case 43:
			return NetID::LIN_05;
		case 44:
			return NetID::ETHERNET_01;
		case 45:
			return NetID::ETHERNET_DAQ;
		case 46:
			return NetID::RED_App_Error;
		case 47:
			return NetID::DWCAN_06;
		case 48:
			return NetID::DWCAN_07;
		case 49:
			return NetID::LIN_06;
		case 50:
			return NetID::LSFTCAN_02;
		}
	}
	static bool Within(neonetid_t value, neonetid_t min, neonetid_t max) {
		return ((min <= value) && (value < max));
	}
	static bool IdIsSlaveARange1(neonetid_t fullNetid) {
		return Within(fullNetid, OFFSET_PLASMA_SLAVE1, OFFSET_PLASMA_SLAVE1 + COUNT_PLASMA_SLAVE);
	}
	static bool IdIsSlaveARange2(neonetid_t fullNetid) {
		return Within(fullNetid, OFFSET_PLASMA_SLAVE1_RANGE2, OFFSET_PLASMA_SLAVE2_RANGE2);
	}
	static bool IdIsSlaveBRange1(neonetid_t fullNetid) {
		return Within(fullNetid, OFFSET_PLASMA_SLAVE2, OFFSET_PLASMA_SLAVE2 + COUNT_PLASMA_SLAVE);
	}
	static bool IdIsSlaveBRange2(neonetid_t fullNetid) {
		return Within(fullNetid, OFFSET_PLASMA_SLAVE2_RANGE2, OFFSET_PLASMA_SLAVE3_RANGE2);
	}
	static std::pair<VnetId, NetID> GetVnetAgnosticNetid(neonetid_t fullNetid) {
		VnetId vnetId = VnetId::None;
		NetID netId;

		if(fullNetid < OFFSET_PLASMA_SLAVE1) {
			netId = static_cast<NetID>(fullNetid);
		} else if(IdIsSlaveARange1(fullNetid)) {
			netId = OffsetToSimpleNetworkId(fullNetid - OFFSET_PLASMA_SLAVE1);
			vnetId = VnetId::VNET_A;
		} else if(IdIsSlaveARange2(fullNetid)) {
			netId = static_cast<NetID>((fullNetid - OFFSET_PLASMA_SLAVE1_RANGE2));
			vnetId = VnetId::VNET_A;
		} else if(IdIsSlaveBRange1(fullNetid)) {
			netId = OffsetToSimpleNetworkId(fullNetid - OFFSET_PLASMA_SLAVE2);
			vnetId = VnetId::VNET_B;
		} else if(IdIsSlaveBRange2(fullNetid)) {
			netId = static_cast<NetID>((fullNetid - OFFSET_PLASMA_SLAVE2_RANGE2));
			vnetId = VnetId::VNET_B;
		} else {
			netId = static_cast<NetID>(fullNetid);
		}

		return std::make_pair(vnetId, netId);
	}
	static const char* GetVnetIdString(VnetId vnetId) {
		switch(vnetId) {
		case VnetId::None:
			return "None";
		case VnetId::VNET_A:
			return "VNET A";
		case VnetId::VNET_B:
			return "VNET B";
		}
		return "Invalid VNET ID";
	}
	static Type GetTypeOfNetID(NetID netid, bool expand = true) {
		if(expand) {
			netid = GetVnetAgnosticNetid((neonetid_t)netid).second;
		}

		switch(netid) {
		case NetID::DWCAN_01:
		case NetID::DWCAN_08:
		case NetID::DWCAN_02:
		case NetID::DWCAN_03:
		case NetID::DWCAN_04:
		case NetID::DWCAN_05:
		case NetID::DWCAN_06:
		case NetID::DWCAN_07:
		case NetID::DWCAN_09:
		case NetID::DWCAN_10:
		case NetID::DWCAN_11:
		case NetID::DWCAN_12:
		case NetID::DWCAN_13:
		case NetID::DWCAN_14:
		case NetID::DWCAN_15:
		case NetID::DWCAN_16:
			return Type::CAN;
		case NetID::LIN_01:
		case NetID::LIN_02:
		case NetID::LIN_03:
		case NetID::LIN_04:
		case NetID::LIN_05:
		case NetID::LIN_06:
		case NetID::LIN_07:
		case NetID::LIN_08:
		case NetID::LIN_09:
		case NetID::LIN_10:
		case NetID::LIN_11:
		case NetID::LIN_12:
		case NetID::LIN_13:
		case NetID::LIN_14:
		case NetID::LIN_15:
		case NetID::LIN_16:
			return Type::LIN;
		case NetID::FLEXRAY_01:
		case NetID::FLEXRAY_01A:
		case NetID::FLEXRAY_01B:
		case NetID::FLEXRAY_02:
		case NetID::FLEXRAY_02A:
		case NetID::FLEXRAY_02B:
			return Type::FlexRay;
		case NetID::MOST_25:
		case NetID::MOST_50:
		case NetID::MOST_150:
			return Type::MOST;
		case NetID::RED:
		case NetID::RED_OLDFORMAT:
		case NetID::Device:
		case NetID::Reset_Status:
		case NetID::DeviceStatus:
		case NetID::FlexRayControl:
		case NetID::Main51:
		case NetID::ReadSettings:
		case NetID::LogicalDiskInfo:
		case NetID::WiVICommand:
		case NetID::ScriptStatus:
		case NetID::EthPHYControl:
		case NetID::CoreMiniPreLoad:
		case NetID::ExtendedCommand:
		case NetID::ExtendedData:
		case NetID::RED_INT_MEMORYREAD:
		case NetID::NeoMemorySDRead:
		case NetID::NeoMemoryWriteDone:
		case NetID::RED_GET_RTC:
		case NetID::DiskData:
		case NetID::RED_App_Error:
			return Type::Internal;
		case NetID::Invalid:
		case NetID::Any:
			return Type::Invalid;
		case NetID::ETHERNET_01:
		case NetID::ETHERNET_DAQ:
		case NetID::ETHERNET_02:
		case NetID::ETHERNET_03:
		case NetID::AE_01:
		case NetID::AE_02:
		case NetID::AE_03:
		case NetID::AE_04:
		case NetID::AE_05:
		case NetID::AE_06:
		case NetID::AE_07:
		case NetID::AE_08:
		case NetID::AE_09:
		case NetID::AE_10:
		case NetID::AE_11:
		case NetID::AE_12:
		case NetID::AE_13:
		case NetID::AE_14:
		case NetID::AE_15:
		case NetID::AE_16:
			return Type::Ethernet;
		case NetID::LSFTCAN_01:
		case NetID::LSFTCAN_02:
			return Type::LSFTCAN;
		case NetID::SWCAN_01:
		case NetID::SWCAN_02:
			return Type::SWCAN;
		case NetID::ISO9141_01:
		case NetID::ISO9141_02:
		case NetID::ISO9141_03:
		case NetID::ISO9141_04:
			return Type::ISO9141;
		case NetID::I2C_01:
		case NetID::I2C_02:
		case NetID::I2C_03:
		case NetID::I2C_04:
			return Type::I2C;
		case NetID::A2B_01:
		case NetID::A2B_02:
			return Type::A2B;
		case NetID::SPI_01:
		case NetID::SPI_02:
		case NetID::SPI_03:
		case NetID::SPI_04:
		case NetID::SPI_05:
		case NetID::SPI_06:
		case NetID::SPI_07:
		case NetID::SPI_08:
			return Type::SPI;
		case NetID::MDIO_01:
		case NetID::MDIO_02:
		case NetID::MDIO_03:
		case NetID::MDIO_04:
		case NetID::MDIO_05:
		case NetID::MDIO_06:
		case NetID::MDIO_07:
		case NetID::MDIO_08:
			return Type::MDIO;
		default:
			return Type::Other;
		}
	}
	static const char* GetNetIDString(NetID netid, bool expand = true) {
		if(expand) {
			netid = GetVnetAgnosticNetid((neonetid_t)netid).second;
		}
		switch(netid) {
		case NetID::Device:
			return "neoVI";
		case NetID::DWCAN_01:
			return "DW CAN 01";
		case NetID::DWCAN_08:
			return "DW CAN 08";
		case NetID::SWCAN_01:
			return "SW CAN 01";
		case NetID::LSFTCAN_01:
			return "LSFT CAN 01";
		case NetID::FordSCP:
			return "FordSCP";
		case NetID::J1708:
			return "J1708";
		case NetID::Aux:
			return "Aux";
		case NetID::J1850VPW:
			return "J1850 VPW";
		case NetID::ISO9141_01:
			return "ISO 9141 01";
		case NetID::DiskData:
			return "Disk Data";
		case NetID::Main51:
			return "Main51";
		case NetID::RED:
			return "RED";
		case NetID::SCI:
			return "SCI";
		case NetID::ISO9141_02:
			return "ISO 9141 02";
		case NetID::ISO14230:
			return "ISO 14230";
		case NetID::LIN_01:
			return "LIN 01";
		case NetID::AE_01:
			return "AE 01";
		case NetID::AE_02:
			return "AE 02";
		case NetID::AE_03:
			return "AE 03";
		case NetID::RED_EXT_MEMORYREAD:
			return "RED_EXT_MEMORYREAD";
		case NetID::RED_INT_MEMORYREAD:
			return "RED_INT_MEMORYREAD";
		case NetID::RED_DFLASH_READ:
			return "RED_DFLASH_READ";
		case NetID::NeoMemorySDRead:
			return "NeoMemorySDRead";
		case NetID::CAN_ERRBITS:
			return "CAN_ERRBITS";
		case NetID::NeoMemoryWriteDone:
			return "NeoMemoryWriteDone";
		case NetID::RED_WAVE_CAN1_LOGICAL:
			return "RED_WAVE_CAN1_LOGICAL";
		case NetID::RED_WAVE_CAN2_LOGICAL:
			return "RED_WAVE_CAN2_LOGICAL";
		case NetID::RED_WAVE_LIN1_LOGICAL:
			return "RED_WAVE_LIN1_LOGICAL";
		case NetID::RED_WAVE_LIN2_LOGICAL:
			return "RED_WAVE_LIN2_LOGICAL";
		case NetID::RED_WAVE_LIN1_ANALOG:
			return "RED_WAVE_LIN1_ANALOG";
		case NetID::RED_WAVE_LIN2_ANALOG:
			return "RED_WAVE_LIN2_ANALOG";
		case NetID::RED_WAVE_MISC_ANALOG:
			return "RED_WAVE_MISC_ANALOG";
		case NetID::RED_WAVE_MISCDIO2_LOGICAL:
			return "RED_WAVE_MISCDIO2_LOGICAL";
		case NetID::RED_NETWORK_COM_ENABLE_EX:
			return "RED_NETWORK_COM_ENABLE_EX";
		case NetID::RED_NEOVI_NETWORK:
			return "RED_NEOVI_NETWORK";
		case NetID::RED_READ_BAUD_SETTINGS:
			return "RED_READ_BAUD_SETTINGS";
		case NetID::RED_OLDFORMAT:
			return "RED_OLDFORMAT";
		case NetID::RED_SCOPE_CAPTURE:
			return "RED_SCOPE_CAPTURE";
		case NetID::RED_HARDWARE_EXCEP:
			return "RED_HARDWARE_EXCEP";
		case NetID::RED_GET_RTC:
			return "RED_GET_RTC";
		case NetID::ISO9141_03:
			return "ISO 9141 03";
		case NetID::DWCAN_02:
			return "DW CAN 02";
		case NetID::DWCAN_03:
			return "DW CAN 03";
		case NetID::AE_04:
			return "AE 04";
		case NetID::AE_05:
			return "AE 05";
		case NetID::ISO9141_04:
			return "ISO 9141 04";
		case NetID::LIN_02:
			return "LIN 02";
		case NetID::LIN_03:
			return "LIN 03";
		case NetID::LIN_04:
			return "LIN 04";
		case NetID::RED_App_Error:
			return "App Error";
		case NetID::CGI:
			return "CGI";
		case NetID::Reset_Status:
			return "Reset Status";
		case NetID::FB_Status:
			return "FB Status";
		case NetID::App_Signal_Status:
			return "App Signal Status";
		case NetID::Read_Datalink_Cm_Tx_Msg:
			return "Read Datalink Cm Tx Msg";
		case NetID::Read_Datalink_Cm_Rx_Msg:
			return "Read Datalink Cm Rx Msg";
		case NetID::Logging_Overflow:
			return "Logging Overflow";
		case NetID::ReadSettings:
			return "Read Settings";
		case NetID::DWCAN_04:
			return "DW CAN 04";
		case NetID::DWCAN_05:
			return "DW CAN 05";
		case NetID::RS232:
			return "RS232";
		case NetID::UART_01:
			return "UART 01";
		case NetID::UART_02:
			return "UART 02";
		case NetID::UART_03:
			return "UART 03";
		case NetID::UART_04:
			return "UART 04";
		case NetID::SWCAN_02:
			return "SW CAN 02";
		case NetID::ETHERNET_DAQ:
			return "Ethernet DAQ";
		case NetID::Data_To_Host:
			return "Data To Host";
		case NetID::TextAPI_To_Host:
			return "TextAPI To Host";
		case NetID::AE_06:
			return "AE 06";
		case NetID::Red_VBat:
			return "Red VBat";
		case NetID::AE_07:
			return "AE 07";
		case NetID::AE_08:
			return "AE 08";
		case NetID::AE_09:
			return "AE 09";
		case NetID::AE_10:
			return "AE 10";
		case NetID::AE_11:
			return "AE 11";
		case NetID::FLEXRAY_01A:
			return "FlexRay 01A";
		case NetID::FLEXRAY_01B:
			return "FlexRay 01B";
		case NetID::FLEXRAY_02A:
			return "FlexRay 02A";
		case NetID::FLEXRAY_02B:
			return "FlexRay 02B";
		case NetID::LIN_05:
			return "LIN 05";
		case NetID::FLEXRAY_01:
			return "FlexRay 01";
		case NetID::FLEXRAY_02:
			return "FlexRay 02";
		case NetID::AE_12:
			return "AE 12";
		case NetID::AE_13:
			return "AE 13";
		case NetID::AE_14:
			return "AE 14";
		case NetID::AE_15:
			return "AE 15";
		case NetID::AE_16:
			return "AE 16";
		case NetID::I2C_01:
			return "I2C 01";
		case NetID::MOST_25:
			return "MOST 25";
		case NetID::MOST_50:
			return "MOST 50";
		case NetID::MOST_150:
			return "MOST 150";
		case NetID::ETHERNET_01:
			return "Ethernet";
		case NetID::GMFSA:
			return "GMFSA";
		case NetID::TCP:
			return "TCP";
		case NetID::DWCAN_06:
			return "DW CAN 06";
		case NetID::DWCAN_07:
			return "DW CAN 07";
		case NetID::LIN_06:
			return "LIN 06";
		case NetID::LSFTCAN_02:
			return "LSFT CAN 02";
		case NetID::LogicalDiskInfo:
			return "Logical Disk Information";
		case NetID::WiVICommand:
			return "WiVI Command";
		case NetID::ScriptStatus:
			return "Script Status";
		case NetID::CoreMiniPreLoad:
			return "CoreMini PreLoad";
		case NetID::EthPHYControl:
			return "Ethernet PHY Register Control";
		case NetID::ExtendedCommand:
			return "Extended Command";
		case NetID::ExtendedData:
			return "Extended Data";
		case NetID::FlexRayControl:
			return "FlexRay Control";
		case NetID::HW_COM_Latency_Test:
			return "HW COM Latency Test";
		case NetID::DeviceStatus:
			return "Device Status";
		case NetID::UDP:
			return "UDP";
		case NetID::ForwardedMessage:
			return "Forwarded Message";
		case NetID::I2C_02:
			return "I2C 02";
		case NetID::I2C_03:
			return "I2C 03";
		case NetID::I2C_04:
			return "I2C 04";
		case NetID::ETHERNET_02:
			return "Ethernet 02";
		case NetID::A2B_01:
			return "A2B 01";
		case NetID::A2B_02:
			return "A2B 02";
		case NetID::DWCAN_09:
			return "DW CAN 09";
		case NetID::DWCAN_10:
			return "DW CAN 10";
		case NetID::DWCAN_11:
			return "DW CAN 11";
		case NetID::DWCAN_12:
			return "DW CAN 12";
		case NetID::DWCAN_13:
			return "DW CAN 13";
		case NetID::DWCAN_14:
			return "DW CAN 14";
		case NetID::DWCAN_15:
			return "DW CAN 15";
		case NetID::DWCAN_16:
			return "DW CAN 16";
		case NetID::ETHERNET_03:
			return "Ethernet 03";
		case NetID::LIN_07:
			return "LIN 07";
		case NetID::LIN_08:
			return "LIN 08";
		case NetID::LIN_09:
			return "LIN 09";
		case NetID::LIN_10:
			return "LIN 10";
		case NetID::LIN_11:
			return "LIN 11";
		case NetID::LIN_12:
			return "LIN 12";
		case NetID::LIN_13:
			return "LIN 13";
		case NetID::LIN_14:
			return "LIN 14";
		case NetID::LIN_15:
			return "LIN 15";
		case NetID::LIN_16:
			return "LIN 16";
		case NetID::WBMS_01:
			return "WBMS 01";
		case NetID::SPI_01:
			return "SPI 01";
		case NetID::SPI_02:
			return "SPI 02";
		case NetID::SPI_03:
			return "SPI 03";
		case NetID::SPI_04:
			return "SPI 04";
		case NetID::SPI_05:
			return "SPI 05";
		case NetID::SPI_06:
			return "SPI 06";
		case NetID::SPI_07:
			return "SPI 07";
		case NetID::SPI_08:
			return "SPI 08";
		case NetID::MDIO_01:
			return "MDIO 01";
		case NetID::MDIO_02:
			return "MDIO 02";
		case NetID::MDIO_03:
			return "MDIO 03";
		case NetID::MDIO_04:
			return "MDIO 04";
		case NetID::MDIO_05:
			return "MDIO 05";
		case NetID::MDIO_06:
			return "MDIO 06";
		case NetID::MDIO_07:
			return "MDIO 07";
		case NetID::MDIO_08:
			return "MDIO 08";
		case NetID::Any:
		case NetID::Invalid:
			break;
		}
		return "Invalid Network";
	}
	static std::optional<CoreMini> GetCoreMiniNetworkFromNetID(NetID netid) {
		switch(netid) {
		case NetID::DWCAN_01:
			return CoreMini::DWCAN_01;
		case NetID::DWCAN_08:
			return CoreMini::DWCAN_08;
		case NetID::LIN_01:
			return CoreMini::LIN_01;
		case NetID::LIN_02:
			return CoreMini::LIN_02;
		case NetID::Device:
			return CoreMini::Virtual;
		case NetID::DWCAN_02:
			return CoreMini::DWCAN_02;
		case NetID::LSFTCAN_01:
			return CoreMini::LSFTCAN_01;
		case NetID::SWCAN_01:
			return CoreMini::SWCAN_01;
		case NetID::DWCAN_03:
			return CoreMini::DWCAN_03;
		case NetID::CGI:
			return CoreMini::CGI;
		case NetID::J1850VPW:
			return CoreMini::J1850VPW;
		case NetID::LIN_03:
			return CoreMini::LIN_03;
		case NetID::LIN_04:
			return CoreMini::LIN_04;
		case NetID::J1708:
			return CoreMini::J1708;
		case NetID::DWCAN_04:
			return CoreMini::DWCAN_04;
		case NetID::DWCAN_05:
			return CoreMini::DWCAN_05;
		case NetID::ISO9141_01:
			return CoreMini::ISO9141_01;
		case NetID::ISO9141_02:
			return CoreMini::ISO9141_02;
		case NetID::ISO9141_03:
			return CoreMini::ISO9141_03;
		case NetID::ISO9141_04:
			return CoreMini::ISO9141_04;
		case NetID::FLEXRAY_01A:
			return CoreMini::FLEXRAY_01A;
		case NetID::UART_01:
			return CoreMini::UART_01;
		case NetID::UART_02:
			return CoreMini::UART_02;
		case NetID::LIN_05:
			return CoreMini::LIN_05;
		case NetID::MOST_25:
			return CoreMini::MOST_25;
		case NetID::MOST_50:
			return CoreMini::MOST_50;
		case NetID::FLEXRAY_01B:
			return CoreMini::FLEXRAY_01B;
		case NetID::SWCAN_02:
			return CoreMini::SWCAN_02;
		case NetID::ETHERNET_DAQ:
			return CoreMini::ETHERNET_DAQ;
		case NetID::ETHERNET_01:
			return CoreMini::ETHERNET_01;
		case NetID::FLEXRAY_02A:
			return CoreMini::FLEXRAY_02A;
		case NetID::FLEXRAY_02B:
			return CoreMini::FLEXRAY_02B;
		case NetID::DWCAN_06:
			return CoreMini::DWCAN_06;
		case NetID::DWCAN_07:
			return CoreMini::DWCAN_07;
		case NetID::LIN_06:
			return CoreMini::LIN_06;
		case NetID::LSFTCAN_02:
			return CoreMini::LSFTCAN_02;
		case NetID::AE_01:
			return CoreMini::AE_01;
		case NetID::AE_02:
			return CoreMini::AE_02;
		case NetID::AE_03:
			return CoreMini::AE_03;
		case NetID::AE_04:
			return CoreMini::AE_04;
		case NetID::AE_05:
			return CoreMini::AE_05;
		case NetID::AE_06:
			return CoreMini::AE_06;
		case NetID::AE_07:
			return CoreMini::AE_07;
		case NetID::AE_08:
			return CoreMini::AE_08;
		case NetID::AE_09:
			return CoreMini::AE_09;
		case NetID::AE_10:
			return CoreMini::AE_10;
		case NetID::AE_11:
			return CoreMini::AE_11;
		case NetID::AE_12:
			return CoreMini::AE_12;
		case NetID::AE_13:
			return CoreMini::AE_13;
		case NetID::AE_14:
			return CoreMini::AE_14;
		case NetID::AE_15:
			return CoreMini::AE_15;
		case NetID::AE_16:
			return CoreMini::AE_16;
		case NetID::TCP:
			return CoreMini::TCPVirtual;
		case NetID::UDP:
			return CoreMini::UDPVirtual;
		case NetID::FLEXRAY_01:
			return CoreMini::FLEXRAY_01;
		case NetID::FLEXRAY_02:
			return CoreMini::FLEXRAY_02;
		case NetID::ForwardedMessage:
			return CoreMini::ForwardedMessage;
		case NetID::I2C_01:
			return CoreMini::I2C_01;
		case NetID::I2C_02:
			return CoreMini::I2C_02;
		case NetID::I2C_03:
			return CoreMini::I2C_03;
		case NetID::I2C_04:
			return CoreMini::I2C_04;
		case NetID::ETHERNET_02:
			return CoreMini::ETHERNET_02;
		case NetID::A2B_01:
			return CoreMini::A2B_01;
		case NetID::A2B_02:
			return CoreMini::A2B_02;
		case NetID::ETHERNET_03:
			return CoreMini::ETHERNET_03;
		case NetID::WBMS_01:
			return CoreMini::WBMS_01;
		case NetID::DWCAN_09:
			return CoreMini::DWCAN_09;
		case NetID::DWCAN_10:
			return CoreMini::DWCAN_10;
		case NetID::DWCAN_11:
			return CoreMini::DWCAN_11;
		case NetID::DWCAN_12:
			return CoreMini::DWCAN_12;
		case NetID::DWCAN_13:
			return CoreMini::DWCAN_13;
		case NetID::DWCAN_14:
			return CoreMini::DWCAN_14;
		case NetID::DWCAN_15:
			return CoreMini::DWCAN_15;
		case NetID::DWCAN_16:
			return CoreMini::DWCAN_16;
		case NetID::LIN_07:
			return CoreMini::LIN_07;
		case NetID::LIN_08:
			return CoreMini::LIN_08;
		case NetID::LIN_09:
			return CoreMini::LIN_09;
		case NetID::LIN_10:
			return CoreMini::LIN_10;
		case NetID::LIN_11:
			return CoreMini::LIN_11;
		case NetID::LIN_12:
			return CoreMini::LIN_12;
		case NetID::LIN_13:
			return CoreMini::LIN_13;
		case NetID::LIN_14:
			return CoreMini::LIN_14;
		case NetID::LIN_15:
			return CoreMini::LIN_15;
		case NetID::LIN_16:
			return CoreMini::LIN_16;
		case NetID::SPI_01:
			return CoreMini::SPI_01;
		case NetID::SPI_02:
			return CoreMini::SPI_02;
		case NetID::SPI_03:
			return CoreMini::SPI_03;
		case NetID::SPI_04:
			return CoreMini::SPI_04;
		case NetID::SPI_05:
			return CoreMini::SPI_05;
		case NetID::SPI_06:
			return CoreMini::SPI_06;
		case NetID::SPI_07:
			return CoreMini::SPI_07;
		case NetID::SPI_08:
			return CoreMini::SPI_08;
		case NetID::MDIO_01:
			return CoreMini::MDIO_01;
		case NetID::MDIO_02:
			return CoreMini::MDIO_02;
		case NetID::MDIO_03:
			return CoreMini::MDIO_03;
		case NetID::MDIO_04:
			return CoreMini::MDIO_04;
		case NetID::MDIO_05:
			return CoreMini::MDIO_05;
		case NetID::MDIO_06:
			return CoreMini::MDIO_06;
		case NetID::MDIO_07:
			return CoreMini::MDIO_07;
		case NetID::MDIO_08:
			return CoreMini::MDIO_08;
		default:
			return std::nullopt;
		}
	}
	static NetID GetNetIDFromCoreMiniNetwork(CoreMini cm) {
		switch(cm) {
		case CoreMini::DWCAN_01:
			return NetID::DWCAN_01;
		case CoreMini::DWCAN_08:
			return NetID::DWCAN_08;
		case CoreMini::LIN_01:
			return NetID::LIN_01;
		case CoreMini::LIN_02:
			return NetID::LIN_02;
		case CoreMini::Virtual:
			return NetID::Device;
		case CoreMini::DWCAN_02:
			return NetID::DWCAN_02;
		case CoreMini::LSFTCAN_01:
			return NetID::LSFTCAN_01;
		case CoreMini::SWCAN_01:
			return NetID::SWCAN_01;
		case CoreMini::DWCAN_03:
			return NetID::DWCAN_03;
		case CoreMini::CGI:
			return NetID::CGI;
		case CoreMini::J1850VPW:
			return NetID::J1850VPW;
		case CoreMini::LIN_03:
			return NetID::LIN_03;
		case CoreMini::LIN_04:
			return NetID::LIN_04;
		case CoreMini::J1708:
			return NetID::J1708;
		case CoreMini::DWCAN_04:
			return NetID::DWCAN_04;
		case CoreMini::DWCAN_05:
			return NetID::DWCAN_05;
		case CoreMini::ISO9141_01:
			return NetID::ISO9141_01;
		case CoreMini::ISO9141_02:
			return NetID::ISO9141_02;
		case CoreMini::ISO9141_03:
			return NetID::ISO9141_03;
		case CoreMini::ISO9141_04:
			return NetID::ISO9141_04;
		case CoreMini::FLEXRAY_01A:
			return NetID::FLEXRAY_01A;
		case CoreMini::UART_01:
			return NetID::UART_01;
		case CoreMini::UART_02:
			return NetID::UART_02;
		case CoreMini::LIN_05:
			return NetID::LIN_05;
		case CoreMini::MOST_25:
			return NetID::MOST_25;
		case CoreMini::MOST_50:
			return NetID::MOST_50;
		case CoreMini::FLEXRAY_01B:
			return NetID::FLEXRAY_01B;
		case CoreMini::SWCAN_02:
			return NetID::SWCAN_02;
		case CoreMini::ETHERNET_DAQ:
			return NetID::ETHERNET_DAQ;
		case CoreMini::ETHERNET_01:
			return NetID::ETHERNET_01;
		case CoreMini::FLEXRAY_02A:
			return NetID::FLEXRAY_02A;
		case CoreMini::FLEXRAY_02B:
			return NetID::FLEXRAY_02B;
		case CoreMini::DWCAN_06:
			return NetID::DWCAN_06;
		case CoreMini::DWCAN_07:
			return NetID::DWCAN_07;
		case CoreMini::LIN_06:
			return NetID::LIN_06;
		case CoreMini::LSFTCAN_02:
			return NetID::LSFTCAN_02;
		case CoreMini::AE_01:
			return NetID::AE_01;
		case CoreMini::AE_02:
			return NetID::AE_02;
		case CoreMini::AE_03:
			return NetID::AE_03;
		case CoreMini::AE_04:
			return NetID::AE_04;
		case CoreMini::AE_05:
			return NetID::AE_05;
		case CoreMini::AE_06:
			return NetID::AE_06;
		case CoreMini::AE_07:
			return NetID::AE_07;
		case CoreMini::AE_08:
			return NetID::AE_08;
		case CoreMini::AE_09:
			return NetID::AE_09;
		case CoreMini::AE_10:
			return NetID::AE_10;
		case CoreMini::AE_11:
			return NetID::AE_11;
		case CoreMini::AE_12:
			return NetID::AE_12;
		case CoreMini::AE_13:
			return NetID::AE_13;
		case CoreMini::AE_14:
			return NetID::AE_14;
		case CoreMini::AE_15:
			return NetID::AE_15;
		case CoreMini::AE_16:
			return NetID::AE_16;
		case CoreMini::TCPVirtual:
			return NetID::TCP;
		case CoreMini::UDPVirtual:
			return NetID::UDP;
		case CoreMini::FLEXRAY_01:
			return NetID::FLEXRAY_01;
		case CoreMini::FLEXRAY_02:
			return NetID::FLEXRAY_02;
		case CoreMini::ForwardedMessage:
			return NetID::ForwardedMessage;
		case CoreMini::I2C_01:
			return NetID::I2C_01;
		case CoreMini::I2C_02:
			return NetID::I2C_02;
		case CoreMini::I2C_03:
			return NetID::I2C_03;
		case CoreMini::I2C_04:
			return NetID::I2C_04;
		case CoreMini::ETHERNET_02:
			return NetID::ETHERNET_02;
		case CoreMini::A2B_01:
			return NetID::A2B_01;
		case CoreMini::A2B_02:
			return NetID::A2B_02;
		case CoreMini::ETHERNET_03:
			return NetID::ETHERNET_03;
		case CoreMini::WBMS_01:
			return NetID::WBMS_01;
		case CoreMini::DWCAN_09:
			return NetID::DWCAN_09;
		case CoreMini::DWCAN_10:
			return NetID::DWCAN_10;
		case CoreMini::DWCAN_11:
			return NetID::DWCAN_11;
		case CoreMini::DWCAN_12:
			return NetID::DWCAN_12;
		case CoreMini::DWCAN_13:
			return NetID::DWCAN_13;
		case CoreMini::DWCAN_14:
			return NetID::DWCAN_14;
		case CoreMini::DWCAN_15:
			return NetID::DWCAN_15;
		case CoreMini::DWCAN_16:
			return NetID::DWCAN_16;
		case CoreMini::LIN_07:
			return NetID::LIN_07;
		case CoreMini::LIN_08:
			return NetID::LIN_08;
		case CoreMini::LIN_09:
			return NetID::LIN_09;
		case CoreMini::LIN_10:
			return NetID::LIN_10;
		case CoreMini::LIN_11:
			return NetID::LIN_11;
		case CoreMini::LIN_12:
			return NetID::LIN_12;
		case CoreMini::LIN_13:
			return NetID::LIN_13;
		case CoreMini::LIN_14:
			return NetID::LIN_14;
		case CoreMini::LIN_15:
			return NetID::LIN_15;
		case CoreMini::LIN_16:
			return NetID::LIN_16;
		case CoreMini::SPI_01:
			return NetID::SPI_01;
		case CoreMini::SPI_02:
			return NetID::SPI_02;
		case CoreMini::SPI_03:
			return NetID::SPI_03;
		case CoreMini::SPI_04:
			return NetID::SPI_04;
		case CoreMini::SPI_05:
			return NetID::SPI_05;
		case CoreMini::SPI_06:
			return NetID::SPI_06;
		case CoreMini::SPI_07:
			return NetID::SPI_07;
		case CoreMini::SPI_08:
			return NetID::SPI_08;
		case CoreMini::MDIO_01:
			return NetID::MDIO_01;
		case CoreMini::MDIO_02:
			return NetID::MDIO_02;
		case CoreMini::MDIO_03:
			return NetID::MDIO_03;
		case CoreMini::MDIO_04:
			return NetID::MDIO_04;
		case CoreMini::MDIO_05:
			return NetID::MDIO_05;
		case CoreMini::MDIO_06:
			return NetID::MDIO_06;
		case CoreMini::MDIO_07:
			return NetID::MDIO_07;
		case CoreMini::MDIO_08:
			return NetID::MDIO_08;

		}
		return NetID::Invalid; // Should be unreachable, the compiler should warn about new CoreMini IDs
	}

	Network() { setValue(NetID::Invalid); }
	Network(neonetid_t netid, bool expand = true) { setValue(static_cast<NetID>(netid), expand); }
	Network(NetID netid) { setValue(netid); }
	Network(CoreMini cm) { setValue(GetNetIDFromCoreMiniNetwork(cm)); }
	NetID getNetID() const { return value; }
	Type getType() const { return type; }
	VnetId getVnetId() const { return vnetId; }
	std::optional<CoreMini> getCoreMini() const { return GetCoreMiniNetworkFromNetID(getNetID()); }
	friend std::ostream& operator<<(std::ostream& os, const Network& network) {
		os << GetNetIDString(network.getNetID());
		return os;
	}
	friend bool operator==(const Network& net1, const Network& net2) { return net1.getNetID() == net2.getNetID(); }
	friend bool operator!=(const Network& net1, const Network& net2) { return !(net1 == net2); }

private:
	NetID value; // Always use setValue so that value and type stay in sync
	Type type;
	NetID commonNetId;
	VnetId vnetId;
	void setValue(NetID id, bool expand = true) {
		value = id;
		// Constructor can optionally suppress the expansion of of the ID into a VNET slot and common ID. The communication decoder and packetizer require this for device responses.
		type = GetTypeOfNetID(value, expand);
		if(expand) {
			std::tie(vnetId, commonNetId) = GetVnetAgnosticNetid((neonetid_t)id);
		} else {
			// Caller wanted to suppress VNET ID decoding.
			vnetId = VnetId::None;
			commonNetId = id;
		}
	}
};

}

#endif // __cplusplus

#ifdef __ICSNEOC_H_
#define ICSNEO_NETID_DEVICE 0
#define ICSNEO_NETID_DWCAN_01 1
#define ICSNEO_NETID_DWCAN_08 2
#define ICSNEO_NETID_SWCAN_01 3
#define ICSNEO_NETID_LSFTCAN_01 4
#define ICSNEO_NETID_FORDSCP 5
#define ICSNEO_NETID_J1708 6
#define ICSNEO_NETID_AUX 7
#define ICSNEO_NETID_J1850VPW 8
#define ICSNEO_NETID_ISO9141_01 9
#define ICSNEO_NETID_DISK_DATA 10
#define ICSNEO_NETID_MAIN51 11
#define ICSNEO_NETID_RED 12
#define ICSNEO_NETID_SCI 13
#define ICSNEO_NETID_ISO9141_02 14
#define ICSNEO_NETID_ISO14230 15
#define ICSNEO_NETID_LIN_01 16
#define ICSNEO_NETID_AE_01 17
#define ICSNEO_NETID_AE_02 18
#define ICSNEO_NETID_AE_03 19

// START Device Command Returns
// When we send a command, the device returns on one of these, depending on command
#define ICSNEO_NETID_RED_EXT_MEMORYREAD 20
#define ICSNEO_NETID_RED_INT_MEMORYREAD 21
#define ICSNEO_NETID_RED_DFLASH_READ 22
#define ICSNEO_NETID_NEOMEMORY_SD_READ 23
#define ICSNEO_NETID_CAN_ERRBITS 24
#define ICSNEO_NETID_NEOMEMORY_WRITE_DONE 25
#define ICSNEO_NETID_RED_WAVE_CAN1_LOGICAL 26
#define ICSNEO_NETID_RED_WAVE_CAN2_LOGICAL 27
#define ICSNEO_NETID_RED_WAVE_LIN1_LOGICAL 28
#define ICSNEO_NETID_RED_WAVE_LIN2_LOGICAL 29
#define ICSNEO_NETID_RED_WAVE_LIN1_ANALOG 30
#define ICSNEO_NETID_RED_WAVE_LIN2_ANALOG 31
#define ICSNEO_NETID_RED_WAVE_MISC_ANALOG 32
#define ICSNEO_NETID_RED_WAVE_MISCDIO2_LOGICAL 33
#define ICSNEO_NETID_RED_NETWORK_COM_ENABLE_EX 34
#define ICSNEO_NETID_RED_NEOVI_NETWORK 35
#define ICSNEO_NETID_RED_READ_BAUD_SETTINGS 36
#define ICSNEO_NETID_RED_OLDFORMAT 37
#define ICSNEO_NETID_RED_SCOPE_CAPTURE 38
#define ICSNEO_NETID_RED_HARDWARE_EXCEP 39
#define ICSNEO_NETID_RED_GET_RTC 40
// END Device Command Returns

#define ICSNEO_NETID_ISO9141_03 41
#define ICSNEO_NETID_DWCAN_02 42
#define ICSNEO_NETID_DWCAN_03 44
#define ICSNEO_NETID_AE_04 45
#define ICSNEO_NETID_AE_05 46
#define ICSNEO_NETID_ISO9141_04 47
#define ICSNEO_NETID_LIN_02 48
#define ICSNEO_NETID_LIN_03 49
#define ICSNEO_NETID_LIN_04 50
//#define ICSNEO_NETID_MOST 51 Old and unused
#define ICSNEO_NETID_RED_APP_ERROR 52
#define ICSNEO_NETID_CGI 53
#define ICSNEO_NETID_RESET_STATUS 54
#define ICSNEO_NETID_FB_STATUS 55
#define ICSNEO_NETID_APP_SIGNAL_STATUS 56
#define ICSNEO_NETID_READ_DATALINK_CM_TX_MSG 57
#define ICSNEO_NETID_READ_DATALINK_CM_RX_MSG 58
#define ICSNEO_NETID_LOGGING_OVERFLOW 59
#define ICSNEO_NETID_READ_SETTINGS 60
#define ICSNEO_NETID_DWCAN_04 61
#define ICSNEO_NETID_DWCAN_05 62
#define ICSNEO_NETID_RS232 63
#define ICSNEO_NETID_UART_01 64
#define ICSNEO_NETID_UART_02 65
#define ICSNEO_NETID_UART_03 66
#define ICSNEO_NETID_UART_04 67
#define ICSNEO_NETID_SWCAN_02 68
#define ICSNEO_NETID_ETHERNET_DAQ 69
#define ICSNEO_NETID_DATA_TO_HOST 70
#define ICSNEO_NETID_TEXTAPI_TO_HOST 71
#define ICSNEO_NETID_AE_06 73
#define ICSNEO_NETID_RED_VBAT 74
#define ICSNEO_NETID_AE_07 75
#define ICSNEO_NETID_AE_08 76
#define ICSNEO_NETID_AE_09 77
#define ICSNEO_NETID_AE_10 78
#define ICSNEO_NETID_AE_11 79
#define ICSNEO_NETID_FLEXRAY_01A 80
#define ICSNEO_NETID_FLEXRAY_01B 81
#define ICSNEO_NETID_FLEXRAY_02A 82
#define ICSNEO_NETID_FLEXRAY_02B 83
#define ICSNEO_NETID_LIN_05 84
#define ICSNEO_NETID_FLEXRAY 85
#define ICSNEO_NETID_FLEXRAY_02 86
#define ICSNEO_NETID_AE_12 87
#define ICSNEO_NETID_I2C_01 88
#define ICSNEO_NETID_MOST_25 90
#define ICSNEO_NETID_MOST_50 91
#define ICSNEO_NETID_MOST_150 92
#define ICSNEO_NETID_ETHERNET_01 93
#define ICSNEO_NETID_GMFSA 94
#define ICSNEO_NETID_TCP 95
#define ICSNEO_NETID_DWCAN_06 96
#define ICSNEO_NETID_DWCAN_07 97
#define ICSNEO_NETID_LIN_06 98
#define ICSNEO_NETID_LSFTCAN_02 99
#define ICSNEO_NETID_LOGICAL_DISK_INFO 187
#define ICSNEO_NETID_WIVI_COMMAND 221
#define ICSNEO_NETID_SCRIPT_STATUS 224
#define ICSNEO_NETID_ETH_PHY_CONTROL 239
#define ICSNEO_NETID_EXTENDED_COMMAND 240
#define ICSNEO_NETID_FLEXRAY_CONTROL 243
#define ICSNEO_NETID_COREMINI_PRELOAD 244
#define ICSNEO_NETID_HW_COM_LATENCY_TEST 512
#define ICSNEO_NETID_DEVICE_STATUS 513
#define ICSNEO_NETID_UDP 514
#define ICSNEO_NETID_FORWARDED_MESSAGE 516
#define ICSNEO_NETID_I2C_02 517
#define ICSNEO_NETID_I2C_03 518
#define ICSNEO_NETID_I2C_04 519
#define ICSNEO_NETID_ETHERNET_02 520
#define ICSNEO_NETID_ANY 0xfffe // Never actually set as type, but used as flag for filtering
#define ICSNEO_NETID_INVALID 0xffff

#define ICSNEO_NETWORK_TYPE_INVALID 0
#define ICSNEO_NETWORK_TYPE_INTERNAL 1 // Used for statuses that don't actually need to be transferred to the client application
#define ICSNEO_NETWORK_TYPE_CAN 2
#define ICSNEO_NETWORK_TYPE_LIN 3
#define ICSNEO_NETWORK_TYPE_FLEXRAY 4
#define ICSNEO_NETWORK_TYPE_MOST 5
#define ICSNEO_NETWORK_TYPE_ETHERNET 6
#define ICSNEO_NETWORK_TYPE_LSFTCAN 7
#define ICSNEO_NETWORK_TYPE_SWCAN 8
#define ICSNEO_NETWORK_TYPE_ISO9141 9
#define ICSNEO_NETWORK_TYPE_I2C 10
#define ICSNEO_NETWORK_TYPE_ANY 0xFE // Never actually set as type, but used as flag for filtering
#define ICSNEO_NETWORK_TYPE_OTHER 0xFF
#endif

#endif
