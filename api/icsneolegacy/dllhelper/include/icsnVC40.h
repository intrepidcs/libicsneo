
#include "string.h"
#include <string>
#include <cstdint>

// OpenPort "OpenType" Argument Constants
#define	NEOVI_COMMTYPE_RS232  0
#define	NEOVI_COMMTYPE_USB_BULK	1
#define	NEOVI_COMMTYPE_USB_ISO_DONT_USE	2
#define	NEOVI_COMMTYPE_TCPIP 3

//hardware constants
#define NEODEVICE_UNKNOWN  0
#define NEODEVICE_BLUE  1
#define NEODEVICE_SW_VCAN  2
#define NEODEVICE_DW_VCAN  4
#define NEODEVICE_FIRE  8
#define NEODEVICE_VCAN3  16
#define NEODEVICE_YELLOW  32 //Deprecated
#define NEODEVICE_RED  64
#define NEODEVICE_ECU  128
#define NEODEVICE_IEVB  256
#define NEODEVICE_PENDANT  512
#define NEODEVICE_VIRTUAL_NEOVI  1024
#define NEODEVICE_ECUCHIP_UART  2048
#define NEODEVICE_PLASMA_1_11  0x1000
#define NEODEVICE_PLASMA_1_12  0x10000
#define NEODEVICE_PLASMA_1_13  0x20000
#define NEODEVICE_ANY_PLASMA  0x31000
#define NEODEVICE_FIRE_VNET  0x2000
#define NEODEVICE_NEOANALOG  0x4000
#define NEODEVICE_ION_2  0x40000
#define NEODEVICE_ION_3  0x100000
#define NEODEVICE_ANY_ION  0x140000
#define NEODEVICE_RADSTAR  0x80000
#define NEODEVICE_VCANFD  0x200000
#define NEODEVICE_VCAN412  0x400000
#define NEODEVICE_ECU25  0x800000
#define NEODEVICE_EEVB  0x1000000
#define NEODEVICE_VCANRF  0x2000000
#define NEODEVICE_FIRE2  0x4000000
#define NEODEVICE_FLEX  0x8000000
#define NEODEVICE_RADGALAXY 0x10000000
#define NEODEVICE_RADSTAR2 0x20000000
#define NEODEVICE_VIVIDCAN 0x40000000
#define NEODEVICE_OBD2_SIM 0x80000000
#define NEODEVICE_ALL  0xFFFFBFFF

//device ID's
const int NETID_DEVICE = 0;
const int NETID_HSCAN = 1;
const int NETID_MSCAN = 2;
const int NETID_SWCAN = 3;
const int NETID_LSFTCAN = 4;
const int NETID_FORDSCP = 5;
const int NETID_J1708 = 6;
const int NETID_JVPW = 8;
const int NETID_ISO = 9;
const int NETID_ISO2 = 14;
const int NETID_ISO14230 = 15;
const int NETID_LIN = 16;
const int NETID_ISO3 = 41;
const int NETID_HSCAN2 = 42;
const int NETID_HSCAN3 = 44;
const int NETID_ISO4 = 47;
const int NETID_LIN2 = 48;
const int NETID_LIN3 = 49;
const int NETID_LIN4 = 50;
const int NETID_LIN5 = 84;
const int NETID_MOST = 51;
const int NETID_CGI = 53;
const int NETID_HSCAN4 = 61;
const int NETID_HSCAN5 = 62;
const int NETID_UART = 64;
const int NETID_UART2 = 65;
const int NETID_UART3 = 66;
const int NETID_UART4 = 67;
const int NETID_SWCAN2 = 68; //changed from ANALOG
const int NETID_ETHERNET_DAQ = 69; //NETID_WIZNET - plasma/ ion daq port ethernet
const int NETID_TEXTAPI_TO_HOST = 71;
const int NETID_FLEXRAY1A = 80;
const int NETID_FLEXRAY1B = 81;
const int NETID_FLEXRAY2A = 82;
const int NETID_FLEXRAY2B = 83;
const int NETID_FLEXRAY = 85;
const int NETID_MOST25 = 90;
const int NETID_MOST50 = 91;
const int NETID_MOST150 = 92;
const int NETID_ETHERNET = 93;
const int NETID_GMFSA = 94;
const int NETID_TCP = 95;
const int NETID_HSCAN6 = 96;
const int NETID_HSCAN7 = 97;
const int NETID_LIN6 = 98;
const int NETID_LSFTCAN2 = 99;
const int NETID_OP_ETHERNET1 = 17;
const int NETID_OP_ETHERNET2 = 18;
const int NETID_OP_ETHERNET3 = 19;
const int NETID_OP_ETHERNET4 = 45;
const int NETID_OP_ETHERNET5 = 46;
const int NETID_OP_ETHERNET6 = 73;
const int NETID_OP_ETHERNET7 = 75;
const int NETID_OP_ETHERNET8 = 76;
const int NETID_OP_ETHERNET9 = 77;
const int NETID_OP_ETHERNET10 = 78;
const int NETID_OP_ETHERNET11 = 79;
const int NETID_OP_ETHERNET12 = 87;


// Constants used to calculate the timestamp
const double NEOVI_TIMESTAMP_2 = 0.1048576;
const double NEOVI_TIMESTAMP_1 = 0.0000016;

const double NEOVIPRO_VCAN_TIMESTAMP_2 = 0.065536;
const double NEOVIPRO_VCAN_TIMESTAMP_1 = 0.000001;

const double NEOVI6_VCAN_TIMESTAMP_2 = 0.065536;
const double NEOVI6_VCAN_TIMESTAMP_1 = 0.000001;

const double NEOVI_RED_TIMESTAMP_2_25NS = 107.3741824;
const double NEOVI_RED_TIMESTAMP_1_25NS = 0.000000025;

