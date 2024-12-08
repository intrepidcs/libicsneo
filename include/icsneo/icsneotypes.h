#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** @brief
 * Options for opening a device. See icsneo_device_open() for more info.
 */
typedef enum _icsneo_open_options_t {
    // No options
    icsneo_open_options_none = 0x0,
    // After opening, go online
    icsneo_open_options_go_online = 0x1,
    // After opening, enable message polling
    icsneo_open_options_enable_message_polling = 0x2,
    // After opening, sync RTC
    icsneo_open_options_sync_rtc = 0x4,
    // After opening, enable auto update
    icsneo_open_options_enable_auto_update = 0x8,
    // After opening, force update
    icsneo_open_options_force_update = 0x10,
} _icsneo_open_options_t;

/** @brief Integer representation of _icsneo_open_options_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint32_t icsneo_open_options_t;


/** @brief
 * Intrepid hardware device types, useful for filtering out or identifying devices.
*/
typedef enum _icsneo_devicetype_t {
    // Unknown device type
    icsneo_devicetype_unknown,
    // neoVI Blue - Obsolete
    icsneo_devicetype_blue,
    // neoECU AVB/TSN
    icsneo_devicetype_ecu_avb,
    // RAD-SuperMoon
    icsneo_devicetype_rad_supermoon,
    // DualWire ValueCAN - Obsolete
    icsneo_devicetype_dw_vcan,
    // RAD-Moon 2
    icsneo_devicetype_rad_moon2,
    // RAD-Mars
    icsneo_devicetype_rad_mars,
    // ValueCAN 4-1
    icsneo_devicetype_vcan41,
    // neoVI FIRE
    icsneo_devicetype_fire,
    // RAD-Pluto
    icsneo_devicetype_rad_pluto,
    // ValueCAN 4-2EL
    icsneo_devicetype_vcan42_el,
    // RAD-IO CAN-HUB
    icsneo_devicetype_radio_canhub,
    // neoECU12
    icsneo_devicetype_neo_ecu12,
    // neoOBD2-LC Badge
    icsneo_devicetype_obd2_lc_badge,
    // RAD-Moon Duo
    icsneo_devicetype_rad_moon_duo,
    // neoVI FIRE3
    icsneo_devicetype_fire3,
    // ValueCAN3
    icsneo_devicetype_vcan3,
    // RAD-Jupiter
    icsneo_devicetype_rad_jupiter,
    // ValueCAN4 Industrial
    icsneo_devicetype_vcan4_industrial,
    // RAD-Gigastar
    icsneo_devicetype_rad_gigastar,
    // neoVI RED2
    icsneo_devicetype_red2,
    // EtherBADGE
    icsneo_devicetype_etherbadge,
    // RAD-A2B
    icsneo_devicetype_rad_a2b,
    // RAD-Epsilon
    icsneo_devicetype_rad_epsilon,
    // RAD-Moon 3
    icsneo_devicetype_rad_moon3,
    // RAD-Comet
    icsneo_devicetype_rad_comet,
    // neoVI FIRE3 FlexRay
    icsneo_devicetype_fire3_flexray,
    // neoVI CONNECT
    icsneo_devicetype_connect,
    // RAD-Comet 3
    icsneo_devicetype_rad_comet3,
    // RAD-Moon T1S
    icsneo_devicetype_rad_moon_t1s,
    // RAD-Gigastar 2
    icsneo_devicetype_rad_gigastar2,
    // neoVI RED
    icsneo_devicetype_red,
    // neoECU - Obsolete
    icsneo_devicetype_ecu,
    // IEVB - Obsolete
    icsneo_devicetype_ievb,
    // Pendant - Obsolete
    icsneo_devicetype_pendant,
    // neoOBD2 Pro - Obsolete
    icsneo_devicetype_obd2_pro,
    // neoECU Chip - Obsolete
    icsneo_devicetype_ecuchip_uart,
    // neoVI PLASMA
    icsneo_devicetype_plasma,
    // neoAnalog - Obsolete
    icsneo_devicetype_neo_analog,
    // Obsolete
    icsneo_devicetype_ct_obd,
    // neoVI ION
    icsneo_devicetype_ion,
    // RAD-Star - Obsolete
    icsneo_devicetype_rad_star,
    // ValueCAN4-4
    icsneo_devicetype_vcan44,
    // ValueCAN4-2
    icsneo_devicetype_vcan42,
    // CMProbe - Obsolete
    icsneo_devicetype_cm_probe,
    // Ethernet EVB - Obsolete
    icsneo_devicetype_eevb,
    // ValueCAN.rf - Obsolete
    icsneo_devicetype_vcan_rf,
    // neoVI FIRE2
    icsneo_devicetype_fire2,
    // neoVI FLEX - Obsolete
    icsneo_devicetype_flex,
    // RAD-Galaxy
    icsneo_devicetype_rad_galaxy,
    // RAD-Star 2
    icsneo_devicetype_rad_star2,
    // VividCAN
    icsneo_devicetype_vividcan,
    // neoOBD2 SIM
    icsneo_devicetype_obd2_sim,


    // Must be last entry
    icsneo_devicetype_maxsize,
} _icsneo_devicetype_t;

