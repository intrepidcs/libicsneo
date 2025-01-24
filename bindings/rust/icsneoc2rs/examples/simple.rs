use icsneoc2rs::ffi::_icsneoc2_msg_bus_type_t::*;
use icsneoc2rs::ffi::_icsneoc2_msg_type_t::*;
use icsneoc2rs::ffi::_icsneoc2_netid_t;
use icsneoc2rs::ffi::_icsneoc2_netid_t::*;
use icsneoc2rs::ffi::_icsneoc2_open_options_t::*;
use icsneoc2rs::ffi::icsneoc2_message_t;
use icsneoc2rs::ffi::icsneoc2_msg_bus_type_t;
use icsneoc2rs::ffi::icsneoc2_msg_type_t;
use icsneoc2rs::ffi::icsneoc2_netid_t;
use icsneoc2rs::Device;
use icsneoc2rs::Error;
use icsneoc2rs::Result as ICSNeoResult;

use scopeguard::defer;

fn main() -> ICSNeoResult<()> {
    print!("Finding devices... ");
    let devices = Device::find_all(255)?;
    println!(
        "OK, {} device{} found...",
        devices.len(),
        if devices.len() > 1 { "s" } else { "" }
    );
    for device in devices {
        device.device_is_valid()?;
        // Get description of the device
        let description = match device.device_description_get() {
            Ok(description) => description,
            Err(err) => {
                println!("Failed to get description of device: {err}");
                continue;
            }
        };
        println!("{} @ Handle {:?}", description, device.handle);
        // Get/Set open options
        let mut open_options = match device.device_open_options_get() {
            Ok(value) => value,
            Err(err) => {
                println!("Failed to get open options of device: {err}");
                continue;
            }
        };
        // Disable Syncing RTC and going online
        open_options &= !(icsneoc2_open_options_sync_rtc as u32);
        open_options &= !(icsneoc2_open_options_go_online as u32);
        match device.device_open_options_set(open_options) {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("\tDevice open options: {open_options:#02x}");
        // Open the device
        print!("\tOpening device... ");
        match device.device_open() {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("OK");
        defer! {
            print_device_events(&device, &description).expect("Critical: Failed to print device events");
        }
        // Get timestamp resolution of the device
        print!("\tGetting timestamp resolution... ");
        let timestamp_resolution = match device.device_timestamp_resolution_get() {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("{timestamp_resolution}ns");
        // Get baudrates for HSCAN
        print!("\tGetting HSCAN baudrate... ");
        let baudrate = match device.device_baudrate_get(icsneoc2_netid_hscan) {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("{baudrate}mbit/s");
        // Get FD baudrates for HSCAN
        print!("\tGetting FD HSCAN baudrate... ");
        let fd_baudrate = match device.device_canfd_baudrate_get(icsneoc2_netid_hscan) {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("{fd_baudrate}mbit/s");
        // Set baudrates for HSCAN
        // saveToDevice: If this is set to true, the baudrate will be saved on the device
        // and will persist through a power cycle
        print!("\tSetting HSCAN baudrate... ");
        let save_to_device = false;
        match device.device_baudrate_set(icsneoc2_netid_hscan, baudrate, save_to_device) {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("OK");
        // Set FD baudrates for HSCAN
        print!("\tSetting FD HSCAN baudrate... ");
        match device.device_canfd_baudrate_set(icsneoc2_netid_hscan, fd_baudrate, save_to_device) {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("OK");
        // Get RTC
        print!("\tGetting RTC... ");
        let current_rtc = match device.device_rtc_get() {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("{current_rtc}");
        // Set RTC
        print!("\tSetting RTC... ");
        match device.device_rtc_set(current_rtc) {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("OK");
        // Get RTC
        print!("\tGetting RTC... ");
        let rtc = match device.device_rtc_get() {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("{rtc}");
        // Go online, start acking traffic
        print!("\tGoing online... ");
        match device.device_go_online(true) {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        // Redundant check to show how to check if the device is online, if the previous
        // icsneoc2_device_go_online call was successful we can assume we are online already
        let is_online = match device.device_is_online() {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("{}", if is_online { "Online" } else { "Offline" });
        // Transmit CAN messages
        if transmit_can_messages(&device).is_err() {
            continue;
        }
        println!("\tWaiting 1 second for messages...");
        std::thread::sleep(std::time::Duration::from_secs(1));
        // Get the messages
        let messages = match device.device_messages_get(20000, 3000) {
            Ok(messages) => messages,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        // Process the messages
        if process_messages(&device, messages).is_err() {
            continue;
        }
        // Close the device
        print!("\tClosing device... ");
        match device.device_close() {
            Ok(value) => value,
            Err(err) => {
                print_error(&device, &description, err);
                continue;
            }
        };
        println!("OK");
    }
    Ok(())
}

fn print_error(device: &Device, device_description: &String, err: Error) {
    println!("Failed: {err}");
    print_device_events(device, device_description)
        .expect("Critical: Failed to print device events");
}

fn transmit_can_messages(device: &Device) -> ICSNeoResult<()> {
    const MESSAGE_COUNT: usize = 100;
    println!("\tTransmitting {MESSAGE_COUNT} messages...");
    for i in 0..MESSAGE_COUNT {
        let message = match device.message_can_create(1) {
            Ok(messages) => messages[0],
            Err(err) => {
                println!("Failed to create CAN message #{i}: {err}");
                continue;
            }
        };
        device.message_netid_set(message, icsneoc2_netid_hscan)?;
        device.message_can_arbid_set(message, 0x10)?;
        device.message_can_canfd_set(message, true)?;
        device.message_can_extended_set(message, true)?;
        device.message_can_baudrate_switch_set(message, true)?;
        device.message_data_set(
            message,
            vec![
                (i >> 56 & 0xFF) as u8,
                (i >> 48 & 0xFF) as u8,
                (i >> 40 & 0xFF) as u8,
                (i >> 32 & 0xFF) as u8,
                (i >> 24 & 0xFF) as u8,
                (i >> 16 & 0xFF) as u8,
                (i >> 8 & 0xFF) as u8,
                (i & 0xFF) as u8,
            ],
        )?;
        device.message_can_dlc_set(message, -1)?;
        device.device_messages_transmit(vec![message])?;
    }
    Ok(())
}

fn process_messages(device: &Device, messages: Vec<*mut icsneoc2_message_t>) -> ICSNeoResult<()> {
    let mut tx_count: u32 = 0;
    for (i, message) in messages.iter().enumerate() {
        // Get the message type
        let msg_type = device.message_type_get(*message)?;
        // Get the message type name
        let msg_type_name = device.message_type_name_get(msg_type)?;
        // Check if the message is a bus message, ignore otherwise
        if msg_type != icsneoc2_msg_type_bus as icsneoc2_msg_type_t {
            println!("\tIgnoring message type : {msg_type} ({msg_type_name})");
            continue;
        }
        // Get the message bus type
        let msg_bus_type = device.message_bus_type_get(*message)?;
        // Get the message type name
        let msg_bus_type_name = device.bus_type_name_get(msg_bus_type)?;
        // Check if message is a transmit message
        if device.message_is_transmit(*message)? {
            tx_count += 1;
            continue;
        }
        println!("\t{i} Message type: {msg_type} bus type: {msg_bus_type_name} ({msg_bus_type})\n");
        // Check if the message is a CAN message, ignore otherwise
        if msg_bus_type == icsneoc2_msg_bus_type_can as icsneoc2_msg_bus_type_t {
            let netid = device.message_netid_get(*message)?;
            let netid_name = device.netid_name_get(netid)?;
            let arbid = device.message_can_arbid_get(*message)?;
            let dlc = device.message_can_dlc_get(*message)?;
            let is_remote = device.message_can_is_remote(*message)?;
            let is_canfd = device.message_can_is_canfd(*message)?;
            let is_extended = device.message_can_is_extended(*message)?;
            let data = device.message_data_get(*message, 64)?;
            // Finally lets print the RX message
            println!("\t  NetID: {netid_name} ({:#02x})\tArbID: {arbid:#02x}\t DLC: {dlc}\t Remote: {is_remote}\t \
                CANFD: {is_canfd}\t Extended: {is_extended}\t Data length: {}\n", netid as icsneoc2_netid_t,data.len());
            println!("\t  Data: {data:#02x?}\n");
        } else {
            println!("\tIgnoring bus message type: {msg_bus_type} ({msg_bus_type_name})\n");
        }
    }
    println!(
        "\tReceived {} messages total, {} were TX messages",
        messages.len(),
        tx_count
    );
    Ok(())
}

fn print_device_events(device: &Device, device_description: &String) -> ICSNeoResult<()> {
    let events = match device.device_events_get(1024) {
        Ok(events) => events,
        Err(err) => {
            println!("Failed to get device events: {err}");
            return Err(err);
        }
    };
    for (i, event) in events.iter().enumerate() {
        let description = match device.event_description_get(*event) {
            Ok(description) => description,
            Err(err) => {
                println!("Failed to get event description: {err}");
                continue;
            }
        };
        println!("\t{device_description}: Event {i}: {description}");
    }
    // Get global events
    let global_events = match device.events_get(1024) {
        Ok(events) => events,
        Err(err) => {
            println!("Failed to get global events: {err}");
            return Err(err);
        }
    };
    for (i, event) in global_events.iter().enumerate() {
        let description = match device.event_description_get(*event) {
            Ok(description) => description,
            Err(err) => {
                println!("Failed to get event description: {err}");
                continue;
            }
        };
        println!("\t{device_description}: Global Event {i}: {description}");
    }
    println!(
        "\t{device_description}: Received {} events and {} global events\n",
        events.len(),
        global_events.len()
    );

    Ok(())
}
