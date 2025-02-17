#ifndef __MACSECMESSAGE_H__
#define __MACSECMESSAGE_H__

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/packet.h"
#include "icsneo/api/eventmanager.h"
#include <vector>
#include <memory>
#include <array>
#include <cstring>

namespace icsneo {

struct MACsecVLANTag
{
	uint16_t vid; /*!< 12 bits */
	uint8_t priCfi; /*!< PRI - 3 bits, CFI - 1bit */
};

struct MACsecMPLSOuter
{
	uint32_t mplsLabel; /*!< 20 bits */
	uint8_t exp; /*!< 3 bits */
};

enum class MACsecPacketType : uint8_t
{
	NoVLANOrMPLS = 0,
	SingleVLAN = 1,
	DualVLAN = 2,
	MPLS = 3,
	SingleVLANFollowedByMPLS = 4,
	DualVLANFollowedByMPLS = 5,
	Unsupported = 6,
};

struct MACsecRule
{
	uint8_t index;
	std::array<uint8_t, 6> keyMacDa; /*!< MAC DA field extracted from the packet */
	std::array<uint8_t, 6> maskMacDa; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	std::array<uint8_t, 6> keyMacSa; /*!< MAC SA field extracted from the packet */
	std::array<uint8_t, 6> maskMacSa; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	uint16_t keyEthertype; /*!< First E-Type found in the packet that doesn't match one of the preconfigured custom tag. */
	uint16_t maskEthertype; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	MACsecVLANTag keyVlanTagOuter1; /*!< outermost/1st VLAN ID {8'd0, VLAN_ID[11:0]}, or 20-bit MPLS label. */
	MACsecMPLSOuter keyMplsOuter1;
	MACsecVLANTag maskVlanTagOuter1; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	MACsecMPLSOuter maskMplsOuter1;
	MACsecVLANTag keyVlanTagOuter2; /*!< 2nd outermost VLAN ID {8'd0, VLAN_ID[11:0]}, or 20-bit MPLS label. */
	MACsecMPLSOuter keyMplsOuter2;
	MACsecVLANTag maskVlanTagOuter2; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	MACsecMPLSOuter maskMplsOuter2;
	uint16_t keyBonusData; /*!< 2 bytes of additional bonus data extracted from one of the custom tags. */
	uint16_t maskBonusData; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	uint8_t keyTagMatchBitmap; /*!< 8 bits total. Maps 1 to 1 bitwise with the set of custom tags. (set bit[N]=1 if check Nth custom tag) */
	uint8_t maskTagMatchBitmap; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	MACsecPacketType keyPacketType; /*!< Encoded Packet Type, see MACSEC_PACKET_TYPE */
	uint8_t maskPacketType; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	uint16_t keyInnerVlanType; /*!< 3 bits total. Encoded value indicating which VLAN TPID value matched for the second outermost VLAN Tag. */
	uint16_t maskInnerVlanType; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	uint16_t keyOuterVlanType; /*!< 3 bits total. Encoded value indicating which VLAN TPID value matched for the outermost VLAN Tag. */
	uint16_t maskOuterVlanType; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	uint8_t keyNumTags; /*!< 7 bits total. Number of VLAN/custom tags or MPLS lables detected. Ingress: before SecTag; Egress: total detected. Exclude MCS header tags. i.e. Bit 2: 2 tags/labels before SecTAG...Bit 6: 6 or more tags/labels before SecTAG. */
	uint8_t maskNumTags; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	bool keyExpress; /*!< 1 bits. Express packet. */
	uint8_t maskExpress; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	bool isMpls;
	bool enable;
};

struct MACsecMap
{
	uint8_t index;
	uint64_t secTagSci; /*!< Identifies the SecTAG SCI for this Flow. */
	uint8_t secYIndex; /*!< index for entry in Egress secY Policy */
	bool isControlPacket; /*!< Identifies all packets matching this index lookup as control packets. */
	uint8_t scIndex; /*!< Identifies the SC for this Flow. */
	uint8_t auxiliaryPlcy; /*!< Auxiliary policy bits. */
	uint8_t ruleId; /*!< Identifies the Rule for this Flow. */
	bool enable;
};

enum class MACsecValidateFrameType : uint8_t
{
	Disabled = 0, /*!< Disable validation */
	Check = 1, /*!< Enable validation, do not discard invalid frames*/
	Strict = 2, /*!< Enable validation and discard invalid frames */
	NA = 3 /*!< No processing or accounting */
};

enum class MACsecSecTagIcvStripType : uint8_t
{
	StripBoth = 0, /*!< Strip both SecTag and ICV from packet */
	StripSecTagPreserveICV = 1,
	PreserveSecTagStripICV = 2, /*!< Preserve SecTag, Strip ICV */
	PreserveBoth = 3 /*!< Preserve both SecTag and ICV */
};

enum class MACsecCipherSuiteType : uint8_t
{
	GcmAes128 = 0,
	GcmAes256 = 1,
	GcmAes128Xpn = 2,
	GcmAes256Xpn = 3
};

struct MACsecSecY
{
	uint8_t index; /*!< Identifies the SecY for this Flow. */
	bool controlledPortEnabled; /*!< Enable (or disable) operation of the Controlled port associated with this SecY */
	MACsecValidateFrameType frameValidationType; /*!< see MACSEC_VALIDATEFRAME */
	MACsecSecTagIcvStripType secTagIcvStripType; /*!< see MACSEC_STRIP_SECTAG_ICV */
	MACsecCipherSuiteType cipher; /*!< Define the cipher suite to use for this SecY see MACSEC_CIPHER_SUITE */
	uint8_t confidentialOffset; /*!< Define the number of bytes that are unencrypted following the SecTag. */
	bool icvIncludesDaSa; /*!< When set, the outer DA/SA bytes are included in the authentication GHASH calculation */
	bool replayProtect; /*!< Enables Anti-Replay protection */
	uint32_t replayWindow; /*!< Unsigned value indicating the size of the anti-replay window. */
	bool protectFrames; /*!< 0 = do not encrypt or authenticate this packet; 1 = always Authenticate frame and if SecTag.TCI.E = 1 encrypt the packet as well. */
	uint8_t secTagOffset; /*!< Define the offset in bytes from either the start of the packet or a matching Etype depending on SecTag_Insertion_Mode. */
	uint8_t secTagTci; /*!< Tag Control Information excluding the AN field which originates from the SA Policy table */
	uint16_t mtu; /*!< Specifies the outgoing MTU for this SecY */
	bool enable;
};

struct MACsecSc
{
	uint8_t index; /*!< SC index. */
	uint8_t secYIndex; /*!< SecY associated with this packet. */
	uint64_t sci; /*!< The Secure Channel Identifier. */
	uint8_t saIndex0; /*!< Define the 1st SA to use */
	uint8_t saIndex1; /*!< Define the 2nd SA to use */
	bool saIndex0InUse; /*!< Specifies whether 1st SA is in use or not. */
	bool saIndex1InUse; /*!< Specifies whether 2nd SA is in use or not.  */
	bool enableAutoRekey; /*!< If enabled, then once the pn_threshold is reached, auto rekey will happen. */
	bool isActiveSa1; /*!< If set, then sa_index1 is the currently active SA index. If cleared, the sa_index0 is the currently active SA index). */
	bool enable;
};

struct MACsecSa
{
	uint8_t index; /*!< SA index */
	std::array<uint8_t, 32> sak; /*!< 256b SAK: Define the encryption key to be used to encrypte this packet. The lower 128 bits are used for 128-bit ciphers. */
	std::array<uint8_t, 16> hashKey; /*!< 128b Hash Key: Key used for authentication. */
	std::array<uint8_t, 12> salt; /*!< 96b Salt value: Salt value used in XPN ciphers. */
	uint32_t ssci; /*!< 32b SSCI value: Short Secure Channel Identifier, used in XPN ciphers. */
	uint8_t an; /*!< 2b SecTag Association Number (AN) */
	uint64_t nextPn; /*!< 64b next_pn value: Next packet number to insert into outgoing packet on a particular SA. */
	bool enable;
};

struct MACSecFlags
{
	bool en; // '1' = enable; '0' = disable
};

/* MACSec Settings for 1 port/phy */
struct MACsecConfig
{
	static constexpr int NumFlags = 1;
	static constexpr int NumRules = 2;
	static constexpr int NumMaps = 2;
	static constexpr int NumSecY = 2;
	static constexpr int NumSc = 2;
	static constexpr int NumSa = 4;

	MACSecFlags flags;
	std::array<MACsecRule, NumRules> rule;
	std::array<MACsecMap, NumMaps> map;
	std::array<MACsecSecY, NumSecY> secy;
	std::array<MACsecSc, NumSc> sc;
	std::array<MACsecSa, NumSa> sa;
};

struct MACSecGlobalFlags
{
	bool en; // '1' = enable; '0' = disable
	bool nvm; // store macsec config in non-volatile memory
};

class MACsecMessage : public Message
{
public:
	MACsecMessage(void) : Message(Message::Type::RawMessage) {}

	MACSecGlobalFlags flags;
	MACsecConfig rx;
	MACsecConfig tx;

	static std::shared_ptr<MACsecMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);
	bool EncodeFromMessage(std::vector<uint8_t>& bytestream, const device_eventhandler_t& report) const;
};

}

#endif // __cplusplus

#endif