/** @brief Integer representation of _icsneo_devicetype_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint32_t icsneo_devicetype_t;

typedef enum _icsneo_msg_type_t {
    icsneo_msg_type_device,
    icsneo_msg_type_internal,
    icsneo_msg_type_bus,

    icsneo_msg_type_maxsize,
} _icsneo_msg_type_t;

/** @brief Integer representation of _icsneo_msg_type_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint32_t icsneo_msg_type_t;


/** @brief
 * Bus message types, useful for filtering out or identifying Bus Messages.
*/
typedef enum _icsneo_msg_bus_type_t {
    icsneo_msg_bus_type_invalid = 0,
    icsneo_msg_bus_type_internal = 1, // Used for statuses that don't actually need to be transferred to the client application
    icsneo_msg_bus_type_can = 2,
    icsneo_msg_bus_type_lin = 3,
    icsneo_msg_bus_type_flexray = 4,
    icsneo_msg_bus_type_most = 5,
    icsneo_msg_bus_type_ethernet = 6,
    icsneo_msg_bus_type_lsftcan = 7,
    icsneo_msg_bus_type_swcan = 8,
    icsneo_msg_bus_type_iso9141 = 9,
    icsneo_msg_bus_type_i2c = 10,
    icsneo_msg_bus_type_a2b = 11,
    icsneo_msg_bus_type_spi = 12,
    icsneo_msg_bus_type_mdio = 13,
    icsneo_msg_bus_type_any = 0xFE, // Never actually set as type, but used as flag for filtering
    icsneo_msg_bus_type_other = 0xFF
} _icsneo_msg_bus_type_t;

