pub mod ffi;

use ffi::_icsneoc2_error_t::{icsneoc2_error_invalid_device, icsneoc2_error_success};
use ffi::_icsneoc2_open_options_t::*;
use ffi::*;

use std::ffi::CString;
use std::fmt::Display;

#[derive(Debug, Clone)]
pub enum Error {
    /// icsneoc2 API error
    APIError(icsneoc2_error_t, String),
}

impl Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Error::APIError(error_code, error_string) => write!(f, "API Error: \"{}\" ({})", error_string, error_code),
        }
    }
}

pub type Result<T> = core::result::Result<T, Error>;

#[derive(Debug, Clone)]
pub struct Device {
    /// Handle to the device.
    pub handle: *mut icsneoc2_device_t,
}

impl Drop for Device {
    fn drop(&mut self) {
        // Make sure the device is closed before we go out of scope.
        self.device_close().expect("Failed to close device!");
    }
}

impl Device {
    pub fn find_all(device_count: u32) -> Result<Vec<Self>> {
        // Find all the devices
        const MAX_DEVICE_COUNT: u32 = 255;
        let mut devices: [*mut icsneoc2_device_t; MAX_DEVICE_COUNT as usize] =
            [std::ptr::null_mut(); 255];
        let mut devices_count: u32 = if device_count > MAX_DEVICE_COUNT {
            MAX_DEVICE_COUNT
        } else {
            device_count
        };
        let res = unsafe {
            icsneoc2_device_find_all(
                devices.as_mut_ptr(),
                &mut devices_count as *mut u32,
                std::ptr::null_mut(),
            )
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        // Return the results
        let found_devices: Vec<Self> = devices[..devices_count as usize]
            .iter()
            .map(|d| Self { handle: *d })
            .collect();
        Ok(found_devices)
    }

    pub fn bus_type_name_get(&self, bus_type: icsneoc2_msg_bus_type_t) -> Result<String> {
        // Get the string
        let mut str: Vec<u8> = vec![0; 255];
        let mut str_length: u32 = 255;
        let res = unsafe {
            icsneoc2_bus_type_name_get(bus_type, str.as_mut_ptr() as *mut i8, &mut str_length)
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        // Convert the vec to an String
        str.resize(str_length as usize, 0);
        let str_string = {
            CString::new(str)
                .expect("CString::new failed")
                .into_string()
                .expect("CString::new::into_string")
        };
        Ok(str_string)
    }

    pub fn device_baudrate_get(&self, netid: _icsneoc2_netid_t) -> Result<u64> {
        let mut value: u64 = 0;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_baudrate_get(self.handle, netid as icsneoc2_netid_t, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn device_baudrate_set(
        &self,
        netid: _icsneoc2_netid_t,
        value: u64,
        save: bool,
    ) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_baudrate_set(self.handle, netid as icsneoc2_netid_t, value, save) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn device_canfd_baudrate_get(&self, netid: _icsneoc2_netid_t) -> Result<u64> {
        let mut value: u64 = 0;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_canfd_baudrate_get(self.handle, netid as icsneoc2_netid_t, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn device_canfd_baudrate_set(
        &self,
        netid: _icsneoc2_netid_t,
        value: u64,
        save: bool,
    ) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_canfd_baudrate_set(self.handle, netid as icsneoc2_netid_t, value, save) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn device_close(&self) -> Result<()> {
        let res: icsneoc2_error_t = unsafe { ffi::icsneoc2_device_close(self.handle) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn device_description_get(&self) -> Result<String> {
        // Get the error code
        let mut description: Vec<u8> = vec![0; 255];
        let mut description_length: u32 = 255;
        let res = unsafe {
            icsneoc2_device_description_get(
                self.handle,
                description.as_mut_ptr() as *mut i8,
                &mut description_length,
            )
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        // Convert the vec to an String
        description.resize(description_length as usize, 0);
        let description_str = {
            CString::new(description)
                .expect("CString::new failed")
                .into_string()
                .expect("CString::new::into_string")
        };
        Ok(description_str)
    }

    pub fn device_events_get(&self, event_count: u32) -> Result<Vec<*mut icsneoc2_event_t>> {
        let mut events: Vec<*mut icsneoc2_event_t> =
            vec![std::ptr::null_mut(); event_count as usize];
        let mut event_count: u32 = event_count;
        let res: icsneoc2_error_t = unsafe {
            ffi::icsneoc2_device_events_get(self.handle, events.as_mut_ptr(), &mut event_count)
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        events.resize(event_count as usize, std::ptr::null_mut());
        Ok(events)
    }

    pub fn device_go_online(&self, go_online: bool) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_go_online(self.handle, go_online) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn device_is_online(&self) -> Result<bool> {
        let mut is_online: bool = false;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_is_online(self.handle, &mut is_online) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(is_online)
    }

    pub fn device_is_online_supported(&self) -> Result<bool> {
        let mut is_online_supported: bool = false;
        let res: icsneoc2_error_t = unsafe {
            ffi::icsneoc2_device_is_online_supported(self.handle, &mut is_online_supported)
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(is_online_supported)
    }

    pub fn device_is_valid(&self) -> Result<bool> {
        let res: icsneoc2_error_t = unsafe { icsneoc2_device_is_valid(self.handle) };
        match res {
            res if res == icsneoc2_error_success as icsneoc2_error_t => Ok(true),
            res if res == icsneoc2_error_invalid_device as icsneoc2_error_t => Ok(false),
            _ => Err(Self::error_code_get(res)),
        }
    }

    pub fn device_is_open(&self) -> Result<bool> {
        let mut is_open: bool = false;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_is_open(self.handle, &mut is_open) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(is_open)
    }

    pub fn device_is_disconnected(&self) -> Result<bool> {
        let mut is_disconnected: bool = false;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_is_disconnected(self.handle, &mut is_disconnected) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(is_disconnected)
    }

    pub fn device_load_default_settings(&self, save: bool) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_load_default_settings(self.handle, save) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn device_message_count_get(&self) -> Result<u32> {
        let mut value: u32 = 0;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_message_count_get(self.handle, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn device_message_polling_get(&self) -> Result<bool> {
        let mut value: bool = false;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_message_polling_get(self.handle, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn device_message_polling_limit_get(&self) -> Result<u32> {
        let mut value: u32 = 0;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_message_polling_limit_get(self.handle, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn device_message_polling_set(&self, enable: bool) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_message_polling_set(self.handle, enable) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn device_message_polling_set_limit(&self, value: u32) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_message_polling_set_limit(self.handle, value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn device_messages_get(
        &self,
        message_count: u32,
        timeout_ms: u32,
    ) -> Result<Vec<*mut icsneoc2_message_t>> {
        let mut messages: Vec<*mut icsneoc2_message_t> =
            vec![std::ptr::null_mut(); message_count as usize];
        let mut message_count: u32 = message_count;
        let res: icsneoc2_error_t = unsafe {
            ffi::icsneoc2_device_messages_get(
                self.handle,
                messages.as_mut_ptr(),
                &mut message_count,
                timeout_ms,
            )
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        messages.resize(message_count as usize, std::ptr::null_mut());
        Ok(messages)
    }

    pub fn device_messages_transmit(&self, messages: Vec<*mut icsneoc2_message_t>) -> Result<u32> {
        let mut messages_count: u32 = messages.len() as u32;
        let mut messages = messages;
        let res: icsneoc2_error_t = unsafe {
            ffi::icsneoc2_device_messages_transmit(
                self.handle,
                messages.as_mut_ptr(),
                &mut messages_count,
            )
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(messages_count)
    }

    pub fn device_open(&self) -> Result<()> {
        let res: icsneoc2_error_t = unsafe { ffi::icsneoc2_device_open(self.handle) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn device_open_options_get(&self) -> Result<ffi::icsneoc2_open_options_t> {
        let mut open_options: ffi::icsneoc2_open_options_t =
            icsneoc2_open_options_none as ffi::icsneoc2_open_options_t;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_open_options_get(self.handle, &mut open_options) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(open_options)
    }

    pub fn device_open_options_set(
        &self,
        open_options: ffi::icsneoc2_open_options_t,
    ) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_open_options_set(self.handle, open_options) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn device_rtc_get(&self) -> Result<i64> {
        let mut value: i64 = 0;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_rtc_get(self.handle, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn device_rtc_set(&self, value: i64) -> Result<()> {
        let res: icsneoc2_error_t = unsafe { ffi::icsneoc2_device_rtc_set(self.handle, value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn device_serial_get(&self) -> Result<String> {
        // Get the string
        let mut str: Vec<u8> = vec![0; 255];
        let mut str_length: u32 = 255;
        let res = unsafe {
            icsneoc2_device_serial_get(self.handle, str.as_mut_ptr() as *mut i8, &mut str_length)
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        // Convert the vec to an String
        str.resize(str_length as usize, 0);
        let str_string = {
            CString::new(str)
                .expect("CString::new failed")
                .into_string()
                .expect("CString::new::into_string")
        };
        Ok(str_string)
    }

    pub fn device_supports_tc10(&self) -> Result<bool> {
        let mut value: bool = false;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_supports_tc10(self.handle, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn device_timestamp_resolution_get(&self) -> Result<u32> {
        let mut timestamp_resolution: u32 = 0;
        let res: icsneoc2_error_t = unsafe {
            ffi::icsneoc2_device_timestamp_resolution_get(self.handle, &mut timestamp_resolution)
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(timestamp_resolution)
    }

    pub fn device_type_from_type(&self, device_type: icsneoc2_devicetype_t) -> Result<String> {
        // Get the string
        let mut str: Vec<u8> = vec![0; 255];
        let mut str_length: u32 = 255;
        let res = unsafe {
            icsneoc2_device_type_name_get(
                device_type,
                str.as_mut_ptr() as *mut i8,
                &mut str_length,
            )
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        // Convert the vec to an String
        str.resize(str_length as usize, 0);
        let str_string = {
            CString::new(str)
                .expect("CString::new failed")
                .into_string()
                .expect("CString::new::into_string")
        };
        Ok(str_string)
    }

    pub fn device_type_get(&self) -> Result<icsneoc2_devicetype_t> {
        let mut value: icsneoc2_devicetype_t = 0;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_device_type_get(self.handle, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn error_code_get(error_code: icsneoc2_error_t) -> Error {
        // Get the error code
        let mut error_description: Vec<u8> = vec![0; 255];
        let mut error_description_length: u32 = 255;
        let res = unsafe {
            icsneoc2_error_code_get(
                error_code,
                error_description.as_mut_ptr() as *mut i8,
                &mut error_description_length,
            )
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Error::APIError(res, "icsneoc2_error_code_get() failed.".to_string());
        }
        // Convert the vec to an String
        error_description.resize(error_description_length as usize, 0);
        let error_str = CString::new(error_description)
            .expect("CString::new failed")
            .into_string()
            .expect("CString::new::into_string");
        Error::APIError(error_code, error_str)
    }

    pub fn event_description_get(&self, event: *mut icsneoc2_event_t) -> Result<String> {
        // Get the string
        let mut str: Vec<u8> = vec![0; 255];
        let mut str_length: u32 = 255;
        let res = unsafe {
            icsneoc2_event_description_get(event, str.as_mut_ptr() as *mut i8, &mut str_length)
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        // Convert the vec to an String
        str.resize(str_length as usize, 0);
        let str_string = {
            CString::new(str)
                .expect("CString::new failed")
                .into_string()
                .expect("CString::new::into_string")
        };
        Ok(str_string)
    }

    pub fn events_get(&self, event_count: u32) -> Result<Vec<*mut icsneoc2_event_t>> {
        let mut events: Vec<*mut icsneoc2_event_t> =
            vec![std::ptr::null_mut(); event_count as usize];
        let mut event_count: u32 = event_count;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_events_get(events.as_mut_ptr(), &mut event_count) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        events.resize(event_count as usize, std::ptr::null_mut());
        Ok(events)
    }

    pub fn message_bus_type_get(
        &self,
        message: *mut icsneoc2_message_t,
    ) -> Result<icsneoc2_msg_bus_type_t> {
        let mut value: icsneoc2_msg_bus_type_t = 0;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_bus_type_get(self.handle, message, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn message_can_arbid_get(&self, message: *mut icsneoc2_message_t) -> Result<u32> {
        let mut value: u32 = 0;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_can_arbid_get(self.handle, message, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn message_can_arbid_set(
        &self,
        message: *mut icsneoc2_message_t,
        value: u32,
    ) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_can_arbid_set(self.handle, message, value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn message_can_baudrate_switch_get(
        &self,
        message: *mut icsneoc2_message_t,
    ) -> Result<bool> {
        let mut value: bool = false;
        let res: icsneoc2_error_t = unsafe {
            ffi::icsneoc2_message_can_baudrate_switch_get(self.handle, message, &mut value)
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn message_can_baudrate_switch_set(
        &self,
        message: *mut icsneoc2_message_t,
        value: bool,
    ) -> Result<()> {
        let res: icsneoc2_error_t = unsafe {
            ffi::icsneoc2_message_can_baudrate_switch_set(self.handle, message, value)
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn message_can_canfd_set(
        &self,
        message: *mut icsneoc2_message_t,
        value: bool,
    ) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_can_canfd_set(self.handle, message, value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn message_can_create(&self, message_count: u32) -> Result<Vec<*mut icsneoc2_message_t>> {
        let mut messages: Vec<*mut icsneoc2_message_t> =
            vec![std::ptr::null_mut(); message_count as usize];
        let res: icsneoc2_error_t = unsafe {
            ffi::icsneoc2_message_can_create(self.handle, messages.as_mut_ptr(), message_count)
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(messages)
    }

    pub fn message_can_dlc_get(&self, message: *mut icsneoc2_message_t) -> Result<i32> {
        let mut value: i32 = 0;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_can_dlc_get(self.handle, message, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn message_can_dlc_set(&self, message: *mut icsneoc2_message_t, value: i32) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_can_dlc_set(self.handle, message, value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn message_can_error_state_indicator_get(
        &self,
        message: *mut icsneoc2_message_t,
    ) -> Result<bool> {
        let mut value: bool = false;
        let res: icsneoc2_error_t = unsafe {
            ffi::icsneoc2_message_can_error_state_indicator_get(self.handle, message, &mut value)
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn message_can_extended_set(
        &self,
        message: *mut icsneoc2_message_t,
        value: bool,
    ) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_can_extended_set(self.handle, message, value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn message_can_free(&self, message: *mut icsneoc2_message_t) -> Result<()> {
        let res: icsneoc2_error_t = unsafe { ffi::icsneoc2_message_can_free(self.handle, message) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn message_can_is_canfd(&self, message: *mut icsneoc2_message_t) -> Result<bool> {
        let mut value: bool = false;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_can_is_canfd(self.handle, message, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn message_can_is_extended(&self, message: *mut icsneoc2_message_t) -> Result<bool> {
        let mut value: bool = false;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_can_is_extended(self.handle, message, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn message_can_is_remote(&self, message: *mut icsneoc2_message_t) -> Result<bool> {
        let mut value: bool = false;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_can_is_remote(self.handle, message, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn message_data_get(
        &self,
        message: *mut icsneoc2_message_t,
        data_length: u32,
    ) -> Result<Vec<u8>> {
        let mut data: Vec<u8> = vec![0; data_length as usize];
        let mut data_length = data_length;
        let res: icsneoc2_error_t = unsafe {
            ffi::icsneoc2_message_data_get(
                self.handle,
                message,
                data.as_mut_ptr(),
                &mut data_length,
            )
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        data.resize(data_length as usize, 0);
        Ok(data)
    }

    pub fn message_data_set(&self, message: *mut icsneoc2_message_t, data: Vec<u8>) -> Result<()> {
        let mut data = data;
        let res: icsneoc2_error_t = unsafe {
            ffi::icsneoc2_message_data_set(
                self.handle,
                message,
                data.as_mut_ptr(),
                data.len() as u32,
            )
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn message_is_transmit(&self, message: *mut icsneoc2_message_t) -> Result<bool> {
        let mut value: bool = false;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_is_transmit(self.handle, message, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn message_is_valid(&self, message: *mut icsneoc2_message_t) -> Result<bool> {
        let mut value: bool = false;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_is_valid(self.handle, message, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn message_netid_get(&self, message: *mut icsneoc2_message_t) -> Result<_icsneoc2_netid_t> {
        let mut value: icsneoc2_netid_t = 0;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_netid_get(self.handle, message, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        // TODO: This is gross, there is probably a better way to do this.
        // TryFrom impl would be a lot of work to implement because there are so many values.
        // We are relying on the function call to always return a valid value.
        let value: _icsneoc2_netid_t = unsafe { std::mem::transmute(value as u32) };
        Ok(value)
    }

    pub fn message_netid_set(
        &self,
        message: *mut icsneoc2_message_t,
        value: _icsneoc2_netid_t,
    ) -> Result<()> {
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_netid_set(self.handle, message, value as icsneoc2_netid_t) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(())
    }

    pub fn message_type_get(
        &self,
        message: *mut icsneoc2_message_t,
    ) -> Result<icsneoc2_msg_type_t> {
        let mut value: icsneoc2_msg_type_t = 0;
        let res: icsneoc2_error_t =
            unsafe { ffi::icsneoc2_message_type_get(self.handle, message, &mut value) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        Ok(value)
    }

    pub fn message_type_name_get(&self, msg_type: icsneoc2_msg_type_t) -> Result<String> {
        // Get the string
        let mut str: Vec<u8> = vec![0; 255];
        let mut str_length: u32 = 255;
        let res = unsafe {
            icsneoc2_message_type_name_get(msg_type, str.as_mut_ptr() as *mut i8, &mut str_length)
        };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        // Convert the vec to an String
        str.resize(str_length as usize, 0);
        let str_string = {
            CString::new(str)
                .expect("CString::new failed")
                .into_string()
                .expect("CString::new::into_string")
        };
        Ok(str_string)
    }

    pub fn netid_name_get(&self, netid: _icsneoc2_netid_t) -> Result<String> {
        // Get the string
        let mut str: Vec<u8> = vec![0; 255];
        let mut str_length: u32 = 255;
        let res =
            unsafe { icsneoc2_netid_name_get(netid as icsneoc2_netid_t, str.as_mut_ptr() as *mut i8, &mut str_length) };
        // Check the error code
        if res != icsneoc2_error_success as icsneoc2_error_t {
            return Err(Self::error_code_get(res));
        }
        // Convert the vec to an String
        str.resize(str_length as usize, 0);
        let str_string = {
            CString::new(str)
                .expect("CString::new failed")
                .into_string()
                .expect("CString::new::into_string")
        };
        Ok(str_string)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test_device_find_all() {
        let devices = Device::find_all(0).unwrap();
        assert_eq!(devices.len(), 0);
    }
}
