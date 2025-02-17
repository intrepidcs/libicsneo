#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/communication/message/macsecmessage.h"

namespace icsneo {

void init_macsecmessage(pybind11::module_ & m)
{
	pybind11::enum_<MACsecPacketType>(m, "MACsecPacketType")
		.value("NoVLAN_OrMPLS", MACsecPacketType::NoVLANOrMPLS)
		.value("SingleVLAN", MACsecPacketType::SingleVLAN)
		.value("DualVLAN", MACsecPacketType::DualVLAN)
		.value("MPLS", MACsecPacketType::MPLS)
		.value("SingleVLAN_FollowByMPLS", MACsecPacketType::SingleVLANFollowedByMPLS)
		.value("DualVLAN_FollowByMPLS", MACsecPacketType::DualVLANFollowedByMPLS)
		.value("Unsupported", MACsecPacketType::Unsupported);

	pybind11::enum_<MACsecValidateFrameType>(m, "MACsecValidateFrameType")
		.value("Disabled", MACsecValidateFrameType::Disabled)
		.value("Check", MACsecValidateFrameType::Check)
		.value("Strict", MACsecValidateFrameType::Strict)
		.value("NA", MACsecValidateFrameType::NA);

	pybind11::enum_<MACsecSecTagIcvStripType>(m, "MACsecSecTagIcvStripType")
		.value("StripBoth", MACsecSecTagIcvStripType::StripBoth)
		.value("StripSecTagPreserveICV", MACsecSecTagIcvStripType::StripSecTagPreserveICV)
		.value("PreserveSecTagStripICV", MACsecSecTagIcvStripType::PreserveSecTagStripICV)
		.value("PreserveBoth", MACsecSecTagIcvStripType::PreserveBoth);

	pybind11::enum_<MACsecCipherSuiteType>(m, "MACsecCipherSuiteType")
		.value("AES_128", MACsecCipherSuiteType::GcmAes128)
		.value("AES_256", MACsecCipherSuiteType::GcmAes256)
		.value("AES_128_XPN", MACsecCipherSuiteType::GcmAes128Xpn)
		.value("AES_256_XPN", MACsecCipherSuiteType::GcmAes256Xpn);

	pybind11::class_<MACsecVLANTag, std::shared_ptr<MACsecVLANTag>>(m, "MACsecVLANTag")
		.def(pybind11::init())
		.def_readwrite("vid", &MACsecVLANTag::vid)
		.def_readwrite("pri_cfi", &MACsecVLANTag::priCfi);

	pybind11::class_<MACsecMPLSOuter, std::shared_ptr<MACsecMPLSOuter>>(m, "MACsecMPLSOuter")
		.def(pybind11::init())
		.def_readwrite("mpls_label", &MACsecMPLSOuter::mplsLabel)
		.def_readwrite("exp", &MACsecMPLSOuter::exp);

	pybind11::class_<MACsecRule, std::shared_ptr<MACsecRule>>(m, "MACsecRule")
		.def(pybind11::init())
		.def_readwrite("index", &MACsecRule::index)
		.def_readwrite("keyMacDa", &MACsecRule::keyMacDa)
		.def_readwrite("maskMacDa", &MACsecRule::maskMacDa)
		.def_readwrite("keyMacSa", &MACsecRule::keyMacSa)
		.def_readwrite("maskMacSa", &MACsecRule::maskMacSa)
		.def_readwrite("keyEthertype", &MACsecRule::keyEthertype)
		.def_readwrite("maskEthertype", &MACsecRule::maskEthertype)
		.def_readwrite("keyVlanTagOuter1", &MACsecRule::keyVlanTagOuter1)
		.def_readwrite("keyMplsOuter1", &MACsecRule::keyMplsOuter1)
		.def_readwrite("maskVlanTagOuter1", &MACsecRule::maskVlanTagOuter1)
		.def_readwrite("maskMplsOuter1", &MACsecRule::maskMplsOuter1)
		.def_readwrite("keyVlanTagOuter2", &MACsecRule::keyVlanTagOuter2)
		.def_readwrite("keyMplsOuter2", &MACsecRule::keyMplsOuter2)
		.def_readwrite("maskVlanTagOuter2", &MACsecRule::maskVlanTagOuter2)
		.def_readwrite("maskMplsOuter2", &MACsecRule::maskMplsOuter2)
		.def_readwrite("keyBonusData", &MACsecRule::keyBonusData)
		.def_readwrite("maskBonusData", &MACsecRule::maskBonusData)
		.def_readwrite("keyTagMatchBitmap", &MACsecRule::keyTagMatchBitmap)
		.def_readwrite("maskTagMatchBitmap", &MACsecRule::maskTagMatchBitmap)
		.def_readwrite("keyPacketType", &MACsecRule::keyPacketType)
		.def_readwrite("maskPacketType", &MACsecRule::maskPacketType)
		.def_readwrite("keyInnerVlanType", &MACsecRule::keyInnerVlanType)
		.def_readwrite("maskInnerVlanType", &MACsecRule::maskInnerVlanType)
		.def_readwrite("keyOuterVlanType", &MACsecRule::keyOuterVlanType)
		.def_readwrite("maskOuterVlanType", &MACsecRule::maskOuterVlanType)
		.def_readwrite("keyNumTags", &MACsecRule::keyNumTags)
		.def_readwrite("maskNumTags", &MACsecRule::maskNumTags)
		.def_readwrite("keyExpress", &MACsecRule::keyExpress)
		.def_readwrite("maskExpress", &MACsecRule::maskExpress)
		.def_readwrite("isMpls", &MACsecRule::isMpls)
		.def_readwrite("enable", &MACsecRule::enable);

	pybind11::class_<MACsecMap, std::shared_ptr<MACsecMap>>(m, "MACsecMap")
		.def(pybind11::init())
		.def_readwrite("index", &MACsecMap::index)
		.def_readwrite("secTagSci", &MACsecMap::secTagSci)
		.def_readwrite("secYIndex", &MACsecMap::secYIndex)
		.def_readwrite("isControlPacket", &MACsecMap::isControlPacket)
		.def_readwrite("scIndex", &MACsecMap::scIndex)
		.def_readwrite("auxiliaryPlcy", &MACsecMap::auxiliaryPlcy)
		.def_readwrite("ruleId", &MACsecMap::ruleId)
		.def_readwrite("enable", &MACsecMap::enable);

	pybind11::class_<MACsecSecY, std::shared_ptr<MACsecSecY>>(m, "MACsecSecY")
		.def(pybind11::init())
		.def_readwrite("index", &MACsecSecY::index)
		.def_readwrite("controlledPortEnabled", &MACsecSecY::controlledPortEnabled)
		.def_readwrite("frameValidationType", &MACsecSecY::frameValidationType)
		.def_readwrite("secTagIcvStripType", &MACsecSecY::secTagIcvStripType)
		.def_readwrite("cipher", &MACsecSecY::cipher)
		.def_readwrite("confidentialOffset", &MACsecSecY::confidentialOffset)
		.def_readwrite("icvIncludesDaSa", &MACsecSecY::icvIncludesDaSa)
		.def_readwrite("replayProtect", &MACsecSecY::replayProtect)
		.def_readwrite("replayWindow", &MACsecSecY::replayWindow)
		.def_readwrite("protectFrames", &MACsecSecY::protectFrames)
		.def_readwrite("secTagOffset", &MACsecSecY::secTagOffset)
		.def_readwrite("secTagTci", &MACsecSecY::secTagTci)
		.def_readwrite("mtu", &MACsecSecY::mtu)
		.def_readwrite("enable", &MACsecSecY::enable);

	pybind11::class_<MACsecSc, std::shared_ptr<MACsecSc>>(m, "MACsecSc")
		.def(pybind11::init())
		.def_readwrite("index", &MACsecSc::index)
		.def_readwrite("secYIndex", &MACsecSc::secYIndex)
		.def_readwrite("sci", &MACsecSc::sci)
		.def_readwrite("saIndex0", &MACsecSc::saIndex0)
		.def_readwrite("saIndex1", &MACsecSc::saIndex1)
		.def_readwrite("saIndex0InUse", &MACsecSc::saIndex0InUse)
		.def_readwrite("saIndex1InUse", &MACsecSc::saIndex1InUse)
		.def_readwrite("enableAutoRekey", &MACsecSc::enableAutoRekey)
		.def_readwrite("isActiveSa1", &MACsecSc::isActiveSa1)
		.def_readwrite("enable", &MACsecSc::enable);

	pybind11::class_<MACsecSa, std::shared_ptr<MACsecSa>>(m, "MACsecSa")
		.def(pybind11::init())
		.def_readwrite("index", &MACsecSa::index)
		.def_readwrite("sak", &MACsecSa::sak)
		.def_readwrite("hashKey", &MACsecSa::hashKey)
		.def_readwrite("salt", &MACsecSa::salt)
		.def_readwrite("ssci", &MACsecSa::ssci)
		.def_readwrite("an", &MACsecSa::an)
		.def_readwrite("nextPn", &MACsecSa::nextPn)
		.def_readwrite("enable", &MACsecSa::enable);

	pybind11::class_<MACSecFlags, std::shared_ptr<MACSecFlags>>(m, "MACSecFlags")
		.def(pybind11::init())
		.def_readwrite("en", &MACSecFlags::en);

	pybind11::class_<MACsecConfig, std::shared_ptr<MACsecConfig>>(m, "MACsecConfig")
		.def(pybind11::init())
		.def_readwrite("flags", &MACsecConfig::flags)
		.def_readwrite("rule", &MACsecConfig::rule)
		.def_readwrite("map", &MACsecConfig::map)
		.def_readwrite("secy", &MACsecConfig::secy)
		.def_readwrite("sc", &MACsecConfig::sc)
		.def_readwrite("sa", &MACsecConfig::sa);

	pybind11::class_<MACSecGlobalFlags, std::shared_ptr<MACSecGlobalFlags>>(m, "MACSecGlobalFlags")
		.def(pybind11::init())
		.def_readwrite("en", &MACSecGlobalFlags::en)
		.def_readwrite("nvm", &MACSecGlobalFlags::nvm);

	pybind11::class_<MACsecMessage, std::shared_ptr<MACsecMessage>>(m, "MACsecMessage")
		.def(pybind11::init())
		.def_readwrite("flags", &MACsecMessage::flags)
		.def_readwrite("rx", &MACsecMessage::rx)
		.def_readwrite("tx", &MACsecMessage::tx);
}

} // namespace icsneo 

