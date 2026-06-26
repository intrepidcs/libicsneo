#include <gtest/gtest.h>
#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2settings.h>
#include <icsneo/icsneoc2messages.h>
#include "../../api/icsneoc2/icsneoc2_internal.h"
#include <icsneo/device/devicefinder.h>
#include <icsneo/device/devicetype.h>
#include <icsneo/device/chipid.h>
#include <icsneo/communication/message/linmessage.h>
#include <icsneo/communication/message/canmessage.h>
#include <icsneo/communication/message/canerrormessage.h>
#include <icsneo/communication/message/apperrormessage.h>


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

TEST(icsneoc2, test_icsneoc2_supported_devices_enumerate)
{
	icsneoc2_supported_device_t* supported_devices = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_supported_devices_enumerate(&supported_devices));
	ASSERT_NE(nullptr, supported_devices);
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_supported_device_get(supported_devices, nullptr));

	const auto& expected_devices = icsneo::DeviceFinder::GetSupportedDevices();
	size_t count = 0;
	for(icsneoc2_supported_device_t* cur = supported_devices; cur != nullptr; cur = icsneoc2_supported_devices_next(cur)) {
		ASSERT_LT(count, expected_devices.size());

		icsneoc2_devicetype_t device_type = 0;
		ASSERT_EQ(icsneoc2_error_success, icsneoc2_supported_device_get(cur, &device_type));
		ASSERT_EQ(static_cast<icsneoc2_devicetype_t>(expected_devices[count].getDeviceType()), device_type);

		char name[ICSNEO_DEVICETYPE_LONGEST_NAME] = {0};
		size_t name_len = sizeof(name);
		ASSERT_EQ(icsneoc2_error_success, icsneoc2_device_type_name_get(device_type, name, &name_len));
		ASSERT_STREQ(icsneo::DeviceType::GetGenericProductName(expected_devices[count].getDeviceType()), name);

		count++;
	}

	ASSERT_EQ(expected_devices.size(), count);
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_supported_devices_free(supported_devices));
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
	icsneoc2_linux_configuration_port_t placeholderLinuxConfigPort = 0;

	// All of these don't have a device parameter
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_enumerate(0, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_supported_devices_enumerate(NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_supported_devices_free(NULL));
	ASSERT_EQ(nullptr, icsneoc2_supported_devices_next(NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_supported_device_get(NULL, NULL));
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
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_timestamp_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_timestamp_set(NULL, 0));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_netid_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_netid_set(NULL, 0));

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_network_type_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_is_transmit(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_is_error(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_is_valid(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_is_can_error(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_can_error_props_get(NULL, NULL, NULL, NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_is_app_error(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_app_error_props_get(NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_app_error_string_get(NULL, NULL, NULL));

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
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_product_name_get(NULL, placeholderStr, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_pcb_serial_get(NULL, &placeholderInteger8, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_mac_addresses_enumerate(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_mac_network_id_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_mac_address_get(NULL, &placeholderInteger8, &placeholderSizeT));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_mac_addresses_free(NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_supports_tc10(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_tc10_wake_request(NULL, icsneoc2_netid_dwcan_01));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_tc10_sleep_request(NULL, icsneoc2_netid_dwcan_01));
	{
		icsneoc2_tc10_sleep_status_t sleep_s = icsneoc2_tc10_sleep_status_no_sleep_received;
		icsneoc2_tc10_wake_status_t wake_s = icsneoc2_tc10_wake_status_no_wake_received;
		ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_tc10_status_get(NULL, icsneoc2_netid_dwcan_01, &sleep_s, &wake_s));
		ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_tc10_status_get(NULL, icsneoc2_netid_dwcan_01, NULL, NULL));
	}
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_supports_reboot(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_reboot(NULL, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_netid_network_type_get(icsneoc2_netid_dwcan_01, NULL));
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
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_burst_timer_get(NULL, 0, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_burst_timer_set(NULL, 0, placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_local_id_alternate_get(NULL, 0, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_local_id_alternate_set(NULL, 0, placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_is_termination_enabled_for(NULL, 0, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_termination_for_set(NULL, 0, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_is_bus_decoding_beacons_enabled_for(NULL, 0, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_bus_decoding_beacons_for_set(NULL, 0, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_is_bus_decoding_all_enabled_for(NULL, 0, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_bus_decoding_all_for_set(NULL, 0, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_multi_id_enable_mask_get(NULL, 0, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_multi_id_enable_mask_set(NULL, 0, placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_multi_id_get(NULL, 0, 0, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_t1s_multi_id_set(NULL, 0, 0, placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_misc_io_analog_output_enabled_set(NULL, 0, placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_misc_io_analog_output_set(NULL, 0, placeholderMiscIoAnalogVoltage));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_linux_boot_enabled_get(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_linux_boot_enabled_set(NULL, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_external_wifi_antenna_enabled_get(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_external_wifi_antenna_enabled_set(NULL, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_perf_test_enabled_get(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_perf_test_enabled_set(NULL, false));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_linux_configuration_port_get(NULL, &placeholderLinuxConfigPort));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_linux_configuration_port_set(NULL, placeholderLinuxConfigPort));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_disabled_get(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_readonly_get(NULL, &placeholderBool));

	// gPTP settings NULL parameter checks
	icsneoc2_gptp_profile_t placeholderGptpProfile = 0;
	icsneoc2_gptp_role_t placeholderGptpRole = 0;
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_profile_get(NULL, &placeholderGptpProfile));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_profile_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_profile_set(NULL, placeholderGptpProfile));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_role_get(NULL, &placeholderGptpRole));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_role_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_role_set(NULL, placeholderGptpRole));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_enabled_port_get(NULL, &placeholderInteger8));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_enabled_port_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_enabled_port_set(NULL, 0));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_clock_syntonization_enabled_get(NULL, &placeholderBool));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_clock_syntonization_enabled_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_settings_gptp_clock_syntonization_enabled_set(NULL, false));

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

	// Ethernet message functions
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_eth_create(NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_eth_props_set(NULL, NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_eth_props_get(NULL, NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_eth_mac_get(NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_eth_ether_type_get(NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_eth_t1s_props_set(NULL, NULL, NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_eth_t1s_props_get(NULL, NULL, NULL, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_is_ethernet(NULL, NULL));
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
	ASSERT_EQ(icsneoc2_devicetype_rad_comet2, 0x00000024);
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
	ASSERT_EQ(icsneo::DeviceType::RADComet2, icsneoc2_devicetype_rad_comet2);
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

TEST(icsneoc2, test_icsneoc2_netid_network_type_get)
{
	icsneoc2_network_type_t type = icsneoc2_network_type_invalid;

	// CAN
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_netid_network_type_get(icsneoc2_netid_dwcan_01, &type));
	ASSERT_EQ(icsneoc2_network_type_can, type);

	// LIN
	type = icsneoc2_network_type_invalid;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_netid_network_type_get(icsneoc2_netid_lin_01, &type));
	ASSERT_EQ(icsneoc2_network_type_lin, type);

	// Ethernet
	type = icsneoc2_network_type_invalid;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_netid_network_type_get(icsneoc2_netid_ethernet_01, &type));
	ASSERT_EQ(icsneoc2_network_type_ethernet, type);

	// Automotive Ethernet
	type = icsneoc2_network_type_invalid;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_netid_network_type_get(icsneoc2_netid_ae_01, &type));
	ASSERT_EQ(icsneoc2_network_type_automotive_ethernet, type);

	// SWCAN
	type = icsneoc2_network_type_invalid;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_netid_network_type_get(icsneoc2_netid_swcan_01, &type));
	ASSERT_EQ(icsneoc2_network_type_swcan, type);

	// LSFTCAN
	type = icsneoc2_network_type_invalid;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_netid_network_type_get(icsneoc2_netid_lsftcan_01, &type));
	ASSERT_EQ(icsneoc2_network_type_lsftcan, type);
}

TEST(icsneoc2, test_icsneoc2_tc10_wake_status_t)
{
	ASSERT_EQ(icsneoc2_tc10_wake_status_no_wake_received, 0);
	ASSERT_EQ(icsneoc2_tc10_wake_status_wake_received, 1);
	ASSERT_EQ(icsneoc2_tc10_wake_status_maxsize, 2);

	ASSERT_EQ(sizeof(icsneoc2_tc10_wake_status_t), sizeof(uint8_t));
}

TEST(icsneoc2, test_icsneoc2_tc10_sleep_status_t)
{
	ASSERT_EQ(icsneoc2_tc10_sleep_status_no_sleep_received, 0);
	ASSERT_EQ(icsneoc2_tc10_sleep_status_sleep_received, 1);
	ASSERT_EQ(icsneoc2_tc10_sleep_status_sleep_failed, 2);
	ASSERT_EQ(icsneoc2_tc10_sleep_status_sleep_aborted, 3);
	ASSERT_EQ(icsneoc2_tc10_sleep_status_maxsize, 4);

	ASSERT_EQ(sizeof(icsneoc2_tc10_sleep_status_t), sizeof(uint8_t));
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

TEST(icsneoc2, test_icsneoc2_chip_id_t)
{
	ASSERT_EQ(icsneoc2_chip_id_neovifire_mchip, 0);
	ASSERT_EQ(icsneoc2_chip_id_neovifire_lchip, 1);
	ASSERT_EQ(icsneoc2_chip_id_neovifire_uchip, 2);
	ASSERT_EQ(icsneoc2_chip_id_neovifire_jchip, 3);
	ASSERT_EQ(icsneoc2_chip_id_valuecan3_mchip, 4);
	ASSERT_EQ(icsneoc2_chip_id_neoviecu_mpic, 6);
	ASSERT_EQ(icsneoc2_chip_id_neoviievb_mpic, 7);
	ASSERT_EQ(icsneoc2_chip_id_neovipendant_mpic, 8);
	ASSERT_EQ(icsneoc2_chip_id_neovifire_vnet_mchip, 9);
	ASSERT_EQ(icsneoc2_chip_id_neovifire_vnet_lchip, 10);
	ASSERT_EQ(icsneoc2_chip_id_neoviplasma_core, 11);
	ASSERT_EQ(icsneoc2_chip_id_neoviplasma_hid, 12);
	ASSERT_EQ(icsneoc2_chip_id_neovianalog_mpic, 13);
	ASSERT_EQ(icsneoc2_chip_id_neoviplasma_analog_core, 14);
	ASSERT_EQ(icsneoc2_chip_id_neoviplasma_flexray_core, 15);
	ASSERT_EQ(icsneoc2_chip_id_neoviplasma_core_1_12, 16);
	ASSERT_EQ(icsneoc2_chip_id_neovifire_slave_vnet_mchip, 17);
	ASSERT_EQ(icsneoc2_chip_id_neovifire_slave_vnet_lchip, 18);
	ASSERT_EQ(icsneoc2_chip_id_neoviion_core, 19);
	ASSERT_EQ(icsneoc2_chip_id_neoviion_hid, 20);
	ASSERT_EQ(icsneoc2_chip_id_neoviion_core_loader, 21);
	ASSERT_EQ(icsneoc2_chip_id_neoviion_hid_loader, 22);
	ASSERT_EQ(icsneoc2_chip_id_neoviion_fpga_bit, 23);
	ASSERT_EQ(icsneoc2_chip_id_neovifire_vnet_ep_mchip, 24);
	ASSERT_EQ(icsneoc2_chip_id_neovifire_vnet_ep_lchip, 25);
	ASSERT_EQ(icsneoc2_chip_id_neovianalogout_mchip, 26);
	ASSERT_EQ(icsneoc2_chip_id_neovimost25_mchip, 27);
	ASSERT_EQ(icsneoc2_chip_id_neovimost50_mchip, 28);
	ASSERT_EQ(icsneoc2_chip_id_neovimost150_mchip, 29);
	ASSERT_EQ(icsneoc2_chip_id_valuecan4_4_mchip, 30);
	ASSERT_EQ(icsneoc2_chip_id_valuecan4_4_schip, 31);
	ASSERT_EQ(icsneoc2_chip_id_cmprobe_zynq, 33);
	ASSERT_EQ(icsneoc2_chip_id_eevb_stm32, 34);
	ASSERT_EQ(icsneoc2_chip_id_neovifire_slave_vnet_ep_mchip, 35);
	ASSERT_EQ(icsneoc2_chip_id_neovifire_slave_vnet_ep_lchip, 36);
	ASSERT_EQ(icsneoc2_chip_id_radstar_mchip, 37);
	ASSERT_EQ(icsneoc2_chip_id_valuecanrf_mchip, 38);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_mchip, 39);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_cchip, 40);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_core, 41);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_blechip, 42);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_zynq, 43);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_securitychip, 44);
	ASSERT_EQ(icsneoc2_chip_id_radgalaxy_zynq, 45);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_vnet_mchip, 46);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_slave_vnet_a_mchip, 47);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_slave_vnet_a_cchip, 48);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_vnet_cchip, 49);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_vnet_core, 50);
	ASSERT_EQ(icsneoc2_chip_id_radstar2_zynq, 51);
	ASSERT_EQ(icsneoc2_chip_id_vividcan_mchip, 52);
	ASSERT_EQ(icsneoc2_chip_id_neoobd2sim_mchip, 53);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_vnetz_mchip, 54);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_vnetz_zynq, 55);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_slave_vnetz_a_mchip, 56);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_slave_vnetz_a_zynq, 57);
	ASSERT_EQ(icsneoc2_chip_id_vividcan_ext_flash, 58);
	ASSERT_EQ(icsneoc2_chip_id_vividcan_nrf52, 59);
	ASSERT_EQ(icsneoc2_chip_id_cmprobe_zynq_unused, 60);
	ASSERT_EQ(icsneoc2_chip_id_neoobd2pro_mchip, 61);
	ASSERT_EQ(icsneoc2_chip_id_valuecan4_1_mchip, 62);
	ASSERT_EQ(icsneoc2_chip_id_valuecan4_2_mchip, 63);
	ASSERT_EQ(icsneoc2_chip_id_valuecan4_4_2el_core, 64);
	ASSERT_EQ(icsneoc2_chip_id_neoobd2pro_schip, 65);
	ASSERT_EQ(icsneoc2_chip_id_valuecan4_2el_mchip, 67);
	ASSERT_EQ(icsneoc2_chip_id_neoecuavbtsn_mchip, 68);
	ASSERT_EQ(icsneoc2_chip_id_neoobd2pro_core, 69);
	ASSERT_EQ(icsneoc2_chip_id_radsupermoon_zynq, 70);
	ASSERT_EQ(icsneoc2_chip_id_radmoon2_zynq, 71);
	ASSERT_EQ(icsneoc2_chip_id_vividcanpro_mchip, 72);
	ASSERT_EQ(icsneoc2_chip_id_vividcanpro_ext_flash, 73);
	ASSERT_EQ(icsneoc2_chip_id_radpluto_mchip, 74);
	ASSERT_EQ(icsneoc2_chip_id_radmars_zynq, 75);
	ASSERT_EQ(icsneoc2_chip_id_neoecu12_mchip, 76);
	ASSERT_EQ(icsneoc2_chip_id_radiocanhub_mchip, 77);
	ASSERT_EQ(icsneoc2_chip_id_flexray_vnetz_zchip, 78);
	ASSERT_EQ(icsneoc2_chip_id_neoobd2_lcbadge_mchip, 79);
	ASSERT_EQ(icsneoc2_chip_id_neoobd2_lcbadge_schip, 80);
	ASSERT_EQ(icsneoc2_chip_id_radmoonduo_mchip, 81);
	ASSERT_EQ(icsneoc2_chip_id_neovifire3_zchip, 82);
	ASSERT_EQ(icsneoc2_chip_id_flexray_vnetz_fchip, 83);
	ASSERT_EQ(icsneoc2_chip_id_radjupiter_mchip, 84);
	ASSERT_EQ(icsneoc2_chip_id_valuecan4industrial_mchip, 85);
	ASSERT_EQ(icsneoc2_chip_id_etherbadge_mchip, 86);
	ASSERT_EQ(icsneoc2_chip_id_radmars_3_zynq, 87);
	ASSERT_EQ(icsneoc2_chip_id_radgigastar_usbz_zynq, 88);
	ASSERT_EQ(icsneoc2_chip_id_radgigastar_zynq, 89);
	ASSERT_EQ(icsneoc2_chip_id_rad4g_mchip, 90);
	ASSERT_EQ(icsneoc2_chip_id_neovifire3_schip, 91);
	ASSERT_EQ(icsneoc2_chip_id_radepsilon_mchip, 92);
	ASSERT_EQ(icsneoc2_chip_id_rada2b_zchip, 93);
	ASSERT_EQ(icsneoc2_chip_id_neoobd2dev_mchip, 94);
	ASSERT_EQ(icsneoc2_chip_id_neoobd2dev_schip, 95);
	ASSERT_EQ(icsneoc2_chip_id_neoobd2simdoip_mchip, 96);
	ASSERT_EQ(icsneoc2_chip_id_sfpmodule_88q2112_mchip, 97);
	ASSERT_EQ(icsneoc2_chip_id_radepsilont_mchip, 98);
	ASSERT_EQ(icsneoc2_chip_id_radepsilonexpress_mchip, 99);
	ASSERT_EQ(icsneoc2_chip_id_radproxima_mchip, 100);
	ASSERT_EQ(icsneoc2_chip_id_newdevice57_zchip, 101);
	ASSERT_EQ(icsneoc2_chip_id_rad_galaxy_2_zmpchip_id, 102);
	ASSERT_EQ(icsneoc2_chip_id_newdevice59_mchip, 103);
	ASSERT_EQ(icsneoc2_chip_id_radmoon2_z7010_zynq, 104);
	ASSERT_EQ(icsneoc2_chip_id_neovifire2_core_sg4, 105);
	ASSERT_EQ(icsneoc2_chip_id_radbms_mchip, 106);
	ASSERT_EQ(icsneoc2_chip_id_radmoon2_zl_mchip, 107);
	ASSERT_EQ(icsneoc2_chip_id_radgigastar_usbz_z7010_zynq, 108);
	ASSERT_EQ(icsneoc2_chip_id_neovifire3_linux, 109);
	ASSERT_EQ(icsneoc2_chip_id_radgigastar_usbz_z7007s_zynq, 110);
	ASSERT_EQ(icsneoc2_chip_id_vem_01_8dw_zchip, 111);
	ASSERT_EQ(icsneoc2_chip_id_radgalaxy_ffg_zynq, 112);
	ASSERT_EQ(icsneoc2_chip_id_radmoon3_mchip, 113);
	ASSERT_EQ(icsneoc2_chip_id_radcomet2_zynq, 114);
	ASSERT_EQ(icsneoc2_chip_id_vem_02_fr_zchip, 115);
	ASSERT_EQ(icsneoc2_chip_id_rada2b_revb_zchip, 116);
	ASSERT_EQ(icsneoc2_chip_id_radgigastar_ffg_zynq, 117);
	ASSERT_EQ(icsneoc2_chip_id_vem_02_fr_fchip, 118);
	ASSERT_EQ(icsneoc2_chip_id_connect_zchip, 121);
	ASSERT_EQ(icsneoc2_chip_id_sfpmodule_88q2221_mchip, 122);
	ASSERT_EQ(icsneoc2_chip_id_radgalaxy2_sysmon_chip, 123);
	ASSERT_EQ(icsneoc2_chip_id_sfpmodule_88q3244_mchip, 124);
	ASSERT_EQ(icsneoc2_chip_id_radcomet3_zchip, 125);
	ASSERT_EQ(icsneoc2_chip_id_connect_linux, 126);
	ASSERT_EQ(icsneoc2_chip_id_sfpmodule_lan8670_mchip, 127);
	ASSERT_EQ(icsneoc2_chip_id_vem_04_t1s_lin_zchip, 129);
	ASSERT_EQ(icsneoc2_chip_id_radmoont1s_zchip, 130);
	ASSERT_EQ(icsneoc2_chip_id_radgigastar2_zynq, 131);
	ASSERT_EQ(icsneoc2_chip_id_sfpmodule_ent11100_mchip, 132);
	ASSERT_EQ(icsneoc2_chip_id_radgemini_mchip, 135);
	ASSERT_EQ(icsneoc2_chip_id_maxsize, 136);
	
	ASSERT_EQ(icsneoc2_chip_id_invalid, 255);

	ASSERT_EQ(sizeof(icsneoc2_chip_id_t), sizeof(uint8_t));
}

TEST(icsneoc2, test_chip_id_enum_alignment)
{
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_MCHIP), icsneoc2_chip_id_neovifire_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_LCHIP), icsneoc2_chip_id_neovifire_lchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_UCHIP), icsneoc2_chip_id_neovifire_uchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_JCHIP), icsneoc2_chip_id_neovifire_jchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::ValueCAN3_MCHIP), icsneoc2_chip_id_valuecan3_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIECU_MPIC), icsneoc2_chip_id_neoviecu_mpic);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIIEVB_MPIC), icsneoc2_chip_id_neoviievb_mpic);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIPENDANT_MPIC), icsneoc2_chip_id_neovipendant_mpic);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_VNET_MCHIP), icsneoc2_chip_id_neovifire_vnet_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_VNET_LCHIP), icsneoc2_chip_id_neovifire_vnet_lchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIPLASMA_Core), icsneoc2_chip_id_neoviplasma_core);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIPLASMA_HID), icsneoc2_chip_id_neoviplasma_hid);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIANALOG_MPIC), icsneoc2_chip_id_neovianalog_mpic);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIPLASMA_ANALOG_Core), icsneoc2_chip_id_neoviplasma_analog_core);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIPLASMA_FlexRay_Core), icsneoc2_chip_id_neoviplasma_flexray_core);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIPLASMA_Core_1_12), icsneoc2_chip_id_neoviplasma_core_1_12);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_Slave_VNET_MCHIP), icsneoc2_chip_id_neovifire_slave_vnet_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_Slave_VNET_LCHIP), icsneoc2_chip_id_neovifire_slave_vnet_lchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIION_Core), icsneoc2_chip_id_neoviion_core);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIION_HID), icsneoc2_chip_id_neoviion_hid);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIION_Core_Loader), icsneoc2_chip_id_neoviion_core_loader);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIION_HID_Loader), icsneoc2_chip_id_neoviion_hid_loader);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIION_FPGA_BIT), icsneoc2_chip_id_neoviion_fpga_bit);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_VNET_EP_MCHIP), icsneoc2_chip_id_neovifire_vnet_ep_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_VNET_EP_LCHIP), icsneoc2_chip_id_neovifire_vnet_ep_lchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIAnalogOut_MCHIP), icsneoc2_chip_id_neovianalogout_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIMOST25_MCHIP), icsneoc2_chip_id_neovimost25_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIMOST50_MCHIP), icsneoc2_chip_id_neovimost50_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIMOST150_MCHIP), icsneoc2_chip_id_neovimost150_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::ValueCAN4_4_MCHIP), icsneoc2_chip_id_valuecan4_4_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::ValueCAN4_4_SCHIP), icsneoc2_chip_id_valuecan4_4_schip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::cmProbe_ZYNQ), icsneoc2_chip_id_cmprobe_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::EEVB_STM32), icsneoc2_chip_id_eevb_stm32);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_Slave_VNET_EP_MCHIP), icsneoc2_chip_id_neovifire_slave_vnet_ep_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE_Slave_VNET_EP_LCHIP), icsneoc2_chip_id_neovifire_slave_vnet_ep_lchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADStar_MCHIP), icsneoc2_chip_id_radstar_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::ValueCANrf_MCHIP), icsneoc2_chip_id_valuecanrf_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_MCHIP), icsneoc2_chip_id_neovifire2_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_CCHIP), icsneoc2_chip_id_neovifire2_cchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_Core), icsneoc2_chip_id_neovifire2_core);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_BLECHIP), icsneoc2_chip_id_neovifire2_blechip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_ZYNQ), icsneoc2_chip_id_neovifire2_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_SECURITYCHIP), icsneoc2_chip_id_neovifire2_securitychip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADGalaxy_ZYNQ), icsneoc2_chip_id_radgalaxy_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_VNET_MCHIP), icsneoc2_chip_id_neovifire2_vnet_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_Slave_VNET_A_MCHIP), icsneoc2_chip_id_neovifire2_slave_vnet_a_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_Slave_VNET_A_CCHIP), icsneoc2_chip_id_neovifire2_slave_vnet_a_cchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_VNET_CCHIP), icsneoc2_chip_id_neovifire2_vnet_cchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_VNET_Core), icsneoc2_chip_id_neovifire2_vnet_core);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADStar2_ZYNQ), icsneoc2_chip_id_radstar2_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::VividCAN_MCHIP), icsneoc2_chip_id_vividcan_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoOBD2SIM_MCHIP), icsneoc2_chip_id_neoobd2sim_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_VNETZ_MCHIP), icsneoc2_chip_id_neovifire2_vnetz_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_VNETZ_ZYNQ), icsneoc2_chip_id_neovifire2_vnetz_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_Slave_VNETZ_A_MCHIP), icsneoc2_chip_id_neovifire2_slave_vnetz_a_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_Slave_VNETZ_A_ZYNQ), icsneoc2_chip_id_neovifire2_slave_vnetz_a_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::VividCAN_EXT_FLASH), icsneoc2_chip_id_vividcan_ext_flash);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::VividCAN_NRF52), icsneoc2_chip_id_vividcan_nrf52);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::cmProbe_ZYNQ_Unused), icsneoc2_chip_id_cmprobe_zynq_unused);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoOBD2PRO_MCHIP), icsneoc2_chip_id_neoobd2pro_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::ValueCAN4_1_MCHIP), icsneoc2_chip_id_valuecan4_1_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::ValueCAN4_2_MCHIP), icsneoc2_chip_id_valuecan4_2_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::ValueCAN4_4_2EL_Core), icsneoc2_chip_id_valuecan4_4_2el_core);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoOBD2PRO_SCHIP), icsneoc2_chip_id_neoobd2pro_schip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::ValueCAN4_2EL_MCHIP), icsneoc2_chip_id_valuecan4_2el_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoECUAVBTSN_MCHIP), icsneoc2_chip_id_neoecuavbtsn_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoOBD2PRO_Core), icsneoc2_chip_id_neoobd2pro_core);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADSupermoon_ZYNQ), icsneoc2_chip_id_radsupermoon_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADMoon2_ZYNQ), icsneoc2_chip_id_radmoon2_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::VividCANPRO_MCHIP), icsneoc2_chip_id_vividcanpro_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::VividCANPRO_EXT_FLASH), icsneoc2_chip_id_vividcanpro_ext_flash);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADPluto_MCHIP), icsneoc2_chip_id_radpluto_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADMars_ZYNQ), icsneoc2_chip_id_radmars_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoECU12_MCHIP), icsneoc2_chip_id_neoecu12_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADIOCANHUB_MCHIP), icsneoc2_chip_id_radiocanhub_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::FlexRay_VNETZ_ZCHIP), icsneoc2_chip_id_flexray_vnetz_zchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoOBD2_LCBADGE_MCHIP), icsneoc2_chip_id_neoobd2_lcbadge_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoOBD2_LCBADGE_SCHIP), icsneoc2_chip_id_neoobd2_lcbadge_schip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADMoonDuo_MCHIP), icsneoc2_chip_id_radmoonduo_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE3_ZCHIP), icsneoc2_chip_id_neovifire3_zchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::FlexRay_VNETZ_FCHIP), icsneoc2_chip_id_flexray_vnetz_fchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADJupiter_MCHIP), icsneoc2_chip_id_radjupiter_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::ValueCAN4Industrial_MCHIP), icsneoc2_chip_id_valuecan4industrial_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::EtherBADGE_MCHIP), icsneoc2_chip_id_etherbadge_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADMars_3_ZYNQ), icsneoc2_chip_id_radmars_3_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADGigastar_USBZ_ZYNQ), icsneoc2_chip_id_radgigastar_usbz_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADGigastar_ZYNQ), icsneoc2_chip_id_radgigastar_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RAD4G_MCHIP), icsneoc2_chip_id_rad4g_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE3_SCHIP), icsneoc2_chip_id_neovifire3_schip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADEpsilon_MCHIP), icsneoc2_chip_id_radepsilon_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADA2B_ZCHIP), icsneoc2_chip_id_rada2b_zchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoOBD2Dev_MCHIP), icsneoc2_chip_id_neoobd2dev_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoOBD2Dev_SCHIP), icsneoc2_chip_id_neoobd2dev_schip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoOBD2SIMDoIP_MCHIP), icsneoc2_chip_id_neoobd2simdoip_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::SFPModule_88q2112_MCHIP), icsneoc2_chip_id_sfpmodule_88q2112_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADEpsilonT_MCHIP), icsneoc2_chip_id_radepsilont_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADEpsilonExpress_MCHIP), icsneoc2_chip_id_radepsilonexpress_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADProxima_MCHIP), icsneoc2_chip_id_radproxima_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::NewDevice57_ZCHIP), icsneoc2_chip_id_newdevice57_zchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RAD_GALAXY_2_ZMPCHIP_ID), icsneoc2_chip_id_rad_galaxy_2_zmpchip_id);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::NewDevice59_MCHIP), icsneoc2_chip_id_newdevice59_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADMoon2_Z7010_ZYNQ), icsneoc2_chip_id_radmoon2_z7010_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE2_Core_SG4), icsneoc2_chip_id_neovifire2_core_sg4);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADBMS_MCHIP), icsneoc2_chip_id_radbms_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADMoon2_ZL_MCHIP), icsneoc2_chip_id_radmoon2_zl_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADGigastar_USBZ_Z7010_ZYNQ), icsneoc2_chip_id_radgigastar_usbz_z7010_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::neoVIFIRE3_LINUX), icsneoc2_chip_id_neovifire3_linux);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADGigastar_USBZ_Z7007S_ZYNQ), icsneoc2_chip_id_radgigastar_usbz_z7007s_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::VEM_01_8DW_ZCHIP), icsneoc2_chip_id_vem_01_8dw_zchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADGalaxy_FFG_Zynq), icsneoc2_chip_id_radgalaxy_ffg_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADMoon3_MCHIP), icsneoc2_chip_id_radmoon3_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADComet2_ZYNQ), icsneoc2_chip_id_radcomet2_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::VEM_02_FR_ZCHIP), icsneoc2_chip_id_vem_02_fr_zchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADA2B_REVB_ZCHIP), icsneoc2_chip_id_rada2b_revb_zchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADGigastar_FFG_ZYNQ), icsneoc2_chip_id_radgigastar_ffg_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::VEM_02_FR_FCHIP), icsneoc2_chip_id_vem_02_fr_fchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::Connect_ZCHIP), icsneoc2_chip_id_connect_zchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::SFPModule_88q2221_MCHIP), icsneoc2_chip_id_sfpmodule_88q2221_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADGALAXY2_SYSMON_CHIP), icsneoc2_chip_id_radgalaxy2_sysmon_chip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::SFPModule_88q3244_MCHIP), icsneoc2_chip_id_sfpmodule_88q3244_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADCOMET3_ZCHIP), icsneoc2_chip_id_radcomet3_zchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::Connect_LINUX), icsneoc2_chip_id_connect_linux);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::SFPModule_lan8670_MCHIP), icsneoc2_chip_id_sfpmodule_lan8670_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::VEM_04_T1S_LIN_ZCHIP), icsneoc2_chip_id_vem_04_t1s_lin_zchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADMOONT1S_ZCHIP), icsneoc2_chip_id_radmoont1s_zchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADGigastar2_ZYNQ), icsneoc2_chip_id_radgigastar2_zynq);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::SFPModule_ent11100_MCHIP), icsneoc2_chip_id_sfpmodule_ent11100_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::RADGemini_MCHIP), icsneoc2_chip_id_radgemini_mchip);
	ASSERT_EQ(static_cast<icsneoc2_chip_id_t>(icsneo::ChipID::Invalid), icsneoc2_chip_id_invalid);
}

