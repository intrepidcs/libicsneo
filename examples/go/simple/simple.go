package main

// #cgo CFLAGS: -I../../../include
// #cgo LDFLAGS: -L../../../build -licsneoc2
// #include "icsneo/icsneoc2.h"
// #include "stdint.h"
import "C"
import (
	"fmt"
	"time"
	"unsafe"
)

func main() {
	// Find devices connected to host.
	devices := [255]*C.icsneoc2_device_t{nil}
	devicesCount := 255
	print("Finding devices... ")
	if res := C.icsneoc2_device_find_all(&devices[0], (*C.uint)(unsafe.Pointer(&devicesCount)), nil); res != C.icsneoc2_error_success {
		printError(res)
		return
	}
	fmt.Printf("OK, %d device(s) found\n", devicesCount)
	// List off the devices
	for _, device := range devices[:devicesCount] {
		// Get description of the device
		description := make([]byte, 255)
		descriptionLength := 255
		if res := C.icsneoc2_device_description_get(device, (*C.char)(unsafe.Pointer(&description[0])), (*C.uint)(unsafe.Pointer(&descriptionLength))); res != C.icsneoc2_error_success {
			printError(res)
			continue
		}
		fmt.Printf("%s @ Handle %p\n", description, device)
		// Get/Set open options
		options := C.icsneoc2_open_options_none
		if res := C.icsneoc2_device_open_options_get(device, (*C.icsneoc2_open_options_t)(unsafe.Pointer(&options))); res != C.icsneoc2_error_success {
			printError(res)
			continue
		}
		options &= ^C.icsneoc2_open_options_sync_rtc
		options &= ^C.icsneoc2_open_options_go_online
		fmt.Printf("\tDevice open options: 0x%X\n", options)
		if res := C.icsneoc2_device_open_options_set(device, (C.icsneoc2_open_options_t)(options)); res != C.icsneoc2_error_success {
			printError(res)
			continue
		}
		// Open the device
		fmt.Printf("\tOpening device: %s...\n", description)
		if res := C.icsneoc2_device_open(device); res != C.icsneoc2_error_success {
			printError(res)
			continue
		}
		defer func() {
			if !printDeviceEvents(device, string(description)) {
				println("\tFailed to print events...")
			}
			fmt.Printf("\tClosing device: %s...\n", description)
			if res := C.icsneoc2_device_close(device); res != C.icsneoc2_error_success {
				printError(res)
				return
			}
		}()
		// Get timestamp resolution of the device
		fmt.Printf("\tGetting timestamp resolution... ")
		var timestampResolution C.uint = 0
		if res := C.icsneoc2_device_timestamp_resolution_get(device, &timestampResolution); res != C.icsneoc2_error_success {
			printError(res)
			return
		}
		fmt.Printf("%dns\n", timestampResolution)
		// Get baudrates for HSCAN
		fmt.Printf("\tGetting HSCAN Baudrate... ")
		var baudrate uint64 = 0
		if res := C.icsneoc2_device_baudrate_get(device, (C.icsneoc2_netid_t)(C.icsneoc2_netid_hscan), (*C.uint64_t)(unsafe.Pointer(&baudrate))); res != C.icsneoc2_error_success {
			printError(res)
			return
		}
		fmt.Printf("%dmbit/s\n", baudrate)
		// Get FD baudrates for HSCAN
		fmt.Printf("\tGetting FD HSCAN Baudrate... ")
		var fdBaudrate uint64 = 0
		if res := C.icsneoc2_device_canfd_baudrate_get(device, (C.icsneoc2_netid_t)(C.icsneoc2_netid_hscan), (*C.uint64_t)(unsafe.Pointer(&fdBaudrate))); res != C.icsneoc2_error_success {
			printError(res)
			return
		}
		fmt.Printf("%dmbit/s\n", fdBaudrate)
		// Set baudrates for HSCAN
		// saveToDevice: If this is set to true, the baudrate will be saved on the device
		// and will persist through a power cycle
		var saveToDevice C.bool = false
		fmt.Printf("\tSetting HSCAN Baudrate... ")
		if res := C.icsneoc2_device_baudrate_set(device, (C.icsneoc2_netid_t)(C.icsneoc2_netid_hscan), (C.uint64_t)(baudrate), saveToDevice); res != C.icsneoc2_error_success {
			printError(res)
			return
		}
		fmt.Printf("OK\n")
		// Set FD baudrates for HSCAN
		fmt.Printf("\tSetting FD HSCAN Baudrate... ")
		if res := C.icsneoc2_device_canfd_baudrate_set(device, (C.icsneoc2_netid_t)(C.icsneoc2_netid_hscan), (C.uint64_t)(fdBaudrate), saveToDevice); res != C.icsneoc2_error_success {
			printError(res)
			return
		}
		fmt.Printf("OK\n")
		// Get RTC
		fmt.Printf("\tGetting RTC... ")
		var unix_epoch C.int64_t = 0
		if res := C.icsneoc2_device_rtc_get(device, (*C.int64_t)(unsafe.Pointer(&unix_epoch))); res != C.icsneoc2_error_success {
			printError(res)
			return
		}
		currentRTC := time.Unix(int64(unix_epoch), 0)
		fmt.Printf("%d %s\n", currentRTC.Unix(), currentRTC)
		// Set RTC
		fmt.Printf("\tSetting RTC... ")
		unix_epoch = (C.int64_t)(time.Now().Unix())
		if res := C.icsneoc2_device_rtc_set(device, unix_epoch); res != C.icsneoc2_error_success {
			printError(res)
			return
		}
		fmt.Printf("OK\n")
		// Get RTC
		fmt.Printf("\tGetting RTC... ")
		if res := C.icsneoc2_device_rtc_get(device, (*C.int64_t)(unsafe.Pointer(&unix_epoch))); res != C.icsneoc2_error_success {
			printError(res)
			return
		}
		currentRTC = time.Unix(int64(unix_epoch), 0)
		fmt.Printf("%d %s\n", currentRTC.Unix(), currentRTC)
		// Go online, start acking traffic
		fmt.Printf("\tGoing online... ")
		if res := C.icsneoc2_device_go_online(device, true); res != C.icsneoc2_error_success {
			printError(res)
			return
		}
		// Redundant check to show how to check if the device is online, if the previous
		// icsneoc2_device_go_online call was successful we can assume we are online already
		var isOnline C.bool = false
		if res := C.icsneoc2_device_is_online(device, &isOnline); res != C.icsneoc2_error_success {
			printError(res)
			return
		}
		if isOnline {
			println("Online")
		} else {
			println("Offline")
		}
		// Transmit CAN messages
		if !transmitCANMessages(device) {
			return
		}
		// Wait for the bus to collect some messages, requires an active bus to get messages
		println("\tWaiting 1 second for messages...")
		time.Sleep(1 * time.Second)
		// Get the messages
		messages := [20000]*C.icsneoc2_message_t{nil}
		var messagesCount C.uint32_t = 20000
		if res := C.icsneoc2_device_messages_get(device, &messages[0], &messagesCount, 3000); res != C.icsneoc2_error_success {
			printError(res)
			return
		}
		// Process the messages
		if !processMessages(device, messages[0:messagesCount]) {
			return
		}
	}
}

