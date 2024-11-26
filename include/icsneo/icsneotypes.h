#pragma once

// _Static_assert support, this can be removed once C23 is the "standard"
#if __STDC_VERSION__ < 202311L && !defined(__cplusplus)
#include <assert.h>
#endif

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
    // Unknown device type
    icsneo_devicetype_unknown,
    // neoVI Blue - Obsolete
    icsneo_devicetype_blue,
    // neoECU AVB/TSN
    icsneo_devicetype_ecu_avb,
    // RAD-SuperMoon
    icsneo_devicetype_rad_supermoon,
    // DualWire ValueCAN - Obsolete
    icsneo_devicetype_dw_vcan,
    // RAD-Moon 2
    icsneo_devicetype_rad_moon2,
    // RAD-Mars
    icsneo_devicetype_rad_mars,
    // ValueCAN 4-1
    icsneo_devicetype_vcan41,
    // neoVI FIRE
    icsneo_devicetype_fire,
    // RAD-Pluto
    icsneo_devicetype_rad_pluto,
    // ValueCAN 4-2EL
    icsneo_devicetype_vcan42_el,
    // RAD-IO CAN-HUB
    icsneo_devicetype_radio_canhub,
    // neoECU12
    icsneo_devicetype_neo_ecu12,
    // neoOBD2-LC Badge
    icsneo_devicetype_obd2_lc_badge,
    // RAD-Moon Duo
    icsneo_devicetype_rad_moon_duo,
    // neoVI FIRE3
    icsneo_devicetype_fire3,
    // ValueCAN3
    icsneo_devicetype_vcan3,
    // RAD-Jupiter
    icsneo_devicetype_rad_jupiter,
    // ValueCAN4 Industrial
    icsneo_devicetype_vcan4_industrial,
    // RAD-Gigastar
    icsneo_devicetype_rad_gigastar,
    // neoVI RED2
    icsneo_devicetype_red2,
    // EtherBADGE
    icsneo_devicetype_etherbadge,
    // RAD-A2B
    icsneo_devicetype_rad_a2b,
    // RAD-Epsilon
    icsneo_devicetype_rad_epsilon,
    // RAD-Moon 3
    icsneo_devicetype_rad_moon3,
    // RAD-Comet
    icsneo_devicetype_rad_comet,
    // neoVI FIRE3 FlexRay
    icsneo_devicetype_fire3_flexray,
    // neoVI CONNECT
    icsneo_devicetype_connect,
    // RAD-Comet 3
    icsneo_devicetype_rad_comet3,
    // RAD-Moon T1S
    icsneo_devicetype_rad_moon_t1s,
    // RAD-Gigastar 2
    icsneo_devicetype_rad_gigastar2,
    // neoVI RED
    icsneo_devicetype_red,
    // neoECU - Obsolete
    icsneo_devicetype_ecu,
    // IEVB - Obsolete
    icsneo_devicetype_ievb,
    // Pendant - Obsolete
    icsneo_devicetype_pendant,
    // neoOBD2 Pro - Obsolete
    icsneo_devicetype_obd2_pro,
    // neoECU Chip - Obsolete
    icsneo_devicetype_ecuchip_uart,
    // neoVI PLASMA
    icsneo_devicetype_plasma,
    // neoAnalog - Obsolete
    icsneo_devicetype_neo_analog,
    // Obsolete
    icsneo_devicetype_ct_obd,
    // neoVI ION
    icsneo_devicetype_ion,
    // RAD-Star - Obsolete
    icsneo_devicetype_rad_star,
    // ValueCAN4-4
    icsneo_devicetype_vcan44,
    // ValueCAN4-2
    icsneo_devicetype_vcan42,
    // CMProbe - Obsolete
    icsneo_devicetype_cm_probe,
    // Ethernet EVB - Obsolete
    icsneo_devicetype_eevb,
    // ValueCAN.rf - Obsolete
    icsneo_devicetype_vcan_rf,
    // neoVI FIRE2
    icsneo_devicetype_fire2,
    // neoVI FLEX - Obsolete
    icsneo_devicetype_flex,
    // RAD-Galaxy
    icsneo_devicetype_rad_galaxy,
    // RAD-Star 2
    icsneo_devicetype_rad_star2,
    // VividCAN
    icsneo_devicetype_vividcan,
    // neoOBD2 SIM
    icsneo_devicetype_obd2_sim,


    // Must be last entry
    icsneo_devicetype_maxsize,
} _icsneo_devicetype_t;

typedef uint32_t icsneo_devicetype_t;

// Make sure icsneo_devicetype_t is never smaller than the actual enum
#if __STDC_VERSION__ < 202311L && !defined(__cplusplus)
_Static_assert(sizeof(_icsneo_devicetype_t) <= sizeof(icsneo_devicetype_t));
#else // C++ or C23
static_assert(sizeof(_icsneo_devicetype_t) <= sizeof(icsneo_devicetype_t));
#endif

#ifdef __cplusplus
}
#endif
