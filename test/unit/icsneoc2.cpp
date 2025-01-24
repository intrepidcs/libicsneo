#include <gtest/gtest.h>
#include <icsneo/icsneoc2.h>

#include <vector>
#include <tuple>

TEST(icsneoc2, test_icsneoc2_device_find_all) {
    const auto MAX_DEV_SIZE = 255;
    icsneoc2_device_t* devices[MAX_DEV_SIZE] = {0};
    uint32_t devices_count = MAX_DEV_SIZE;

    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_find_all(NULL, NULL, NULL));
    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_find_all(devices, NULL, NULL));
    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_find_all(NULL, &devices_count, NULL));
    ASSERT_EQ(icsneoc2_error_success, icsneoc2_device_find_all(devices, &devices_count, NULL));
    for (uint32_t i=0; i < MAX_DEV_SIZE; ++i) {
        if (i < devices_count) {
            ASSERT_EQ(icsneoc2_error_success, icsneoc2_device_is_valid(devices[i]));
        } else {
            ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_is_valid(devices[i]));
        }
    }
}

TEST(icsneoc2, test_icsneoc2_is_device) {
    icsneoc2_device_t* invalid_device = static_cast<icsneoc2_device_t*>((void*)0x1234);
    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_is_valid(NULL));
    ASSERT_EQ(icsneoc2_error_invalid_device, icsneoc2_device_is_valid(invalid_device));
}