TEST(icsneoc2, test_icsneoc2_chip_versions_invalid_parameters)
{
	size_t count = 0;
	icsneoc2_chip_versions_t* chip_versions = nullptr;

	// enumerate: NULL device
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_device_chip_versions_enumerate(NULL, &chip_versions, false, &count));

	// props_get: NULL handle
	uint8_t major = 0, minor = 0, maintenance = 0, build = 0;
	char name[32] = {0};
	size_t name_length = sizeof(name);
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_chip_versions_props_get(NULL, name, &name_length, &major, &minor, &maintenance, &build));

	// free: NULL handle
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_chip_versions_free(NULL));

	// next: NULL handle returns NULL
	ASSERT_EQ(nullptr, icsneoc2_chip_versions_next(NULL));
}

TEST(icsneoc2, test_icsneoc2_chip_versions_props_get)
{
	// Build an in-memory list of two chip_versions nodes
	icsneoc2_chip_versions_t second{};
	second.version_report.id = icsneo::ChipID::RADGemini_MCHIP;
	second.version_report.name = "second";
	second.version_report.major = 9;
	second.version_report.minor = 8;
	second.version_report.maintenance = 7;
	second.version_report.build = 6;
	second.next = nullptr;

	icsneoc2_chip_versions_t first{};
	first.version_report.id = icsneo::ChipID::neoVIFIRE_MCHIP;
	first.version_report.name = "first";
	first.version_report.major = 1;
	first.version_report.minor = 2;
	first.version_report.maintenance = 3;
	first.version_report.build = 4;
	first.next = &second;

	// next() walks the list
	ASSERT_EQ(&second, icsneoc2_chip_versions_next(&first));
	ASSERT_EQ(nullptr, icsneoc2_chip_versions_next(&second));

	// props_get: full extraction
	char name[32] = {0};
	size_t name_length = sizeof(name);
	uint8_t major = 0, minor = 0, maintenance = 0, build = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_chip_versions_props_get(&first, name, &name_length, &major, &minor, &maintenance, &build));
	ASSERT_STREQ(name, "first");
	ASSERT_EQ(name_length, 5u);
	ASSERT_EQ(major, 1);
	ASSERT_EQ(minor, 2);
	ASSERT_EQ(maintenance, 3);
	ASSERT_EQ(build, 4);

	// props_get: all output pointers optional (NULL allowed)
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_chip_versions_props_get(&second, NULL, NULL, NULL, NULL, NULL, NULL));

	// props_get: only some output pointers
	major = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_chip_versions_props_get(&second, NULL, NULL, &major, NULL, NULL, NULL));
	ASSERT_EQ(major, 9);

	// props_get: name buffer too small returns string_copy_failed
	char tiny[2] = {0};
	size_t tiny_length = sizeof(tiny);
	ASSERT_EQ(icsneoc2_error_string_copy_failed, icsneoc2_chip_versions_props_get(&first, tiny, &tiny_length, NULL, NULL, NULL, NULL));
}

