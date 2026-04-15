#include <gtest/gtest.h>
#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2settings.h>
#include <icsneo/icsneoc2messages.h>
#include <icsneo/device/devicetype.h>
#include <icsneo/communication/message/linmessage.h>


#include <vector>
#include <tuple>

// Forward declare the safe_str_copy function for testing
extern bool safe_str_copy(char* dest, size_t* dest_size, std::string_view src);

TEST(icsneoc2, test_safe_str_copy)
{
	char buffer[10] = {0};
	size_t buffer_size = sizeof(buffer);

	// Test normal copy
	ASSERT_TRUE(safe_str_copy(buffer, &buffer_size, "test"));
	ASSERT_STREQ(buffer, "test");
	ASSERT_EQ(buffer_size, 4);

	// Test copy with string too long - buffer should be null terminated, buffer_size should reflect truncated length
	buffer_size = sizeof(buffer);
	ASSERT_FALSE(safe_str_copy(buffer, &buffer_size, "this is a very long string")) << buffer << ": " << buffer_size;
	ASSERT_EQ(buffer[sizeof(buffer) - 1], '\0');
	ASSERT_STREQ(buffer, "this is a");
	ASSERT_EQ(buffer_size, 9); // truncated to buffer capacity - 1
	ASSERT_EQ(buffer[sizeof(buffer) - 1], '\0');

	// Test null parameters - buffer_size should be unmodified on failure
	buffer_size = sizeof(buffer);
	ASSERT_FALSE(safe_str_copy(NULL, &buffer_size, "test"));
	ASSERT_EQ(buffer_size, sizeof(buffer));
	ASSERT_FALSE(safe_str_copy(buffer, NULL, "test"));
	ASSERT_FALSE(safe_str_copy(NULL, NULL, "test"));

	// Test empty string
	buffer_size = sizeof(buffer);
	memset(buffer, 0xCA, sizeof(buffer));
	ASSERT_TRUE(safe_str_copy(buffer, &buffer_size, ""));
	ASSERT_STREQ(buffer, "");
	ASSERT_EQ(buffer_size, 0);
	// Make sure the rest of the buffer is untouched (should be 0xCA)
	for (size_t i = buffer_size + 1; i < sizeof(buffer) - 1; ++i)
	{
		ASSERT_EQ(buffer[i], '\xCA') << i << ") '" << buffer << "': " << buffer_size;
	}

	// Test string that fits exactly
	buffer_size = sizeof(buffer);
	ASSERT_TRUE(safe_str_copy(buffer, &buffer_size, "123456789"));
	ASSERT_STREQ(buffer, "123456789");
	ASSERT_EQ(buffer_size, 9);
	ASSERT_EQ(buffer[sizeof(buffer) - 1], '\0');

	// Test zero-size buffer - should always fail and write nothing
	char tiny0[1] = {0};
	buffer_size = 0;
	ASSERT_FALSE(safe_str_copy(tiny0, &buffer_size, "test"));
	ASSERT_FALSE(safe_str_copy(tiny0, &buffer_size, ""));

	// Test single-byte buffer (room for null terminator only)
	char tiny1[1];
	buffer_size = sizeof(tiny1);
	memset(tiny1, 0xFF, sizeof(tiny1));
	ASSERT_FALSE(safe_str_copy(tiny1, &buffer_size, "test")); // non-empty: truncates to just '\0'
	ASSERT_EQ(tiny1[0], '\0');

	// Test single-byte buffer with empty string - should succeed and write just the null terminator
	buffer_size = sizeof(tiny1);
	memset(tiny1, 0xFF, sizeof(tiny1));
	ASSERT_TRUE(safe_str_copy(tiny1, &buffer_size, "")); // empty string fits exactly
	ASSERT_EQ(tiny1[0], '\0');
	ASSERT_EQ(buffer_size, 0);
}

TEST(icsneoc2, test_icsneoc2_device_enumerate)
{
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_enumerate(0, NULL));

	icsneoc2_device_info_t* devices = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_device_enumerate(0, &devices));
	icsneoc2_enumeration_free(devices);
}

TEST(icsneoc2, test_icsneoc2_device_is_valid)
{
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_is_valid(NULL));
}