const double NEOVI_RED_TIMESTAMP_2_10NS = 429.4967296;
const double NEOVI_RED_TIMESTAMP_1_10NS = 0.00000001;

//Status Bitfield 1
const long SPY_STATUS_GLOBAL_ERR = 0x01;
const long SPY_STATUS_TX_MSG = 0x02;
const long SPY_STATUS_XTD_FRAME = 0x04;
const long SPY_STATUS_REMOTE_FRAME = 0x08;

const long SPY_STATUS_CRC_ERROR = 0x10;
const long SPY_STATUS_CAN_ERROR_PASSIVE = 0x20;
const long SPY_STATUS_INCOMPLETE_FRAME = 0x40;
const long SPY_STATUS_LOST_ARBITRATION = 0x80;

const long SPY_STATUS_UNDEFINED_ERROR = 0x100;
const long SPY_STATUS_CAN_BUS_OFF = 0x200;
const long SPY_STATUS_CAN_ERROR_WARNING = 0x400;
const long SPY_STATUS_BUS_SHORTED_PLUS = 0x800;

const long SPY_STATUS_BUS_SHORTED_GND = 0x1000;
const long SPY_STATUS_CHECKSUM_ERROR = 0x2000;
const long SPY_STATUS_BAD_MESSAGE_BIT_TIME_ERROR = 0x4000;
const long SPY_STATUS_IFR_DATA = 0x8000;

const long SPY_STATUS_COMM_IN_OVERFLOW = 0x10000;
const long SPY_STATUS_COMM_OUT_OVERFLOW = 0x20000;
const long SPY_STATUS_COMM_MISC_ERROR = 0x40000;
const long SPY_STATUS_BREAK = 0x80000;

const long SPY_STATUS_AVSI_REC_OVERFLOW = 0x100000;
const long SPY_STATUS_TEST_TRIGGER = 0x200000;
const long SPY_STATUS_AUDIO_COMMENT = 0x400000;
const long SPY_STATUS_GPS_DATA = 0x800000;

const long SPY_STATUS_ANALOG_DIGITAL_INPUT = 0x1000000;
const long SPY_STATUS_TEXT_COMMENT = 0x2000000;
const long SPY_STATUS_NETWORK_MESSAGE_TYPE = 0x4000000;
const long SPY_STATUS_VSI_TX_UNDERRUN = 0x8000000;

const long SPY_STATUS_VSI_IFR_CRC_BIT = 0x10000000;
const long SPY_STATUS_INIT_MESSAGE = 0x20000000;
const long SPY_STATUS_HIGH_SPEED = 0x40000000;
const long SPY_STATUS_FLEXRAY_SECOND_STARTUP_FRAME = 0x40000000;
const long SPY_STATUS_EXTENDED = 0x80000000; // if this bit is set than decode status bitfield3 in the ackbytes

// Configuration Array constants
// high speed CAN neoVI / valuecan baud rate constants
const long NEO_CFG_MPIC_HS_CAN_CNF1 = 512 + 10;
const long NEO_CFG_MPIC_HS_CAN_CNF2 = 512 + 9;
const long NEO_CFG_MPIC_HS_CAN_CNF3 = 512 + 8;
const long NEO_CFG_MPIC_HS_CAN_MODE = 512 + 54;
// med speed CAN
const long NEO_CFG_MPIC_MS_CAN_CNF1 = 512 + 22;
const long NEO_CFG_MPIC_MS_CAN_CNF2 = 512 + 21;
const long NEO_CFG_MPIC_MS_CAN_CNF3 = 512 + 20;
//SW CAN
const long NEO_CFG_MPIC_SW_CAN_CNF1 = 512 + 34;
const long NEO_CFG_MPIC_SW_CAN_CNF2 = 512 + 33;
const long NEO_CFG_MPIC_SW_CAN_CNF3 = 512 + 32;
//LSFT CAN
const long NEO_CFG_MPIC_LSFT_CAN_CNF1 = 512 + 46;
const long NEO_CFG_MPIC_LSFT_CAN_CNF2 = 512 + 45;
const long NEO_CFG_MPIC_LSFT_CAN_CNF3 = 512 + 44;

