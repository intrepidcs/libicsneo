const std = @import("std");
const print = @import("std").debug.print;

const ics = @cImport({
    @cInclude("icsneo/icsneo.h");
});

pub fn find_all() !void {
    // Find devices connected to host.
    const MAX_DEVICE_COUNT: u32 = 255;
    var device_buffer: [MAX_DEVICE_COUNT]?*ics.icsneo_device_t = undefined;
    var devices_count: u32 = MAX_DEVICE_COUNT;
    var res: ics.icsneo_error_t = ics.icsneo_device_find_all(&device_buffer, &devices_count, null);
    if (res != ics.icsneo_error_success) {
        print("Failed to find device(s) with error: {}\n", .{res});
    }
    // Lets just take a slice of the entire device buffer
    const devices = device_buffer[0..devices_count];
    print("Found {} devices!\n", .{devices.len});
    // Lets display all the devices we found.
    for (devices, 1..) |device, i| {
        var description: [128]u8 = [_:0]u8{'a'} ** 128;
        var description_length: u32 = 128;
        res = ics.icsneo_device_get_description(device, &description, &description_length);
        if (res != ics.icsneo_error_success) {
            print("Failed to find devices with error: {}\n", .{res});
        }
        print("\t{}. {s}\n", .{ i, description });
    }
}

pub fn main() !void {
    try find_all();
}
