#include "icsneo/communication/decoder.h"
#include "icsneo/communication/communication.h"
#include "icsneo/communication/message/serialnumbermessage.h"
#include "icsneo/communication/message/resetstatusmessage.h"
#include "icsneo/communication/message/readsettingsmessage.h"
#include "icsneo/communication/command.h"
#include "icsneo/device/device.h"
#include <iostream>

using namespace icsneo;

uint64_t Decoder::GetUInt64FromLEBytes(uint8_t* bytes) {
	uint64_t ret = 0;
	for(int i = 0; i < 8; i++)
		ret |= (bytes[i] << (i * 8));
	return ret;
}

bool Decoder::decode(std::shared_ptr<Message>& result, const std::shared_ptr<Packet>& packet) {
	switch(packet->network.getType()) {
		case Network::Type::CAN: {
			if(packet->data.size() < 24)
				return false;

			HardwareCANPacket* data = (HardwareCANPacket*)packet->data.data();
			auto msg = std::make_shared<CANMessage>();
			msg->network = packet->network;

			// Timestamp calculation
			msg->timestamp = data->timestamp.TS * 25; // Timestamps are in 25ns increments since 1/1/2007 GMT 00:00:00.0000

			// Arb ID
			if(data->header.IDE) { // Extended 29-bit ID
				msg->arbid = (data->header.SID & 0x7ff) << 18;
				msg->arbid |= (data->eid.EID & 0xfff) << 6;
				msg->arbid |= (data->dlc.EID2 & 0x3f);
				msg->isExtended = true;
			} else { // Standard 11-bit ID
				msg->arbid = data->header.SID;
			}

			// DLC
			uint8_t length = data->dlc.DLC;
			msg->dlcOnWire = length; // This will hold the real DLC on wire 0x0 - 0xF
			if(data->header.EDL && data->timestamp.IsExtended) { // CAN FD
				msg->isCANFD = true;
				msg->baudrateSwitch = data->header.BRS; // CAN FD Baudrate Switch
				if(length > 8) {
					switch(length) { // CAN FD Length Decoding
						case 0x9:
							length = 12;
							break;
						case 0xa:
							length = 16;
							break;
						case 0xb:
							length = 20;
							break;
						case 0xc:
							length = 24;
							break;
						case 0xd:
							length = 32;
							break;
						case 0xe:
							length = 48;
							break;
						case 0xf:
							length = 64;
							break;
						default:
							return false;
					}
				}
			} else if(length > 8) { // This is a standard CAN frame with a length of more than 8
				// Yes, this is possible. On the wire, the length field is a nibble, and we do want to return an accurate value
				// We don't want to overread our buffer, though, so make sure we cap the length
				length = 8;
			}
			
			// Data
			// The first 8 bytes are always in the standard place
			if((data->dlc.RTR && data->header.IDE) || (!data->header.IDE && data->header.SRR)) { // Remote Request Frame
				msg->data.resize(length); // This data will be all zeros, but the length will be set
				msg->isRemote = true;
			} else {
				msg->data.reserve(length);
				msg->data.insert(msg->data.end(), data->data, data->data + (length > 8 ? 8 : length));
				if(length > 8) { // If there are more than 8 bytes, they come at the end of the message
					// Messages with extra data are formatted as message, then uint16_t netid, then uint16_t length, then extra data
					uint8_t* extraDataStart = packet->data.data() + sizeof(HardwareCANPacket) + 2 + 2;
					msg->data.insert(msg->data.end(), extraDataStart, extraDataStart + (length - 8));
				}
			}

			result = msg;
			return true;
		}
		case Network::Type::Internal: {
			switch(packet->network.getNetID()) {
				case Network::NetID::Reset_Status: {
					if(packet->data.size() < sizeof(HardwareResetStatusPacket))
						return false;

					HardwareResetStatusPacket* data = (HardwareResetStatusPacket*)packet->data.data();
					auto msg = std::make_shared<ResetStatusMessage>();
					msg->network = packet->network;
					msg->mainLoopTime = data->main_loop_time_25ns * 25;
					msg->maxMainLoopTime = data->max_main_loop_time_25ns * 25;
					msg->busVoltage = data->busVoltage;
					msg->deviceTemperature = data->deviceTemperature;
					msg->justReset = data->status.just_reset;
					msg->comEnabled = data->status.com_enabled;
					msg->cmRunning = data->status.cm_is_running;
					msg->cmChecksumFailed = data->status.cm_checksum_failed;
					msg->cmLicenseFailed = data->status.cm_license_failed;
					msg->cmVersionMismatch = data->status.cm_version_mismatch;
					msg->cmBootOff = data->status.cm_boot_off;
					msg->hardwareFailure = data->status.hardware_failure;
					msg->usbComEnabled = data->status.usbComEnabled;
					msg->linuxComEnabled = data->status.linuxComEnabled;
					msg->cmTooBig = data->status.cm_too_big;
					msg->hidUsbState = data->status.hidUsbState;
					msg->fpgaUsbState = data->status.fpgaUsbState;
					result = msg;
					return true;
				}
				default:
					break;//return false;
			}
		}
		default:
			switch(packet->network.getNetID()) {
				case Network::NetID::Main51: {
					switch((Command)packet->data[0]) {
						case Command::RequestSerialNumber: {
							auto msg = std::make_shared<SerialNumberMessage>();
							msg->network = packet->network;
							uint64_t serial = GetUInt64FromLEBytes(packet->data.data() + 1);
							// The device sends 64-bits of serial number, but we never use more than 32-bits.
							msg->deviceSerial = Device::SerialNumToString((uint32_t)serial);
							msg->hasMacAddress = packet->data.size() >= 15;
							if(msg->hasMacAddress)
								memcpy(msg->macAddress, packet->data.data() + 9, sizeof(msg->macAddress));
							msg->hasPCBSerial = packet->data.size() >= 31;
							if(msg->hasPCBSerial)
								memcpy(msg->pcbSerial, packet->data.data() + 15, sizeof(msg->pcbSerial));
							result = msg;
							return true;
						}
						default:
							auto msg = std::make_shared<Main51Message>();
							msg->network = packet->network;
							msg->command = Command(packet->data[0]);
							msg->data.insert(msg->data.begin(), packet->data.begin() + 1, packet->data.end());
							result = msg;
							return true;
					}
				}
				case Network::NetID::RED_OLDFORMAT: {
					/* So-called "old format" messages are a "new style, long format" wrapper around the old short messages.
					 * They consist of a 16-bit LE length first, then the 8-bit length and netid combo byte, then the payload
					 * with no checksum. The upper-nibble length of the combo byte should be ignored completely, using the
					 * length from the first two bytes in it's place. Ideally, we never actually send the oldformat messages
					 * out to the rest of the application as they can recursively get decoded to another message type here.
					 * Feed the result back into the decoder in case we do something special with the resultant netid.
					 */
					uint16_t length = packet->data[0] | (packet->data[1] << 8);
					packet->network = Network(packet->data[2] & 0xF);
					packet->data.erase(packet->data.begin(), packet->data.begin() + 3);
					if(packet->data.size() != length)
						packet->data.resize(length);
					return decode(result, packet);
				}
				case Network::NetID::ReadSettings: {
					auto msg = std::make_shared<ReadSettingsMessage>();
					msg->network = packet->network;
					msg->response = ReadSettingsMessage::Response(packet->data[0]);
					
					if(msg->response == ReadSettingsMessage::Response::OK) {
						// The global settings structure is the payload of the message in this case
						msg->data.insert(msg->data.begin(), packet->data.begin() + 10, packet->data.end());
						uint16_t resp_len = msg->data[8] | (msg->data[9] << 8);
						if(msg->data.size() - 1 == resp_len) // There is a padding byte at the end
							msg->data.pop_back();
						result = msg;
						return true;
					}

					// We did not get a successful response, so the payload is all of the data
					msg->data.insert(msg->data.begin(), packet->data.begin(), packet->data.end());
					result = msg;
					return true;
				}
			}
	}

	// For the moment other types of messages will automatically be decoded as raw messages
	auto msg = std::make_shared<Message>();
	msg->network = packet->network;
	msg->data = packet->data;
	result = msg;
	return true;
}