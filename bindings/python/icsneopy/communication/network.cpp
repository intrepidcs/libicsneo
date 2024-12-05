#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/network.h"

namespace icsneo {

void init_network(pybind11::module_& m) {
	pybind11::class_<Network> network(m, "Network");

	pybind11::enum_<Network::_icsneo_netid_t>(network, "_icsneo_netid_t")
		.value("Device", Network::_icsneo_netid_t::icsneo_netid_device)
		.value("HSCAN", Network::_icsneo_netid_t::icsneo_netid_hscan)
		.value("MSCAN", Network::_icsneo_netid_t::icsneo_netid_mscan)
		.value("SWCAN", Network::_icsneo_netid_t::icsneo_netid_swcan)
		.value("LSFTCAN", Network::_icsneo_netid_t::icsneo_netid_lsftcan)
		.value("FordSCP", Network::_icsneo_netid_t::icsneo_netid_fordscp)
		.value("J1708", Network::_icsneo_netid_t::icsneo_netid_j1708)
		.value("Aux", Network::_icsneo_netid_t::icsneo_netid_aux)
		.value("J1850VPW", Network::_icsneo_netid_t::icsneo_netid_j1850vpw)
		.value("ISO9141", Network::_icsneo_netid_t::icsneo_netid_iso9141)
		.value("DiskData", Network::_icsneo_netid_t::icsneo_netid_disk_data)
		.value("Main51", Network::_icsneo_netid_t::icsneo_netid_main51)
		.value("RED", Network::_icsneo_netid_t::icsneo_netid_red)
		.value("SCI", Network::_icsneo_netid_t::icsneo_netid_sci)
		.value("ISO9141_2", Network::_icsneo_netid_t::icsneo_netid_iso9141_2)
		.value("ISO14230", Network::_icsneo_netid_t::icsneo_netid_iso14230)
		.value("LIN", Network::_icsneo_netid_t::icsneo_netid_lin)
		.value("OP_Ethernet1", Network::_icsneo_netid_t::icsneo_netid_op_ethernet1)
		.value("OP_Ethernet2", Network::_icsneo_netid_t::icsneo_netid_op_ethernet2)
		.value("OP_Ethernet3", Network::_icsneo_netid_t::icsneo_netid_op_ethernet3)
		.value("RED_EXT_MEMORYREAD", Network::_icsneo_netid_t::icsneo_netid_red_ext_memoryread)
		.value("RED_INT_MEMORYREAD", Network::_icsneo_netid_t::icsneo_netid_red_int_memoryread)
		.value("RED_DFLASH_READ", Network::_icsneo_netid_t::icsneo_netid_red_dflash_read)
		.value("NeoMemorySDRead", Network::_icsneo_netid_t::icsneo_netid_neo_memory_sdread)
		.value("CAN_ERRBITS", Network::_icsneo_netid_t::icsneo_netid_can_errbits)
		.value("NeoMemoryWriteDone", Network::_icsneo_netid_t::icsneo_netid_neo_memory_write_done)
		.value("RED_WAVE_CAN1_LOGICAL", Network::_icsneo_netid_t::icsneo_netid_red_wave_can1_logical)
		.value("RED_WAVE_CAN2_LOGICAL", Network::_icsneo_netid_t::icsneo_netid_red_wave_can2_logical)
		.value("RED_WAVE_LIN1_LOGICAL", Network::_icsneo_netid_t::icsneo_netid_red_wave_lin1_logical)
		.value("RED_WAVE_LIN2_LOGICAL", Network::_icsneo_netid_t::icsneo_netid_red_wave_lin2_logical)
		.value("RED_WAVE_LIN1_ANALOG", Network::_icsneo_netid_t::icsneo_netid_red_wave_lin1_analog)
		.value("RED_WAVE_LIN2_ANALOG", Network::_icsneo_netid_t::icsneo_netid_red_wave_lin2_analog)
		.value("RED_WAVE_MISC_ANALOG", Network::_icsneo_netid_t::icsneo_netid_red_wave_misc_analog)
		.value("RED_WAVE_MISCDIO2_LOGICAL", Network::_icsneo_netid_t::icsneo_netid_red_wave_miscdio2_logical)
		.value("RED_NETWORK_COM_ENABLE_EX", Network::_icsneo_netid_t::icsneo_netid_red_network_com_enable_ex)
		.value("RED_NEOVI_NETWORK", Network::_icsneo_netid_t::icsneo_netid_red_neovi_network)
		.value("RED_READ_BAUD_SETTINGS", Network::_icsneo_netid_t::icsneo_netid_red_read_baud_settings)
		.value("RED_OLDFORMAT", Network::_icsneo_netid_t::icsneo_netid_red_oldformat)
		.value("RED_SCOPE_CAPTURE", Network::_icsneo_netid_t::icsneo_netid_red_scope_capture)
		.value("RED_HARDWARE_EXCEP", Network::_icsneo_netid_t::icsneo_netid_red_hardware_excep)
		.value("RED_GET_RTC", Network::_icsneo_netid_t::icsneo_netid_red_get_rtc)
		.value("ISO9141_3", Network::_icsneo_netid_t::icsneo_netid_iso9141_3)
		.value("HSCAN2", Network::_icsneo_netid_t::icsneo_netid_hscan2)
		.value("HSCAN3", Network::_icsneo_netid_t::icsneo_netid_hscan3)
		.value("OP_Ethernet4", Network::_icsneo_netid_t::icsneo_netid_op_ethernet4)
		.value("OP_Ethernet5", Network::_icsneo_netid_t::icsneo_netid_op_ethernet5)
		.value("ISO9141_4", Network::_icsneo_netid_t::icsneo_netid_iso9141_4)
		.value("LIN2", Network::_icsneo_netid_t::icsneo_netid_lin2)
		.value("LIN3", Network::_icsneo_netid_t::icsneo_netid_lin3)
		.value("LIN4", Network::_icsneo_netid_t::icsneo_netid_lin4)
		.value("RED_App_Error", Network::_icsneo_netid_t::icsneo_netid_red_app_error)
		.value("CGI", Network::_icsneo_netid_t::icsneo_netid_cgi)
		.value("Reset_Status", Network::_icsneo_netid_t::icsneo_netid_reset_status)
		.value("FB_Status", Network::_icsneo_netid_t::icsneo_netid_fb_status)
		.value("App_Signal_Status", Network::_icsneo_netid_t::icsneo_netid_app_signal_status)
		.value("Read_Datalink_Cm_Tx_Msg", Network::_icsneo_netid_t::icsneo_netid_read_datalink_cm_tx_msg)
		.value("Read_Datalink_Cm_Rx_Msg", Network::_icsneo_netid_t::icsneo_netid_read_datalink_cm_rx_msg)
		.value("Logging_Overflow", Network::_icsneo_netid_t::icsneo_netid_logging_overflow)
		.value("ReadSettings", Network::_icsneo_netid_t::icsneo_netid_read_settings)
		.value("HSCAN4", Network::_icsneo_netid_t::icsneo_netid_hscan4)
		.value("HSCAN5", Network::_icsneo_netid_t::icsneo_netid_hscan5)
		.value("RS232", Network::_icsneo_netid_t::icsneo_netid_rs232)
		.value("UART", Network::_icsneo_netid_t::icsneo_netid_uart)
		.value("UART2", Network::_icsneo_netid_t::icsneo_netid_uart2)
		.value("UART3", Network::_icsneo_netid_t::icsneo_netid_uart3)
		.value("UART4", Network::_icsneo_netid_t::icsneo_netid_uart4)
		.value("SWCAN2", Network::_icsneo_netid_t::icsneo_netid_swcan2)
		.value("Ethernet_DAQ", Network::_icsneo_netid_t::icsneo_netid_ethernet_daq)
		.value("Data_To_Host", Network::_icsneo_netid_t::icsneo_netid_data_to_host)
		.value("TextAPI_To_Host", Network::_icsneo_netid_t::icsneo_netid_textapi_to_host)
		.value("SPI1", Network::_icsneo_netid_t::icsneo_netid_spi1)
		.value("OP_Ethernet6", Network::_icsneo_netid_t::icsneo_netid_op_ethernet6)
		.value("Red_VBat", Network::_icsneo_netid_t::icsneo_netid_red_vbat)
		.value("OP_Ethernet7", Network::_icsneo_netid_t::icsneo_netid_op_ethernet7)
		.value("OP_Ethernet8", Network::_icsneo_netid_t::icsneo_netid_op_ethernet8)
		.value("OP_Ethernet9", Network::_icsneo_netid_t::icsneo_netid_op_ethernet9)
		.value("OP_Ethernet10", Network::_icsneo_netid_t::icsneo_netid_op_ethernet10)
		.value("OP_Ethernet11", Network::_icsneo_netid_t::icsneo_netid_op_ethernet11)
		.value("FlexRay1a", Network::_icsneo_netid_t::icsneo_netid_flexray1a)
		.value("FlexRay1b", Network::_icsneo_netid_t::icsneo_netid_flexray1b)
		.value("FlexRay2a", Network::_icsneo_netid_t::icsneo_netid_flexray2a)
		.value("FlexRay2b", Network::_icsneo_netid_t::icsneo_netid_flexray2b)
		.value("LIN5", Network::_icsneo_netid_t::icsneo_netid_lin5)
		.value("FlexRay", Network::_icsneo_netid_t::icsneo_netid_flexray)
		.value("FlexRay2", Network::_icsneo_netid_t::icsneo_netid_flexray2)
		.value("OP_Ethernet12", Network::_icsneo_netid_t::icsneo_netid_op_ethernet12)
		.value("I2C", Network::_icsneo_netid_t::icsneo_netid_i2c)
		.value("MOST25", Network::_icsneo_netid_t::icsneo_netid_most25)
		.value("MOST50", Network::_icsneo_netid_t::icsneo_netid_most50)
		.value("MOST150", Network::_icsneo_netid_t::icsneo_netid_most150)
		.value("Ethernet", Network::_icsneo_netid_t::icsneo_netid_ethernet)
		.value("GMFSA", Network::_icsneo_netid_t::icsneo_netid_gmfsa)
		.value("TCP", Network::_icsneo_netid_t::icsneo_netid_tcp)
		.value("HSCAN6", Network::_icsneo_netid_t::icsneo_netid_hscan6)
		.value("HSCAN7", Network::_icsneo_netid_t::icsneo_netid_hscan7)
		.value("LIN6", Network::_icsneo_netid_t::icsneo_netid_lin6)
		.value("LSFTCAN2", Network::_icsneo_netid_t::icsneo_netid_lsftcan2)
		.value("LogicalDiskInfo", Network::_icsneo_netid_t::icsneo_netid_logical_disk_info)
		.value("WiVICommand", Network::_icsneo_netid_t::icsneo_netid_wivi_command)
		.value("ScriptStatus", Network::_icsneo_netid_t::icsneo_netid_script_status)
		.value("EthPHYControl", Network::_icsneo_netid_t::icsneo_netid_eth_phy_control)
		.value("ExtendedCommand", Network::_icsneo_netid_t::icsneo_netid_extended_command)
		.value("ExtendedData", Network::_icsneo_netid_t::icsneo_netid_extended_data)
		.value("FlexRayControl", Network::_icsneo_netid_t::icsneo_netid_flexray_control)
		.value("CoreMiniPreLoad", Network::_icsneo_netid_t::icsneo_netid_coremini_preload)
		.value("HW_COM_Latency_Test", Network::_icsneo_netid_t::icsneo_netid_hw_com_latency_test)
		.value("DeviceStatus", Network::_icsneo_netid_t::icsneo_netid_device_status)
		.value("UDP", Network::_icsneo_netid_t::icsneo_netid_udp)
		.value("ForwardedMessage", Network::_icsneo_netid_t::icsneo_netid_forwarded_message)
		.value("I2C2", Network::_icsneo_netid_t::icsneo_netid_i2c2)
		.value("I2C3", Network::_icsneo_netid_t::icsneo_netid_i2c3)
		.value("I2C4", Network::_icsneo_netid_t::icsneo_netid_i2c4)
		.value("Ethernet2", Network::_icsneo_netid_t::icsneo_netid_ethernet2)
		.value("A2B1", Network::_icsneo_netid_t::A2B1)
		.value("A2B2", Network::_icsneo_netid_t::A2B2)
		.value("Ethernet3", Network::_icsneo_netid_t::Ethernet3)
		.value("WBMS", Network::_icsneo_netid_t::WBMS)
		.value("DWCAN9", Network::_icsneo_netid_t::DWCAN9)
		.value("DWCAN10", Network::_icsneo_netid_t::DWCAN10)
		.value("DWCAN11", Network::_icsneo_netid_t::DWCAN11)
		.value("DWCAN12", Network::_icsneo_netid_t::DWCAN12)
		.value("DWCAN13", Network::_icsneo_netid_t::DWCAN13)
		.value("DWCAN14", Network::_icsneo_netid_t::DWCAN14)
		.value("DWCAN15", Network::_icsneo_netid_t::DWCAN15)
		.value("DWCAN16", Network::_icsneo_netid_t::DWCAN16)
		.value("LIN7", Network::_icsneo_netid_t::LIN7)
		.value("LIN8", Network::_icsneo_netid_t::LIN8)
		.value("SPI2", Network::_icsneo_netid_t::SPI2)
		.value("MDIO1", Network::_icsneo_netid_t::MDIO1)
		.value("MDIO2", Network::_icsneo_netid_t::MDIO2)
		.value("MDIO3", Network::_icsneo_netid_t::MDIO3)
		.value("MDIO4", Network::_icsneo_netid_t::MDIO4)
		.value("MDIO5", Network::_icsneo_netid_t::MDIO5)
		.value("MDIO6", Network::_icsneo_netid_t::MDIO6)
		.value("MDIO7", Network::_icsneo_netid_t::MDIO7)
		.value("MDIO8", Network::_icsneo_netid_t::MDIO8)
		.value("OP_Ethernet13", Network::_icsneo_netid_t::OP_Ethernet13)
		.value("OP_Ethernet14", Network::_icsneo_netid_t::OP_Ethernet14)
		.value("OP_Ethernet15", Network::_icsneo_netid_t::OP_Ethernet15)
		.value("OP_Ethernet16", Network::_icsneo_netid_t::OP_Ethernet16)
		.value("SPI3", Network::_icsneo_netid_t::SPI3)
		.value("SPI4", Network::_icsneo_netid_t::SPI4)
		.value("SPI5", Network::_icsneo_netid_t::SPI5)
		.value("SPI6", Network::_icsneo_netid_t::SPI6)
		.value("SPI7", Network::_icsneo_netid_t::SPI7)
		.value("SPI8", Network::_icsneo_netid_t::SPI8)
		.value("LIN9", Network::_icsneo_netid_t::LIN9)
		.value("LIN10", Network::_icsneo_netid_t::LIN10)
		.value("LIN11", Network::_icsneo_netid_t::LIN11)
		.value("LIN12", Network::_icsneo_netid_t::LIN12)
		.value("LIN13", Network::_icsneo_netid_t::LIN13)
		.value("LIN14", Network::_icsneo_netid_t::LIN14)
		.value("LIN15", Network::_icsneo_netid_t::LIN15)
		.value("LIN16", Network::_icsneo_netid_t::LIN16)
		.value("Any", Network::_icsneo_netid_t::Any)
		.value("Invalid", Network::_icsneo_netid_t::Invalid);
	
    network.def(pybind11::init<Network::_icsneo_netid_t>());
}

} // namespace icsneo 

