#ifndef __IDEVICESETTINGS_H_
#define __IDEVICESETTINGS_H_

#include <stdint.h>
#include "icsneo/platform/unaligned.h"

#pragma pack(push, 2)

/* SetBaudrate in CAN_SETTINGS */
enum
{
	AUTO,
	USE_TQ
};

/* Baudrate in CAN_SETTINGS/CANFD_SETTINGS */
enum CANBaudrate
{
	BPS20,
	BPS33,
	BPS50,
	BPS62,
	BPS83,
	BPS100,
	BPS125,
	BPS250,
	BPS500,
	BPS800,
	BPS1000,
	BPS666,
	BPS2000,
	BPS4000,
	CAN_BPS5000,
	CAN_BPS6667,
	CAN_BPS8000,
	CAN_BPS10000,
};

/* Mode in CAN_SETTINGS */
enum
{
	NORMAL = 0,
	DISABLE = 1,
	LOOPBACK = 2,
	LISTEN_ONLY = 3,
	LISTEN_ALL = 7
};

enum OPEthLinkMode
{
	OPETH_LINK_INVALID = -1,
	OPETH_LINK_AUTO = 0,
	OPETH_LINK_MASTER,
	OPETH_LINK_SLAVE
};

enum EthLinkSpeed
{
	ETH_SPEED_10 = 0,
	ETH_SPEED_100,
	ETH_SPEED_1000,
};

typedef struct
{
	uint16_t networkId;
	uint8_t linkStatus;
	uint8_t linkFullDuplex;
	uint8_t linkSpeed; // see EthLinkSpeed
	int8_t linkMode; // for automotive networks - see OPEthLinkMode
} EthernetNetworkStatus;

typedef struct
{
	uint8_t Mode;
	uint8_t SetBaudrate;
	uint8_t Baudrate;
	uint8_t transceiver_mode;
	uint8_t TqSeg1;
	uint8_t TqSeg2;
	uint8_t TqProp;
	uint8_t TqSync;
	uint16_t BRP;
	uint8_t auto_baud;
	uint8_t innerFrameDelay25us;
} CAN_SETTINGS;
#define CAN_SETTINGS_SIZE 12
static_assert(sizeof(CAN_SETTINGS) == CAN_SETTINGS_SIZE, "CAN_SETTINGS is the wrong size!");

/* FDMode in CANFD_SETTINGS */
enum
{
	NO_CANFD,
	CANFD_ENABLED,
	CANFD_BRS_ENABLED,
	CANFD_ENABLED_ISO,
	CANFD_BRS_ENABLED_ISO
};

typedef struct _CANFD_SETTINGS
{
	uint8_t FDMode; /* mode, secondary baudrate for canfd */
	uint8_t FDBaudrate;
	uint8_t FDTqSeg1;
	uint8_t FDTqSeg2;
	uint8_t FDTqProp;
	uint8_t FDTqSync;
	uint16_t FDBRP;
	uint8_t FDTDC;
	uint8_t reserved;
} CANFD_SETTINGS;
#define CANFD_SETTINGS_SIZE 10

typedef struct ETHERNET_SETTINGS_t
{
	uint8_t duplex; /* 0 = half, 1 = full */
	uint8_t link_speed;
	uint8_t auto_neg;
	uint8_t led_mode;
	uint8_t rsvd[4];
} ETHERNET_SETTINGS;
#define ETHERNET_SETTINGS_SIZE 8

typedef struct OP_ETH_GENERAL_SETTINGS_t
{
	uint8_t ucInterfaceType;
	uint8_t reserved0[3];
	uint16_t tapPair0;
	uint16_t tapPair1;
	uint16_t tapPair2;
	uint16_t tapPair3;
	uint16_t tapPair4;
	uint16_t tapPair5;
	union {
		struct
		{
			unsigned bTapEnSwitch : 1;
			unsigned bTapEnPtp : 1;
			unsigned bEnReportLinkQuality : 1;
		} flags;
		unsigned uFlags;
	};
} OP_ETH_GENERAL_SETTINGS;
#define OP_ETH_GENERAL_SETTINGS_SIZE 20
static_assert(sizeof(OP_ETH_GENERAL_SETTINGS) == OP_ETH_GENERAL_SETTINGS_SIZE, "OP_ETH_GENERAL_SETTINGS is the wrong size!");

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // nameless struct/union
#endif
typedef struct OP_ETH_SETTINGS_t
{
	uint8_t ucConfigMode;
	unsigned char preemption_en;
	union {
		struct {
			// Reuse the mac_addr for switch mode if required!
			unsigned char mac_addr1[6];// Original Addr for spoofing
			unsigned char mac_addr2[6];// Target Addr for spoofing
			unsigned short mac_spoofing_en : 1;
			unsigned short mac_spoofing_isDstOrSrc : 1;
			unsigned short link_spd : 2;
			unsigned short q2112_phy_mode : 1;
			unsigned short reserved : 11;
		};
		unsigned char reserved0[14];
	};
} OP_ETH_SETTINGS;
#define OP_ETH_SETTINGS_SIZE 16
static_assert(sizeof(OP_ETH_SETTINGS) == OP_ETH_SETTINGS_SIZE, "OP_ETH_SETTINGS is the wrong size!");
#ifdef _MSC_VER
#pragma warning(pop)
#endif