// The second status bitfield
const long SPY_STATUS2_HAS_VALUE = 1;
const long SPY_STATUS2_VALUE_IS_BOOLEAN = 2;
const long SPY_STATUS2_HIGH_VOLTAGE = 4;
const long SPY_STATUS2_LONG_MESSAGE = 8;
const long SPY_STATUS2_GLOBAL_CHANGE = 0x10000;
const long SPY_STATUS2_ERROR_FRAME = 0x20000;
const long SPY_STATUS2_END_OF_LONG_MESSAGE = 0x100000; //for ISO and J1708;
//LIN/ISO Specific - check protocol before handling 
const long SPY_STATUS2_LIN_ERR_RX_BREAK_NOT_0 = 0x200000;
const long SPY_STATUS2_LIN_ERR_RX_BREAK_TOO_SHORT = 0x400000;
const long SPY_STATUS2_LIN_ERR_RX_SYNC_NOT_55 = 0x800000;
const long SPY_STATUS2_LIN_ERR_RX_DATA_GREATER_8 = 0x1000000;
const long SPY_STATUS2_LIN_ERR_TX_RX_MISMATCH = 0x2000000;
const long SPY_STATUS2_LIN_ERR_MSG_ID_PARITY = 0x4000000;
const long SPY_STATUS2_ISO_FRAME_ERROR = 0x8000000;
const long SPY_STATUS2_LIN_SYNC_FRAME_ERROR = 0x8000000;
const long SPY_STATUS2_ISO_OVERFLOW_ERROR = 0x10000000;
const long SPY_STATUS2_LIN_ID_FRAME_ERROR = 0x10000000;
const long SPY_STATUS2_ISO_PARITY_ERROR = 0x20000000;
const long SPY_STATUS2_LIN_SLAVE_BYTE_ERROR = 0x20000000;
const long SPY_STATUS2_RX_TIMEOUT_ERROR = 0x40000000;
const long SPY_STATUS2_LIN_NO_SLAVE_DATA = 0x80000000;
//MOST Specific - check protocol before handling
const long SPY_STATUS2_MOST_PACKET_DATA = 0x200000;
const long SPY_STATUS2_MOST_STATUS = 0x400000; //reflects changes in light/lock/MPR/SBC/etc.
const long PY_STATUS2_MOST_LOW_LEVEL = 0x800000; //MOST low level message, allocs, deallocs, remote requests.
const long SPY_STATUS2_MOST_CONTROL_DATA = 0x1000000;
const long SPY_STATUS2_MOST_MHP_USER_DATA = 0x2000000; //MOST HIGH User Data Frame
const long SPY_STATUS2_MOST_MHP_CONTROL_DATA = 0x4000000; //MOST HIGH Control Data
const long SPY_STATUS2_MOST_I2S_DUMP = 0x8000000;
const long SPY_STATUS2_MOST_TOO_SHORT = 0x10000000;
const long SPY_STATUS2_MOST_MOST50 = 0x20000000; //absence of MOST50 and MOST150 implies it's MOST25
const long SPY_STATUS2_MOST_MOST150 = 0x40000000;
const long SPY_STATUS2_MOST_CHANGED_PAR = 0x80000000; //first byte in ack reflects what changed
//Ethernet Specific - check protocol before handling
const long SPY_STATUS2_ETHERNET_CRC_ERROR = 0x200000;
const long SPY_STATUS2_ETHERNET_FRAME_TOO_SHORT = 0x400000;
const long SPY_STATUS2_ETHERNET_FCS_AVAILABLE = 0x800000; //This frame contains FCS (4 bytes) obtained from ICS Ethernet hardware (ex. RAD-STAR)

//Spy Status Bitfield 3 constants
const long SPY_STATUS3_LIN_JUST_BREAK_SYNC = 1;
const long SPY_STATUS3_LIN_SLAVE_DATA_TOO_SHORT = 2;
const long SPY_STATUS3_LIN_ONLY_UPDATE_SLAVE_TABLE_ONCE = 4;

//Protocols
const int SPY_PROTOCOL_CUSTOM = 0;
const int SPY_PROTOCOL_CAN = 1;
const int SPY_PROTOCOL_GMLAN = 2;
const int SPY_PROTOCOL_J1850VPW = 3;
const int SPY_PROTOCOL_J1850PWM = 4;
const int SPY_PROTOCOL_ISO9141 = 5;
const int SPY_PROTOCOL_Keyword2000 = 6;
const int SPY_PROTOCOL_GM_ALDL_UART = 7;
const int SPY_PROTOCOL_CHRYSLER_CCD = 8;
const int SPY_PROTOCOL_CHRYSLER_SCI = 9;
const int SPY_PROTOCOL_FORD_UBP = 10;
const int SPY_PROTOCOL_BEAN = 11;
const int SPY_PROTOCOL_LIN = 12;
const int SPY_PROTOCOL_J1708 = 13;
const int SPY_PROTOCOL_CHRYSLER_JVPW = 14;
const int SPY_PROTOCOL_J1939 = 15;
const int SPY_PROTOCOL_FLEXRAY = 16;
const int SPY_PROTOCOL_MOST = 17;
const int SPY_PROTOCOL_CGI = 18;
const int SPY_PROTOCOL_GME_CIM_SCL_KLINE = 19;
const int SPY_PROTOCOL_SPI = 20;
const int SPY_PROTOCOL_I2C = 21;
const int SPY_PROTOCOL_GENERIC_UART = 22;
const int SPY_PROTOCOL_JTAG = 23;
const int SPY_PROTOCOL_UNIO = 24;
const int SPY_PROTOCOL_DALLAS_1WIRE = 25;
const int SPY_PROTOCOL_GENERIC_MANCHSESTER = 26;
const int SPY_PROTOCOL_SENT_PROTOCOL = 27;
const int SPY_PROTOCOL_UART = 28;
const int SPY_PROTOCOL_ETHERNET = 29;
const int SPY_PROTOCOL_CANFD = 30;
const int SPY_PROTOCOL_GMFSA = 31;
const int SPY_PROTOCOL_TCP = 32;


// these are used in status2 for Vehicle Spy 3
const long icsspystatusChangeLength =0x10;
const long icsspystatusChangeBitH1 = 0x20;
const long icsspystatusChangeBitH2 = 0x40;
const long icsspystatusChangeBitH3 = 0x80 ;
const long icsspystatusChangeBitB1 = 0x100;
const long icsspystatusChangeBitB2 = 0x200 ;
const long icsspystatusChangeBitB3 = 0x400 ;
const long icsspystatusChangeBitB4 = 0x800 ;
const long icsspystatusChangeBitB5 = 0x1000;
const long icsspystatusChangeBitB6 = 0x2000;
const long icsspystatusChangeBitB7 = 0x4000;
const long icsspystatusChangeBitB8 = 32768  ;
const long icsspystatusChangedGlobal = 65536 ;

const long SCRIPT_STATUS_STOPPED  = 0;
const long SCRIPT_STATUS_RUNNING = 1;

const long SCRIPT_LOCATION_FLASH_MEM = 0;		//(Valid only on a neoVI Fire or neoVI Red)
const long SCRIPT_LOCATION_SDCARD = 1;		//(Valid only on a neoVI Fire or neoVI Red)
const long SCRIPT_LOCATION_VCAN3_MEM = 2;	 //(Valid only on a ValueCAN 3 device)

