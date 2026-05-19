#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define ICSNEOC2_OPEN_OPTIONS_NONE               0x0 // No options
#define ICSNEOC2_OPEN_OPTIONS_GO_ONLINE          0x1 // After opening, go online
#define ICSNEOC2_OPEN_OPTIONS_SYNC_RTC           0x2 // After opening, sync RTC
#define ICSNEOC2_OPEN_OPTIONS_ENABLE_AUTO_UPDATE 0x4 // After opening, enable auto update
#define ICSNEOC2_OPEN_OPTIONS_FORCE_UPDATE       0x8 // After opening, force update

typedef uint32_t icsneoc2_open_options_t;

typedef enum _icsneoc2_devicetype_t {
	// Ordered by numeric value to keep bitmask values grouped and predictable.
	icsneoc2_devicetype_unknown = 0x00000000,
	icsneoc2_devicetype_blue = 0x00000001,
	icsneoc2_devicetype_ecu_avb = 0x00000002,
	icsneoc2_devicetype_rad_supermoon = 0x00000003,
	icsneoc2_devicetype_dw_vcan = 0x00000004,
	icsneoc2_devicetype_rad_moon2 = 0x00000005,
	icsneoc2_devicetype_rad_mars = 0x00000006,
	icsneoc2_devicetype_vcan41 = 0x00000007,
	icsneoc2_devicetype_fire = 0x00000008,
	icsneoc2_devicetype_rad_pluto = 0x00000009,
	icsneoc2_devicetype_vcan42_el = 0x0000000a,
	icsneoc2_devicetype_radio_canhub = 0x0000000b,
	icsneoc2_devicetype_neo_ecu12 = 0x0000000c,
	icsneoc2_devicetype_obd2_lc_badge = 0x0000000d,
	icsneoc2_devicetype_rad_moon_duo = 0x0000000e,
	icsneoc2_devicetype_fire3 = 0x0000000f,
	icsneoc2_devicetype_vcan3 = 0x00000010,
	icsneoc2_devicetype_rad_jupiter = 0x00000011,
	icsneoc2_devicetype_vcan4_industrial = 0x00000012,
	icsneoc2_devicetype_rad_gigastar = 0x00000013,
	icsneoc2_devicetype_red2 = 0x00000014,
	icsneoc2_devicetype_etherbadge = 0x00000016,
	icsneoc2_devicetype_rad_a2b = 0x00000017,
	icsneoc2_devicetype_rad_epsilon = 0x00000018,
	icsneoc2_devicetype_rad_epsilon_xl = 0x0000001e,
	icsneoc2_devicetype_rad_galaxy2 = 0x00000021,
	icsneoc2_devicetype_rad_moon3 = 0x00000023,
	icsneoc2_devicetype_rad_comet2 = 0x00000024,
	icsneoc2_devicetype_fire3_flexray = 0x00000025,
	icsneoc2_devicetype_connect = 0x00000026,
	icsneoc2_devicetype_rad_comet3 = 0x00000027,
	icsneoc2_devicetype_rad_moon_t1s = 0x00000028,
	icsneoc2_devicetype_rad_gigastar2 = 0x00000029,
	icsneoc2_devicetype_fire3_t1s_lin = 0x0000002a,
	icsneoc2_devicetype_fire3_t1s_sent = 0x0000002b,
	icsneoc2_devicetype_rad_gemini = 0x0000002c,
	icsneoc2_devicetype_red = 0x00000040,
	icsneoc2_devicetype_ecu = 0x00000080,
	icsneoc2_devicetype_ievb = 0x00000100,
	icsneoc2_devicetype_pendant = 0x00000200,
	icsneoc2_devicetype_obd2_pro = 0x00000400,
	icsneoc2_devicetype_ecuchip_uart = 0x00000800,
	icsneoc2_devicetype_plasma = 0x00001000,
	icsneoc2_devicetype_neo_analog = 0x00004000,
	icsneoc2_devicetype_ct_obd = 0x00008000,
	icsneoc2_devicetype_ion = 0x00040000,
	icsneoc2_devicetype_rad_star = 0x00080000,
	icsneoc2_devicetype_vcan44 = 0x00200000,
	icsneoc2_devicetype_vcan42 = 0x00400000,
	icsneoc2_devicetype_cm_probe = 0x00800000,
	icsneoc2_devicetype_eevb = 0x01000000,
	icsneoc2_devicetype_vcan_rf = 0x02000000,
	icsneoc2_devicetype_fire2 = 0x04000000,
	icsneoc2_devicetype_flex = 0x08000000,
	icsneoc2_devicetype_rad_galaxy = 0x10000000,
	icsneoc2_devicetype_rad_star2 = 0x20000000,
	icsneoc2_devicetype_vividcan = 0x40000000,
	icsneoc2_devicetype_obd2_sim = 0x80000000,

	icsneoc2_devicetype_maxsize // Must be last entry
} _icsneoc2_devicetype_t;