typedef struct
{
	uint8_t MasterEnable;
	uint8_t SlaveEnable;
	uint8_t MasterNetwork;
	uint8_t SlaveNetwork;
} TIMESYNC_ICSHARDWARE_SETTINGS;
#define TIMESYNC_ICSHARDWARE_SETTINGS_SIZE 4

typedef struct DISK_SETTINGS_t
{
	uint8_t disk_layout;// RAID0, spanned, etc
	uint8_t disk_format;// FAT32
	uint32_t disk_enables;// mask of enabled disks in this layout
	uint8_t rsvd[8];
} DISK_SETTINGS;
#define DISK_SETTINGS_SIZE 14

#define SERDESCAM_SETTINGS_FLAG_ENABLE 0x01
#define SERDESCAM_SETTINGS_RTSP_ENABLE 0x02
#define SERDESCAM_SETTINGS_AUTO_DETECT_ENABLE 0x04
#define SERDESCAM_SETTINGS_CONFIG_ENABLE 0x08

/* mode in SERDESCAM_SETTINGS */
enum
{
	SERDESCAM_MODE_PASSTHROUGH = 0,
	SERDESCAM_MODE_TAP,
	SERDESCAM_MODE_PLAYBACK,
};

/* ucCamConfigMode in SERDESCAM_SETTINGS */
enum
{
	SERDESCAM_CONFIG_MODE_EXTERNAL_OVER_TAP = 0,
	SERDESCAM_CONFIG_MODE_LOCAL_SCRIPT,
};

/* bitPos in SERDESCAM_SETTINGS */
enum
{
	SERDESCAM_PIXEL_BIT_POS_0 = 0,
	SERDESCAM_PIXEL_BIT_POS_1,
	SERDESCAM_PIXEL_BIT_POS_2,
	SERDESCAM_PIXEL_BIT_POS_3,
};

