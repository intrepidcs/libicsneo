const std = @import("std");
const print = std.debug.print;

const ics = @cImport({
    @cInclude("icsneo/icsneoc2.h");
    @cInclude("icsneo/icsneoc2types.h");
});

const c = @cImport({
    @cInclude("time.h");
});

pub fn main() !void {
    // Find devices connected to host.
    const MAX_DEVICE_COUNT: u32 = 255;
    var device_buffer: [MAX_DEVICE_COUNT]?*ics.icsneoc2_device_t = undefined;
    var devices_count: u32 = MAX_DEVICE_COUNT;
    print("Finding devices... ", .{});
    if (!check_error(
        ics.icsneoc2_device_find_all(&device_buffer, &devices_count, null),
        "Failed to find device",
    )) {
        return;
    }
    print("OK, {d} device{s} found\n", .{ devices_count, if (devices_count == 1) "" else "s" });
    // Lets just take a slice of the entire device buffer
    const devices = device_buffer[0..devices_count];
    // List off the devices
    for (devices) |device| {
        // Get description of the device
        var description: [255]u8 = [_:0]u8{0} ** 255;
        var description_length: u32 = 255;
        if (!check_error(
            ics.icsneoc2_device_description_get(device, &description, &description_length),
            "\tFailed to get device description",
        )) {
            return;
        }
        print("{s}. {*}\n", .{ description, device.? });
        // Get/Set open options
        var options: ics.icsneoc2_open_options_t = ics.icsneoc2_open_options_none;
        if (!check_error(
            ics.icsneoc2_device_open_options_get(device, &options),
            "\tFailed to get device open options",
        )) {
            return;
        }
        // Disable Syncing RTC and going online
        options &= ~@as(ics.icsneoc2_open_options_t, ics.icsneoc2_open_options_sync_rtc);
        options &= ~@as(ics.icsneoc2_open_options_t, ics.icsneoc2_open_options_go_online);
        print("\tDevice open options: 0x{X}\n", .{options});
        if (!check_error(
            ics.icsneoc2_device_open_options_set(device, options),
            "\tFailed to set device open options",
        )) {
            return;
        }
        // Open the device
        print("\tOpening device: {s}...\n", .{description});
        if (!check_error(
            ics.icsneoc2_device_open(device),
            "\tFailed to open device",
        )) {
            return;
        }
        defer {
            // Finally, close the device.
            if (!print_device_events(device)) {
                print("\tFailed to print events...\n", .{});
            }
            print("\tClosing device: {s}... ", .{description});
            if (check_error(
                ics.icsneoc2_device_close(device),
                "\tFailed to close device",
            )) {
                print("OK\n", .{});
            }
        }
        // Get timestamp resolution of the device
        print("\tGetting timestamp resolution... ", .{});
        var timestamp_resolution: u32 = 0;
        if (!check_error(
            ics.icsneoc2_device_timestamp_resolution_get(device, &timestamp_resolution),
            "\tFailed to get timestamp resolution",
        )) {
            return;
        }
        print("{d}ns\n", .{timestamp_resolution});
        // Get baudrates for HSCAN
        print("\tGetting HSCAN baudrate... ", .{});
        var baudrate: u64 = 0;
        if (!check_error(
            ics.icsneoc2_device_baudrate_get(device, ics.icsneoc2_netid_hscan, &baudrate),
            "\tFailed to get baudrate",
        )) {
            return;
        }
        print("{d}mbit/s\n", .{baudrate});
        // Get FDbaudrates for HSCAN
        print("\tGetting FD HSCAN baudrate... ", .{});
        var fd_baudrate: u64 = 0;
        if (!check_error(
            ics.icsneoc2_device_canfd_baudrate_get(device, ics.icsneoc2_netid_hscan, &fd_baudrate),
            "\tFailed to get FD baudrate",
        )) {
            return;
        }
        print("{d}mbit/s\n", .{fd_baudrate});
        // Set baudrates for HSCAN
        // save_to_device: If this is set to true, the baudrate will be saved on the device
        // and will persist through a power cycle
        print("\tSetting HSCAN Baudrate... ", .{});
        const save_to_device: bool = false;
        if (!check_error(
            ics.icsneoc2_device_baudrate_set(device, ics.icsneoc2_netid_hscan, baudrate, save_to_device),
            "\tFailed to set baudrate",
        )) {
            return;
        }
        print("OK\n", .{});
        // Set FDbaudrates for HSCAN
        print("\tSetting FD HSCAN Baudrate... ", .{});
        if (!check_error(
            ics.icsneoc2_device_canfd_baudrate_set(device, ics.icsneoc2_netid_hscan, baudrate, save_to_device),
            "\tFailed to set FD baudrate",
        )) {
            return;
        }
        print("OK\n", .{});
        // Get RTC
        print("\tGetting RTC... ", .{});
        var unix_epoch: c.time_t = 0;
        if (!check_error(
            ics.icsneoc2_device_rtc_get(device, &unix_epoch),
            "\tFailed to get RTC",
        )) {
            return;
        }
        print_rtc(unix_epoch);
        // Set RTC
        print("\tSetting RTC to current time... ", .{});
        const current_time: i64 = c.time(0);
        if (!check_error(
            ics.icsneoc2_device_rtc_set(device, current_time),
            "\tFailed to set RTC",
        )) {
            return;
        }
        print("OK\n", .{});
        // Get RTC
        print("\tGetting RTC... ", .{});
        if (!check_error(
            ics.icsneoc2_device_rtc_get(device, &unix_epoch),
            "\tFailed to get RTC",
        )) {
            return;
        }
        print_rtc(unix_epoch);
        // Go online, start acking traffic
        print("\tGoing online... ", .{});
        if (!check_error(
            ics.icsneoc2_device_go_online(device, true),
            "\tFailed to go online",
        )) {
            return;
        }
        // Redundant check to show how to check if the device is online, if the previous
        // icsneoc2_device_go_online call was successful we can assume we are online already
        var is_online: bool = false;
        if (!check_error(
            ics.icsneoc2_device_is_online(device, &is_online),
            "\tFailed to check if online",
        )) {
            return;
        }
        print("{s}\n", .{if (is_online) "Online" else "Offline"});
        // Transmit CAN messages
        if (!transmit_can_messages(device)) {
            return;
        }
        // Wait for the bus to collect some messages, requires an active bus to get messages
        print("\tWaiting 1 second for messages...\n", .{});
        std.time.sleep(std.time.ns_per_s);
        // Get the messages
        var messages: [20000]?*ics.icsneoc2_message_t = [_]?*ics.icsneoc2_message_t{null} ** 20000;
        var messages_count: u32 = 20000;
        const res = ics.icsneoc2_device_messages_get(device, &messages, &messages_count, 3000);
        if (!check_error(
            res,
            "\tFailed to get messages on device",
        )) {
            return;
        }
        // Process the messages
        if (!process_messages(device, messages[0..messages_count])) {
            return;
        }
    }
}