typedef uint32_t icsneoc2_devicetype_t;

typedef enum _icsneoc2_network_type_t {
	icsneoc2_network_type_invalid = 0, // Invalid network type, used for error checking
	icsneoc2_network_type_internal = 1, // Used for statuses that don't actually need to be transferred to the client application
	icsneoc2_network_type_can = 2, // CAN (Controller Area Network)
	icsneoc2_network_type_lin = 3, // LIN (Local Interconnect Network)
	icsneoc2_network_type_flexray = 4, // FlexRay
	icsneoc2_network_type_most = 5, // MOST (Media Oriented Systems Transport)
	icsneoc2_network_type_ethernet = 6, // Ethernet
	icsneoc2_network_type_lsftcan = 7, // LSFTCAN (Low Speed/Fault Tolerant CAN)
	icsneoc2_network_type_swcan = 8, // SWCAN (Single-Wire CAN)
	icsneoc2_network_type_iso9141 = 9, // ISO 9141-2 (K-Line)
	icsneoc2_network_type_i2c = 10, // I2C (Inter-Integrated Circuit)
	icsneoc2_network_type_a2b = 11, // A2B (Automotive Audio Bus)
	icsneoc2_network_type_spi = 12, // SPI (Serial Peripheral Interface)
	icsneoc2_network_type_mdio = 13, // MDIO (Management Data Input/Output)
	icsneoc2_network_type_automotive_ethernet = 14, // Automotive Ethernet

	// Must be the last entry, Don't use as a network type.
	icsneoc2_network_type_maxsize,

	// Used as a wildcard for filtering, never actually set as a type
	icsneoc2_network_type_any = 0xFE,
	// Used for unknown or other network types
	icsneoc2_network_type_other = 0xFF
} _icsneoc2_network_type_t;

typedef uint8_t icsneoc2_network_type_t;