/* videoFormat in SERDESCAM_SETTINGS */
enum
{
	SERDESCAM_VIDEO_FORMAT_NONE = -1,
	SERDESCAM_VIDEO_FORMAT_UYVY_422_8 = 0, // packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
	SERDESCAM_VIDEO_FORMAT_YUYV_422_8, // packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
	SERDESCAM_VIDEO_FORMAT_YVYU_422_8, // packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb
	SERDESCAM_VIDEO_FORMAT_VYUY_422_8, // packed YUV 4:2:2, 16bpp, Cr Y0 Cb Y1
	SERDESCAM_VIDEO_FORMAT_BAYER_BGGR_8,
	SERDESCAM_VIDEO_FORMAT_RAW_8,  // e.g. bayer 8 bit, gray 8 bit
	SERDESCAM_VIDEO_FORMAT_RAW_10, // e.g. bayer 10 bit, gray 10 bit
	SERDESCAM_VIDEO_FORMAT_RAW_12,
	SERDESCAM_VIDEO_FORMAT_RAW_16, // e.g. planar YUV 4:2:2, 16bpp, 8 bit samples
	SERDESCAM_VIDEO_FORMAT_RAW_20, // e.g. planar YUV 4:2:2, 20bpp, 10 bit samples
	SERDESCAM_VIDEO_FORMAT_RAW_24, // e.g. packed RGB 8:8:8 24bpp, 8 bit samples
	SERDESCAM_VIDEO_FORMAT_RAW_30, // e.g. planar YUV 4:4:4, 30bpp, 10 bit samples
	SERDESCAM_VIDEO_FORMAT_RAW_32, // e.g. packed ARGB 8:8:8:8, 32bpp, 8 bit samples
	SERDESCAM_VIDEO_FORMAT_RAW_36,
	SERDESCAM_VIDEO_FORMAT_RGB888, // packed RGB 8:8:8, 24bpp, RGBRGB...
	SERDESCAM_VIDEO_FORMAT_UYVY_422_10LE_PACKED,// packed YUV 4:2:2, 20bpp, Cb Y0 Cr Y1, bitpacked
	SERDESCAM_VIDEO_FORMAT_YUYV_422_10LE_PACKED,// packed YUV 4:2:2, 20bpp, Y0 Cb Y1 Cr, FOURCC Y210 bitpacked
	SERDESCAM_VIDEO_FORMAT_YVYU_422_10LE_PACKED,// packed YUV 4:2:2, 20bpp, Y0 Cr Y1 Cb, bitpacked
	SERDESCAM_VIDEO_FORMAT_VYUY_422_10LE_PACKED,// packed YUV 4:2:2, 20bpp, Cr Y0 Cb Y1, bitpacked
	SERDESCAM_VIDEO_FORMAT_BAYER_BGGR_10LE_PACKED,// 10-bit samples bitpacked into 40-bits little endian
	SERDESCAM_VIDEO_FORMAT_BAYER_BGGR_12LE_PACKED,// 12-bit samples bitpacked into 40-bits little endian
	SERDESCAM_VIDEO_FORMAT_BAYER_BGGR_16LE,// 16-bit samples little endian
	SERDESCAM_VIDEO_FORMAT_BAYER_BGGR_16BE,// 16-bit samples big endian
	SERDESCAM_VIDEO_FORMAT_JPEG,
	SERDESCAM_VIDEO_FORMAT_UYVY_422_12LE_PACKED,// packed YUV 4:2:2, 24bpp, Cb Y0 Cr Y1, bitpacked
	SERDESCAM_VIDEO_FORMAT_YUYV_422_12LE_PACKED,// packed YUV 4:2:2, 24bpp, Y0 Cb Y1 Cr, bitpacked
	SERDESCAM_VIDEO_FORMAT_YVYU_422_12LE_PACKED,// packed YUV 4:2:2, 24bpp, Y0 Cr Y1 Cb, bitpacked
	SERDESCAM_VIDEO_FORMAT_VYUY_422_12LE_PACKED,// packed YUV 4:2:2, 24bpp, Cr Y0 Cb Y1, bitpacked
	SERDESCAM_VIDEO_FORMAT_YUV422_10LE_PLANAR,// planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	SERDESCAM_VIDEO_FORMAT_YUV422_16LE_PLANAR,// planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
	SERDESCAM_VIDEO_FORMAT_RGB565, // packed RGB 5:6:5, 16bpp, RGBRGB...
	SERDESCAM_VIDEO_FORMAT_RGB666, // packed RGB 6:6:6, 18bpp, RGBRGB...
	SERDESCAM_VIDEO_FORMAT_RAW_11x2,
	SERDESCAM_VIDEO_FORMAT_RAW_12x2,
	SERDESCAM_VIDEO_FORMAT_RAW_14,
	// NOTE: CSI2 formats are only used internal to VSPY
	// Firmware should flag video as CSI2 source types
	// Vspy will then convert formats into the proper CSI2 version
	SERDESCAM_VIDEO_FORMAT_CSI2_UYVY_422_8, // packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
	SERDESCAM_VIDEO_FORMAT_CSI2_YUYV_422_8, // packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
	SERDESCAM_VIDEO_FORMAT_CSI2_YVYU_422_8, // packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb
	SERDESCAM_VIDEO_FORMAT_CSI2_VYUY_422_8, // packed YUV 4:2:2, 16bpp, Cr Y0 Cb Y1
	SERDESCAM_VIDEO_FORMAT_CSI2_UYVY_422_10LE_PACKED,// packed YUV 4:2:2, 20bpp, Cb Y0 Cr Y1, bitpacked
	SERDESCAM_VIDEO_FORMAT_CSI2_YUYV_422_10LE_PACKED,// packed YUV 4:2:2, 20bpp, Y0 Cb Y1 Cr, FOURCC Y210 bitpacked
	SERDESCAM_VIDEO_FORMAT_CSI2_YVYU_422_10LE_PACKED,// packed YUV 4:2:2, 20bpp, Y0 Cr Y1 Cb, bitpacked
	SERDESCAM_VIDEO_FORMAT_CSI2_VYUY_422_10LE_PACKED,// packed YUV 4:2:2, 20bpp, Cr Y0 Cb Y1, bitpacked
	SERDESCAM_VIDEO_FORMAT_CSI2_UYVY_422_12LE_PACKED,// packed YUV 4:2:2, 24bpp, Cb Y0 Cr Y1, bitpacked
	SERDESCAM_VIDEO_FORMAT_CSI2_YUYV_422_12LE_PACKED,// packed YUV 4:2:2, 24bpp, Y0 Cb Y1 Cr, bitpacked
	SERDESCAM_VIDEO_FORMAT_CSI2_YVYU_422_12LE_PACKED,// packed YUV 4:2:2, 24bpp, Y0 Cr Y1 Cb, bitpacked
	SERDESCAM_VIDEO_FORMAT_CSI2_VYUY_422_12LE_PACKED,// packed YUV 4:2:2, 24bpp, Cr Y0 Cb Y1, bitpacked
	SERDESCAM_VIDEO_FORMAT_CSI2_RGB565, // packed RGB 5:6:5, 16bpp, BGRBGR...
	SERDESCAM_VIDEO_FORMAT_CSI2_RGB666, // packed RGB 6:6:6, 18bpp, BGRBGR...
	SERDESCAM_VIDEO_FORMAT_CSI2_RGB888, // packed RGB 8:8:8, 24bpp, BGRBGR...
	SERDESCAM_VIDEO_FORMAT_CSI2_BAYER_BGGR_8,
	SERDESCAM_VIDEO_FORMAT_CSI2_BAYER_BGGR_10LE_PACKED,// 10-bit samples bitpacked into 40-bits little endian
	SERDESCAM_VIDEO_FORMAT_CSI2_BAYER_BGGR_12LE_PACKED,// 12-bit samples bitpacked into 40-bits little endian
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_8,  // e.g. bayer 8 bit, gray 8 bit
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_10, // e.g. bayer 10 bit, gray 10 bit
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_11x2,
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_12,
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_12x2,
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_14,
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_16, // e.g. planar YUV 4:2:2, 16bpp, 8 bit samples
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_20, // e.g. planar YUV 4:2:2, 20bpp, 10 bit samples
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_24, // e.g. packed RGB 8:8:8 24bpp, 8 bit samples
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_30, // e.g. planar YUV 4:4:4, 30bpp, 10 bit samples
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_32, // e.g. packed ARGB 8:8:8:8, 32bpp, 8 bit samples
	SERDESCAM_VIDEO_FORMAT_CSI2_RAW_36,
	// NOTE: new entries must be appended to maintain backwards compatibility
	// insert new entries before this
	SERDESCAM_VIDEO_FORMAT_COUNT,
};