pub fn check_error(error_code: ics.icsneoc2_error_t, error_msg: []const u8) bool {
    if (error_code == ics.icsneoc2_error_success) {
        return true;
    }
    var error_str: [256]u8 = [_:0]u8{0} ** 256;
    var error_length: u32 = 256;
    const res: ics.icsneoc2_error_t = ics.icsneoc2_error_code_get(
        error_code,
        &error_str,
        &error_length,
    );
    if (res != ics.icsneoc2_error_success) {
        print(
            "{s}: Failed to get string for error code {d} with error code {d}\n",
            .{ error_msg, error_code, res },
        );
        return false;
    }
    print(
        "{s}: \"{s}\" ({d})\n",
        .{ error_msg, error_str, error_code },
    );
    return error_code == ics.icsneoc2_error_success;
}

pub fn print_rtc(unix_epoch: c.time_t) void {
    var rtc_time: [32]u8 = [_:0]u8{0} ** 32;
    _ = c.strftime(&rtc_time, rtc_time.len, "%Y-%m-%d %H:%M:%S", c.localtime(&unix_epoch));
    print("{d} {s}\n", .{ unix_epoch, rtc_time });
}

pub fn transmit_can_messages(device: ?*ics.icsneoc2_device_t) bool {
    const msg_count: usize = 100;
    print("\tTransmitting {} messages...\n", .{msg_count});
    for (0..msg_count) |counter| {
        // Create the message
        var message: ?*ics.icsneoc2_message_t = null;
        var message_count: u32 = 1;
        if (!check_error(
            ics.icsneoc2_message_can_create(device, &message, message_count),
            "\tFailed to create CAN message",
        )) {
            return false;
        }
        defer {
            _ = check_error(
                ics.icsneoc2_message_can_free(device, message),
                "\tFailed to free CAN message",
            );
        }
        // Set the message attributes
        var res: ics.icsneoc2_error_t = ics.icsneoc2_message_netid_set(device, message, ics.icsneoc2_netid_hscan);
        res += ics.icsneoc2_message_can_arbid_set(device, message, 0x10);
        res += ics.icsneoc2_message_can_canfd_set(device, message, true);
        res += ics.icsneoc2_message_can_extended_set(device, message, true);
        res += ics.icsneoc2_message_can_baudrate_switch_set(device, message, true);
        // Create the payload
        var data: [8]u8 = .{
            @intCast(counter >> 56),
            @intCast(counter >> 48),
            @intCast(counter >> 40),
            @intCast(counter >> 32),
            @intCast(counter >> 24),
            @intCast(counter >> 16),
            @intCast(counter >> 8),
            @intCast(counter >> 0),
        };
        res += ics.icsneoc2_message_data_set(device, message, &data, data.len);
        res += ics.icsneoc2_message_can_dlc_set(device, message, -1);
        if (!check_error(res, "\tFailed to set CAN Message attributes!")) {
            return false;
        }

        if (!check_error(
            ics.icsneoc2_device_messages_transmit(device, &message, &message_count),
            "\tFailed to transmit message",
        )) {
            return false;
        }
    }
    return true;
}