typedef enum _icsneoc2_netid_t {
	icsneoc2_netid_device = 0,
	icsneoc2_netid_dwcan_01 = 1,
	icsneoc2_netid_dwcan_08 = 2,
	icsneoc2_netid_swcan_01 = 3,
	icsneoc2_netid_lsftcan_01 = 4,
	icsneoc2_netid_fordscp = 5,
	icsneoc2_netid_j1708 = 6,
	icsneoc2_netid_aux = 7,
	icsneoc2_netid_j1850vpw = 8,
	icsneoc2_netid_iso9141 = 9,
	icsneoc2_netid_disk_data = 10,
	icsneoc2_netid_main51 = 11,
	icsneoc2_netid_red = 12,
	icsneoc2_netid_sci = 13,
	icsneoc2_netid_iso9141_02 = 14,
	icsneoc2_netid_iso14230 = 15,
	icsneoc2_netid_lin_01 = 16,
	icsneoc2_netid_ae_01 = 17,
	icsneoc2_netid_ae_02 = 18,
	icsneoc2_netid_ae_03 = 19,

	// START Device Command Returns
	// When we send a command, the device returns on one of these, depending on command
	icsneoc2_netid_red_ext_memoryread = 20,
	icsneoc2_netid_red_int_memoryread = 21,
	icsneoc2_netid_red_dflash_read = 22,
	icsneoc2_netid_neo_memory_sdread = 23,
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

	icsneoc2_netid_iso9141_03 = 41,
	icsneoc2_netid_dwcan_02 = 42,
	icsneoc2_netid_dwcan_03 = 44,
	icsneoc2_netid_ae_04 = 45,
	icsneoc2_netid_ae_05 = 46,
	icsneoc2_netid_iso9141_04 = 47,
	icsneoc2_netid_lin_02 = 48,
	icsneoc2_netid_lin_03 = 49,
	icsneoc2_netid_lin_04 = 50,
	icsneoc2_netid_most_unused = 51,
	icsneoc2_netid_red_app_error = 52,
	icsneoc2_netid_cgi = 53,
	icsneoc2_netid_reset_status = 54,
	icsneoc2_netid_fb_status = 55,
	icsneoc2_netid_app_signal_status = 56,
	icsneoc2_netid_read_datalink_cm_tx_msg = 57,
	icsneoc2_netid_read_datalink_cm_rx_msg = 58,
	icsneoc2_netid_logging_overflow = 59,
	icsneoc2_netid_read_settings = 60,
	icsneoc2_netid_dwcan_04 = 61,
	icsneoc2_netid_dwcan_05 = 62,
	icsneoc2_netid_rs232 = 63,
	icsneoc2_netid_uart_01 = 64,
	icsneoc2_netid_uart_02 = 65,
	icsneoc2_netid_uart_03 = 66,
	icsneoc2_netid_uart_04 = 67,
	icsneoc2_netid_swcan_02 = 68,
	icsneoc2_netid_ethernet_daq = 69,
	icsneoc2_netid_data_to_host = 70,
	icsneoc2_netid_textapi_to_host = 71,
	icsneoc2_netid_spi_01 = 72,
	icsneoc2_netid_ae_06 = 73,
	icsneoc2_netid_red_vbat = 74,
	icsneoc2_netid_ae_07 = 75,
	icsneoc2_netid_ae_08 = 76,
	icsneoc2_netid_ae_09 = 77,
	icsneoc2_netid_ae_10 = 78,
	icsneoc2_netid_ae_11 = 79,
	icsneoc2_netid_flexray_01a = 80,
	icsneoc2_netid_flexray_01b = 81,
	icsneoc2_netid_flexray_02a = 82,
	icsneoc2_netid_flexray_02b = 83,
	icsneoc2_netid_lin_05 = 84,
	icsneoc2_netid_flexray_01 = 85,
	icsneoc2_netid_flexray_02 = 86,
	icsneoc2_netid_ae_12 = 87,
	icsneoc2_netid_i2c_01 = 88,
	icsneoc2_netid_most_25 = 90,
	icsneoc2_netid_most_50 = 91,
	icsneoc2_netid_most_150 = 92,
	icsneoc2_netid_ethernet_01 = 93,
	icsneoc2_netid_gmfsa = 94,
	icsneoc2_netid_tcp = 95,
	icsneoc2_netid_dwcan_06 = 96,
	icsneoc2_netid_dwcan_07 = 97,
	icsneoc2_netid_lin_06 = 98,
	icsneoc2_netid_lsftcan_02 = 99,
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
	icsneoc2_netid_i2c_02 = 517,
	icsneoc2_netid_i2c_03 = 518,
	icsneoc2_netid_i2c_04 = 519,
	icsneoc2_netid_ethernet_02 = 520,
	icsneoc2_netid_ethernet_tx_wrap = 521,
	icsneoc2_netid_a2b_01 = 522,
	icsneoc2_netid_a2b_02 = 523,
	icsneoc2_netid_ethernet_03 = 524,
	icsneoc2_netid_wbms_01 = 532,
	icsneoc2_netid_dwcan_09 = 534,
	icsneoc2_netid_dwcan_10 = 535,
	icsneoc2_netid_dwcan_11 = 536,
	icsneoc2_netid_dwcan_12 = 537,
	icsneoc2_netid_dwcan_13 = 538,
	icsneoc2_netid_dwcan_14 = 539,
	icsneoc2_netid_dwcan_15 = 540,
	icsneoc2_netid_dwcan_16 = 541,
	icsneoc2_netid_lin_07 = 542,
	icsneoc2_netid_lin_08 = 543,
	icsneoc2_netid_spi_02 = 544,
	icsneoc2_netid_mdio_01 = 545,
	icsneoc2_netid_mdio_02 = 546,
	icsneoc2_netid_mdio_03 = 547,
	icsneoc2_netid_mdio_04 = 548,
	icsneoc2_netid_mdio_05 = 549,
	icsneoc2_netid_mdio_06 = 550,
	icsneoc2_netid_mdio_07 = 551,
	icsneoc2_netid_mdio_08 = 552,
	icsneoc2_netid_ae_13 = 553,
	icsneoc2_netid_ae_14 = 554,
	icsneoc2_netid_ae_15 = 555,
	icsneoc2_netid_ae_16 = 556,
	icsneoc2_netid_spi_03 = 557,
	icsneoc2_netid_spi_04 = 558,
	icsneoc2_netid_spi_05 = 559,
	icsneoc2_netid_spi_06 = 560,
	icsneoc2_netid_spi_07 = 561,
	icsneoc2_netid_spi_08 = 562,
	icsneoc2_netid_lin_09 = 563,
	icsneoc2_netid_lin_10 = 564,
	icsneoc2_netid_lin_11 = 565,
	icsneoc2_netid_lin_12 = 566,
	icsneoc2_netid_lin_13 = 567,
	icsneoc2_netid_lin_14 = 568,
	icsneoc2_netid_lin_15 = 569,
	icsneoc2_netid_lin_16 = 570,

	// Must be the last entry, Don't use as a NetID.
	icsneoc2_netid_maxsize,

	// Used as a wildcard for filtering, never actually set as a NetID
	icsneoc2_netid_any = 0xfffe,
	// Used for unknown or invalid NetID
	icsneoc2_netid_invalid = 0xffff
} _icsneoc2_netid_t;

