// Suppress the flurry of warnings caused by using "C" naming conventions
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

pub unsafe fn find_devices() {
    let mut devices: [*mut icsneo_device_t; 255] = [std::ptr::null_mut(); 255];
    let mut device_count: u32 = 255;
    let res = icsneo_device_find_all(devices.as_mut_ptr(), &mut device_count as *mut u32, std::ptr::null_mut());

    if res.0 != _icsneo_error_t::icsneo_error_success as u32 {
        return;
    }
    println!("Found {} devices", device_count);
    for i in 0..device_count as usize {
        let device = devices[i];
        let res = icsneo_device_is_valid(device);
        if res.0 != _icsneo_error_t::icsneo_error_success as u32 {
            return;
        }

        let mut description_length: u32 = 255;
        let mut description: [::core::ffi::c_char; 255] = [0i8; 255];
        let res = icsneo_device_get_description(device, description.as_mut_ptr(), &mut description_length);
        if res.0 != _icsneo_error_t::icsneo_error_success as u32 {
            return;
        }
        let description_str = std::ffi::CStr::from_ptr(description.as_ptr()).to_str().unwrap().to_string();
        println!("\t{}: {}", i, description_str);
    }
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        unsafe { find_devices(); }
    }
}
