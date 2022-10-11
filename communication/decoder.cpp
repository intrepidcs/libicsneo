#include "icsneo/communication/decoder.h"
#include "icsneo/communication/communication.h"
#include "icsneo/communication/message/serialnumbermessage.h"
#include "icsneo/communication/message/resetstatusmessage.h"
#include "icsneo/communication/message/readsettingsmessage.h"
#include "icsneo/communication/message/canerrorcountmessage.h"
#include "icsneo/communication/message/neoreadmemorysdmessage.h"
#include "icsneo/communication/message/extendedresponsemessage.h"
#include "icsneo/communication/message/wiviresponsemessage.h"
#include "icsneo/communication/message/a2bmessage.h"
#include "icsneo/communication/message/flexray/control/flexraycontrolmessage.h"
#include "icsneo/communication/command.h"
#include "icsneo/device/device.h"
#include "icsneo/communication/packet/canpacket.h"
#include "icsneo/communication/packet/a2bpacket.h"
#include "icsneo/communication/packet/ethernetpacket.h"
#include "icsneo/communication/packet/flexraypacket.h"
#include "icsneo/communication/packet/iso9141packet.h"
#include "icsneo/communication/packet/versionpacket.h"
#include "icsneo/communication/packet/ethphyregpacket.h"
#include "icsneo/communication/packet/logicaldiskinfopacket.h"
#include "icsneo/communication/packet/wivicommandpacket.h"
#include <iostream>

using namespace icsneo;

uint64_t Decoder::GetUInt64FromLEBytes(const uint8_t* bytes) {
	uint64_t ret = 0;
	for(int i = 0; i < 8; i++)
		ret |= (uint64_t(bytes[i]) << (i * 8));
	return ret;
}