typedef uint16_t icsneoc2_netid_t;

typedef enum _icsneoc2_io_type_t {
	icsneoc2_io_type_eth_activation = 0, // DoIP activation line (0=HiZ, 1=pulled up to VBAT)
	icsneoc2_io_type_usb_host_power = 1, // USB host power control
	icsneoc2_io_type_backup_power_en = 2, // Backup super capacitor enable (FIRE 2)
	icsneoc2_io_type_backup_power_good = 3, // Backup super capacitor charged status (FIRE 2, read only)
	icsneoc2_io_type_misc = 4, // General purpose I/O on the device
	icsneoc2_io_type_emisc = 5, // Extended general purpose I/O on the device

	// Must be last entry. Don't use as an IO type.
	icsneoc2_io_type_maxsize
} _icsneoc2_io_type_t;

typedef uint32_t icsneoc2_io_type_t;

typedef enum _icsneoc2_lin_mode_t {
	icsneoc2_lin_mode_sleep = 0,
	icsneoc2_lin_mode_slow = 1,
	icsneoc2_lin_mode_normal = 2,
	icsneoc2_lin_mode_fast = 3,

	// Must be last entry. Don't use as a LIN mode.
	icsneoc2_lin_mode_maxsize
} _icsneoc2_lin_mode_t;

typedef uint32_t icsneoc2_lin_mode_t;

typedef enum _icsneoc2_ae_link_mode_t {
	icsneoc2_ae_link_mode_invalid = -1, // Invalid/Unknown link mode
	icsneoc2_ae_link_mode_auto = 0,     // Auto select link mode
	icsneoc2_ae_link_mode_master = 1,   // Master mode
	icsneoc2_ae_link_mode_slave = 2,    // Slave mode

	// Must be last entry. Don't use as an AE link mode.
	icsneoc2_ae_link_mode_maxsize
} _icsneoc2_ae_link_mode_t;

typedef uint32_t icsneoc2_ae_link_mode_t;