typedef struct SERDESCAM_SETTINGS_t
{
	/* bit0: enable
	 * bit1: RTSP stream enable
	 * bit2: auto detect resolution
	 */
	uint32_t flags;
	uint8_t mode;// passthrough, tap, etc
	uint8_t rsvd1;
	uint8_t bitPos;
	uint8_t videoFormat;// bytes per pixel
	uint16_t resWidth;
	uint16_t resHeight;
	uint8_t frameSkip;// skip every nth frame
	uint8_t rsvd2[19];
} SERDESCAM_SETTINGS;
#define SERDESCAM_SETTINGS_SIZE 32

#define SERDESPOC_SETTINGS_MODE_DISABLED 0x00
#define SERDESPOC_SETTINGS_MODE_SUPPLY 0x01
#define SERDESPOC_SETTINGS_MODE_SERIALIZER 0x02

typedef struct SERDESPOC_SETTINGS_t
{
	uint8_t mode;// no poc, generated supply, serializer passthrough
	uint8_t rsvd[6];
	uint8_t voltage;// generated voltage
	uint16_t chksum;// checksum to protect settings structure (don't want corrupt voltage settings)
} SERDESPOC_SETTINGS;
#define SERDESPOC_SETTINGS_SIZE 10

#define ETHERNET_SETTINGS2_FLAG_FULL_DUPLEX  0x01
#define ETHERNET_SETTINGS2_FLAG_AUTO_NEG     0x02
#define ETHERNET_SETTINGS2_FLAG_TCPIP_ENABLE 0x04
#define ETHERNET_SETTINGS2_FLAG_RTSP_ENABLE  0x08
#define ETHERNET_SETTINGS2_FLAG_DEVICE_HOSTING_ENABLE  0x10
#define ETHERNET_SETTINGS2_FLAG_COMM_IN_USE  0x80

typedef struct ETHERNET_SETTINGS2_t
{
	/* bit0: 0=half duplex, 1=full duplex
	 * bit1: auto negot
	 * bit2: enable tcp/ip stack
	 * bit3: enable rtsp server
	 * bit4: enable intepid device hosting (go online and log other devices)
	 */
	uint8_t flags;
	uint8_t link_speed;//0=10, 1=100, 2=1000
	uint32_t ip_addr;
	uint32_t netmask;
	uint32_t gateway;
	uint8_t rsvd[2];
} ETHERNET_SETTINGS2;
#define ETHERNET_SETTINGS2_SIZE 16

