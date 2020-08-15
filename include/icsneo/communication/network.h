#ifndef __NETWORKID_H_
#define __NETWORKID_H_

#ifdef __cplusplus

#include <cstdint>
#include <ostream>

namespace icsneo {

class Network {
public:
	enum class NetID : uint16_t {
		Device = 0,
		HSCAN = 1,
		MSCAN = 2,
		SWCAN = 3,
		LSFTCAN = 4,
		FordSCP = 5,
		J1708 = 6,
		Aux = 7,
		J1850VPW = 8,
		ISO = 9,
		ISOPIC = 10,
		Main51 = 11,
		RED = 12,
		SCI = 13,
		ISO2 = 14,
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
		RED_SDCARD_READ = 23,
		CAN_ERRBITS = 24,
		RED_DFLASH_WRITE_DONE = 25,
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

		ISO3 = 41,
		HSCAN2 = 42,
		HSCAN3 = 44,
		OP_Ethernet4 = 45,
		OP_Ethernet5 = 46,
		ISO4 = 47,
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
		FlexRayControl = 243,
		HW_COM_Latency_Test = 512,
		Device_Status = 513,
		Any = 0xfffe, // Never actually set as type, but used as flag for filtering
		Invalid = 0xffff
	};
	enum class Type : uint8_t {
		Invalid = 0,
		Internal = 1, // Used for statuses that don't actually need to be transferred to the client application
		CAN = 2,
		LIN = 3,
		FlexRay = 4,
		MOST = 5,
		Ethernet = 6,
		LSFTCAN = 7,
		SWCAN = 8,
		Any = 0xFE, // Never actually set as type, but used as flag for filtering
		Other = 0xFF
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
			case Type::Ethernet:
				return "Ethernet";
			case Type::LSFTCAN:
				return "Low Speed Fault Tolerant CAN";
			case Type::SWCAN:
				return "Single Wire CAN";
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
				return Type::CAN;
			case NetID::LIN:
			case NetID::LIN2:
			case NetID::LIN3:
			case NetID::LIN4:
			case NetID::LIN5:
			case NetID::LIN6:
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
			case NetID::Reset_Status:
			case NetID::Device_Status:
			case NetID::FlexRayControl:
				return Type::Internal;
			case NetID::Invalid:
			case NetID::Any:
				return Type::Invalid;
			case NetID::Ethernet:
			case NetID::Ethernet_DAQ:
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
			default:
				return Type::Other;
		}
	}
	static const char* GetNetIDString(NetID netid) {
		switch(netid) {
			case NetID::Device:
				return "Device";
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
			case NetID::ISO:
				return "ISO";
			case NetID::ISOPIC:
				return "ISOPIC";
			case NetID::Main51:
				return "Main51";
			case NetID::RED:
				return "RED";
			case NetID::SCI:
				return "SCI";
			case NetID::ISO2:
				return "ISO 2";
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
			case NetID::RED_SDCARD_READ:
				return "RED_SDCARD_READ";
			case NetID::CAN_ERRBITS:
				return "CAN_ERRBITS";
			case NetID::RED_DFLASH_WRITE_DONE:
				return "RED_DFLASH_WRITE_DONE";
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
			case NetID::ISO3:
				return "ISO 3";
			case NetID::HSCAN2:
				return "HSCAN 2";
			case NetID::HSCAN3:
				return "HSCAN 3";
			case NetID::OP_Ethernet4:
				return "OP (BR) Ethernet 4";
			case NetID::OP_Ethernet5:
				return "OP (BR) Ethernet 5";
			case NetID::ISO4:
				return "ISO 4";
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
			case NetID::FlexRayControl:
				return "FlexRay Control";
			case NetID::HW_COM_Latency_Test:
				return "HW COM Latency Test";
			case NetID::Device_Status:
				return "Device Status";
			case NetID::Any:
			case NetID::Invalid:
				break;
		}
		return "Invalid Network";
	}

	Network() { setValue(NetID::Invalid); }
	Network(uint16_t netid) { setValue((NetID)netid); }
	Network(NetID netid) { setValue(netid); }
	NetID getNetID() const { return value; }
	Type getType() const { return type; }
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
#define ICSNEO_NETID_ISO 9
#define ICSNEO_NETID_ISOPIC 10
#define ICSNEO_NETID_MAIN51 11
#define ICSNEO_NETID_RED 12
#define ICSNEO_NETID_SCI 13
#define ICSNEO_NETID_ISO2 14
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
#define ICSNEO_NETID_RED_SDCARD_READ 23
#define ICSNEO_NETID_CAN_ERRBITS 24
#define ICSNEO_NETID_RED_DFLASH_WRITE_DONE 25
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

#define ICSNEO_NETID_ISO3 41
#define ICSNEO_NETID_HSCAN2 42
#define ICSNEO_NETID_HSCAN3 44
#define ICSNEO_NETID_OP_ETHERNET4 45
#define ICSNEO_NETID_OP_ETHERNET5 46
#define ICSNEO_NETID_ISO4 47
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
#define ICSNEO_NETID_HW_COM_LATENCY_TEST 512
#define ICSNEO_NETID_DEVICE_STATUS 513
#define ICSNEO_NETID_ANY 0xfffe // Never actually set as type, but used as flag for filtering
#define ICSNEO_NETID_INVALID 0xffff

#define ICSNEO_NETWORK_TYPE_INVALID 0
#define ICSNEO_NETWORK_TYPE_INTERNAL 1 // Used for statuses that don't actually need to be transferred to the client application
#define ICSNEO_NETWORK_TYPE_CAN 2
#define ICSNEO_NETWORK_TYPE_LIN 3
#define ICSNEO_NETWORK_TYPE_FLEXRAY 4
#define ICSNEO_NETWORK_TYPE_MOST 5
#define ICSNEO_NETWORK_TYPE_ETHERNET 6
#define ICSNEO_NETWORK_TYPE_ANY 0xFE // Never actually set as type, but used as flag for filtering
#define ICSNEO_NETWORK_TYPE_OTHER 0xFF
#endif

#endif