pub fn process_messages(device: ?*ics.icsneoc2_device_t, messages: []const ?*ics.icsneoc2_message_t) bool {
    var tx_count: usize = 0;
    for (messages, 0..) |message, i| {
        // Get the message type
        var msg_type: ics.icsneoc2_msg_type_t = 0;
        if (!check_error(
            ics.icsneoc2_message_type_get(device, message.?, &msg_type),
            "\tFailed to get message type",
        )) {
            return false;
        }
        // Get the message type name
        var msg_type_name: [128]u8 = [_:0]u8{0} ** 128;
        var msg_type_name_length: u32 = 128;
        if (!check_error(
            ics.icsneoc2_message_type_name_get(msg_type, &msg_type_name, &msg_type_name_length),
            "\tFailed to get message type name",
        )) {
            return false;
        }
        // Check if the message is a bus message, ignore otherwise
        if (msg_type != ics.icsneoc2_msg_type_bus) {
            print("\tIgnoring message type: {d} ({s})\n", .{ msg_type, msg_type_name });
            continue;
        }
        // Get the message bus type
        var msg_bus_type: ics.icsneoc2_msg_bus_type_t = 0;
        if (!check_error(
            ics.icsneoc2_message_bus_type_get(device, message, &msg_bus_type),
            "\tFailed to get message bus type",
        )) {
            return false;
        }
        // Get the message type name
        var msg_bus_type_name: [128]u8 = [_:0]u8{0} ** 128;
        var msg_bus_type_name_length: u32 = 128;
        if (!check_error(
            ics.icsneoc2_bus_type_name_get(msg_bus_type, &msg_bus_type_name, &msg_bus_type_name_length),
            "\tFailed to get message bus type name",
        )) {
            return false;
        }
        // Check if message is a transmit message
        var is_tx: bool = false;
        if (!check_error(
            ics.icsneoc2_message_is_transmit(device, message, &is_tx),
            "\tFailed to get message is transmit",
        )) {
            return false;
        }
        if (is_tx) {
            tx_count += 1;
            continue;
        }
        print("\t{d} Message type: {d} bus type: {s} ({d})\n", .{ i, msg_type, msg_bus_type_name, msg_bus_type });
        // Check if the message is a CAN message, ignore otherwise
        if (msg_bus_type == ics.icsneoc2_msg_bus_type_can) {
            var arbid: u32 = 0;
            var dlc: i32 = 0;
            var netid: ics.icsneoc2_netid_t = 0;
            var is_remote: bool = false;
            var is_canfd: bool = false;
            var is_extended: bool = false;
            var data: [64]u8 = [_]u8{0} ** 64;
            var data_length: u32 = 64;
            var netid_name: [128]u8 = [_:0]u8{0} ** 128;
            var netid_name_length: u32 = 128;
            var res: ics.icsneoc2_error_t = ics.icsneoc2_error_success;
            res = ics.icsneoc2_message_netid_get(device, message, &netid);
            res += ics.icsneoc2_netid_name_get(netid, &netid_name, &netid_name_length);
            res += ics.icsneoc2_message_can_arbid_get(device, message, &arbid);
            res += ics.icsneoc2_message_can_dlc_get(device, message, &dlc);
            res += ics.icsneoc2_message_can_is_remote(device, message, &is_remote);
            res += ics.icsneoc2_message_can_is_canfd(device, message, &is_canfd);
            res += ics.icsneoc2_message_can_is_extended(device, message, &is_extended);
            res += ics.icsneoc2_message_data_get(device, message, &data, &data_length);
            // We really should check the error message for all of these since we can't tell the exact error if something
            // bad happens but for an example this should be okay.
            if (res != ics.icsneoc2_error_success) {
                print("\tFailed to get CAN parameters (error: {d}) for index {d}\n", .{ res, i });
                continue;
            }
            // Finally lets print the RX message
            print("\t  NetID: {s} (0x{X})\tArbID: 0x{X}\t DLC: {d}\t Remote: {}\t CANFD: {}\t Extended: {}\t Data length: {d}\n", .{ netid_name, netid, arbid, dlc, is_remote, is_canfd, is_extended, data_length });
            print("\t  Data: {any}\n", .{data[0..data_length]});
        } else {
            print("\tIgnoring bus message type: {d} ({s})\n", .{ msg_bus_type, msg_bus_type_name });
            continue;
        }
    }
    print("\tReceived {d} messages total, {d} were TX messages\n", .{ messages.len, tx_count });
    return true;
}