typedef struct ETHERNET10T1S_SETTINGS_t
{
	uint8_t max_burst_count;
	uint8_t burst_timer;
	uint8_t max_num_nodes;
	uint8_t local_id;
	uint8_t to_timer;
	uint8_t flags;
	uint8_t local_id_alternate;
	uint8_t rsvd[5];
} ETHERNET10T1S_SETTINGS;
#define ETHERNET10T1S_SETTINGS_SIZE 12

#define ETHERNET10T1S_SETTINGS_FLAG_ENABLE_PLCA 0x01
#define ETHERNET10T1S_SETTINGS_FLAG_TERMINATION 0x02

#define ETHERNET_SETTINGS10G_FLAG_FULL_DUPLEX  0x01
#define ETHERNET_SETTINGS10G_FLAG_AUTO_NEG     0x02
#define ETHERNET_SETTINGS10G_FLAG_TCPIP_ENABLE 0x04
#define ETHERNET_SETTINGS10G_FLAG_RTSP_ENABLE  0x08
#define ETHERNET_SETTINGS10G_FLAG_DEVICE_HOSTING_ENABLE  0x10
#define ETHERNET_SETTINGS10G_FLAG_COMM_IN_USE  0x80000000

typedef struct ETHERNET10G_SETTINGS_t
{
	/* bit0: 0=half duplex, 1=full duplex
	 * bit1: auto negot
	 * bit2: enable tcp/ip stack
	 * bit3: enable rtsp server
	 * bit4: enable intepid device hosting (go online and log other devices)
	 * bit31: comm in use
	 */
	uint32_t flags;
	uint32_t ip_addr;
	uint32_t netmask;
	uint32_t gateway;
	uint8_t link_speed;//0=10, 1=100, 2=1000, 3=2500, 4=5000, 5=10000
	uint8_t rsvd2[7];
} ETHERNET10G_SETTINGS;
#define ETHERNET10G_SETTINGS_SIZE 24

typedef struct LOGGER_SETTINGS_t
{
	/* bit6-0: timeout in seconds
	 * bit7: 1=disable coremini/logging during extraction for timeout
	 */
	uint8_t extraction_timeout;
	uint8_t rsvd[3];
} LOGGER_SETTINGS;
#define LOGGER_SETTINGS_SIZE 4

#define RAD_GPTP_NUM_PORTS 1 // 1 because only supported as gPTP endpoint
typedef struct RAD_GPTP_SETTINGS_t
{
	uint32_t neighborPropDelayThresh;//ns
	uint32_t sys_phc_sync_interval;//ns
	int8_t logPDelayReqInterval;// log2ms
	int8_t logSyncInterval;// log2ms
	int8_t logAnnounceInterval;// log2ms
	uint8_t profile;
	uint8_t priority1;
	uint8_t clockclass;
	uint8_t clockaccuracy;
	uint8_t priority2;
	uint16_t offset_scaled_log_variance;
	uint8_t gPTPportRole[RAD_GPTP_NUM_PORTS];
	uint8_t portEnable[RAD_GPTP_NUM_PORTS];
	uint8_t rsvd[16];
} RAD_GPTP_SETTINGS;//36 Bytes with RAD_GPTP_NUM_PORTS = 1
#define RAD_GPTP_SETTINGS_SIZE 36

#define RAD_REPORTING_SETTINGS_FLAG_TEMP_ENABLE 		0x00000001
#define RAD_REPORTING_SETTINGS_FLAG_MIC2_GPS_ENABLE 	0x00000002 // USB port 1
#define RAD_REPORTING_SETTINGS_FLAG_INT_GPS_ENABLE 		0x00000004
#define RAD_REPORTING_SETTINGS_FLAG_MIC2_GPS_ENABLE2 	0x00000008 // USB port 2
#define RAD_REPORTING_SETTINGS_FLAG_MISC1_DIN 			0x00000010
#define RAD_REPORTING_SETTINGS_FLAG_MISC2_DIN 			0x00000020
#define RAD_REPORTING_SETTINGS_FLAG_MISC1_PWMIN 		0x00000040
#define RAD_REPORTING_SETTINGS_FLAG_MISC2_PWMIN 		0x00000080
#define RAD_REPORTING_SETTINGS_FLAG_AIN1 				0x00000100
#define RAD_REPORTING_SETTINGS_FLAG_AIN2 				0x00000200

