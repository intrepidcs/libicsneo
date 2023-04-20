#ifndef __NETWORKID_H_
#define __NETWORKID_H_

#include <stdint.h>
typedef uint16_t neonetid_t;
typedef uint8_t neonettype_t;

#ifdef __cplusplus

#include <ostream>
#include <optional>

namespace icsneo {

class Network {
public:
	enum class NetID : neonetid_t {
		Device = 0,
		HSCAN = 1,
		MSCAN = 2,
		SWCAN = 3,
		LSFTCAN = 4,
		FordSCP = 5,
		J1708 = 6,
		Aux = 7,
		J1850VPW = 8,
		ISO9141 = 9,
		ISOPIC = 10,
		Main51 = 11,
		RED = 12,
		SCI = 13,
		ISO9141_2 = 14,
		ISO14230 = 15,
		LIN = 16,
		OP_Ethernet1 = 17,
		OP_Ethernet2 = 18,
		OP_Ethernet3 = 19,

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

		ISO9141_3 = 41,
		HSCAN2 = 42,
		HSCAN3 = 44,
		OP_Ethernet4 = 45,
		OP_Ethernet5 = 46,
		ISO9141_4 = 47,
		LIN2 = 48,
		LIN3 = 49,
		LIN4 = 50,
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
		HSCAN4 = 61,
		HSCAN5 = 62,
		RS232 = 63,
		UART = 64,
		UART2 = 65,
		UART3 = 66,
		UART4 = 67,
		SWCAN2 = 68,
		Ethernet_DAQ = 69,
		Data_To_Host = 70,
		TextAPI_To_Host = 71,
		OP_Ethernet6 = 73,
		Red_VBat = 74,
		OP_Ethernet7 = 75,
		OP_Ethernet8 = 76,
		OP_Ethernet9 = 77,
		OP_Ethernet10 = 78,
		OP_Ethernet11 = 79,
		FlexRay1a = 80,
		FlexRay1b = 81,
		FlexRay2a = 82,
		FlexRay2b = 83,
		LIN5 = 84,
		FlexRay = 85,
		FlexRay2 = 86,
		OP_Ethernet12 = 87,
		I2C = 88,
		MOST25 = 90,
		MOST50 = 91,
		MOST150 = 92,
		Ethernet = 93,
		GMFSA = 94,
		TCP = 95,
		HSCAN6 = 96,
		HSCAN7 = 97,
		LIN6 = 98,
		LSFTCAN2 = 99,
		LogicalDiskInfo = 187,
		WiVICommand = 221,
		ScriptStatus = 224,
		EthPHYControl = 239,
		ExtendedCommand = 240,
		FlexRayControl = 243,
		CoreMiniPreLoad = 244,
		HW_COM_Latency_Test = 512,
		DeviceStatus = 513,
		UDP = 514,
		ForwardedMessage = 516,
		I2C2 = 517,
		I2C3 = 518,
		I2C4 = 519,
		Ethernet2 = 520,
		A2B1 = 522,
		A2B2 = 523,
		Ethernet3 = 524,
		WBMS = 532,
		DWCAN9 = 534,
		DWCAN10 = 535,
		DWCAN11 = 536,
		DWCAN12 = 537,
		DWCAN13 = 538,
		DWCAN14 = 539,
		DWCAN15 = 540,
		DWCAN16 = 541,
		LIN7 = 542,
		LIN8 = 543,
		RED_SET_RTC = 544,
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
		Any = 0xFE, // Never actually set as type, but used as flag for filtering
		Other = 0xFF
	};
	enum class CoreMini : uint8_t {
		HSCAN = 0,
		MSCAN = 1,
		LIN = 2,
		LIN2 = 3,
		Virtual = 4,
		HSCAN2 = 5,
		LSFTCAN = 6,
		SWCAN = 7,
		HSCAN3 = 8,
		CGI = 9,
		J1850VPW = 10,
		LIN3 = 11,
		LIN4 = 12,
		J1708 = 13,
		HSCAN4 = 14,
		HSCAN5 = 15,
		KLine1 = 16,
		KLine2 = 17,
		KLine3 = 18,
		KLine4 = 19,
		FlexRay1a = 20,
		UART = 21,
		UART2 = 22,
		LIN5 = 23,
		MOST25 = 24,
		MOST50 = 25,
		FlexRay1b = 26,
		SWCAN2 = 27,
		EthernetDAQ = 28,
		Ethernet = 29,
		FlexRay2a = 30,
		FlexRay2b = 31,
		HSCAN6 = 32,
		HSCAN7 = 33,
		LIN6 = 34,
		LSFTCAN2 = 35,
		OP_Ethernet1 = 36,
		OP_Ethernet2 = 37,
		OP_Ethernet3 = 38,
		OP_Ethernet4 = 39,
		OP_Ethernet5 = 40,
		OP_Ethernet6 = 41,
		OP_Ethernet7 = 42,
		OP_Ethernet8 = 43,
		OP_Ethernet9 = 44,
		OP_Ethernet10 = 45,
		OP_Ethernet11 = 46,
		OP_Ethernet12 = 47,
		TCPVirtual = 48,
		UDPVirtual = 49,
		FlexRay1 = 50,
		FlexRay2 = 51,
		ForwardedMessage = 52,
		I2C1 = 53,
		I2C2 = 54,
		I2C3 = 55,
		I2C4 = 56,
		Ethernet2 = 57,
		A2B1 = 58,
		A2B2 = 59,
		Ethernet3 = 60,
		WBMS = 61,
		DWCAN9 = 62,
		DWCAN10 = 63,
		DWCAN11 = 64,
		DWCAN12 = 65,
		DWCAN13 = 66,
		DWCAN14 = 67,
		DWCAN15 = 68,
		DWCAN16 = 69,
		LIN7 = 70,
		LIN8 = 71,
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
				return "I²C";
			case Type::A2B:
				return "A2B";
			case Type::Invalid:
			default:
				return "Invalid Type";
		}
	}
	static Type GetTypeOfNetID(NetID netid) {
		switch(netid) {
			case NetID::HSCAN:
			case NetID::MSCAN:
			case NetID::HSCAN2:
			case NetID::HSCAN3:
			case NetID::HSCAN4:
			case NetID::HSCAN5:
			case NetID::HSCAN6:
			case NetID::HSCAN7:
			case NetID::DWCAN9:
			case NetID::DWCAN10:
			case NetID::DWCAN11:
			case NetID::DWCAN12:
			case NetID::DWCAN13:
			case NetID::DWCAN14:
			case NetID::DWCAN15:
			case NetID::DWCAN16:
				return Type::CAN;
			case NetID::LIN:
			case NetID::LIN2:
			case NetID::LIN3:
			case NetID::LIN4:
			case NetID::LIN5:
			case NetID::LIN6:
			case NetID::LIN7:
			case NetID::LIN8:
				return Type::LIN;
			case NetID::FlexRay:
			case NetID::FlexRay1a:
			case NetID::FlexRay1b:
			case NetID::FlexRay2:
			case NetID::FlexRay2a:
			case NetID::FlexRay2b:
				return Type::FlexRay;
			case NetID::MOST25:
			case NetID::MOST50:
			case NetID::MOST150:
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
			case NetID::NeoMemorySDRead:
			case NetID::NeoMemoryWriteDone:
			case NetID::RED_GET_RTC:
			case NetID::RED_SET_RTC:
				return Type::Internal;
			case NetID::Invalid:
			case NetID::Any:
				return Type::Invalid;
			case NetID::Ethernet:
			case NetID::Ethernet_DAQ:
			case NetID::Ethernet2:
			case NetID::Ethernet3:
			case NetID::OP_Ethernet1:
			case NetID::OP_Ethernet2:
			case NetID::OP_Ethernet3:
			case NetID::OP_Ethernet4:
			case NetID::OP_Ethernet5:
			case NetID::OP_Ethernet6:
			case NetID::OP_Ethernet7:
			case NetID::OP_Ethernet8:
			case NetID::OP_Ethernet9:
			case NetID::OP_Ethernet10:
			case NetID::OP_Ethernet11:
			case NetID::OP_Ethernet12:
				return Type::Ethernet;
			case NetID::LSFTCAN:
			case NetID::LSFTCAN2:
				return Type::LSFTCAN;
			case NetID::SWCAN:
			case NetID::SWCAN2:
				return Type::SWCAN;
			case NetID::ISO9141:
			case NetID::ISO9141_2:
			case NetID::ISO9141_3:
			case NetID::ISO9141_4:
				return Type::ISO9141;
			case NetID::I2C:
			case NetID::I2C2:
			case NetID::I2C3:
			case NetID::I2C4:
				return Type::I2C;
			case NetID::A2B1:
			case NetID::A2B2:
				return Type::A2B;
			default:
				return Type::Other;
		}
	}
	static const char* GetNetIDString(NetID netid) {
		switch(netid) {
			case NetID::Device:
				return "neoVI";
			case NetID::HSCAN:
				return "HSCAN";
			case NetID::MSCAN:
				return "MSCAN";
			case NetID::SWCAN:
				return "SWCAN";
			case NetID::LSFTCAN:
				return "LSFTCAN";
			case NetID::FordSCP:
				return "FordSCP";
			case NetID::J1708:
				return "J1708";
			case NetID::Aux:
				return "Aux";
			case NetID::J1850VPW:
				return "J1850 VPW";
			case NetID::ISO9141:
				return "ISO 9141";
			case NetID::ISOPIC:
				return "ISOPIC";
			case NetID::Main51:
				return "Main51";
			case NetID::RED:
				return "RED";
			case NetID::SCI:
				return "SCI";
			case NetID::ISO9141_2:
				return "ISO 9141 2";
			case NetID::ISO14230:
				return "ISO 14230";
			case NetID::LIN:
				return "LIN";
			case NetID::OP_Ethernet1:
				return "OP (BR) Ethernet 1";
			case NetID::OP_Ethernet2:
				return "OP (BR) Ethernet 2";
			case NetID::OP_Ethernet3:
				return "OP (BR) Ethernet 3";
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
			case NetID::RED_SET_RTC:
				return "RED_SET_RTC";
			case NetID::ISO9141_3:
				return "ISO 9141 3";
			case NetID::HSCAN2:
				return "HSCAN 2";
			case NetID::HSCAN3:
				return "HSCAN 3";
			case NetID::OP_Ethernet4:
				return "OP (BR) Ethernet 4";
			case NetID::OP_Ethernet5:
				return "OP (BR) Ethernet 5";
			case NetID::ISO9141_4:
				return "ISO 9141 4";
			case NetID::LIN2:
				return "LIN 2";
			case NetID::LIN3:
				return "LIN 3";
			case NetID::LIN4:
				return "LIN 4";
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
			case NetID::HSCAN4:
				return "HSCAN 4";
			case NetID::HSCAN5:
				return "HSCAN 5";
			case NetID::RS232:
				return "RS232";
			case NetID::UART:
				return "UART";
			case NetID::UART2:
				return "UART 2";
			case NetID::UART3:
				return "UART 3";
			case NetID::UART4:
				return "UART 4";
			case NetID::SWCAN2:
				return "SWCAN 2";
			case NetID::Ethernet_DAQ:
				return "Ethernet DAQ";
			case NetID::Data_To_Host:
				return "Data To Host";
			case NetID::TextAPI_To_Host:
				return "TextAPI To Host";
			case NetID::OP_Ethernet6:
				return "OP (BR) Ethernet 6";
			case NetID::Red_VBat:
				return "Red VBat";
			case NetID::OP_Ethernet7:
				return "OP (BR) Ethernet 7";
			case NetID::OP_Ethernet8:
				return "OP (BR) Ethernet 8";
			case NetID::OP_Ethernet9:
				return "OP (BR) Ethernet 9";
			case NetID::OP_Ethernet10:
				return "OP (BR) Ethernet 10";
			case NetID::OP_Ethernet11:
				return "OP (BR) Ethernet 11";
			case NetID::FlexRay1a:
				return "FlexRay 1a";
			case NetID::FlexRay1b:
				return "FlexRay 1b";
			case NetID::FlexRay2a:
				return "FlexRay 2a";
			case NetID::FlexRay2b:
				return "FlexRay 2b";
			case NetID::LIN5:
				return "LIN 5";
			case NetID::FlexRay:
				return "FlexRay";
			case NetID::FlexRay2:
				return "FlexRay 2";
			case NetID::OP_Ethernet12:
				return "OP (BR) Ethernet 12";
			case NetID::I2C:
				return "I2C";
			case NetID::MOST25:
				return "MOST25";
			case NetID::MOST50:
				return "MOST50";
			case NetID::MOST150:
				return "MOST150";
			case NetID::Ethernet:
				return "Ethernet";
			case NetID::GMFSA:
				return "GMFSA";
			case NetID::TCP:
				return "TCP";
			case NetID::HSCAN6:
				return "HSCAN 6";
			case NetID::HSCAN7:
				return "HSCAN 7";
			case NetID::LIN6:
				return "LIN 6";
			case NetID::LSFTCAN2:
				return "LSFTCAN 2";
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
			case NetID::I2C2:
				return "I2C 2";
			case NetID::I2C3:
				return "I2C 3";
			case NetID::I2C4:
				return "I2C 4";
			case NetID::Ethernet2:
				return "Ethernet 2";
			case NetID::A2B1:
				return "A2B 1";
			case NetID::A2B2:
				return "A2B 2";
			case NetID::DWCAN9:
				return "DWCAN 09";
			case NetID::DWCAN10:
				return "DWCAN 10";
			case NetID::DWCAN11:
				return "DWCAN 11";
			case NetID::DWCAN12:
				return "DWCAN 12";
			case NetID::DWCAN13:
				return "DWCAN 13";
			case NetID::DWCAN14:
				return "DWCAN 14";
			case NetID::DWCAN15:
				return "DWCAN 15";
			case NetID::DWCAN16:
				return "DWCAN 16";
			case NetID::Ethernet3:
				return "Ethernet 03";
			case NetID::LIN7:
				return "LIN 07";
			case NetID::LIN8:
				return "LIN 08";
			case NetID::WBMS:
				return "WBMS";
			case NetID::Any:
			case NetID::Invalid:
				break;
		}
		return "Invalid Network";
	}
	static std::optional<CoreMini> GetCoreMiniNetworkFromNetID(NetID netid) {
		switch(netid) {
			case NetID::HSCAN:
				return CoreMini::HSCAN;
			case NetID::MSCAN:
				return CoreMini::MSCAN;
			case NetID::LIN:
				return CoreMini::LIN;
			case NetID::LIN2:
				return CoreMini::LIN2;
			case NetID::Device:
				return CoreMini::Virtual;
			case NetID::HSCAN2:
				return CoreMini::HSCAN2;
			case NetID::LSFTCAN:
				return CoreMini::LSFTCAN;
			case NetID::SWCAN:
				return CoreMini::SWCAN;
			case NetID::HSCAN3:
				return CoreMini::HSCAN3;
			case NetID::CGI:
				return CoreMini::CGI;
			case NetID::J1850VPW:
				return CoreMini::J1850VPW;
			case NetID::LIN3:
				return CoreMini::LIN3;
			case NetID::LIN4:
				return CoreMini::LIN4;
			case NetID::J1708:
				return CoreMini::J1708;
			case NetID::HSCAN4:
				return CoreMini::HSCAN4;
			case NetID::HSCAN5:
				return CoreMini::HSCAN5;
			case NetID::ISO9141:
				return CoreMini::KLine1;
			case NetID::ISO9141_2:
				return CoreMini::KLine2;
			case NetID::ISO9141_3:
				return CoreMini::KLine3;
			case NetID::ISO9141_4:
				return CoreMini::KLine4;
			case NetID::FlexRay1a:
				return CoreMini::FlexRay1a;
			case NetID::UART:
				return CoreMini::UART;
			case NetID::UART2:
				return CoreMini::UART2;
			case NetID::LIN5:
				return CoreMini::LIN5;
			case NetID::MOST25:
				return CoreMini::MOST25;
			case NetID::MOST50:
				return CoreMini::MOST50;
			case NetID::FlexRay1b:
				return CoreMini::FlexRay1b;
			case NetID::SWCAN2:
				return CoreMini::SWCAN2;
			case NetID::Ethernet_DAQ:
				return CoreMini::EthernetDAQ;
			case NetID::Ethernet:
				return CoreMini::Ethernet;
			case NetID::FlexRay2a:
				return CoreMini::FlexRay2a;
			case NetID::FlexRay2b:
				return CoreMini::FlexRay2b;
			case NetID::HSCAN6:
				return CoreMini::HSCAN6;
			case NetID::HSCAN7:
				return CoreMini::HSCAN7;
			case NetID::LIN6:
				return CoreMini::LIN6;
			case NetID::LSFTCAN2:
				return CoreMini::LSFTCAN2;
			case NetID::OP_Ethernet1:
				return CoreMini::OP_Ethernet1;
			case NetID::OP_Ethernet2:
				return CoreMini::OP_Ethernet2;
			case NetID::OP_Ethernet3:
				return CoreMini::OP_Ethernet3;
			case NetID::OP_Ethernet4:
				return CoreMini::OP_Ethernet4;
			case NetID::OP_Ethernet5:
				return CoreMini::OP_Ethernet5;
			case NetID::OP_Ethernet6:
				return CoreMini::OP_Ethernet6;
			case NetID::OP_Ethernet7:
				return CoreMini::OP_Ethernet7;
			case NetID::OP_Ethernet8:
				return CoreMini::OP_Ethernet8;
			case NetID::OP_Ethernet9:
				return CoreMini::OP_Ethernet9;
			case NetID::OP_Ethernet10:
				return CoreMini::OP_Ethernet10;
			case NetID::OP_Ethernet11:
				return CoreMini::OP_Ethernet11;
			case NetID::OP_Ethernet12:
				return CoreMini::OP_Ethernet12;
			case NetID::TCP:
				return CoreMini::TCPVirtual;
			case NetID::UDP:
				return CoreMini::UDPVirtual;
			case NetID::FlexRay:
				return CoreMini::FlexRay1;
			case NetID::FlexRay2:
				return CoreMini::FlexRay2;
			case NetID::ForwardedMessage:
				return CoreMini::ForwardedMessage;
			case NetID::I2C:
				return CoreMini::I2C1;
			case NetID::I2C2:
				return CoreMini::I2C2;
			case NetID::I2C3:
				return CoreMini::I2C3;
			case NetID::I2C4:
				return CoreMini::I2C4;
			case NetID::Ethernet2:
				return CoreMini::Ethernet2;
			case NetID::A2B1:
				return CoreMini::A2B1;
			case NetID::A2B2:
				return CoreMini::A2B2;
			case NetID::Ethernet3:
				return CoreMini::Ethernet3;
			case NetID::WBMS:
				return CoreMini::WBMS;
			case NetID::DWCAN9:
				return CoreMini::DWCAN9;
			case NetID::DWCAN10:
				return CoreMini::DWCAN10;
			case NetID::DWCAN11:
				return CoreMini::DWCAN11;
			case NetID::DWCAN12:
				return CoreMini::DWCAN12;
			case NetID::DWCAN13:
				return CoreMini::DWCAN13;
			case NetID::DWCAN14:
				return CoreMini::DWCAN14;
			case NetID::DWCAN15:
				return CoreMini::DWCAN15;
			case NetID::DWCAN16:
				return CoreMini::DWCAN16;
			case NetID::LIN7:
				return CoreMini::LIN7;
			case NetID::LIN8:
				return CoreMini::LIN8;
			default:
				return std::nullopt;
		}
	}
	static NetID GetNetIDFromCoreMiniNetwork(CoreMini cm) {
		switch(cm) {
			case CoreMini::HSCAN:
				return NetID::HSCAN;
			case CoreMini::MSCAN:
				return NetID::MSCAN;
			case CoreMini::LIN:
				return NetID::LIN;
			case CoreMini::LIN2:
				return NetID::LIN2;
			case CoreMini::Virtual:
				return NetID::Device;
			case CoreMini::HSCAN2:
				return NetID::HSCAN2;
			case CoreMini::LSFTCAN:
				return NetID::LSFTCAN;
			case CoreMini::SWCAN:
				return NetID::SWCAN;
			case CoreMini::HSCAN3:
				return NetID::HSCAN3;
			case CoreMini::CGI:
				return NetID::CGI;
			case CoreMini::J1850VPW:
				return NetID::J1850VPW;
			case CoreMini::LIN3:
				return NetID::LIN3;
			case CoreMini::LIN4:
				return NetID::LIN4;
			case CoreMini::J1708:
				return NetID::J1708;
			case CoreMini::HSCAN4:
				return NetID::HSCAN4;
			case CoreMini::HSCAN5:
				return NetID::HSCAN5;
			case CoreMini::KLine1:
				return NetID::ISO9141;
			case CoreMini::KLine2:
				return NetID::ISO9141_2;
			case CoreMini::KLine3:
				return NetID::ISO9141_3;
			case CoreMini::KLine4:
				return NetID::ISO9141_4;
			case CoreMini::FlexRay1a:
				return NetID::FlexRay1a;
			case CoreMini::UART:
				return NetID::UART;
			case CoreMini::UART2:
				return NetID::UART2;
			case CoreMini::LIN5:
				return NetID::LIN5;
			case CoreMini::MOST25:
				return NetID::MOST25;
			case CoreMini::MOST50:
				return NetID::MOST50;
			case CoreMini::FlexRay1b:
				return NetID::FlexRay1b;
			case CoreMini::SWCAN2:
				return NetID::SWCAN2;
			case CoreMini::EthernetDAQ:
				return NetID::Ethernet_DAQ;
			case CoreMini::Ethernet:
				return NetID::Ethernet;
			case CoreMini::FlexRay2a:
				return NetID::FlexRay2a;
			case CoreMini::FlexRay2b:
				return NetID::FlexRay2b;
			case CoreMini::HSCAN6:
				return NetID::HSCAN6;
			case CoreMini::HSCAN7:
				return NetID::HSCAN7;
			case CoreMini::LIN6:
				return NetID::LIN6;
			case CoreMini::LSFTCAN2:
				return NetID::LSFTCAN2;
			case CoreMini::OP_Ethernet1:
				return NetID::OP_Ethernet1;
			case CoreMini::OP_Ethernet2:
				return NetID::OP_Ethernet2;
			case CoreMini::OP_Ethernet3:
				return NetID::OP_Ethernet3;
			case CoreMini::OP_Ethernet4:
				return NetID::OP_Ethernet4;
			case CoreMini::OP_Ethernet5:
				return NetID::OP_Ethernet5;
			case CoreMini::OP_Ethernet6:
				return NetID::OP_Ethernet6;
			case CoreMini::OP_Ethernet7:
				return NetID::OP_Ethernet7;
			case CoreMini::OP_Ethernet8:
				return NetID::OP_Ethernet8;
			case CoreMini::OP_Ethernet9:
				return NetID::OP_Ethernet9;
			case CoreMini::OP_Ethernet10:
				return NetID::OP_Ethernet10;
			case CoreMini::OP_Ethernet11:
				return NetID::OP_Ethernet11;
			case CoreMini::OP_Ethernet12:
				return NetID::OP_Ethernet12;
			case CoreMini::TCPVirtual:
				return NetID::TCP;
			case CoreMini::UDPVirtual:
				return NetID::UDP;
			case CoreMini::FlexRay1:
				return NetID::FlexRay;
			case CoreMini::FlexRay2:
				return NetID::FlexRay2;
			case CoreMini::ForwardedMessage:
				return NetID::ForwardedMessage;
			case CoreMini::I2C1:
				return NetID::I2C;
			case CoreMini::I2C2:
				return NetID::I2C2;
			case CoreMini::I2C3:
				return NetID::I2C3;
			case CoreMini::I2C4:
				return NetID::I2C4;
			case CoreMini::Ethernet2:
				return NetID::Ethernet2;
			case CoreMini::A2B1:
				return NetID::A2B1;
			case CoreMini::A2B2:
				return NetID::A2B2;
			case CoreMini::Ethernet3:
				return NetID::Ethernet3;
			case CoreMini::WBMS:
				return NetID::WBMS;
			case CoreMini::DWCAN9:
				return NetID::DWCAN9;
			case CoreMini::DWCAN10:
				return NetID::DWCAN10;
			case CoreMini::DWCAN11:
				return NetID::DWCAN11;
			case CoreMini::DWCAN12:
				return NetID::DWCAN12;
			case CoreMini::DWCAN13:
				return NetID::DWCAN13;
			case CoreMini::DWCAN14:
				return NetID::DWCAN14;
			case CoreMini::DWCAN15:
				return NetID::DWCAN15;
			case CoreMini::DWCAN16:
				return NetID::DWCAN16;
			case CoreMini::LIN7:
				return NetID::LIN7;
			case CoreMini::LIN8:
				return NetID::LIN8;
		}
		return NetID::Invalid; // Should be unreachable, the compiler should warn about new CoreMini IDs
	}

