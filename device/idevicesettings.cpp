#include "icsneo/device/idevicesettings.h"
#include "icsneo/communication/message/filter/main51messagefilter.h"
#include <cstring>

using namespace icsneo;

optional<uint16_t> IDeviceSettings::CalculateGSChecksum(const std::vector<uint8_t>& settings, optional<size_t> knownSize) {
	const uint16_t* p = reinterpret_cast<const uint16_t*>(settings.data());
	size_t words = std::min(knownSize.value_or(0), settings.size());
	if(words % 2 == 1)
		return nullopt; // Somehow settings is not word aligned
	words /= 2;

	uint16_t gsCrc = 0;
	while(words--) {
		uint16_t temp = *p;

		for (int i = 0; i < 16; i++) {
			bool iBit = temp & 1;
			
			int iCrcNxt;
			//CRCNXT = NXTBIT EXOR CRC_RG(15);
			if (gsCrc & (1 << 15))
				iCrcNxt = iBit ^ 1;
			else
				iCrcNxt = iBit;
			iCrcNxt = iCrcNxt & 0x01;


			// CRC_RG(15:1) = CRC_RG(14:0); // shift left by
			gsCrc = gsCrc << 1;
			gsCrc = gsCrc & 0xFFFE;// clear first bit

			if (iCrcNxt)//CRC_RG(14:0) = CRC_RG(14:0) EXOR (4599hex);
				gsCrc = gsCrc ^ 0xa001;

			temp >>= 1;
		}

		p++;
	}
	return gsCrc;
}

CANBaudrate IDeviceSettings::GetEnumValueForBaudrate(int64_t baudrate) {
	switch(baudrate) {
		case 20000:
			return BPS20;
		case 33000:
			return BPS33;
		case 50000:
			return BPS50;
		case 62000:
			return BPS62;
		case 83000:
			return BPS83;
		case 100000:
			return BPS100;
		case 125000:
			return BPS125;
		case 250000:
			return BPS250;
		case 500000:
			return BPS500;
		case 800000:
			return BPS800;
		case 1000000:
			return BPS1000;
		case 666000:
			return BPS666;
		case 2000000:
			return BPS2000;
		case 4000000:
			return BPS4000;
		case 5000000:
			return CAN_BPS5000;
		case 6667000:
			return CAN_BPS6667;
		case 8000000:
			return CAN_BPS8000;
		case 10000000:
			return CAN_BPS10000;
		default:
			return (CANBaudrate)-1;
	}
}

int64_t IDeviceSettings::GetBaudrateValueForEnum(CANBaudrate enumValue) {
	switch(enumValue) {
		case BPS20:
			return 20000;
		case BPS33:
			return 33000;
		case BPS50:
			return 50000;
		case BPS62:
			return 62000;
		case BPS83:
			return 83000;
		case BPS100:
			return 100000;
		case BPS125:
			return 125000;
		case BPS250:
			return 250000;
		case BPS500:
			return 500000;
		case BPS800:
			return 800000;
		case BPS1000:
			return 1000000;
		case BPS666:
			return 666000;
		case BPS2000:
			return 2000000;
		case BPS4000:
			return 4000000;
		case CAN_BPS5000:
			return 5000000;
		case CAN_BPS6667:
			return 6667000;
		case CAN_BPS8000:
			return 8000000;
		case CAN_BPS10000:
			return 10000000;
		default:
			return -1;
	}
}

bool IDeviceSettings::refresh(bool ignoreChecksum) {
	if(disabled) {
		report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return false;
	}

	if(disableGSChecksumming)
		ignoreChecksum = true;

	std::vector<uint8_t> rxSettings;
	bool ret = com->getSettingsSync(rxSettings);
	if(!ret) {
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
		return false;
	}

	constexpr size_t GsSize = 3 * sizeof(uint16_t);
	if(rxSettings.size() < GsSize) { // We need to at least have the header of GLOBAL_SETTINGS
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
		return false;
	}

	// The length of the settings structure sent to us
	// This is the length the firmware thinks the current version of the structure is
	const size_t rxLen = rxSettings.size() - GsSize;

	const uint16_t gsVersion = rxSettings[0] | (rxSettings[1] << 8);

	// The length of the settings last saved
	// If the firmware is updated, it will have either extended (with zeros) or truncated
	// the structure, but this value will continue to be set to the last saved value
	const uint16_t gsLen = rxSettings[2] | (rxSettings[3] << 8);

	const uint16_t gsChecksum = rxSettings[4] | (rxSettings[5] << 8);
	rxSettings.erase(rxSettings.begin(), rxSettings.begin() + GsSize);

	if(gsVersion != GS_VERSION) {
		report(APIEvent::Type::SettingsVersionError, APIEvent::Severity::Error);
		return false;
	}

	if(rxLen < gsLen) {
		// We got less data, i.e. the firmware thinks the strucure is smaller than what
		// was last saved. Usually this is due to a firmware downgrade. We'll ignore the
		// checksum for now, because it will definitely be wrong.
		ignoreChecksum = true;
	}

	// We check the checksum against the data last saved
	if(!ignoreChecksum && gsChecksum != CalculateGSChecksum(rxSettings, gsLen)) {
		report(APIEvent::Type::SettingsChecksumError, APIEvent::Severity::Error);
		return false;
	}

	settings = std::move(rxSettings);
	settingsInDeviceRAM = settings;
	settingsLoaded = true;

	// TODO Warn user that their API version differs from the device firmware version
	//if(settings.size() != structSize)	

	return settingsLoaded;
}