TEST(icsneoc2, test_icsneoc2_error_invalid_parameters_and_invalid_device) {
    bool placeholderBool = false;
    uint32_t placeholderInteger32 = 0;
    uint64_t placeholderInteger64 = 0;
    const char placeholderStr[255] = {0};
    icsneoc2_event_t* eventPlaceHolder = nullptr;

    // All of these don't have a device parameter
    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_find_all(NULL, NULL, NULL));
    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_bus_type_name_get(0, NULL, NULL));

    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_event_description_get(eventPlaceHolder, placeholderStr, &placeholderInteger32));
    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_events_get(NULL, &placeholderInteger32));

    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_type_name_get(0, NULL, NULL));

    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_error_code_get(0, NULL, NULL));
    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_type_name_get(0, NULL, NULL));
    ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_netid_name_get(0, NULL, NULL));

    // Test everything with a device and with a device with a bad pointer
    std::vector<std::tuple<_icsneoc2_error_t, icsneoc2_device_t*>> testPatterns = {
        { icsneoc2_error_invalid_parameters, NULL},
        { icsneoc2_error_invalid_device, static_cast<icsneoc2_device_t*>((void*)0x1234)},
    };
    for (const auto& testPattern : testPatterns) {
        const auto& errorCode = std::get<0>(testPattern);
        const auto& device = std::get<1>(testPattern);

        ASSERT_EQ(errorCode, icsneoc2_device_baudrate_get(device, 0, &placeholderInteger64));
        ASSERT_EQ(errorCode, icsneoc2_device_baudrate_set(device, 0, 0, placeholderInteger64));
        ASSERT_EQ(errorCode, icsneoc2_device_canfd_baudrate_get(device, 0, &placeholderInteger64));
        ASSERT_EQ(errorCode, icsneoc2_device_canfd_baudrate_set(device, 0, 0, placeholderInteger64));
        ASSERT_EQ(errorCode, icsneoc2_device_close(device));
        ASSERT_EQ(errorCode, icsneoc2_device_description_get(device, placeholderStr, &placeholderInteger32));
        
        ASSERT_EQ(errorCode, icsneoc2_device_go_online(device, false));
        ASSERT_EQ(errorCode, icsneoc2_device_is_online(device, &placeholderBool));
        ASSERT_EQ(errorCode, icsneoc2_device_is_online_supported(device, &placeholderBool));
        ASSERT_EQ(errorCode, icsneoc2_device_is_valid(device));
        ASSERT_EQ(errorCode, icsneoc2_device_is_open(device, &placeholderBool));
        ASSERT_EQ(errorCode, icsneoc2_device_is_disconnected(device, &placeholderBool));
        ASSERT_EQ(errorCode, icsneoc2_device_load_default_settings(device, false));

        ASSERT_EQ(errorCode, icsneoc2_device_open(device));
        ASSERT_EQ(errorCode, icsneoc2_device_open_options_get(device, &placeholderInteger32));
        ASSERT_EQ(errorCode, icsneoc2_device_open_options_set(device, 0));
        ASSERT_EQ(errorCode, icsneoc2_device_rtc_get(device, (int64_t*)&placeholderInteger64));
        ASSERT_EQ(errorCode, icsneoc2_device_rtc_set(device, 0));
        ASSERT_EQ(errorCode, icsneoc2_device_serial_get(device, placeholderStr, &placeholderInteger32));
        ASSERT_EQ(errorCode, icsneoc2_device_supports_tc10(device, &placeholderBool));
        ASSERT_EQ(errorCode, icsneoc2_device_timestamp_resolution_get(device, &placeholderInteger32));
        
        ASSERT_EQ(errorCode, icsneoc2_device_type_get(device, NULL));

        ASSERT_EQ(errorCode, icsneoc2_device_message_count_get(device, &placeholderInteger32));
        ASSERT_EQ(errorCode, icsneoc2_device_message_polling_get(device, &placeholderBool));
        ASSERT_EQ(errorCode, icsneoc2_device_message_polling_limit_get(device, &placeholderInteger32));
        ASSERT_EQ(errorCode, icsneoc2_device_message_polling_set(device, false));
        ASSERT_EQ(errorCode, icsneoc2_device_message_polling_set_limit(device, 0));
        ASSERT_EQ(errorCode, icsneoc2_device_messages_get(device, NULL, NULL, 0));
        ASSERT_EQ(errorCode, icsneoc2_device_messages_transmit(device, NULL, NULL));

        ASSERT_EQ(errorCode, icsneoc2_message_bus_type_get(device, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_can_arbid_get(device, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_can_arbid_set(device, NULL, 0));
        ASSERT_EQ(errorCode, icsneoc2_message_can_baudrate_switch_get(device, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_can_baudrate_switch_set(device, NULL, false));
        ASSERT_EQ(errorCode, icsneoc2_message_can_canfd_set(device, NULL, false));
        ASSERT_EQ(errorCode, icsneoc2_message_can_create(device, NULL, 0));
        ASSERT_EQ(errorCode, icsneoc2_message_can_dlc_get(device, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_can_dlc_set(device, NULL, 0));
        ASSERT_EQ(errorCode, icsneoc2_message_can_error_state_indicator_get(device, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_can_extended_set(device, NULL, false));
        ASSERT_EQ(errorCode, icsneoc2_message_can_free(device, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_can_is_canfd(device, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_can_is_extended(device, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_can_is_remote(device, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_data_get(device, NULL, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_data_set(device, NULL, NULL, 0));
        ASSERT_EQ(errorCode, icsneoc2_message_is_transmit(device, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_is_valid(device, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_netid_get(device, NULL, NULL));
        ASSERT_EQ(errorCode, icsneoc2_message_netid_set(device, NULL, 0));
        ASSERT_EQ(errorCode, icsneoc2_message_type_get(device, NULL, NULL));
        
        ASSERT_EQ(errorCode, icsneoc2_device_events_get(device, NULL, NULL));
    }
}

TEST(icsneoc2, test_icsneoc2_devicetype_t) {
    std::vector<_icsneoc2_devicetype_t> devicetypeValues = {
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
    };
    auto i = 0;
    for (const auto& devicetypeValue : devicetypeValues) {
        ASSERT_EQ(devicetypeValue, i++);
    }

    ASSERT_EQ(sizeof(icsneoc2_devicetype_t), sizeof(uint32_t));
}

TEST(icsneoc2, test_icsneoc2_msg_type_t) {
    std::vector<_icsneoc2_msg_type_t> msgtypeValues = {
        icsneoc2_msg_type_device,
        icsneoc2_msg_type_internal,
        icsneoc2_msg_type_bus,

        icsneoc2_msg_type_maxsize,
    };
    auto i = 0;
    for (const auto& msgtypeValue : msgtypeValues) {
        ASSERT_EQ(msgtypeValue, i++);
    }

    ASSERT_EQ(sizeof(icsneoc2_msg_type_t), sizeof(uint32_t));
}

TEST(icsneoc2, test_icsneoc2_msg_bus_type_t) {
    std::vector<_icsneoc2_msg_bus_type_t> msgbustypeValues = {
        icsneoc2_msg_bus_type_invalid,
        icsneoc2_msg_bus_type_internal, // Used for statuses that don't actually need to be transferred to the client application
        icsneoc2_msg_bus_type_can,
        icsneoc2_msg_bus_type_lin,
        icsneoc2_msg_bus_type_flexray,
        icsneoc2_msg_bus_type_most,
        icsneoc2_msg_bus_type_ethernet,
        icsneoc2_msg_bus_type_lsftcan,
        icsneoc2_msg_bus_type_swcan,
        icsneoc2_msg_bus_type_iso9141,
        icsneoc2_msg_bus_type_i2c,
        icsneoc2_msg_bus_type_a2b,
        icsneoc2_msg_bus_type_spi,
        icsneoc2_msg_bus_type_mdio,

        // Must be last entry
        icsneoc2_msg_bus_type_maxsize,
    };
    auto i = 0;
    for (const auto& msgbustypeValue : msgbustypeValues) {
        ASSERT_EQ(msgbustypeValue, i++);
    }

    ASSERT_EQ(sizeof(icsneoc2_msg_bus_type_t), sizeof(uint8_t));
}

TEST(icsneoc2, test_icsneoc2_netid_t) {
    std::vector<std::tuple<_icsneoc2_netid_t, icsneoc2_netid_t>> netidValues = {
        { icsneoc2_netid_device, 0 },
        { icsneoc2_netid_hscan, 1 },
        { icsneoc2_netid_mscan, 2 },
        { icsneoc2_netid_swcan, 3 },
        { icsneoc2_netid_lsftcan, 4 },
        { icsneoc2_netid_fordscp, 5 },
        { icsneoc2_netid_j1708, 6 },
        { icsneoc2_netid_aux, 7 },
        { icsneoc2_netid_j1850vpw, 8 },
        { icsneoc2_netid_iso9141, 9 },
        { icsneoc2_netid_disk_data, 10 },
        { icsneoc2_netid_main51, 11 },
        { icsneoc2_netid_red, 12 },
        { icsneoc2_netid_sci, 13 },
        { icsneoc2_netid_iso9141_2, 14 },
        { icsneoc2_netid_iso14230, 15 },
        { icsneoc2_netid_lin, 16 },
        { icsneoc2_netid_op_ethernet1, 17 },
        { icsneoc2_netid_op_ethernet2, 18 },
        { icsneoc2_netid_op_ethernet3, 19 },
        { icsneoc2_netid_red_ext_memoryread, 20 },
        { icsneoc2_netid_red_int_memoryread, 21 },
        { icsneoc2_netid_red_dflash_read, 22 },
        { icsneoc2_netid_neo_memory_sdread, 23 }, // Response from NeoMemory (MemoryTypeSD)
        { icsneoc2_netid_can_errbits, 24 },
        { icsneoc2_netid_neo_memory_write_done, 25 },
        { icsneoc2_netid_red_wave_can1_logical, 26 },
        { icsneoc2_netid_red_wave_can2_logical, 27 },
        { icsneoc2_netid_red_wave_lin1_logical, 28 },
        { icsneoc2_netid_red_wave_lin2_logical, 29 },
        { icsneoc2_netid_red_wave_lin1_analog, 30 },
        { icsneoc2_netid_red_wave_lin2_analog, 31 },
        { icsneoc2_netid_red_wave_misc_analog, 32 },
        { icsneoc2_netid_red_wave_miscdio2_logical, 33 },
        { icsneoc2_netid_red_network_com_enable_ex, 34 },
        { icsneoc2_netid_red_neovi_network, 35 },
        { icsneoc2_netid_red_read_baud_settings, 36 },
        { icsneoc2_netid_red_oldformat, 37 },
        { icsneoc2_netid_red_scope_capture, 38 },
        { icsneoc2_netid_red_hardware_excep, 39 },
        { icsneoc2_netid_red_get_rtc, 40 },
        { icsneoc2_netid_iso9141_3, 41 },
        { icsneoc2_netid_hscan2, 42 },
        { icsneoc2_netid_hscan3, 44 },
        { icsneoc2_netid_op_ethernet4, 45 },
        { icsneoc2_netid_op_ethernet5, 46 },
        { icsneoc2_netid_iso9141_4, 47 },
        { icsneoc2_netid_lin2, 48 },
        { icsneoc2_netid_lin3, 49 },
        { icsneoc2_netid_lin4, 50 },
        { icsneoc2_netid_most_unused, 51 }, // MOST = 51, Old and unused
        { icsneoc2_netid_red_app_error, 52 },
        { icsneoc2_netid_cgi, 53 },
        { icsneoc2_netid_reset_status, 54 },
        { icsneoc2_netid_fb_status, 55 },
        { icsneoc2_netid_app_signal_status, 56 },
        { icsneoc2_netid_read_datalink_cm_tx_msg, 57 },
        { icsneoc2_netid_read_datalink_cm_rx_msg, 58 },
        { icsneoc2_netid_logging_overflow, 59 },
        { icsneoc2_netid_read_settings, 60 },
        { icsneoc2_netid_hscan4, 61 },
        { icsneoc2_netid_hscan5, 62 },
        { icsneoc2_netid_rs232, 63 },
        { icsneoc2_netid_uart, 64 },
        { icsneoc2_netid_uart2, 65 },
        { icsneoc2_netid_uart3, 66 },
        { icsneoc2_netid_uart4, 67 },
        { icsneoc2_netid_swcan2, 68 },
        { icsneoc2_netid_ethernet_daq, 69 },
        { icsneoc2_netid_data_to_host, 70 },
        { icsneoc2_netid_textapi_to_host, 71 },
        { icsneoc2_netid_spi1, 72 },
        { icsneoc2_netid_op_ethernet6, 73 },
        { icsneoc2_netid_red_vbat, 74 },
        { icsneoc2_netid_op_ethernet7, 75 },
        { icsneoc2_netid_op_ethernet8, 76 },
        { icsneoc2_netid_op_ethernet9, 77 },
        { icsneoc2_netid_op_ethernet10, 78 },
        { icsneoc2_netid_op_ethernet11, 79 },
        { icsneoc2_netid_flexray1a, 80 },
        { icsneoc2_netid_flexray1b, 81 },
        { icsneoc2_netid_flexray2a, 82 },
        { icsneoc2_netid_flexray2b, 83 },
        { icsneoc2_netid_lin5, 84 },
        { icsneoc2_netid_flexray, 85 },
        { icsneoc2_netid_flexray2, 86 },
        { icsneoc2_netid_op_ethernet12, 87 },
        { icsneoc2_netid_i2c, 88 },
        { icsneoc2_netid_most25, 90 },
        { icsneoc2_netid_most50, 91 },
        { icsneoc2_netid_most150, 92 },
        { icsneoc2_netid_ethernet, 93 },
        { icsneoc2_netid_gmfsa, 94 },
        { icsneoc2_netid_tcp, 95 },
        { icsneoc2_netid_hscan6, 96 },
        { icsneoc2_netid_hscan7, 97 },
        { icsneoc2_netid_lin6, 98 },
        { icsneoc2_netid_lsftcan2, 99 },
        { icsneoc2_netid_logical_disk_info, 187 },
        { icsneoc2_netid_wivi_command, 221 },
        { icsneoc2_netid_script_status, 224 },
        { icsneoc2_netid_eth_phy_control, 239 },
        { icsneoc2_netid_extended_command, 240 },
        { icsneoc2_netid_extended_data, 242 },
        { icsneoc2_netid_flexray_control, 243 },
        { icsneoc2_netid_coremini_preload, 244 },
        { icsneoc2_netid_hw_com_latency_test, 512 },
        { icsneoc2_netid_device_status, 513 },
        { icsneoc2_netid_udp, 514 },
        { icsneoc2_netid_forwarded_message, 516 },
        { icsneoc2_netid_i2c2, 517 },
        { icsneoc2_netid_i2c3, 518 },
        { icsneoc2_netid_i2c4, 519 },
        { icsneoc2_netid_ethernet2, 520 },
        { icsneoc2_netid_a2b1, 522 },
        { icsneoc2_netid_a2b2, 523 },
        { icsneoc2_netid_ethernet3, 524 },
        { icsneoc2_netid_wbms, 532 },
        { icsneoc2_netid_dwcan9, 534 },
        { icsneoc2_netid_dwcan10, 535 },
        { icsneoc2_netid_dwcan11, 536 },
        { icsneoc2_netid_dwcan12, 537 },
        { icsneoc2_netid_dwcan13, 538 },
        { icsneoc2_netid_dwcan14, 539 },
        { icsneoc2_netid_dwcan15, 540 },
        { icsneoc2_netid_dwcan16, 541 },
        { icsneoc2_netid_lin7, 542 },
        { icsneoc2_netid_lin8, 543 },
        { icsneoc2_netid_spi2, 544 },
        { icsneoc2_netid_mdio1, 545 },
        { icsneoc2_netid_mdio2, 546 },
        { icsneoc2_netid_mdio3, 547 },
        { icsneoc2_netid_mdio4, 548 },
        { icsneoc2_netid_mdio5, 549 },
        { icsneoc2_netid_mdio6, 550 },
        { icsneoc2_netid_mdio7, 551 },
        { icsneoc2_netid_mdio8, 552 },
        { icsneoc2_netid_op_ethernet13, 553 },
        { icsneoc2_netid_op_ethernet14, 554 },
        { icsneoc2_netid_op_ethernet15, 555 },
        { icsneoc2_netid_op_ethernet16, 556 },
        { icsneoc2_netid_spi3, 557 },
        { icsneoc2_netid_spi4, 558 },
        { icsneoc2_netid_spi5, 559 },
        { icsneoc2_netid_spi6, 560 },
        { icsneoc2_netid_spi7, 561 },
        { icsneoc2_netid_spi8, 562 },
        { icsneoc2_netid_lin9, 563 },
        { icsneoc2_netid_lin10, 564 },
        { icsneoc2_netid_lin11, 565 },
        { icsneoc2_netid_lin12, 566 },
        { icsneoc2_netid_lin13, 567 },
        { icsneoc2_netid_lin14, 568 },
        { icsneoc2_netid_lin15, 569 },
        { icsneoc2_netid_lin16, 570 },
        { icsneoc2_netid_maxsize, 571 }
    };

    for (const auto& netidValue : netidValues) {
        const _icsneoc2_netid_t actualValue = std::get<0>(netidValue);
        const icsneoc2_netid_t expectedValue = std::get<1>(netidValue);
        ASSERT_EQ(actualValue, expectedValue);
    } 
}