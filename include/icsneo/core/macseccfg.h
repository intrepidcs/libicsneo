#ifndef __MACSEC_CONFIG_H_
#define __MACSEC_CONFIG_H_

#include <vector>
#include <array>
#include <stdint.h>
#include "icsneo/device/devicetype.h"

namespace icsneo {


struct MACsecVLANTag {
	uint16_t vid = 0xFFFFu; /*!< 12 bits */
	uint8_t priCfi = 0xFFu; /*!< PRI - 3 bits, CFI - 1bit */
};

struct MACsecMPLSOuter {
	uint32_t mplsLabel = 0xFFFFFFFFu; /*!< 20 bits */
	uint8_t exp = 0xFFu; /*!< 3 bits */
};

enum class MACsecPacketType : uint8_t {
	Default = 0,
	SingleVLAN = 1,
	DualVLAN = 2,
	MPLS = 3,
	SingleVLANFollowedByMPLS = 4,
	DualVLANFollowedByMPLS = 5,
	Unsupported = 6,
};

// Tell the MACsec phy which packets to accept
struct MACsecRxRule {
	std::array<uint8_t, 6> keyMacDa = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu}; /*!< MAC DA field extracted from the packet */
	std::array<uint8_t, 6> maskMacDa = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu}; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	std::array<uint8_t, 6> keyMacSa = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu}; /*!< MAC SA field extracted from the packet */
	std::array<uint8_t, 6> maskMacSa = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFFu}; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	uint16_t keyEthertype = 0xFFFFu; /*!< First E-Type found in the packet that doesn't match one of the preconfigured custom tag. */
	uint16_t maskEthertype = 0xFFFFu; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	MACsecVLANTag keyVlanTagOuter1; /*!< outermost/1st VLAN ID {8'd0, VLAN_ID[11:0]}, or 20-bit MPLS label. */
	MACsecMPLSOuter keyMplsOuter1;
	MACsecVLANTag maskVlanTagOuter1; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	MACsecMPLSOuter maskMplsOuter1;
	MACsecVLANTag keyVlanTagOuter2; /*!< 2nd outermost VLAN ID {8'd0, VLAN_ID[11:0]}, or 20-bit MPLS label. */
	MACsecMPLSOuter keyMplsOuter2;
	MACsecVLANTag maskVlanTagOuter2; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	MACsecMPLSOuter maskMplsOuter2;
	uint16_t keyBonusData = 0xFFFFu; /*!< 2 bytes of additional bonus data extracted from one of the custom tags. */
	uint16_t maskBonusData = 0xFFFFu; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	uint8_t keyTagMatchBitmap = 0xFFu; /*!< 8 bits total. Maps 1 to 1 bitwise with the set of custom tags. (set bit[N]=1 if check Nth custom tag) */
	uint8_t maskTagMatchBitmap = 0xFFu; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	MACsecPacketType keyPacketType = MACsecPacketType::Default; /*!< Encoded Packet Type, see MACSEC_PACKET_TYPE */
	uint8_t maskPacketType = 0xFFu; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	uint16_t keyInnerVlanType = 0xFFFFu; /*!< 3 bits total. Encoded value indicating which VLAN TPID value matched for the second outermost VLAN Tag. */
	uint16_t maskInnerVlanType = 0xFFFFu; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	uint16_t keyOuterVlanType = 0xFFFFu; /*!< 3 bits total. Encoded value indicating which VLAN TPID value matched for the outermost VLAN Tag. */
	uint16_t maskOuterVlanType = 0xFFFFu; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	uint8_t keyNumTags = 0xFFu; /*!< 7 bits total. Number of VLAN/custom tags or MPLS lables detected. Ingress: before SecTag; Egress: total detected. Exclude MCS header tags. i.e. Bit 2: 2 tags/labels before SecTAG...Bit 6: 6 or more tags/labels before SecTAG. */
	uint8_t maskNumTags = 0xFFu; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	bool keyExpress = true; /*!< 1 bits. Express packet. */
	bool maskExpress = true; /*!< Set bits to 1 to mask/exclude corresponding flowid_tcam_data bit from compare */
	bool isMpls = false;
};