//ISO15765 constants
const int ISO15765_2_NETWORK_HSCAN = 0x1;
const int ISO15765_2_NETWORK_MSCAN = 0x2;
const int ISO15765_2_NETWORK_HSCAN2 = 0x4;
const int ISO15765_2_NETWORK_HSCAN3 = 0x8;
const int ISO15765_2_NETWORK_SWCAN = 0x10;
const int ISO15765_2_NETWORK_HSCAN4 = 0x14;
const int ISO15765_2_NETWORK_HSCAN5 = 0x18;
const int ISO15765_2_NETWORK_HSCAN6 = 0x1C;
const int ISO15765_2_NETWORK_HSCAN7 = 0x20;
const int ISO15765_2_NETWORK_SWCAN2 = 0x24;

//Structure for neoVI device types

#pragma pack(push)
#pragma pack(1)
//One byte aligned

typedef struct __declspec(align(1))
{
        int iType;// 1,2,3 for Generation of HW

        // Date and Time type 1 and 2
        int iMainFirmDateDay;
        int iMainFirmDateMonth;
        int iMainFirmDateYear;
        int iMainFirmDateHour;
        int iMainFirmDateMin;
        int iMainFirmDateSecond;
        int iMainFirmChkSum;

        // Version data (only valid for type 3)
        unsigned char iAppMajor;
        unsigned char iAppMinor;
        unsigned char iManufactureDay;
        unsigned char iManufactureMonth;
        unsigned char iManufactureYear;
        unsigned char iBoardRevMajor;
        unsigned char iBoardRevMinor;
        unsigned char iBootLoaderVersionMajor;
        unsigned char iBootLoaderVersionMinor;

        unsigned char iMainVnetHWrevMajor;
        unsigned char iMainVnetHWrevMinor;
        unsigned char iMainVnetSRAMSize;
}stAPIFirmwareInfo;

#pragma pack(pop)



#pragma pack(push)
#pragma pack(2)
//two byte aligned

typedef struct __declspec(align(2))
{
    unsigned char Mode;
    unsigned char SetBaudrate;
    unsigned char Baudrate;
    unsigned char Transceiver_Mode;
    unsigned char TqSeg1;
    unsigned char TqSeg2;
    unsigned char TqProp;
    unsigned char TqSync;
    unsigned short BRP;
    unsigned char auto_baud;
	unsigned char innerFrameDelay25us;
} CAN_SETTINGS;

typedef struct __declspec(align(2))
{
    unsigned char FDMode;     /* mode, secondary baudrate for canfd */
    unsigned char FDBaudrate;
    unsigned char FDTqSeg1;
    unsigned char FDTqSeg2; 
    unsigned char FDTqProp;
    unsigned char FDTqSync;
    unsigned short FDBRP;
    unsigned char FDTDC;
    unsigned char reserved;
}   CANFD_SETTINGS;

typedef struct __declspec(align(2))
{
    unsigned char Mode;
    unsigned char SetBaudrate;
    unsigned char Baudrate;
    unsigned char Transceiver_Mode;
    unsigned char TqSeg1;
    unsigned char TqSeg2;
    unsigned char TqProp;
    unsigned char TqSync;
    unsigned short BRP;
    unsigned short high_speed_auto_switch;
    unsigned char auto_baud;
	unsigned char Reserved; //leave as 0
} SWCAN_SETTINGS;

typedef struct __declspec(align(2))
{
    unsigned int Baudrate;
    unsigned short spbrg;
    unsigned char brgh;
	unsigned char NumBitsDelay;
    unsigned char MasterResistor;
    unsigned char Mode;
} LIN_SETTINGS;

typedef struct __declspec(align(2))
{
	uint16_t monkey;
    unsigned char duplex; /* 0 = half, 1 = full */
    unsigned char link_speed;
    unsigned char auto_neg;
    unsigned char led_mode;
    unsigned char rsvd[4];
}   ETHERNET_SETTINGS;

typedef struct __declspec(align(2))
{
	//CAN
	unsigned int can1_tx_id;
	unsigned int can1_rx_id;
	union
	{
		struct
		{
			unsigned bExtended:1;
			unsigned :15;
		};
		unsigned int DWord;
	}can1_options;
	
	unsigned int can2_tx_id;
	unsigned int can2_rx_id;
	union
	{
		struct
		{
			unsigned bExtended:1;
			unsigned :15;
		};
		unsigned int DWord;
	}can2_options;

        unsigned int network_enables;

	unsigned int can3_tx_id;
	unsigned int can3_rx_id;
	union
	{
		struct
		{
			unsigned bExtended:1;
			unsigned :15;
		};
		unsigned int DWord;
	}can3_options;

	unsigned int can4_tx_id;
	unsigned int can4_rx_id;
	union
	{
		struct
		{
			unsigned bExtended:1;
			unsigned :15;
		};
		unsigned int DWord;
	}can4_options;

	unsigned int reserved[5];
	
}STextAPISettings;

typedef struct __declspec(align(2))
{
    unsigned short time_500us;
    unsigned short k;
    unsigned short l;
}ISO9141_KEYWORD2000__INIT_STEP;

typedef struct __declspec(align(2))
{
    unsigned int Baudrate;
    unsigned short spbrg;
    unsigned short brgh;
    ISO9141_KEYWORD2000__INIT_STEP init_steps[16]; //See the ISO9141_KW2000__INIT_STEP structure
    unsigned char init_step_count;
    unsigned short p2_500us;
    unsigned short p3_500us;
    unsigned short p4_500us;
    unsigned short chksum_enabled;
} ISO9141_KEYWORD2000_SETTINGS;

typedef struct __declspec (align(2))
{    
	uint8_t ucInterfaceType;
    uint8_t reserved0;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
    uint16_t tapPair0;
    uint16_t tapPair1;
    uint16_t tapPair2;
    uint16_t tapPair3;
    uint16_t tapPair4;
    uint16_t tapPair5;
    uint32_t uFlags;
}OP_ETH_GENERAL_SETTINGS; 