func printError(err C.icsneoc2_error_t) C.icsneoc2_error_t {
	buffer := make([]byte, 255)
	bufferLength := 255
	res := C.icsneoc2_error_code_get(err, (*C.char)(unsafe.Pointer(&buffer[0])), (*C.uint)(unsafe.Pointer(&bufferLength)))
	if res != C.icsneoc2_error_success {
		println("\ticsneoc2_get_error_code failed, original error:", err)
		return res
	}
	println("\tError:", string(buffer[:bufferLength]))
	return res
}

func printDeviceEvents(device *C.icsneoc2_device_t, deviceDescription string) bool {
	// Get device events
	events := [1024]*C.icsneoc2_event_t{nil}
	var eventsCount C.uint32_t = 1024
	if res := C.icsneoc2_device_events_get(device, &events[0], &eventsCount); res != C.icsneoc2_error_success {
		printError(res)
		return false
	}
	for i, event := range events[:eventsCount] {
		eventDescription := make([]byte, 255)
		var eventDescriptionLength C.uint32_t = 255
		if res := C.icsneoc2_event_description_get(event, (*C.char)(unsafe.Pointer(&eventDescription[0])), &eventDescriptionLength); res != C.icsneoc2_error_success {
			printError(res)
			continue
		}
		fmt.Printf("\t%s: Event %d: %s\n", deviceDescription, i, eventDescription)
	}
	// Get global events
	globalEvents := [1024]*C.icsneoc2_event_t{nil}
	var globalEventsCount C.uint32_t = 1024
	if res := C.icsneoc2_events_get(&globalEvents[0], &globalEventsCount); res != C.icsneoc2_error_success {
		printError(res)
		return false
	}
	for i, event := range globalEvents[:globalEventsCount] {
		globalEventsDescription := make([]byte, 255)
		var globalEventsDescriptionLength C.uint32_t = 255
		if res := C.icsneoc2_event_description_get(event, (*C.char)(unsafe.Pointer(&globalEventsDescription[0])), &globalEventsDescriptionLength); res != C.icsneoc2_error_success {
			printError(res)
			continue
		}
		fmt.Printf("\t%s: Global Event %d: %s\n", deviceDescription, i, globalEventsDescription)
	}
	fmt.Printf("\t%s: Received %d events and %d global events\n", deviceDescription, eventsCount, globalEventsCount)

	return true
}