enum class MACsecValidation : uint8_t {
	Disabled = 0, /*!< Disable validation */
	Check = 1, /*!< Enable validation, do not discard invalid frames*/
	Strict = 2, /*!< Enable validation and discard invalid frames */
	NA = 3 /*!< No processing or accounting */
};

enum class MACsecStrip : uint8_t {
	StripSecTagAndIcv = 0, /*!< Strip both SecTag and ICV from packet */
	StripSecTagPreserveICV = 1,
	PreserveSecTagStripICV = 2, /*!< Preserve SecTag, Strip ICV */
	NoStrip = 3 /*!< Preserve both SecTag and ICV */
};

enum class MACsecCipherSuite : uint8_t {
	GcmAes128 = 0,
	GcmAes256 = 1,
	GcmAes128Xpn = 2,
	GcmAes256Xpn = 3
};

// Tag control information
struct MACsecTci {
	bool es = false; // End station bit
	bool sc = true; // SCI included bit
	bool scb = false; // Single Copy Broadcast
	bool e = false; // Encryption bit
	bool c = false; // Changed text bit
};


struct MACsecRxSecY {
	bool enableControlPort = true; /*!< Enable (or disable) operation of the Controlled port associated with this SecY */
	MACsecValidation frameValidation = MACsecValidation::Strict; /*!< see MACSEC_VALIDATEFRAME */
	MACsecStrip frameStrip = MACsecStrip::NoStrip; /*!< see MACSEC_STRIP_SECTAG_ICV */
	MACsecCipherSuite cipher = MACsecCipherSuite::GcmAes128; /*!< Define the cipher suite to use for this SecY see MACSEC_CIPHER_SUITE */
	uint8_t confidentialityOffset = 0; /*!< Define the number of bytes that are unencrypted following the SecTag. */
	bool icvIncludesDaSa = true; /*!< When set, the outer DA/SA bytes are included in the authentication GHASH calculation */
	bool replayProtect = true; /*!< Enables Anti-Replay protection */
	uint32_t replayWindow = 1; /*!< Unsigned value indicating the size of the anti-replay window. */
	bool isControlPacket = false; /*!< Identifies all packets matching this index lookup as control packets. */
	uint64_t sci; /** The SCI of this secY */
};

struct MACsecTxSecY {
	bool enableControlPort = true; /*!< Enable (or disable) operation of the Controlled port associated with this SecY */
	MACsecCipherSuite cipher = MACsecCipherSuite::GcmAes128; /*!< Define the cipher suite to use for this SecY see MACSEC_CIPHER_SUITE */
	uint8_t confidentialityOffset = 0; /*!< Define the number of bytes that are unencrypted following the SecTag. */
	bool icvIncludesDaSa = true; /*!< When set, the outer DA/SA bytes are included in the authentication GHASH calculation */
	bool protectFrames = true; /*!< 0 = do not encrypt or authenticate this packet; 1 = always Authenticate frame and if SecTag.TCI.E = 1 encrypt the packet as well. */
	uint8_t secTagOffset = 12; /*!< Define the offset in bytes from either the start of the packet or a matching Etype depending on SecTag_Insertion_Mode. */
	MACsecTci tci; /*!< Tag Control Information excluding the AN field which originates from the SA Policy table */
	uint16_t mtu = 0xFFFFu; /*!< Specifies the outgoing MTU for this SecY */
	bool isControlPacket = false; /*!< Identifies all packets matching this index lookup as control packets. */
	uint8_t auxiliaryPolicy = 0u; /*!< Auxiliary policy bits. */
	uint64_t sci = 0x1122334455660001u;
};

struct MACsecTxSa {
	std::array<uint8_t, 32> sak = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 
		0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}; /*!< 256b SAK: Define the encryption key to be used to encrypte this packet. The lower 128 bits are used for 128-bit ciphers. */
	std::array<uint8_t, 16> hashKey = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}; /*!< 128b Hash Key: Key used for authentication. */
	std::array<uint8_t, 12> salt = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}; /*!< 96b Salt value: Salt value used in XPN ciphers. */
	uint32_t ssci = 0x01u; /*!< 32b SSCI value: Short Secure Channel Identifier, used in XPN ciphers. */
	uint8_t an = 0x00;; /*!< 2b SecTag Association Number (AN) */
	uint64_t nextPn = 0x01u; /*!< 64b next_pn value: Next packet number to insert into outgoing packet on a particular SA. */
};