typedef struct __declspec (align(2))
{
   uint8_t ucConfigMode;
   uint8_t preemption_en;
   uint8_t reserved0[14];
}OP_ETH_SETTINGS;

typedef struct __declspec (align(2))
{
    uint32_t ecu_id;
    CAN_SETTINGS can1;
    SWCAN_SETTINGS swcan1;
    CAN_SETTINGS lsftcan1;
    uint16_t network_enables;
    uint16_t network_enabled_on_boot;
    uint16_t iso15765_separation_time_offset;
    uint16_t perf_en;
    uint32_t pwr_man_timeout;
    uint16_t pwr_man_enable;
    uint16_t can_switch_mode;
    uint16_t rsvd;
}SVividCANSettings; 

typedef struct __declspec (align(2))
{
	CAN_SETTINGS can1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd1;
	CANFD_SETTINGS canfd2;

	uint64_t network_enables;
	uint16_t network_enabled_on_boot;

	int16_t iso15765_separation_time_offset;

	uint16_t perf_en;

	uint16_t misc_io_initial_ddr;
	uint16_t misc_io_initial_latch;
	uint16_t misc_io_report_period;
	uint16_t misc_io_on_report_events;
	uint16_t misc_io_analog_enable;
	uint16_t ain_sample_period;
	uint16_t ain_threshold;

	struct
	{
		uint32_t : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t reserved : 30;
	} flags;

	STextAPISettings text_api;
} SOBD2SimSettings;

typedef struct __declspec(align(2))
{
        unsigned short perf_en;

        //CAN 
        CAN_SETTINGS can1;
        CAN_SETTINGS can2;
        CANFD_SETTINGS canfd2;
        CAN_SETTINGS can3;
        CANFD_SETTINGS canfd3;
        CAN_SETTINGS can4;
        CAN_SETTINGS can5;
        CAN_SETTINGS can6;
        CAN_SETTINGS can7;
        CAN_SETTINGS can8;

        //Native CAN are either LS1/LS2 or SW1/SW2
        SWCAN_SETTINGS swcan1;
        unsigned short network_enables;
        SWCAN_SETTINGS swcan2;
        unsigned short network_enables_2;

        LIN_SETTINGS lin1;
        unsigned short misc_io_initial_ddr;
        unsigned short misc_io_initial_latch;
        unsigned short misc_io_report_period;
        unsigned short misc_io_on_report_events;
        unsigned short misc_io_analog_enable;
        unsigned short ain_sample_period;
        unsigned short ain_threshold;

        unsigned int pwr_man_timeout;
        unsigned short pwr_man_enable;

        unsigned short network_enabled_on_boot;

        //ISO15765-2 Transport Layer
        unsigned short iso15765_separation_time_offset;

        //ISO9141 - Keyword
        unsigned short iso_9141_kwp_enable_reserved;
        ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
        unsigned short iso_parity_1;

        ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_2;
        unsigned short iso_parity_2;

        ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_3;
        unsigned short iso_parity_3;

        ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_4;
        unsigned short iso_parity_4;

        unsigned short iso_msg_termination_1;
        unsigned short iso_msg_termination_2;
        unsigned short iso_msg_termination_3;
        unsigned short iso_msg_termination_4;

        unsigned short idle_wakeup_network_enables_1;
        unsigned short idle_wakeup_network_enables_2;

        //reserved for HSCAN6/7, LSFT2, etc..
        unsigned short network_enables_3;
        unsigned short idle_wakeup_network_enables_3;

        unsigned short can_switch_mode;
        STextAPISettings text_api;
}SRADGalaxySettings;

typedef struct __declspec (align(2))
{
	/* Performance Test */
	uint16_t perf_en;

	CAN_SETTINGS can1;
	CANFD_SETTINGS canfd1;
	CAN_SETTINGS can2;
	CANFD_SETTINGS canfd2;

	uint64_t network_enables;
	uint64_t termination_enables;

	uint32_t pwr_man_timeout;
	uint16_t pwr_man_enable;

	uint16_t network_enabled_on_boot;

	/* ISO15765-2 Transport Layer */
	int16_t iso15765_separation_time_offset;

	STextAPISettings text_api;
	struct
	{
		uint32_t disableUsbCheckOnBoot : 1;
		uint32_t enableLatencyTest : 1;
		uint32_t reserved : 30;
	} flags;
} SVCAN412Settings;


typedef struct __declspec (align(2))
{
    uint16_t perf_en;
    OP_ETH_GENERAL_SETTINGS opEthGen;
    OP_ETH_SETTINGS opEth1;
    OP_ETH_SETTINGS opEth2;
    CAN_SETTINGS can1;
    CANFD_SETTINGS canfd1;
    CAN_SETTINGS can2;
    CANFD_SETTINGS canfd2;
    uint16_t network_enables;
    uint16_t network_enables_2;
    LIN_SETTINGS lin1;
    uint16_t misc_io_initial_ddr;
    uint16_t misc_io_initial_latch;
    uint16_t misc_io_report_period;
    uint16_t misc_io_on_report_events;
    uint16_t misc_io_analog_enable;
    uint16_t ain_sample_period;
    uint16_t ain_threshold;
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
    uint16_t can_switch_mode;
    STextAPISettings text_api;
}SRADStar2Settings;

    //_stChipVersions
typedef struct __declspec(align(2))
{
        unsigned char mpic_maj;
        unsigned char mpic_min;
        unsigned char upic_maj;
        unsigned char upic_min;
        unsigned char lpic_maj;
        unsigned char lpic_min;
        unsigned char jpic_maj;
        unsigned char jpic_min;
}fire_versions;