/** @brief Integer representation of _icsneo_msg_bus_type_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint8_t icsneo_msg_bus_type_t;

typedef enum _icsneo_netid_t {
    icsneo_netid_device = 0,
    icsneo_netid_hscan = 1,
    icsneo_netid_mscan = 2,
    icsneo_netid_swcan = 3,
    icsneo_netid_lsftcan = 4,
    icsneo_netid_fordscp = 5,
    icsneo_netid_j1708 = 6,
    icsneo_netid_aux = 7,
    icsneo_netid_j1850vpw = 8,
    icsneo_netid_iso9141 = 9,
    icsneo_netid_disk_data = 10,
    icsneo_netid_main51 = 11,
    icsneo_netid_red = 12,
    icsneo_netid_sci = 13,
    icsneo_netid_iso9141_2 = 14,
    icsneo_netid_iso14230 = 15,
    icsneo_netid_lin = 16,
    icsneo_netid_op_ethernet1 = 17,
    icsneo_netid_op_ethernet2 = 18,
    icsneo_netid_op_ethernet3 = 19,

    // START Device Command Returns
    // When we send a command, the device returns on one of these, depending on command
    icsneo_netid_red_ext_memoryread = 20,
    icsneo_netid_red_int_memoryread = 21,
    icsneo_netid_red_dflash_read = 22,
    icsneo_netid_neo_memory_sdread = 23, // Response from NeoMemory (MemoryTypeSD)
    icsneo_netid_can_errbits = 24,
    icsneo_netid_neo_memory_write_done = 25,
    icsneo_netid_red_wave_can1_logical = 26,
    icsneo_netid_red_wave_can2_logical = 27,
    icsneo_netid_red_wave_lin1_logical = 28,
    icsneo_netid_red_wave_lin2_logical = 29,
    icsneo_netid_red_wave_lin1_analog = 30,
    icsneo_netid_red_wave_lin2_analog = 31,
    icsneo_netid_red_wave_misc_analog = 32,
    icsneo_netid_red_wave_miscdio2_logical = 33,
    icsneo_netid_red_network_com_enable_ex = 34,
    icsneo_netid_red_neovi_network = 35,
    icsneo_netid_red_read_baud_settings = 36,
    icsneo_netid_red_oldformat = 37,
    icsneo_netid_red_scope_capture = 38,
    icsneo_netid_red_hardware_excep = 39,
    icsneo_netid_red_get_rtc = 40,
    // END Device Command Returns

    icsneo_netid_iso9141_3 = 41,
    icsneo_netid_hscan2 = 42,
    icsneo_netid_hscan3 = 44,
    icsneo_netid_op_ethernet4 = 45,
    icsneo_netid_op_ethernet5 = 46,
    icsneo_netid_iso9141_4 = 47,
    icsneo_netid_lin2 = 48,
    icsneo_netid_lin3 = 49,
    icsneo_netid_lin4 = 50,
    // MOST = 51, Old and unused
    icsneo_netid_red_app_error = 52,
    icsneo_netid_cgi = 53,
    icsneo_netid_reset_status = 54,
    icsneo_netid_fb_status = 55,
    icsneo_netid_app_signal_status = 56,
    icsneo_netid_read_datalink_cm_tx_msg = 57,
    icsneo_netid_read_datalink_cm_rx_msg = 58,
    icsneo_netid_logging_overflow = 59,
    icsneo_netid_read_settings = 60,
    icsneo_netid_hscan4 = 61,
    icsneo_netid_hscan5 = 62,
    icsneo_netid_rs232 = 63,
    icsneo_netid_uart = 64,
    icsneo_netid_uart2 = 65,
    icsneo_netid_uart3 = 66,
    icsneo_netid_uart4 = 67,
    icsneo_netid_swcan2 = 68,
    icsneo_netid_ethernet_daq = 69,
    icsneo_netid_data_to_host = 70,
    icsneo_netid_textapi_to_host = 71,
    icsneo_netid_spi1 = 72,
    icsneo_netid_op_ethernet6 = 73,
    icsneo_netid_red_vbat = 74,
    icsneo_netid_op_ethernet7 = 75,
    icsneo_netid_op_ethernet8 = 76,
    icsneo_netid_op_ethernet9 = 77,
    icsneo_netid_op_ethernet10 = 78,
    icsneo_netid_op_ethernet11 = 79,
    icsneo_netid_flexray1a = 80,
    icsneo_netid_flexray1b = 81,
    icsneo_netid_flexray2a = 82,
    icsneo_netid_flexray2b = 83,
    icsneo_netid_lin5 = 84,
    icsneo_netid_flexray = 85,
    icsneo_netid_flexray2 = 86,
    icsneo_netid_op_ethernet12 = 87,
    icsneo_netid_i2c = 88,
    icsneo_netid_most25 = 90,
    icsneo_netid_most50 = 91,
    icsneo_netid_most150 = 92,
    icsneo_netid_ethernet = 93,
    icsneo_netid_gmfsa = 94,
    icsneo_netid_tcp = 95,
    icsneo_netid_hscan6 = 96,
    icsneo_netid_hscan7 = 97,
    icsneo_netid_lin6 = 98,
    icsneo_netid_lsftcan2 = 99,
    icsneo_netid_logical_disk_info = 187,
    icsneo_netid_wivi_command = 221,
    icsneo_netid_script_status = 224,
    icsneo_netid_eth_phy_control = 239,
    icsneo_netid_extended_command = 240,
    icsneo_netid_extended_data = 242,
    icsneo_netid_flexray_control = 243,
    icsneo_netid_coremini_preload = 244,
    icsneo_netid_hw_com_latency_test = 512,
    icsneo_netid_device_status = 513,
    icsneo_netid_udp = 514,
    icsneo_netid_forwarded_message = 516,
    icsneo_netid_i2c2 = 517,
    icsneo_netid_i2c3 = 518,
    icsneo_netid_i2c4 = 519,
    icsneo_netid_ethernet2 = 520,
    icsneo_netid_a2b1 = 522,
    icsneo_netid_a2b2 = 523,
    icsneo_netid_ethernet3 = 524,
    icsneo_netid_wbms = 532,
    icsneo_netid_dwcan9 = 534,
    icsneo_netid_dwcan10 = 535,
    icsneo_netid_dwcan11 = 536,
    icsneo_netid_dwcan12 = 537,
    icsneo_netid_dwcan13 = 538,
    icsneo_netid_dwcan14 = 539,
    icsneo_netid_dwcan15 = 540,
    icsneo_netid_dwcan16 = 541,
    icsneo_netid_lin7 = 542,
    icsneo_netid_lin8 = 543,
    icsneo_netid_spi2 = 544,
    icsneo_netid_mdio1 = 545,
    icsneo_netid_mdio2 = 546,
    icsneo_netid_mdio3 = 547,
    icsneo_netid_mdio4 = 548,
    icsneo_netid_mdio5 = 549,
    icsneo_netid_mdio6 = 550,
    icsneo_netid_mdio7 = 551,
    icsneo_netid_mdio8 = 552,
    icsneo_netid_op_ethernet13 = 553,
    icsneo_netid_op_ethernet14 = 554,
    icsneo_netid_op_ethernet15 = 555,
    icsneo_netid_op_ethernet16 = 556,
    icsneo_netid_spi3 = 557,
    icsneo_netid_spi4 = 558,
    icsneo_netid_spi5 = 559,
    icsneo_netid_spi6 = 560,
    icsneo_netid_spi7 = 561,
    icsneo_netid_spi8 = 562,
    icsneo_netid_lin9 = 563,
    icsneo_netid_lin10 = 564,
    icsneo_netid_lin11 = 565,
    icsneo_netid_lin12 = 566,
    icsneo_netid_lin13 = 567,
    icsneo_netid_lin14 = 568,
    icsneo_netid_lin15 = 569,
    icsneo_netid_lin16 = 570,
    icsneo_netid_any = 0xfffe, // Never actually set as type, but used as flag for filtering
    icsneo_netid_invalid = 0xffff
} _icsneo_netid_t;

/** @brief Integer representation of _icsneo_netid_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint16_t icsneo_netid_t;

#ifdef __cplusplus
}
#endif