bool IDeviceSettings::apply(bool temporary) {
	if(readonly) {
		report(APIEvent::Type::SettingsReadOnly, APIEvent::Severity::Error);
		return false;
	}

	if(disabled) {
		report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return false;
	}

	if(!settingsLoaded) {
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
		return false;
	}

	std::vector<uint8_t> bytestream;
	bytestream.resize(7 + settings.size());
	bytestream[0] = 0x00;
	bytestream[1] = GS_VERSION;
	bytestream[2] = GS_VERSION >> 8;
	bytestream[3] = (uint8_t)settings.size();
	bytestream[4] = (uint8_t)(settings.size() >> 8);
	optional<uint16_t> gsChecksum = CalculateGSChecksum(settings);
	if(!gsChecksum) {
		// Could not calculate the checksum for some reason
		report(APIEvent::Type::SettingsChecksumError, APIEvent::Severity::Error);
		return false;
	}
	bytestream[5] = (uint8_t)*gsChecksum;
	bytestream[6] = (uint8_t)(*gsChecksum >> 8);
	memcpy(bytestream.data() + 7, getMutableRawStructurePointer(), settings.size());

	// Pause I/O with the device while the settings are applied
	applyingSettings = true;
	
	std::shared_ptr<Message> msg = com->waitForMessageSync([this, &bytestream]() {
		return com->sendCommand(Command::SetSettings, bytestream);
	}, Main51MessageFilter(Command::SetSettings), std::chrono::milliseconds(1000));

	if(!msg || msg->data[0] != 1) { // We did not receive a response
		// Attempt to get the settings from the device so we're up to date if possible
		if(refresh()) {
			// refresh succeeded but previously there was an error
			report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		}
		return false;
	}

	refresh(true); // Refresh ignoring checksum
	// The device might modify the settings once they are applied, however in this case it does not update the checksum
	// We refresh to get these updates, update the checksum, and send it back so it's all in sync
	gsChecksum = CalculateGSChecksum(settings);
	if(!gsChecksum) {
		// Could not calculate the checksum for some reason
		report(APIEvent::Type::SettingsChecksumError, APIEvent::Severity::Error);
		return false;
	}
	bytestream[5] = (uint8_t)*gsChecksum;
	bytestream[6] = (uint8_t)(*gsChecksum >> 8);
	memcpy(bytestream.data() + 7, getMutableRawStructurePointer(), settings.size());

	msg = com->waitForMessageSync([this, &bytestream]() {
		return com->sendCommand(Command::SetSettings, bytestream);
	}, Main51MessageFilter(Command::SetSettings), std::chrono::milliseconds(1000));
	if(!msg || msg->data[0] != 1) {
		// Attempt to get the settings from the device so we're up to date if possible
		if(refresh()) {
			// refresh succeeded but previously there was an error
			report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		}
		return false;
	}

	if(!temporary) {
		msg = com->waitForMessageSync([this]() {
			return com->sendCommand(Command::SaveSettings);
		}, Main51MessageFilter(Command::SaveSettings), std::chrono::milliseconds(5000));
	}
	
	applyingSettings = false;

	refresh(); // Refresh our buffer with what the device has, whether we were successful or not

	bool ret = (msg && msg->data[0] == 1); // Device sends 0x01 for success
	if(!ret) {
		report(APIEvent::Type::FailedToWrite, APIEvent::Severity::Error);
	}
	return ret;
}

