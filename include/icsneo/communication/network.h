#ifndef __NETWORKID_H_
#define __NETWORKID_H_

#include <stdint.h>
#include <icsneo/icsneotypes.h>

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
		SPI1 = 72,
		SPI2 = 73,
		MDIO1 = 75,
		MDIO2 = 76,
		MDIO3 = 77,
		MDIO4 = 78,
		MDIO5 = 79,
		MDIO6 = 80,
		MDIO7 = 81,
		MDIO8 = 82,
		OP_Ethernet13 = 83,
		OP_Ethernet14 = 84,
		OP_Ethernet15 = 85,
		OP_Ethernet16 = 86,
		SPI3 = 87,
		SPI4 = 88,
		SPI5 = 89,
		SPI6 = 90,
		SPI7 = 91,
		SPI8 = 92,
		LIN9 = 93,
		LIN10 = 94,
		LIN11 = 95,
		LIN12 = 96,
		LIN13 = 97,
		LIN14 = 98,
		LIN15 = 99,
		LIN16 = 100,
	};
	static const char* GetTypeString(icsneo_msg_bus_type_t type) {
		switch(type) {
		case icsneo_msg_bus_type_can:
			return "CAN";
		case icsneo_msg_bus_type_lin:
			return "LIN";
		case icsneo_msg_bus_type_flexray:
			return "FlexRay";
		case icsneo_msg_bus_type_most:
			return "MOST";
		case icsneo_msg_bus_type_other:
			return "Other";
		case icsneo_msg_bus_type_internal:
			return "Internal";
		case icsneo_msg_bus_type_iso9141:
			return "ISO 9141-2";
		case icsneo_msg_bus_type_ethernet:
			return "Ethernet";
		case icsneo_msg_bus_type_lsftcan:
			return "Low Speed Fault Tolerant CAN";
		case icsneo_msg_bus_type_swcan:
			return "Single Wire CAN";
		case icsneo_msg_bus_type_i2c:
			return "I²C";
		case icsneo_msg_bus_type_a2b:
			return "A2B";
		case icsneo_msg_bus_type_spi:
			return "SPI";
		case icsneo_msg_bus_type_mdio:
			return "MDIO";
		case icsneo_msg_bus_type_invalid:
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
	static _icsneo_netid_t OffsetToSimpleNetworkId(uint16_t offset) {
		switch(offset) {
		default:
		case 0:
			return _icsneo_netid_t::icsneo_netid_device;
		case 1:
			return _icsneo_netid_t::icsneo_netid_hscan;
		case 2:
			return _icsneo_netid_t::icsneo_netid_mscan;
		case 3:
			return _icsneo_netid_t::icsneo_netid_swcan;
		case 4:
			return _icsneo_netid_t::icsneo_netid_lsftcan;
		case 5:
			return _icsneo_netid_t::icsneo_netid_fordscp;
		case 6:
			return _icsneo_netid_t::icsneo_netid_j1708;
		case 7:
			return _icsneo_netid_t::icsneo_netid_aux;
		case 8:
			return _icsneo_netid_t::icsneo_netid_j1850vpw;
		case 9:
			return _icsneo_netid_t::icsneo_netid_iso9141;
		case 10:
			return _icsneo_netid_t::icsneo_netid_disk_data;
		case 11:
			return _icsneo_netid_t::icsneo_netid_main51;
		case 12:
			return _icsneo_netid_t::icsneo_netid_red;
		case 13:
			return _icsneo_netid_t::icsneo_netid_sci;
		case 14:
			return _icsneo_netid_t::icsneo_netid_iso9141_2;
		case 15:
			return _icsneo_netid_t::icsneo_netid_iso14230;
		case 16:
			return _icsneo_netid_t::icsneo_netid_lin;
		case 17:
			return _icsneo_netid_t::icsneo_netid_iso9141_3;
		case 18:
			return _icsneo_netid_t::icsneo_netid_hscan2;
		case 19:
			return _icsneo_netid_t::icsneo_netid_hscan3;
		case 20:
			return _icsneo_netid_t::icsneo_netid_iso9141_4;
		case 21:
			return _icsneo_netid_t::icsneo_netid_lin2;
		case 22:
			return _icsneo_netid_t::icsneo_netid_lin3;
		case 23:
			return _icsneo_netid_t::icsneo_netid_lin4;
		//case 24:
		//	return NetID::MOST;	// Deprecated
		case 25:
			return _icsneo_netid_t::icsneo_netid_cgi;
		case 26:
			return _icsneo_netid_t::icsneo_netid_i2c;
		case 27:
			return _icsneo_netid_t::icsneo_netid_spi1;
		case 28:
			return _icsneo_netid_t::icsneo_netid_flexray1a;
		case 29:
			return _icsneo_netid_t::icsneo_netid_most25;
		case 30:
			return _icsneo_netid_t::icsneo_netid_most50;
		case 31:
			return _icsneo_netid_t::icsneo_netid_most150;
		case 32:
			return _icsneo_netid_t::icsneo_netid_hscan4;
		case 33:
			return _icsneo_netid_t::icsneo_netid_hscan5;
		case 34:
			return _icsneo_netid_t::icsneo_netid_rs232;
		case 35:
			return _icsneo_netid_t::icsneo_netid_uart;
		case 36:
			return _icsneo_netid_t::icsneo_netid_uart2;
		case 37:
			return _icsneo_netid_t::icsneo_netid_uart3;
		case 38:
			return _icsneo_netid_t::icsneo_netid_uart4;
		case 39:
			return _icsneo_netid_t::icsneo_netid_swcan2;
		case 40:
			return _icsneo_netid_t::icsneo_netid_flexray1b;
		case 41:
			return _icsneo_netid_t::icsneo_netid_flexray2a;
		case 42:
			return _icsneo_netid_t::icsneo_netid_flexray2b;
		case 43:
			return _icsneo_netid_t::icsneo_netid_lin5;
		case 44:
			return _icsneo_netid_t::icsneo_netid_ethernet;
		case 45:
			return _icsneo_netid_t::icsneo_netid_ethernet_daq;
		case 46:
			return _icsneo_netid_t::icsneo_netid_red_app_error;
		case 47:
			return _icsneo_netid_t::icsneo_netid_hscan6;
		case 48:
			return _icsneo_netid_t::icsneo_netid_hscan7;
		case 49:
			return _icsneo_netid_t::icsneo_netid_lin6;
		case 50:
			return _icsneo_netid_t::icsneo_netid_lsftcan2;
		}
	}
	static bool Within(icsneo_netid_t value, icsneo_netid_t min, icsneo_netid_t max) {
		return ((min <= value) && (value < max));
	}
	static bool IdIsSlaveARange1(icsneo_netid_t fullNetid) {
		return Within(fullNetid, OFFSET_PLASMA_SLAVE1, OFFSET_PLASMA_SLAVE1 + COUNT_PLASMA_SLAVE);
	}
	static bool IdIsSlaveARange2(icsneo_netid_t fullNetid) {
		return Within(fullNetid, OFFSET_PLASMA_SLAVE1_RANGE2, OFFSET_PLASMA_SLAVE2_RANGE2);
	}
	static bool IdIsSlaveBRange1(icsneo_netid_t fullNetid) {
		return Within(fullNetid, OFFSET_PLASMA_SLAVE2, OFFSET_PLASMA_SLAVE2 + COUNT_PLASMA_SLAVE);
	}
	static bool IdIsSlaveBRange2(icsneo_netid_t fullNetid) {
		return Within(fullNetid, OFFSET_PLASMA_SLAVE2_RANGE2, OFFSET_PLASMA_SLAVE3_RANGE2);
	}
	static std::pair<VnetId, _icsneo_netid_t> GetVnetAgnosticNetid(icsneo_netid_t fullNetid) {
		VnetId vnetId = VnetId::None;
		_icsneo_netid_t netId;

		if(fullNetid < OFFSET_PLASMA_SLAVE1) {
			netId = static_cast<_icsneo_netid_t>(fullNetid);
		} else if(IdIsSlaveARange1(fullNetid)) {
			netId = OffsetToSimpleNetworkId(fullNetid - OFFSET_PLASMA_SLAVE1);
			vnetId = VnetId::VNET_A;
		} else if(IdIsSlaveARange2(fullNetid)) {
			netId = static_cast<_icsneo_netid_t>((fullNetid - OFFSET_PLASMA_SLAVE1_RANGE2));
			vnetId = VnetId::VNET_A;
		} else if(IdIsSlaveBRange1(fullNetid)) {
			netId = OffsetToSimpleNetworkId(fullNetid - OFFSET_PLASMA_SLAVE2);
			vnetId = VnetId::VNET_B;
		} else if(IdIsSlaveBRange2(fullNetid)) {
			netId = static_cast<_icsneo_netid_t>((fullNetid - OFFSET_PLASMA_SLAVE2_RANGE2));
			vnetId = VnetId::VNET_B;
		} else {
			netId = static_cast<_icsneo_netid_t>(fullNetid);
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
	static icsneo_msg_bus_type_t GetTypeOfNetID(_icsneo_netid_t netid, bool expand = true) {
		if(expand) {
			netid = GetVnetAgnosticNetid((icsneo_netid_t)netid).second;
		}

		switch(netid) {
		case _icsneo_netid_t::icsneo_netid_hscan:
		case _icsneo_netid_t::icsneo_netid_mscan:
		case _icsneo_netid_t::icsneo_netid_hscan2:
		case _icsneo_netid_t::icsneo_netid_hscan3:
		case _icsneo_netid_t::icsneo_netid_hscan4:
		case _icsneo_netid_t::icsneo_netid_hscan5:
		case _icsneo_netid_t::icsneo_netid_hscan6:
		case _icsneo_netid_t::icsneo_netid_hscan7:
		case _icsneo_netid_t::icsneo_netid_dwcan9:
		case _icsneo_netid_t::icsneo_netid_dwcan10:
		case _icsneo_netid_t::icsneo_netid_dwcan11:
		case _icsneo_netid_t::icsneo_netid_dwcan12:
		case _icsneo_netid_t::icsneo_netid_dwcan13:
		case _icsneo_netid_t::icsneo_netid_dwcan14:
		case _icsneo_netid_t::icsneo_netid_dwcan15:
		case _icsneo_netid_t::icsneo_netid_dwcan16:
			return icsneo_msg_bus_type_can;
		case _icsneo_netid_t::icsneo_netid_lin:
		case _icsneo_netid_t::icsneo_netid_lin2:
		case _icsneo_netid_t::icsneo_netid_lin3:
		case _icsneo_netid_t::icsneo_netid_lin4:
		case _icsneo_netid_t::icsneo_netid_lin5:
		case _icsneo_netid_t::icsneo_netid_lin6:
		case _icsneo_netid_t::icsneo_netid_lin7:
		case _icsneo_netid_t::icsneo_netid_lin8:
		case _icsneo_netid_t::icsneo_netid_lin9:
		case _icsneo_netid_t::icsneo_netid_lin10:
		case _icsneo_netid_t::icsneo_netid_lin11:
		case _icsneo_netid_t::icsneo_netid_lin12:
		case _icsneo_netid_t::icsneo_netid_lin13:
		case _icsneo_netid_t::icsneo_netid_lin14:
		case _icsneo_netid_t::icsneo_netid_lin15:
		case _icsneo_netid_t::icsneo_netid_lin16:
			return icsneo_msg_bus_type_lin;
		case _icsneo_netid_t::icsneo_netid_flexray:
		case _icsneo_netid_t::icsneo_netid_flexray1a:
		case _icsneo_netid_t::icsneo_netid_flexray1b:
		case _icsneo_netid_t::icsneo_netid_flexray2:
		case _icsneo_netid_t::icsneo_netid_flexray2a:
		case _icsneo_netid_t::icsneo_netid_flexray2b:
			return icsneo_msg_bus_type_flexray;
		case _icsneo_netid_t::icsneo_netid_most25:
		case _icsneo_netid_t::icsneo_netid_most50:
		case _icsneo_netid_t::icsneo_netid_most150:
			return icsneo_msg_bus_type_most;
		case _icsneo_netid_t::icsneo_netid_red:
		case _icsneo_netid_t::icsneo_netid_red_oldformat:
		case _icsneo_netid_t::icsneo_netid_device:
		case _icsneo_netid_t::icsneo_netid_reset_status:
		case _icsneo_netid_t::icsneo_netid_device_status:
		case _icsneo_netid_t::icsneo_netid_flexray_control:
		case _icsneo_netid_t::icsneo_netid_main51:
		case _icsneo_netid_t::icsneo_netid_read_settings:
		case _icsneo_netid_t::icsneo_netid_logical_disk_info:
		case _icsneo_netid_t::icsneo_netid_wivi_command:
		case _icsneo_netid_t::icsneo_netid_script_status:
		case _icsneo_netid_t::icsneo_netid_eth_phy_control:
		case _icsneo_netid_t::icsneo_netid_coremini_preload:
		case _icsneo_netid_t::icsneo_netid_extended_command:
		case _icsneo_netid_t::icsneo_netid_extended_data:
		case _icsneo_netid_t::icsneo_netid_red_int_memoryread:
		case _icsneo_netid_t::icsneo_netid_neo_memory_sdread:
		case _icsneo_netid_t::icsneo_netid_neo_memory_write_done:
		case _icsneo_netid_t::icsneo_netid_red_get_rtc:
		case _icsneo_netid_t::icsneo_netid_disk_data:
		case _icsneo_netid_t::icsneo_netid_red_app_error:
			return icsneo_msg_bus_type_internal;
		case _icsneo_netid_t::icsneo_netid_invalid:
		case _icsneo_netid_t::icsneo_netid_any:
			return icsneo_msg_bus_type_invalid;
		case _icsneo_netid_t::icsneo_netid_ethernet:
		case _icsneo_netid_t::icsneo_netid_ethernet_daq:
		case _icsneo_netid_t::icsneo_netid_ethernet2:
		case _icsneo_netid_t::icsneo_netid_ethernet3:
		case _icsneo_netid_t::icsneo_netid_op_ethernet1:
		case _icsneo_netid_t::icsneo_netid_op_ethernet2:
		case _icsneo_netid_t::icsneo_netid_op_ethernet3:
		case _icsneo_netid_t::icsneo_netid_op_ethernet4:
		case _icsneo_netid_t::icsneo_netid_op_ethernet5:
		case _icsneo_netid_t::icsneo_netid_op_ethernet6:
		case _icsneo_netid_t::icsneo_netid_op_ethernet7:
		case _icsneo_netid_t::icsneo_netid_op_ethernet8:
		case _icsneo_netid_t::icsneo_netid_op_ethernet9:
		case _icsneo_netid_t::icsneo_netid_op_ethernet10:
		case _icsneo_netid_t::icsneo_netid_op_ethernet11:
		case _icsneo_netid_t::icsneo_netid_op_ethernet12:
		case _icsneo_netid_t::icsneo_netid_op_ethernet13:
		case _icsneo_netid_t::icsneo_netid_op_ethernet14:
		case _icsneo_netid_t::icsneo_netid_op_ethernet15:
		case _icsneo_netid_t::icsneo_netid_op_ethernet16:
			return icsneo_msg_bus_type_ethernet;
		case _icsneo_netid_t::icsneo_netid_lsftcan:
		case _icsneo_netid_t::icsneo_netid_lsftcan2:
			return icsneo_msg_bus_type_lsftcan;
		case _icsneo_netid_t::icsneo_netid_swcan:
		case _icsneo_netid_t::icsneo_netid_swcan2:
			return icsneo_msg_bus_type_swcan;
		case _icsneo_netid_t::icsneo_netid_iso9141:
		case _icsneo_netid_t::icsneo_netid_iso9141_2:
		case _icsneo_netid_t::icsneo_netid_iso9141_3:
		case _icsneo_netid_t::icsneo_netid_iso9141_4:
			return icsneo_msg_bus_type_iso9141;
		case _icsneo_netid_t::icsneo_netid_i2c:
		case _icsneo_netid_t::icsneo_netid_i2c2:
		case _icsneo_netid_t::icsneo_netid_i2c3:
		case _icsneo_netid_t::icsneo_netid_i2c4:
			return icsneo_msg_bus_type_i2c;
		case _icsneo_netid_t::icsneo_netid_a2b1:
		case _icsneo_netid_t::icsneo_netid_a2b2:
			return icsneo_msg_bus_type_a2b;
		case _icsneo_netid_t::icsneo_netid_spi1:
		case _icsneo_netid_t::icsneo_netid_spi2:
		case _icsneo_netid_t::icsneo_netid_spi3:
		case _icsneo_netid_t::icsneo_netid_spi4:
		case _icsneo_netid_t::icsneo_netid_spi5:
		case _icsneo_netid_t::icsneo_netid_spi6:
		case _icsneo_netid_t::icsneo_netid_spi7:
		case _icsneo_netid_t::icsneo_netid_spi8:
			return icsneo_msg_bus_type_spi;
		case _icsneo_netid_t::icsneo_netid_mdio1:
		case _icsneo_netid_t::icsneo_netid_mdio2:
		case _icsneo_netid_t::icsneo_netid_mdio3:
		case _icsneo_netid_t::icsneo_netid_mdio4:
		case _icsneo_netid_t::icsneo_netid_mdio5:
		case _icsneo_netid_t::icsneo_netid_mdio6:
		case _icsneo_netid_t::icsneo_netid_mdio7:
		case _icsneo_netid_t::icsneo_netid_mdio8:
			return icsneo_msg_bus_type_mdio;
		default:
			return icsneo_msg_bus_type_other;
		}
	}
	static const char* GetNetIDString(_icsneo_netid_t netid, bool expand = true) {
		if(expand) {
			netid = GetVnetAgnosticNetid((icsneo_netid_t)netid).second;
		}
		switch(netid) {
		case _icsneo_netid_t::icsneo_netid_device:
			return "neoVI";
		case _icsneo_netid_t::icsneo_netid_hscan:
			return "HSCAN";
		case _icsneo_netid_t::icsneo_netid_mscan:
			return "MSCAN";
		case _icsneo_netid_t::icsneo_netid_swcan:
			return "SWCAN";
		case _icsneo_netid_t::icsneo_netid_lsftcan:
			return "LSFTCAN";
		case _icsneo_netid_t::icsneo_netid_fordscp:
			return "FordSCP";
		case _icsneo_netid_t::icsneo_netid_j1708:
			return "J1708";
		case _icsneo_netid_t::icsneo_netid_aux:
			return "Aux";
		case _icsneo_netid_t::icsneo_netid_j1850vpw:
			return "J1850 VPW";
		case _icsneo_netid_t::icsneo_netid_iso9141:
			return "ISO 9141";
		case _icsneo_netid_t::icsneo_netid_disk_data:
			return "Disk Data";
		case _icsneo_netid_t::icsneo_netid_main51:
			return "Main51";
		case _icsneo_netid_t::icsneo_netid_red:
			return "RED";
		case _icsneo_netid_t::icsneo_netid_sci:
			return "SCI";
		case _icsneo_netid_t::icsneo_netid_iso9141_2:
			return "ISO 9141 2";
		case _icsneo_netid_t::icsneo_netid_iso14230:
			return "ISO 14230";
		case _icsneo_netid_t::icsneo_netid_lin:
			return "LIN";
		case _icsneo_netid_t::icsneo_netid_op_ethernet1:
			return "OP (BR) Ethernet 1";
		case _icsneo_netid_t::icsneo_netid_op_ethernet2:
			return "OP (BR) Ethernet 2";
		case _icsneo_netid_t::icsneo_netid_op_ethernet3:
			return "OP (BR) Ethernet 3";
		case _icsneo_netid_t::icsneo_netid_red_ext_memoryread:
			return "RED_EXT_MEMORYREAD";
		case _icsneo_netid_t::icsneo_netid_red_int_memoryread:
			return "RED_INT_MEMORYREAD";
		case _icsneo_netid_t::icsneo_netid_red_dflash_read:
			return "RED_DFLASH_READ";
		case _icsneo_netid_t::icsneo_netid_neo_memory_sdread:
			return "NeoMemorySDRead";
		case _icsneo_netid_t::icsneo_netid_can_errbits:
			return "CAN_ERRBITS";
		case _icsneo_netid_t::icsneo_netid_neo_memory_write_done:
			return "NeoMemoryWriteDone";
		case _icsneo_netid_t::icsneo_netid_red_wave_can1_logical:
			return "RED_WAVE_CAN1_LOGICAL";
		case _icsneo_netid_t::icsneo_netid_red_wave_can2_logical:
			return "RED_WAVE_CAN2_LOGICAL";
		case _icsneo_netid_t::icsneo_netid_red_wave_lin1_logical:
			return "RED_WAVE_LIN1_LOGICAL";
		case _icsneo_netid_t::icsneo_netid_red_wave_lin2_logical:
			return "RED_WAVE_LIN2_LOGICAL";
		case _icsneo_netid_t::icsneo_netid_red_wave_lin1_analog:
			return "RED_WAVE_LIN1_ANALOG";
		case _icsneo_netid_t::icsneo_netid_red_wave_lin2_analog:
			return "RED_WAVE_LIN2_ANALOG";
		case _icsneo_netid_t::icsneo_netid_red_wave_misc_analog:
			return "RED_WAVE_MISC_ANALOG";
		case _icsneo_netid_t::icsneo_netid_red_wave_miscdio2_logical:
			return "RED_WAVE_MISCDIO2_LOGICAL";
		case _icsneo_netid_t::icsneo_netid_red_network_com_enable_ex:
			return "RED_NETWORK_COM_ENABLE_EX";
		case _icsneo_netid_t::icsneo_netid_red_neovi_network:
			return "RED_NEOVI_NETWORK";
		case _icsneo_netid_t::icsneo_netid_red_read_baud_settings:
			return "RED_READ_BAUD_SETTINGS";
		case _icsneo_netid_t::icsneo_netid_red_oldformat:
			return "RED_OLDFORMAT";
		case _icsneo_netid_t::icsneo_netid_red_scope_capture:
			return "RED_SCOPE_CAPTURE";
		case _icsneo_netid_t::icsneo_netid_red_hardware_excep:
			return "RED_HARDWARE_EXCEP";
		case _icsneo_netid_t::icsneo_netid_red_get_rtc:
			return "RED_GET_RTC";
		case _icsneo_netid_t::icsneo_netid_iso9141_3:
			return "ISO 9141 3";
		case _icsneo_netid_t::icsneo_netid_hscan2:
			return "HSCAN 2";
		case _icsneo_netid_t::icsneo_netid_hscan3:
			return "HSCAN 3";
		case _icsneo_netid_t::icsneo_netid_op_ethernet4:
			return "OP (BR) Ethernet 4";
		case _icsneo_netid_t::icsneo_netid_op_ethernet5:
			return "OP (BR) Ethernet 5";
		case _icsneo_netid_t::icsneo_netid_iso9141_4:
			return "ISO 9141 4";
		case _icsneo_netid_t::icsneo_netid_lin2:
			return "LIN 2";
		case _icsneo_netid_t::icsneo_netid_lin3:
			return "LIN 3";
		case _icsneo_netid_t::icsneo_netid_lin4:
			return "LIN 4";
		case _icsneo_netid_t::icsneo_netid_red_app_error:
			return "App Error";
		case _icsneo_netid_t::icsneo_netid_cgi:
			return "CGI";
		case _icsneo_netid_t::icsneo_netid_reset_status:
			return "Reset Status";
		case _icsneo_netid_t::icsneo_netid_fb_status:
			return "FB Status";
		case _icsneo_netid_t::icsneo_netid_app_signal_status:
			return "App Signal Status";
		case _icsneo_netid_t::icsneo_netid_read_datalink_cm_tx_msg:
			return "Read Datalink Cm Tx Msg";
		case _icsneo_netid_t::icsneo_netid_read_datalink_cm_rx_msg:
			return "Read Datalink Cm Rx Msg";
		case _icsneo_netid_t::icsneo_netid_logging_overflow:
			return "Logging Overflow";
		case _icsneo_netid_t::icsneo_netid_read_settings:
			return "Read Settings";
		case _icsneo_netid_t::icsneo_netid_hscan4:
			return "HSCAN 4";
		case _icsneo_netid_t::icsneo_netid_hscan5:
			return "HSCAN 5";
		case _icsneo_netid_t::icsneo_netid_rs232:
			return "RS232";
		case _icsneo_netid_t::icsneo_netid_uart:
			return "UART";
		case _icsneo_netid_t::icsneo_netid_uart2:
			return "UART 2";
		case _icsneo_netid_t::icsneo_netid_uart3:
			return "UART 3";
		case _icsneo_netid_t::icsneo_netid_uart4:
			return "UART 4";
		case _icsneo_netid_t::icsneo_netid_swcan2:
			return "SWCAN 2";
		case _icsneo_netid_t::icsneo_netid_ethernet_daq:
			return "Ethernet DAQ";
		case _icsneo_netid_t::icsneo_netid_data_to_host:
			return "Data To Host";
		case _icsneo_netid_t::icsneo_netid_textapi_to_host:
			return "TextAPI To Host";
		case _icsneo_netid_t::icsneo_netid_op_ethernet6:
			return "OP (BR) Ethernet 6";
		case _icsneo_netid_t::icsneo_netid_red_vbat:
			return "Red VBat";
		case _icsneo_netid_t::icsneo_netid_op_ethernet7:
			return "OP (BR) Ethernet 7";
		case _icsneo_netid_t::icsneo_netid_op_ethernet8:
			return "OP (BR) Ethernet 8";
		case _icsneo_netid_t::icsneo_netid_op_ethernet9:
			return "OP (BR) Ethernet 9";
		case _icsneo_netid_t::icsneo_netid_op_ethernet10:
			return "OP (BR) Ethernet 10";
		case _icsneo_netid_t::icsneo_netid_op_ethernet11:
			return "OP (BR) Ethernet 11";
		case _icsneo_netid_t::icsneo_netid_flexray1a:
			return "FlexRay 1a";
		case _icsneo_netid_t::icsneo_netid_flexray1b:
			return "FlexRay 1b";
		case _icsneo_netid_t::icsneo_netid_flexray2a:
			return "FlexRay 2a";
		case _icsneo_netid_t::icsneo_netid_flexray2b:
			return "FlexRay 2b";
		case _icsneo_netid_t::icsneo_netid_lin5:
			return "LIN 5";
		case _icsneo_netid_t::icsneo_netid_flexray:
			return "FlexRay";
		case _icsneo_netid_t::icsneo_netid_flexray2:
			return "FlexRay 2";
		case _icsneo_netid_t::icsneo_netid_op_ethernet12:
			return "OP (BR) Ethernet 12";
		case _icsneo_netid_t::icsneo_netid_op_ethernet13:
			return "OP (BR) Ethernet 13";
		case _icsneo_netid_t::icsneo_netid_op_ethernet14:
			return "OP (BR) Ethernet 14";
		case _icsneo_netid_t::icsneo_netid_op_ethernet15:
			return "OP (BR) Ethernet 15";
		case _icsneo_netid_t::icsneo_netid_op_ethernet16:
			return "OP (BR) Ethernet 16";
		case _icsneo_netid_t::icsneo_netid_i2c:
			return "I2C";
		case _icsneo_netid_t::icsneo_netid_most25:
			return "MOST25";
		case _icsneo_netid_t::icsneo_netid_most50:
			return "MOST50";
		case _icsneo_netid_t::icsneo_netid_most150:
			return "MOST150";
		case _icsneo_netid_t::icsneo_netid_ethernet:
			return "Ethernet";
		case _icsneo_netid_t::icsneo_netid_gmfsa:
			return "GMFSA";
		case _icsneo_netid_t::icsneo_netid_tcp:
			return "TCP";
		case _icsneo_netid_t::icsneo_netid_hscan6:
			return "HSCAN 6";
		case _icsneo_netid_t::icsneo_netid_hscan7:
			return "HSCAN 7";
		case _icsneo_netid_t::icsneo_netid_lin6:
			return "LIN 6";
		case _icsneo_netid_t::icsneo_netid_lsftcan2:
			return "LSFTCAN 2";
		case _icsneo_netid_t::icsneo_netid_logical_disk_info:
			return "Logical Disk Information";
		case _icsneo_netid_t::icsneo_netid_wivi_command:
			return "WiVI Command";
		case _icsneo_netid_t::icsneo_netid_script_status:
			return "Script Status";
		case _icsneo_netid_t::icsneo_netid_coremini_preload:
			return "CoreMini PreLoad";
		case _icsneo_netid_t::icsneo_netid_eth_phy_control:
			return "Ethernet PHY Register Control";
		case _icsneo_netid_t::icsneo_netid_extended_command:
			return "Extended Command";
		case _icsneo_netid_t::icsneo_netid_extended_data:
			return "Extended Data";
		case _icsneo_netid_t::icsneo_netid_flexray_control:
			return "FlexRay Control";
		case _icsneo_netid_t::icsneo_netid_hw_com_latency_test:
			return "HW COM Latency Test";
		case _icsneo_netid_t::icsneo_netid_device_status:
			return "Device Status";
		case _icsneo_netid_t::icsneo_netid_udp:
			return "UDP";
		case _icsneo_netid_t::icsneo_netid_forwarded_message:
			return "Forwarded Message";
		case _icsneo_netid_t::icsneo_netid_i2c2:
			return "I2C 2";
		case _icsneo_netid_t::icsneo_netid_i2c3:
			return "I2C 3";
		case _icsneo_netid_t::icsneo_netid_i2c4:
			return "I2C 4";
		case _icsneo_netid_t::icsneo_netid_ethernet2:
			return "Ethernet 2";
		case _icsneo_netid_t::icsneo_netid_a2b1:
			return "A2B 1";
		case _icsneo_netid_t::icsneo_netid_a2b2:
			return "A2B 2";
		case _icsneo_netid_t::icsneo_netid_dwcan9:
			return "DWCAN 09";
		case _icsneo_netid_t::icsneo_netid_dwcan10:
			return "DWCAN 10";
		case _icsneo_netid_t::icsneo_netid_dwcan11:
			return "DWCAN 11";
		case _icsneo_netid_t::icsneo_netid_dwcan12:
			return "DWCAN 12";
		case _icsneo_netid_t::icsneo_netid_dwcan13:
			return "DWCAN 13";
		case _icsneo_netid_t::icsneo_netid_dwcan14:
			return "DWCAN 14";
		case _icsneo_netid_t::icsneo_netid_dwcan15:
			return "DWCAN 15";
		case _icsneo_netid_t::icsneo_netid_dwcan16:
			return "DWCAN 16";
		case _icsneo_netid_t::icsneo_netid_ethernet3:
			return "Ethernet 03";
		case _icsneo_netid_t::icsneo_netid_lin7:
			return "LIN 07";
		case _icsneo_netid_t::icsneo_netid_lin8:
			return "LIN 08";
		case _icsneo_netid_t::icsneo_netid_lin9:
			return "LIN 09";
		case _icsneo_netid_t::icsneo_netid_lin10:
			return "LIN 10";
		case _icsneo_netid_t::icsneo_netid_lin11:
			return "LIN 11";
		case _icsneo_netid_t::icsneo_netid_lin12:
			return "LIN 12";
		case _icsneo_netid_t::icsneo_netid_lin13:
			return "LIN 13";
		case _icsneo_netid_t::icsneo_netid_lin14:
			return "LIN 14";
		case _icsneo_netid_t::icsneo_netid_lin15:
			return "LIN 15";
		case _icsneo_netid_t::icsneo_netid_lin16:
			return "LIN 16";
		case _icsneo_netid_t::icsneo_netid_wbms:
			return "WBMS";
		case _icsneo_netid_t::icsneo_netid_spi1:
			return "SPI 1";
		case _icsneo_netid_t::icsneo_netid_spi2:
			return "SPI 2";
		case _icsneo_netid_t::icsneo_netid_spi3:
			return "SPI 3";
		case _icsneo_netid_t::icsneo_netid_spi4:
			return "SPI 4";
		case _icsneo_netid_t::icsneo_netid_spi5:
			return "SPI 5";
		case _icsneo_netid_t::icsneo_netid_spi6:
			return "SPI 6";
		case _icsneo_netid_t::icsneo_netid_spi7:
			return "SPI 7";
		case _icsneo_netid_t::icsneo_netid_spi8:
			return "SPI 8";
		case _icsneo_netid_t::icsneo_netid_mdio1:
			return "MDIO 1";
		case _icsneo_netid_t::icsneo_netid_mdio2:
			return "MDIO 2";
		case _icsneo_netid_t::icsneo_netid_mdio3:
			return "MDIO 3";
		case _icsneo_netid_t::icsneo_netid_mdio4:
			return "MDIO 4";
		case _icsneo_netid_t::icsneo_netid_mdio5:
			return "MDIO 5";
		case _icsneo_netid_t::icsneo_netid_mdio6:
			return "MDIO 6";
		case _icsneo_netid_t::icsneo_netid_mdio7:
			return "MDIO 7";
		case _icsneo_netid_t::icsneo_netid_mdio8:
			return "MDIO 8";
		case _icsneo_netid_t::icsneo_netid_any:
		case _icsneo_netid_t::icsneo_netid_invalid:
			break;
		}
		return "Invalid Network";
	}
	static std::optional<CoreMini> GetCoreMiniNetworkFromNetID(_icsneo_netid_t netid) {
		switch(netid) {
		case _icsneo_netid_t::icsneo_netid_hscan:
			return CoreMini::HSCAN;
		case _icsneo_netid_t::icsneo_netid_mscan:
			return CoreMini::MSCAN;
		case _icsneo_netid_t::icsneo_netid_lin:
			return CoreMini::LIN;
		case _icsneo_netid_t::icsneo_netid_lin2:
			return CoreMini::LIN2;
		case _icsneo_netid_t::icsneo_netid_device:
			return CoreMini::Virtual;
		case _icsneo_netid_t::icsneo_netid_hscan2:
			return CoreMini::HSCAN2;
		case _icsneo_netid_t::icsneo_netid_lsftcan:
			return CoreMini::LSFTCAN;
		case _icsneo_netid_t::icsneo_netid_swcan:
			return CoreMini::SWCAN;
		case _icsneo_netid_t::icsneo_netid_hscan3:
			return CoreMini::HSCAN3;
		case _icsneo_netid_t::icsneo_netid_cgi:
			return CoreMini::CGI;
		case _icsneo_netid_t::icsneo_netid_j1850vpw:
			return CoreMini::J1850VPW;
		case _icsneo_netid_t::icsneo_netid_lin3:
			return CoreMini::LIN3;
		case _icsneo_netid_t::icsneo_netid_lin4:
			return CoreMini::LIN4;
		case _icsneo_netid_t::icsneo_netid_j1708:
			return CoreMini::J1708;
		case _icsneo_netid_t::icsneo_netid_hscan4:
			return CoreMini::HSCAN4;
		case _icsneo_netid_t::icsneo_netid_hscan5:
			return CoreMini::HSCAN5;
		case _icsneo_netid_t::icsneo_netid_iso9141:
			return CoreMini::KLine1;
		case _icsneo_netid_t::icsneo_netid_iso9141_2:
			return CoreMini::KLine2;
		case _icsneo_netid_t::icsneo_netid_iso9141_3:
			return CoreMini::KLine3;
		case _icsneo_netid_t::icsneo_netid_iso9141_4:
			return CoreMini::KLine4;
		case _icsneo_netid_t::icsneo_netid_flexray1a:
			return CoreMini::FlexRay1a;
		case _icsneo_netid_t::icsneo_netid_uart:
			return CoreMini::UART;
		case _icsneo_netid_t::icsneo_netid_uart2:
			return CoreMini::UART2;
		case _icsneo_netid_t::icsneo_netid_lin5:
			return CoreMini::LIN5;
		case _icsneo_netid_t::icsneo_netid_most25:
			return CoreMini::MOST25;
		case _icsneo_netid_t::icsneo_netid_most50:
			return CoreMini::MOST50;
		case _icsneo_netid_t::icsneo_netid_flexray1b:
			return CoreMini::FlexRay1b;
		case _icsneo_netid_t::icsneo_netid_swcan2:
			return CoreMini::SWCAN2;
		case _icsneo_netid_t::icsneo_netid_ethernet_daq:
			return CoreMini::EthernetDAQ;
		case _icsneo_netid_t::icsneo_netid_ethernet:
			return CoreMini::Ethernet;
		case _icsneo_netid_t::icsneo_netid_flexray2a:
			return CoreMini::FlexRay2a;
		case _icsneo_netid_t::icsneo_netid_flexray2b:
			return CoreMini::FlexRay2b;
		case _icsneo_netid_t::icsneo_netid_hscan6:
			return CoreMini::HSCAN6;
		case _icsneo_netid_t::icsneo_netid_hscan7:
			return CoreMini::HSCAN7;
		case _icsneo_netid_t::icsneo_netid_lin6:
			return CoreMini::LIN6;
		case _icsneo_netid_t::icsneo_netid_lsftcan2:
			return CoreMini::LSFTCAN2;
		case _icsneo_netid_t::icsneo_netid_op_ethernet1:
			return CoreMini::OP_Ethernet1;
		case _icsneo_netid_t::icsneo_netid_op_ethernet2:
			return CoreMini::OP_Ethernet2;
		case _icsneo_netid_t::icsneo_netid_op_ethernet3:
			return CoreMini::OP_Ethernet3;
		case _icsneo_netid_t::icsneo_netid_op_ethernet4:
			return CoreMini::OP_Ethernet4;
		case _icsneo_netid_t::icsneo_netid_op_ethernet5:
			return CoreMini::OP_Ethernet5;
		case _icsneo_netid_t::icsneo_netid_op_ethernet6:
			return CoreMini::OP_Ethernet6;
		case _icsneo_netid_t::icsneo_netid_op_ethernet7:
			return CoreMini::OP_Ethernet7;
		case _icsneo_netid_t::icsneo_netid_op_ethernet8:
			return CoreMini::OP_Ethernet8;
		case _icsneo_netid_t::icsneo_netid_op_ethernet9:
			return CoreMini::OP_Ethernet9;
		case _icsneo_netid_t::icsneo_netid_op_ethernet10:
			return CoreMini::OP_Ethernet10;
		case _icsneo_netid_t::icsneo_netid_op_ethernet11:
			return CoreMini::OP_Ethernet11;
		case _icsneo_netid_t::icsneo_netid_op_ethernet12:
			return CoreMini::OP_Ethernet12;
		case _icsneo_netid_t::icsneo_netid_op_ethernet13:
			return CoreMini::OP_Ethernet13;
		case _icsneo_netid_t::icsneo_netid_op_ethernet14:
			return CoreMini::OP_Ethernet14;
		case _icsneo_netid_t::icsneo_netid_op_ethernet15:
			return CoreMini::OP_Ethernet15;
		case _icsneo_netid_t::icsneo_netid_op_ethernet16:
			return CoreMini::OP_Ethernet16;
		case _icsneo_netid_t::icsneo_netid_tcp:
			return CoreMini::TCPVirtual;
		case _icsneo_netid_t::icsneo_netid_udp:
			return CoreMini::UDPVirtual;
		case _icsneo_netid_t::icsneo_netid_flexray:
			return CoreMini::FlexRay1;
		case _icsneo_netid_t::icsneo_netid_flexray2:
			return CoreMini::FlexRay2;
		case _icsneo_netid_t::icsneo_netid_forwarded_message:
			return CoreMini::ForwardedMessage;
		case _icsneo_netid_t::icsneo_netid_i2c:
			return CoreMini::I2C1;
		case _icsneo_netid_t::icsneo_netid_i2c2:
			return CoreMini::I2C2;
		case _icsneo_netid_t::icsneo_netid_i2c3:
			return CoreMini::I2C3;
		case _icsneo_netid_t::icsneo_netid_i2c4:
			return CoreMini::I2C4;
		case _icsneo_netid_t::icsneo_netid_ethernet2:
			return CoreMini::Ethernet2;
		case _icsneo_netid_t::icsneo_netid_a2b1:
			return CoreMini::A2B1;
		case _icsneo_netid_t::icsneo_netid_a2b2:
			return CoreMini::A2B2;
		case _icsneo_netid_t::icsneo_netid_ethernet3:
			return CoreMini::Ethernet3;
		case _icsneo_netid_t::icsneo_netid_wbms:
			return CoreMini::WBMS;
		case _icsneo_netid_t::icsneo_netid_dwcan9:
			return CoreMini::DWCAN9;
		case _icsneo_netid_t::icsneo_netid_dwcan10:
			return CoreMini::DWCAN10;
		case _icsneo_netid_t::icsneo_netid_dwcan11:
			return CoreMini::DWCAN11;
		case _icsneo_netid_t::icsneo_netid_dwcan12:
			return CoreMini::DWCAN12;
		case _icsneo_netid_t::icsneo_netid_dwcan13:
			return CoreMini::DWCAN13;
		case _icsneo_netid_t::icsneo_netid_dwcan14:
			return CoreMini::DWCAN14;
		case _icsneo_netid_t::icsneo_netid_dwcan15:
			return CoreMini::DWCAN15;
		case _icsneo_netid_t::icsneo_netid_dwcan16:
			return CoreMini::DWCAN16;
		case _icsneo_netid_t::icsneo_netid_lin7:
			return CoreMini::LIN7;
		case _icsneo_netid_t::icsneo_netid_lin8:
			return CoreMini::LIN8;
		case _icsneo_netid_t::icsneo_netid_lin9:
			return CoreMini::LIN9;
		case _icsneo_netid_t::icsneo_netid_lin10:
			return CoreMini::LIN10;
		case _icsneo_netid_t::icsneo_netid_lin11:
			return CoreMini::LIN11;
		case _icsneo_netid_t::icsneo_netid_lin12:
			return CoreMini::LIN12;
		case _icsneo_netid_t::icsneo_netid_lin13:
			return CoreMini::LIN13;
		case _icsneo_netid_t::icsneo_netid_lin14:
			return CoreMini::LIN14;
		case _icsneo_netid_t::icsneo_netid_lin15:
			return CoreMini::LIN15;
		case _icsneo_netid_t::icsneo_netid_lin16:
			return CoreMini::LIN16;
		case _icsneo_netid_t::icsneo_netid_spi1:
			return CoreMini::SPI1;
		case _icsneo_netid_t::icsneo_netid_spi2:
			return CoreMini::SPI2;
		case _icsneo_netid_t::icsneo_netid_spi3:
			return CoreMini::SPI3;
		case _icsneo_netid_t::icsneo_netid_spi4:
			return CoreMini::SPI4;
		case _icsneo_netid_t::icsneo_netid_spi5:
			return CoreMini::SPI5;
		case _icsneo_netid_t::icsneo_netid_spi6:
			return CoreMini::SPI6;
		case _icsneo_netid_t::icsneo_netid_spi7:
			return CoreMini::SPI7;
		case _icsneo_netid_t::icsneo_netid_spi8:
			return CoreMini::SPI8;
		case _icsneo_netid_t::icsneo_netid_mdio1:
			return CoreMini::MDIO1;
		case _icsneo_netid_t::icsneo_netid_mdio2:
			return CoreMini::MDIO2;
		case _icsneo_netid_t::icsneo_netid_mdio3:
			return CoreMini::MDIO3;
		case _icsneo_netid_t::icsneo_netid_mdio4:
			return CoreMini::MDIO4;
		case _icsneo_netid_t::icsneo_netid_mdio5:
			return CoreMini::MDIO5;
		case _icsneo_netid_t::icsneo_netid_mdio6:
			return CoreMini::MDIO6;
		case _icsneo_netid_t::icsneo_netid_mdio7:
			return CoreMini::MDIO7;
		case _icsneo_netid_t::icsneo_netid_mdio8:
			return CoreMini::MDIO8;
		default:
			return std::nullopt;
		}
	}
	static _icsneo_netid_t GetNetIDFromCoreMiniNetwork(CoreMini cm) {
		switch(cm) {
		case CoreMini::HSCAN:
			return _icsneo_netid_t::icsneo_netid_hscan;
		case CoreMini::MSCAN:
			return _icsneo_netid_t::icsneo_netid_mscan;
		case CoreMini::LIN:
			return _icsneo_netid_t::icsneo_netid_lin;
		case CoreMini::LIN2:
			return _icsneo_netid_t::icsneo_netid_lin2;
		case CoreMini::Virtual:
			return _icsneo_netid_t::icsneo_netid_device;
		case CoreMini::HSCAN2:
			return _icsneo_netid_t::icsneo_netid_hscan2;
		case CoreMini::LSFTCAN:
			return _icsneo_netid_t::icsneo_netid_lsftcan;
		case CoreMini::SWCAN:
			return _icsneo_netid_t::icsneo_netid_swcan;
		case CoreMini::HSCAN3:
			return _icsneo_netid_t::icsneo_netid_hscan3;
		case CoreMini::CGI:
			return _icsneo_netid_t::icsneo_netid_cgi;
		case CoreMini::J1850VPW:
			return _icsneo_netid_t::icsneo_netid_j1850vpw;
		case CoreMini::LIN3:
			return _icsneo_netid_t::icsneo_netid_lin3;
		case CoreMini::LIN4:
			return _icsneo_netid_t::icsneo_netid_lin4;
		case CoreMini::J1708:
			return _icsneo_netid_t::icsneo_netid_j1708;
		case CoreMini::HSCAN4:
			return _icsneo_netid_t::icsneo_netid_hscan4;
		case CoreMini::HSCAN5:
			return _icsneo_netid_t::icsneo_netid_hscan5;
		case CoreMini::KLine1:
			return _icsneo_netid_t::icsneo_netid_iso9141;
		case CoreMini::KLine2:
			return _icsneo_netid_t::icsneo_netid_iso9141_2;
		case CoreMini::KLine3:
			return _icsneo_netid_t::icsneo_netid_iso9141_3;
		case CoreMini::KLine4:
			return _icsneo_netid_t::icsneo_netid_iso9141_4;
		case CoreMini::FlexRay1a:
			return _icsneo_netid_t::icsneo_netid_flexray1a;
		case CoreMini::UART:
			return _icsneo_netid_t::icsneo_netid_uart;
		case CoreMini::UART2:
			return _icsneo_netid_t::icsneo_netid_uart2;
		case CoreMini::LIN5:
			return _icsneo_netid_t::icsneo_netid_lin5;
		case CoreMini::MOST25:
			return _icsneo_netid_t::icsneo_netid_most25;
		case CoreMini::MOST50:
			return _icsneo_netid_t::icsneo_netid_most50;
		case CoreMini::FlexRay1b:
			return _icsneo_netid_t::icsneo_netid_flexray1b;
		case CoreMini::SWCAN2:
			return _icsneo_netid_t::icsneo_netid_swcan2;
		case CoreMini::EthernetDAQ:
			return _icsneo_netid_t::icsneo_netid_ethernet_daq;
		case CoreMini::Ethernet:
			return _icsneo_netid_t::icsneo_netid_ethernet;
		case CoreMini::FlexRay2a:
			return _icsneo_netid_t::icsneo_netid_flexray2a;
		case CoreMini::FlexRay2b:
			return _icsneo_netid_t::icsneo_netid_flexray2b;
		case CoreMini::HSCAN6:
			return _icsneo_netid_t::icsneo_netid_hscan6;
		case CoreMini::HSCAN7:
			return _icsneo_netid_t::icsneo_netid_hscan7;
		case CoreMini::LIN6:
			return _icsneo_netid_t::icsneo_netid_lin6;
		case CoreMini::LSFTCAN2:
			return _icsneo_netid_t::icsneo_netid_lsftcan2;
		case CoreMini::OP_Ethernet1:
			return _icsneo_netid_t::icsneo_netid_op_ethernet1;
		case CoreMini::OP_Ethernet2:
			return _icsneo_netid_t::icsneo_netid_op_ethernet2;
		case CoreMini::OP_Ethernet3:
			return _icsneo_netid_t::icsneo_netid_op_ethernet3;
		case CoreMini::OP_Ethernet4:
			return _icsneo_netid_t::icsneo_netid_op_ethernet4;
		case CoreMini::OP_Ethernet5:
			return _icsneo_netid_t::icsneo_netid_op_ethernet5;
		case CoreMini::OP_Ethernet6:
			return _icsneo_netid_t::icsneo_netid_op_ethernet6;
		case CoreMini::OP_Ethernet7:
			return _icsneo_netid_t::icsneo_netid_op_ethernet7;
		case CoreMini::OP_Ethernet8:
			return _icsneo_netid_t::icsneo_netid_op_ethernet8;
		case CoreMini::OP_Ethernet9:
			return _icsneo_netid_t::icsneo_netid_op_ethernet9;
		case CoreMini::OP_Ethernet10:
			return _icsneo_netid_t::icsneo_netid_op_ethernet10;
		case CoreMini::OP_Ethernet11:
			return _icsneo_netid_t::icsneo_netid_op_ethernet11;
		case CoreMini::OP_Ethernet12:
			return _icsneo_netid_t::icsneo_netid_op_ethernet12;
		case CoreMini::OP_Ethernet13:
			return _icsneo_netid_t::icsneo_netid_op_ethernet13;
		case CoreMini::OP_Ethernet14:
			return _icsneo_netid_t::icsneo_netid_op_ethernet14;
		case CoreMini::OP_Ethernet15:
			return _icsneo_netid_t::icsneo_netid_op_ethernet15;
		case CoreMini::OP_Ethernet16:
			return _icsneo_netid_t::icsneo_netid_op_ethernet16;
		case CoreMini::TCPVirtual:
			return _icsneo_netid_t::icsneo_netid_tcp;
		case CoreMini::UDPVirtual:
			return _icsneo_netid_t::icsneo_netid_udp;
		case CoreMini::FlexRay1:
			return _icsneo_netid_t::icsneo_netid_flexray;
		case CoreMini::FlexRay2:
			return _icsneo_netid_t::icsneo_netid_flexray2;
		case CoreMini::ForwardedMessage:
			return _icsneo_netid_t::icsneo_netid_forwarded_message;
		case CoreMini::I2C1:
			return _icsneo_netid_t::icsneo_netid_i2c;
		case CoreMini::I2C2:
			return _icsneo_netid_t::icsneo_netid_i2c2;
		case CoreMini::I2C3:
			return _icsneo_netid_t::icsneo_netid_i2c3;
		case CoreMini::I2C4:
			return _icsneo_netid_t::icsneo_netid_i2c4;
		case CoreMini::Ethernet2:
			return _icsneo_netid_t::icsneo_netid_ethernet2;
		case CoreMini::A2B1:
			return _icsneo_netid_t::icsneo_netid_a2b1;
		case CoreMini::A2B2:
			return _icsneo_netid_t::icsneo_netid_a2b2;
		case CoreMini::Ethernet3:
			return _icsneo_netid_t::icsneo_netid_ethernet3;
		case CoreMini::WBMS:
			return _icsneo_netid_t::icsneo_netid_wbms;
		case CoreMini::DWCAN9:
			return _icsneo_netid_t::icsneo_netid_dwcan9;
		case CoreMini::DWCAN10:
			return _icsneo_netid_t::icsneo_netid_dwcan10;
		case CoreMini::DWCAN11:
			return _icsneo_netid_t::icsneo_netid_dwcan11;
		case CoreMini::DWCAN12:
			return _icsneo_netid_t::icsneo_netid_dwcan12;
		case CoreMini::DWCAN13:
			return _icsneo_netid_t::icsneo_netid_dwcan13;
		case CoreMini::DWCAN14:
			return _icsneo_netid_t::icsneo_netid_dwcan14;
		case CoreMini::DWCAN15:
			return _icsneo_netid_t::icsneo_netid_dwcan15;
		case CoreMini::DWCAN16:
			return _icsneo_netid_t::icsneo_netid_dwcan16;
		case CoreMini::LIN7:
			return _icsneo_netid_t::icsneo_netid_lin7;
		case CoreMini::LIN8:
			return _icsneo_netid_t::icsneo_netid_lin8;
		case CoreMini::LIN9:
			return _icsneo_netid_t::icsneo_netid_lin9;
		case CoreMini::LIN10:
			return _icsneo_netid_t::icsneo_netid_lin10;
		case CoreMini::LIN11:
			return _icsneo_netid_t::icsneo_netid_lin11;
		case CoreMini::LIN12:
			return _icsneo_netid_t::icsneo_netid_lin12;
		case CoreMini::LIN13:
			return _icsneo_netid_t::icsneo_netid_lin13;
		case CoreMini::LIN14:
			return _icsneo_netid_t::icsneo_netid_lin14;
		case CoreMini::LIN15:
			return _icsneo_netid_t::icsneo_netid_lin15;
		case CoreMini::LIN16:
			return _icsneo_netid_t::icsneo_netid_lin16;
		case CoreMini::SPI1:
			return _icsneo_netid_t::icsneo_netid_spi1;
		case CoreMini::SPI2:
			return _icsneo_netid_t::icsneo_netid_spi2;
		case CoreMini::SPI3:
			return _icsneo_netid_t::icsneo_netid_spi3;
		case CoreMini::SPI4:
			return _icsneo_netid_t::icsneo_netid_spi4;
		case CoreMini::SPI5:
			return _icsneo_netid_t::icsneo_netid_spi5;
		case CoreMini::SPI6:
			return _icsneo_netid_t::icsneo_netid_spi6;
		case CoreMini::SPI7:
			return _icsneo_netid_t::icsneo_netid_spi7;
		case CoreMini::SPI8:
			return _icsneo_netid_t::icsneo_netid_spi8;
		case CoreMini::MDIO1:
			return _icsneo_netid_t::icsneo_netid_mdio1;
		case CoreMini::MDIO2:
			return _icsneo_netid_t::icsneo_netid_mdio2;
		case CoreMini::MDIO3:
			return _icsneo_netid_t::icsneo_netid_mdio3;
		case CoreMini::MDIO4:
			return _icsneo_netid_t::icsneo_netid_mdio4;
		case CoreMini::MDIO5:
			return _icsneo_netid_t::icsneo_netid_mdio5;
		case CoreMini::MDIO6:
			return _icsneo_netid_t::icsneo_netid_mdio6;
		case CoreMini::MDIO7:
			return _icsneo_netid_t::icsneo_netid_mdio7;
		case CoreMini::MDIO8:
			return _icsneo_netid_t::icsneo_netid_mdio8;

		}
		return _icsneo_netid_t::icsneo_netid_invalid; // Should be unreachable, the compiler should warn about new CoreMini IDs
	}

	Network() { setValue(_icsneo_netid_t::icsneo_netid_invalid); }
	Network(icsneo_netid_t netid, bool expand = true) { setValue(static_cast<_icsneo_netid_t>(netid), expand); }
	Network(_icsneo_netid_t netid) { setValue(netid); }
	Network(CoreMini cm) { setValue(GetNetIDFromCoreMiniNetwork(cm)); }
	_icsneo_netid_t getNetID() const { return value; }
	icsneo_msg_bus_type_t getType() const { return type; }
	VnetId getVnetId() const { return vnetId; }
	std::optional<CoreMini> getCoreMini() const { return GetCoreMiniNetworkFromNetID(getNetID()); }
	friend std::ostream& operator<<(std::ostream& os, const Network& network) {
		os << GetNetIDString(network.getNetID());
		return os;
	}
	friend bool operator==(const Network& net1, const Network& net2) { return net1.getNetID() == net2.getNetID(); }
	friend bool operator!=(const Network& net1, const Network& net2) { return !(net1 == net2); }

private:
	_icsneo_netid_t value; // Always use setValue so that value and type stay in sync
	icsneo_msg_bus_type_t type;
	_icsneo_netid_t commonNetId;
	VnetId vnetId;
	void setValue(_icsneo_netid_t id, bool expand = true) {
		value = id;
		// Constructor can optionally suppress the expansion of of the ID into a VNET slot and common ID. The communication decoder and packetizer require this for device responses.
		type = GetTypeOfNetID(value, expand);
		if(expand) {
			std::tie(vnetId, commonNetId) = GetVnetAgnosticNetid((icsneo_netid_t)id);
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
#define ICSNEO_NETID_HSCAN 1
#define ICSNEO_NETID_MSCAN 2
#define ICSNEO_NETID_SWCAN 3
#define ICSNEO_NETID_LSFTCAN 4
#define ICSNEO_NETID_FORDSCP 5
#define ICSNEO_NETID_J1708 6
#define ICSNEO_NETID_AUX 7
#define ICSNEO_NETID_J1850VPW 8
#define ICSNEO_NETID_ISO9141 9
#define ICSNEO_NETID_DISK_DATA 10
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