typedef struct __declspec(align(2))
{
        unsigned char mpic_maj;
        unsigned char mpic_min;
        unsigned char core_maj;
        unsigned char core_min;
        unsigned char lpic_maj;
        unsigned char lpic_min;
        unsigned char hid_maj;
        unsigned char hid_min;
}plasma_fire_vnet;

typedef struct __declspec(align(2))
{
        unsigned char mpic_maj;
        unsigned char mpic_min;
        unsigned int Reserve;
        unsigned short Reserve2;
}vcan3_versions;

typedef struct __declspec(align(2))
{
        CAN_SETTINGS can1;
        CAN_SETTINGS can2;
        CAN_SETTINGS can3;
        CAN_SETTINGS can4;

        LIN_SETTINGS lin1;
        LIN_SETTINGS lin2;

        unsigned short network_enables;
        unsigned short network_enabled_on_boot;

        unsigned int pwr_man_timeout;
        unsigned short pwr_man_enable; // 0 - off, 1 - sleep enabled, 2- idle enabled (fast wakeup)

        unsigned short misc_io_initial_ddr;
        unsigned short misc_io_initial_latch;
        unsigned short misc_io_analog_enable;
        unsigned short misc_io_report_period;
        unsigned short misc_io_on_report_events;

        //ISO 15765-2 Transport Layer
        short iso15765_separation_time_offset;

        //ISO9141 - KEYWORD 2000 1
        short iso9141_kwp_enable_reserved;
        ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings;

        //Performance Test
        unsigned short perf_en;

        //ISO9141 - Parity
        unsigned short iso_parity; // 0 - no parity, 1 - event, 2 - odd
        unsigned short iso_msg_termination; // 0 - use inner frame time, 1 - GME CIM-SCL
        unsigned short iso_tester_pullup_enable;

        //Additional network enables
        unsigned short network_enables_2;

        ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_2;
        unsigned short iso_parity_2;      // 0 - no parity, 1 - event, 2 - odd
        unsigned short iso_msg_termination_2;     // 0 - use inner frame time, 1 - GME CIM-SCL

        unsigned short idle_wakeup_network_enables_1;
        unsigned short idle_wakeup_network_enables_2;

        unsigned short reservedZero;
}SVCANRFSettings;

typedef struct __declspec(align(2))
{
        unsigned int ecu_id;
        CAN_SETTINGS can1;

        unsigned short network_enables;
        unsigned short network_enabled_on_boot;

        // ISO 15765-2 Transport layer
        unsigned short iso15765_separation_time_offset;

        // Performance test
        unsigned short perf_en;

        // Analog input
        unsigned short ain_sample_period;
        unsigned short ain_threshold;

        unsigned int rsvd;
}SEEVBSettings;

// --- J1708 Settings
typedef struct __declspec(align(2))
{
	unsigned short enable_convert_mode;
} J1708_SETTINGS;


// --- UART Settings
typedef struct __declspec(align(2))
{
	unsigned short Baudrate;
	unsigned short spbrg;
	unsigned short brgh;
	unsigned short parity;
	unsigned short stop_bits;   
	byte flow_control; // 0- off, 1 - Simple CTS RTS,
	byte reserved_1;

	union
	{
		unsigned int bOptions;
		struct
		{
			unsigned invert_tx : 1;
			unsigned invert_rx : 1;
			unsigned half_duplex : 1;
			unsigned reserved_bits : 13;
			unsigned reserved_bits2 : 16;
		};	
	};
} UART_SETTINGS;

 typedef struct __declspec(align(2))
 {
        unsigned short netId; // Netid of CAN network to use.
        unsigned char zero0;
        unsigned char Config;
        //Bit 0: enable bit to enalbe most
        ///Bit 1-3: index of which miscio to use for timestamp sync. 0 => MISC1
        //Bit 4:  Echo to CAN enable
        //Bit 5-7: Reserve
 } SNeoMostGatewaySettings;

typedef struct __declspec(align(2))
{ 
        CAN_SETTINGS can1;
        CAN_SETTINGS can2;
        CAN_SETTINGS can3;
        CAN_SETTINGS can4;
 
        SWCAN_SETTINGS swcan;
        CAN_SETTINGS lsftcan;
 
        LIN_SETTINGS lin1;
        LIN_SETTINGS lin2;
        LIN_SETTINGS lin3;
        LIN_SETTINGS lin4; 

        unsigned short cgi_enable_reserved;
        unsigned short cgi_baud;
        unsigned short cgi_tx_ifs_bit_times;
        unsigned short cgi_rx_ifs_bit_times;  
        unsigned short cgi_chksum_enable;
 
        unsigned short network_enables;
        unsigned short network_enabled_on_boot;
 
        unsigned int pwm_man_timeout;
        unsigned short pwr_man_enable;
 
        unsigned short misc_io_initial_ddr;  
        unsigned short misc_io_initial_latch;
        unsigned short misc_io_analog_enable;
        unsigned short misc_io_report_period;
        unsigned short misc_io_on_report_events;
        unsigned short ain_sample_period;
        unsigned short ain_threshold;
 
        //ISO 15765-2 Transport Layer
        short iso15765_separation_time_offset;
 
        //ISO9141 - KEYWORD 2000
        short iso9141_kwp_enable_reserved;
        ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings;
 
        //Performance Test
        unsigned short perf_en;

        //ISO9141 - Parity
	unsigned short iso_parity; // 0 - no parity, 1 - event, 2 - odd
	unsigned short iso_msg_termination; // 0 - use inner frame time, 1 - GME CIM-SCL
	unsigned short iso_tester_pullup_enable;

	//Additional network enables
    unsigned short network_enables_2;     
	
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_2;
	unsigned short iso_parity_2; 							// 0 - no parity, 1 - event, 2 - odd
	unsigned short iso_msg_termination_2; 				// 0 - use inner frame time, 1 - GME CIM-SCL
	
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_3;
	unsigned short iso_parity_3; 							// 0 - no parity, 1 - event, 2 - odd
	unsigned short iso_msg_termination_3; 				// 0 - use inner frame time, 1 - GME CIM-SCL
	
	ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_4;
	unsigned short iso_parity_4; 							// 0 - no parity, 1 - event, 2 - odd
	unsigned short iso_msg_termination_4; 				// 0 - use inner frame time, 1 - GME CIM-SCL    
	
	unsigned short fast_init_network_enables_1;
	unsigned short fast_init_network_enables_2;
	
	UART_SETTINGS uart;
	UART_SETTINGS uart2;
	
	STextAPISettings text_api;  

	SNeoMostGatewaySettings neoMostGateway;
    unsigned short vnetBits;  //First bit enables Android Messages
}SFireSettings; 

