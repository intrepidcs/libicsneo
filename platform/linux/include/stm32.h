#ifndef __STM32_LINUX_H_
#define __STM32_LINUX_H_

#include "communication/include/icommunication.h"
#include "device/include/neodevice.h"
#include <chrono>
#include <stdint.h>

namespace icsneo {

class STM32 : public ICommunication {
public:
	STM32(neodevice_t& forDevice) : device(forDevice) {}
	static std::vector<neodevice_t> FindByProduct(int product);

	bool open();
	bool isOpen();
	bool close();

private:
	neodevice_t& device;
	int fd = -1;
	static constexpr neodevice_handle_t HANDLE_OFFSET = 10;

	void readTask();
	void writeTask();
};

};

#endif