pub fn print_device_events(device: ?*ics.icsneoc2_device_t) bool {
    // Get device events
    var events: [1024]?*ics.icsneoc2_event_t = [_]?*ics.icsneoc2_event_t{null} ** 1024;
    var events_count: u32 = 1024;
    if (!check_error(
        ics.icsneoc2_device_events_get(device, &events, &events_count),
        "\tFailed to get device events",
    )) {
        return false;
    }
    for (events[0..events_count], 0..) |event, i| {
        var event_description: [256]u8 = [_:0]u8{0} ** 256;
        var event_description_length: u32 = 256;
        if (!check_error(
            ics.icsneoc2_event_description_get(event, &event_description, &event_description_length),
            "\tFailed to get event description",
        )) {
            continue;
        }
        print("\tEvent {d}: {s}\n", .{ i, event_description });
    }
    // Get global events
    var global_events: [1024]?*ics.icsneoc2_event_t = [_]?*ics.icsneoc2_event_t{null} ** 1024;
    var global_events_count: u32 = 1024;
    if (!check_error(
        ics.icsneoc2_events_get(&global_events, &global_events_count),
        "\tFailed to get device global events",
    )) {
        return false;
    }
    for (global_events[0..global_events_count], 0..) |event, i| {
        var event_description: [256]u8 = [_:0]u8{0} ** 256;
        var event_description_length: u32 = 256;
        if (!check_error(
            ics.icsneoc2_event_description_get(event, &event_description, &event_description_length),
            "\tFailed to get event description",
        )) {
            continue;
        }
        print("\tGlobal event {d}: {s}\n", .{ i, event_description });
    }
    print("\tReceived {d} events and {d} global events\n", .{ events_count, global_events_count });
    return true;
}
