#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** @brief
 * Options for opening a device. See icsneoc2_device_open() for more info.
 */
typedef enum _icsneoc2_open_options_t {
    // No options
    icsneoc2_open_options_none = 0x0,
    // After opening, go online
    icsneoc2_open_options_go_online = 0x1,
    // After opening, enable message polling
    icsneoc2_open_options_enable_message_polling = 0x2,
    // After opening, sync RTC
    icsneoc2_open_options_sync_rtc = 0x4,
    // After opening, enable auto update
    icsneoc2_open_options_enable_auto_update = 0x8,
    // After opening, force update
    icsneoc2_open_options_force_update = 0x10,
} _icsneoc2_open_options_t;

/** @brief Integer representation of _icsneoc2_open_options_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint32_t icsneoc2_open_options_t;


/** @brief
 * Intrepid hardware device types, useful for filtering out or identifying devices.
*/
typedef enum _icsneoc2_devicetype_t {
    // Unknown device type
    icsneoc2_devicetype_unknown,
    // neoVI Blue - Obsolete
    icsneoc2_devicetype_blue,
    // neoECU AVB/TSN
    icsneoc2_devicetype_ecu_avb,
    // RAD-SuperMoon
    icsneoc2_devicetype_rad_supermoon,
    // DualWire ValueCAN - Obsolete
    icsneoc2_devicetype_dw_vcan,
    // RAD-Moon 2
    icsneoc2_devicetype_rad_moon2,
    // RAD-Mars
    icsneoc2_devicetype_rad_mars,
    // ValueCAN 4-1
    icsneoc2_devicetype_vcan41,
    // neoVI FIRE
    icsneoc2_devicetype_fire,
    // RAD-Pluto
    icsneoc2_devicetype_rad_pluto,
    // ValueCAN 4-2EL
    icsneoc2_devicetype_vcan42_el,
    // RAD-IO CAN-HUB
    icsneoc2_devicetype_radio_canhub,
    // neoECU12
    icsneoc2_devicetype_neo_ecu12,
    // neoOBD2-LC Badge
    icsneoc2_devicetype_obd2_lc_badge,
    // RAD-Moon Duo
    icsneoc2_devicetype_rad_moon_duo,
    // neoVI FIRE3
    icsneoc2_devicetype_fire3,
    // ValueCAN3
    icsneoc2_devicetype_vcan3,
    // RAD-Jupiter
    icsneoc2_devicetype_rad_jupiter,
    // ValueCAN4 Industrial
    icsneoc2_devicetype_vcan4_industrial,
    // RAD-Gigastar
    icsneoc2_devicetype_rad_gigastar,
    // neoVI RED2
    icsneoc2_devicetype_red2,
    // EtherBADGE
    icsneoc2_devicetype_etherbadge,
    // RAD-A2B
    icsneoc2_devicetype_rad_a2b,
    // RAD-Epsilon
    icsneoc2_devicetype_rad_epsilon,
    // RAD-Moon 3
    icsneoc2_devicetype_rad_moon3,
    // RAD-Comet
    icsneoc2_devicetype_rad_comet,
    // neoVI FIRE3 FlexRay
    icsneoc2_devicetype_fire3_flexray,
    // neoVI CONNECT
    icsneoc2_devicetype_connect,
    // RAD-Comet 3
    icsneoc2_devicetype_rad_comet3,
    // RAD-Moon T1S
    icsneoc2_devicetype_rad_moon_t1s,
    // RAD-Gigastar 2
    icsneoc2_devicetype_rad_gigastar2,
    // neoVI RED
    icsneoc2_devicetype_red,
    // neoECU - Obsolete
    icsneoc2_devicetype_ecu,
    // IEVB - Obsolete
    icsneoc2_devicetype_ievb,
    // Pendant - Obsolete
    icsneoc2_devicetype_pendant,
    // neoOBD2 Pro - Obsolete
    icsneoc2_devicetype_obd2_pro,
    // neoECU Chip - Obsolete
    icsneoc2_devicetype_ecuchip_uart,
    // neoVI PLASMA
    icsneoc2_devicetype_plasma,
    // neoAnalog - Obsolete
    icsneoc2_devicetype_neo_analog,
    // Obsolete
    icsneoc2_devicetype_ct_obd,
    // neoVI ION
    icsneoc2_devicetype_ion,
    // RAD-Star - Obsolete
    icsneoc2_devicetype_rad_star,
    // ValueCAN4-4
    icsneoc2_devicetype_vcan44,
    // ValueCAN4-2
    icsneoc2_devicetype_vcan42,
    // CMProbe - Obsolete
    icsneoc2_devicetype_cm_probe,
    // Ethernet EVB - Obsolete
    icsneoc2_devicetype_eevb,
    // ValueCAN.rf - Obsolete
    icsneoc2_devicetype_vcan_rf,
    // neoVI FIRE2
    icsneoc2_devicetype_fire2,
    // neoVI FLEX - Obsolete
    icsneoc2_devicetype_flex,
    // RAD-Galaxy
    icsneoc2_devicetype_rad_galaxy,
    // RAD-Star 2
    icsneoc2_devicetype_rad_star2,
    // VividCAN
    icsneoc2_devicetype_vividcan,
    // neoOBD2 SIM
    icsneoc2_devicetype_obd2_sim,
    // RAD-Galaxy 2
    icsneoc2_devicetype_rad_galaxy2,


    // Must be last entry
    icsneoc2_devicetype_maxsize,
} _icsneoc2_devicetype_t;

