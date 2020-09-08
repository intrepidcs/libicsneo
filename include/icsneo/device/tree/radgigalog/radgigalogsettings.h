#ifndef __RADGIGALOGSETTINGS_H_
#define __RADGIGALOGSETTINGS_H_

#include <stdint.h>
#include "icsneo/device/idevicesettings.h"

#ifdef __cplusplus

namespace icsneo {

#endif

#pragma pack(push, 2)
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

typedef struct {
	uint32_t ecu_id;

	uint16_t perf_en;

	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;
	CAN_SETTINGS can3;
	CANFD_SETTINGS canfd3;
	CAN_SETTINGS can4;
	CANFD_SETTINGS canfd4;
	CAN_SETTINGS can5;
	CANFD_SETTINGS canfd5;
	CAN_SETTINGS can6;
	CANFD_SETTINGS canfd6;
	CAN_SETTINGS can7;
	CANFD_SETTINGS canfd7;
	CAN_SETTINGS can8;
	CANFD_SETTINGS canfd8;

	uint16_t network_enables;
	uint16_t network_enables_2;

	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;

	uint16_t network_enabled_on_boot;

	uint16_t iso15765_separation_time_offset;

	uint16_t iso_9141_kwp_enable_reserved;
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
	uint16_t iso_parity_1;

	uint16_t iso_msg_termination_1;

	uint16_t idle_wakeup_network_enables_1;
	uint16_t idle_wakeup_network_enables_2;

	uint16_t network_enables_3;
	uint16_t idle_wakeup_network_enables_3;

	STextAPISettings text_api;
	uint64_t termination_enables;
	uint8_t rsvd1[8];// previously ETHERNET_SETTINGS
	uint8_t rsvd2[8];// previously ETHERNET10G_SETTINGS

	DISK_SETTINGS disk;

	TIMESYNC_ICSHARDWARE_SETTINGS timeSyncSettings;
	struct
	{
		uint16_t hwComLatencyTestEn : 1;
		uint16_t disableUsbCheckOnBoot : 1;
		uint16_t reserved : 14;
	} flags;
	ETHERNET_SETTINGS2 ethernet;

	SERDESCAM_SETTINGS serdescam1;
	ETHERNET10G_SETTINGS ethernet10g;

	LIN_SETTINGS lin1;

	SERDESPOC_SETTINGS serdespoc;
	LOGGER_SETTINGS logger;
	SERDESCAM_SETTINGS serdescam2;
	SERDESCAM_SETTINGS serdescam3;
	SERDESCAM_SETTINGS serdescam4;

	ETHERNET_SETTINGS2 ethernet2;
	uint16_t network_enables_4;
	RAD_REPORTING_SETTINGS reporting;
} radgigalog_settings_t;
#pragma pack(pop)

#ifdef __cplusplus

static_assert(sizeof(radgigalog_settings_t) == 666, "RADGigalog settings size mismatch");

#include <iostream>

class RADGigalogSettings : public IDeviceSettings {
public:
	RADGigalogSettings(std::shared_ptr<Communication> com) : IDeviceSettings(com, sizeof(radgigalog_settings_t)) {}
	const CAN_SETTINGS* getCANSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgigalog_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->can1);
			case Network::NetID::MSCAN:
				return &(cfg->can2);
			case Network::NetID::HSCAN2:
				return &(cfg->can3);
			case Network::NetID::HSCAN3:
				return &(cfg->can4);
			case Network::NetID::HSCAN4:
				return &(cfg->can5);
			case Network::NetID::HSCAN5:
				return &(cfg->can6);
			case Network::NetID::HSCAN6:
				return &(cfg->can7);
			case Network::NetID::HSCAN7:
				return &(cfg->can8);
			default:
				return nullptr;
		}
	}
	const CANFD_SETTINGS* getCANFDSettingsFor(Network net) const override {
		auto cfg = getStructurePointer<radgigalog_settings_t>();
		if(cfg == nullptr)
			return nullptr;
		switch(net.getNetID()) {
			case Network::NetID::HSCAN:
				return &(cfg->canfd1);
			case Network::NetID::MSCAN:
				return &(cfg->canfd2);
			case Network::NetID::HSCAN2:
				return &(cfg->canfd3);
			case Network::NetID::HSCAN3:
				return &(cfg->canfd4);
			case Network::NetID::HSCAN4:
				return &(cfg->canfd5);
			case Network::NetID::HSCAN5:
				return &(cfg->canfd6);
			case Network::NetID::HSCAN6:
				return &(cfg->canfd7);
			case Network::NetID::HSCAN7:
				return &(cfg->canfd8);
			default:
				return nullptr;
		}
	}
};

}

#endif // __cplusplus

#endif