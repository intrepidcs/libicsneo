#include "icsneo/communication/message/macsecmessage.h"

#include "icsneo/communication/crc32.h"
#include <memory>
#include <cstdint>
#include <iostream>
#include <cstring>

namespace icsneo
{

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // nameless struct/union
#endif

#pragma pack(push, 1)
/* MACsec Rule */
/**
 * @brief Structure of Vlan tag
 *
 */
typedef struct
{
	uint16_t vid; /*!< 12 bits */
	uint8_t priCfi; /*!< PRI - 3 bits, CFI - 1bit */
} MACSEC_VLANTAG_t;
/**
 * @brief Structure of MPLS
 *
 */
typedef struct
{
	uint32_t mplsLabel; /*!< 20 bits */
	uint8_t exp; /*!< 3 bits */
} MACSEC_MPLS_OUTER_t;
/**
 * @brief Define Encoded Packet Type from the parser
 *
 */
static constexpr int MACSEC_SETTINGS_RULE_SIZE = 88;
typedef union _MACSecRule
{
	struct
	{
		uint8_t index;
		uint8_t keyMacDa[6]; /*!< MAC DA field extracted from the packet */
		uint8_t maskMacDa[6]; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint8_t keyMacSa[6]; /*!< MAC SA field extracted from the packet */
		uint8_t maskMacSa[6]; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint16_t keyEthertype; /*!< First E-Type found in the packet that doesn't match one of the preconfigured custom tag. */
		uint16_t maskEthertype; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		MACSEC_VLANTAG_t keyVlanTagOuter1; /*!< outermost/1st VLAN ID {8'd0, VLAN_ID[11:0]}, or 20-bit MPLS label. */
		MACSEC_MPLS_OUTER_t keyMplsOuter1;
		MACSEC_VLANTAG_t maskVlanTagOuter1; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		MACSEC_MPLS_OUTER_t maskMplsOuter1;
		MACSEC_VLANTAG_t keyVlanTagOuter2; /*!< 2nd outermost VLAN ID {8'd0, VLAN_ID[11:0]}, or 20-bit MPLS label. */
		MACSEC_MPLS_OUTER_t keyMplsOuter2;
		MACSEC_VLANTAG_t maskVlanTagOuter2; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		MACSEC_MPLS_OUTER_t maskMplsOuter2;
		uint16_t keyBonusData; /*!< 2 bytes of additional bonus data extracted from one of the custom tags. */
		uint16_t maskBonusData; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint8_t
			keyTagMatchBitmap; /*!< 8 bits total. Maps 1 to 1 bitwise with the set of custom tags. (set bit[N]=1 if check Nth custom tag) */
		uint8_t maskTagMatchBitmap; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint8_t keyPacketType; /*!< Encoded Packet Type, see MACSEC_PACKET_TYPE */
		uint8_t maskPacketType; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint16_t
			keyInnerVlanType; /*!< 3 bits total. Encoded value indicating which VLAN TPID value matched for the second outermost VLAN Tag. */
		uint16_t maskInnerVlanType; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint16_t keyOuterVlanType; /*!< 3 bits total. Encoded value indicating which VLAN TPID value matched for the outermost VLAN Tag. */
		uint16_t maskOuterVlanType; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint8_t
			keyNumTags; /*!< 7 bits total. Number of VLAN/custom tags or MPLS lables detected. Ingress: before SecTag; Egress: total detected. Exclude MCS header tags. i.e. Bit 2: 2 tags/labels before SecTAG...Bit 6: 6 or more tags/labels before SecTAG. */
		uint8_t maskNumTags; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint8_t keyExpress; /*!< 1 bits. Express packet. */
		uint8_t maskExpress; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
		uint8_t isMpls;
		uint8_t rsvd[5];
		uint8_t enable;
	};
	uint8_t byte[MACSEC_SETTINGS_RULE_SIZE];
} MACSecRule_t;
/* MACsec Map */
static constexpr int MACSEC_SETTINGS_MAP_SIZE = 20;
typedef union _MACSecMap
{
	struct
	{
		uint8_t index;
		uint64_t secTagSci; /*!< Identifies the SecTAG SCI for this Flow. */
		uint8_t secYIndex; /*!< index for entry in Egress secY Policy */
		uint8_t isControlPacket; /*!< Identifies all packets matching this index lookup as control packets. */
		uint8_t scIndex; /*!< Identifies the SC for this Flow. */
		uint8_t auxiliaryPlcy; /*!< Auxiliary policy bits. */
		uint8_t ruleId; /*!< Identifies the Rule for this Flow. */
		uint8_t rsvd[5];
		uint8_t enable;
	};
	uint8_t byte[MACSEC_SETTINGS_MAP_SIZE];
} MACSecMap_t;
/* MACsec SecY */
/**
 * @brief Define the permit police for frames as defined in 802.1ae
 *
 */
static constexpr int MACSEC_SETTINGS_SECY_SIZE = 24;
typedef union _MACSecSecY
{
	struct
	{
		uint8_t index; /*!< Identifies the SecY for this Flow. */
		uint8_t controlledPortEnabled; /*!< Enable (or disable) operation of the Controlled port associated with this SecY */
		uint8_t frameValidationType; /*!< see MACSEC_VALIDATEFRAME */
		uint8_t secTagIcvStripType; /*!< see MACSEC_STRIP_SECTAG_ICV */
		uint8_t cipher; /*!< Define the cipher suite to use for this SecY see MACSEC_CIPHER_SUITE */
		uint8_t confidentialOffset; /*!< Define the number of bytes that are unencrypted following the SecTag. */
		uint8_t icvIncludesDaSa; /*!< When set, the outer DA/SA bytes are included in the authentication GHASH calculation */
		uint8_t replayProtect; /*!< Enables Anti-Replay protection */
		uint32_t replayWindow; /*!< Unsigned value indicating the size of the anti-replay window. */
		uint8_t
			protectFrames; /*!< 0 = do not encrypt or authenticate this packet; 1 = always Authenticate frame and if SecTag.TCI.E = 1 encrypt the packet as well. */
		uint8_t
			secTagOffset; /*!< Define the offset in bytes from either the start of the packet or a matching Etype depending on SecTag_Insertion_Mode. */
		uint8_t secTagTci; /*!< Tag Control Information excluding the AN field which originates from the SA Policy table */
		uint16_t mtu; /*!< Specifies the outgoing MTU for this SecY */
		uint8_t rsvd[6];
		uint8_t enable;
	};
	uint8_t byte[MACSEC_SETTINGS_SECY_SIZE];
} MACSecSecY_t;
/* MACsec SC */
static constexpr int MACSEC_SETTINGS_SC_SIZE = 24;
typedef union _MACSecSc
{
	struct
	{
		uint8_t index; /*!< SC index. */
		uint8_t secYIndex; /*!< SecY associated with this packet. */
		uint64_t sci; /*!< The Secure Channel Identifier. */
		uint8_t saIndex0; /*!< Define the 1st SA to use */
		uint8_t saIndex1; /*!< Define the 2nd SA to use */
		uint8_t saIndex0InUse; /*!< Specifies whether 1st SA is in use or not. */
		uint8_t saIndex1InUse; /*!< Specifies whether 2nd SA is in use or not.  */
		uint8_t enableAutoRekey; /*!< If enabled, then once the pn_threshold is reached, auto rekey will happen. */
		uint8_t
			isActiveSa1; /*!< If set, then sa_index1 is the currently active SA index. If cleared, the sa_index0 is the currently active SA index). */
		uint8_t rsvd[7];
		uint8_t enable;
	};
	uint8_t byte[MACSEC_SETTINGS_SC_SIZE];
} MACSecSc_t;
/* MACsec SA */
static constexpr int MACSEC_SETTINGS_SA_SIZE = 80;
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
		uint8_t an; /*!< 2b SecTag Association Number (AN) */
		uint64_t nextPn; /*!< 64b next_pn value: Next packet number to insert into outgoing packet on a particular SA. */
		uint8_t rsvd[5];
		uint8_t enable;
	};
	uint8_t byte[MACSEC_SETTINGS_SA_SIZE];
} MACSecSa_t;
/* MACsec Flags */
static constexpr int MACSEC_SETTINGS_FLAGS_SIZE = 4;
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
typedef struct MACSEC_CONFIG_t
{
	MACSecFlags_t flags;
	MACSecRule_t rule[MACsecConfig::NumRules];
	MACSecMap_t map[MACsecConfig::NumMaps];
	MACSecSecY_t secy[MACsecConfig::NumSecY];
	MACSecSc_t sc[MACsecConfig::NumSc];
	MACSecSa_t sa[MACsecConfig::NumSa];
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
struct MACSEC_SETTINGS_W_HDR
{
	uint16_t version;
	uint16_t len;
	uint32_t crc32;
	MACSEC_SETTINGS macsec;
};

#pragma pack(pop)

#ifdef _MSC_VER
#pragma warning(pop)
#endif

std::shared_ptr<MACsecMessage> MACsecMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream, const device_eventhandler_t& report)
{
	if (bytestream.empty() || (bytestream.size() < sizeof(MACSEC_SETTINGS_W_HDR)))
	{
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return nullptr;
	}

	MACSEC_SETTINGS_W_HDR* macsecArgs = (MACSEC_SETTINGS_W_HDR*)bytestream.data();
	if (macsecArgs->version != MACSEC_SETTINGS_VERSION)
	{
		report(APIEvent::Type::SettingsVersionError, APIEvent::Severity::Error);
		return nullptr;
	}
	if (macsecArgs->len != sizeof(MACSEC_SETTINGS))
	{
		report(APIEvent::Type::SettingsLengthError, APIEvent::Severity::Error);
		return nullptr;
	}
	const auto crcCalculted = crc32(0, (uint8_t*)&macsecArgs->macsec, macsecArgs->len);
	if (macsecArgs->crc32 != crcCalculted)
	{
		report(APIEvent::Type::SettingsChecksumError, APIEvent::Severity::Error);
		return nullptr;
	}

	auto msg = std::make_shared<MACsecMessage>();

	const auto& copyConfig = [](const MACSEC_CONFIG& source, MACsecConfig& dest)
	{
		dest.flags.en = source.flags.en;
		for (int index = 0; index < MACsecConfig::NumRules; index++)
		{
			if(!source.rule[index].enable) continue;
#undef __COPY_ITEM
#define __COPY_ITEM(___name__) dest.rule[index].___name__ = (decltype(dest.rule[index].___name__))source.rule[index].___name__
#undef __COPY_ARR
#define __COPY_ARR(___name__) (void)memcpy(dest.rule[index].___name__.data(), source.rule[index].___name__, dest.rule[index].___name__.size())
			__COPY_ITEM(index);
			__COPY_ARR(keyMacDa);
			__COPY_ARR(maskMacDa);
			__COPY_ARR(keyMacSa);
			__COPY_ARR(maskMacSa);
			__COPY_ITEM(keyVlanTagOuter1.vid);
			__COPY_ITEM(keyVlanTagOuter1.priCfi);
			__COPY_ITEM(keyMplsOuter1.mplsLabel);
			__COPY_ITEM(keyMplsOuter1.exp);
			__COPY_ITEM(maskVlanTagOuter1.vid);
			__COPY_ITEM(maskVlanTagOuter1.priCfi);
			__COPY_ITEM(maskMplsOuter1.mplsLabel);
			__COPY_ITEM(maskMplsOuter1.exp);
			__COPY_ITEM(keyVlanTagOuter2.vid);
			__COPY_ITEM(keyVlanTagOuter2.priCfi);
			__COPY_ITEM(keyMplsOuter2.mplsLabel);
			__COPY_ITEM(keyMplsOuter2.exp);
			__COPY_ITEM(maskVlanTagOuter2.vid);
			__COPY_ITEM(maskVlanTagOuter2.priCfi);
			__COPY_ITEM(maskMplsOuter2.mplsLabel);
			__COPY_ITEM(maskMplsOuter2.exp);
			__COPY_ITEM(keyEthertype);
			__COPY_ITEM(maskEthertype);
			__COPY_ITEM(keyBonusData);
			__COPY_ITEM(maskBonusData);
			__COPY_ITEM(keyTagMatchBitmap);
			__COPY_ITEM(maskTagMatchBitmap);
			__COPY_ITEM(keyPacketType);
			__COPY_ITEM(maskPacketType);
			__COPY_ITEM(keyInnerVlanType);
			__COPY_ITEM(maskInnerVlanType);
			__COPY_ITEM(keyOuterVlanType);
			__COPY_ITEM(maskOuterVlanType);
			__COPY_ITEM(keyNumTags);
			__COPY_ITEM(maskNumTags);
			__COPY_ITEM(keyExpress);
			__COPY_ITEM(maskExpress);
			__COPY_ITEM(isMpls);
			__COPY_ITEM(enable);
		}

		for (int index = 0; index < MACsecConfig::NumMaps; index++)
		{
			if(!source.map[index].enable) continue;
#undef __COPY_ITEM
#define __COPY_ITEM(___name__) dest.map[index].___name__ = source.map[index].___name__
			__COPY_ITEM(index);
			__COPY_ITEM(secTagSci);
			__COPY_ITEM(secYIndex);
			__COPY_ITEM(isControlPacket);
			__COPY_ITEM(scIndex);
			__COPY_ITEM(auxiliaryPlcy);
			__COPY_ITEM(ruleId);
			__COPY_ITEM(enable);
		}

		for (int index = 0; index < MACsecConfig::NumSecY; index++)
		{
			if(!source.secy[index].enable) continue;
#undef __COPY_ITEM
#define __COPY_ITEM(___name__) dest.secy[index].___name__ = (decltype(dest.secy[index].___name__))source.secy[index].___name__
			__COPY_ITEM(index);
			__COPY_ITEM(controlledPortEnabled);
			__COPY_ITEM(frameValidationType);
			__COPY_ITEM(secTagIcvStripType);
			__COPY_ITEM(cipher);
			__COPY_ITEM(confidentialOffset);
			__COPY_ITEM(icvIncludesDaSa);
			__COPY_ITEM(replayProtect);
			__COPY_ITEM(replayWindow);
			__COPY_ITEM(protectFrames);
			__COPY_ITEM(secTagOffset);
			__COPY_ITEM(secTagTci);
			__COPY_ITEM(mtu);
			__COPY_ITEM(enable);
		}

		for (int index = 0; index < MACsecConfig::NumSc; index++)
		{
			if(!source.sc[index].enable) continue;
#undef __COPY_ITEM
#define __COPY_ITEM(___name__) dest.sc[index].___name__ = source.sc[index].___name__
			__COPY_ITEM(index);
			__COPY_ITEM(secYIndex);
			__COPY_ITEM(sci);
			__COPY_ITEM(saIndex0);
			__COPY_ITEM(saIndex1);
			__COPY_ITEM(saIndex0InUse);
			__COPY_ITEM(saIndex1InUse);
			__COPY_ITEM(enableAutoRekey);
			__COPY_ITEM(isActiveSa1);
			__COPY_ITEM(enable);
		}

		for (int index = 0; index < MACsecConfig::NumSa; index++)
		{
			if(!source.sa[index].enable) continue;
#undef __COPY_ITEM
#define __COPY_ITEM(___name__) dest.sa[index].___name__ = source.sa[index].___name__
#undef __COPY_ARR
#define __COPY_ARR(___name__) (void)memcpy(dest.sa[index].___name__.data(), source.sa[index].___name__, dest.sa[index].___name__.size())
			__COPY_ITEM(index);
			__COPY_ARR(sak);
			__COPY_ARR(hashKey);
			__COPY_ARR(salt);
			__COPY_ITEM(ssci);
			__COPY_ITEM(an);
			__COPY_ITEM(nextPn);
			__COPY_ITEM(enable);
		}
	};

	msg->flags.en = macsecArgs->macsec.flags.en;
	copyConfig(macsecArgs->macsec.rx, msg->rx);
	copyConfig(macsecArgs->macsec.tx, msg->tx);

	return msg;
}

