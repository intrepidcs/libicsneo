#include "icsneo/core/macseccfg.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/core/crc32.h"
#include <cstring>

namespace icsneo {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // nameless struct/union
#endif

#pragma pack(push, 1)

typedef struct
{
	uint16_t VID; /*!< 12 bits */
	uint8_t PRI_CFI; /*!< PRI - 3 bits, CFI - 1bit */
} MACSEC_VLANTAG_t;
/**
 * @brief Structure of MPLS
 *
 */
typedef struct
{
	uint32_t MPLS_label; /*!< 20 bits */
	uint8_t exp; /*!< 3 bits */
} MACSEC_MPLS_OUTER_t;

#define MACSEC_SETTINGS_RULE_SIZE (88)
typedef union _MACSecRule
{
	struct
	{
		uint8_t index;
		uint8_t key_MAC_DA[6]; /*!< MAC DA field extracted from the packet */
		uint8_t mask_MAC_DA[6]; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint8_t key_MAC_SA[6]; /*!< MAC SA field extracted from the packet */
		uint8_t mask_MAC_SA[6]; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint16_t key_Ethertype; /*!< First E-Type found in the packet that doesn't match one of the preconfigured custom tag. */
		uint16_t mask_Ethertype; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		MACSEC_VLANTAG_t key_vlantag_outer1; /*!< outermost/1st VLAN ID {8'd0, VLAN_ID[11:0]}, or 20-bit MPLS label. */
		MACSEC_MPLS_OUTER_t key_MPLS_outer1;
		MACSEC_VLANTAG_t mask_vlantag_outer1; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		MACSEC_MPLS_OUTER_t mask_MPLS_outer1;
		MACSEC_VLANTAG_t key_vlantag_outer2; /*!< 2nd outermost VLAN ID {8'd0, VLAN_ID[11:0]}, or 20-bit MPLS label. */
		MACSEC_MPLS_OUTER_t key_MPLS_outer2;
		MACSEC_VLANTAG_t mask_vlantag_outer2; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		MACSEC_MPLS_OUTER_t mask_MPLS_outer2;
		uint16_t key_bonus_data; /*!< 2 bytes of additional bonus data extracted from one of the custom tags. */
		uint16_t mask_bonus_data; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint8_t
			key_tag_match_bitmap; /*!< 8 bits total. Maps 1 to 1 bitwise with the set of custom tags. (set bit[N]=1 if check Nth custom tag) */
		uint8_t mask_tag_match_bitmap; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		MACsecPacketType key_packet_type; /*!< Encoded Packet Type, see MACSEC_PACKET_TYPE */
		uint8_t mask_packet_type; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint16_t
			key_inner_vlan_type; /*!< 3 bits total. Encoded value indicating which VLAN TPID value matched for the second outermost VLAN Tag. */
		uint16_t mask_inner_vlan_type; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint16_t key_outer_vlan_type; /*!< 3 bits total. Encoded value indicating which VLAN TPID value matched for the outermost VLAN Tag. */
		uint16_t mask_outer_vlan_type; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint8_t
			key_num_tags; /*!< 7 bits total. Number of VLAN/custom tags or MPLS lables detected. Ingress: before SecTag; Egress: total detected. Exclude MCS header tags. i.e. Bit 2: 2 tags/labels before SecTAG...Bit 6: 6 or more tags/labels before SecTAG. */
		uint8_t mask_num_tags; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint8_t key_express; /*!< 1 bits. Express packet. */
		uint8_t mask_express; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint8_t isMPLS;
		uint8_t rsvd[5];
		uint8_t enable;
	};
	uint8_t byte[MACSEC_SETTINGS_RULE_SIZE];
} MACSecRule_t;
/* MACsec Map */
#define MACSEC_SETTINGS_MAP_SIZE (20)
typedef union _MACSecMap
{
	struct
	{
		uint8_t index;
		uint64_t sectag_sci; /*!< Identifies the SecTAG SCI for this Flow. */
		uint8_t secYIndex; /*!< index for entry in Egress secY Policy */
		uint8_t isControlPacket; /*!< Identifies all packets matching this index lookup as control packets. */
		uint8_t scIndex; /*!< Identifies the SC for this Flow. */
		uint8_t auxiliary_plcy; /*!< Auxiliary policy bits. */
		uint8_t ruleId; /*!< Identifies the Rule for this Flow. */
		uint8_t rsvd[5];
		uint8_t enable;
	};
	uint8_t byte[MACSEC_SETTINGS_MAP_SIZE];
} MACSecMap_t;
/* MACsec SecY */

#define MACSEC_SETTINGS_SECY_SIZE (24)
typedef union _MACSecSecY
{
	struct
	{
		uint8_t index; /*!< Identifies the SecY for this Flow. */
		uint8_t controlled_port_enabled; /*!< Enable (or disable) operation of the Controlled port associated with this SecY */
		MACsecValidation validate_frames; /*!< see MACSEC_VALIDATEFRAME */
		MACsecStrip strip_sectag_icv; /*!< see MACSEC_STRIP_SECTAG_ICV */
		MACsecCipherSuite cipher; /*!< Define the cipher suite to use for this SecY */
		uint8_t confidential_offset; /*!< Define the number of bytes that are unencrypted following the SecTag. */
		uint8_t icv_includes_da_sa; /*!< When set, the outer DA/SA bytes are included in the authentication GHASH calculation */
		uint8_t replay_protect; /*!< Enables Anti-Replay protection */
		uint32_t replay_window; /*!< Unsigned value indicating the size of the anti-replay window. */
		uint8_t
			protect_frames; /*!< 0 = do not encrypt or authenticate this packet; 1 = always Authenticate frame and if SecTag.TCI.E = 1 encrypt the packet as well. */
		uint8_t
			sectag_offset; /*!< Define the offset in bytes from either the start of the packet or a matching Etype depending on SecTag_Insertion_Mode. */
		uint8_t sectag_tci; /*!< Tag Control Information excluding the AN field which originates from the SA Policy table */
		uint16_t mtu; /*!< Specifies the outgoing MTU for this SecY */
		uint8_t rsvd[6];
		uint8_t enable;
	};
	uint8_t byte[MACSEC_SETTINGS_SECY_SIZE];
} MACSecSecY_t;
/* MACsec SC */
#define MACSEC_SETTINGS_SC_SIZE (24)
typedef union _MACSecSc
{
	struct
	{
		uint8_t index; /*!< SC index. */
		uint8_t secYIndex; /*!< SecY associated with this packet. */
		uint64_t sci; /*!< The Secure Channel Identifier. */
		uint8_t sa_index0; /*!< Define the 1st SA to use */
		uint8_t sa_index1; /*!< Define the 2nd SA to use */
		uint8_t sa_index0_in_use; /*!< Specifies whether 1st SA is in use or not. */
		uint8_t sa_index1_in_use; /*!< Specifies whether 2nd SA is in use or not.  */
		uint8_t enable_auto_rekey; /*!< If enabled, then once the pn_threshold is reached, auto rekey will happen. */
		uint8_t
			isActiveSA1; /*!< If set, then sa_index1 is the currently active SA index. If cleared, the sa_index0 is the currently active SA index). */
		uint8_t rsvd[7];
		uint8_t enable;
	};
	uint8_t byte[MACSEC_SETTINGS_SC_SIZE];
} MACSecSc_t;
/* MACsec SA */
#define MACSEC_SETTINGS_SA_SIZE (80)
typedef union _MACSecSa
{
	struct
	{
		uint8_t index; /*!< SA index */
		uint8_t
			sak[32]; /*!< 256b SAK: Define the encryption key to be used to encrypte this packet. The lower 128 bits are used for 128-bit ciphers. */
		uint8_t hashKey[16]; /*!< 128b Hash Key: Key used for authentication. */
		uint8_t salt[12]; /*!< 96b Salt value: Salt value used in XPN ciphers. */
		uint32_t ssci; /*!< 32b SSCI value: Short Secure Channel Identifier, used in XPN ciphers. */
		uint8_t AN; /*!< 2b SecTag Association Number (AN) */
		uint64_t nextPN; /*!< 64b next_pn value: Next packet number to insert into outgoing packet on a particular SA. */
		uint8_t rsvd[5];
		uint8_t enable;
	};
	uint8_t byte[MACSEC_SETTINGS_SA_SIZE];
} MACSecSa_t;
/* MACsec Flags */
#define MACSEC_SETTINGS_FLAGS_SIZE (4)
typedef union _MACSecFlags
{
	struct
	{
		uint32_t en : 1; // '1' = enable; '0' = disable
		uint32_t reserved : 31;
	};
	uint32_t flags_32b;
} MACSecFlags_t;
/* MACSec Settings for 1 port/phy */
#define MACSEC_NUM_FLAGS_PER_CONFIG (1)
#define MACSEC_NUM_RULES_PER_CONFIG (2)
#define MACSEC_NUM_MAPS_PER_CONFIG (2)
#define MACSEC_NUM_SECY_PER_CONFIG (2)
#define MACSEC_NUM_SC_PER_CONFIG (2)
#define MACSEC_NUM_SA_PER_CONFIG (4)
typedef struct MACSEC_CONFIG_t
{
	MACSecFlags_t flags;
	MACSecRule_t rule[MACSEC_NUM_RULES_PER_CONFIG];
	MACSecMap_t map[MACSEC_NUM_MAPS_PER_CONFIG];
	MACSecSecY_t secy[MACSEC_NUM_SECY_PER_CONFIG];
	MACSecSc_t sc[MACSEC_NUM_SC_PER_CONFIG];
	MACSecSa_t sa[MACSEC_NUM_SA_PER_CONFIG];
} MACSEC_CONFIG;
typedef union _MACSecGlobalFlags
{
	struct
	{
		uint32_t en : 1; // '1' = enable; '0' = disable
		uint32_t nvm : 1; // store macsec config in non-volatile memory
		uint32_t reserved : 30;
	};
	uint32_t flags_32b;
} MACSecGlobalFlags_t;
#define MACSEC_SETTINGS_SIZE (2040) // leave space for expansion and keep nicely aligned for flashing
typedef union _MACSEC_SETTINGS
{
	struct
	{
		MACSecGlobalFlags_t flags;
		MACSEC_CONFIG rx;
		MACSEC_CONFIG tx;
	};
	uint8_t byte[MACSEC_SETTINGS_SIZE];
} MACSEC_SETTINGS;

#define MACSEC_SETTINGS_VERSION 1
typedef struct MACSEC_SETTINGS_W_HDR
{
	uint16_t version;
	uint16_t len;
	uint32_t crc32;
	MACSEC_SETTINGS macsec;
} MACSEC_SETTINGS_W_HDR;
#define MACSEC_SETTINGS_W_HDR_SIZE (2048)

#pragma pack(pop)

#ifdef _MSC_VER
#pragma warning(pop)
#endif

static void ReportEvent(APIEvent::Type event, APIEvent::Severity severity) {
	auto& em = EventManager::GetInstance();
	em.add(APIEvent(event, severity));
}

MACsecConfig::MACsecConfig(const DeviceType& deviceType) : type(deviceType) {
	switch(deviceType.getDeviceType()) {
		case icsneo::DeviceType::Enum::RADMoon2:
		case icsneo::DeviceType::Enum::RADMoon3:
		case icsneo::DeviceType::Enum::RADEpsilon:
			maxSecY = 2;
			maxRule = 2;
			maxSa = 4;
			binIndex = 0;
			break;
		default:
			maxSecY = 0;
			maxSa = 0;
			maxRule = 0;
			binIndex = 0;
			ReportEvent(APIEvent::Type::MACsecNotSupported, APIEvent::Severity::Error);
			return;
	}
}

int MACsecConfig::addRxSecY(const MACsecRxSecY& secY, uint8_t saIndex) {
	if(rxSecY.size() >= maxSecY) {
		ReportEvent(APIEvent::Type::MACsecSecYLimit, APIEvent::Severity::Error);
		return -1;
	}

	if(saIndex >= rxSa.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSaIndex, APIEvent::Severity::Error);
		return -1;
	}