typedef struct RAD_REPORTING_SETTINGS_t
{
	uint32_t flags;
	uint16_t temp_interval_ms;
	uint16_t gps_interval_ms;
	uint16_t serdes_interval_ms;
	uint16_t io_interval_ms;
	uint8_t rsvd[4];
} RAD_REPORTING_SETTINGS;
#define RAD_REPORTING_SETTINGS_SIZE 16

typedef struct _STextAPISettings
{
	uint32_t can1_tx_id;
	uint32_t can1_rx_id;
	union {
		struct sCAN1Options
		{
			unsigned bExtended : 1;
			unsigned : 15;
		};
		uint32_t DWord;
	} can1_options;
	uint32_t can2_tx_id;
	uint32_t can2_rx_id;
	union {
		struct sCAN2Options
		{
			unsigned bExtended : 1;
			unsigned : 15;
		};
		uint32_t DWord;
	} can2_options;

	uint32_t network_enables;

	uint32_t can3_tx_id;
	uint32_t can3_rx_id;
	union {
		struct sCAN3Options
		{
			unsigned bExtended : 1;
			unsigned : 15;
		};
		uint32_t DWord;
	} can3_options;

	uint32_t can4_tx_id;
	uint32_t can4_rx_id;
	union {
		struct sCAN4Options
		{
			unsigned bExtended : 1;
			unsigned : 15;
		};
		uint32_t DWord;
	} can4_options;

	uint32_t reserved[5];

} STextAPISettings;
#define STextAPISettings_SIZE 72

/* high_speed_auto_switch in SWCAN_SETTINGS */
enum
{
	SWCAN_AUTOSWITCH_DISABLED,
	SWCAN_AUTOSWITCH_NO_RESISTOR,
	SWCAN_AUTOSWITCH_WITH_RESISTOR,
	SWCAN_AUTOSWITCH_DISABLED_RESISTOR_ENABLED
};

typedef struct
{
	uint8_t Mode;
	uint8_t SetBaudrate;
	uint8_t Baudrate;
	uint8_t transceiver_mode;
	uint8_t TqSeg1;
	uint8_t TqSeg2;
	uint8_t TqProp;
	uint8_t TqSync;
	uint16_t BRP;
	uint16_t high_speed_auto_switch;
	uint8_t auto_baud;
	uint8_t RESERVED;
} SWCAN_SETTINGS;
#define SWCAN_SETTINGS_SIZE 14

/* Baudrate in LIN_SETTINGS / ISO9141_KEYWORD2000_SETTINGS / UART_SETTINGS */
enum
{
	BPS5000,
	BPS10400,
	BPS33333,
	BPS50000,
	BPS62500,
	BPS71429,
	BPS83333,
	BPS100000,
	BPS117647
};

/* MasterResistor in LIN_SETTINGS */
enum
{
	RESISTOR_ON,
	RESISTOR_OFF
};

/* Mode in LIN_SETTINGS */
enum
{
	SLEEP_MODE,
	SLOW_MODE,
	NORMAL_MODE,
	FAST_MODE
};

typedef struct _LIN_SETTINGS
{
	uint32_t Baudrate; /* New products since FIREVNETEP should rely on this only */
	uint16_t spbrg; /* Precompiled to be 40Mhz/Baudrate/16 - 1.  Only used in neoVI FIRE/FIREVNET(4dw) */
	uint8_t brgh; /* Must be zero */
	uint8_t numBitsDelay;
	uint8_t MasterResistor;
	uint8_t Mode;
} LIN_SETTINGS;
#define LIN_SETTINGS_SIZE 10

typedef struct
{
	uint16_t time_500us;
	uint16_t k;
	uint16_t l;
} ISO9141_KEYWORD2000__INIT_STEP;
#define ISO9141_KEYWORD2000__INIT_STEP_SIZE 6

typedef struct
{
	uint32_t Baudrate;
	uint16_t spbrg;
	uint16_t brgh;
	ISO9141_KEYWORD2000__INIT_STEP init_steps[16];
	uint8_t init_step_count;
	uint16_t p2_500us;
	uint16_t p3_500us;
	uint16_t p4_500us;
	uint16_t chksum_enabled;
} ISO9141_KEYWORD2000_SETTINGS;
#define ISO9141_KEYWORD2000_SETTINGS_SIZE 114
static_assert(sizeof(ISO9141_KEYWORD2000_SETTINGS) == ISO9141_KEYWORD2000_SETTINGS_SIZE, "ISO9141_KEYWORD2000_SETTINGS is the wrong size!");