bool IDeviceSettings::applyDefaults(bool temporary) {
	if(disabled) {
		report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return false;
	}

	if(readonly) {
		report(APIEvent::Type::SettingsReadOnly, APIEvent::Severity::Error);
		return false;
	}

	applyingSettings = true;

	std::shared_ptr<Message> msg = com->waitForMessageSync([this]() {
		return com->sendCommand(Command::SetDefaultSettings);
	}, Main51MessageFilter(Command::SetDefaultSettings), std::chrono::milliseconds(1000));
	if(!msg || msg->data[0] != 1) {
		// Attempt to get the settings from the device so we're up to date if possible
		if(refresh()) {
			// refresh succeeded but previously there was an error
			report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		}
		return false;
	}

	// This short wait helps on FIRE devices, otherwise the checksum might be wrong!
	std::this_thread::sleep_for(std::chrono::milliseconds(3));

	refresh(true); // Refresh ignoring checksum
	// The device might modify the settings once they are applied, however in this case it does not update the checksum
	// We refresh to get these updates, update the checksum, and send it back so it's all in sync
	std::vector<uint8_t> bytestream;
	bytestream.resize(7 + settings.size());
	bytestream[0] = 0x00;
	bytestream[1] = GS_VERSION;
	bytestream[2] = GS_VERSION >> 8;
	bytestream[3] = (uint8_t)settings.size();
	bytestream[4] = (uint8_t)(settings.size() >> 8);
	const optional<uint16_t> gsChecksum = CalculateGSChecksum(settings);
	if(!gsChecksum) {
		// Could not calculate the checksum for some reason
		report(APIEvent::Type::SettingsChecksumError, APIEvent::Severity::Error);
		return false;
	}
	bytestream[5] = (uint8_t)*gsChecksum;
	bytestream[6] = (uint8_t)(*gsChecksum >> 8);
	memcpy(bytestream.data() + 7, getMutableRawStructurePointer(), settings.size());

	msg = com->waitForMessageSync([this, &bytestream]() {
		return com->sendCommand(Command::SetSettings, bytestream);
	}, Main51MessageFilter(Command::SetSettings), std::chrono::milliseconds(1000));
	if(!msg || msg->data[0] != 1) {
		// Attempt to get the settings from the device so we're up to date if possible
		if(refresh()) {
			// refresh succeeded but previously there was an error
			report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		}
		return false;
	}

	if(!temporary) {
		msg = com->waitForMessageSync([this]() {
			return com->sendCommand(Command::SaveSettings);
		}, Main51MessageFilter(Command::SaveSettings), std::chrono::milliseconds(5000));
	}

	applyingSettings = false;
	
	refresh(); // Refresh our buffer with what the device has, whether we were successful or not

	bool ret = (msg && msg->data[0] == 1); // Device sends 0x01 for success
	if(!ret) {
		report(APIEvent::Type::FailedToWrite, APIEvent::Severity::Error);
	}
	return ret;
}

int64_t IDeviceSettings::getBaudrateFor(Network net) const {
	if(disabled) {
		report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return -1;
	}

	if(!settingsLoaded) {
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
		return -1;
	}

	switch(net.getType()) {
		case Network::Type::CAN: {
			const CAN_SETTINGS* cfg = getCANSettingsFor(net);
			if(cfg == nullptr) {
				report(APIEvent::Type::CANFDSettingsNotAvailable, APIEvent::Severity::Error);
				return -1;
			}

			int64_t baudrate = GetBaudrateValueForEnum((CANBaudrate)cfg->Baudrate);
			if(baudrate == -1) {
				report(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::Error);
				return -1;
			}
			return baudrate;
		}
		default:
			report(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::Error);
			return -1;
	}
}