TEST(icsneoc2, test_icsneoc2_error_invalid_parameters_and_invalid_device)
{
	bool placeholderBool = false;
	uint8_t placeholderInteger8 = 0;
	uint32_t placeholderInteger32 = 0;
	int64_t placeholderInteger64 = 0;
	uint64_t placeholderUnsignedInteger64 = 0;
	size_t placeholderSizeT = 0;
	char placeholderStr[255] = {0};
	icsneoc2_event_t *eventPlaceHolder = nullptr;
	icsneoc2_lin_mode_t placeholderLinMode = 0;
	icsneoc2_ae_link_mode_t placeholderAeLinkMode = 0;
	icsneoc2_eth_phy_link_mode_t placeholderEthPhyLinkMode = 0;
	icsneoc2_eth_phy_link_mode_t* placeholderEthPhyLinkModePtr = nullptr;
	icsneoc2_misc_io_analog_voltage_t placeholderMiscIoAnalogVoltage = 0;

	// All of these don't have a device parameter
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_enumerate(0, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_network_type_name_get(0, NULL, NULL));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_event_description_get(eventPlaceHolder, placeholderStr, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_event_get(NULL, NULL));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_type_name_get(0, NULL, NULL));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_error_code_get(0, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_netid_name_get(0, NULL, NULL));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_can_create(NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_free(NULL));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_data_get(NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_data_set(NULL, NULL, 0));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_netid_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_netid_set(NULL, 0));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_network_type_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_is_transmit(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_is_valid(NULL, NULL));

	// LIN message NULL parameter checks
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_is_lin(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_lin_create(NULL, 0));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_lin_props_get(NULL, NULL, NULL, NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_lin_props_set(NULL, NULL, NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_lin_err_flags_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_lin_status_flags_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_lin_calc_checksum(NULL));

	// Test utility functions with NULL parameters
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_version_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_serial_num_to_string(0, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_serial_string_to_num(NULL, 0, NULL));

	// Test new enumeration/info accessors with NULL parameters
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_info_serial_get(NULL, placeholderStr, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_info_type_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_info_type_name_get(NULL, placeholderStr, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_info_description_get(NULL, placeholderStr, &placeholderSizeT));

	// Test open/close with NULL parameters
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_create(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_free(NULL));
	
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_open(NULL, 0));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_open_serial(NULL, 0, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_open_first(0, 0, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_close(NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_description_get(NULL, placeholderStr, &placeholderSizeT));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_go_online(NULL, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_is_online(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_is_online_supported(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_is_valid(NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_is_open(NULL, &placeholderBool));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_rtc_get(NULL, (int64_t *)&placeholderUnsignedInteger64));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_rtc_set(NULL, 0));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_serial_get(NULL, placeholderStr, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_pcb_serial_get(NULL, &placeholderInteger8, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_mac_address_get(NULL, &placeholderInteger8, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_supports_tc10(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_timestamp_resolution_get(NULL, &placeholderInteger32));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_digital_io_get(NULL, 0, 0, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_digital_io_set(NULL, 0, 0, false));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_type_get(NULL, NULL));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_message_polling_limit_get(NULL, &placeholderInteger32));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_message_polling_limit_set(NULL, 0));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_message_get(NULL, NULL, 0));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_message_transmit(NULL, NULL));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_apply_defaults(NULL, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_apply(NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_refresh(NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_baudrate_get(NULL, 0, &placeholderInteger64));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_baudrate_set(NULL, 0, placeholderInteger64));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_canfd_baudrate_get(NULL, 0, &placeholderInteger64));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_canfd_baudrate_set(NULL, 0, placeholderInteger64));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_termination_is_supported(NULL, 0, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_termination_can_enable(NULL, 0, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_termination_is_enabled(NULL, 0, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_termination_set(NULL, 0, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_commander_resistor_enabled(NULL, 0, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_commander_resistor_set(NULL, 0, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_lin_mode_get(NULL, 0, &placeholderLinMode));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_lin_mode_set(NULL, 0, placeholderLinMode));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_lin_commander_response_time_get(NULL, 0, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_lin_commander_response_time_set(NULL, 0, placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_enable_get(NULL, 0, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_enable_set(NULL, 0, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_mode_get(NULL, 0, &placeholderAeLinkMode));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_mode_set(NULL, 0, placeholderAeLinkMode));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_speed_get(NULL, 0, &placeholderEthPhyLinkMode));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_speed_set(NULL, 0, placeholderEthPhyLinkMode));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_role_for_get(NULL, 0, &placeholderAeLinkMode));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_role_for_set(NULL, 0, placeholderAeLinkMode));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_link_mode_for_get(NULL, 0, &placeholderEthPhyLinkMode));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_link_mode_for_set(NULL, 0, placeholderEthPhyLinkMode));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_enable_for_get(NULL, 0, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_phy_enable_for_set(NULL, 0, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_supported_phy_link_modes_for(NULL, 0, &placeholderEthPhyLinkModePtr, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_is_plca_enabled_for(NULL, 0, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_plca_enabled_for_set(NULL, 0, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_local_id_get(NULL, 0, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_local_id_set(NULL, 0, placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_max_nodes_get(NULL, 0, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_max_nodes_set(NULL, 0, placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_tx_opp_timer_get(NULL, 0, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_tx_opp_timer_set(NULL, 0, placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_max_burst_timer_for_get(NULL, 0, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_max_burst_timer_for_set(NULL, 0, placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_misc_io_analog_output_enabled_set(NULL, 0, placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_misc_io_analog_output_set(NULL, 0, placeholderMiscIoAnalogVoltage));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_disabled_get(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_readonly_get(NULL, &placeholderBool));

	// Disk formatting functions
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_disk_count_get(NULL, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_supports_disk_formatting(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_disk_details_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_format_disk(NULL, NULL, NULL, NULL));

	// Disk details accessors with NULL details
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_count_get(NULL, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_count_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_layout_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_layout_set(NULL, 0));
	icsneoc2_disk_format_flags_t placeholderFlags = 0;
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_flags_get(NULL, 0, &placeholderFlags));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_flags_get(NULL, 0, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_flags_set(NULL, 0, placeholderFlags));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_flags_set(NULL, 0, 0));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_size_get(NULL, 0, &placeholderUnsignedInteger64, &placeholderUnsignedInteger64));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_size_get(NULL, 0, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_full_format_get(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_full_format_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_disk_details_full_format_set(NULL, false));

	// Supported networks
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_supported_rx_networks_get(NULL, NULL, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_supported_rx_networks_get(NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_supported_tx_networks_get(NULL, NULL, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_supported_tx_networks_get(NULL, NULL, NULL));

	// Script functions
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_supports_coremini_script(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_supports_coremini_script(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_script_start(NULL, icsneoc2_memory_type_sd));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_script_stop(NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_script_clear(NULL, icsneoc2_memory_type_sd));
	int8_t placeholderSignedInteger8 = 0;
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_script_prepare_load(NULL, &placeholderSignedInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_script_prepare_load(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_coremini_upload_file(NULL, "test.bin", icsneoc2_memory_type_sd));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_coremini_upload_file(NULL, NULL, icsneoc2_memory_type_sd));
	const uint8_t placeholderData[] = {0};
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_coremini_upload(NULL, placeholderData, sizeof(placeholderData), icsneoc2_memory_type_sd));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_coremini_upload(NULL, NULL, 0, icsneoc2_memory_type_sd));
	icsneoc2_script_status_t* placeholderScriptStatus = NULL;
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_script_status_get(NULL, &placeholderScriptStatus));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_script_status_get(NULL, NULL));

	// Script status accessors with NULL handle
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_is_coremini_running(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_is_coremini_running(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_is_encrypted(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_sector_overflows_get(NULL, &placeholderInteger32));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_sector_overflows_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_remaining_sector_buffers_get(NULL, &placeholderInteger32));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_last_sector_get(NULL, &placeholderInteger32));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_read_bin_size_get(NULL, &placeholderInteger32));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_min_sector_get(NULL, &placeholderInteger32));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_max_sector_get(NULL, &placeholderInteger32));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_current_sector_get(NULL, &placeholderInteger32));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_coremini_create_time_get(NULL, &placeholderUnsignedInteger64));
	uint16_t placeholderInteger16 = 0;
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_file_checksum_get(NULL, &placeholderInteger16));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_coremini_version_get(NULL, &placeholderInteger16));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_coremini_header_size_get(NULL, &placeholderInteger16));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_diagnostic_error_code_get(NULL, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_diagnostic_error_code_count_get(NULL, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_script_status_max_coremini_size_kb_get(NULL, &placeholderInteger16));
}

TEST(icsneoc2, test_icsneoc2_devicetype_t)
{
	ASSERT_EQ(icsneoc2_devicetype_unknown, 0x00000000);
	ASSERT_EQ(icsneoc2_devicetype_blue, 0x00000001);
	ASSERT_EQ(icsneoc2_devicetype_ecu_avb, 0x00000002);
	ASSERT_EQ(icsneoc2_devicetype_rad_supermoon, 0x00000003);
	ASSERT_EQ(icsneoc2_devicetype_dw_vcan, 0x00000004);
	ASSERT_EQ(icsneoc2_devicetype_rad_moon2, 0x00000005);
	ASSERT_EQ(icsneoc2_devicetype_rad_mars, 0x00000006);
	ASSERT_EQ(icsneoc2_devicetype_vcan41, 0x00000007);
	ASSERT_EQ(icsneoc2_devicetype_fire, 0x00000008);
	ASSERT_EQ(icsneoc2_devicetype_rad_pluto, 0x00000009);
	ASSERT_EQ(icsneoc2_devicetype_vcan42_el, 0x0000000a);
	ASSERT_EQ(icsneoc2_devicetype_radio_canhub, 0x0000000b);
	ASSERT_EQ(icsneoc2_devicetype_neo_ecu12, 0x0000000c);
	ASSERT_EQ(icsneoc2_devicetype_obd2_lc_badge, 0x0000000d);
	ASSERT_EQ(icsneoc2_devicetype_rad_moon_duo, 0x0000000e);
	ASSERT_EQ(icsneoc2_devicetype_fire3, 0x0000000f);
	ASSERT_EQ(icsneoc2_devicetype_vcan3, 0x00000010);
	ASSERT_EQ(icsneoc2_devicetype_rad_jupiter, 0x00000011);
	ASSERT_EQ(icsneoc2_devicetype_vcan4_industrial, 0x00000012);
	ASSERT_EQ(icsneoc2_devicetype_rad_gigastar, 0x00000013);
	ASSERT_EQ(icsneoc2_devicetype_red2, 0x00000014);
	ASSERT_EQ(icsneoc2_devicetype_etherbadge, 0x00000016);
	ASSERT_EQ(icsneoc2_devicetype_rad_a2b, 0x00000017);
	ASSERT_EQ(icsneoc2_devicetype_rad_epsilon, 0x00000018);
	ASSERT_EQ(icsneoc2_devicetype_rad_epsilon_xl, 0x0000001e);
	ASSERT_EQ(icsneoc2_devicetype_rad_galaxy2, 0x00000021);
	ASSERT_EQ(icsneoc2_devicetype_rad_moon3, 0x00000023);
	ASSERT_EQ(icsneoc2_devicetype_rad_comet, 0x00000024);
	ASSERT_EQ(icsneoc2_devicetype_fire3_flexray, 0x00000025);
	ASSERT_EQ(icsneoc2_devicetype_connect, 0x00000026);
	ASSERT_EQ(icsneoc2_devicetype_rad_comet3, 0x00000027);
	ASSERT_EQ(icsneoc2_devicetype_rad_moon_t1s, 0x00000028);
	ASSERT_EQ(icsneoc2_devicetype_rad_gigastar2, 0x00000029);
	ASSERT_EQ(icsneoc2_devicetype_fire3_t1s_lin, 0x0000002a);
	ASSERT_EQ(icsneoc2_devicetype_fire3_t1s_sent, 0x0000002b);
	ASSERT_EQ(icsneoc2_devicetype_rad_gemini, 0x0000002c);
	ASSERT_EQ(icsneoc2_devicetype_red, 0x00000040);
	ASSERT_EQ(icsneoc2_devicetype_ecu, 0x00000080);
	ASSERT_EQ(icsneoc2_devicetype_ievb, 0x00000100);
	ASSERT_EQ(icsneoc2_devicetype_pendant, 0x00000200);
	ASSERT_EQ(icsneoc2_devicetype_obd2_pro, 0x00000400);
	ASSERT_EQ(icsneoc2_devicetype_ecuchip_uart, 0x00000800);
	ASSERT_EQ(icsneoc2_devicetype_plasma, 0x00001000);
	ASSERT_EQ(icsneoc2_devicetype_neo_analog, 0x00004000);
	ASSERT_EQ(icsneoc2_devicetype_ct_obd, 0x00008000);
	ASSERT_EQ(icsneoc2_devicetype_ion, 0x00040000);
	ASSERT_EQ(icsneoc2_devicetype_rad_star, 0x00080000);
	ASSERT_EQ(icsneoc2_devicetype_vcan44, 0x00200000);
	ASSERT_EQ(icsneoc2_devicetype_vcan42, 0x00400000);
	ASSERT_EQ(icsneoc2_devicetype_cm_probe, 0x00800000);
	ASSERT_EQ(icsneoc2_devicetype_eevb, 0x01000000);
	ASSERT_EQ(icsneoc2_devicetype_vcan_rf, 0x02000000);
	ASSERT_EQ(icsneoc2_devicetype_fire2, 0x04000000);
	ASSERT_EQ(icsneoc2_devicetype_flex, 0x08000000);
	ASSERT_EQ(icsneoc2_devicetype_rad_galaxy, 0x10000000);
	ASSERT_EQ(icsneoc2_devicetype_rad_star2, 0x20000000);
	ASSERT_EQ(icsneoc2_devicetype_vividcan, 0x40000000);
	ASSERT_EQ(icsneoc2_devicetype_obd2_sim, 0x80000000);
	ASSERT_EQ(icsneoc2_devicetype_maxsize, 0x80000001);
	ASSERT_EQ(sizeof(icsneoc2_devicetype_t), sizeof(uint32_t));
}

TEST(icsneoc2, test_devicetype_enum_alignment)
{
	ASSERT_EQ(icsneo::DeviceType::Unknown, icsneoc2_devicetype_unknown);
	ASSERT_EQ(icsneo::DeviceType::BLUE, icsneoc2_devicetype_blue);
	ASSERT_EQ(icsneo::DeviceType::ECU_AVB, icsneoc2_devicetype_ecu_avb);
	ASSERT_EQ(icsneo::DeviceType::RADSupermoon, icsneoc2_devicetype_rad_supermoon);
	ASSERT_EQ(icsneo::DeviceType::DW_VCAN, icsneoc2_devicetype_dw_vcan);
	ASSERT_EQ(icsneo::DeviceType::RADMoon2, icsneoc2_devicetype_rad_moon2);
	ASSERT_EQ(icsneo::DeviceType::RADMars, icsneoc2_devicetype_rad_mars);
	ASSERT_EQ(icsneo::DeviceType::VCAN4_1, icsneoc2_devicetype_vcan41);
	ASSERT_EQ(icsneo::DeviceType::FIRE, icsneoc2_devicetype_fire);
	ASSERT_EQ(icsneo::DeviceType::RADPluto, icsneoc2_devicetype_rad_pluto);
	ASSERT_EQ(icsneo::DeviceType::VCAN4_2EL, icsneoc2_devicetype_vcan42_el);
	ASSERT_EQ(icsneo::DeviceType::RADIO_CANHUB, icsneoc2_devicetype_radio_canhub);
	ASSERT_EQ(icsneo::DeviceType::NEOECU12, icsneoc2_devicetype_neo_ecu12);
	ASSERT_EQ(icsneo::DeviceType::OBD2_LCBADGE, icsneoc2_devicetype_obd2_lc_badge);
	ASSERT_EQ(icsneo::DeviceType::RADMoonDuo, icsneoc2_devicetype_rad_moon_duo);
	ASSERT_EQ(icsneo::DeviceType::FIRE3, icsneoc2_devicetype_fire3);
	ASSERT_EQ(icsneo::DeviceType::VCAN3, icsneoc2_devicetype_vcan3);
	ASSERT_EQ(icsneo::DeviceType::RADJupiter, icsneoc2_devicetype_rad_jupiter);
	ASSERT_EQ(icsneo::DeviceType::VCAN4_IND, icsneoc2_devicetype_vcan4_industrial);
	ASSERT_EQ(icsneo::DeviceType::RADGigastar, icsneoc2_devicetype_rad_gigastar);
	ASSERT_EQ(icsneo::DeviceType::RED2, icsneoc2_devicetype_red2);
	ASSERT_EQ(icsneo::DeviceType::EtherBADGE, icsneoc2_devicetype_etherbadge);
	ASSERT_EQ(icsneo::DeviceType::RAD_A2B, icsneoc2_devicetype_rad_a2b);
	ASSERT_EQ(icsneo::DeviceType::RADEpsilon, icsneoc2_devicetype_rad_epsilon);
	ASSERT_EQ(icsneo::DeviceType::RADEpsilonXL, icsneoc2_devicetype_rad_epsilon_xl);
	ASSERT_EQ(icsneo::DeviceType::RADMoon3, icsneoc2_devicetype_rad_moon3);
	ASSERT_EQ(icsneo::DeviceType::RADComet, icsneoc2_devicetype_rad_comet);
	ASSERT_EQ(icsneo::DeviceType::FIRE3_FlexRay, icsneoc2_devicetype_fire3_flexray);
	ASSERT_EQ(icsneo::DeviceType::Connect, icsneoc2_devicetype_connect);
	ASSERT_EQ(icsneo::DeviceType::RADComet3, icsneoc2_devicetype_rad_comet3);
	ASSERT_EQ(icsneo::DeviceType::RADMoonT1S, icsneoc2_devicetype_rad_moon_t1s);
	ASSERT_EQ(icsneo::DeviceType::RADGigastar2, icsneoc2_devicetype_rad_gigastar2);
	ASSERT_EQ(icsneo::DeviceType::FIRE3_T1S_LIN, icsneoc2_devicetype_fire3_t1s_lin);
	ASSERT_EQ(icsneo::DeviceType::FIRE3_T1S_SENT, icsneoc2_devicetype_fire3_t1s_sent);
	ASSERT_EQ(icsneo::DeviceType::RADGemini, icsneoc2_devicetype_rad_gemini);
	ASSERT_EQ(icsneo::DeviceType::RED, icsneoc2_devicetype_red);
	ASSERT_EQ(icsneo::DeviceType::ECU, icsneoc2_devicetype_ecu);
	ASSERT_EQ(icsneo::DeviceType::IEVB, icsneoc2_devicetype_ievb);
	ASSERT_EQ(icsneo::DeviceType::Pendant, icsneoc2_devicetype_pendant);
	ASSERT_EQ(icsneo::DeviceType::OBD2_PRO, icsneoc2_devicetype_obd2_pro);
	ASSERT_EQ(icsneo::DeviceType::ECUChip_UART, icsneoc2_devicetype_ecuchip_uart);
	ASSERT_EQ(icsneo::DeviceType::PLASMA, icsneoc2_devicetype_plasma);
	ASSERT_EQ(icsneo::DeviceType::NEOAnalog, icsneoc2_devicetype_neo_analog);
	ASSERT_EQ(icsneo::DeviceType::CT_OBD, icsneoc2_devicetype_ct_obd);
	ASSERT_EQ(icsneo::DeviceType::ION, icsneoc2_devicetype_ion);
	ASSERT_EQ(icsneo::DeviceType::RADStar, icsneoc2_devicetype_rad_star);
	ASSERT_EQ(icsneo::DeviceType::VCAN4_4, icsneoc2_devicetype_vcan44);
	ASSERT_EQ(icsneo::DeviceType::VCAN4_2, icsneoc2_devicetype_vcan42);
	ASSERT_EQ(icsneo::DeviceType::CMProbe, icsneoc2_devicetype_cm_probe);
	ASSERT_EQ(icsneo::DeviceType::EEVB, icsneoc2_devicetype_eevb);
	ASSERT_EQ(icsneo::DeviceType::VCANrf, icsneoc2_devicetype_vcan_rf);
	ASSERT_EQ(icsneo::DeviceType::FIRE2, icsneoc2_devicetype_fire2);
	ASSERT_EQ(icsneo::DeviceType::Flex, icsneoc2_devicetype_flex);
	ASSERT_EQ(icsneo::DeviceType::RADGalaxy, icsneoc2_devicetype_rad_galaxy);
	ASSERT_EQ(icsneo::DeviceType::RADStar2, icsneoc2_devicetype_rad_star2);
	ASSERT_EQ(icsneo::DeviceType::VividCAN, icsneoc2_devicetype_vividcan);
	ASSERT_EQ(icsneo::DeviceType::OBD2_SIM, icsneoc2_devicetype_obd2_sim);
	ASSERT_EQ(icsneo::DeviceType::RADGalaxy2, icsneoc2_devicetype_rad_galaxy2);
}

TEST(icsneoc2, icsneoc2_network_type_t)
{
	ASSERT_EQ(icsneoc2_network_type_invalid, 0);
	ASSERT_EQ(icsneoc2_network_type_internal, 1);
	ASSERT_EQ(icsneoc2_network_type_can, 2);
	ASSERT_EQ(icsneoc2_network_type_lin, 3);
	ASSERT_EQ(icsneoc2_network_type_flexray, 4);
	ASSERT_EQ(icsneoc2_network_type_most, 5);
	ASSERT_EQ(icsneoc2_network_type_ethernet, 6);
	ASSERT_EQ(icsneoc2_network_type_lsftcan, 7);
	ASSERT_EQ(icsneoc2_network_type_swcan, 8);
	ASSERT_EQ(icsneoc2_network_type_iso9141, 9);
	ASSERT_EQ(icsneoc2_network_type_i2c, 10);
	ASSERT_EQ(icsneoc2_network_type_a2b, 11);
	ASSERT_EQ(icsneoc2_network_type_spi, 12);
	ASSERT_EQ(icsneoc2_network_type_mdio, 13);
	ASSERT_EQ(icsneoc2_network_type_automotive_ethernet, 14);
	ASSERT_EQ(icsneoc2_network_type_maxsize, 15);

	ASSERT_EQ(sizeof(icsneoc2_network_type_t), sizeof(uint8_t));
}

TEST(icsneoc2, test_icsneoc2_io_type_t)
{
	ASSERT_EQ(icsneoc2_io_type_eth_activation, 0);
	ASSERT_EQ(icsneoc2_io_type_usb_host_power, 1);
	ASSERT_EQ(icsneoc2_io_type_backup_power_en, 2);
	ASSERT_EQ(icsneoc2_io_type_backup_power_good, 3);
	ASSERT_EQ(icsneoc2_io_type_misc, 4);
	ASSERT_EQ(icsneoc2_io_type_emisc, 5);

	ASSERT_EQ(sizeof(icsneoc2_io_type_t), sizeof(uint32_t));
}

TEST(icsneoc2, test_icsneoc2_netid_t)
{
	ASSERT_EQ(icsneoc2_netid_device, 0);
	ASSERT_EQ(icsneoc2_netid_dwcan_01, 1);
	ASSERT_EQ(icsneoc2_netid_dwcan_08, 2);
	ASSERT_EQ(icsneoc2_netid_swcan_01, 3);
	ASSERT_EQ(icsneoc2_netid_lsftcan_01, 4);
	ASSERT_EQ(icsneoc2_netid_fordscp, 5);
	ASSERT_EQ(icsneoc2_netid_j1708, 6);
	ASSERT_EQ(icsneoc2_netid_aux, 7);
	ASSERT_EQ(icsneoc2_netid_j1850vpw, 8);
	ASSERT_EQ(icsneoc2_netid_iso9141, 9);
	ASSERT_EQ(icsneoc2_netid_disk_data, 10);
	ASSERT_EQ(icsneoc2_netid_main51, 11);
	ASSERT_EQ(icsneoc2_netid_red, 12);
	ASSERT_EQ(icsneoc2_netid_sci, 13);
	ASSERT_EQ(icsneoc2_netid_iso9141_02, 14);
	ASSERT_EQ(icsneoc2_netid_iso14230, 15);
	ASSERT_EQ(icsneoc2_netid_lin_01, 16);
	ASSERT_EQ(icsneoc2_netid_ae_01, 17);
	ASSERT_EQ(icsneoc2_netid_ae_02, 18);
	ASSERT_EQ(icsneoc2_netid_ae_03, 19);
	ASSERT_EQ(icsneoc2_netid_red_ext_memoryread, 20);
	ASSERT_EQ(icsneoc2_netid_red_int_memoryread, 21);
	ASSERT_EQ(icsneoc2_netid_red_dflash_read, 22);
	ASSERT_EQ(icsneoc2_netid_neo_memory_sdread, 23); // Response from NeoMemory (MemoryTypeSD)
	ASSERT_EQ(icsneoc2_netid_can_errbits, 24);
	ASSERT_EQ(icsneoc2_netid_neo_memory_write_done, 25);
	ASSERT_EQ(icsneoc2_netid_red_wave_can1_logical, 26);
	ASSERT_EQ(icsneoc2_netid_red_wave_can2_logical, 27);
	ASSERT_EQ(icsneoc2_netid_red_wave_lin1_logical, 28);
	ASSERT_EQ(icsneoc2_netid_red_wave_lin2_logical, 29);
	ASSERT_EQ(icsneoc2_netid_red_wave_lin1_analog, 30);
	ASSERT_EQ(icsneoc2_netid_red_wave_lin2_analog, 31);
	ASSERT_EQ(icsneoc2_netid_red_wave_misc_analog, 32);
	ASSERT_EQ(icsneoc2_netid_red_wave_miscdio2_logical, 33);
	ASSERT_EQ(icsneoc2_netid_red_network_com_enable_ex, 34);
	ASSERT_EQ(icsneoc2_netid_red_neovi_network, 35);
	ASSERT_EQ(icsneoc2_netid_red_read_baud_settings, 36);
	ASSERT_EQ(icsneoc2_netid_red_oldformat, 37);
	ASSERT_EQ(icsneoc2_netid_red_scope_capture, 38);
	ASSERT_EQ(icsneoc2_netid_red_hardware_excep, 39);
	ASSERT_EQ(icsneoc2_netid_red_get_rtc, 40);
	ASSERT_EQ(icsneoc2_netid_iso9141_03, 41);
	ASSERT_EQ(icsneoc2_netid_dwcan_02, 42);
	ASSERT_EQ(icsneoc2_netid_dwcan_03, 44);
	ASSERT_EQ(icsneoc2_netid_ae_04, 45);
	ASSERT_EQ(icsneoc2_netid_ae_05, 46);
	ASSERT_EQ(icsneoc2_netid_iso9141_04, 47);
	ASSERT_EQ(icsneoc2_netid_lin_02, 48);
	ASSERT_EQ(icsneoc2_netid_lin_03, 49);
	ASSERT_EQ(icsneoc2_netid_lin_04, 50);
	ASSERT_EQ(icsneoc2_netid_most_unused, 51); // MOST = 51, Old and unused
	ASSERT_EQ(icsneoc2_netid_red_app_error, 52);
	ASSERT_EQ(icsneoc2_netid_cgi, 53);
	ASSERT_EQ(icsneoc2_netid_reset_status, 54);
	ASSERT_EQ(icsneoc2_netid_fb_status, 55);
	ASSERT_EQ(icsneoc2_netid_app_signal_status, 56);
	ASSERT_EQ(icsneoc2_netid_read_datalink_cm_tx_msg, 57);
	ASSERT_EQ(icsneoc2_netid_read_datalink_cm_rx_msg, 58);
	ASSERT_EQ(icsneoc2_netid_logging_overflow, 59);
	ASSERT_EQ(icsneoc2_netid_read_settings, 60);
	ASSERT_EQ(icsneoc2_netid_dwcan_04, 61);
	ASSERT_EQ(icsneoc2_netid_dwcan_05, 62);
	ASSERT_EQ(icsneoc2_netid_rs232, 63);
	ASSERT_EQ(icsneoc2_netid_uart_01, 64);
	ASSERT_EQ(icsneoc2_netid_uart_02, 65);
	ASSERT_EQ(icsneoc2_netid_uart_03, 66);
	ASSERT_EQ(icsneoc2_netid_uart_04, 67);
	ASSERT_EQ(icsneoc2_netid_swcan_02, 68);
	ASSERT_EQ(icsneoc2_netid_ethernet_daq, 69);
	ASSERT_EQ(icsneoc2_netid_data_to_host, 70);
	ASSERT_EQ(icsneoc2_netid_textapi_to_host, 71);
	ASSERT_EQ(icsneoc2_netid_spi_01, 72);
	ASSERT_EQ(icsneoc2_netid_ae_06, 73);
	ASSERT_EQ(icsneoc2_netid_red_vbat, 74);
	ASSERT_EQ(icsneoc2_netid_ae_07, 75);
	ASSERT_EQ(icsneoc2_netid_ae_08, 76);
	ASSERT_EQ(icsneoc2_netid_ae_09, 77);
	ASSERT_EQ(icsneoc2_netid_ae_10, 78);
	ASSERT_EQ(icsneoc2_netid_ae_11, 79);
	ASSERT_EQ(icsneoc2_netid_flexray_01a, 80);
	ASSERT_EQ(icsneoc2_netid_flexray_01b, 81);
	ASSERT_EQ(icsneoc2_netid_flexray_02a, 82);
	ASSERT_EQ(icsneoc2_netid_flexray_02b, 83);
	ASSERT_EQ(icsneoc2_netid_lin_05, 84);
	ASSERT_EQ(icsneoc2_netid_flexray_01, 85);
	ASSERT_EQ(icsneoc2_netid_flexray_02, 86);
	ASSERT_EQ(icsneoc2_netid_ae_12, 87);
	ASSERT_EQ(icsneoc2_netid_i2c_01, 88);
	ASSERT_EQ(icsneoc2_netid_most_25, 90);
	ASSERT_EQ(icsneoc2_netid_most_50, 91);
	ASSERT_EQ(icsneoc2_netid_most_150, 92);
	ASSERT_EQ(icsneoc2_netid_ethernet_01, 93);
	ASSERT_EQ(icsneoc2_netid_gmfsa, 94);
	ASSERT_EQ(icsneoc2_netid_tcp, 95);
	ASSERT_EQ(icsneoc2_netid_dwcan_06, 96);
	ASSERT_EQ(icsneoc2_netid_dwcan_07, 97);
	ASSERT_EQ(icsneoc2_netid_lin_06, 98);
	ASSERT_EQ(icsneoc2_netid_lsftcan_02, 99);
	ASSERT_EQ(icsneoc2_netid_logical_disk_info, 187);
	ASSERT_EQ(icsneoc2_netid_wivi_command, 221);
	ASSERT_EQ(icsneoc2_netid_script_status, 224);
	ASSERT_EQ(icsneoc2_netid_eth_phy_control, 239);
	ASSERT_EQ(icsneoc2_netid_extended_command, 240);
	ASSERT_EQ(icsneoc2_netid_extended_data, 242);
	ASSERT_EQ(icsneoc2_netid_flexray_control, 243);
	ASSERT_EQ(icsneoc2_netid_coremini_preload, 244);
	ASSERT_EQ(icsneoc2_netid_hw_com_latency_test, 512);
	ASSERT_EQ(icsneoc2_netid_device_status, 513);
	ASSERT_EQ(icsneoc2_netid_udp, 514);
	ASSERT_EQ(icsneoc2_netid_forwarded_message, 516);
	ASSERT_EQ(icsneoc2_netid_i2c_02, 517);
	ASSERT_EQ(icsneoc2_netid_i2c_03, 518);
	ASSERT_EQ(icsneoc2_netid_i2c_04, 519);
	ASSERT_EQ(icsneoc2_netid_ethernet_02, 520);
	ASSERT_EQ(icsneoc2_netid_ethernet_tx_wrap, 521);
	ASSERT_EQ(icsneoc2_netid_a2b_01, 522);
	ASSERT_EQ(icsneoc2_netid_a2b_02, 523);
	ASSERT_EQ(icsneoc2_netid_ethernet_03, 524);
	ASSERT_EQ(icsneoc2_netid_wbms_01, 532);
	ASSERT_EQ(icsneoc2_netid_dwcan_09, 534);
	ASSERT_EQ(icsneoc2_netid_dwcan_10, 535);
	ASSERT_EQ(icsneoc2_netid_dwcan_11, 536);
	ASSERT_EQ(icsneoc2_netid_dwcan_12, 537);
	ASSERT_EQ(icsneoc2_netid_dwcan_13, 538);
	ASSERT_EQ(icsneoc2_netid_dwcan_14, 539);
	ASSERT_EQ(icsneoc2_netid_dwcan_15, 540);
	ASSERT_EQ(icsneoc2_netid_dwcan_16, 541);
	ASSERT_EQ(icsneoc2_netid_lin_07, 542);
	ASSERT_EQ(icsneoc2_netid_lin_08, 543);
	ASSERT_EQ(icsneoc2_netid_spi_02, 544);
	ASSERT_EQ(icsneoc2_netid_mdio_01, 545);
	ASSERT_EQ(icsneoc2_netid_mdio_02, 546);
	ASSERT_EQ(icsneoc2_netid_mdio_03, 547);
	ASSERT_EQ(icsneoc2_netid_mdio_04, 548);
	ASSERT_EQ(icsneoc2_netid_mdio_05, 549);
	ASSERT_EQ(icsneoc2_netid_mdio_06, 550);
	ASSERT_EQ(icsneoc2_netid_mdio_07, 551);
	ASSERT_EQ(icsneoc2_netid_mdio_08, 552);
	ASSERT_EQ(icsneoc2_netid_ae_13, 553);
	ASSERT_EQ(icsneoc2_netid_ae_14, 554);
	ASSERT_EQ(icsneoc2_netid_ae_15, 555);
	ASSERT_EQ(icsneoc2_netid_ae_16, 556);
	ASSERT_EQ(icsneoc2_netid_spi_03, 557);
	ASSERT_EQ(icsneoc2_netid_spi_04, 558);
	ASSERT_EQ(icsneoc2_netid_spi_05, 559);
	ASSERT_EQ(icsneoc2_netid_spi_06, 560);
	ASSERT_EQ(icsneoc2_netid_spi_07, 561);
	ASSERT_EQ(icsneoc2_netid_spi_08, 562);
	ASSERT_EQ(icsneoc2_netid_lin_09, 563);
	ASSERT_EQ(icsneoc2_netid_lin_10, 564);
	ASSERT_EQ(icsneoc2_netid_lin_11, 565);
	ASSERT_EQ(icsneoc2_netid_lin_12, 566);
	ASSERT_EQ(icsneoc2_netid_lin_13, 567);
	ASSERT_EQ(icsneoc2_netid_lin_14, 568);
	ASSERT_EQ(icsneoc2_netid_lin_15, 569);
	ASSERT_EQ(icsneoc2_netid_lin_16, 570);
	ASSERT_EQ(icsneoc2_netid_maxsize, 571);
}

TEST(icsneoc2, test_icsneoc2_open_options_default)
{
	icsneoc2_open_options_t expected = ICSNEOC2_OPEN_OPTIONS_GO_ONLINE | ICSNEOC2_OPEN_OPTIONS_SYNC_RTC | ICSNEOC2_OPEN_OPTIONS_ENABLE_AUTO_UPDATE;
	ASSERT_EQ(icsneoc2_open_options_default, expected);
}

TEST(icsneoc2, test_icsneoc2_disk_format_enums)
{
	// Disk layout enum values
	ASSERT_EQ(icsneoc2_disk_layout_spanned, 0);
	ASSERT_EQ(icsneoc2_disk_layout_raid0, 1);

	// Disk format flag bitmask values
	ASSERT_EQ(ICSNEOC2_DISK_FORMAT_FLAGS_PRESENT, 0x01);
	ASSERT_EQ(ICSNEOC2_DISK_FORMAT_FLAGS_INITIALIZED, 0x02);
	ASSERT_EQ(ICSNEOC2_DISK_FORMAT_FLAGS_FORMATTED, 0x04);

	// Disk format directive values
	ASSERT_EQ(icsneoc2_disk_format_directive_continue, 0);
	ASSERT_EQ(icsneoc2_disk_format_directive_stop, 1);

	// Type sizes
	ASSERT_EQ(sizeof(icsneoc2_disk_layout_t), sizeof(uint8_t));
	ASSERT_EQ(sizeof(icsneoc2_disk_format_flags_t), sizeof(uint32_t));
	ASSERT_EQ(sizeof(icsneoc2_disk_format_directive_t), sizeof(uint8_t));
}

TEST(icsneoc2, test_icsneoc2_free_null)
{
	// Free functions should be safe to call with NULL
	icsneoc2_disk_details_free(NULL);
	icsneoc2_script_status_free(NULL);
}

TEST(icsneoc2, test_icsneoc2_format_disk_error_code)
{
	// Verify the new error code exists and has a valid string
	char error_str[64] = {0};
	size_t error_str_len = sizeof(error_str);
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_error_code_get(icsneoc2_error_format_disk_failed, error_str, &error_str_len));
	ASSERT_GT(error_str_len, 0u);
	ASSERT_STREQ(error_str, "Disk format failed");
}

TEST(icsneoc2, test_icsneoc2_memory_type_enums)
{
	// Memory type enum values should match Disk::MemoryType
	ASSERT_EQ(0, icsneoc2_memory_type_flash);
	ASSERT_EQ(1, icsneoc2_memory_type_sd);
}

TEST(icsneoc2, test_icsneoc2_script_error_codes)
{
	// Verify script error codes exist and have distinct string representations
	char buf[256];
	size_t len;

	len = sizeof(buf);
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_error_code_get(icsneoc2_error_script_start_failed, buf, &len));
	ASSERT_GT(len, 0u);

	len = sizeof(buf);
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_error_code_get(icsneoc2_error_script_stop_failed, buf, &len));
	ASSERT_GT(len, 0u);

	len = sizeof(buf);
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_error_code_get(icsneoc2_error_script_clear_failed, buf, &len));
	ASSERT_GT(len, 0u);

	len = sizeof(buf);
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_error_code_get(icsneoc2_error_script_upload_failed, buf, &len));
	ASSERT_GT(len, 0u);

	len = sizeof(buf);
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_error_code_get(icsneoc2_error_script_load_prepare_failed, buf, &len));
	ASSERT_GT(len, 0u);
}