	int ret = static_cast<int>(rxSecY.size());
	rxSecY.emplace_back(secY);
	rxSecYSaIndices.emplace_back(saIndex, saIndex + 1);
	rxSecYRekey.emplace_back(false);
	rxRuleIndices.emplace_back(0);

	return ret;
}

int MACsecConfig::addTxSecY(const MACsecTxSecY& secY, uint8_t saIndex) {
	if(txSecY.size() >= maxSecY) {
		ReportEvent(APIEvent::Type::MACsecSecYLimit, APIEvent::Severity::Error);
		return -1;
	}

	if(saIndex >= txSa.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSaIndex, APIEvent::Severity::Error);
		return -1;
	}

	int ret = static_cast<int>(txSecY.size());
	txSecY.emplace_back(secY);
	txSecYSaIndices.emplace_back(saIndex, saIndex + 1);
	txSecYRekey.emplace_back(false);

	return ret;
}

int MACsecConfig::addRxSa(const MACsecRxSa& sa) {
	if(rxSa.size() >= maxSa) {
		ReportEvent(APIEvent::Type::MACsecSaLimit, APIEvent::Severity::Error);
		return -1;
	}

	int ret = static_cast<int>(rxSa.size());
	rxSa.emplace_back(sa);
	return ret;
}

int MACsecConfig::addTxSa(const MACsecTxSa& sa) {
	if(txSa.size() >= maxSa) {
		ReportEvent(APIEvent::Type::MACsecSaLimit, APIEvent::Severity::Error);
		return -1;
	}

	int ret = static_cast<int>(txSa.size());
	txSa.emplace_back(sa);
	return ret;
}