typedef enum _icsneoc2_eth_phy_link_mode_t {
	icsneoc2_eth_phy_link_mode_auto_negotiation = 1,    // Auto negotiation mode
	icsneoc2_eth_phy_link_mode_10mbps_halfduplex = 2,   // 10 Mbps half duplex
	icsneoc2_eth_phy_link_mode_10mbps_full_duplex = 3,  // 10 Mbps full duplex
	icsneoc2_eth_phy_link_mode_100mbps_half_duplex = 4, // 100 Mbps half duplex
	icsneoc2_eth_phy_link_mode_100mbps_full_duplex = 5, // 100 Mbps full duplex
	icsneoc2_eth_phy_link_mode_1gbps_half_duplex = 6,   // 1 Gbps half duplex
	icsneoc2_eth_phy_link_mode_1gbps_full_duplex = 7,   // 1 Gbps full duplex
	icsneoc2_eth_phy_link_mode_2_5gbps_full_duplex = 8, // 2.5 Gbps full duplex
	icsneoc2_eth_phy_link_mode_5gbps_full_duplex = 9,   // 5 Gbps full duplex
	icsneoc2_eth_phy_link_mode_10gbps_full_duplex = 10,  // 10 Gbps full duplex

	// Must be last entry. Don't use as an Ethernet PHY link mode.
	icsneoc2_eth_phy_link_mode_maxsize
} _icsneoc2_eth_phy_link_mode_t;

typedef uint32_t icsneoc2_eth_phy_link_mode_t;

typedef enum _icsneoc2_misc_io_analog_voltage_t {
	icsneoc2_misc_io_analog_voltage_v0 = 0, // Analog voltage level 0V
	icsneoc2_misc_io_analog_voltage_v1 = 1, // Analog voltage level 1V
	icsneoc2_misc_io_analog_voltage_v2 = 2, // Analog voltage level 2V
	icsneoc2_misc_io_analog_voltage_v3 = 3, // Analog voltage level 3V
	icsneoc2_misc_io_analog_voltage_v4 = 4, // Analog voltage level 4V
	icsneoc2_misc_io_analog_voltage_v5 = 5, // Analog voltage level 5V

	// Must be last entry. Don't use as an analog voltage.
	icsneoc2_misc_io_analog_voltage_maxsize
} _icsneoc2_misc_io_analog_voltage_t;

typedef uint8_t icsneoc2_misc_io_analog_voltage_t;

typedef struct icsneoc2_disk_details_t icsneoc2_disk_details_t;

typedef enum _icsneoc2_disk_layout_t {
	// Spanned layout, where each disk is treated as one contiguous space and data is written to each disk until it's full before moving to the next.
	icsneoc2_disk_layout_spanned = 0,
	// RAID 0 layout, where data is striped across multiple disks.
	icsneoc2_disk_layout_raid0 = 1,

	// Must be last entry. Don't use as a disk layout.
	icsneoc2_disk_layout_maxsize
} _icsneoc2_disk_layout_t;

typedef uint8_t icsneoc2_disk_layout_t;

#define ICSNEOC2_DISK_FORMAT_FLAGS_PRESENT     0x1 // Indicates that the disk is present
#define ICSNEOC2_DISK_FORMAT_FLAGS_INITIALIZED 0x2 // Indicates that the disk is initialized
#define ICSNEOC2_DISK_FORMAT_FLAGS_FORMATTED   0x4 // Indicates that the disk is already formatted

typedef uint32_t icsneoc2_disk_format_flags_t;

typedef enum _icsneoc2_disk_format_directive_t {
	icsneoc2_disk_format_directive_continue = 0,
	icsneoc2_disk_format_directive_stop = 1,

	// Must be last entry. Don't use as a disk format directive.
	icsneoc2_disk_format_directive_maxsize
} _icsneoc2_disk_format_directive_t;

typedef uint8_t icsneoc2_disk_format_directive_t;

typedef icsneoc2_disk_format_directive_t (*icsneoc2_disk_format_progress_fn)(
	uint64_t sectors_formatted,
	uint64_t total_sectors,
	void* user_data
);

typedef struct icsneoc2_script_status_t icsneoc2_script_status_t;

typedef enum _icsneoc2_memory_type_t {
	icsneoc2_memory_type_flash = 0, // Flash memory
	icsneoc2_memory_type_sd = 1, // SD card memory

	// Must be last entry. Don't use as a memory type.
	icsneoc2_memory_type_maxsize
} _icsneoc2_memory_type_t;

