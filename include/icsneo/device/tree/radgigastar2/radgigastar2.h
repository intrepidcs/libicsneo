#ifndef __RADGIGASTAR2_H_
#define __RADGIGASTAR2_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/device/tree/radgigastar2/radgigastar2settings.h"

namespace icsneo
{

	class RADGigastar2 : public Device
	{
	public:
		// Serial numbers start with GT
		// USB PID is 0x1210, standard driver is FTDI3
		// Ethernet MAC allocation is 0x22, standard driver is Raw
		ICSNEO_FINDABLE_DEVICE(RADGigastar2, DeviceType::RADGigastar2, "GT");

		static const std::vector<Network> &GetSupportedNetworks()
		{
			static std::vector<Network> supportedNetworks = {
				Network::NetID::HSCAN,
				Network::NetID::HSCAN2,
				Network::NetID::HSCAN3,
				Network::NetID::HSCAN4,

				Network::NetID::Ethernet,
				Network::NetID::Ethernet2,

				Network::NetID::OP_Ethernet1,
				Network::NetID::OP_Ethernet2,
				Network::NetID::OP_Ethernet3,
				Network::NetID::OP_Ethernet4,
				Network::NetID::OP_Ethernet5,
				Network::NetID::OP_Ethernet6,
				Network::NetID::OP_Ethernet7,
				Network::NetID::OP_Ethernet8,
				Network::NetID::OP_Ethernet9,
				Network::NetID::OP_Ethernet10,

				Network::NetID::LIN,
				Network::NetID::LIN2,
				Network::NetID::LIN3,
				Network::NetID::LIN4,
				Network::NetID::LIN5,
				Network::NetID::LIN6,
				Network::NetID::LIN7,
				Network::NetID::LIN8,
				Network::NetID::LIN9,
				Network::NetID::LIN10,

				Network::NetID::I2C,
				Network::NetID::I2C2,

				Network::NetID::MDIO1,
				Network::NetID::MDIO2,

				Network::NetID::SPI1,
				Network::NetID::SPI2,
				Network::NetID::SPI3,
				Network::NetID::SPI4,
				Network::NetID::SPI5,
				Network::NetID::SPI6,
				Network::NetID::SPI7,
				Network::NetID::SPI8,
			};
			return supportedNetworks;
		}

		size_t getEthernetActivationLineCount() const override { return 1; }

		bool getEthPhyRegControlSupported() const override { return true; }

		bool supportsTC10() const override { return true; }
		bool supportsGPTP() const override { return true; }
	
	protected:
		RADGigastar2(neodevice_t neodevice, const driver_factory_t &makeDriver) : Device(neodevice)
		{
			initialize<RADGigastar2Settings>(makeDriver);
		}

		void setupPacketizer(Packetizer &packetizer) override
		{
			Device::setupPacketizer(packetizer);
			packetizer.disableChecksum = true;
			packetizer.align16bit = false;
		}

		void setupDecoder(Decoder &decoder) override
		{
			Device::setupDecoder(decoder);
			decoder.timestampResolution = 10; // Timestamps are in 10ns increments instead of the usual 25ns
		}

		void setupEncoder(Encoder &encoder) override
		{
			Device::setupEncoder(encoder);
			encoder.supportCANFD = true;
			encoder.supportEthPhy = true;
		}

		void setupSupportedRXNetworks(std::vector<Network> &rxNetworks) override
		{
			for (auto &netid : GetSupportedNetworks())
				rxNetworks.emplace_back(netid);
		}

		// The supported TX networks are the same as the supported RX networks for this device
		void setupSupportedTXNetworks(std::vector<Network> &txNetworks) override { setupSupportedRXNetworks(txNetworks); }

		void handleDeviceStatus(const std::shared_ptr<InternalMessage> &message) override
		{
			if (message->data.size() < sizeof(radgigastar2_status_t))
				return;
			std::lock_guard<std::mutex> lk(ioMutex);
			const radgigastar2_status_t *status = reinterpret_cast<const radgigastar2_status_t *>(message->data.data());
			ethActivationStatus = status->ethernetActivationLineEnabled;
		}

		std::optional<MemoryAddress> getCoreminiStartAddressFlash() const override
		{
			return 512 * 4;
		}

		std::optional<MemoryAddress> getCoreminiStartAddressSD() const override
		{
			return 0;
		}
	};

}

#endif // __cplusplus

#endif