int MACsecConfig::addRxRule(const MACsecRxRule& rule, uint8_t secYIndex) {
	if(rxRule.size() >= maxRule) {
		ReportEvent(APIEvent::Type::MACsecSaLimit, APIEvent::Severity::Error);
		return -1;
	}

	if(maxSecY >= rxRule.size()) {
		ReportEvent(APIEvent::Type::MACsecSecYLimit, APIEvent::Severity::Error);
		return -1;
	}

	int ret = static_cast<int>(rxRule.size());
	rxRuleIndices[secYIndex] = static_cast<uint8_t>(rxRule.size());
	rxRule.emplace_back(rule);
	return ret;
}

MACsecRxRule& MACsecConfig::getRxRule(uint8_t ruleIndex) {
	return rxRule[ruleIndex];
}

const MACsecRxRule& MACsecConfig::getRxRule(uint8_t ruleIndex) const {
	return rxRule[ruleIndex];
}

MACsecRxSecY& MACsecConfig::getRxSecY(uint8_t secYIndex) {
	return rxSecY[secYIndex];
}

const MACsecRxSecY& MACsecConfig::getRxSecY(uint8_t secYIndex) const {
	return rxSecY[secYIndex];
}

MACsecTxSecY& MACsecConfig::getTxSecY(uint8_t secYIndex) {
	return txSecY[secYIndex];
}

