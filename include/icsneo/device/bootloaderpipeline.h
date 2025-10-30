#ifndef __BOOTLOADER_PIPELINE_H_
#define __BOOTLOADER_PIPELINE_H_

#ifdef __cplusplus

#include "icsneo/device/chipid.h"

#include <vector>
#include <memory>
#include <variant>
#include <utility>

namespace icsneo {

enum class BootloaderCommunication {
	RAD,
	RED,
	REDCore,
	RADGalaxy2Peripheral,
	RADMultiChip,
	Application,
	Invalid
};

struct BootloaderPhase {
	enum class Type {
		Flash,
		Finalize,
		Reconnect,
		EnterBootloader,
		Wait,
		EnterApplication
	};

	virtual Type getType() const = 0;
	virtual ~BootloaderPhase() = default;
};

struct ReconnectPhase : public BootloaderPhase {
	Type getType() const override {
		return Type::Reconnect;
	}

	ReconnectPhase() = default;
};

struct FinalizePhase : public BootloaderPhase {
	Type getType() const override {
		return Type::Finalize;
	}
	
	ChipID chip;
	BootloaderCommunication comm;

	FinalizePhase(ChipID chip, BootloaderCommunication comm) : chip(chip), comm(comm) {}
};

struct WaitPhase : public BootloaderPhase {
	Type getType() const override {
		return Type::Wait;
	}

	std::chrono::milliseconds timeout;

	WaitPhase(std::chrono::milliseconds timeout) : timeout(timeout) {}
};

struct EnterBootloaderPhase : public BootloaderPhase {
	Type getType() const override {
		return Type::EnterBootloader;
	}
};

struct EnterApplicationPhase : public BootloaderPhase {
	Type getType() const override {
		return Type::EnterApplication;
	}

	ChipID chip;

	EnterApplicationPhase(ChipID chip) : chip(chip) {}
};

struct FlashPhase : public BootloaderPhase {
	Type getType() const override {
		return Type::Flash;
	}

	ChipID chip;
	BootloaderCommunication comm;
	bool authenticate = true;
	bool encrypt = true;
	bool checkOutOfDate = true;

	FlashPhase(ChipID chip, BootloaderCommunication comm, bool authenticate = true, bool encrypt = true, bool checkOutOfDate = true) 
		: chip(chip), comm(comm), authenticate(authenticate), encrypt(encrypt), checkOutOfDate(checkOutOfDate) {}
};

enum class BootloaderSetting {
	UpdateAll
};

struct BootloaderPipeline {
	std::vector<std::shared_ptr<BootloaderPhase>> phases;
	std::unordered_map<BootloaderSetting, std::variant<int, bool>> settings;

	template<typename Phase, typename... Args>
	BootloaderPipeline& add(Args... args) {
		phases.emplace_back(std::make_shared<Phase>(std::forward<Args>(args)...));
		return *this;
	}

	BootloaderPipeline& addSetting(BootloaderSetting type, const std::variant<int, bool>& setting) {
		settings.insert({type, setting});
		return *this;
	}

	operator bool() const {
		return !phases.empty();
	}
	
};

}

#endif // __cplusplus

#endif