struct MACsecRxSa {
	std::array<uint8_t, 32> sak = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 
		0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}; /*!< 256b SAK: Define the encryption key to be used to encrypte this packet. The lower 128 bits are used for 128-bit ciphers. */
	std::array<uint8_t, 16> hashKey = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}; /*!< 128b Hash Key: Key used for authentication. */
	std::array<uint8_t, 12> salt = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}; /*!< 96b Salt value: Salt value used in XPN ciphers. */
	uint32_t ssci = 0x01u; /*!< 32b SSCI value: Short Secure Channel Identifier, used in XPN ciphers. */
	uint64_t nextPn = 0x01u; /*!< 64b next_pn value: Next packet number to insert into outgoing packet on a particular SA. */
};

class MACsecConfig {
private:
	bool enableRx = false;
	bool enableTx = false;
	bool nvm = false;

	uint8_t maxSecY;
	uint8_t maxSa;
	uint8_t maxRule;
	uint16_t binIndex;
	DeviceType type;

	std::vector<MACsecRxRule> rxRule;
	std::vector<MACsecRxSecY> rxSecY;
	std::vector<MACsecTxSecY> txSecY;
	std::vector<MACsecTxSa> txSa;
	std::vector<MACsecRxSa> rxSa;
	std::vector<std::pair<uint8_t, uint8_t>> txSecYSaIndices;
	std::vector<std::pair<uint8_t, uint8_t>> rxSecYSaIndices;
	std::vector<bool> rxSecYRekey;
	std::vector<bool> txSecYRekey;
	std::vector<uint8_t> rxRuleIndices;
public:
	MACsecConfig() = delete;
	MACsecConfig(const DeviceType& deviceType);

	int addRxSecY(const MACsecRxSecY& secY, uint8_t saIndex);
	int addTxSecY(const MACsecTxSecY& secY, uint8_t saIndex);
	
	int addRxRule(const MACsecRxRule& rule, uint8_t secYIndex);

	int addRxSa(const MACsecRxSa& sa);
	int addTxSa(const MACsecTxSa& sa);

	MACsecRxSecY& getRxSecY(uint8_t secYIndex);
	const MACsecRxSecY& getRxSecY(uint8_t secYIndex) const;
	MACsecTxSecY& getTxSecY(uint8_t secYIndex);
	const MACsecTxSecY& getTxSecY(uint8_t secYIndex) const;

	MACsecRxSa& getRxSa(uint8_t saIndex);
	const MACsecRxSa& getRxSa(uint8_t saIndex) const;
	MACsecTxSa& getTxSa(uint8_t saIndex);
	const MACsecTxSa& getTxSa(uint8_t saIndex) const;

	MACsecRxRule& getRxRule(uint8_t ruleIndex);
	const MACsecRxRule& getRxRule(uint8_t ruleIndex) const;

	bool setTxSaIndex(uint8_t secYIndex, uint8_t saIndex);
	bool enableTxRekey(uint8_t secYIndex, uint8_t rekeySaIndex);
	bool setTxSaRekeyIndex(uint8_t secYIndex, uint8_t saIndex);
	void disableTxRekey(uint8_t secYIndex);

	bool setRxSaIndex(uint8_t secYIndex, uint8_t saIndex);
	bool enableRxRekey(uint8_t secYIndex, uint8_t rekeySaIndex);
	bool setRxSaRekeyIndex(uint8_t secYIndex, uint8_t saIndex);
	void disableRxRekey(uint8_t secYIndex);

	void setRxEnable(bool rxEnable);
	void setTxEnable(bool txEnable);
	void setStorage(bool temporary);
	
	void clear();
	std::vector<uint8_t> serialize() const;
	operator bool() const;
	uint16_t getBinIndex() const;
	DeviceType getType() const;
	uint8_t getMaxNumRule() const;
	uint8_t getMaxNumSecY() const;
	uint8_t getMaxNumSa() const;
};

}

#endif