const MACsecTxSecY& MACsecConfig::getTxSecY(uint8_t secYIndex) const {
	return txSecY[secYIndex];	
}

MACsecRxSa& MACsecConfig::getRxSa(uint8_t saIndex) {
	return rxSa[saIndex];
}

const MACsecRxSa& MACsecConfig::getRxSa(uint8_t saIndex) const {
	return rxSa[saIndex];
}

MACsecTxSa& MACsecConfig::getTxSa(uint8_t saIndex) {
	return txSa[saIndex];
}

const MACsecTxSa& MACsecConfig::getTxSa(uint8_t saIndex) const {
	return txSa[saIndex];
}

bool MACsecConfig::setTxSaIndex(uint8_t secYIndex, uint8_t saIndex) {
	if(secYIndex >= txSecY.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSecYIndex, APIEvent::Severity::Error);
		return false;
	}
	if(saIndex >= txSa.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSaIndex, APIEvent::Severity::Error);
		return false;
	}

	txSecYSaIndices[secYIndex].first = saIndex;
	return true;
}

bool MACsecConfig::enableTxRekey(uint8_t secYIndex, uint8_t rekeySaIndex) {
	if(secYIndex >= txSecY.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSecYIndex, APIEvent::Severity::Error);
		return false;
	}
	if(rekeySaIndex >= txSa.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSaIndex, APIEvent::Severity::Error);
		return false;
	}

	txSecYSaIndices[secYIndex].second = rekeySaIndex;
	txSecYRekey[secYIndex] = true;
	return true;
}