bool Decoder::decode(std::shared_ptr<Message>& result, const std::shared_ptr<Packet>& packet) {
	switch(packet->network.getType()) {
		case Network::Type::Ethernet: {
			result = HardwareEthernetPacket::DecodeToMessage(packet->data, report);
			if(!result) {
				report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
				return false; // A nullptr was returned, the packet was not long enough to decode
			}

			// Timestamps are in (resolution) ns increments since 1/1/2007 GMT 00:00:00.0000
			// The resolution depends on the device
			EthernetMessage& eth = *static_cast<EthernetMessage*>(result.get());
			eth.timestamp *= timestampResolution;
			eth.network = packet->network;
			return true;
		}
		case Network::Type::CAN:
		case Network::Type::SWCAN:
		case Network::Type::LSFTCAN: {
			if(packet->data.size() < 24) {
				report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
				return false;
			}

			result = HardwareCANPacket::DecodeToMessage(packet->data);
			if(!result) {
				report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
				return false; // A nullptr was returned, the packet was malformed
			}

			// Timestamps are in (resolution) ns increments since 1/1/2007 GMT 00:00:00.0000
			// The resolution depends on the device
			result->timestamp *= timestampResolution;

			switch(result->type) {
				case Message::Type::Frame: {
					CANMessage& can = *static_cast<CANMessage*>(result.get());
					can.network = packet->network;
					break;
				}
				case Message::Type::CANErrorCount: {
					CANErrorCountMessage& can = *static_cast<CANErrorCountMessage*>(result.get());
					can.network = packet->network;
					break;
				}
				default: {
					report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
					return false; // An unknown type was returned, the packet was malformed
				}
			}

			return true;
		}
		case Network::Type::FlexRay: {
			if(packet->data.size() < 24) {
				report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
				return false;
			}

			result = HardwareFlexRayPacket::DecodeToMessage(packet->data);
			if(!result) {
				report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
				return false; // A nullptr was returned, the packet was malformed
			}

			// Timestamps are in (resolution) ns increments since 1/1/2007 GMT 00:00:00.0000
			// The resolution depends on the device
			FlexRayMessage& fr = *static_cast<FlexRayMessage*>(result.get());
			fr.timestamp *= timestampResolution;
			fr.network = packet->network;
			return true;
		}
		case Network::Type::ISO9141: {
			if(packet->data.size() < sizeof(HardwareISO9141Packet)) {
				report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
				return false;
			}

			result = iso9141decoder.decodeToMessage(packet->data);
			if(!result)
				return false; // A nullptr was returned, more data is required to decode this packet

			// Timestamps are in (resolution) ns increments since 1/1/2007 GMT 00:00:00.0000
			// The resolution depends on the device
			ISO9141Message& iso = *static_cast<ISO9141Message*>(result.get());
			iso.timestamp *= timestampResolution;
			iso.network = packet->network;
			return true;
		}
		case Network::Type::Internal: {
			switch(packet->network.getNetID()) {
				case Network::NetID::Reset_Status: {
					// We can deal with not having the last two fields (voltage and temperature)
					if(packet->data.size() < (sizeof(HardwareResetStatusPacket) - (sizeof(uint16_t) * 2))) {
						report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
						return false;
					}

					HardwareResetStatusPacket* data = (HardwareResetStatusPacket*)packet->data.data();
					auto msg = std::make_shared<ResetStatusMessage>();
					msg->mainLoopTime = data->main_loop_time_25ns * 25;
					msg->maxMainLoopTime = data->max_main_loop_time_25ns * 25;
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
					if(packet->data.size() >= sizeof(HardwareResetStatusPacket)) {
						msg->busVoltage = data->busVoltage;
						msg->deviceTemperature = data->deviceTemperature;
					}
					result = msg;
					return true;
				}
				case Network::NetID::Device: {
					// These are neoVI network messages
					// They come in as CAN but we will handle them in the device rather than
					// passing them onto the user.
					if(packet->data.size() < 24) {
						auto rawmsg = std::make_shared<RawMessage>(Network::NetID::Device);
						result = rawmsg;
						rawmsg->data = packet->data;
						return true;
					}

					result = HardwareCANPacket::DecodeToMessage(packet->data);
					if(!result) {
						report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
						return false; // A nullptr was returned, the packet was malformed
					}

					// Timestamps are in (resolution) ns increments since 1/1/2007 GMT 00:00:00.0000
					// The resolution depends on the device
					auto* raw = dynamic_cast<RawMessage*>(result.get());
					if(raw == nullptr) {
						report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
						return false; // A nullptr was returned, the packet was malformed
					}
					raw->timestamp *= timestampResolution;
					raw->network = packet->network;
					return true;
				}
				case Network::NetID::DeviceStatus: {
					// Just pass along the data, the device needs to handle this itself
					result = std::make_shared<RawMessage>(packet->network, packet->data);
					return true;
				}
				case Network::NetID::NeoMemorySDRead: {
					if(packet->data.size() != 512 + sizeof(uint32_t)) {
						report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
						return false; // Should get enough data for a start address and sector
					}

					const auto msg = std::make_shared<NeoReadMemorySDMessage>();
					result = msg;
					msg->startAddress = *reinterpret_cast<uint32_t*>(packet->data.data());
					msg->data.insert(msg->data.end(), packet->data.begin() + 4, packet->data.end());
					return true;
				}
				case Network::NetID::ExtendedCommand: {
					if(packet->data.size() < sizeof(ExtendedResponseMessage::PackedGenericResponse))
						break; // Handle as a raw message, might not be a generic response

					const auto& resp = *reinterpret_cast<ExtendedResponseMessage::PackedGenericResponse*>(packet->data.data());
					if(resp.header.command != ExtendedCommand::GenericReturn)
						break; // Handle as a raw message

					const auto msg = std::make_shared<ExtendedResponseMessage>(resp.command, resp.returnCode);
					result = msg;
					return true;
				}
				case Network::NetID::FlexRayControl: {
					auto frResult = std::make_shared<FlexRayControlMessage>(*packet);
					if(!frResult->decoded) {
						report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
						return false;
					}
					result = frResult;
					return true;
				}
				case Network::NetID::Main51: {
					switch((Command)packet->data[0]) {
						case Command::RequestSerialNumber: {
							auto msg = std::make_shared<SerialNumberMessage>();
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
						case Command::GetMainVersion: {
							result = HardwareVersionPacket::DecodeMainToMessage(packet->data);
							if(!result) {
								report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
								return false;
							}

							return true;
						}
						case Command::GetSecondaryVersions: {
							result = HardwareVersionPacket::DecodeSecondaryToMessage(packet->data);
							if(!result) {
								report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
								return false;
							}

							return true;
						}
						default:
							auto msg = std::make_shared<Main51Message>();
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
					 * length from the first two bytes in its place. Ideally, we never actually send the oldformat messages
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
				case Network::NetID::LogicalDiskInfo: {
					result = LogicalDiskInfoPacket::DecodeToMessage(packet->data);
					if(!result) {
						report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::EventWarning);
						return false;
					}
					return true;
				}
				case Network::NetID::WiVICommand: {
					result = WiVI::CommandPacket::DecodeToMessage(packet->data);
					if(!result) {
						report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::EventWarning);
						return false;
					}
					return true;
				}
				case Network::NetID::EthPHYControl: {
					result = HardwareEthernetPhyRegisterPacket::DecodeToMessage(packet->data, report);
					if(!result) {
						report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::EventWarning);
						return false;
					}
					return true;
				}
				default:
					break;
			}
		}
		case Network::Type::A2B: {
			result = HardwareA2BPacket::DecodeToMessage(packet->data);

			if(!result) {
				report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
				return false; // A nullptr was returned, the packet was not long enough to decode
			}

			A2BMessage& msg = *static_cast<A2BMessage*>(result.get());
			msg.network = packet->network;
			return true;
		}
	}

	// For the moment other types of messages will automatically be decoded as raw messages
	result = std::make_shared<RawMessage>(packet->network, packet->data);
	return true;
}