TEST(icsneoc2, test_icsneoc2_chip_versions_free_list)
{
	// Allocate a two-node list the same way the API does, so free() walks and deletes it.
	auto* head = new icsneoc2_chip_versions_t{};
	head->version_report.name = "a";
	head->next = new icsneoc2_chip_versions_t{};
	head->next->version_report.name = "b";
	head->next->next = nullptr;

	ASSERT_EQ(icsneoc2_error_success, icsneoc2_chip_versions_free(head));
}

TEST(icsneoc2, test_icsneoc2_open_options_default)
{
	icsneoc2_open_options_t expected = ICSNEOC2_OPEN_OPTIONS_GO_ONLINE | ICSNEOC2_OPEN_OPTIONS_SYNC_RTC | ICSNEOC2_OPEN_OPTIONS_ENABLE_AUTO_UPDATE;
	ASSERT_EQ(icsneoc2_open_options_default, expected);
}

TEST(icsneoc2, test_icsneoc2_message_is_error)
{
	icsneoc2_message_t message;
	message.message = std::make_shared<CANMessage>();

	bool value = false;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_error(&message, &value));
	ASSERT_FALSE(value);

	auto frame = std::dynamic_pointer_cast<Frame>(message.message);
	ASSERT_NE(frame, nullptr);
	frame->error = true;

	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_error(&message, &value));
	ASSERT_TRUE(value);

	icsneoc2_message_t raw_message;
	raw_message.message = std::make_shared<CANErrorMessage>();
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_is_error(&raw_message, &value));
}