bool MACsecConfig::setTxSaRekeyIndex(uint8_t secYIndex, uint8_t saIndex) {
	if(secYIndex >= txSecY.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSecYIndex, APIEvent::Severity::Error);
		return false;
	}
	if(saIndex >= txSa.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSaIndex, APIEvent::Severity::Error);
		return false;
	}

	txSecYSaIndices[secYIndex].second = saIndex;
	return true;
}

void MACsecConfig::disableTxRekey(uint8_t secYIndex) {
	if(secYIndex >= txSecY.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSecYIndex, APIEvent::Severity::EventWarning);
		return;
	}
	txSecYRekey[secYIndex] = false;
}

bool MACsecConfig::setRxSaIndex(uint8_t secYIndex, uint8_t saIndex) {
	if(secYIndex >= rxSecY.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSecYIndex, APIEvent::Severity::Error);
		return false;
	}
	if(saIndex >= rxSa.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSaIndex, APIEvent::Severity::Error);
		return false;
	}

	rxSecYSaIndices[secYIndex].first = saIndex;
	return true;
}

bool MACsecConfig::enableRxRekey(uint8_t secYIndex, uint8_t rekeySaIndex) {
	if(secYIndex >= rxSecY.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSecYIndex, APIEvent::Severity::Error);
		return false;
	}
	if(rekeySaIndex >= rxSa.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSaIndex, APIEvent::Severity::Error);
		return false;
	}

	rxSecYSaIndices[secYIndex].second = rekeySaIndex;
	rxSecYRekey[secYIndex] = true;
	return true;
}

bool MACsecConfig::setRxSaRekeyIndex(uint8_t secYIndex, uint8_t saIndex) {
	if(secYIndex >= rxSecY.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSecYIndex, APIEvent::Severity::Error);
		return false;
	}
	if(saIndex >= rxSa.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSaIndex, APIEvent::Severity::Error);
		return false;
	}

	rxSecYSaIndices[secYIndex].second = saIndex;
	return true;
}

void MACsecConfig::disableRxRekey(uint8_t secYIndex) {
	if(secYIndex >= rxSecY.size()) {
		ReportEvent(APIEvent::Type::MACsecInvalidSecYIndex, APIEvent::Severity::EventWarning);
		return;
	}
	rxSecYRekey[secYIndex] = false;
}

void MACsecConfig::setRxEnable(bool newRxEnable) {
	enableRx = newRxEnable;
}

void MACsecConfig::setTxEnable(bool newTxEnable) {
	enableTx = newTxEnable;
}

void MACsecConfig::setStorage(bool temporary) {
	nvm = !temporary;
}


void MACsecConfig::clear() {
	// Set everything back to default, except device maximums
	rxSecY.clear();
	txSecY.clear();
	rxSa.clear();
	txSa.clear();
	rxRule.clear();
	rxSecYRekey.clear();
	txSecYRekey.clear();
	rxSecYSaIndices.clear();
	txSecYSaIndices.clear();
	rxRuleIndices.clear();
	enableRx = false;
	enableTx = false;
	nvm = false;
}

MACsecConfig::operator bool() const {
	return (maxSa != 0) || (maxSecY != 0) || (maxRule != 0);
}

uint16_t MACsecConfig::getBinIndex() const {
	return binIndex;
}

DeviceType MACsecConfig::getType() const {
	return type;
}

uint8_t MACsecConfig::getMaxNumRule() const {
	return maxRule;	
}

uint8_t MACsecConfig::getMaxNumSecY() const {
	return maxSecY;
}

uint8_t MACsecConfig::getMaxNumSa() const {
	return maxSa;
}


static uint8_t TCItoInt(const MACsecTci& tci) {
	uint8_t res = 0;
	res |= tci.c ? 0x01u : 0;
	res |= tci.e ? 0x02u : 0;
	res |= tci.scb ? 0x04u : 0;
	res |= tci.sc ? 0x08u : 0;
	res |= tci.es ? 0x10u : 0;
	return res;
}

