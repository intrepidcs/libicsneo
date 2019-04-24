#ifndef _J2534_H
#define _J2534_H

//J2534 Commands
#define J1534_NUM_PERIOD_TX_MSGS 128
#define J1534_NUM_PERIOD_RX_MSGS 128
#define J2534NVCMD_SetISJ2534 0
#define J2534NVCMD_SetISO5Baud 1
#define J2534NVCMD_SetISOFastInit 2
#define J2534NVCMD_SetISOCheckSum 3
#define J2534NVCMD_SetISO9141Parms 4
#define J2534NVCMD_GetISO9141Parms 5
#define J2534NVCMD_ISO9141APIChkSum 6
#define J2534NVCMD_SetNetworkBaudRate 7
#define J2534NVCMD_GetNetworkBaudRate 8
#define J2534NVCMD_EnableTransmitEvent 9
#define J2534NVCMD_SetTransmitEvent 10
#define J2534NVCMD_BlueEnableStopFilters 11
#define J2534NVCMD_Blue15765HWSupport 12
#define J2534NVCMD_GetTXBufferInfo 13
#define J2534NVCMD_GetEncryptionKey 14
#define J2534NVCMD_SetMiscIOForVBATT 15
#define J2534NVCMD_EnableISO_KW_Network 16
#define J2534NVCMD_SetJ1708CheckSum 17
#define J2534NVCMD_GetTimestamp 18
#define J2534NVCMD_GetCANFDRate 19
#define J2534NVCMD_SetCANFDRate 20
#define J2534NVCMD_GetCANFDTermination 21
#define J2534NVCMD_SetCANFDTermination 22
#define J2534NVCMD_GetCANFDFormat 23
#define J2534NVCMD_SetCANFDFormat 24

#endif