typedef uint8_t icsneoc2_memory_type_t;

typedef enum _icsneoc2_can_error_code_t {
	icsneoc2_can_error_code_no_error = 0, // No error
	icsneoc2_can_error_code_stuff_error,  // Stuff error
	icsneoc2_can_error_code_form_error,   // Form error
	icsneoc2_can_error_code_ack_error,    // Ack error
	icsneoc2_can_error_code_bit1_error,   // Bit1 error
	icsneoc2_can_error_code_bit0_error,   // Bit0 error
	icsneoc2_can_error_code_crc_error,    // CRC error
	icsneoc2_can_error_code_no_change,    // No change

	// Must be last entry. Don't use as a CAN error code.
	icsneoc2_can_error_code_maxsize
} _icsneoc2_can_error_code_t;

typedef uint8_t icsneoc2_can_error_code_t;

#define ICSNEOC2_MESSAGE_CAN_ERROR_FLAGS_BUS_OFF       0x01 // Bus off state
#define ICSNEOC2_MESSAGE_CAN_ERROR_FLAGS_ERROR_PASSIVE 0x02 // Error passive state
#define ICSNEOC2_MESSAGE_CAN_ERROR_FLAGS_ERROR_WARN    0x04 // Error warning state

typedef uint64_t icsneoc2_message_can_error_flags_t;

typedef enum _icsneoc2_tc10_wake_status_t {
	icsneoc2_tc10_wake_status_no_wake_received = 0, // No wake signal received
	icsneoc2_tc10_wake_status_wake_received = 1,    // Wake signal received

	// Must be last entry. Don't use as a TC10 wake status.
	icsneoc2_tc10_wake_status_maxsize
} _icsneoc2_tc10_wake_status_t;

typedef uint8_t icsneoc2_tc10_wake_status_t;

typedef enum _icsneoc2_tc10_sleep_status_t {
	icsneoc2_tc10_sleep_status_no_sleep_received = 0, // No sleep signal received
	icsneoc2_tc10_sleep_status_sleep_received = 1,    // Sleep signal received
	icsneoc2_tc10_sleep_status_sleep_failed = 2,      // Sleep attempt failed
	icsneoc2_tc10_sleep_status_sleep_aborted = 3,     // Sleep attempt aborted

	// Must be last entry. Don't use as a TC10 sleep status.
	icsneoc2_tc10_sleep_status_maxsize
} _icsneoc2_tc10_sleep_status_t;

typedef uint8_t icsneoc2_tc10_sleep_status_t;

typedef struct icsneoc2_chip_versions_t icsneoc2_chip_versions_t;