	Network() { setValue(NetID::Invalid); }
	Network(neonetid_t netid) { setValue((NetID)netid); }
	Network(NetID netid) { setValue(netid); }
	Network(CoreMini cm) { setValue(GetNetIDFromCoreMiniNetwork(cm)); }
	NetID getNetID() const { return value; }
	Type getType() const { return type; }
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
	void setValue(NetID id) {
		value = id;
		type = GetTypeOfNetID(value);
	}
};

}

#endif // __cplusplus

#ifdef __ICSNEOC_H_
#define ICSNEO_NETID_DEVICE 0
#define ICSNEO_NETID_HSCAN 1
#define ICSNEO_NETID_MSCAN 2
#define ICSNEO_NETID_SWCAN 3
#define ICSNEO_NETID_LSFTCAN 4
#define ICSNEO_NETID_FORDSCP 5
#define ICSNEO_NETID_J1708 6
#define ICSNEO_NETID_AUX 7
#define ICSNEO_NETID_J1850VPW 8
#define ICSNEO_NETID_ISO9141 9
#define ICSNEO_NETID_ISOPIC 10
#define ICSNEO_NETID_MAIN51 11
#define ICSNEO_NETID_RED 12
#define ICSNEO_NETID_SCI 13
#define ICSNEO_NETID_ISO9141_2 14
#define ICSNEO_NETID_ISO14230 15
#define ICSNEO_NETID_LIN 16
#define ICSNEO_NETID_OP_ETHERNET1 17
#define ICSNEO_NETID_OP_ETHERNET2 18
#define ICSNEO_NETID_OP_ETHERNET3 19

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