typedef struct _UART_SETTINGS
{
	uint16_t Baudrate;
	uint16_t spbrg;
	uint16_t brgh;
	uint16_t parity;
	uint16_t stop_bits;
	uint8_t flow_control; /* 0- off, 1 - Simple CTS RTS */
	uint8_t reserved_1;
	union {
		uint32_t bOptions;
		struct {
			uint32_t invert_tx : 1;
			uint32_t invert_rx : 1;
			uint32_t half_duplex : 1;
			uint32_t reserved_bits : 13;
			uint32_t reserved_bits2 : 16;
		} sOptions;
	};
} UART_SETTINGS;
#define UART_SETTINGS_SIZE 16
static_assert(sizeof(UART_SETTINGS) == UART_SETTINGS_SIZE, "UART_SETTINGS is the wrong size!");

typedef struct {
	uint8_t ethernetActivationLineEnabled;
	EthernetNetworkStatus ethernetStatus;
	uint8_t unused;
} fire2vnet_status_t, flexray_vnetz_status_t;

typedef struct
{
	uint8_t allowBoot; // 0 - disable booting Linux, 1 - enable booting Linux, Others - Disable booting linux
	uint8_t useExternalWifiAntenna; // 0 for internal, 1 for external, Others - Internal
	uint8_t
		ethConfigurationPort; // 0 - both ports used by logger, 1 - ETH 01 for Linux Configuration, 2 - ETH 02 for Linux Configuration, Others - both ports used by logger. See defines above
	uint8_t reserved[5];
} Fire3LinuxSettings;
#define FIRE3LINUXSETTINGS_SIZE 8
static_assert(sizeof(Fire3LinuxSettings) == FIRE3LINUXSETTINGS_SIZE, "Fire3LinuxSettings is the wrong size!");
#pragma pack(pop)

#ifdef __cplusplus
#include "icsneo/communication/communication.h"
#include <optional>
#include <iostream>
#include <atomic>

namespace icsneo {

class IDeviceSettings {
public:
	using TerminationGroup = std::vector<Network>;

	static constexpr uint16_t GS_VERSION = 5;
	static std::optional<uint16_t> CalculateGSChecksum(const std::vector<uint8_t>& settings, std::optional<size_t> knownSize = std::nullopt);
	static CANBaudrate GetEnumValueForBaudrate(int64_t baudrate);
	static int64_t GetBaudrateValueForEnum(CANBaudrate enumValue);

	IDeviceSettings(std::shared_ptr<Communication> com, size_t size) : com(com), report(com->report), structSize(size) {}
	virtual ~IDeviceSettings() {}
	bool ok() { return !disabled && settingsLoaded; }

	virtual bool refresh(bool ignoreChecksum = false); // Get from device

	// Send to device, if temporary device keeps settings in volatile RAM until power cycle, otherwise saved to EEPROM
	virtual bool apply(bool temporary = false);
	bool applyDefaults(bool temporary = false);

	virtual int64_t getBaudrateFor(Network net) const;
	virtual bool setBaudrateFor(Network net, int64_t baudrate);

	virtual int64_t getFDBaudrateFor(Network net) const;
	virtual bool setFDBaudrateFor(Network net, int64_t baudrate);

	virtual const CAN_SETTINGS* getCANSettingsFor(Network net) const { (void)net; return nullptr; }
	CAN_SETTINGS* getMutableCANSettingsFor(Network net) {
		if(disabled || readonly)
			return nullptr;
		const uint8_t* offset = (const uint8_t*)getCANSettingsFor(net);
		if(offset == nullptr)
			return nullptr;
		return reinterpret_cast<CAN_SETTINGS*>((void*)(settings.data() + (offset - settingsInDeviceRAM.data())));
	}

	virtual const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const { (void)net; return nullptr; }
	CANFD_SETTINGS* getMutableCANFDSettingsFor(Network net) {
		if(disabled || readonly)
			return nullptr;
		const uint8_t* offset = (const uint8_t*)getCANFDSettingsFor(net);
		if(offset == nullptr)
			return nullptr;
		return reinterpret_cast<CANFD_SETTINGS*>((void*)(settings.data() + (offset - settingsInDeviceRAM.data())));
	}

	virtual const CAN_SETTINGS* getLSFTCANSettingsFor(Network net) const { (void)net; return nullptr; }
	CAN_SETTINGS* getMutableLSFTCANSettingsFor(Network net) {
		if(disabled || readonly)
			return nullptr;
		const uint8_t* offset = (const uint8_t*)getLSFTCANSettingsFor(net);
		if(offset == nullptr)
			return nullptr;
		return reinterpret_cast<CAN_SETTINGS*>((void*)(settings.data() + (offset - settingsInDeviceRAM.data())));
	}

