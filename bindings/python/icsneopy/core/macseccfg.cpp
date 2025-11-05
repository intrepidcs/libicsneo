#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "icsneo/core/macseccfg.h"

namespace icsneo {

void init_macsecconfig(pybind11::module_ & m)
{
	pybind11::enum_<MACsecPacketType>(m, "MACsecPacketType")
		.value("DEFAULT", MACsecPacketType::Default)
		.value("SINGLE_VLAN", MACsecPacketType::SingleVLAN)
		.value("DUAL_VLAN", MACsecPacketType::DualVLAN)
		.value("MPLS", MACsecPacketType::MPLS)
		.value("SINGLE_VLAN_FOLLOWED_BY_MPLS", MACsecPacketType::SingleVLANFollowedByMPLS)
		.value("DUAL_VLAN_FOLLOWED_BY_MPLS", MACsecPacketType::DualVLANFollowedByMPLS)
		.value("UNSUPPORTED", MACsecPacketType::Unsupported);

	pybind11::enum_<MACsecValidation>(m, "MACsecValidation")
		.value("DISABLED", MACsecValidation::Disabled)
		.value("CHECK", MACsecValidation::Check)
		.value("STRICT", MACsecValidation::Strict)
		.value("NA", MACsecValidation::NA);

	pybind11::enum_<MACsecStrip>(m, "MACsecStrip")
		.value("STRIP_SECTAG_AND_ICV", MACsecStrip::StripSecTagAndIcv)
		.value("STRIP_SECTAG_PRESERVE_ICV", MACsecStrip::StripSecTagPreserveICV)
		.value("PRESERVE_SECTAG_STRIP_ICV", MACsecStrip::PreserveSecTagStripICV)
		.value("NO_STRIP", MACsecStrip::NoStrip);

	pybind11::enum_<MACsecCipherSuite>(m, "MACsecCipherSuite")
		.value("GCM_AES_128", MACsecCipherSuite::GcmAes128)
		.value("GCM_AES_256", MACsecCipherSuite::GcmAes256)
		.value("GCM_AES_128_XPN", MACsecCipherSuite::GcmAes128Xpn)
		.value("GCM_AES_256_XPN", MACsecCipherSuite::GcmAes256Xpn);

	pybind11::class_<MACsecVLANTag, std::shared_ptr<MACsecVLANTag>>(m, "MACsecVLANTag")
		.def(pybind11::init())
		.def_readwrite("vid", &MACsecVLANTag::vid)
		.def_readwrite("pri_cfi", &MACsecVLANTag::priCfi);

	pybind11::class_<MACsecMPLSOuter, std::shared_ptr<MACsecMPLSOuter>>(m, "MACsecMPLSOuter")
		.def(pybind11::init())
		.def_readwrite("mpls_label", &MACsecMPLSOuter::mplsLabel)
		.def_readwrite("exp", &MACsecMPLSOuter::exp);

	pybind11::class_<MACsecTci, std::shared_ptr<MACsecTci>>(m, "MACsecTci")
		.def(pybind11::init())
		.def_readwrite("es", &MACsecTci::es)
		.def_readwrite("sc", &MACsecTci::sc)
		.def_readwrite("scb", &MACsecTci::scb)
		.def_readwrite("e", &MACsecTci::e)
		.def_readwrite("c", &MACsecTci::c);

	pybind11::class_<MACsecRxRule, std::shared_ptr<MACsecRxRule>>(m, "MACsecRxRule")
		.def(pybind11::init())
		.def_readwrite("key_mac_da", &MACsecRxRule::keyMacDa)
		.def_readwrite("mask_mac_da", &MACsecRxRule::maskMacDa)
		.def_readwrite("key_mask_sa", &MACsecRxRule::keyMacSa)
		.def_readwrite("mask_mac_sa", &MACsecRxRule::maskMacSa)
		.def_readwrite("key_ether_type", &MACsecRxRule::keyEthertype)
		.def_readwrite("mask_ether_type", &MACsecRxRule::maskEthertype)
		.def_readwrite("key_vlan_tag_outer1", &MACsecRxRule::keyVlanTagOuter1)
		.def_readwrite("key_mpls_outer1", &MACsecRxRule::keyMplsOuter1)
		.def_readwrite("mask_vlan_tag_outer1", &MACsecRxRule::maskVlanTagOuter1)
		.def_readwrite("mask_mpls_outer1", &MACsecRxRule::maskMplsOuter1)
		.def_readwrite("key_vlan_tag_outer2", &MACsecRxRule::keyVlanTagOuter2)
		.def_readwrite("key_mpls_outer2", &MACsecRxRule::keyMplsOuter2)
		.def_readwrite("mask_vlan_tag_outer2", &MACsecRxRule::maskVlanTagOuter2)
		.def_readwrite("mask_mpls_outer2", &MACsecRxRule::maskMplsOuter2)
		.def_readwrite("key_bonus_data", &MACsecRxRule::keyBonusData)
		.def_readwrite("mask_bonus_data", &MACsecRxRule::maskBonusData)
		.def_readwrite("key_tag_match_bitmap", &MACsecRxRule::keyTagMatchBitmap)
		.def_readwrite("mask_tag_match_bitmap", &MACsecRxRule::maskTagMatchBitmap)
		.def_readwrite("key_packet_type", &MACsecRxRule::keyPacketType)
		.def_readwrite("mask_packet_type", &MACsecRxRule::maskPacketType)
		.def_readwrite("key_inner_vlan_type", &MACsecRxRule::keyInnerVlanType)
		.def_readwrite("mask_inner_vlan_type", &MACsecRxRule::maskInnerVlanType)
		.def_readwrite("key_outer_vlan_type", &MACsecRxRule::keyOuterVlanType)
		.def_readwrite("mask_outer_vlan_type", &MACsecRxRule::maskOuterVlanType)
		.def_readwrite("key_num_tags", &MACsecRxRule::keyNumTags)
		.def_readwrite("mask_num_tags", &MACsecRxRule::maskNumTags)
		.def_readwrite("key_express", &MACsecRxRule::keyExpress)
		.def_readwrite("mask_express", &MACsecRxRule::maskExpress)
		.def_readwrite("is_mpls", &MACsecRxRule::isMpls);

	pybind11::class_<MACsecTxSecY, std::shared_ptr<MACsecTxSecY>>(m, "MACsecTxSecY")
		.def(pybind11::init())
		.def_readwrite("enable_control_port", &MACsecTxSecY::enableControlPort)
		.def_readwrite("cipher", &MACsecTxSecY::cipher)
		.def_readwrite("confidentiality_offset", &MACsecTxSecY::confidentialityOffset)
		.def_readwrite("icv_includes_da_sa", &MACsecTxSecY::icvIncludesDaSa)
		.def_readwrite("protect_frames", &MACsecTxSecY::protectFrames)
		.def_readwrite("sec_tag_offset", &MACsecTxSecY::secTagOffset)
		.def_readwrite("sec_tag_tci", &MACsecTxSecY::tci)
		.def_readwrite("mtu", &MACsecTxSecY::mtu)
		.def_readwrite("is_control_packet", &MACsecTxSecY::isControlPacket)
		.def_readwrite("auxiliary_policy", &MACsecTxSecY::auxiliaryPolicy)
		.def_readwrite("sci", &MACsecTxSecY::sci);
		
	pybind11::class_<MACsecRxSecY, std::shared_ptr<MACsecRxSecY>>(m, "MACsecRxSecY")
		.def(pybind11::init())
		.def_readwrite("enable_control_port", &MACsecRxSecY::enableControlPort)
		.def_readwrite("frame_validation", &MACsecRxSecY::frameValidation)
		.def_readwrite("frame_strip", &MACsecRxSecY::frameStrip)
		.def_readwrite("cipher", &MACsecRxSecY::cipher)
		.def_readwrite("confidentiality_offset", &MACsecRxSecY::confidentialityOffset)
		.def_readwrite("icv_includes_da_sa", &MACsecRxSecY::icvIncludesDaSa)
		.def_readwrite("replay_protect", &MACsecRxSecY::replayProtect)
		.def_readwrite("replay_window", &MACsecRxSecY::replayWindow)
		.def_readwrite("is_control_packet", &MACsecRxSecY::isControlPacket)
		.def_readwrite("sci", &MACsecRxSecY::sci);

	pybind11::class_<MACsecTxSa, std::shared_ptr<MACsecTxSa>>(m, "MACsecTxSa")
		.def(pybind11::init())
		.def_readwrite("sak", &MACsecTxSa::sak)
		.def_readwrite("hash_key", &MACsecTxSa::hashKey)
		.def_readwrite("salt", &MACsecTxSa::salt)
		.def_readwrite("ssci", &MACsecTxSa::ssci)
		.def_readwrite("next_pn", &MACsecTxSa::nextPn)
		.def_readwrite("an", &MACsecTxSa::an);

	pybind11::class_<MACsecRxSa, std::shared_ptr<MACsecRxSa>>(m, "MACsecRxSa")
		.def(pybind11::init())
		.def_readwrite("sak", &MACsecRxSa::sak)
		.def_readwrite("hash_key", &MACsecRxSa::hashKey)
		.def_readwrite("salt", &MACsecRxSa::salt)
		.def_readwrite("ssci", &MACsecRxSa::ssci)
		.def_readwrite("next_pn", &MACsecRxSa::nextPn);

	pybind11::class_<MACsecConfig, std::shared_ptr<MACsecConfig>>(m, "MACsecConfig")
		.def(pybind11::init<icsneo::DeviceType>())
		.def("add_rx_secy", &MACsecConfig::addRxSecY, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("add_tx_secY", &MACsecConfig::addTxSecY, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("add_rx_rule", &MACsecConfig::addRxRule, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("add_rx_sa", &MACsecConfig::addRxSa, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("add_tx_sa", &MACsecConfig::addTxSa, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_rx_secy", [](MACsecConfig& cfg, uint8_t index) -> MACsecRxSecY& { return cfg.getRxSecY(index); }, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_tx_secy", [](MACsecConfig& cfg, uint8_t index) -> MACsecTxSecY& { return cfg.getTxSecY(index); }, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_rx_sa", [](MACsecConfig& cfg, uint8_t index) -> MACsecRxSa& { return cfg.getRxSa(index); }, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_tx_sa", [](MACsecConfig& cfg, uint8_t index) -> MACsecTxSa& { return cfg.getTxSa(index); }, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_rx_rule", [](MACsecConfig& cfg, uint8_t index) -> MACsecRxRule& { return cfg.getRxRule(index); }, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("set_tx_sa_index", &MACsecConfig::setTxSaIndex, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("enable_tx_rekey", &MACsecConfig::enableTxRekey, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("set_tx_sa_rekey_index", &MACsecConfig::setTxSaRekeyIndex, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("disable_tx_rekey", &MACsecConfig::disableTxRekey, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("set_rx_sa_index", &MACsecConfig::setRxSaIndex, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("enable_rx_rekey", &MACsecConfig::enableRxRekey, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("set_rx_sa_rekey_index", &MACsecConfig::setRxSaRekeyIndex, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("disable_rx_rekey", &MACsecConfig::disableRxRekey, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("set_rx_enable", &MACsecConfig::setRxEnable, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("set_tx_enable", &MACsecConfig::setTxEnable, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("set_storage", &MACsecConfig::setStorage, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("clear", &MACsecConfig::clear, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("serialize", &MACsecConfig::serialize, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_bin_index", &MACsecConfig::getBinIndex, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_type", &MACsecConfig::getType, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_max_num_rule", &MACsecConfig::getMaxNumRule, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_max_num_secy", &MACsecConfig::getMaxNumSecY, pybind11::call_guard<pybind11::gil_scoped_release>())
		.def("get_max_num_sa", &MACsecConfig::getMaxNumSa, pybind11::call_guard<pybind11::gil_scoped_release>());

}

} // namespace icsneo 