TEST(icsneoc2, test_icsneoc2_message_can_props_get_can_tx_flags)
{
	icsneoc2_message_t message;
	auto can_message = std::make_shared<CANMessage>();
	message.message = can_message;

	uint64_t arb_id = 0;
	icsneoc2_message_can_flags_t flags = 0;

	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_can_props_get(&message, &arb_id, &flags));
	ASSERT_EQ(arb_id, 0u);
	ASSERT_EQ(flags, 0u);

	can_message->txAborted = true;
	can_message->txLostArb = true;
	can_message->txError = true;

	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_can_props_get(&message, &arb_id, &flags));
	ASSERT_EQ(flags,
		ICSNEOC2_MESSAGE_CAN_FLAGS_TX_ABORTED |
		ICSNEOC2_MESSAGE_CAN_FLAGS_TX_LOST_ARB |
		ICSNEOC2_MESSAGE_CAN_FLAGS_TX_ERROR);
}

TEST(icsneoc2, test_icsneoc2_message_timestamp_accessors)
{
	icsneoc2_message_t* message = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_can_create(&message));

	uint64_t timestamp = 1;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_timestamp_get(message, &timestamp));
	ASSERT_EQ(0u, timestamp);

	const uint64_t expected_timestamp = 1234567890123ULL;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_timestamp_set(message, expected_timestamp));
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_timestamp_get(message, &timestamp));
	ASSERT_EQ(expected_timestamp, timestamp);

	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_timestamp_get(message, nullptr));

	icsneoc2_message_free(message);

	icsneoc2_message_t invalid_message;
	ASSERT_EQ(icsneoc2_error_invalid_message, icsneoc2_message_timestamp_set(&invalid_message, expected_timestamp));
	ASSERT_EQ(icsneoc2_error_invalid_message, icsneoc2_message_timestamp_get(&invalid_message, &timestamp));
}

