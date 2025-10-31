#include "icsneo/communication/message/neomessage.h"
#include "icsneo/communication/message/canmessage.h"
#include "icsneo/communication/message/ethernetmessage.h"
#include "icsneo/communication/message/canerrormessage.h"
#include "icsneo/communication/message/linmessage.h"

using namespace icsneo;

neomessage_t icsneo::CreateNeoMessage(const std::shared_ptr<Message> message) {
	// This function is not responsible for storing the message!
	// Keep the shared_ptr around for the lifetime of the data access
	neomessage_t neomsg = {}; // Clear out the memory
	neomsg.messageType = (neomessagetype_t)message->type;
	neomsg.timestamp = message->timestamp;
	switch (message->type)
	{
	case Message::Type::Frame: {
		neomessage_frame_t& frame = *(neomessage_frame_t*)&neomsg;
		auto framemsg = std::static_pointer_cast<Frame>(message);
		const auto netType = framemsg->network.getType();

		frame.netid = (neonetid_t)framemsg->network.getNetID();
		frame.type = (neonettype_t)netType;
		frame.description = framemsg->description;
		frame.length = framemsg->data.size();
		frame.data = framemsg->data.data();
		frame.timestamp = framemsg->timestamp;
		frame.status.globalError = framemsg->error;
		frame.status.transmitMessage = framemsg->transmitted;

		switch(netType) {
			case Network::Type::CAN:
			case Network::Type::SWCAN:
			case Network::Type::LSFTCAN: {
				neomessage_can_t& can = *(neomessage_can_t*)&neomsg;
				auto canmsg = std::static_pointer_cast<CANMessage>(message);
				can.arbid = canmsg->arbid;
				can.dlcOnWire = canmsg->dlcOnWire;
				can.status.extendedFrame = canmsg->isExtended;
				can.status.remoteFrame = canmsg->isRemote;
				can.status.canfdRTR = canmsg->isRemote;
				can.status.canfdFDF = canmsg->isCANFD;
				can.status.canfdBRS = canmsg->baudrateSwitch;
				can.status.canfdESI = canmsg->errorStateIndicator;
				break;
			}
			case Network::Type::Ethernet:
			case Network::Type::AutomotiveEthernet: {
				neomessage_eth_t& eth = *(neomessage_eth_t*)&neomsg;
				auto ethmsg = std::static_pointer_cast<EthernetMessage>(message);
				eth.preemptionFlags = ethmsg->preemptionFlags;
				eth.status.incompleteFrame = ethmsg->frameTooShort;
				// TODO Fill in extra status bits
				//eth.status.xyz = ethmsg->preemptionEnabled;
				//eth.status.xyz = ethmsg->fcs;
				//eth.status.xyz = ethmsg->noPadding;
				break;
			}
			case Network::Type::LIN: {
				neomessage_lin_t& lin = *(neomessage_lin_t*)&neomsg;
				auto linmsg = std::static_pointer_cast<LINMessage>(message);
				if(!linmsg) { break; }
				const auto linHdrBytes = std::min(linmsg->data.size(), static_cast<size_t>(2));
				lin.header[0] = linmsg->protectedID;
				std::copy(linmsg->data.begin(), linmsg->data.begin() + linHdrBytes, lin.header + 1);
				linmsg->calcChecksum(*linmsg);
				lin.checksum = linmsg->checksum;
				lin.data = linmsg->data.data() + linHdrBytes;
				if(linmsg->isEnhancedChecksum != linmsg->statusFlags.TxChecksumEnhanced) {
					linmsg->isEnhancedChecksum = true;
					linmsg->statusFlags.TxChecksumEnhanced = true;
				}
				if(linmsg->data.size())
					lin.length = linmsg->data.size() + 2;
				else
					lin.length = 1;
				
				lin.linStatus = {
					linmsg->statusFlags.TxChecksumEnhanced, // .txChecksumEnhanced
					linmsg->statusFlags.TxCommander, // .txCommander
					linmsg->statusFlags.TxResponder, // .txResponder
					0, // .txAborted
					linmsg->statusFlags.UpdateResponderOnce, // .updateResponderOnce
					linmsg->statusFlags.HasUpdatedResponderOnce, // .hasUpdatedResponderOnce
					linmsg->statusFlags.BusRecovered, // .busRecovered
					linmsg->statusFlags.BreakOnly // .breakOnly
					
				};

				lin.status.linJustBreakSync = linmsg->errFlags.ErrRxBreakSyncOnly;
				lin.status.linErrorTXRXMismatch = linmsg->errFlags.ErrTxRxMismatch;
				lin.status.linErrorRXBreakNotZero = linmsg->errFlags.ErrRxBreakNotZero;
				lin.status.linErrorRXBreakTooShort = linmsg->errFlags.ErrRxBreakTooShort;
				lin.status.linErrorRXSyncNot55 = linmsg->errFlags.ErrRxSyncNot55;
				lin.status.linErrorRXDataGreaterEight = linmsg->errFlags.ErrRxDataLenOver8;
				lin.status.linSyncFrameError = linmsg->errFlags.ErrFrameSync;
				lin.status.linIDFrameError = linmsg->errFlags.ErrFrameMessageID;
				lin.status.linSlaveByteError = linmsg->errFlags.ErrFrameResponderData;
				lin.status.checksumError = linmsg->errFlags.ErrChecksumMatch;
				break;
			}
			default:
				// TODO Implement others
				break;
		}
		break;
	}
	case Message::Type::CANErrorCount: {
		neomessage_can_error_t& canerror = *(neomessage_can_error_t*)&neomsg;
		auto canerrormsg = std::static_pointer_cast<CANErrorMessage>(message);
		canerror.transmitErrorCount = canerrormsg->transmitErrorCount;
		canerror.receiveErrorCount = canerrormsg->receiveErrorCount;
		canerror.status.canBusOff = canerrormsg->busOff;
		canerror.netid = (neonetid_t)canerrormsg->network.getNetID();
		canerror.type = (neonettype_t)canerrormsg->network.getType();
		break;
	}
	default:
		break;
	}
	return neomsg;
}

