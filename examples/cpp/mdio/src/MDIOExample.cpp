#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

#include "icsneo/icsneocpp.h"
#include "icsneo/communication/message/mdiomessage.h"

/* Note: This example requires MDIO 1 channels to be connected on the device, and uses RADGalaxy or RADStar2 */

char getCharInput(std::vector<char> allowed)
{
	bool found = false;
	std::string input;

	while (!found)
	{
		std::cin >> input;

		if (input.length() == 1)
		{
			for (char compare : allowed)
			{
				if (compare == input.c_str()[0])
				{
					found = true;
					break;
				}
			}
		}

		if (!found)
		{
			std::cout << "Input did not match expected options. Please try again." << std::endl;
			std::cout << "<X or x to quit>" << std::endl;
		}
	}

	return input.c_str()[0];
}

int main()
{
	// Print version
	std::cout << "Running libicsneo " << icsneo::GetVersion() << std::endl;
	std::cout << "\nFinding devices... " << std::flush;
	auto devices = icsneo::FindAllDevices(); // This is type std::vector<std::shared_ptr<icsneo::Device>>
	// You now hold the shared_ptrs for these devices, you are considered to "own" these devices from a memory perspective
	std::cout << "OK, " << devices.size() << " device" << (devices.size() == 1 ? "" : "s") << " found" << std::endl;

	// List off the devices
	for (auto &device : devices)
		std::cout << '\t' << device->describe() << " @ Handle " << device->getNeoDevice().handle << std::endl;
	std::cout << std::endl;

	for (auto &device : devices)
	{
		std::cout << "Connecting to " << device->describe() << "... ";
		bool ret = device->open();
		if (!ret)
		{ // Failed to open
			std::cout << "FAIL" << std::endl;
			std::cout << icsneo::GetLastError() << std::endl
					  << std::endl;
			continue;
		}
		std::cout << "OK" << std::endl;

		// The concept of going "online" tells the connected device to start listening, i.e. ACKing traffic and giving it to us
		std::cout << "\tGoing online... ";
		ret = device->goOnline();
		if (!ret)
		{
			std::cout << "FAIL" << std::endl;
			device->close();
			continue;
		}
		std::cout << "OK" << std::endl;

		// A real application would just check the result of icsneo_goOnline() rather than calling this
		// This function is intended to be called later on if needed
		std::cout << "\tChecking online status... ";
		ret = device->isOnline();
		if (!ret)
		{
			std::cout << "FAIL\n"
					  << std::endl;
			device->close();
			continue;
		}
		std::cout << "OK" << std::endl;

		auto handler = device->addMessageCallback(std::make_shared<icsneo::MessageCallback>([&](std::shared_ptr<icsneo::Message> message)
																							{
			if(icsneo::Message::Type::Frame == message->type) {
				auto frame = std::static_pointer_cast<icsneo::Frame>(message);
				if(icsneo::Network::Type::MDIO == frame->network.getType()) {
					auto msg = std::static_pointer_cast<icsneo::MDIOMessage>(message);
					std::cout << msg->network << " " << ((msg->isTXMsg)? "TX" : "RX") << " frame\n";
					std::cout << "Clause: " << ((msg->clause ==  icsneo::MDIOMessage::Clause::Clause22) ? "22" : "45") << "\n";
					std::cout << "Direction: " << ((msg->direction ==  icsneo::MDIOMessage::Direction::Read) ? "Read" : "Write") << "\n";
					std::cout << std::hex << "PHY Address: 0x" << static_cast<int>(msg->phyAddress) << "\n";
					if(msg->clause ==  icsneo::MDIOMessage::Clause::Clause45)
						std::cout << std::hex << "Dev Address: 0x" << static_cast<int>(msg->devAddress) << "\n";
					std::cout << std::hex << "Reg Address: 0x" << static_cast<int>(msg->regAddress) << "\n";
					std::cout << "Data: \n";
					for(uint8_t& each : msg->data) {
						std::cout << std::hex << "0x" << static_cast<int>(each) << " ";
					}
					std::cout << "\n";
				}
			} }));

		/*
		 * BCM89810 on RADGalaxy/RADGigastar
		 */
		// We can transmit messages to read the PHY ID of BCM89810 PHY
		std::cout << "\tTransmitting a MDIO request to read ID on BCM89810...\n";
		auto mdio_r = std::make_shared<icsneo::MDIOMessage>();
		mdio_r->network = icsneo::Network::NetID::MDIO1;
		mdio_r->phyAddress = 0x00u;
		mdio_r->regAddress = 0x02u;
		mdio_r->direction = icsneo::MDIOMessage::Direction::Read;
		mdio_r->clause = icsneo::MDIOMessage::Clause::Clause22;
		ret = device->transmit(mdio_r); // This will return false if the device does not support MDIO
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		// We can transmit messages to write to arbitrary register
		std::cout << "\tTransmitting a MDIO request to write register on BCM89810...\n";
		mdio_r = std::make_shared<icsneo::MDIOMessage>();
		mdio_r->network = icsneo::Network::NetID::MDIO1;
		mdio_r->phyAddress = 0x00u;
		mdio_r->regAddress = 0x1Bu;
		mdio_r->data = {0xAA, 0xAF};
		mdio_r->direction = icsneo::MDIOMessage::Direction::Write;
		mdio_r->clause = icsneo::MDIOMessage::Clause::Clause22;
		ret = device->transmit(mdio_r); // This will return false if the device does not support MDIO
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		// We can transmit messages to read back to arbitrary register
		std::cout << "\tTransmitting a MDIO request to read register on BCM89810...\n";
		mdio_r = std::make_shared<icsneo::MDIOMessage>();
		mdio_r->network = icsneo::Network::NetID::MDIO1;
		mdio_r->phyAddress = 0x00u;
		mdio_r->regAddress = 0x1Bu;
		mdio_r->direction = icsneo::MDIOMessage::Direction::Read;
		mdio_r->clause = icsneo::MDIOMessage::Clause::Clause22;
		ret = device->transmit(mdio_r); // This will return false if the device does not support MDIO
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		/*
		 * 88Q2112 on RADGigastar, RADSupermoon, RADMoon2
		 */
		// We can transmit messages to read the PHY ID of BCM89810 PHY
		std::cout << "\tTransmitting a MDIO request to read ID on 88Q2112...\n";
		mdio_r = std::make_shared<icsneo::MDIOMessage>();
		mdio_r->network = icsneo::Network::NetID::MDIO1;
		mdio_r->phyAddress = 0x06u;
		mdio_r->devAddress = 0x01u;
		mdio_r->regAddress = 0x0002u;
		mdio_r->direction = icsneo::MDIOMessage::Direction::Read;
		mdio_r->clause = icsneo::MDIOMessage::Clause::Clause45;
		ret = device->transmit(mdio_r); // This will return false if the device does not support MDIO
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		// We can transmit messages to write to arbitrary register
		std::cout << "\tTransmitting a MDIO request to write register on 88Q2112...\n";
		mdio_r = std::make_shared<icsneo::MDIOMessage>();
		mdio_r->network = icsneo::Network::NetID::MDIO1;
		mdio_r->phyAddress = 0x06u;
		mdio_r->devAddress = 0x01u;
		mdio_r->regAddress = 0x0902u;
		mdio_r->data = {0xA3, 0x02};
		mdio_r->direction = icsneo::MDIOMessage::Direction::Write;
		mdio_r->clause = icsneo::MDIOMessage::Clause::Clause45;
		ret = device->transmit(mdio_r); // This will return false if the device does not support MDIO
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		// We can transmit messages to read back to arbitrary register
		std::cout << "\tTransmitting a MDIO request to read register on 88Q2112...\n";
		mdio_r = std::make_shared<icsneo::MDIOMessage>();
		mdio_r->network = icsneo::Network::NetID::MDIO1;
		mdio_r->phyAddress = 0x06u;
		mdio_r->devAddress = 0x01u;
		mdio_r->regAddress = 0x0902u;
		mdio_r->direction = icsneo::MDIOMessage::Direction::Read;
		mdio_r->clause = icsneo::MDIOMessage::Clause::Clause45;
		ret = device->transmit(mdio_r); // This will return false if the device does not support MDIO
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "<X or x to quit>\n\n";

		// Go offline, stop sending and receiving traffic
		auto shutdown = [&]()
		{
			device->removeMessageCallback(handler);
			std::cout << "\tGoing offline... ";
			ret = device->goOffline();
			std::cout << (ret ? "OK" : "FAIL") << std::endl;
			std::cout << "\tDisconnecting... ";
			ret = device->close();
			std::cout << (ret ? "OK\n" : "FAIL\n") << std::endl;
		};

		while (true)
		{
			char input = getCharInput(std::vector<char>{'X', 'x'});
			switch (input)
			{
			case 'X':
			case 'x':
				shutdown();
				printf("Exiting program\n");
				return 0;
			default:
				break;
			}
		}
	}
	return 0;
}