	virtual const SWCAN_SETTINGS* getSWCANSettingsFor(Network net) const { (void)net; return nullptr; }
	SWCAN_SETTINGS* getMutableSWCANSettingsFor(Network net) {
		if(disabled || readonly)
			return nullptr;
		const uint8_t* offset = (const uint8_t*)getSWCANSettingsFor(net);
		if(offset == nullptr)
			return nullptr;
		return reinterpret_cast<SWCAN_SETTINGS*>((void*)(settings.data() + (offset - settingsInDeviceRAM.data())));
	}

	/**
	 * Some devices have groupings of networks, where software
	 * switchable termination can only be applied to one network
	 * in the group at a time. This function returns those groups
	 * for the given device.
	 *
	 * If a device does not support CAN Termination, an empty vector
	 * is returned.
	 */
	virtual std::vector<TerminationGroup> getTerminationGroups() const { return {}; }

	/**
	 * Check whether software switchable termination is supported
	 * for a given network on this device.
	 *
	 * This does not check whether another network in the termination
	 * group has termination enabled, check canTerminationBeEnabledFor
	 * for that.
	 */
	bool isTerminationSupportedFor(Network net) const;

	/**
	 * Check whether software switchable termination can currently
	 * be enabled for a given network. If another network in the
	 * group is already enabled, or if termination is not supported
	 * on this network, false is returned and an error will have
	 * been reported in icsneo::getLastError().
	 */
	bool canTerminationBeEnabledFor(Network net) const;

	/**
	 * Check whether software switchable termination is currently
	 * enabled for a given network in the currently active device settings.
	 *
	 * Note that if the termination status is set, but not yet
	 * applied to the device, the current device status will be
	 * reflected here rather than the pending status.
	 */
	std::optional<bool> isTerminationEnabledFor(Network net) const;

	/**
	 * Enable or disable software switchable termination for a
	 * given network.
	 *
	 * All other networks in the termination group must be disabled
	 * prior to the call, but the change does not need to be applied
	 * to the device before enqueing the enable.
	 *
	 * Returns true if the call was successful, otherwise an error
	 * will have been reported in icsneo::getLastError().
	 */
	bool setTerminationFor(Network net, bool enabled);

	const void* getRawStructurePointer() const { return settingsInDeviceRAM.data(); }
	void* getMutableRawStructurePointer() { return settings.data(); }
	template<typename T> const T* getStructurePointer() const { return reinterpret_cast<const T*>(getRawStructurePointer()); }
	template<typename T> T* getMutableStructurePointer() { return reinterpret_cast<T*>(getMutableRawStructurePointer()); }
	template<typename T> T getStructure() const { return *getStructurePointer<T>(); }
	template<typename T> bool applyStructure(const T& newStructure);

	const size_t& getSize() const { return structSize; }

	// if settings are disabled for this device. always false unless constructed null
	bool disabled = false;

	bool readonly = false;
	bool disableGSChecksumming = false;

	std::atomic<bool> applyingSettings{false};
protected:
	std::shared_ptr<Communication> com;
	device_eventhandler_t report;
	size_t structSize;

	// if we hold any local copies of the device settings
	bool settingsLoaded = false;

	std::vector<uint8_t> settings; // For writing settings to, calling apply() should copy over to device RAM (and EEPROM)
	std::vector<uint8_t> settingsInDeviceRAM; // For reading settings from

	// Parameter createInoperableSettings exists because it is serving as a warning that you probably don't want to do this
	typedef void* warn_t;
	IDeviceSettings(warn_t createInoperableSettings, std::shared_ptr<Communication> com)
		: disabled(true), readonly(true), report(com->report), structSize(0) { (void)createInoperableSettings; }

	virtual ICSNEO_UNALIGNED(const uint64_t*) getTerminationEnables() const { return nullptr; }
	virtual ICSNEO_UNALIGNED(uint64_t*) getMutableTerminationEnables() {
		if(disabled || readonly)
			return nullptr;
		const auto offset = reinterpret_cast<ICSNEO_UNALIGNED(const uint8_t*)>(getTerminationEnables());
		if(offset == nullptr)
			return nullptr;
		return reinterpret_cast<ICSNEO_UNALIGNED(uint64_t*)>((void*)(settings.data() + (offset - settingsInDeviceRAM.data())));
	}
};

}

#endif // __cplusplus

#endif