/** @brief Integer representation of _icsneoc2_devicetype_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint32_t icsneoc2_devicetype_t;

typedef enum _icsneoc2_msg_type_t {
    icsneoc2_msg_type_device,
    icsneoc2_msg_type_internal,
    icsneoc2_msg_type_bus,

    icsneoc2_msg_type_maxsize,
} _icsneoc2_msg_type_t;

/** @brief Integer representation of _icsneoc2_msg_type_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint32_t icsneoc2_msg_type_t;


/** @brief
 * Bus message types, useful for filtering out or identifying Bus Messages.
*/
typedef enum _icsneoc2_msg_bus_type_t {
    icsneoc2_msg_bus_type_invalid = 0,
    icsneoc2_msg_bus_type_internal = 1, // Used for statuses that don't actually need to be transferred to the client application
    icsneoc2_msg_bus_type_can = 2,
    icsneoc2_msg_bus_type_lin = 3,
    icsneoc2_msg_bus_type_flexray = 4,
    icsneoc2_msg_bus_type_most = 5,
    icsneoc2_msg_bus_type_ethernet = 6,
    icsneoc2_msg_bus_type_lsftcan = 7,
    icsneoc2_msg_bus_type_swcan = 8,
    icsneoc2_msg_bus_type_iso9141 = 9,
    icsneoc2_msg_bus_type_i2c = 10,
    icsneoc2_msg_bus_type_a2b = 11,
    icsneoc2_msg_bus_type_spi = 12,
    icsneoc2_msg_bus_type_mdio = 13,

    // Must be last entry
    icsneoc2_msg_bus_type_maxsize,

    icsneoc2_msg_bus_type_any = 0xFE, // Never actually set as type, but used as flag for filtering
    icsneoc2_msg_bus_type_other = 0xFF
} _icsneoc2_msg_bus_type_t;

