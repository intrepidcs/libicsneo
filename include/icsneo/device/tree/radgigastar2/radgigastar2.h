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
		ICSNEO_FINDABLE_DEVICE(RADGigastar2, _icsneo_devicetype_t::icsneo_devicetype_rad_gigastar2, "GT");

		static const std::vector<Network> &GetSupportedNetworks()
		{
			static std::vector<Network> supportedNetworks = {
				_icsneo_netid_t::icsneo_netid_hscan,
				_icsneo_netid_t::icsneo_netid_hscan2,
				_icsneo_netid_t::icsneo_netid_hscan3,
				_icsneo_netid_t::icsneo_netid_hscan4,

				_icsneo_netid_t::icsneo_netid_ethernet,
				_icsneo_netid_t::icsneo_netid_ethernet2,

				_icsneo_netid_t::icsneo_netid_op_ethernet1,
				_icsneo_netid_t::icsneo_netid_op_ethernet2,
				_icsneo_netid_t::icsneo_netid_op_ethernet3,
				_icsneo_netid_t::icsneo_netid_op_ethernet4,
				_icsneo_netid_t::icsneo_netid_op_ethernet5,
				_icsneo_netid_t::icsneo_netid_op_ethernet6,
				_icsneo_netid_t::icsneo_netid_op_ethernet7,
				_icsneo_netid_t::icsneo_netid_op_ethernet8,
				_icsneo_netid_t::icsneo_netid_op_ethernet9,
				_icsneo_netid_t::icsneo_netid_op_ethernet10,

				_icsneo_netid_t::icsneo_netid_lin,
				_icsneo_netid_t::icsneo_netid_lin2,
				_icsneo_netid_t::icsneo_netid_lin3,
				_icsneo_netid_t::icsneo_netid_lin4,
				_icsneo_netid_t::icsneo_netid_lin5,
				_icsneo_netid_t::icsneo_netid_lin6,
				_icsneo_netid_t::icsneo_netid_lin7,
				_icsneo_netid_t::icsneo_netid_lin8,
				_icsneo_netid_t::icsneo_netid_lin9,
				_icsneo_netid_t::icsneo_netid_lin10,

				_icsneo_netid_t::icsneo_netid_i2c,
				_icsneo_netid_t::icsneo_netid_i2c2,

				_icsneo_netid_t::icsneo_netid_mdio1,
				_icsneo_netid_t::icsneo_netid_mdio2,

				_icsneo_netid_t::icsneo_netid_spi1,
				_icsneo_netid_t::icsneo_netid_spi2,
				_icsneo_netid_t::icsneo_netid_spi3,
				_icsneo_netid_t::icsneo_netid_spi4,
				_icsneo_netid_t::icsneo_netid_spi5,
				_icsneo_netid_t::icsneo_netid_spi6,
				_icsneo_netid_t::icsneo_netid_spi7,
				_icsneo_netid_t::icsneo_netid_spi8,
			};
			return supportedNetworks;
		}

		size_t getEthernetActivationLineCount() const override { return 1; }

		bool getEthPhyRegControlSupported() const override { return true; }

		bool supportsTC10() const override { return true; }

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