TEST(icsneoc2, test_lin_message_create_and_props)
{
	// Create a LIN message with ID 0x15
	icsneoc2_message_t* msg = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_create(&msg, 0x15));
	ASSERT_NE(msg, nullptr);

	// Verify it reports as LIN
	bool is_lin = false;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_lin(msg, &is_lin));
	ASSERT_TRUE(is_lin);

	// Verify it does NOT report as CAN
	bool is_can = true;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_can(msg, &is_can));
	ASSERT_FALSE(is_can);

	// CAN props should fail on a LIN message
	uint64_t arb_id = 0;
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_can_props_get(msg, &arb_id, NULL));

	// Read back default props
	uint8_t id = 0xFF, protected_id = 0, checksum = 0xFF;
	icsneoc2_lin_msg_type_t msg_type = 0xFF;
	bool enhanced = true;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_get(msg, &id, &protected_id, &checksum, &msg_type, &enhanced));
	ASSERT_EQ(id, 0x15);
	ASSERT_NE(protected_id, 0); // Should have parity bits
	ASSERT_EQ(checksum, 0);
	ASSERT_EQ(msg_type, icsneoc2_lin_msg_type_not_set);
	ASSERT_FALSE(enhanced);

	// ID should be masked to 6 bits
	icsneoc2_message_t* msg_masked = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_create(&msg_masked, 0xFF));
	uint8_t masked_id = 0xFF;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_get(msg_masked, &masked_id, NULL, NULL, NULL, NULL));
	ASSERT_EQ(masked_id, 0x3F); // 0xFF & 0x3F
	icsneoc2_message_free(msg_masked);

	icsneoc2_message_free(msg);
}