typedef struct __declspec (align(2))
{
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
    SWCAN_SETTINGS swcan1;
    uint16_t network_enables;
    SWCAN_SETTINGS swcan2;
    uint16_t network_enables_2;
    CAN_SETTINGS lsftcan1;
    CAN_SETTINGS lsftcan2;
    LIN_SETTINGS lin1;
    uint16_t misc_io_initial_ddr;
    LIN_SETTINGS lin2;
    uint16_t misc_io_initial_latch;
    LIN_SETTINGS lin3;
    uint16_t misc_io_report_period;
    LIN_SETTINGS lin4;
    uint16_t misc_io_on_report_events;
    LIN_SETTINGS lin5;
    uint16_t misc_io_analog_enable;
    uint16_t ain_sample_period;
    uint16_t ain_threshold;
    uint32_t pwr_man_timeout;
    uint16_t pwr_man_enable;
    uint16_t network_enabled_on_boot;
    uint16_t iso15765_separation_time_offset;
    uint16_t iso_9141_kwp_enable_reserved;
    ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_1;
    uint16_t iso_parity_1;
    ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_2;
    uint16_t iso_parity_2;
    ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_3;
    uint16_t iso_parity_3;
    ISO9141_KEYWORD2000_SETTINGS iso9141_kwp_settings_4;
    uint16_t iso_parity_4;
    uint16_t iso_msg_termination_1;
    uint16_t iso_msg_termination_2;
    uint16_t iso_msg_termination_3;
    uint16_t iso_msg_termination_4;
    uint16_t idle_wakeup_network_enables_1;
    uint16_t idle_wakeup_network_enables_2;
    uint16_t network_enables_3;
    uint16_t idle_wakeup_network_enables_3;
    uint16_t can_switch_mode;
    STextAPISettings text_api;
    uint64_t termination_enables;
    LIN_SETTINGS lin6;
    ETHERNET_SETTINGS ethernet;
    uint16_t slaveVnetA;
    uint16_t slaveVnetB;
    uint32_t flags;
    uint16_t digitalIoThresholdTicks;
    uint16_t digitalIoThresholdEnable;
}SFire2Settings; 

typedef struct __declspec(align(2))
{
    CAN_SETTINGS can1;
    CAN_SETTINGS can2;
    unsigned short  network_enables;
    unsigned short network_enabled_on_boot;
    short iso15765_separation_time_offset;
    unsigned short perf_en;
    unsigned short misc_io_initial_ddr;
    unsigned short misc_io_initial_latch;
    unsigned short misc_io_report_period;
    unsigned short misc_io_on_report_events;
} SVCAN3Settings;


typedef struct __declspec(align(2))  _stCM_ISO157652_TxMessage
{
	//transmit message
	unsigned short vs_netid;	///< The netid of the message (determines which network to transmit on),  not supported

	unsigned char padding;	///< The padding byte to use to fill the unused portion of
							///  transmitted CAN frames (single frame, first frame, consecutive frame),
							///  not supported as it is always 0xAA.
	
	unsigned char	reserved2;

	unsigned int    id;	///< arbId of transmitted frames (CAN id to transmit to).
	unsigned int    fc_id;		///< flow control arb id filter value (response id from receiver).
    unsigned int    fc_id_mask;	///< The flow control arb filter mask (response id from receiver).

	unsigned char	stMin; ///< Overrides the stMin that the receiver reports, see overrideSTmin. Set to J2534's STMIN_TX if <= 0xFF
	unsigned char	blockSize;///< Overrides the block size that the receiver reports, see overrideBlockSize.   Set to J2534's BS_TX if <= 0xFF.
	unsigned char		flowControlExtendedAddress;	///< Expected Extended Address byte of response from receiver.  see fc_ext_address_enable, not supported.
	unsigned char		extendedAddress;			///< Extended Address byte of transmitter. see ext_address_enable, not supported.

	//flow control timeouts
	unsigned short    fs_timeout;				///< max timeout (ms) for waiting on flow control respons, not supported.
	unsigned short    fs_wait;				///< max timeout (ms) for waiting on flow control response that does not have 
											///flow status set to WAIT, not supported.

	unsigned char data[4*1024];	///< The data

	unsigned int num_bytes;	///< Number of data bytes

	//option bits
	union
	{
		struct
		{
			unsigned id_29_bit_enable:1;	///< Enables 29 bit arbId for transmitted frames.  Set to 1 so transmitted frames use 29 bit ids, not supported.
			unsigned fc_id_29_bit_enable:1;	///< Enables 29 bit arbId for Flow Control filter.  Set to 1 if receiver response uses 29 bit ids, not supported.
			unsigned ext_address_enable:1;	///< Enables Extended Addressing, Set to 1 if transmitted frames should have extended addres byte, not supported.
			unsigned fc_ext_address_enable:1;	///< Enables Extended Addressing for Flow Control filter.  Set to 1 if receiver responds with extended address byte, not supported.
	        unsigned overrideSTmin:1; ///< Uses member stMin and not receiver's flow control's stMin.
        	unsigned overrideBlockSize:1; ///< Uses member BlockSize and not receiver's flow control's BlockSize.
			unsigned paddingEnable:1; ///< Enable's padding
            unsigned iscanFD : 1;  //Enables CANFD
			unsigned isBRSEnabled : 1; //Enables bitrate switch
			unsigned Padding: 15;
			unsigned tx_dl : 8;
		};
		unsigned int flags;
	};
      
	
}stCM_ISO157652_TxMessage;