TEST(icsneoc2, test_icsneoc2_message_can_error_props_get_invalid_type_for_can_message)
{
	icsneoc2_message_t message;
	message.message = std::make_shared<CANMessage>();

	uint8_t tx_err_count = 0;
	uint8_t rx_err_count = 0;
	icsneoc2_can_error_code_t error_code = icsneoc2_can_error_code_no_error;
	icsneoc2_can_error_code_t data_error_code = icsneoc2_can_error_code_no_error;
	icsneoc2_message_can_error_flags_t flags = 0;

	ASSERT_EQ(icsneoc2_error_invalid_type,
		icsneoc2_message_can_error_props_get(&message, &tx_err_count, &rx_err_count, &error_code, &data_error_code, &flags));
}

TEST(icsneoc2, test_icsneoc2_message_app_error_props_get_invalid_type_for_can_message)
{
	icsneoc2_message_t message;
	message.message = std::make_shared<CANMessage>();

	bool is_app_error = true;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_app_error(&message, &is_app_error));
	ASSERT_FALSE(is_app_error);

	icsneoc2_app_error_type_t error_type = icsneoc2_app_error_type_no_error;
	icsneoc2_netid_t error_netid = 0;
	ASSERT_EQ(icsneoc2_error_invalid_type,
		icsneoc2_message_app_error_props_get(&message, &error_type, &error_netid));
}

TEST(icsneoc2, test_icsneoc2_message_app_error_props_get)
{
	auto app_err = std::make_shared<icsneo::AppErrorMessage>();
	app_err->errorType = icsneoc2_app_error_type_pc_buff_overflow;
	app_err->errorNetID = icsneo::Network::NetID::DWCAN_01;

	icsneoc2_message_t message;
	message.message = app_err;

	bool is_app_error = false;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_app_error(&message, &is_app_error));
	ASSERT_TRUE(is_app_error);

	icsneoc2_app_error_type_t error_type = icsneoc2_app_error_type_no_error;
	icsneoc2_netid_t error_netid = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_app_error_props_get(&message, &error_type, &error_netid));
	ASSERT_EQ(error_type, icsneoc2_app_error_type_pc_buff_overflow);
	ASSERT_EQ(error_netid, static_cast<icsneoc2_netid_t>(icsneo::Network::NetID::DWCAN_01));

	// NULL out-params are ignored
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_app_error_props_get(&message, NULL, NULL));

	// String getter returns the human-readable description
	char buf[128] = {0};
	size_t buf_len = sizeof(buf);
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_app_error_string_get(&message, buf, &buf_len));
	ASSERT_STREQ(buf, "PC buffer overflow");
	ASSERT_EQ(buf_len, sizeof("PC buffer overflow") - 1);
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