TEST(icsneoc2, test_lin_message_props_set)
{
	icsneoc2_message_t* msg = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_create(&msg, 0x00));

	// Set individual properties using NULL to skip others
	uint8_t new_id = 0x2A;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_set(msg, &new_id, NULL, NULL, NULL));

	uint8_t read_id = 0, read_pid = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_get(msg, &read_id, &read_pid, NULL, NULL, NULL));
	ASSERT_EQ(read_id, 0x2A);
	ASSERT_NE(read_pid, 0x2A); // Protected ID should differ (has parity bits)

	// Set checksum
	uint8_t new_checksum = 0xAB;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_set(msg, NULL, &new_checksum, NULL, NULL));
	uint8_t read_checksum = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_get(msg, NULL, NULL, &read_checksum, NULL, NULL));
	ASSERT_EQ(read_checksum, 0xAB);

	// Set msg type
	icsneoc2_lin_msg_type_t new_type = icsneoc2_lin_msg_type_commander_msg;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_set(msg, NULL, NULL, &new_type, NULL));
	icsneoc2_lin_msg_type_t read_type = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_get(msg, NULL, NULL, NULL, &read_type, NULL));
	ASSERT_EQ(read_type, icsneoc2_lin_msg_type_commander_msg);

	// Set enhanced checksum
	bool new_enhanced = true;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_set(msg, NULL, NULL, NULL, &new_enhanced));
	bool read_enhanced = false;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_get(msg, NULL, NULL, NULL, NULL, &read_enhanced));
	ASSERT_TRUE(read_enhanced);

	// LIN props set on a CAN message should fail
	icsneoc2_message_t* can_msg = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_can_create(&can_msg));
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_lin_props_set(can_msg, &new_id, NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_lin_props_get(can_msg, &read_id, NULL, NULL, NULL, NULL));
	icsneoc2_message_free(can_msg);

	icsneoc2_message_free(msg);
}