static void SetHardwareTxSecY(
	MACSEC_SETTINGS_W_HDR* hwSettings,
	const MACsecTxSecY& secY,
	bool rekeyEnabled,
	const std::pair<uint8_t, uint8_t>& saIndices,
	uint8_t index
) {

	MACSecSecY_t* hwSecY = &hwSettings->macsec.tx.secy[index];
	MACSecSc_t* hwSc = &hwSettings->macsec.tx.sc[index];
	MACSecMap_t* hwMap = &hwSettings->macsec.tx.map[index]; 

	hwSecY->index = index;
	hwSecY->enable = true;
	hwSecY->controlled_port_enabled = secY.enableControlPort ? 0x1u : 0x0u;
	hwSecY->cipher = secY.cipher;
	hwSecY->confidential_offset = secY.confidentialityOffset;
	hwSecY->icv_includes_da_sa = secY.icvIncludesDaSa ? 0x1u : 0x0u;
	hwSecY->mtu = secY.mtu;
	hwSecY->sectag_tci = TCItoInt(secY.tci);
	hwSecY->sectag_offset = secY.secTagOffset;
	hwSecY->protect_frames = secY.protectFrames;

	hwSc->index = index;
	hwSc->enable = true;
	hwSc->secYIndex = index;
	hwSc->enable_auto_rekey = rekeyEnabled ? 0x1u : 0x0u;
	hwSc->sa_index0 = saIndices.first;
	hwSc->sa_index1 = saIndices.second;
	hwSc->sa_index0_in_use = true;
	hwSc->sa_index1_in_use = rekeyEnabled ? true : false;
	hwSc->isActiveSA1 = rekeyEnabled ? true : false;
	hwSc->sci = secY.sci;

	hwMap->index = index;
	hwMap->enable = true;
	hwMap->auxiliary_plcy = secY.auxiliaryPolicy;
	hwMap->secYIndex = index;
	hwMap->isControlPacket = secY.isControlPacket ? 0x1u : 0x0u;
	hwMap->scIndex = index;
	hwMap->sectag_sci = secY.sci;

}

static void SetHardwareRxSecY(
	MACSEC_SETTINGS_W_HDR* hwSettings,
	const MACsecRxSecY& secY,
	bool rekeyEnabled,
	uint8_t ruleIndex,
	const std::pair<uint8_t, uint8_t>& saIndices,
	uint8_t index
) {

	MACSecSecY_t* hwSecY = &hwSettings->macsec.rx.secy[index];
	MACSecSc_t* hwSc = &hwSettings->macsec.rx.sc[index];
	MACSecMap_t* hwMap = &hwSettings->macsec.rx.map[index]; 

	hwSecY->index = index;
	hwSecY->enable = 0x1u;
	hwSecY->controlled_port_enabled = secY.enableControlPort ? 0x1u : 0x0u;
	hwSecY->cipher = secY.cipher;
	hwSecY->confidential_offset = secY.confidentialityOffset;
	hwSecY->icv_includes_da_sa = secY.icvIncludesDaSa ? 0x1u : 0x0u;
	hwSecY->replay_protect = secY.replayProtect ? 0x1u : 0x0u;
	hwSecY->replay_window = secY.replayWindow;
	hwSecY->validate_frames = secY.frameValidation;
	hwSecY->strip_sectag_icv = secY.frameStrip;
	hwSecY->sectag_offset = 12;

	hwSc->index = index;
	hwSc->enable = 0x1u;
	hwSc->secYIndex = index;
	hwSc->enable_auto_rekey = rekeyEnabled ? 0x1u : 0x0u;
	hwSc->sa_index0 = saIndices.first;
	hwSc->sa_index1 = saIndices.second;
	hwSc->sa_index0_in_use = 0x1u;
	hwSc->sa_index1_in_use = rekeyEnabled ? 0x1u : 0x0u;
	hwSc->isActiveSA1 = rekeyEnabled ? 0x1u : 0x0u;
	hwSc->sci = secY.sci;

	hwMap->index = index;
	hwMap->enable = 0x1u;
	hwMap->secYIndex = index;
	hwMap->ruleId = ruleIndex;
	hwMap->isControlPacket = secY.isControlPacket ? 0x1u : 0x0u;
	hwMap->scIndex = index;
	hwMap->sectag_sci = secY.sci;

}