TEST(icsneoc2, test_icsneoc2_can_error_code_t)
{
	// CAN error code enum values
	ASSERT_EQ(icsneoc2_can_error_code_no_error, 0);
	ASSERT_EQ(icsneoc2_can_error_code_stuff_error, 1);
	ASSERT_EQ(icsneoc2_can_error_code_form_error, 2);
	ASSERT_EQ(icsneoc2_can_error_code_ack_error, 3);
	ASSERT_EQ(icsneoc2_can_error_code_bit1_error, 4);
	ASSERT_EQ(icsneoc2_can_error_code_bit0_error, 5);
	ASSERT_EQ(icsneoc2_can_error_code_crc_error, 6);
	ASSERT_EQ(icsneoc2_can_error_code_no_change, 7);
	ASSERT_EQ(icsneoc2_can_error_code_maxsize, 8);

	using _T = icsneo::CANErrorCode;
	ASSERT_EQ(static_cast<icsneoc2_can_error_code_t>(_T::NoError), icsneoc2_can_error_code_no_error);
	ASSERT_EQ(static_cast<icsneoc2_can_error_code_t>(_T::StuffError), icsneoc2_can_error_code_stuff_error);
	ASSERT_EQ(static_cast<icsneoc2_can_error_code_t>(_T::FormError), icsneoc2_can_error_code_form_error);
	ASSERT_EQ(static_cast<icsneoc2_can_error_code_t>(_T::AckError), icsneoc2_can_error_code_ack_error);
	ASSERT_EQ(static_cast<icsneoc2_can_error_code_t>(_T::Bit1Error), icsneoc2_can_error_code_bit1_error);
	ASSERT_EQ(static_cast<icsneoc2_can_error_code_t>(_T::Bit0Error), icsneoc2_can_error_code_bit0_error);
	ASSERT_EQ(static_cast<icsneoc2_can_error_code_t>(_T::CRCError), icsneoc2_can_error_code_crc_error);
	ASSERT_EQ(static_cast<icsneoc2_can_error_code_t>(_T::NoChange), icsneoc2_can_error_code_no_change);

	ASSERT_EQ(sizeof(icsneoc2_can_error_code_t), sizeof(uint8_t));

	// CAN error flag bitmask values
	ASSERT_EQ(ICSNEOC2_MESSAGE_CAN_ERROR_FLAGS_BUS_OFF, 0x01);
	ASSERT_EQ(ICSNEOC2_MESSAGE_CAN_ERROR_FLAGS_ERROR_PASSIVE, 0x02);
	ASSERT_EQ(ICSNEOC2_MESSAGE_CAN_ERROR_FLAGS_ERROR_WARN, 0x04);

	ASSERT_EQ(sizeof(icsneoc2_message_can_error_flags_t), sizeof(uint64_t));
}

TEST(icsneoc2, test_icsneoc2_app_error_type_t)
{
	ASSERT_EQ(sizeof(icsneoc2_app_error_type_t), sizeof(uint16_t));

	ASSERT_EQ(icsneoc2_app_error_type_rx_messages_full, 0);
	ASSERT_EQ(icsneoc2_app_error_type_tx_messages_full, 1);
	ASSERT_EQ(icsneoc2_app_error_type_tx_report_messages_full, 2);
	ASSERT_EQ(icsneoc2_app_error_type_bad_comm_with_dsp_ic, 3);
	ASSERT_EQ(icsneoc2_app_error_type_driver_overflow, 4);
	ASSERT_EQ(icsneoc2_app_error_type_pc_buff_overflow, 5);
	ASSERT_EQ(icsneoc2_app_error_type_pc_chksum_error, 6);
	ASSERT_EQ(icsneoc2_app_error_type_pc_missed_byte, 7);
	ASSERT_EQ(icsneoc2_app_error_type_pc_overrun_error, 8);
	ASSERT_EQ(icsneoc2_app_error_type_setting_failure, 9);
	ASSERT_EQ(icsneoc2_app_error_type_too_many_selected_networks, 10);
	ASSERT_EQ(icsneoc2_app_error_type_network_not_enabled, 11);
	ASSERT_EQ(icsneoc2_app_error_type_rtc_not_correct, 12);
	ASSERT_EQ(icsneoc2_app_error_type_loaded_default_settings, 13);
	ASSERT_EQ(icsneoc2_app_error_type_feature_not_unlocked, 14);
	ASSERT_EQ(icsneoc2_app_error_type_feature_rtc_cmd_dropped, 15);
	ASSERT_EQ(icsneoc2_app_error_type_tx_messages_flushed, 16);
	ASSERT_EQ(icsneoc2_app_error_type_tx_messages_half_full, 17);
	ASSERT_EQ(icsneoc2_app_error_type_network_not_valid, 18);
	ASSERT_EQ(icsneoc2_app_error_type_tx_interface_not_implemented, 19);
	ASSERT_EQ(icsneoc2_app_error_type_tx_messages_comm_enable_is_off, 20);
	ASSERT_EQ(icsneoc2_app_error_type_rx_filter_match_count_exceeded, 21);
	ASSERT_EQ(icsneoc2_app_error_type_eth_preemption_not_enabled, 22);
	ASSERT_EQ(icsneoc2_app_error_type_tx_not_supported_in_mode, 23);
	ASSERT_EQ(icsneoc2_app_error_type_jumbo_frames_not_supported, 24);
	ASSERT_EQ(icsneoc2_app_error_type_ethernet_ip_fragment, 25);
	ASSERT_EQ(icsneoc2_app_error_type_tx_messages_underrun, 26);
	ASSERT_EQ(icsneoc2_app_error_type_device_fan_failure, 27);
	ASSERT_EQ(icsneoc2_app_error_type_device_overtemperature, 28);
	ASSERT_EQ(icsneoc2_app_error_type_tx_message_index_out_of_range, 29);
	ASSERT_EQ(icsneoc2_app_error_type_undersized_frame_dropped, 30);
	ASSERT_EQ(icsneoc2_app_error_type_oversized_frame_dropped, 31);
	ASSERT_EQ(icsneoc2_app_error_type_watchdog_event, 32);
	ASSERT_EQ(icsneoc2_app_error_type_system_clock_failure, 33);
	ASSERT_EQ(icsneoc2_app_error_type_system_clock_recovered, 34);
	ASSERT_EQ(icsneoc2_app_error_type_system_peripheral_reset, 35);
	ASSERT_EQ(icsneoc2_app_error_type_system_communication_failure, 36);
	ASSERT_EQ(icsneoc2_app_error_type_tx_messages_unsupported_source_or_packet_id, 37);
	ASSERT_EQ(icsneoc2_app_error_type_wbms_manager_connect_failed, 38);
	ASSERT_EQ(icsneoc2_app_error_type_wbms_manager_connect_bad_state, 39);
	ASSERT_EQ(icsneoc2_app_error_type_wbms_manager_connect_timeout, 40);
	ASSERT_EQ(icsneoc2_app_error_type_failed_to_initialize_logger_disk, 41);
	ASSERT_EQ(icsneoc2_app_error_type_invalid_setting, 42);
	ASSERT_EQ(icsneoc2_app_error_type_system_failure_requested_reset, 43);
	ASSERT_EQ(icsneoc2_app_error_type_port_key_mistmatch, 45);
	ASSERT_EQ(icsneoc2_app_error_type_bus_failure, 46);
	ASSERT_EQ(icsneoc2_app_error_type_tap_overflow, 47);
	ASSERT_EQ(icsneoc2_app_error_type_eth_tx_no_link, 48);
	ASSERT_EQ(icsneoc2_app_error_type_error_buffer_overflow, 254);
	ASSERT_EQ(icsneoc2_app_error_type_no_error, 255);

	using _T = icsneo::AppErrorType;
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorRxMessagesFull), icsneoc2_app_error_type_rx_messages_full);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTxMessagesFull), icsneoc2_app_error_type_tx_messages_full);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTxReportMessagesFull), icsneoc2_app_error_type_tx_report_messages_full);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorBadCommWithDspIC), icsneoc2_app_error_type_bad_comm_with_dsp_ic);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorDriverOverflow), icsneoc2_app_error_type_driver_overflow);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorPCBuffOverflow), icsneoc2_app_error_type_pc_buff_overflow);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorPCChksumError), icsneoc2_app_error_type_pc_chksum_error);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorPCMissedByte), icsneoc2_app_error_type_pc_missed_byte);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorPCOverrunError), icsneoc2_app_error_type_pc_overrun_error);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorSettingFailure), icsneoc2_app_error_type_setting_failure);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTooManySelectedNetworks), icsneoc2_app_error_type_too_many_selected_networks);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorNetworkNotEnabled), icsneoc2_app_error_type_network_not_enabled);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorRtcNotCorrect), icsneoc2_app_error_type_rtc_not_correct);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorLoadedDefaultSettings), icsneoc2_app_error_type_loaded_default_settings);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorFeatureNotUnlocked), icsneoc2_app_error_type_feature_not_unlocked);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorFeatureRtcCmdDropped), icsneoc2_app_error_type_feature_rtc_cmd_dropped);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTxMessagesFlushed), icsneoc2_app_error_type_tx_messages_flushed);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTxMessagesHalfFull), icsneoc2_app_error_type_tx_messages_half_full);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorNetworkNotValid), icsneoc2_app_error_type_network_not_valid);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTxInterfaceNotImplemented), icsneoc2_app_error_type_tx_interface_not_implemented);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTxMessagesCommEnableIsOff), icsneoc2_app_error_type_tx_messages_comm_enable_is_off);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorRxFilterMatchCountExceeded), icsneoc2_app_error_type_rx_filter_match_count_exceeded);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorEthPreemptionNotEnabled), icsneoc2_app_error_type_eth_preemption_not_enabled);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTxNotSupportedInMode), icsneoc2_app_error_type_tx_not_supported_in_mode);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorJumboFramesNotSupported), icsneoc2_app_error_type_jumbo_frames_not_supported);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorEthernetIpFragment), icsneoc2_app_error_type_ethernet_ip_fragment);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTxMessagesUnderrun), icsneoc2_app_error_type_tx_messages_underrun);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorDeviceFanFailure), icsneoc2_app_error_type_device_fan_failure);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorDeviceOvertemperature), icsneoc2_app_error_type_device_overtemperature);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTxMessageIndexOutOfRange), icsneoc2_app_error_type_tx_message_index_out_of_range);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorUndersizedFrameDropped), icsneoc2_app_error_type_undersized_frame_dropped);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorOversizedFrameDropped), icsneoc2_app_error_type_oversized_frame_dropped);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorWatchdogEvent), icsneoc2_app_error_type_watchdog_event);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorSystemClockFailure), icsneoc2_app_error_type_system_clock_failure);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorSystemClockRecovered), icsneoc2_app_error_type_system_clock_recovered);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorSystemPeripheralReset), icsneoc2_app_error_type_system_peripheral_reset);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorSystemCommunicationFailure), icsneoc2_app_error_type_system_communication_failure);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTxMessagesUnsupportedSourceOrPacketId), icsneoc2_app_error_type_tx_messages_unsupported_source_or_packet_id);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorWbmsManagerConnectFailed), icsneoc2_app_error_type_wbms_manager_connect_failed);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorWbmsManagerConnectBadState), icsneoc2_app_error_type_wbms_manager_connect_bad_state);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorWbmsManagerConnectTimeout), icsneoc2_app_error_type_wbms_manager_connect_timeout);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorFailedToInitializeLoggerDisk), icsneoc2_app_error_type_failed_to_initialize_logger_disk);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorInvalidSetting), icsneoc2_app_error_type_invalid_setting);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorSystemFailureRequestedReset), icsneoc2_app_error_type_system_failure_requested_reset);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorPortKeyMistmatch), icsneoc2_app_error_type_port_key_mistmatch);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorBusFailure), icsneoc2_app_error_type_bus_failure);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorTapOverflow), icsneoc2_app_error_type_tap_overflow);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorEthTxNoLink), icsneoc2_app_error_type_eth_tx_no_link);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppErrorErrorBufferOverflow), icsneoc2_app_error_type_error_buffer_overflow);
	ASSERT_EQ(static_cast<icsneoc2_app_error_type_t>(_T::AppNoError), icsneoc2_app_error_type_no_error);
}