TEST(icsneoc2, test_lin_message_data_and_netid)
{
	icsneoc2_message_t* msg = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_create(&msg, 0x10));

	// Set data
	uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_data_set(msg, data, sizeof(data)));

	// Read data back
	uint8_t read_data[8] = {0};
	size_t read_len = sizeof(read_data);
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_data_get(msg, read_data, &read_len));
	ASSERT_EQ(read_len, sizeof(data));
	ASSERT_EQ(memcmp(data, read_data, sizeof(data)), 0);

	// Set and verify netid
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_netid_set(msg, icsneoc2_netid_lin_01));
	icsneoc2_netid_t netid = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_netid_get(msg, &netid));
	ASSERT_EQ(netid, icsneoc2_netid_lin_01);

	// Verify is_frame and is_raw
	bool is_frame = false, is_raw = false;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_frame(msg, &is_frame));
	ASSERT_TRUE(is_frame);
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_raw(msg, &is_raw));
	ASSERT_TRUE(is_raw);

	icsneoc2_message_free(msg);
}

TEST(icsneoc2, test_lin_message_flags)
{
	// Create a LIN message and verify default flags are clear
	icsneoc2_message_t* msg = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_create(&msg, 0x01));

	icsneoc2_lin_err_flags_t err_flags = 0xFFFFFFFF;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_err_flags_get(msg, &err_flags));
	ASSERT_EQ(err_flags, 0u);

	icsneoc2_lin_status_flags_t status_flags = 0xFFFFFFFF;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_status_flags_get(msg, &status_flags));
	ASSERT_EQ(status_flags, 0u);

	// Error/status flags on CAN message should fail
	icsneoc2_message_t* can_msg = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_can_create(&can_msg));
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_lin_err_flags_get(can_msg, &err_flags));
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_lin_status_flags_get(can_msg, &status_flags));
	icsneoc2_message_free(can_msg);

	icsneoc2_message_free(msg);
}

