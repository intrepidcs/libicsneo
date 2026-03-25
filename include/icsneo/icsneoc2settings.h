#ifndef __ICSNEOC2SETTINGS_H_
#define __ICSNEOC2SETTINGS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2types.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Apply the default settings for a device
 *
 * @param[in] device The device to apply the settings for.
 * @param[in] save True to make the settings permanent, false will be reverted on power cycle.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_set_settings_failure otherwise.
 */
icsneoc2_error_t icsneoc2_settings_apply_defaults(icsneoc2_device_t* device, bool save);

/**
 * Apply the current settings for a device.
 *
 * This makes all settings persistent across power cycles.
 *
 * @param[in] device The device to apply the settings for.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_set_settings_failure otherwise.
 */
icsneoc2_error_t icsneoc2_settings_apply(icsneoc2_device_t* device);

/**
 * Refresh the settings for a device. This reads the current settings from the device,
 * overwriting any unsaved local changes.
 *
 * @param[in] device The device to refresh the settings for.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_get_settings_failure otherwise.
 */
icsneoc2_error_t icsneoc2_settings_refresh(icsneoc2_device_t* device);

/**
 * Get the baudrate for a network
 *
 * @note @see icsneoc2_settings_canfd_baudrate_get for CANFD.
 *
 * @param[in] device The device to get the baudrate value.
 * @param[in] netid The network to get the baudrate value.
 * @param[out] baudrate Pointer to store the baudrate value.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_baudrate_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, int64_t* baudrate);

/**
 * Set the baudrate for a network
 *
 * @note @see icsneoc2_settings_canfd_baudrate_set for CANFD.
 *
 * @param[in] device The device to set the baudrate for.
 * @param[in] netid The network to set the baudrate for.
 * @param[in] baudrate The baudrate to set the network to.
 * @param[in] save True to make the settings permanent, false will be reverted on power cycle.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_baudrate_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, int64_t baudrate);

/**
 * Get the baudrate for a CAN FD network
 *
 * @param[in] device The device to get the baudrate value.
 * @param[in] netid The network to get the baudrate value.
 * @param[out] baudrate Pointer to store the baudrate value.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_canfd_baudrate_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, int64_t* baudrate);

/**
 * Set the baudrate for a CANFD network
 *
 * @param[in] device The device to set the baudrate for.
 * @param[in] netid The network to set the baudrate for.
 * @param[in] baudrate The baudrate to set the network to.
 * @param[in] save True to make the settings permanent, false will be reverted on power cycle.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_canfd_baudrate_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, int64_t baudrate);

/**
 * Check if termination is supported for a network.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check termination support for.
 * @param[out] supported Pointer to store whether termination is supported.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_termination_is_supported(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* supported);

/**
 * Check if termination can be enabled for a network.
 *
 * Termination may be supported but not currently able to be enabled (e.g., due to hardware constraints).
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] can_enable Pointer to store whether termination can be enabled.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_termination_can_enable(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* can_enable);

/**
 * Check if termination is currently enabled for a network.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] enabled Pointer to store whether termination is currently enabled.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_termination_is_enabled(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* enabled);

/**
 * Enable or disable termination for a network.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure termination for.
 * @param[in] enable True to enable termination, false to disable.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_set_settings_failure otherwise.
 */
icsneoc2_error_t icsneoc2_settings_termination_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool enable);

// TODO: getTerminationGroups

/**
 * Check if the commander resistor is currently enabled for a network.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] enabled Pointer to store whether the commander resistor is currently enabled.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_commander_resistor_enabled(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* enabled);

/**
 * Enable or disable the commander resistor for a network.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure the commander resistor for.
 * @param[in] enable True to enable the commander resistor, false to disable.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_set_settings_failure otherwise.
 */
icsneoc2_error_t icsneoc2_settings_commander_resistor_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool enable);