bool IDeviceSettings::setBaudrateFor(Network net, int64_t baudrate) {
	if(disabled) {
		report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return false;
	}

	if(!settingsLoaded) {
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
		return false;
	}

	if(readonly) {
		report(APIEvent::Type::SettingsReadOnly, APIEvent::Severity::Error);
		return false;
	}

	switch(net.getType()) {
		case Network::Type::CAN: {
			if(baudrate > 1000000) { // This is an FD baudrate. Use setFDBaudrateFor instead.
				report(APIEvent::Type::CANFDSettingsNotAvailable, APIEvent::Severity::Error);
				return false;
			}

			CAN_SETTINGS* cfg = getMutableCANSettingsFor(net);
			if(cfg == nullptr) {
				report(APIEvent::Type::CANSettingsNotAvailable, APIEvent::Severity::Error);
				return false;
			}
								
			CANBaudrate newBaud = GetEnumValueForBaudrate(baudrate);
			if(newBaud == (CANBaudrate)-1) {
				report(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::Error);
				return false;
			}
			cfg->Baudrate = (uint8_t)newBaud;
			cfg->auto_baud = false;
			cfg->SetBaudrate = AUTO; // Device will use the baudrate value to set the TQ values
			return true;
		}
		case Network::Type::LSFTCAN: {
			CAN_SETTINGS* cfg = getMutableLSFTCANSettingsFor(net);
			if(cfg == nullptr) {
				report(APIEvent::Type::LSFTCANSettingsNotAvailable, APIEvent::Severity::Error);
				return false;
			}
				
			CANBaudrate newBaud = GetEnumValueForBaudrate(baudrate);
			if(newBaud == (CANBaudrate)-1) {
				report(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::Error);
				return false;
			}
			cfg->Baudrate = (uint8_t)newBaud;
			cfg->auto_baud = false;
			cfg->SetBaudrate = AUTO; // Device will use the baudrate value to set the TQ values
			return true;
		}
		case Network::Type::SWCAN: {
			SWCAN_SETTINGS* cfg = getMutableSWCANSettingsFor(net);
			if(cfg == nullptr) {
				report(APIEvent::Type::SWCANSettingsNotAvailable, APIEvent::Severity::Error);
				return false;
			}
								
			CANBaudrate newBaud = GetEnumValueForBaudrate(baudrate);
			if(newBaud == (CANBaudrate)-1) {
				report(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::Error);
				return false;
			}
			cfg->Baudrate = (uint8_t)newBaud;
			cfg->auto_baud = false;
			cfg->SetBaudrate = AUTO; // Device will use the baudrate value to set the TQ values
			return true;
		}
		default:
			report(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::Error);
			return false;
	}
}

int64_t IDeviceSettings::getFDBaudrateFor(Network net) const {
	if(disabled) {
		report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return -1;
	}

	if(!settingsLoaded) {
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
		return -1;
	}

	switch(net.getType()) {
		case Network::Type::CAN: {
			const CANFD_SETTINGS* cfg = getCANFDSettingsFor(net);
			if(cfg == nullptr) {
				report(APIEvent::Type::CANFDSettingsNotAvailable, APIEvent::Severity::Error);
				return -1;
			}

			int64_t baudrate = GetBaudrateValueForEnum((CANBaudrate)cfg->FDBaudrate);
			if(baudrate == -1) {
				report(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::Error);
				return -1;
			}

			return baudrate;
		}
		default:
			report(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::Error);
			return -1;
	}
}

bool IDeviceSettings::setFDBaudrateFor(Network net, int64_t baudrate) {
	if(!settingsLoaded) {
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
		return false;
	}

	if(disabled) {
		report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return false;
	}

	if(readonly) {
		report(APIEvent::Type::SettingsReadOnly, APIEvent::Severity::Error);
		return false;
	}

	switch(net.getType()) {
		case Network::Type::CAN: {
			CANFD_SETTINGS* cfg = getMutableCANFDSettingsFor(net);
			if(cfg == nullptr) {
				report(APIEvent::Type::CANFDSettingsNotAvailable, APIEvent::Severity::Error);
				return false;
			}

			CANBaudrate newBaud = GetEnumValueForBaudrate(baudrate);
			if(newBaud == (CANBaudrate)-1) {
				report(APIEvent::Type::BaudrateNotFound, APIEvent::Severity::Error);
				return false;
			}
			cfg->FDBaudrate = (uint8_t)newBaud;
			return true;
		}
		default:
			report(APIEvent::Type::UnexpectedNetworkType, APIEvent::Severity::Error);
			return false;
	}
}

bool IDeviceSettings::isTerminationSupportedFor(Network net) const {
	for(const auto& group : getTerminationGroups()) {
		for(const auto& supportedNet : group) {
			if(net == supportedNet)
				return true;
		}
	}
	return false;
}