typedef struct __declspec(align(2))
{
	//transmit message
	unsigned short vs_netid;	///< The netid of the message (determines which network to decode receives),  not supported

	unsigned char padding;	///< The padding byte to use to fill the unused portion of
							///  transmitted CAN frames (flow control), see paddingEnable.

	unsigned int    id;			///< ArbId filter value for frames from transmitter (from ECU to neoVI).
	unsigned int    id_mask;	///< ArbId filter mask for frames from transmitter (from ECU to neoVI).
	unsigned int    fc_id;		///< flow control arbId to transmit in flow control (from neoVI to ECU).

	unsigned char		flowControlExtendedAddress;	///< Extended Address byte used in flow control (from neoVI to ECU). see fc_ext_address_enable.
	unsigned char		extendedAddress;	///< Expected Extended Address byte of frames sent by transmitter (from ECU to neoVI).  see ext_address_enable.

	unsigned char		blockSize;		///< Block Size to report in flow control response.
	unsigned char		stMin;		///< Minimum seperation time (between consecutive frames) to report in flow control response.

	//flow control timeouts
	unsigned short    cf_timeout;				///< max timeout (ms) for waiting on consecutive frame.  Set this to N_CR_MAX's value in J2534.

	//option bits
	union
	{
		struct
		{
			unsigned id_29_bit_enable:1;	///< Enables 29 bit arbId filter for frames (from ECU to neoVI).
			unsigned fc_id_29_bit_enable:1;	///< Enables 29 bit arbId for Flow Control (from neoVI to ECU).
			unsigned ext_address_enable:1;	///< Enables Extended Addressing (from ECU to neoVI).
			unsigned fc_ext_address_enable:1;	///< Enables Extended Addressing (from neoVI to ECU).
			unsigned enableFlowControlTransmission:1;	///< Enables Flow Control frame transmission (from neoVI to ECU).
			unsigned paddingEnable:1; ///< Enable's padding
			unsigned iscanFD : 1;  //<Enable CAN FD
			unsigned isBRSEnabled : 1;  //<Enable Baud Rate Switch
		};
		unsigned int flags;
	};

	unsigned char reserved[16];
        
}stCM_ISO157652_RxMessage;




#pragma pack(pop)

typedef struct 
{
	int DeviceType;
	int Handle;
	int NumberOfClients;
	int SerialNumber;
	int MaxAllowedClients;
} NeoDevice;


typedef struct // matching C structure
    {
	unsigned long StatusValue;  // 4
	unsigned long StatusMask;  // 4
	unsigned long Status2Value;  // 4
	unsigned long Status2Mask;  // 4
	unsigned long Header;	// 4
	unsigned long HeaderMask;  // 4
	unsigned long MiscData; // 4
	unsigned long MiscDataMask;  // 4 
	unsigned long ByteDataMSB;	// 4
	unsigned long ByteDataLSB;	// 4
	unsigned long ByteDataMaskMSB;  // 4
	unsigned long ByteDataMaskLSB;  // 4
	unsigned long HeaderLength; // 4 
	unsigned long ByteDataLength; // 4
	unsigned long NetworkID;	// 4
	unsigned short FrameMaster;	// 2
	unsigned char bUseArbIdRangeFilter;
	unsigned char bStuff2;
	unsigned long ExpectedLength;
	unsigned long NodeID;
	}  spyFilterLong;

typedef struct // matching C structure
    {
    unsigned long StatusBitField;	// 4
	unsigned long StatusBitField2;	// 4
    unsigned long TimeHardware;		// 4
	unsigned long TimeHardware2;	// 4
    unsigned long TimeSystem;		// 4
	unsigned long TimeSystem2;		// 4
	unsigned char TimeStampHardwareID;		// 1
	unsigned char TimeStampSystemID;
	unsigned char NetworkID;		// 1
	unsigned char NodeID;
	unsigned char Protocol;
	unsigned char MessagePieceID;	// 1
	unsigned char ExtraDataPtrEnabled;			// 1
    unsigned char NumberBytesHeader;// 1
    unsigned char NumberBytesData;  // 1
	short DescriptionID;			// 2
    long ArbIDOrHeader;				// 4
    unsigned char Data[8];			
	unsigned long StatusBitField3;
    unsigned long StatusBitField4;
    void* ExtraDataPtr;
	unsigned char MiscData;
    }  icsSpyMessage;

typedef struct // matching C structure
    {
    unsigned long StatusBitField;	// 4
	unsigned long StatusBitField2;	// 4
    unsigned long TimeHardware;		// 4
	unsigned long TimeHardware2;	// 4
    unsigned long TimeSystem;		// 4
	unsigned long TimeSystem2;		// 4
	unsigned char TimeStampHardwareID;		// 1
	unsigned char TimeStampSystemID;	// 1
	unsigned char NetworkID;		// 1
	unsigned char NodeID;			// 1
	unsigned char Protocol;			// 1
	unsigned char MessagePieceID;	// 1
	unsigned char ExtraDataPtrEnabled;			// 1
    unsigned char NumberBytesHeader;// 1
    unsigned char NumberBytesData;  // 1
	short DescriptionID;			// 2
    unsigned char Header[4];		// 4
    unsigned char Data[8];
	unsigned long StatusBitField3;
    unsigned long StatusBitField4;
    void* ExtraDataPtr;
	unsigned char MiscData;
    }  icsSpyMessageJ1850;

