#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum _icsneo_open_options_t {
    icsneo_open_options_none = 0x0,
    icsneo_open_options_go_online = 0x1,
    icsneo_open_options_enable_message_polling = 0x2,
    icsneo_open_options_sync_rtc = 0x4,
    icsneo_open_options_enable_auto_update = 0x8,
    icsneo_open_options_force_update = 0x10,
} _icsneo_open_options_t;

typedef uint32_t icsneo_open_options_t;



// This enum used to be a bitfield, but has since become an enum as we have more than 32 devices
// Adding something? Make sure you update the type string and C-compatible defines below!
typedef enum _icsneo_devicetype_t {
    Unknown = (0x00000000),
    BLUE = (0x00000001),
    ECU_AVB = (0x00000002),
    RADSupermoon = (0x00000003),
    DW_VCAN = (0x00000004),
    RADMoon2 = (0x00000005),
    RADMars = (0x00000006),
    VCAN4_1 = (0x00000007),
    FIRE = (0x00000008),
    RADPluto = (0x00000009),
    VCAN4_2EL = (0x0000000a),
    RADIO_CANHUB = (0x0000000b),
    NEOECU12 = (0x0000000c),
    OBD2_LCBADGE = (0x0000000d),
    RADMoonDuo = (0x0000000e),
    FIRE3 = (0x0000000f),
    VCAN3 = (0x00000010),
    RADJupiter = (0x00000011),
    VCAN4_IND = (0x00000012),
    RADGigastar = (0x00000013),
    RED2 = (0x00000014),
    EtherBADGE = (0x00000016),
    RAD_A2B = (0x00000017),
    RADEpsilon = (0x00000018),
    RADMoon3 = (0x00000023),
    RADComet = (0x00000024),
    FIRE3_FlexRay = (0x00000025),
    Connect = (0x00000026),
    RADComet3 = (0x00000027),
    RADMoonT1S = (0x00000028),
    RADGigastar2 = (0x00000029),
    RED = (0x00000040),
    ECU = (0x00000080),
    IEVB = (0x00000100),
    Pendant = (0x00000200),
    OBD2_PRO = (0x00000400),
    ECUChip_UART = (0x00000800),
    PLASMA = (0x00001000),
    DONT_REUSE0 = (0x00002000), // Previously FIRE_VNET
    NEOAnalog = (0x00004000),
    CT_OBD = (0x00008000),
    DONT_REUSE1 = (0x00010000), // Previously PLASMA_1_12
    DONT_REUSE2 = (0x00020000), // Previously PLASMA_1_13
    ION = (0x00040000),
    RADStar = (0x00080000),
    DONT_REUSE3 = (0x00100000), // Previously ION3
    VCAN4_4 = (0x00200000),
    VCAN4_2 = (0x00400000),
    CMProbe = (0x00800000),
    EEVB = (0x01000000),
    VCANrf = (0x02000000),
    FIRE2 = (0x04000000),
    Flex = (0x08000000),
    RADGalaxy = (0x10000000),
    RADStar2 = (0x20000000),
    VividCAN = (0x40000000),
    OBD2_SIM = (0x80000000)
} _icsneo_devicetype_t;

typedef uint64_t icsneo_devicetype_t;

#ifdef __cplusplus
}
#endif