TEST(icsneoc2, test_icsneoc2_message_can_flags_t)
{
	ASSERT_EQ(ICSNEOC2_MESSAGE_CAN_FLAGS_RTR, 0x01);
	ASSERT_EQ(ICSNEOC2_MESSAGE_CAN_FLAGS_IDE, 0x02);
	ASSERT_EQ(ICSNEOC2_MESSAGE_CAN_FLAGS_FDF, 0x04);
	ASSERT_EQ(ICSNEOC2_MESSAGE_CAN_FLAGS_BRS, 0x08);
	ASSERT_EQ(ICSNEOC2_MESSAGE_CAN_FLAGS_ESI, 0x10);
	ASSERT_EQ(ICSNEOC2_MESSAGE_CAN_FLAGS_TX_ABORTED, 0x20);
	ASSERT_EQ(ICSNEOC2_MESSAGE_CAN_FLAGS_TX_LOST_ARB, 0x40);
	ASSERT_EQ(ICSNEOC2_MESSAGE_CAN_FLAGS_TX_ERROR, 0x80);

	ASSERT_EQ(sizeof(icsneoc2_message_can_flags_t), sizeof(uint64_t));
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

TEST(icsneoc2, test_icsneoc2_eth_create)
{
	// NULL parameter should fail
	ASSERT_EQ(icsneoc2_error_invalid_parameters, icsneoc2_message_eth_create(NULL));

	// Create an Ethernet message
	icsneoc2_message_t* message = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_create(&message));
	ASSERT_NE(message, nullptr);

	// Verify it is an Ethernet message
	bool is_ethernet = false;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_ethernet(message, &is_ethernet));
	ASSERT_TRUE(is_ethernet);

	// Verify it is NOT a CAN message
	bool is_can = false;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_can(message, &is_can));
	ASSERT_FALSE(is_can);

	// Verify it is a frame and raw message
	bool is_frame = false;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_frame(message, &is_frame));
	ASSERT_TRUE(is_frame);

	bool is_raw = false;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_raw(message, &is_raw));
	ASSERT_TRUE(is_raw);

	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_free(message));
}

TEST(icsneoc2, test_icsneoc2_eth_props_roundtrip)
{
	icsneoc2_message_t* message = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_create(&message));

	// Set several flags
	icsneoc2_message_eth_flags_t flags_in =
		ICSNEOC2_MESSAGE_ETH_FLAGS_NO_PADDING |
		ICSNEOC2_MESSAGE_ETH_FLAGS_FCS_VERIFIED |
		ICSNEOC2_MESSAGE_ETH_FLAGS_TX_ABORTED;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_props_set(message, &flags_in, NULL, NULL));

	// Get them back
	icsneoc2_message_eth_flags_t flags_out = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_props_get(message, &flags_out, NULL, NULL));
	ASSERT_EQ(flags_in, flags_out);

	// Clear all flags
	icsneoc2_message_eth_flags_t flags_zero = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_props_set(message, &flags_zero, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_props_get(message, &flags_out, NULL, NULL));
	ASSERT_EQ(0u, flags_out);

	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_free(message));
}