typedef enum _icsneoc2_chip_id_t {
	icsneoc2_chip_id_neovifire_mchip = 0,
	icsneoc2_chip_id_neovifire_lchip = 1,
	icsneoc2_chip_id_neovifire_uchip = 2,
	icsneoc2_chip_id_neovifire_jchip = 3,
	icsneoc2_chip_id_valuecan3_mchip = 4,
	icsneoc2_chip_id_neoviecu_mpic = 6,
	icsneoc2_chip_id_neoviievb_mpic = 7,
	icsneoc2_chip_id_neovipendant_mpic = 8,
	icsneoc2_chip_id_neovifire_vnet_mchip = 9,
	icsneoc2_chip_id_neovifire_vnet_lchip = 10,
	icsneoc2_chip_id_neoviplasma_core = 11,
	icsneoc2_chip_id_neoviplasma_hid = 12,
	icsneoc2_chip_id_neovianalog_mpic = 13,
	icsneoc2_chip_id_neoviplasma_analog_core = 14,
	icsneoc2_chip_id_neoviplasma_flexray_core = 15,
	icsneoc2_chip_id_neoviplasma_core_1_12 = 16,
	icsneoc2_chip_id_neovifire_slave_vnet_mchip = 17,
	icsneoc2_chip_id_neovifire_slave_vnet_lchip = 18,
	icsneoc2_chip_id_neoviion_core = 19,
	icsneoc2_chip_id_neoviion_hid = 20,
	icsneoc2_chip_id_neoviion_core_loader = 21,
	icsneoc2_chip_id_neoviion_hid_loader = 22,
	icsneoc2_chip_id_neoviion_fpga_bit = 23,
	icsneoc2_chip_id_neovifire_vnet_ep_mchip = 24,
	icsneoc2_chip_id_neovifire_vnet_ep_lchip = 25,
	icsneoc2_chip_id_neovianalogout_mchip = 26,
	icsneoc2_chip_id_neovimost25_mchip = 27,
	icsneoc2_chip_id_neovimost50_mchip = 28,
	icsneoc2_chip_id_neovimost150_mchip = 29,
	icsneoc2_chip_id_valuecan4_4_mchip = 30,
	icsneoc2_chip_id_valuecan4_4_schip = 31,
	icsneoc2_chip_id_cmprobe_zynq = 33,
	icsneoc2_chip_id_eevb_stm32 = 34,
	icsneoc2_chip_id_neovifire_slave_vnet_ep_mchip = 35,
	icsneoc2_chip_id_neovifire_slave_vnet_ep_lchip = 36,
	icsneoc2_chip_id_radstar_mchip = 37,
	icsneoc2_chip_id_valuecanrf_mchip = 38,
	icsneoc2_chip_id_neovifire2_mchip = 39,
	icsneoc2_chip_id_neovifire2_cchip = 40,
	icsneoc2_chip_id_neovifire2_core = 41,
	icsneoc2_chip_id_neovifire2_blechip = 42,
	icsneoc2_chip_id_neovifire2_zynq = 43, // FIRE2 MVNET Z - Zynq
	icsneoc2_chip_id_neovifire2_securitychip = 44,
	icsneoc2_chip_id_radgalaxy_zynq = 45,
	icsneoc2_chip_id_neovifire2_vnet_mchip = 46,
	icsneoc2_chip_id_neovifire2_slave_vnet_a_mchip = 47,
	icsneoc2_chip_id_neovifire2_slave_vnet_a_cchip = 48,
	icsneoc2_chip_id_neovifire2_vnet_cchip = 49,
	icsneoc2_chip_id_neovifire2_vnet_core = 50,
	icsneoc2_chip_id_radstar2_zynq = 51,
	icsneoc2_chip_id_vividcan_mchip = 52,
	icsneoc2_chip_id_neoobd2sim_mchip = 53,
	icsneoc2_chip_id_neovifire2_vnetz_mchip = 54,
	icsneoc2_chip_id_neovifire2_vnetz_zynq = 55,
	icsneoc2_chip_id_neovifire2_slave_vnetz_a_mchip = 56,
	icsneoc2_chip_id_neovifire2_slave_vnetz_a_zynq = 57,
	icsneoc2_chip_id_vividcan_ext_flash = 58,
	icsneoc2_chip_id_vividcan_nrf52 = 59,
	icsneoc2_chip_id_cmprobe_zynq_unused = 60, // Double defined
	icsneoc2_chip_id_neoobd2pro_mchip = 61,
	icsneoc2_chip_id_valuecan4_1_mchip = 62,
	icsneoc2_chip_id_valuecan4_2_mchip = 63,
	icsneoc2_chip_id_valuecan4_4_2el_core = 64,
	icsneoc2_chip_id_neoobd2pro_schip = 65,
	icsneoc2_chip_id_valuecan4_2el_mchip = 67,
	icsneoc2_chip_id_neoecuavbtsn_mchip = 68,
	icsneoc2_chip_id_neoobd2pro_core = 69,
	icsneoc2_chip_id_radsupermoon_zynq = 70,
	icsneoc2_chip_id_radmoon2_zynq = 71,
	icsneoc2_chip_id_vividcanpro_mchip = 72,
	icsneoc2_chip_id_vividcanpro_ext_flash = 73,
	icsneoc2_chip_id_radpluto_mchip = 74,
	icsneoc2_chip_id_radmars_zynq = 75,
	icsneoc2_chip_id_neoecu12_mchip = 76,
	icsneoc2_chip_id_radiocanhub_mchip = 77,
	icsneoc2_chip_id_flexray_vnetz_zchip = 78,
	icsneoc2_chip_id_neoobd2_lcbadge_mchip = 79,
	icsneoc2_chip_id_neoobd2_lcbadge_schip = 80,
	icsneoc2_chip_id_radmoonduo_mchip = 81,
	icsneoc2_chip_id_neovifire3_zchip = 82,
	icsneoc2_chip_id_flexray_vnetz_fchip = 83,
	icsneoc2_chip_id_radjupiter_mchip = 84,
	icsneoc2_chip_id_valuecan4industrial_mchip = 85,
	icsneoc2_chip_id_etherbadge_mchip = 86,
	icsneoc2_chip_id_radmars_3_zynq = 87,
	icsneoc2_chip_id_radgigastar_usbz_zynq = 88,
	icsneoc2_chip_id_radgigastar_zynq = 89,
	icsneoc2_chip_id_rad4g_mchip = 90,
	icsneoc2_chip_id_neovifire3_schip = 91,
	icsneoc2_chip_id_radepsilon_mchip = 92,
	icsneoc2_chip_id_rada2b_zchip = 93,
	icsneoc2_chip_id_neoobd2dev_mchip = 94,
	icsneoc2_chip_id_neoobd2dev_schip = 95,
	icsneoc2_chip_id_neoobd2simdoip_mchip = 96,
	icsneoc2_chip_id_sfpmodule_88q2112_mchip = 97,
	icsneoc2_chip_id_radepsilont_mchip = 98,
	icsneoc2_chip_id_radepsilonexpress_mchip = 99,
	icsneoc2_chip_id_radproxima_mchip = 100,
	icsneoc2_chip_id_newdevice57_zchip = 101,
	icsneoc2_chip_id_rad_galaxy_2_zmpchip_id = 102,
	icsneoc2_chip_id_newdevice59_mchip = 103,
	icsneoc2_chip_id_radmoon2_z7010_zynq = 104,
	icsneoc2_chip_id_neovifire2_core_sg4 = 105,
	icsneoc2_chip_id_radbms_mchip = 106,
	icsneoc2_chip_id_radmoon2_zl_mchip = 107,
	icsneoc2_chip_id_radgigastar_usbz_z7010_zynq = 108,
	icsneoc2_chip_id_neovifire3_linux = 109,
	icsneoc2_chip_id_radgigastar_usbz_z7007s_zynq = 110,
	icsneoc2_chip_id_vem_01_8dw_zchip = 111,
	icsneoc2_chip_id_radgalaxy_ffg_zynq = 112,
	icsneoc2_chip_id_radmoon3_mchip = 113,
	icsneoc2_chip_id_radcomet2_zynq = 114,
	icsneoc2_chip_id_vem_02_fr_zchip = 115,
	icsneoc2_chip_id_rada2b_revb_zchip = 116,
	icsneoc2_chip_id_radgigastar_ffg_zynq = 117,
	icsneoc2_chip_id_vem_02_fr_fchip = 118,
	icsneoc2_chip_id_connect_zchip = 121,
	icsneoc2_chip_id_sfpmodule_88q2221_mchip = 122,
	icsneoc2_chip_id_radgalaxy2_sysmon_chip = 123,
	icsneoc2_chip_id_sfpmodule_88q3244_mchip = 124,
	icsneoc2_chip_id_radcomet3_zchip = 125,
	icsneoc2_chip_id_connect_linux = 126,
	icsneoc2_chip_id_sfpmodule_lan8670_mchip = 127,
	icsneoc2_chip_id_vem_04_t1s_lin_zchip = 129,
	icsneoc2_chip_id_radmoont1s_zchip = 130,
	icsneoc2_chip_id_radgigastar2_zynq = 131,
	icsneoc2_chip_id_sfpmodule_ent11100_mchip = 132,
	icsneoc2_chip_id_radgemini_mchip = 135,
	icsneoc2_chip_id_maxsize, // Must be last entry, Don't use as a chip ID.

	// Used for unknown or invalid chip ID
	icsneoc2_chip_id_invalid = 255
} _icsneoc2_chip_id_t;

typedef uint8_t icsneoc2_chip_id_t;

#ifdef __cplusplus
}
#endif