static void SetHardwareTxSa(MACSEC_SETTINGS_W_HDR* hwSettings, const MACsecTxSa& sa, uint8_t index) {
	MACSecSa_t* hwSa = &hwSettings->macsec.tx.sa[index];

	hwSa->index = index;
	hwSa->enable = 0x1u;
	memcpy(hwSa->sak, sa.sak.data(), 32);
	memcpy(hwSa->hashKey, sa.hashKey.data(), 16);
	memcpy(hwSa->salt, sa.salt.data(), 12);
	hwSa->ssci = sa.ssci;
	hwSa->AN = sa.an;
	hwSa->nextPN = sa.nextPn;

}

static void SetHardwareRxSa(MACSEC_SETTINGS_W_HDR* hwSettings, const MACsecRxSa& sa, uint8_t index) {
	MACSecSa_t* hwSa = &hwSettings->macsec.rx.sa[index];

	hwSa->index = index;
	hwSa->enable = 0x1u;
	memcpy(hwSa->sak, sa.sak.data(), 32);
	memcpy(hwSa->hashKey, sa.hashKey.data(), 16);
	memcpy(hwSa->salt, sa.salt.data(), 12);
	hwSa->ssci = sa.ssci;
	hwSa->nextPN = sa.nextPn;

}

static void SetHardwareRxRule(MACSecRule_t* hwRule, const MACsecRxRule& rule, uint8_t index) {

	hwRule->enable = 0x1u;
	hwRule->index = index;
	memcpy(hwRule->key_MAC_DA, rule.keyMacDa.data(), 6);
	memcpy(hwRule->mask_MAC_DA, rule.maskMacDa.data(), 6);
	memcpy(hwRule->key_MAC_SA, rule.keyMacSa.data(), 6);
	memcpy(hwRule->mask_MAC_SA, rule.maskMacSa.data(), 6);
	hwRule->key_Ethertype = rule.keyEthertype;
	hwRule->mask_Ethertype =  rule.maskEthertype;
	hwRule->key_vlantag_outer1.PRI_CFI = rule.keyVlanTagOuter1.priCfi;
	hwRule->key_vlantag_outer1.VID = rule.keyVlanTagOuter1.vid;
	hwRule->key_MPLS_outer1.exp = rule.keyMplsOuter1.exp;
	hwRule->key_MPLS_outer1.MPLS_label = rule.keyMplsOuter1.mplsLabel;
	hwRule->mask_vlantag_outer1.PRI_CFI = rule.maskVlanTagOuter1.priCfi;
	hwRule->mask_vlantag_outer1.VID = rule.maskVlanTagOuter1.vid;
	hwRule->mask_MPLS_outer1.exp = rule.maskMplsOuter1.exp;
	hwRule->mask_MPLS_outer1.MPLS_label = rule.maskMplsOuter1.mplsLabel;
	hwRule->key_vlantag_outer2.PRI_CFI = rule.keyVlanTagOuter2.priCfi;
	hwRule->key_vlantag_outer2.VID = rule.keyVlanTagOuter2.vid;
	hwRule->key_MPLS_outer2.exp = rule.keyMplsOuter2.exp;
	hwRule->key_MPLS_outer2.MPLS_label = rule.keyMplsOuter2.mplsLabel;
	hwRule->mask_vlantag_outer2.PRI_CFI = rule.maskVlanTagOuter2.priCfi;
	hwRule->mask_vlantag_outer2.VID = rule.maskVlanTagOuter2.vid;
	hwRule->mask_MPLS_outer2.exp = rule.maskMplsOuter2.exp;
	hwRule->mask_MPLS_outer2.MPLS_label = rule.maskMplsOuter2.mplsLabel;
	hwRule->key_bonus_data = rule.keyBonusData;
	hwRule->mask_bonus_data = rule.maskBonusData;
	hwRule->key_tag_match_bitmap = rule.keyTagMatchBitmap;
	hwRule->mask_tag_match_bitmap = rule.maskTagMatchBitmap;
	hwRule->key_packet_type = rule.keyPacketType;
	hwRule->mask_packet_type = rule.maskPacketType;
	hwRule->key_inner_vlan_type = rule.keyInnerVlanType;
	hwRule->mask_inner_vlan_type = rule.maskInnerVlanType;
	hwRule->key_outer_vlan_type = rule.keyOuterVlanType;
	hwRule->mask_outer_vlan_type = rule.maskOuterVlanType;
	hwRule->key_num_tags = rule.keyNumTags;
	hwRule->mask_num_tags = rule.maskNumTags;
	hwRule->key_express = rule.keyExpress ? 0x1u : 0x0u;
	hwRule->mask_express = rule.maskExpress ? 0x1u : 0x0u;
	hwRule->isMPLS = rule.isMpls ? 0x1u : 0x0u;

}