func transmitCANMessages(device *C.icsneoc2_device_t) bool {
	var counter uint32 = 0
	const msgCount int = 100
	fmt.Printf("\tTransmitting %d messages...\n", msgCount)
	for range msgCount {
		// Create the message
		var message *C.icsneoc2_message_t = nil
		if res := C.icsneoc2_message_can_create(device, &message, 1); res != C.icsneoc2_error_success {
			printError(res)
			return false
		}
		defer func() {
			if res := C.icsneoc2_message_can_free(device, message); res != C.icsneoc2_error_success {
				printError(res)
			}
		}()
		// Set the message attributes
		res := C.icsneoc2_message_netid_set(device, message, C.icsneoc2_netid_hscan)
		res += C.icsneoc2_message_can_arbid_set(device, message, 0x10)
		res += C.icsneoc2_message_can_canfd_set(device, message, true)
		res += C.icsneoc2_message_can_extended_set(device, message, true)
		res += C.icsneoc2_message_can_baudrate_switch_set(device, message, true)
		// Create the payload
		data := [...]C.uint8_t{
			(C.uint8_t)(counter >> 56),
			(C.uint8_t)(counter >> 48),
			(C.uint8_t)(counter >> 40),
			(C.uint8_t)(counter >> 32),
			(C.uint8_t)(counter >> 24),
			(C.uint8_t)(counter >> 16),
			(C.uint8_t)(counter >> 8),
			(C.uint8_t)(counter >> 0),
		}
		res += C.icsneoc2_message_data_set(device, message, &data[0], (C.uint32_t)(len(data)))
		res += C.icsneoc2_message_can_dlc_set(device, message, -1)
		if res != C.icsneoc2_error_success {
			fmt.Printf("\tFailed to modify message: %d\n", res)
			return false
		}
		var messageCount C.uint32_t = 1
		if res := C.icsneoc2_device_messages_transmit(device, &message, &messageCount); res != C.icsneoc2_error_success {
			printError(res)
			return false
		}
		counter += 1
	}
	return true
}

