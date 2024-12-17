package main

// #cgo CFLAGS: -I../../../../include
// #cgo LDFLAGS: -L../../../../build -licsneo -lstdc++
// #include "icsneo/icsneo.h"
import "C"
import (
	"fmt"
	"unsafe"
)

func main() {
	findDevices()
}

func findDevices() {
	// Get all devices attached to host
	devices := [255]*C.icsneo_device_t{nil}
	devicesCount := 255
	res := C.icsneo_device_find_all(&devices[0], (*C.uint)(unsafe.Pointer(&devicesCount)), nil)
	if res != C.icsneo_error_success {
		printError(res)
		return
	}
	println("Found", devicesCount, "device(s):")
	// Iterate through all the devices
	for i, device := range devices[:devicesCount] {
		description := make([]byte, 255)
		descriptionLength := 255
		res := C.icsneo_device_get_description(device, (*C.char)(unsafe.Pointer(&description[0])), (*C.uint)(unsafe.Pointer(&descriptionLength)))
		if res != C.icsneo_error_success {
			printError(res)
			continue
		}
		fmt.Printf("\t%d. %s\n", i+1, string(description[:descriptionLength]))
	}
}

func printError(err C.icsneo_error_t) C.icsneo_error_t {
	buffer := make([]byte, 255)
	bufferLength := 255
	res := C.icsneo_get_error_code(err, (*C.char)(unsafe.Pointer(&buffer[0])), (*C.uint)(unsafe.Pointer(&bufferLength)))
	if res != C.icsneo_error_success {
		println("icsneo_get_error_code failed, original error:", err)
		return res
	}
	println("Error:", string(buffer[:bufferLength]))
	return res
}