TEST(icsneoc2, test_lin_message_calc_checksum)
{
	icsneoc2_message_t* msg = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_create(&msg, 0x01));

	// Set some data and calculate checksum (classic)
	uint8_t data[] = {0x01, 0x02, 0x03};
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_data_set(msg, data, sizeof(data)));
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_calc_checksum(msg));

	uint8_t checksum = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_get(msg, NULL, NULL, &checksum, NULL, NULL));
	ASSERT_NE(checksum, 0); // Checksum should be non-zero for this data

	// Now set enhanced checksum and recalculate — should give a different value
	uint8_t classic_checksum = checksum;
	bool enhanced = true;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_set(msg, NULL, NULL, NULL, &enhanced));
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_calc_checksum(msg));
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_lin_props_get(msg, NULL, NULL, &checksum, NULL, NULL));
	ASSERT_NE(checksum, classic_checksum); // Enhanced and classic should differ

	// calc_checksum on a CAN message should fail
	icsneoc2_message_t* can_msg = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_can_create(&can_msg));
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_lin_calc_checksum(can_msg));
	icsneoc2_message_free(can_msg);

	icsneoc2_message_free(msg);
}

TEST(icsneoc2, test_lin_msg_type_enum_values)
{
	ASSERT_EQ(icsneoc2_lin_msg_type_not_set, 0);
	ASSERT_EQ(icsneoc2_lin_msg_type_commander_msg, 1);
	ASSERT_EQ(icsneoc2_lin_msg_type_header_only, 2);
	ASSERT_EQ(icsneoc2_lin_msg_type_break_only, 3);
	ASSERT_EQ(icsneoc2_lin_msg_type_sync_only, 4);
	ASSERT_EQ(icsneoc2_lin_msg_type_update_responder, 5);
	ASSERT_EQ(icsneoc2_lin_msg_type_error, 6);
	ASSERT_EQ(sizeof(icsneoc2_lin_msg_type_t), sizeof(uint8_t));
}

