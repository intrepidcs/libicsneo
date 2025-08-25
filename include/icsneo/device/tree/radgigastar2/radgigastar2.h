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
		// USB PID is 0x1210, standard driver is DXX
		// Ethernet MAC allocation is 0x22, standard driver is Raw
		ICSNEO_FINDABLE_DEVICE(RADGigastar2, DeviceType::RADGigastar2, "GT");

		static const std::vector<Network> &GetSupportedNetworks()
		{
			static std::vector<Network> supportedNetworks = {
				Network::NetID::DWCAN_01,
				Network::NetID::DWCAN_02,
				Network::NetID::DWCAN_03,
				Network::NetID::DWCAN_04,

				Network::NetID::ETHERNET_01,
				Network::NetID::ETHERNET_02,

				Network::NetID::AE_01,
				Network::NetID::AE_02,
				Network::NetID::AE_03,
				Network::NetID::AE_04,
				Network::NetID::AE_05,
				Network::NetID::AE_06,
				Network::NetID::AE_07,
				Network::NetID::AE_08,
				Network::NetID::AE_09,
				Network::NetID::AE_10,

				Network::NetID::LIN_01,
				Network::NetID::LIN_02,
				Network::NetID::LIN_03,
				Network::NetID::LIN_04,
				Network::NetID::LIN_05,
				Network::NetID::LIN_06,
				Network::NetID::LIN_07,
				Network::NetID::LIN_08,
				Network::NetID::LIN_09,
				Network::NetID::LIN_10,
				Network::NetID::LIN_11,
				Network::NetID::LIN_12,
				Network::NetID::LIN_13,
				Network::NetID::LIN_14,
				Network::NetID::LIN_15,
				Network::NetID::LIN_16,

				Network::NetID::I2C_01,
				Network::NetID::I2C_02,

				Network::NetID::MDIO_01,
				Network::NetID::MDIO_02,

				Network::NetID::SPI_01,
				Network::NetID::SPI_02,
				Network::NetID::SPI_03,
				Network::NetID::SPI_04,
				Network::NetID::SPI_05,
				Network::NetID::SPI_06,
				Network::NetID::SPI_07,
				Network::NetID::SPI_08,
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

		void handleDeviceStatus(const std::shared_ptr<RawMessage> &message) override
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
		
		size_t getDiskCount() const override 
		{
			return 1;
		}
	};

}

#endif // __cplusplus

#endif