func processMessages(device *C.icsneoc2_device_t, messages []*C.icsneoc2_message_t) bool {
	txCount := 0
	for i, message := range messages {
		// Get the message type
		var msgType C.icsneoc2_msg_type_t = 0
		if res := C.icsneoc2_message_type_get(device, message, &msgType); res != C.icsneoc2_error_success {
			printError(res)
			return false
		}
		// Get the message type name
		msgTypeName := make([]byte, 128)
		var msgTypeNameLength C.uint32_t = 128
		if res := C.icsneoc2_message_type_name_get(msgType, (*C.char)(unsafe.Pointer(&msgTypeName[0])), &msgTypeNameLength); res != C.icsneoc2_error_success {
			printError(res)
			return false
		}
		// Check if the message is a bus message, ignore otherwise
		if msgType != C.icsneoc2_msg_type_bus {
			fmt.Print("\tIgnoring message type: %d (%s)\n", msgType, msgTypeName)
			continue
		}
		// Get the message bus type
		var msgBusType C.icsneoc2_msg_bus_type_t = 0
		if res := C.icsneoc2_message_bus_type_get(device, message, &msgBusType); res != C.icsneoc2_error_success {
			printError(res)
			return false
		}
		// Get the bus message type name
		msgBusTypeName := make([]byte, 128)
		var msgBusTypeNameLength C.uint32_t = 128
		if res := C.icsneoc2_bus_type_name_get(msgBusType, (*C.char)(unsafe.Pointer(&msgBusTypeName[0])), &msgBusTypeNameLength); res != C.icsneoc2_error_success {
			printError(res)
			return false
		}
		// Check if the message is a transmit message
		var isTransmit C.bool = false
		if res := C.icsneoc2_message_is_transmit(device, message, &isTransmit); res != C.icsneoc2_error_success {
			printError(res)
			return false
		}
		if isTransmit {
			txCount += 1
			continue
		}
		fmt.Printf("\t%d) Message type: %d bus type: %s (%d)\n", i, msgType, msgBusTypeName, msgBusType)
		if msgBusType == C.icsneoc2_msg_bus_type_can {
			var arbid C.uint32_t = 0
			var dlc C.int32_t = 0
			var netid C.icsneoc2_netid_t = 0
			var isRemote C.bool = false
			var isCanfd C.bool = false
			var isExtended C.bool = false
			data := make([]byte, 64)
			var dataLength C.uint32_t = 64
			netidName := make([]byte, 128)
			var netidNameLength C.uint32_t = 128
			var res C.icsneoc2_error_t = C.icsneoc2_error_success
			res = C.icsneoc2_message_netid_get(device, message, &netid)
			res += C.icsneoc2_netid_name_get(netid, (*C.char)(unsafe.Pointer(&netidName)), &netidNameLength)
			res += C.icsneoc2_message_can_arbid_get(device, message, &arbid)
			res += C.icsneoc2_message_can_dlc_get(device, message, &dlc)
			res += C.icsneoc2_message_can_is_remote(device, message, &isRemote)
			res += C.icsneoc2_message_can_is_canfd(device, message, &isCanfd)
			res += C.icsneoc2_message_can_is_extended(device, message, &isExtended)
			res += C.icsneoc2_message_data_get(device, message, (*C.uint8_t)(unsafe.Pointer(&data[0])), &dataLength)
			// We really should check the error message for all of these since we can't tell the exact error if something
			// bad happens but for an example this should be okay.
			if res != C.icsneoc2_error_success {
				fmt.Printf("\tFailed to get CAN parameters (error: %d) for index %d\n", res, i)
				continue
			}
			// Finally lets print the RX message
			fmt.Printf("\t  NetID: %s (0x%X)\tArbID: 0x%X\t DLC: %d\t Remote: %t\t CANFD: %t\t Extended: %t\t Data length: %d\n", netidName, netid, arbid, dlc, isRemote, isCanfd, isExtended, dataLength)
			fmt.Printf("\t  Data: [")
			for _, d := range data[:dataLength] {
				fmt.Printf(" 0x%X", d)
			}
			println(" ]")
			continue
		} else {
			fmt.Printf("\tIgnoring bus message type: %d (%s)\n", msgBusType, msgBusTypeName)
			continue
		}

	}
	fmt.Printf("\tReceived %d messages total, %d were TX messages\n", len(messages), txCount)
	return true
}