TEST(icsneoc2, test_lin_msg_type_cpp_enum_sync)
{
	using T = icsneo::LINMessage::Type;
	ASSERT_EQ(static_cast<uint8_t>(T::NOT_SET), icsneoc2_lin_msg_type_not_set);
	ASSERT_EQ(static_cast<uint8_t>(T::LIN_COMMANDER_MSG), icsneoc2_lin_msg_type_commander_msg);
	ASSERT_EQ(static_cast<uint8_t>(T::LIN_HEADER_ONLY), icsneoc2_lin_msg_type_header_only);
	ASSERT_EQ(static_cast<uint8_t>(T::LIN_BREAK_ONLY), icsneoc2_lin_msg_type_break_only);
	ASSERT_EQ(static_cast<uint8_t>(T::LIN_SYNC_ONLY), icsneoc2_lin_msg_type_sync_only);
	ASSERT_EQ(static_cast<uint8_t>(T::LIN_UPDATE_RESPONDER), icsneoc2_lin_msg_type_update_responder);
	ASSERT_EQ(static_cast<uint8_t>(T::LIN_ERROR), icsneoc2_lin_msg_type_error);
}

TEST(icsneoc2, test_lin_flag_bitmask_values)
{
	// Error flags should be distinct bits
	ASSERT_EQ(ICSNEOC2_LIN_ERR_RX_BREAK_ONLY, 0x0001);
	ASSERT_EQ(ICSNEOC2_LIN_ERR_RX_BREAK_SYNC_ONLY, 0x0002);
	ASSERT_EQ(ICSNEOC2_LIN_ERR_TX_RX_MISMATCH, 0x0004);
	ASSERT_EQ(ICSNEOC2_LIN_ERR_RX_BREAK_NOT_ZERO, 0x0008);
	ASSERT_EQ(ICSNEOC2_LIN_ERR_RX_BREAK_TOO_SHORT, 0x0010);
	ASSERT_EQ(ICSNEOC2_LIN_ERR_RX_SYNC_NOT_55, 0x0020);
	ASSERT_EQ(ICSNEOC2_LIN_ERR_RX_DATA_LEN_OVER_8, 0x0040);
	ASSERT_EQ(ICSNEOC2_LIN_ERR_FRAME_SYNC, 0x0080);
	ASSERT_EQ(ICSNEOC2_LIN_ERR_FRAME_MESSAGE_ID, 0x0100);
	ASSERT_EQ(ICSNEOC2_LIN_ERR_FRAME_RESPONDER_DATA, 0x0200);
	ASSERT_EQ(ICSNEOC2_LIN_ERR_CHECKSUM_MATCH, 0x0400);

	// Status flags should be distinct bits
	ASSERT_EQ(ICSNEOC2_LIN_STATUS_TX_CHECKSUM_ENHANCED, 0x01);
	ASSERT_EQ(ICSNEOC2_LIN_STATUS_TX_COMMANDER, 0x02);
	ASSERT_EQ(ICSNEOC2_LIN_STATUS_TX_RESPONDER, 0x04);
	ASSERT_EQ(ICSNEOC2_LIN_STATUS_TX_ABORTED, 0x08);
	ASSERT_EQ(ICSNEOC2_LIN_STATUS_UPDATE_RESPONDER_ONCE, 0x10);
	ASSERT_EQ(ICSNEOC2_LIN_STATUS_HAS_UPDATED_RESPONDER_ONCE, 0x20);
	ASSERT_EQ(ICSNEOC2_LIN_STATUS_BUS_RECOVERED, 0x40);
	ASSERT_EQ(ICSNEOC2_LIN_STATUS_BREAK_ONLY, 0x80);
}