TEST(icsneoc2, test_icsneoc2_eth_mac_and_ethertype)
{
	icsneoc2_message_t* message = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_create(&message));

	// Set frame data: dst MAC + src MAC + EtherType
	uint8_t frame_data[] = {
		0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, /* Destination MAC */
		0x11, 0x22, 0x33, 0x44, 0x55, 0x66, /* Source MAC */
		0x08, 0x00,                           /* EtherType (IPv4) */
		0x01, 0x02, 0x03, 0x04               /* Payload */
	};
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_data_set(message, frame_data, sizeof(frame_data)));

	// Get MAC addresses
	uint8_t dst_mac[6] = {0};
	uint8_t src_mac[6] = {0};
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_mac_get(message, dst_mac, src_mac));
	ASSERT_EQ(0, memcmp(dst_mac, frame_data, 6));
	ASSERT_EQ(0, memcmp(src_mac, frame_data + 6, 6));

	// Get just one MAC at a time (NULL-safe)
	uint8_t dst_only[6] = {0};
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_mac_get(message, dst_only, NULL));
	ASSERT_EQ(0, memcmp(dst_only, frame_data, 6));

	uint8_t src_only[6] = {0};
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_mac_get(message, NULL, src_only));
	ASSERT_EQ(0, memcmp(src_only, frame_data + 6, 6));

	// Get EtherType
	uint16_t ether_type = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_ether_type_get(message, &ether_type));
	ASSERT_EQ(0x0800, ether_type);

	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_free(message));
}

TEST(icsneoc2, test_icsneoc2_eth_mac_too_short)
{
	icsneoc2_message_t* message = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_create(&message));

	// Set data too short for MAC extraction (< 14 bytes)
	uint8_t short_data[] = {0x01, 0x02, 0x03};
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_data_set(message, short_data, sizeof(short_data)));

	uint8_t dst_mac[6] = {0};
	ASSERT_EQ(icsneoc2_error_invalid_data, icsneoc2_message_eth_mac_get(message, dst_mac, NULL));

	uint8_t src_mac[6] = {0};
	ASSERT_EQ(icsneoc2_error_invalid_data, icsneoc2_message_eth_mac_get(message, NULL, src_mac));

	uint16_t ether_type = 0;
	ASSERT_EQ(icsneoc2_error_invalid_data, icsneoc2_message_eth_ether_type_get(message, &ether_type));

	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_free(message));
}

TEST(icsneoc2, test_icsneoc2_eth_t1s_props_roundtrip)
{
	icsneoc2_message_t* message = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_create(&message));

	// Set T1S properties
	icsneoc2_message_eth_t1s_flags_t flags_in =
		ICSNEOC2_MESSAGE_ETH_T1S_FLAGS_IS_T1S_SYMBOL |
		ICSNEOC2_MESSAGE_ETH_T1S_FLAGS_TX_COLLISION;
	uint8_t node_id = 42;
	uint8_t burst_count = 7;
	uint8_t symbol_type = 3;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_t1s_props_set(message, &flags_in, &node_id, &burst_count, &symbol_type));

	// Get them back
	icsneoc2_message_eth_t1s_flags_t flags_out = 0;
	uint8_t node_id_out = 0, burst_count_out = 0, symbol_type_out = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_t1s_props_get(message, &flags_out, &node_id_out, &burst_count_out, &symbol_type_out));
	ASSERT_EQ(flags_in, flags_out);
	ASSERT_EQ(42, node_id_out);
	ASSERT_EQ(7, burst_count_out);
	ASSERT_EQ(3, symbol_type_out);

	// Set just one at a time (NULL-safe)
	uint8_t new_node_id = 99;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_t1s_props_set(message, NULL, &new_node_id, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_t1s_props_get(message, &flags_out, &node_id_out, &burst_count_out, &symbol_type_out));
	ASSERT_EQ(flags_in, flags_out);
	ASSERT_EQ(99, node_id_out);
	ASSERT_EQ(7, burst_count_out); // Unchanged
	ASSERT_EQ(3, symbol_type_out); // Unchanged

	// Passing all NULL parameters clears the optional T1S state.
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_t1s_props_set(message, NULL, NULL, NULL, NULL));
	flags_out = 0xFF;
	node_id_out = 0xFF;
	burst_count_out = 0xFF;
	symbol_type_out = 0xFF;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_t1s_props_get(message, &flags_out, &node_id_out, &burst_count_out, &symbol_type_out));
	ASSERT_EQ(0, flags_out);
	ASSERT_EQ(0, node_id_out);
	ASSERT_EQ(0, burst_count_out);
	ASSERT_EQ(0, symbol_type_out);

	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_free(message));
}

TEST(icsneoc2, test_icsneoc2_eth_fcs_roundtrip)
{
	icsneoc2_message_t* message = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_create(&message));

	// Initially, FCS should not be set
	bool has_fcs = true;
	uint32_t fcs = 0;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_props_get(message, NULL, &has_fcs, &fcs));
	ASSERT_FALSE(has_fcs);

	// Set an FCS value via eth_props_set
	uint32_t fcs_value = 0xDEADBEEF;
	has_fcs = true;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_props_set(message, NULL, &has_fcs, &fcs_value));

	// Get it back
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_props_get(message, NULL, &has_fcs, &fcs));
	ASSERT_TRUE(has_fcs);
	ASSERT_EQ(0xDEADBEEF, fcs);

	// Clear FCS
	has_fcs = false;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_props_set(message, NULL, &has_fcs, NULL));
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_eth_props_get(message, NULL, &has_fcs, &fcs));
	ASSERT_FALSE(has_fcs);

	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_free(message));
}

TEST(icsneoc2, test_icsneoc2_eth_invalid_type)
{
	// Create a CAN message and try to use Ethernet functions on it
	icsneoc2_message_t* can_msg = nullptr;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_can_create(&can_msg));

	bool is_ethernet = true;
	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_is_ethernet(can_msg, &is_ethernet));
	ASSERT_FALSE(is_ethernet);

	icsneoc2_message_eth_flags_t flags = 0;
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_eth_props_get(can_msg, &flags, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_eth_props_set(can_msg, &flags, NULL, NULL));

	uint8_t mac[6] = {0};
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_eth_mac_get(can_msg, mac, NULL));

	uint16_t ether_type = 0;
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_eth_ether_type_get(can_msg, &ether_type));

	uint8_t val = 0;
	icsneoc2_message_eth_t1s_flags_t t1s_flags = 0;
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_eth_t1s_props_get(can_msg, &t1s_flags, &val, NULL, NULL));
	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_eth_t1s_props_set(can_msg, &t1s_flags, &val, NULL, NULL));

	ASSERT_EQ(icsneoc2_error_invalid_type, icsneoc2_message_eth_props_set(can_msg, NULL, NULL, NULL));

	ASSERT_EQ(icsneoc2_error_success, icsneoc2_message_free(can_msg));
}

TEST(icsneoc2, test_gptp_enum_values)
{
	ASSERT_EQ(icsneoc2_gptp_profile_standard,   0);
	ASSERT_EQ(icsneoc2_gptp_profile_automotive, 1);
	ASSERT_EQ(sizeof(icsneoc2_gptp_profile_t), sizeof(uint8_t));

	ASSERT_EQ(icsneoc2_gptp_role_disabled, 0);
	ASSERT_EQ(icsneoc2_gptp_role_passive,  1);
	ASSERT_EQ(icsneoc2_gptp_role_master,   2);
	ASSERT_EQ(icsneoc2_gptp_role_slave,    3);
	ASSERT_EQ(sizeof(icsneoc2_gptp_role_t), sizeof(uint8_t));
}

TEST(icsneoc2, test_gptp_enum_alignment)
{
	ASSERT_EQ(RADGPTPProfile::RAD_GPTP_PROFILE_STANDARD,   icsneoc2_gptp_profile_standard);
	ASSERT_EQ(RADGPTPProfile::RAD_GPTP_PROFILE_AUTOMOTIVE, icsneoc2_gptp_profile_automotive);
	ASSERT_EQ(sizeof(RADGPTPProfile), sizeof(icsneoc2_gptp_profile_t));

	ASSERT_EQ(RADGPTPRole::RAD_GPTP_ROLE_DISABLED, icsneoc2_gptp_role_disabled);
	ASSERT_EQ(RADGPTPRole::RAD_GPTP_ROLE_PASSIVE,  icsneoc2_gptp_role_passive);
	ASSERT_EQ(RADGPTPRole::RAD_GPTP_ROLE_MASTER,   icsneoc2_gptp_role_master);
	ASSERT_EQ(RADGPTPRole::RAD_GPTP_ROLE_SLAVE,    icsneoc2_gptp_role_slave);
	ASSERT_EQ(sizeof(RADGPTPRole), sizeof(icsneoc2_gptp_role_t));
}