bool IDeviceSettings::canTerminationBeEnabledFor(Network net) const {
	if(!settingsLoaded) {
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
		return false;
	}

	if(disabled) {
		report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return false;
	}

	// Even though we will not be writing here, if the settings are read only the termination will not be enablable
	if(readonly) {
		report(APIEvent::Type::SettingsReadOnly, APIEvent::Severity::Error);
		return false;
	}

	// Reference the mutable termination enables as we want to allow a disable/enable within a group without applying
	ICSNEO_UNALIGNED(const uint64_t*) currentQueuedTerminationEnables = const_cast<IDeviceSettings*>(this)->getMutableTerminationEnables();
	if(currentQueuedTerminationEnables == nullptr) {
		report(APIEvent::Type::TerminationNotSupportedDevice, APIEvent::Severity::Error);
		return false;
	}

	for(const auto& group : getTerminationGroups()) {
		bool found = false;
		for(const auto& supportedNet : group) {
			if(net == supportedNet) {
				found = true;
				break;
			}
		}

		if(found) {
			for(const auto& supportedNet : group) {
				// Allow termination on the current network even if it's already enabled
				if(net == supportedNet)
					continue;

				const auto cmNet = supportedNet.getCoreMini();
				if(!cmNet.has_value() || uint64_t(*cmNet) >= 64) {
					// Hitting this assert means that a supported network has an invalid CoreMini Network ID
					assert(false);
					continue;
				}

				// If this network is enabled, it excludes the queried network from being enabled
				if((*currentQueuedTerminationEnables >> uint64_t(*cmNet)) & 0x1) {
					report(APIEvent::Type::AnotherInTerminationGroupEnabled, APIEvent::Severity::Error);
					return false;
				}
			}
			return true;
		}
	}
	return false;
}

optional<bool> IDeviceSettings::isTerminationEnabledFor(Network net) const {
	if(!settingsLoaded) {
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
		return nullopt;
	}

	if(disabled) {
		report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return nullopt;
	}

	ICSNEO_UNALIGNED(const uint64_t*) terminationEnables = getTerminationEnables();
	if(terminationEnables == nullptr) {
		report(APIEvent::Type::TerminationNotSupportedDevice, APIEvent::Severity::Error);
		return nullopt;
	}

	const auto cmNet = net.getCoreMini();
	if(!cmNet.has_value() || uint64_t(*cmNet) >= 64 || !isTerminationSupportedFor(net)) {
		report(APIEvent::Type::TerminationNotSupportedNetwork, APIEvent::Severity::Error);
		return nullopt;
	}

	return (*terminationEnables >> uint64_t(*cmNet)) & 0x1;
}

bool IDeviceSettings::setTerminationFor(Network net, bool enabled) {
	if(!settingsLoaded) {
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
		return false;
	}

	if(disabled) {
		report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return false;
	}

	if(readonly) {
		report(APIEvent::Type::SettingsReadOnly, APIEvent::Severity::Error);
		return false;
	}

	ICSNEO_UNALIGNED(uint64_t*) terminationEnables = getMutableTerminationEnables();
	if(terminationEnables == nullptr) {
		report(APIEvent::Type::TerminationNotSupportedDevice, APIEvent::Severity::Error);
		return false;
	}

	// This function reports its own error statuses
	if(!canTerminationBeEnabledFor(net))
		return false;

	const auto cmNet = net.getCoreMini();
	if(!cmNet.has_value() || uint8_t(*cmNet) >= 64) {
		report(APIEvent::Type::TerminationNotSupportedNetwork, APIEvent::Severity::Error);
		return false;
	}

	const uint64_t mask = 1ull << uint8_t(*cmNet);
	if(enabled)
		*terminationEnables |= mask;
	else
		*terminationEnables &= ~mask;
	return true;
}

template<typename T> bool IDeviceSettings::applyStructure(const T& newStructure) {
	if(!settingsLoaded) {
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
		return false;
	}

	if(disabled) {
		report(APIEvent::Type::SettingsNotAvailable, APIEvent::Severity::Error);
		return false;
	}

	if(readonly) {
		report(APIEvent::Type::SettingsReadOnly, APIEvent::Severity::Error);
		return false;
	}
	
	// This function is only called from C++ so the caller's structure size and ours should never differ
	if(sizeof(T) != structSize) {
		report(APIEvent::Type::SettingsStructureMismatch, APIEvent::Severity::Error);
		return false; // The wrong structure was passed in for the current device
	}
	size_t copySize = sizeof(T);
	if(copySize > settings.size()) {
		report(APIEvent::Type::SettingsStructureTruncated, APIEvent::Severity::EventWarning);
		copySize = settings.size(); // TODO Warn user that their structure is truncated
	}
	// Warn user that the device firmware doesn't support all the settings in the current API
	if(copySize < settings.size())
		report(APIEvent::Type::DeviceFirmwareOutOfDate, APIEvent::Severity::EventWarning);

	memcpy(settings.data(), &newStructure, structSize);
	return apply();
}