#define ICSNEO_NETID_ISO9141_3 41
#define ICSNEO_NETID_HSCAN2 42
#define ICSNEO_NETID_HSCAN3 44
#define ICSNEO_NETID_OP_ETHERNET4 45
#define ICSNEO_NETID_OP_ETHERNET5 46
#define ICSNEO_NETID_ISO9141_4 47
#define ICSNEO_NETID_LIN2 48
#define ICSNEO_NETID_LIN3 49
#define ICSNEO_NETID_LIN4 50
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
#define ICSNEO_NETID_HSCAN4 61
#define ICSNEO_NETID_HSCAN5 62
#define ICSNEO_NETID_RS232 63
#define ICSNEO_NETID_UART 64
#define ICSNEO_NETID_UART2 65
#define ICSNEO_NETID_UART3 66
#define ICSNEO_NETID_UART4 67
#define ICSNEO_NETID_SWCAN2 68
#define ICSNEO_NETID_ETHERNET_DAQ 69
#define ICSNEO_NETID_DATA_TO_HOST 70
#define ICSNEO_NETID_TEXTAPI_TO_HOST 71
#define ICSNEO_NETID_OP_ETHERNET6 73
#define ICSNEO_NETID_RED_VBAT 74
#define ICSNEO_NETID_OP_ETHERNET7 75
#define ICSNEO_NETID_OP_ETHERNET8 76
#define ICSNEO_NETID_OP_ETHERNET9 77
#define ICSNEO_NETID_OP_ETHERNET10 78
#define ICSNEO_NETID_OP_ETHERNET11 79
#define ICSNEO_NETID_FLEXRAY1A 80
#define ICSNEO_NETID_FLEXRAY1B 81
#define ICSNEO_NETID_FLEXRAY2A 82
#define ICSNEO_NETID_FLEXRAY2B 83
#define ICSNEO_NETID_LIN5 84
#define ICSNEO_NETID_FLEXRAY 85
#define ICSNEO_NETID_FLEXRAY2 86
#define ICSNEO_NETID_OP_ETHERNET12 87
#define ICSNEO_NETID_I2C 88
#define ICSNEO_NETID_MOST25 90
#define ICSNEO_NETID_MOST50 91
#define ICSNEO_NETID_MOST150 92
#define ICSNEO_NETID_ETHERNET 93
#define ICSNEO_NETID_GMFSA 94
#define ICSNEO_NETID_TCP 95
#define ICSNEO_NETID_HSCAN6 96
#define ICSNEO_NETID_HSCAN7 97
#define ICSNEO_NETID_LIN6 98
#define ICSNEO_NETID_LSFTCAN2 99
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
#define ICSNEO_NETID_I2C2 517
#define ICSNEO_NETID_I2C3 518
#define ICSNEO_NETID_I2C4 519
#define ICSNEO_NETID_ETHERNET2 520
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