std::vector<uint8_t> MACsecConfig::serialize() const {
	std::vector<uint8_t> res(sizeof(MACSEC_SETTINGS_W_HDR), 0);
	MACSEC_SETTINGS_W_HDR* hwSettings = (MACSEC_SETTINGS_W_HDR*)(res.data());

	for(uint8_t i = 0; i < maxSecY; i++) {
		if(i < rxSecY.size()) {
			SetHardwareRxSecY(
				hwSettings, 
				rxSecY[i],
				rxSecYRekey[i],
				rxRuleIndices[i],
				rxSecYSaIndices[i],
				i
			);
		} else {
			hwSettings->macsec.rx.secy[i].enable = false;
			hwSettings->macsec.rx.map[i].enable = false;
			hwSettings->macsec.rx.sc[i].enable = false;
		}

		if(i < txSecY.size()) {
			SetHardwareTxSecY(
				hwSettings, 
				txSecY[i],
				txSecYRekey[i],
				txSecYSaIndices[i],
				i
			);
		} else {
			hwSettings->macsec.tx.secy[i].enable = false;
			hwSettings->macsec.tx.map[i].enable = false;
			hwSettings->macsec.tx.sc[i].enable = false;
		}
	}

	for(uint8_t i = 0; i < maxSa; i++) {
		if(i < rxSa.size()) {
			SetHardwareRxSa(hwSettings, rxSa[i], i);
		} else {
			hwSettings->macsec.rx.sa[i].enable = false;
		}
		
		if(i < txSa.size()) {
			SetHardwareTxSa(hwSettings, txSa[i], i);
		} else {
			hwSettings->macsec.tx.sa[i].enable = false;
		}
	}

	if(rxRule.size() == 0) {
		MACsecRxRule defaultRule;
		MACSecRule_t* hwRxRule = &hwSettings->macsec.rx.rule[0];
		MACSecRule_t* hwTxRule = &hwSettings->macsec.tx.rule[0];
		SetHardwareRxRule(hwRxRule, defaultRule, 0);
		SetHardwareRxRule(hwTxRule, defaultRule, 0);
		//hwSettings->macsec.tx.rule[0].enable = false;
		for(uint8_t i = 1; i < maxRule; i++) {
			hwSettings->macsec.rx.rule[i].enable = false;
			hwSettings->macsec.tx.rule[i].enable = false;
		}
	} else  {
		for(uint8_t i = 0; i < maxRule; i++) {
			if(i < rxRule.size()) {
				auto* hwRxRule = &hwSettings->macsec.rx.rule[i];
				SetHardwareRxRule(hwRxRule, rxRule[i], i);
			} else {
				hwSettings->macsec.rx.rule[i].enable = false;
				hwSettings->macsec.tx.rule[i].enable = false;
			}
		}
	}

	hwSettings->len = sizeof(MACSEC_SETTINGS_W_HDR);
	hwSettings->version = MACSEC_SETTINGS_VERSION;
	hwSettings->macsec.flags.en = (enableRx || enableTx) ? 1u : 0u;
	hwSettings->macsec.flags.nvm = nvm ? 1u : 0u;
	hwSettings->macsec.rx.flags.en = enableRx ? 1u : 0u;
	hwSettings->macsec.tx.flags.en = enableTx ? 1u : 0u;
	hwSettings->crc32 = crc32(0, (uint8_t*)&hwSettings->macsec, sizeof(MACSEC_SETTINGS));

	return res;
}
}