std::shared_ptr<Message> icsneo::CreateMessageFromNeoMessage(const neomessage_t* neomessage) {
	switch((Message::Type)neomessage->messageType) {
		case Message::Type::Frame: {
			const Network network = ((neomessage_frame_t*)neomessage)->netid;
			switch(network.getType()) {
				case Network::Type::CAN:
				case Network::Type::SWCAN:
				case Network::Type::LSFTCAN: {
					neomessage_can_t& can = *(neomessage_can_t*)neomessage;
					auto canmsg = std::make_shared<CANMessage>();
					canmsg->network = network;
					canmsg->description = can.description;
					canmsg->data.insert(canmsg->data.end(), can.data, can.data + can.length);
					canmsg->arbid = can.arbid;
					canmsg->dlcOnWire = can.dlcOnWire;
					canmsg->isExtended = can.status.extendedFrame;
					canmsg->isRemote = can.status.remoteFrame | can.status.canfdRTR;
					canmsg->isCANFD = can.status.canfdFDF;
					canmsg->baudrateSwitch = can.status.canfdBRS;
					canmsg->errorStateIndicator = can.status.canfdESI;
					return canmsg;
				}
				case Network::Type::Ethernet:
				case Network::Type::AutomotiveEthernet: {
					neomessage_eth_t& eth = *(neomessage_eth_t*)neomessage;
					auto ethmsg = std::make_shared<EthernetMessage>();
					ethmsg->network = network;
					ethmsg->description = eth.description;
					ethmsg->data.insert(ethmsg->data.end(), eth.data, eth.data + eth.length);
					return ethmsg;
				}
				case Network::Type::LIN: {
					neomessage_lin_t& lin = *(neomessage_lin_t*)neomessage;
					auto linmsg = std::make_shared<LINMessage>();
					linmsg->network = network;
					linmsg->description = lin.description;
					linmsg->protectedID = lin.header[0];
					linmsg->ID = linmsg->protectedID & 0x3F;
					linmsg->statusFlags = {
						static_cast<bool>(lin.linStatus.txChecksumEnhanced),
						static_cast<bool>(lin.linStatus.txCommander),
						static_cast<bool>(lin.linStatus.txResponder),
						static_cast<bool>(lin.linStatus.updateResponderOnce),
						static_cast<bool>(lin.linStatus.hasUpdatedResponderOnce),
						static_cast<bool>(lin.linStatus.busRecovered),
						static_cast<bool>(lin.linStatus.breakOnly)
					};
					linmsg->isEnhancedChecksum = linmsg->statusFlags.TxChecksumEnhanced;
					linmsg->errFlags = {
						static_cast<bool>(lin.linStatus.breakOnly),
						static_cast<bool>(lin.status.linJustBreakSync),
						static_cast<bool>(lin.status.linErrorTXRXMismatch),
						static_cast<bool>(lin.status.linErrorRXBreakNotZero),
						static_cast<bool>(lin.status.linErrorRXBreakTooShort),
						static_cast<bool>(lin.status.linErrorRXSyncNot55),
						static_cast<bool>(lin.status.linErrorRXDataGreaterEight),
						static_cast<bool>(lin.status.linSyncFrameError),
						static_cast<bool>(lin.status.linIDFrameError),
						static_cast<bool>(lin.status.linSlaveByteError),
						static_cast<bool>(lin.status.checksumError)
					};
					if(lin.length > 1) {
						auto numHeaderBytes = std::min(lin.length, static_cast<size_t>(3));
						linmsg->data.insert(linmsg->data.end(), (lin.header + 1), (lin.header + numHeaderBytes));
						linmsg->data.insert(linmsg->data.end(), lin.data, (lin.data + (lin.length - numHeaderBytes)));
						linmsg->checksum = linmsg->data.back();
						linmsg->data.pop_back();
					}
					if (linmsg->statusFlags.TxCommander) {
						if (linmsg->data.size())
							linmsg->linMsgType = icsneo::LINMessage::Type::LIN_COMMANDER_MSG;
						else
							linmsg->linMsgType = icsneo::LINMessage::Type::LIN_HEADER_ONLY;
					} else if (linmsg->statusFlags.TxResponder) {
						linmsg->linMsgType = icsneo::LINMessage::Type::LIN_UPDATE_RESPONDER;
					}
					return linmsg;
				}
				default: break;
			}
			break;
		}
		default: break;
	}
	return std::shared_ptr<Message>();

}