bool MACsecMessage::EncodeFromMessage(std::vector<uint8_t>& bytestream, const device_eventhandler_t& report) const
{
	MACSEC_SETTINGS_W_HDR* macsecArgs;

	bytestream.resize(sizeof(MACSEC_SETTINGS_W_HDR), 0);
	macsecArgs = (MACSEC_SETTINGS_W_HDR*)bytestream.data();

	macsecArgs->version = MACSEC_SETTINGS_VERSION;
	macsecArgs->len = sizeof(MACSEC_SETTINGS_W_HDR);

	const auto& copyConfig = [](const MACsecConfig& source, MACSEC_CONFIG& dest)
	{
		dest.flags.en = (uint8_t)source.flags.en;
		for (int index = 0; index < MACsecConfig::NumRules; index++)
		{
			if(!source.rule[index].enable) continue;
#undef __COPY_ITEM
#define __COPY_ITEM(___name__) dest.rule[index].___name__ = (decltype(dest.rule[index].___name__))source.rule[index].___name__
#undef __COPY_ARR
#define __COPY_ARR(___name__) (void)memcpy(dest.rule[index].___name__, source.rule[index].___name__.data(), source.rule[index].___name__.size())
			__COPY_ITEM(index);
			__COPY_ARR(keyMacDa);
			__COPY_ARR(maskMacDa);
			__COPY_ARR(keyMacSa);
			__COPY_ARR(maskMacSa);
			__COPY_ITEM(keyVlanTagOuter1.vid);
			__COPY_ITEM(keyVlanTagOuter1.priCfi);
			__COPY_ITEM(keyMplsOuter1.mplsLabel);
			__COPY_ITEM(keyMplsOuter1.exp);
			__COPY_ITEM(maskVlanTagOuter1.vid);
			__COPY_ITEM(maskVlanTagOuter1.priCfi);
			__COPY_ITEM(maskMplsOuter1.mplsLabel);
			__COPY_ITEM(maskMplsOuter1.exp);
			__COPY_ITEM(keyVlanTagOuter2.vid);
			__COPY_ITEM(keyVlanTagOuter2.priCfi);
			__COPY_ITEM(keyMplsOuter2.mplsLabel);
			__COPY_ITEM(keyMplsOuter2.exp);
			__COPY_ITEM(maskVlanTagOuter2.vid);
			__COPY_ITEM(maskVlanTagOuter2.priCfi);
			__COPY_ITEM(maskMplsOuter2.mplsLabel);
			__COPY_ITEM(maskMplsOuter2.exp);
			__COPY_ITEM(keyEthertype);
			__COPY_ITEM(maskEthertype);
			__COPY_ITEM(keyBonusData);
			__COPY_ITEM(maskBonusData);
			__COPY_ITEM(keyTagMatchBitmap);
			__COPY_ITEM(maskTagMatchBitmap);
			__COPY_ITEM(keyPacketType);
			__COPY_ITEM(maskPacketType);
			__COPY_ITEM(keyInnerVlanType);
			__COPY_ITEM(maskInnerVlanType);
			__COPY_ITEM(keyOuterVlanType);
			__COPY_ITEM(maskOuterVlanType);
			__COPY_ITEM(keyNumTags);
			__COPY_ITEM(maskNumTags);
			__COPY_ITEM(keyExpress);
			__COPY_ITEM(maskExpress);
			__COPY_ITEM(isMpls);
			__COPY_ITEM(enable);
		}

		for (int index = 0; index < MACsecConfig::NumMaps; index++)
		{
			if(!source.map[index].enable) continue;
#undef __COPY_ITEM
#define __COPY_ITEM(___name__) dest.map[index].___name__ = source.map[index].___name__
			__COPY_ITEM(index);
			__COPY_ITEM(secTagSci);
			__COPY_ITEM(secYIndex);
			__COPY_ITEM(isControlPacket);
			__COPY_ITEM(scIndex);
			__COPY_ITEM(auxiliaryPlcy);
			__COPY_ITEM(ruleId);
			__COPY_ITEM(enable);
		}

		for (int index = 0; index < MACsecConfig::NumSecY; index++)
		{
			if(!source.secy[index].enable) continue;
#undef __COPY_ITEM
#define __COPY_ITEM(___name__) dest.secy[index].___name__ = (decltype(dest.secy[index].___name__))source.secy[index].___name__
			__COPY_ITEM(index);
			__COPY_ITEM(controlledPortEnabled);
			__COPY_ITEM(frameValidationType);
			__COPY_ITEM(secTagIcvStripType);
			__COPY_ITEM(cipher);
			__COPY_ITEM(confidentialOffset);
			__COPY_ITEM(icvIncludesDaSa);
			__COPY_ITEM(replayProtect);
			__COPY_ITEM(replayWindow);
			__COPY_ITEM(protectFrames);
			__COPY_ITEM(secTagOffset);
			__COPY_ITEM(secTagTci);
			__COPY_ITEM(mtu);
			__COPY_ITEM(enable);
		}

		for (int index = 0; index < MACsecConfig::NumSc; index++)
		{
			if(!source.sc[index].enable) continue;
#undef __COPY_ITEM
#define __COPY_ITEM(___name__) dest.sc[index].___name__ = source.sc[index].___name__
			__COPY_ITEM(index);
			__COPY_ITEM(secYIndex);
			__COPY_ITEM(sci);
			__COPY_ITEM(saIndex0);
			__COPY_ITEM(saIndex1);
			__COPY_ITEM(saIndex0InUse);
			__COPY_ITEM(saIndex1InUse);
			__COPY_ITEM(enableAutoRekey);
			__COPY_ITEM(isActiveSa1);
			__COPY_ITEM(enable);
		}

		for (int index = 0; index < MACsecConfig::NumSa; index++)
		{
			if(!source.sa[index].enable) continue;
#undef __COPY_ITEM
#define __COPY_ITEM(___name__) dest.sa[index].___name__ = source.sa[index].___name__
#undef __COPY_ARR
#define __COPY_ARR(___name__) (void)memcpy(dest.sa[index].___name__, source.sa[index].___name__.data(), source.sa[index].___name__.size())
			__COPY_ITEM(index);
			__COPY_ARR(sak);
			__COPY_ARR(hashKey);
			__COPY_ARR(salt);
			__COPY_ITEM(ssci);
			__COPY_ITEM(an);
			__COPY_ITEM(nextPn);
			__COPY_ITEM(enable);
		}
	};
	macsecArgs->macsec.flags.en = this->flags.en;
	macsecArgs->macsec.flags.nvm = this->flags.nvm;

	copyConfig(this->rx, macsecArgs->macsec.rx);
	copyConfig(this->tx, macsecArgs->macsec.tx);

	macsecArgs->crc32 = crc32(0, (uint8_t*)&macsecArgs->macsec, sizeof(MACSEC_SETTINGS));

	(void)report;

	return true;
}

} // namespace icsneo