/**
 * Get the LIN mode for a given network in the currently active device settings.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] value Pointer to store the LIN mode.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_lin_mode_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_lin_mode_t* value);

// TODO: setLINModeFor
/**
 * Set the LIN mode for a given network in the currently active device settings.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure.
 * @param[in] value The LIN mode to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_lin_mode_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_lin_mode_t value);

/**
 * Get number of bit delays between commander ID and first responder byte for
 * a given network in the currently active device settings.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] value Pointer to store the number of bit delays.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_lin_commander_response_time_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value);

/**
 * Set number of bit delays between commander ID and first responder byte for
 * a given network in the currently active device settings.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure.
 * @param[in] value The number of bit delays to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_lin_commander_response_time_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value);

/**
 * Get PHY enable state for switch devices (Epsilon/XL, Jupiter, etc) using port index.
 * For all other devices, use icsneoc2_settings_phy_enable_for_get() instead.
 *
 * @param[in] device The device to check.
 * @param[in] index The port index to check.
 * @param[out] value Pointer to store the PHY enable state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_enable_get(icsneoc2_device_t* device, uint8_t index, bool* value);

/**
 * Enable/disable PHY for switch devices (Epsilon/XL, Jupiter, etc) using port index.
 * For all other devices, use icsneoc2_settings_phy_enable_for_set() instead.
 *
 * @param[in] device The device to configure.
 * @param[in] index The port index to configure.
 * @param[in] value bool to set the PHY enable state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_enable_set(icsneoc2_device_t* device, uint8_t index, bool value);

/**
 * Get PHY role (Master/Slave/Auto) for switch devices (Epsilon/XL, Jupiter, etc) using port index.
 * For all other devices, use icsneoc2_settings_phy_role_for_get() instead.
 *
 * @param[in] device The device to check.
 * @param[in] index The port index to check.
 * @param[out] value Pointer to store the link mode state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_mode_get(icsneoc2_device_t* device, uint8_t index, icsneoc2_ae_link_mode_t* value);

/**
 * Set PHY role (Master/Slave/Auto) for switch devices (Epsilon/XL, Jupiter, etc) using port index.
 * For all other devices, use icsneoc2_settings_phy_role_for_set() instead.
 *
 * @param[in] device The device to configure.
 * @param[in] index The port index to configure.
 * @param[in] value The link mode state to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_mode_set(icsneoc2_device_t* device, uint8_t index, icsneoc2_ae_link_mode_t value);

/**
 * Get PHY link mode (speed and duplex) for switch devices (Epsilon/XL, Jupiter, etc) using port index.
 * For all other devices, use icsneoc2_settings_phy_link_mode_for_get() instead.
 *
 * @param[in] device The device to check.
 * @param[in] index The port index to check.
 * @param[out] value Pointer to store the link mode state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_speed_get(icsneoc2_device_t* device, uint8_t index, icsneoc2_eth_phy_link_mode_t* value);

/**
 * Set PHY link mode (speed and duplex) for switch devices (Epsilon/XL, Jupiter, etc) using port index.
 * For all other devices, use icsneoc2_settings_phy_link_mode_for_set() instead.
 *
 * @param[in] device The device to configure.
 * @param[in] index The port index to configure.
 * @param[in] value The link mode state to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_speed_set(icsneoc2_device_t* device, uint8_t index, icsneoc2_eth_phy_link_mode_t value);

/**
 * Get PHY role (Master/Slave/Auto) for network-based devices.
 * For switch devices, use icsneoc2_settings_phy_mode_get() with port index instead.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] value Pointer to store the PHY role state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_role_for_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_ae_link_mode_t* value);

/**
 * Set PHY role (Master/Slave/Auto) for network-based devices.
 * For switch devices, use icsneoc2_settings_phy_mode_set() with port index instead.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure.
 * @param[in] value The PHY role state to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_role_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_ae_link_mode_t value);

/**
 * Get PHY link mode (speed and duplex) for network-based devices.
 * For switch devices, use icsneoc2_settings_phy_speed_get() with port index instead.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] value Pointer to store the PHY link mode state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_link_mode_for_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_eth_phy_link_mode_t* value);

/**
 * Set PHY link mode (speed and duplex) for network-based devices.
 * For switch devices, use icsneoc2_settings_phy_speed_set() with port index instead.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure.
 * @param[in] value The PHY link mode to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_set_settings_failure otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_link_mode_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_eth_phy_link_mode_t value);

/**
 * Get PHY enable state for network-based devices.
 * For switch devices, use icsneoc2_settings_phy_enable_get() with port index instead.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] value Pointer to store the PHY enable state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_enable_for_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* value);

/**
 * Enable/disable PHY for network-based devices.
 * For switch devices, use icsneoc2_settings_phy_enable_set() with port index instead.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure.
 * @param[in] value The PHY enable state to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_phy_enable_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool value);

/**
 * Get supported PHY link modes (combined speed+duplex) for a network.
 * Each mode represents a valid hardware configuration.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] link_modes Pointer to store the supported PHY link modes. Should be at least the size of icsneoc2_eth_phy_link_mode_maxsize.
 * @param[in,out] link_modes_count Pointer to the size of link_modes array. Updated with the number of link modes written to the array.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_supported_phy_link_modes_for(icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_eth_phy_link_mode_t** link_modes, size_t* link_modes_count);

/**
 * Check if PLCA is enabled for a network that supports 10BASE-T1S.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] value Pointer to store the PLCA enable state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_t1s_is_plca_enabled_for(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool* value);

/**
 * Set PLCA enabled for a network that supports 10BASE-T1S.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure.
 * @param[in] value The PLCA enable state to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_t1s_plca_enabled_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, bool value);

/**
 * Get the local ID for a network that supports 10BASE-T1S.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] value Pointer to store the local ID.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_t1s_local_id_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value);

/**
 * Set the local ID for a network that supports 10BASE-T1S.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure.
 * @param[in] value The local ID to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_t1s_local_id_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value);

/**
 * Get the maximum number of nodes for a network that supports 10BASE-T1S.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] value Pointer to store the maximum number of nodes.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_t1s_max_nodes_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value);

/**
 * Set the maximum number of nodes for a network that supports 10BASE-T1S.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure.
 * @param[in] value The maximum number of nodes to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_t1s_max_nodes_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value);

/**
 * Get the OOP Timer for a network that supports 10BASE-T1S.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] value Pointer to store the OOP Timer value.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_t1s_tx_opp_timer_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value);

/**
 * Set the OOP Timer for a network that supports 10BASE-T1S.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure.
 * @param[in] value The OOP Timer value to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_t1s_tx_opp_timer_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value);

/**
 * Get the Max burst timer for a network that supports 10BASE-T1S.
 *
 * @param[in] device The device to check.
 * @param[in] netid The network ID to check.
 * @param[out] value Pointer to store the Max burst timer value.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_t1s_max_burst_timer_for_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t* value);

/**
 * Set the Max burst timer for a network that supports 10BASE-T1S.
 *
 * @param[in] device The device to configure.
 * @param[in] netid The network ID to configure.
 * @param[in] value The Max burst timer value to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_t1s_max_burst_timer_for_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t value);

/**
 * Set the analog output enabled.
 *
 * @param[in] device The device to configure.
 * @param[in] pin The analog output pin to configure.
 * @param[in] value The analog output value to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_misc_io_analog_output_enabled_set(icsneoc2_device_t* device, uint8_t pin, uint8_t value);

/**
 * Set the analog output voltage for a misc IO pin.
 *
 * @param[in] device The device to configure.
 * @param[in] pin The analog output pin to configure.
 * @param[in] value The analog output voltage to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_misc_io_analog_output_set(icsneoc2_device_t* device, uint8_t pin, icsneoc2_misc_io_analog_voltage_t value);

/**
 * Check if settings are disabled.
 *
 * @param[in] device The device to check.
 * @param[out] value Pointer to store the disabled state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_disabled_get(icsneoc2_device_t* device, bool* value);

/**
 * Check if settings are read-only.
 *
 * @param[in] device The device to check.
 * @param[out] value Pointer to store the read-only state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_settings_readonly_get(icsneoc2_device_t* device, bool* value);

#ifdef __cplusplus
}
#endif

#endif // __ICSNEOC2SETTINGS_H_