/** @brief Integer representation of _icsneoc2_msg_bus_type_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint8_t icsneoc2_msg_bus_type_t;

typedef enum _icsneoc2_netid_t {
    icsneoc2_netid_device = 0,
    icsneoc2_netid_hscan = 1,
    icsneoc2_netid_mscan = 2,
    icsneoc2_netid_swcan = 3,
    icsneoc2_netid_lsftcan = 4,
    icsneoc2_netid_fordscp = 5,
    icsneoc2_netid_j1708 = 6,
    icsneoc2_netid_aux = 7,
    icsneoc2_netid_j1850vpw = 8,
    icsneoc2_netid_iso9141 = 9,
    icsneoc2_netid_disk_data = 10,
    icsneoc2_netid_main51 = 11,
    icsneoc2_netid_red = 12,
    icsneoc2_netid_sci = 13,
    icsneoc2_netid_iso9141_2 = 14,
    icsneoc2_netid_iso14230 = 15,
    icsneoc2_netid_lin = 16,
    icsneoc2_netid_op_ethernet1 = 17,
    icsneoc2_netid_op_ethernet2 = 18,
    icsneoc2_netid_op_ethernet3 = 19,

    // START Device Command Returns
    // When we send a command, the device returns on one of these, depending on command
    icsneoc2_netid_red_ext_memoryread = 20,
    icsneoc2_netid_red_int_memoryread = 21,
    icsneoc2_netid_red_dflash_read = 22,
    icsneoc2_netid_neo_memory_sdread = 23, // Response from NeoMemory (MemoryTypeSD)
    icsneoc2_netid_can_errbits = 24,
    icsneoc2_netid_neo_memory_write_done = 25,
    icsneoc2_netid_red_wave_can1_logical = 26,
    icsneoc2_netid_red_wave_can2_logical = 27,
    icsneoc2_netid_red_wave_lin1_logical = 28,
    icsneoc2_netid_red_wave_lin2_logical = 29,
    icsneoc2_netid_red_wave_lin1_analog = 30,
    icsneoc2_netid_red_wave_lin2_analog = 31,
    icsneoc2_netid_red_wave_misc_analog = 32,
    icsneoc2_netid_red_wave_miscdio2_logical = 33,
    icsneoc2_netid_red_network_com_enable_ex = 34,
    icsneoc2_netid_red_neovi_network = 35,
    icsneoc2_netid_red_read_baud_settings = 36,
    icsneoc2_netid_red_oldformat = 37,
    icsneoc2_netid_red_scope_capture = 38,
    icsneoc2_netid_red_hardware_excep = 39,
    icsneoc2_netid_red_get_rtc = 40,
    // END Device Command Returns

    icsneoc2_netid_iso9141_3 = 41,
    icsneoc2_netid_hscan2 = 42,
    icsneoc2_netid_hscan3 = 44,
    icsneoc2_netid_op_ethernet4 = 45,
    icsneoc2_netid_op_ethernet5 = 46,
    icsneoc2_netid_iso9141_4 = 47,
    icsneoc2_netid_lin2 = 48,
    icsneoc2_netid_lin3 = 49,
    icsneoc2_netid_lin4 = 50,
    icsneoc2_netid_most_unused = 51, // MOST = 51, Old and unused
    icsneoc2_netid_red_app_error = 52,
    icsneoc2_netid_cgi = 53,
    icsneoc2_netid_reset_status = 54,
    icsneoc2_netid_fb_status = 55,
    icsneoc2_netid_app_signal_status = 56,
    icsneoc2_netid_read_datalink_cm_tx_msg = 57,
    icsneoc2_netid_read_datalink_cm_rx_msg = 58,
    icsneoc2_netid_logging_overflow = 59,
    icsneoc2_netid_read_settings = 60,
    icsneoc2_netid_hscan4 = 61,
    icsneoc2_netid_hscan5 = 62,
    icsneoc2_netid_rs232 = 63,
    icsneoc2_netid_uart = 64,
    icsneoc2_netid_uart2 = 65,
    icsneoc2_netid_uart3 = 66,
    icsneoc2_netid_uart4 = 67,
    icsneoc2_netid_swcan2 = 68,
    icsneoc2_netid_ethernet_daq = 69,
    icsneoc2_netid_data_to_host = 70,
    icsneoc2_netid_textapi_to_host = 71,
    icsneoc2_netid_spi1 = 72,
    icsneoc2_netid_op_ethernet6 = 73,
    icsneoc2_netid_red_vbat = 74,
    icsneoc2_netid_op_ethernet7 = 75,
    icsneoc2_netid_op_ethernet8 = 76,
    icsneoc2_netid_op_ethernet9 = 77,
    icsneoc2_netid_op_ethernet10 = 78,
    icsneoc2_netid_op_ethernet11 = 79,
    icsneoc2_netid_flexray1a = 80,
    icsneoc2_netid_flexray1b = 81,
    icsneoc2_netid_flexray2a = 82,
    icsneoc2_netid_flexray2b = 83,
    icsneoc2_netid_lin5 = 84,
    icsneoc2_netid_flexray = 85,
    icsneoc2_netid_flexray2 = 86,
    icsneoc2_netid_op_ethernet12 = 87,
    icsneoc2_netid_i2c = 88,
    icsneoc2_netid_most25 = 90,
    icsneoc2_netid_most50 = 91,
    icsneoc2_netid_most150 = 92,
    icsneoc2_netid_ethernet = 93,
    icsneoc2_netid_gmfsa = 94,
    icsneoc2_netid_tcp = 95,
    icsneoc2_netid_hscan6 = 96,
    icsneoc2_netid_hscan7 = 97,
    icsneoc2_netid_lin6 = 98,
    icsneoc2_netid_lsftcan2 = 99,
    icsneoc2_netid_logical_disk_info = 187,
    icsneoc2_netid_wivi_command = 221,
    icsneoc2_netid_script_status = 224,
    icsneoc2_netid_eth_phy_control = 239,
    icsneoc2_netid_extended_command = 240,
    icsneoc2_netid_extended_data = 242,
    icsneoc2_netid_flexray_control = 243,
    icsneoc2_netid_coremini_preload = 244,
    icsneoc2_netid_hw_com_latency_test = 512,
    icsneoc2_netid_device_status = 513,
    icsneoc2_netid_udp = 514,
    icsneoc2_netid_forwarded_message = 516,
    icsneoc2_netid_i2c2 = 517,
    icsneoc2_netid_i2c3 = 518,
    icsneoc2_netid_i2c4 = 519,
    icsneoc2_netid_ethernet2 = 520,
    icsneoc2_netid_a2b1 = 522,
    icsneoc2_netid_a2b2 = 523,
    icsneoc2_netid_ethernet3 = 524,
    icsneoc2_netid_wbms = 532,
    icsneoc2_netid_dwcan9 = 534,
    icsneoc2_netid_dwcan10 = 535,
    icsneoc2_netid_dwcan11 = 536,
    icsneoc2_netid_dwcan12 = 537,
    icsneoc2_netid_dwcan13 = 538,
    icsneoc2_netid_dwcan14 = 539,
    icsneoc2_netid_dwcan15 = 540,
    icsneoc2_netid_dwcan16 = 541,
    icsneoc2_netid_lin7 = 542,
    icsneoc2_netid_lin8 = 543,
    icsneoc2_netid_spi2 = 544,
    icsneoc2_netid_mdio1 = 545,
    icsneoc2_netid_mdio2 = 546,
    icsneoc2_netid_mdio3 = 547,
    icsneoc2_netid_mdio4 = 548,
    icsneoc2_netid_mdio5 = 549,
    icsneoc2_netid_mdio6 = 550,
    icsneoc2_netid_mdio7 = 551,
    icsneoc2_netid_mdio8 = 552,
    icsneoc2_netid_op_ethernet13 = 553,
    icsneoc2_netid_op_ethernet14 = 554,
    icsneoc2_netid_op_ethernet15 = 555,
    icsneoc2_netid_op_ethernet16 = 556,
    icsneoc2_netid_spi3 = 557,
    icsneoc2_netid_spi4 = 558,
    icsneoc2_netid_spi5 = 559,
    icsneoc2_netid_spi6 = 560,
    icsneoc2_netid_spi7 = 561,
    icsneoc2_netid_spi8 = 562,
    icsneoc2_netid_lin9 = 563,
    icsneoc2_netid_lin10 = 564,
    icsneoc2_netid_lin11 = 565,
    icsneoc2_netid_lin12 = 566,
    icsneoc2_netid_lin13 = 567,
    icsneoc2_netid_lin14 = 568,
    icsneoc2_netid_lin15 = 569,
    icsneoc2_netid_lin16 = 570,

    // Must be the last entry
    icsneoc2_netid_maxsize,

    icsneoc2_netid_any = 0xfffe, // Never actually set as type, but used as flag for filtering
    icsneoc2_netid_invalid = 0xffff
} _icsneoc2_netid_t;

/** @brief Integer representation of _icsneoc2_netid_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint16_t icsneoc2_netid_t;

#ifdef __cplusplus
}
#endif
