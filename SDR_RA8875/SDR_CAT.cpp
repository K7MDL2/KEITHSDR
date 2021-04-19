//
//  SDR_CAT.cpp
//
//  Serial port control protocol useful for control head and panadapter information exchange.
//
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"
#include "SDR_CAT.h"

#ifdef FT817_CAT
  FT817 ft817;  // assign our class id
#endif

#ifdef ALL_CAT
  #define CAT_Serial Serial6
#endif

#ifdef PANADAPTER
Metro meter_update    = Metro(550);   // used to update CAT port info
#endif

void init_CAT_comms(void);
void CAT_handler();
void BandDecoderInput();
void BandDecoderOutput();
void LcdDisplay();
void watchDog();
void FrequencyRequest();
void PttOff();
void FreqToBandRules();
void serialEcho();
void Filter_Request(void);
void Filter_Decode(void);
void BarGraph_Request(void);
void BarGraph_Decode(void);
void VFOA_Request(void);
void VFOA_Decode(void);
void VFOB_Request(void);
void VFOB_Decode(void);
int16_t CAT_msgs(void);
void AGC_Decode(void);
void AGC_Request(void);
void ANT_Request(void);
void ANT_Decode(void);
void IF_Center_Request(void);
void IF_Center_Decode(void);
void RadioMode_Request(void);
void RadioMode_Decode(void);

//void bandSET();
extern struct   Band_Memory         bandmem[];
extern          uint8_t             curr_band;   // global tracks our current band setting.  
extern struct 	User_Settings 		user_settings[];
extern 			uint8_t 			user_Profile;
extern          uint32_t            VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern          uint32_t            VFOB;
extern			int32_t     		Fc;  
int16_t barGraph  	= 0;   // global for remtoe meter value for Rx and Tx from radio
int16_t filterWidth = 0;   // 4 digit fitler width.  Convert to suitable display label text in panadapter mode
#define S_BUFF 500
byte Ser_Buff[S_BUFF];
const char msg_type_array[8][3] = {"FA","FB","IF","BG","BW","PA","FR","FT"};
static char msg[S_BUFF];

#ifdef FT817_CAT

// set this to the hardware serial port you wish to use
COLD void init_CAT_comms(void)
{
    #ifdef CAT_Serial 
      Serial1.begin(38400);
      //setSerial(6);
    #endif
    #ifdef FT817_CAT
      ft817.begin(19200);
    #endif
}

COLD void print_CAT_status(void)
{
    CAT_Serial.print(F("FT-817 S-meter:")); CAT_Serial.println(ft817.getSMeter());
    CAT_Serial.print(F("FT-817 Active VFO Frequency:")); CAT_Serial.println(ft817.getVFO());	  // get acxtual VF)
    CAT_Serial.print(F("FT-817 Band VFO:")); CAT_Serial.println(ft817.getBandVFO(0)); // 0 is VFOA on FT817, 1 is VFOB
    CAT_Serial.print(F("FT-817 Frequency and Mode:")); CAT_Serial.println(ft817.getFreqMode()); // get frequency and mode
    CAT_Serial.print(F("FT-817 Mode:")); CAT_Serial.println(ft817.getMode());	
}
#endif  //  FT817_CAT


#ifdef ALL_CAT
//#include <Arduino.h>
const char* REV = "20210215-K7MDL";
/*
// Modified 4/2021 for SDR/Panadapter usage by K7MDL

  Band decoder MK2 with TRX control output for Arduino
-----------------------------------------------------------
  https://remoteqth.com/wiki/index.php?page=Band+decoder+MK2

  ___               _        ___ _____ _  _
 | _ \___ _ __  ___| |_ ___ / _ \_   _| || |  __ ___ _ __
 |   / -_) '  \/ _ \  _/ -_) (_) || | | __ |_/ _/ _ \ '  \
 |_|_\___|_|_|_\___/\__\___|\__\_\|_| |_||_(_)__\___/_|_|_|


    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

Features:
Support inputs
  * PTT detector - if PTT on, outputs not change
  * SERIAL terminal (ASCII)
  * ICOM CI-V
  * KENWOOD - CAT
  * FLEX 6000 CAT series
  * YAESU / ELECRAFT BCD
  * ICOM ACC voltage
  * YAESU CAT - TRX since 2008 ascii format
  * YAESU CAT old binary format (tested on FT-817)
  * IP relay with automatic pair by rotary encoder ID https://remoteqth.com/wiki/index.php?page=IP+Switch+with+ESP32-GATEWAY

Outputs
  * 14 local relay
  * 14 remote relay
  * Yaesu BCD
  * Serial echo
  * Icom CIV
  * Kenwood CAT
  * YAESU CAT - TRX since 2008 ascii format
  * IP relay with automatic pair by rotary encoder ID
  * PTT by band - distributed PTT to output, dependency by frequency - TNX Tim W4YN
  * Analog (PWM) output by  preset table

  Major changes
  -------------
  - LCD support
  - Icom with request mode
  - PTT input block changes during transmit
  - own board with all smd parts including arduino nano module
  - without relays, only control driver outputs
  - optional ethernet module

  Changelog
  ---------
  2021-02 add 23cm (IC9700) Icom state machine
  2020-10 PWM (analog) output
  2020-07 PTT by band
  2020-02 fix out set
  2019-11 YAESU / ELECRAFT input BCD format
  2018-03 manual switch between four output on same band by BCD input - TNX ZS1LS behind inspiration
  2018-12 PTT bug fix
          LCD PCF8574 support
          copy YAESU CAT from old code
          added YAESU FT-100 support
  2018-11 support FLEX 6000 CAT series
  2018-06 support IP relay
  2018-05 mk2 initial release

*/
//=====[ Inputs ]=============================================================================================

// #define INPUT_BCD            // TTL BCD in A
//int BcdInputFormat = 0;         // if enable INPUT_BCD, set format 0 - YAESU, 1 ELECRAFT
// #define ICOM_ACC             // voltage 0-8V on pin4 ACC(2) connector - need calibrate table
// #define INPUT_SERIAL         // telnet ascii input - cvs format [band],[freq]\n
//#define ICOM_CIV             // read frequency from CIV
#define KENWOOD_PC           // RS232 CAT
// #define FLEX_6000            // RS232 CAT
// #define YAESU_CAT            // RS232 CAT YAESU CAT since 2015 ascii format
// #define YAESU_CAT_OLD        // Old binary format RS232 CAT ** tested on FT-817 **
// #define YAESU_CAT_FT100      // RS232 CAT YAESU for FT100(D)
// #define MULTI_OUTPUT_BY_BCD  // manual switch between four output on same band by BCD input
                                // - INPUT_BCD input must be disable
                                // - BCD output will be disble
                                // - it must always be select (grounded) one of BCD input
//=====[ Outputs ]============================================================================================
//   If enable:
// - baudrate is same as selected Inputs
// - Inputs work only in 'sniff mode'
// - for operation must disable REQUEST
//
// #define ICOM_CIV_OUT       // send frequency to CIV ** you must set TRX CIV_ADRESS **
// #define KENWOOD_PC_OUT     // send frequency to RS232 CAT
// #define YAESU_CAT_OUT      // send frequency to RS232 CAT ** for operation must disable REQUEST **
// #define SERIAL_echo        // Feedback on serial line in same baudrate, CVS format <[band],[freq]>\n
// #define PTT_BY_BAND        // distributed PTT dependency to band (disable band decoder outputs) TNX Tim W4YN for idea
// #define PWM_OUT               // PWM on D5 with rc filter (10k/10u) represent analog output J2.12

//=====[ Hardware ]=============================================================================================

//#define LCD                      // Uncoment to Enable I2C LCD
//const byte LcdI2Caddress = 0x27; // 0x27 0x3F - may be find with I2C scanner https://playground.arduino.cc/Main/I2cScanner
//#define LCD_PCF8574              // If LCD uses PCF8574 chip
//#define LCD_PCF8574T             // If LCD uses PCF8574T chip
// #define LCD_PCF8574AT            // If LCD uses PCF8574AT chip

// #define EthModule             // enable Ethernet module if installed
// #define __USE_DHCP__          // enable DHCP
//byte NET_ID = 0x00;         // NetID [hex] MUST BE UNIQUE IN NETWORK - replace by P6 board encoder
// #define BcdIpRelay               // control IP relay in BCD format

//=====[ Settings ]===========================================================================================

#define SERBAUD     38400     // [baud] Serial port in/out baudrate
#define WATCHDOG       20     // [sec] determines the time, after which the all relay OFF, if missed next input data - uncomment for the enabled
#define REQUEST        500    // [ms] use TXD output for sending frequency request
#define CIV_ADRESS    0x56    // CIV input HEX Icom adress (0x is prefix)
#define CIV_ADR_OUT   0x56    // CIV output HEX Icom adress (0x is prefix)
// #define DISABLE_DIVIDER    // for lowest voltage D-SUB pin 13 inputs up to 5V only - need open JP9
//#define DEBUG              // enable some debugging
//=====[ FREQUEN RULES ]===========================================================================================

const uint32_t Freq2Band[16][2] = {/*
Freq Hz from       to   Band number
*/   {1810000,   2000000},  // #1 [160m]
     {3500000,   3800000},  // #2  [80m]
     {5298000,   5403000},  // #3  [60m]
     {7000000,   7200000},  // #4  [40m]
    {10100000,  10150000},  // #5  [30m]
    {14000000,  14350000},  // #6  [20m]
    {18068000,  18168000},  // #7  [17m]
    {21000000,  21450000},  // #8  [15m]
    {24890000,  24990000},  // #9  [12m]
    {28000000,  29700000},  // #10  [10m]
    {50000000,  52000000},  // #11  [6m]
    {70000000,  72000000},  // #12  [4m]
   {144000000, 146000000},  // #13  [2m]
   {430000000, 440000000},  // #14  [70cm]
   {1240000000UL, 1300000000UL},  // #15  [23cm]
   {2300000000UL, 2450000000UL},  // #16  [13cm]
   // {3300000000, 3500000000},  // #17  [9cm]
   // {5650000000, 5850000000},  // #18  [6cm]
};
//=====[ Sets band -->  to output in MATRIX table ]===========================================================

        const byte matrix[17][40] = { /* band out

        If enable #define MULTI_OUTPUT_BY_BCD
        you can select outputs manually to ground one from BCD inputs
        to select between four output on same band
        represent by bit in this table
        0x01 = B00000001 = bit 1
        0x02 = B00000010 = bit 2
        0x04 = B00000100 = bit 3
        0x08 = B00001000 = bit 4
        For example record
        Band 1 -->  {      0x01,       0x02,       0x04,  0,       0x08,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 },
        is the same as record
        Band 1 -->  { B00000001,  B00000010,  B00000100,  0,  B00001000,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 },

        0x0F = this output enable for any BCD input (enable all bit)

        Band 0 --> */ { 0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /* first eight shift register board
\       Band 1 --> */ { 0x0F,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
 \      Band 2 --> */ { 0,  0x0F,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
  \     Band 3 --> */ { 0,  0,  0x0F,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
   \    Band 4 --> */ { 0,  0,  0,  0x0F,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
    \   Band 5 --> */ { 0,  0,  0,  0,  0x0F,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
     \  Band 6 --> */ { 0,  0,  0,  0,  0,  0x0F,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
IN    ) Band 7 --> */ { 0,  0,  0,  0,  0,  0,  0x0F,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
     /  Band 8 --> */ { 0,  0,  0,  0,  0,  0,  0,  0x0F,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*

    /   Band 9 --> */ { 0,  0,  0,  0,  0,  0,  0,  0,    0x0F,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /* second eight shift register board
   /    Band 10 -> */ { 0,  0,  0,  0,  0,  0,  0,  0,    0,  0x0F,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /* (optional)
  /     Band 11 -> */ { 0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0x0F,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
 /      Band 12 -> */ { 0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0x0F,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
/       Band 13 -> */ { 0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0x0F,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
        Band 14 -> */ { 0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0x0F,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
        Band 15 -> */ { 0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0x0F,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
        Band 16 -> */ { 0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0x0F,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0,    0,  0,  0,  0,  0,  0,  0,  0 }, /*
                        |   |   |   |   |   |   |   |     |   |   |   |   |   |   |   |
                        V   V   V   V   V   V   V   V     V   V   V   V   V   V   V   V
                     ----------------------------------  ---------------------------------
                     |  1   2   3   4   5   6   7   8     9  10  11  12  13  14  15  16  |
                     ----------------------------------  ---------------------------------
                                                   OUTPUTS
                                    (for second eight need aditional board)*/
        };
        const int NumberOfBoards = 1;    // number of eight byte shift register 0-x

//=====[ BCD OUT ]===========================================================================================

        const boolean BCDmatrixOUT[4][16] = { /*
        --------------------------------------------------------------------
        Band # to output relay   0   1   2   3   4   5   6   7   8   9  10
        (Yaesu BCD)                 160 80  40  30  20  17  15  12  10  6m
        --------------------------------------------------------------------
                                 |   |   |   |   |   |   |   |   |   |   |
                                 V   V   V   V   V   V   V   V   V   V   V
                            */ { 0,  1,  0,  1,  0,  1,  0,  1,  0,  1,  0, 1, 0, 1, 0, 1 }, /* --> DB25 Pin 11
                            */ { 0,  0,  1,  1,  0,  0,  1,  1,  0,  0,  1, 1, 0, 0, 1, 1 }, /* --> DB25 Pin 24
                            */ { 0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0, 0, 1, 1, 1, 1 }, /* --> DB25 Pin 12
                            */ { 0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1, 1, 1, 1, 1, 1 }, /* --> DB25 Pin 25
        */};
//=====[ PWM OUT ]===========================================================================================
#if defined(PWM_OUT)
  const uint32_t PwmByBand[17] = {/*
  PWM 0-255
  */ 0,    // #0  OUT of band
     11,   // #1  [160m] 0,23V
     24,   // #2  [80m] 0,46V
     36,   // #3  [60m] 0,69V
     49,   // #4  [40m] 0,92V
     62,   // #5  [30m] 1,15V
     74,   // #6  [20m] 1,38V
     87,   // #7  [17m] 1,61V
     99,  // #8  [15m] 1,84V
     111,  // #9  [12m] 2,07V
     124,  // #10 [10m] 2,3V
     137,  // #11 [6m]  2,53V
     180,  // #12 [4m]
     195,  // #13 [2m]
     210,  // #14 [70cm]
     225,  // #15 [23cm]
     240,  // #16 [13cm]
  };
#endif
//============================================================================================================

// #define SERIAL_debug
// #define UdpBroadcastDebug_debug

#if defined(LCD)
  #include <Wire.h>

  #if defined(LCD_PCF8574T) || defined(LCD_PCF8574)
    #include <LiquidCrystal_PCF8574.h>
    LiquidCrystal_PCF8574 lcd(LcdI2Caddress);
  #endif
  #if defined(LCD_PCF8574AT)
    #include <LiquidCrystal_I2C.h>
    LiquidCrystal_I2C lcd(LcdI2Caddress,16,2);
  #endif


  uint32_t LcdRefresh[2]{0,500};
  const char* ANTname[17][4] = {

/*
    - If enable #define PTT_BY_BAND output name represent diffrent devices (TRX)

    - If enable #define MULTI_OUTPUT_BY_BCD
      you can fill name for another antennas on the same band
      dependency to select BCD input

Default or BCD-1   BCD-2   BCD-3    BCD-4
             |       |       |        |
*/    {"Out of band", "Out of band", "Out of band", "Out of band"},  // Band 0 (no data)
      {"Dipole", "BCD-2", "BCD-3", "BCD-4"},       // Band 1
      {"Vertical", "BCD-2", "BCD-3", "BCD-4"},     // Band 2
      {"3el Yagi", "BCD-2", "BCD-3", "BCD-4"},     // Band 3
      {"Windom", "BCD-2", "BCD-3", "BCD-4"},       // Band 4
      {"DeltaLoop", "BCD-2", "BCD-3", "BCD-4"},    // Band 5
      {"20m Stack", "BCD-2", "BCD-3", "BCD-4"},    // Band 6
      {"DeltaLoop", "BCD-2", "BCD-3", "BCD-4"},    // Band 7
      {"HB9", "BCD-2", "BCD-3", "BCD-4"},          // Band 8
      {"Dipole", "BCD-2", "BCD-3", "BCD-4"},       // Band 9
      {"5el Yagi", "BCD-2", "BCD-3", "BCD-4"},     // Band 10
      {"7el Yagi", "BCD-2", "BCD-3", "BCD-4"},     // Band 11
      {"24el", "BCD-2", "BCD-3", "BCD-4"},         // Band 12
      {"20el quad", "BCD-2", "BCD-3", "BCD-4"},    // Band 13
      {"Dish 1.2m", "BCD-2", "BCD-3", "BCD-4"},    // Band 14
      {"Dish 1.2m", "BCD-2", "BCD-3", "BCD-4"},    // Band 15
      {"Dish 1m", "BCD-2", "BCD-3", "BCD-4"},      // Band 16
  };
  // byte LockChar[8] = {0b00100, 0b01010, 0b01010, 0b11111, 0b11011, 0b11011, 0b11111, 0b00000};
  uint8_t LockChar[8] = {0x4,0xa,0xa,0x1f,0x1b,0x1b,0x1f,0x0};
  bool LcdNeedRefresh = false;
#endif

#if defined(EthModule)
  const byte RemoteDevice = 's';
  const byte ThisDevice = 'r';
  bool EnableEthernet = 1;
  bool EnableDHCP     = 1;
  uint32_t GetNetIdTimer[2]{0,2000};
  //  #include <util.h>
  #include <Ethernet.h>
  #include <EthernetUdp.h>
  // #include <Dhcp.h>
  // #include <EthernetServer.h>
  #include <SPI.h>
  byte LastMac = 0x00 + NET_ID;

  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, LastMac};
  IPAddress ip(192, 168, 1, 222);         // IP
  IPAddress gateway(192, 168, 1, 200);    // GATE
  IPAddress subnet(255, 255, 255, 0);     // MASK
  IPAddress myDns(8, 8, 8, 8);            // DNS (google pub)
  EthernetServer server(80);              // Web server PORT
  String HTTP_req;

  unsigned int UdpCommandPort = 88;       // local UDP port listen to command
  // #define UDP_TX_PACKET_MAX_SIZE 30       // MIN 30
  // unsigned char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
  unsigned char packetBuffer[30]; //buffer to hold incoming packet,
  int UDPpacketSize;
  EthernetUDP UdpCommand; // An EthernetUDP instance to let us send and receive packets over UDP
  IPAddress BroadcastIP(0, 0, 0, 0);        // Broadcast IP address
  int BroadcastPort       = 88;             // destination broadcast packet port
  IPAddress RemoteSwIP(0, 0, 0, 0);         // remote UDP IP switch - set from UDP DetectRemote array
  int RemoteSwPort         = 0;             // remote UDP IP switch port
  byte DetectedRemoteSw[16][5];             // detect by RX broadcast packet - storage by ID (ID=rows)
  int BandDecoderChange    = 0;             // If band change, send query packet to Remote IP switch
  uint32_t RemoteSwLatency[2];                  // start, stop mark
  byte RemoteSwLatencyAnsw = 0;             // answer (offline) detect
  byte TxUdpBuffer[10];
  uint32_t IpTimeout[1][2] = {0, 60000};          // UDP Broadcast packet [8][0-timer/1-timeout]

  bool EthLinkStatus = 0;
  uint32_t EthLinkStatusTimer[2]{1500,1000};
  uint8_t EthChar[8] = {0x0,0x0,0x1f,0x11,0x11,0x1b,0x1f,0x0};
#endif

// PINOUTS
//const int VoltagePin = A0;        // measure input voltage
  //const int EncBPin = A0;         // Encoder B
//const int Id1Pin = A1;            // ID switch
//const int BcdIn1Pin = A2;         // BCD-1 in/out
  //const int ShiftInClockPin = A2; // [ShiftIn]
//const int ADPin = A3;             // [BD/ROT]
  //const int SequencerPin = A3;    // [IPsw/BD]
// const int SdaPin = A4;         // [LCD]
  //const int Id4Pin = A4;          //
// const int SclPin = A5;         // [LCD]
//const int Id2Pin = A6;            //
//const int Id3Pin = A7;            //
//const int PttDetectorPin = 2;     // PTT in - [interrupt]
//const int BcdIn4Pin = 3;          // BCD-4 in/out
//  const int EncAShiftInPin = 3;   // Encoder+Keyboard - [interrupt]
//const int BcdIn3Pin = 4;          // BCD-3 in/out
//  const int ShiftInDataPin = 4;   // [ShiftIn]
//const int BcdIn2Pin = 5;          // BCD-2 in/out
//  const int ShiftInLatchPin = 5;  // [ShiftIn]
//  const int PwmOutPin = 5;        // [PWM]
//const int PttOffPin = 6;          // PTT out OFF switch
//const int ShiftOutDataPin = 7;    // DATA
//const int ShiftOutLatchPin = 8;   // LATCH
//const int ShiftOutClockPin = 9;   // CLOCK

int BAND = 0;
int previousBAND = -1;
uint32_t freq = 0;
bool PTT = false;
uint32_t PttTiming[2]={0, 10};            // debouncing time and also minimal PTT on time in ms
float DCinVoltage;
#if defined(DISABLE_DIVIDER)
  float ResistorCoeficient = 1.0;
#else
  float ResistorCoeficient = 6.0;
#endif

uint32_t VoltageRefresh[2] = {0, 3000};   // refresh in ms
float ArefVoltage = 4.303;            // Measure on Aref pin 20 for calibrate
float Divider = 1;

byte ShiftByte[NumberOfBoards];

// int SelectOut = 0;
// int x;
  uint32_t RequestTimeout[2]={0,
    #if defined(REQUEST)
      REQUEST
    #else
      0
    #endif
  };

int watchdog2 = 1000;     // REQUEST refresh time [ms]
int previous2;
int timeout2;

#if defined(WATCHDOG)
    int previous;
    int timeout;
    uint32_t WatchdogTimeout[2] = {0, WATCHDOG*1000};  // {-WATCHDOG*1000, WATCHDOG*1000};
#endif
#if defined(ICOM_ACC)
    float AccVoltage = 0;
    float prevAccVoltage=0;
    int band = 0;
    int counter = 0;
#endif
#if defined(INPUT_BCD)
    uint32_t BcdInRefresh[2] = {0, 1000};   // refresh in ms
#endif
#if defined(KENWOOD_PC) || defined(YAESU_CAT) || defined(FLEX_6000)
    int lf = 59;  // 59 = ;
#endif
#if defined(KENWOOD_PC) || defined(FLEX_6000)
    char rdK[37];   //read data kenwood
    String rdKS;    //read data kenwood string
#endif
#if defined(YAESU_CAT)
    char rdY[37];   //read data yaesu
    String rdYS;    //read data yaesu string
#endif
#if defined(YAESU_CAT_OLD) || defined(YAESU_CAT_FT100)
    byte rdYO[37];   //read data yaesu
    String rdYOS;    //read data yaesu string
#endif
#if defined(ICOM_CIV) || defined(ICOM_CIV_OUT)
    int fromAdress = 0xE0;              // 0E
    byte rdI[10];   //read data icom
    String rdIS;    //read data icom string
    uint32_t freqPrev1;
    byte incomingByte = 0;
    int state = 1;  // state machine
    bool StateMachineEnd = false;
#endif
#if defined(KENWOOD_PC_OUT) || defined(YAESU_CAT_OUT)
    uint32_t freqPrev2;
#endif
#if defined(BCD_OUT)
    char BCDout;
#endif
#if defined(MULTI_OUTPUT_BY_BCD)
  byte SelectBank;
  byte SelectBankPrev;
#endif
//---------------------------------------------------------------------------------------------------------

void CAT_setup() {
  	#if defined(YAESU_CAT_OLD) || defined(YAESU_CAT_FT100)
		CAT_Serial.begin(SERBAUD, SERIAL_8N2);
		CAT_Serial.setTimeout(10);
  	#else
		CAT_Serial.begin(SERBAUD);
		//CAT_Serial.setTimeout(10);
  	#endif

  	#if defined(KENWOOD_PC) || defined(YAESU_CAT)
		//CAT_Serial.reserve(200);          // reserve bytes for the CATdata
		CAT_Serial.begin(38400);
		//CAT_Serial.setTimeout(1);
		
		// use a larger RX buffer, needed to prevent buffer overwrite during startup when many messages are arriving
		CAT_Serial.addMemoryForRead(Ser_Buff, S_BUFF);   
		//CAT_Serial.addMemoryForWrite(buffer, size);

		CAT_Serial.flush();
		CAT_Serial.clear();

		CAT_Serial.print(F("K31;"));		// extended K3 mode
		CAT_Serial.print(F("AI2;"));		// Radio will send out events when they happen

		VFOA_Request();			// init our main indicators
		VFOB_Request();			// init our main indicators
		Filter_Request();
		AGC_Request();
		IF_Center_Request();
		ANT_Request();
		FrequencyRequest();		// get VFO freq, mode, RIT/XIT and TX/Rx status
		CAT_msgs();

  	#endif // KENWOOD_PC

  //pinMode(VoltagePin, INPUT);

  #if defined(ICOM_ACC)
    pinMode(ADPin, INPUT);
  #else
    //pinMode(SequencerPin, OUTPUT);
  #endif

  //pinMode(PttDetectorPin, INPUT);
    //digitalWrite(PttDetectorPin, HIGH);

  //pinMode(PttOffPin, OUTPUT);
  #if ( defined(INPUT_BCD) || defined(MULTI_OUTPUT_BY_BCD) ) && !defined(PWM_OUT)
    pinMode(BcdIn1Pin, INPUT);
      // digitalWrite(Id3Pin, INPUT_PULLUP);
      digitalWrite(BcdIn1Pin, HIGH);
    pinMode(BcdIn2Pin, INPUT);
      digitalWrite(BcdIn2Pin, HIGH);
    pinMode(BcdIn3Pin, INPUT);
      digitalWrite(BcdIn3Pin, HIGH);
    pinMode(BcdIn4Pin, INPUT);
      digitalWrite(BcdIn4Pin, HIGH);
  #else
    //pinMode(BcdIn1Pin, OUTPUT);
    //pinMode(BcdIn2Pin, OUTPUT);
    //pinMode(PwmOutPin, OUTPUT);
    //pinMode(BcdIn3Pin, OUTPUT);
    //pinMode(BcdIn4Pin, OUTPUT);
  #endif
  //pinMode(ShiftOutDataPin, OUTPUT);
  // pinMode(ShiftOutLatchPin, OUTPUT);
  //pinMode(ShiftOutClockPin, OUTPUT);

  //analogReference(EXTERNAL);

  #if defined(LCD)

    #if defined(LCD_PCF8574T) || defined(LCD_PCF8574)
      lcd.begin(16, 2); // initialize the lcd PFC8574
      lcd.setBacklight(1);
    #else
      //------------------------------------------------------
      // Enable begin or init in dependence on the GUI version
      // lcd.begin();
      lcd.init();
      lcd.backlight();
      //------------------------------------------------------
    #endif

//    lcd.createChar(0, LockChar);
    #if defined(EthModule)
      // lcd.createChar(1, EthChar);
    #endif
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("Version:"));
    lcd.setCursor(7,1);
    lcd.print(REV);
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);
      #if defined(INPUT_SERIAL)
        lcd.print(F("SERIAL      "));
      #endif
      #if defined(ICOM_CIV)
        lcd.print(F("ICOM        "));
        lcd.print(CIV_ADRESS, HEX);
        lcd.print(F("h"));
      #endif
      #if defined(ICOM_ACC)
      lcd.print(F("ICOM ACC    "));
      #endif
      #if defined(KENWOOD_PC)
        lcd.print(F("KENWOOD      "));
      #endif
      #if defined(FLEX_6000)
        lcd.print(F("FLEX-6000    "));
      #endif
      #if defined(YAESU_CAT) || defined(YAESU_CAT_OLD) || defined(YAESU_CAT_FT100)
        lcd.print(F("YAESU       "));
      #endif
      #if defined(INPUT_BCD)
        lcd.print(F("INPUT BCD   "));
      #endif
      #if !defined(INPUT_BCD) && !defined(ICOM_ACC)
        lcd.setCursor(10,0);
        lcd.print(SERBAUD);
      #endif
    lcd.setCursor(0,1);
    lcd.print(F("DC input "));
    lcd.print(volt(analogRead(VoltagePin), ResistorCoeficient));
    lcd.print(F("V"));
    delay(3000);
    lcd.clear();
  #endif

  #if defined(EthModule)
    pinMode(Id1Pin, INPUT_PULLUP);
    pinMode(Id2Pin, INPUT_PULLUP);
    pinMode(Id3Pin, INPUT_PULLUP);
    NET_ID=GetBoardId();
    LastMac = 0x00 + NET_ID;
    mac[5] = LastMac;
    EthernetCheck();
  #endif
  //InterruptON(1,1); // ptt, enc
}
//---------------------------------------------------------------------------------------------------------

void CAT_handler() 
{
  	//BandDecoderInput();
  	//BandDecoderOutput();
	//LcdDisplay();
	//watchDog();
	
	// We have AI2 turned on so the radio will send IF; and individual events when they happen, no need to poll except to initialize.
	//VFOA_Request();			// Get our VFO A
	//VFOB_Request();			// Get our VFO B=
	//Filter_Request();			// Get the filter width
	//FrequencyRequest();		// get VFO freq, mode, RIT/XIT and TX/Rx status
	
	CAT_msgs();   // This scans the message received and calls the matching function

	if (meter_update.check() == 1)
	{	
		BarGraph_Request();		//get our bar graph update
		CAT_msgs();   // This scans the message received and calls the matching function
	}

  //PttOff();
  #if defined(EthModule)
    // RX_UDP(RemoteDevice, ThisDevice);
    EthernetCheck();
    // WebServer();
  #endif
}

// SUBROUTINES ---------------------------------------------------------------------------------------------------------

/*
ID FROM TO : BRO ;
ID FROM TO : A B C ;

TX  0rs:b;
TX  0rs:123;
*/

void TxUDP(byte FROM, byte TO, byte A, byte B, byte C){
  #if defined(EthModule)
    InterruptON(0,0); // ptt, enc
    RemoteSwLatencyAnsw = 0;   // send command, wait to answer

    TxUdpBuffer[0] = NET_ID;
    TxUdpBuffer[1] = FROM;
    TxUdpBuffer[2] = TO;
    TxUdpBuffer[3] = B00111010;           // :
    TxUdpBuffer[4] = A;
    TxUdpBuffer[5] = B;
    TxUdpBuffer[6] = C;
    TxUdpBuffer[7] = B00111011;           // ;

    // BROADCAST
    if(A=='b' && B=='r' && C=='o'){  // b r o
    // if(A==B01100010 && B==B01110010 && C==B01101111){  // b r o
      // direct
      for (int i=0; i<15; i++){
        if(DetectedRemoteSw[i][4]!=0){
          RemoteSwIP = DetectedRemoteSw[i];
          RemoteSwPort = DetectedRemoteSw[i][4];
          UdpCommand.beginPacket(RemoteSwIP, RemoteSwPort);
            UdpCommand.write(TxUdpBuffer, sizeof(TxUdpBuffer));   // send buffer
          UdpCommand.endPacket();
          // RemoteSwLatency[0] = millis(); // set START time mark UDP command latency

          #if defined(SERIAL_debug)
            CAT_Serial.print(F("TX direct ID-"));
            CAT_Serial.print(i);
            CAT_Serial.print(F(" "));
            CAT_Serial.print(RemoteSwIP);
            CAT_Serial.print(F(":"));
            CAT_Serial.print(RemoteSwPort);
            CAT_Serial.print(F(" ["));
            CAT_Serial.print(TxUdpBuffer[0], HEX);
            for (int i=1; i<8; i++){
              CAT_Serial.print(char(TxUdpBuffer[i]));
              // CAT_Serial.print(F(" "));
            }
            CAT_Serial.println(F("]"));
          #endif
        }
      }

      // broadcast
      BroadcastIP = ~Ethernet.subnetMask() | Ethernet.gatewayIP();
      UdpCommand.beginPacket(BroadcastIP, BroadcastPort);   // Send to IP and port from recived UDP command
        UdpCommand.write(TxUdpBuffer, sizeof(TxUdpBuffer));   // send buffer
      UdpCommand.endPacket();
      IpTimeout[0][0] = millis();                      // set time mark

        #if defined(SERIAL_debug)
          CAT_Serial.print(F("TX broadcast "));
          CAT_Serial.print(BroadcastIP);
          CAT_Serial.print(F(":"));
          CAT_Serial.print(BroadcastPort);
          CAT_Serial.print(F(" ["));
          CAT_Serial.print(TxUdpBuffer[0], HEX);
          for (int i=1; i<8; i++){
            CAT_Serial.print(char(TxUdpBuffer[i]));
            // CAT_Serial.print(F(" "));
          }
          CAT_Serial.println(F("]"));
        #endif

    // DATA
    }else{
      if(DetectedRemoteSw[NET_ID][4]!=0 && PTT==false){
        RemoteSwIP = DetectedRemoteSw[NET_ID];
        RemoteSwPort = DetectedRemoteSw[NET_ID][4];
        UdpCommand.beginPacket(RemoteSwIP, RemoteSwPort);
          UdpCommand.write(TxUdpBuffer, sizeof(TxUdpBuffer));   // send buffer
        UdpCommand.endPacket();
        RemoteSwLatency[0] = millis(); // set START time mark UDP command latency

        #if defined(SERIAL_debug)
          CAT_Serial.println();
          CAT_Serial.print(F("TX ["));
          CAT_Serial.print(TxUdpBuffer[0], HEX);
          for (int i=1; i<4; i++){
            CAT_Serial.print(char(TxUdpBuffer[i]));
          }
          CAT_Serial.print(TxUdpBuffer[4], BIN);
          CAT_Serial.print(F("|"));
          CAT_Serial.print(TxUdpBuffer[5], BIN);
          CAT_Serial.print(F("|"));
          CAT_Serial.print(TxUdpBuffer[6], BIN);
          CAT_Serial.print(char(TxUdpBuffer[7]));
          CAT_Serial.print(F("] "));
          CAT_Serial.print(RemoteSwIP);
          CAT_Serial.print(F(":"));
          CAT_Serial.println(RemoteSwPort);
        #endif
      }
    }
  InterruptON(1,1); // ptt, enc
  #endif
}
//-------------------------------------------------------------------------------------------------------
/*
ID FROM TO : CFM ;
ID FROM TO : A B C ;

RX  0sr:c;
RX  0sr:123;
*/

void RX_UDP(char FROM, char TO){
  #if defined(EthModule)
    InterruptON(0,0); // ptt, enc
    UDPpacketSize = UdpCommand.parsePacket();    // if there's data available, read a packet
    if (UDPpacketSize){
      // UdpCommand.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);      // read the packet into packetBufffer
      UdpCommand.read(packetBuffer, 30);      // read the packet into packetBufffer
      // Print RAW
      // #if defined(SERIAL_debug)
      //     CAT_Serial.print(F("RXraw "));
      //     for (int i = 0; i < 8; i++) {
      //       CAT_Serial.print(packetBuffer[i]);
      //     }
      //     CAT_Serial.print(F(" "));
      //     CAT_Serial.print(UdpCommand.remoteIP());
      //     CAT_Serial.print(":");
      //     CAT_Serial.print(UdpCommand.remotePort());
      //     CAT_Serial.println();
      // #endif
      // ID-FROM-TO filter
      if(String(packetBuffer[0], DEC).toInt()==NET_ID
        && packetBuffer[1]== FROM
        && packetBuffer[2]== TO
        && packetBuffer[3]== ':'
        && packetBuffer[7]== ';'
      ){
        RemoteSwLatency[1] = (millis()-RemoteSwLatency[0])/2; // set latency (half path in ms us/2/1000)
        RemoteSwLatencyAnsw = 1;           // answer packet received

        // RX Broadcast / CFM
        if((packetBuffer[4]== 'b' && packetBuffer[5]== 'r' && packetBuffer[6]== 'o')
          || (packetBuffer[4]== 'c' && packetBuffer[5]== 'f' && packetBuffer[6]== 'm')
          ){
          IPAddress TmpAddr = UdpCommand.remoteIP();
          DetectedRemoteSw [hexToDecBy4bit(packetBuffer[0])] [0]=TmpAddr[0];     // Switch IP addres storage to array
          DetectedRemoteSw [hexToDecBy4bit(packetBuffer[0])] [1]=TmpAddr[1];
          DetectedRemoteSw [hexToDecBy4bit(packetBuffer[0])] [2]=TmpAddr[2];
          DetectedRemoteSw [hexToDecBy4bit(packetBuffer[0])] [3]=TmpAddr[3];
          DetectedRemoteSw [hexToDecBy4bit(packetBuffer[0])] [4]=UdpCommand.remotePort();

          #if defined(LCD)
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Detect SW #"));
            lcd.print(packetBuffer[0], HEX);
            lcd.setCursor(0, 1);
            lcd.print(DetectedRemoteSw [hexToDecBy4bit(packetBuffer[0])] [0]);
            lcd.print(F("."));
            lcd.print(DetectedRemoteSw [hexToDecBy4bit(packetBuffer[0])] [1]);
            lcd.print(F("."));
            lcd.print(DetectedRemoteSw [hexToDecBy4bit(packetBuffer[0])] [2]);
            lcd.print(F("."));
            lcd.print(DetectedRemoteSw [hexToDecBy4bit(packetBuffer[0])] [3]);
            lcd.print(F(":"));
            lcd.print(DetectedRemoteSw [hexToDecBy4bit(packetBuffer[0])] [4]);
            delay(4000);
            LcdNeedRefresh = true;
            lcd.clear();
          #endif

          #if defined(SERIAL_debug)
            CAT_Serial.print(F("RX ["));
            CAT_Serial.print(packetBuffer[0], HEX);
            for(int i=1; i<8; i++){
              CAT_Serial.print(char(packetBuffer[i]));
            }
            CAT_Serial.print(F("] "));
            CAT_Serial.print(UdpCommand.remoteIP());
            CAT_Serial.print(F(":"));
            CAT_Serial.println(UdpCommand.remotePort());
            for (int i = 0; i < 16; i++) {
              CAT_Serial.print(i);
              CAT_Serial.print(F("  "));
              CAT_Serial.print(DetectedRemoteSw [i] [0]);
              CAT_Serial.print(F("."));
              CAT_Serial.print(DetectedRemoteSw [i] [1]);
              CAT_Serial.print(F("."));
              CAT_Serial.print(DetectedRemoteSw [i] [2]);
              CAT_Serial.print(F("."));
              CAT_Serial.print(DetectedRemoteSw [i] [3]);
              CAT_Serial.print(F(":"));
              CAT_Serial.println(DetectedRemoteSw [i] [4]);
            }
          #endif

        // RX DATA
        }else{
          byte ButtonSequence = 0;
          // 4bit shift left OR 4bit shift right = 4bit shift rotate
          ButtonSequence = (byte)packetBuffer[4] >> 4 | (byte)packetBuffer[4] << 4;
          digitalWrite(ShiftOutLatchPin, LOW);  // ready for receive data
          shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ButtonSequence);    // buttons
          shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ButtonSequence);    // buttons
          digitalWrite(ShiftOutLatchPin, HIGH);    // switch to output pin

          #if defined(SERIAL_debug)
            CAT_Serial.print(F("RX ["));
            CAT_Serial.print(packetBuffer[0], HEX);
            for(int i=1; i<4; i++){
              CAT_Serial.print(char(packetBuffer[i]));
            }
            CAT_Serial.print((byte)packetBuffer[4], BIN);
            CAT_Serial.print(F("|"));
            CAT_Serial.print((byte)packetBuffer[5], BIN);
            CAT_Serial.print(F("|"));
            CAT_Serial.print((byte)packetBuffer[6], BIN);
            CAT_Serial.print(F(";] "));
            CAT_Serial.print(UdpCommand.remoteIP());
            CAT_Serial.print(F(":"));
            CAT_Serial.print(UdpCommand.remotePort());
            CAT_Serial.print(F(" Latency: "));
            CAT_Serial.println(RemoteSwLatency[1]);
          #endif
          #if defined(LCD)
            LcdNeedRefresh = true;
          #endif
        }
      } // filtered end
      else{
        #if defined(SERIAL_debug)
          CAT_Serial.println(F("   Different NET-ID, or bad packet format"));
        #endif
      }
      memset(packetBuffer, 0, sizeof(packetBuffer));   // Clear contents of Buffer
    } //end IfUdpPacketSice
    InterruptON(1,1); // ptt, enc
  #endif
}
//-------------------------------------------------------------------------------------------------------

void EthernetCheck(){
  #if defined(EthModule)
  if(millis()-EthLinkStatusTimer[0]>EthLinkStatusTimer[1] && EnableEthernet==1){
    if ((Ethernet.linkStatus() == Unknown || Ethernet.linkStatus() == LinkOFF) && EthLinkStatus==1) {
      EthLinkStatus=0;
      #if defined(SERIAL_debug)
        CAT_Serial.println(F("Ethernet DISCONNECTED"));
      #endif
    }else if (Ethernet.linkStatus() == LinkON && EthLinkStatus==0) {
      EthLinkStatus=1;
      #if defined(SERIAL_debug)
        CAT_Serial.println(F("Ethernet CONNECTED"));
      #endif

      #if defined(LCD)
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print(F("Net-ID: "));
        lcd.print(NET_ID);
        lcd.setCursor(1, 1);
        lcd.print(F("[DHCP-"));
      #endif
      if(EnableDHCP==1){
        #if defined(LCD)
          lcd.print(F("ON]..."));
        #endif
          Ethernet.begin(mac);
          IPAddress CheckIP = Ethernet.localIP();
          if( CheckIP[0]==0 && CheckIP[1]==0 && CheckIP[2]==0 && CheckIP[3]==0 ){
            #if defined(LCD)
              lcd.clear();
              lcd.setCursor(1, 0);
              lcd.print(F("DHCP FAIL"));
              lcd.setCursor(1, 1);
              lcd.print(F("please restart"));
            #endif
            while(1) {
              // infinite loop
            }
          }
      }else{
        #if defined(LCD)
          lcd.print(F("OFF]"));
        #endif
        Ethernet.begin(mac, ip, myDns, gateway, subnet);
      }
        delay(2000);
        #if defined(LCD)
          lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print(F("IP address:"));
          lcd.setCursor(1, 1);
          lcd.print(Ethernet.localIP());
          delay(2500);
          lcd.clear();
        #endif

      server.begin();                     // Web
      UdpCommand.begin(UdpCommandPort);   // UDP
      TxUDP(ThisDevice, RemoteDevice, 'b', 'r', 'o');

    }
    EthLinkStatusTimer[0]=millis();
  }
  #endif
}

//---------------------------------------------------------------------------------------------------------

COLD void TxBroadcastUdp(String MSG){
  #if defined(EthModule)
    InterruptON(0,0); // ptt, enc
    BroadcastIP = ~Ethernet.subnetMask() | Ethernet.gatewayIP();

    UdpCommand.beginPacket(BroadcastIP, BroadcastPort);   // Send to IP and port from recived UDP command
      UdpCommand.print(MSG);
    UdpCommand.endPacket();

    InterruptON(1,1); // ptt, enc
  #endif
}

//-------------------------------------------------------------------------------------------------------

COLD unsigned char hexToDecBy4bit(unsigned char hex)
// convert a character representation of a hexidecimal digit into the actual hexidecimal value
{
  if(hex > 0x39) hex -= 7; // adjust for hex letters upper or lower case
  return(hex & 0xf);
}
//-------------------------------------------------------------------------------------------------------

void InterruptON(int ptt, int enc){
  if(ptt==0){
    //detachInterrupt(digitalPinToInterrupt(PttDetectorPin));
  }else if(ptt==1){
    //attachInterrupt(digitalPinToInterrupt(PttDetectorPin), PttDetector, FALLING);  // need detachInterrupt in IncomingUDP() subroutine
  }
}
//---------------------------------------------------------------------------------------------------------

COLD void PttDetector(){   // call from interupt
  // digitalWrite(PttOffPin, HIGH);
  PTT = true;
  #if defined(PTT_BY_BAND)
    FreqToBandRules();
    bandSET();
  #endif
  #if defined(LCD)
    LcdNeedRefresh = true;
  #endif
  PttTiming[0]=millis();
}
//---------------------------------------------------------------------------------------------------------

COLD void PttOff(){
  //if(PTT==true && millis()-PttTiming[0] > PttTiming[1] && digitalRead(PttDetectorPin)==HIGH && BAND!=0){

  #if defined(EthModule)
    if(DetectedRemoteSw[NET_ID][4]!=0 && RemoteSwLatencyAnsw==1){
  #endif

    // digitalWrite(PttOffPin, LOW);
    #if defined(EthModule) && defined(UdpBroadcastDebug_debug)
      TxBroadcastUdp("PttOff-" + String(DetectedRemoteSw[NET_ID][4]) + "-" + String(RemoteSwLatencyAnsw) );
    #endif
    #if defined(PTT_BY_BAND)
      BAND=0;
      bandSET();
    #endif
    PTT = false;
    #if defined(LCD)
      LcdNeedRefresh = true;
    #endif

  #if defined(EthModule)
    }
  #endif

  //}
}
//---------------------------------------------------------------------------------------------------------

COLD void FrequencyRequest(){
  #if defined(REQUEST)
  if(REQUEST > 0 && (millis() - RequestTimeout[0] > RequestTimeout[1])){

    #if defined(ICOM_CIV)
      txCIV(3, 0, CIV_ADRESS);  // ([command], [freq]) 3=read
    #endif

    #if defined(KENWOOD_PC) || defined(YAESU_CAT)
          CAT_Serial.print("IF;");
          CAT_Serial.flush();       // Waits for the transmission of outgoing serial data to complete
    #endif

    #if defined(FLEX_6000)
          CAT_Serial.print(F("FA;"));
          CAT_Serial.flush();       // Waits for the transmission of outgoing serial data to complete
    #endif

    #if defined(YAESU_CAT_OLD)
        CAT_Serial.write(0);                                    // byte 1
        CAT_Serial.write(0);                                    // byte 2
        CAT_Serial.write(0);                                    // byte 3
        CAT_Serial.write(0);                                    // byte 4
        CAT_Serial.write(3);                                    // read freq
        CAT_Serial.flush();
    #endif

    #if defined(YAESU_CAT_FT100)
        byte readStatusCMD[] = {0x00,0x00,0x00,0x00,0x10};
        CAT_Serial.write(readStatusCMD,5);
        CAT_Serial.flush();
    #endif

    RequestTimeout[0]=millis();
  }
  #endif
}
//---------------------------------------------------------------------------------------------------------

#if defined(LCD)
  void Space(int MAX, int LENGHT, char CHARACTER){
    int NumberOfSpace = MAX-LENGHT;
    if(NumberOfSpace>0){
      for (int i=0; i<NumberOfSpace; i++){
        lcd.print(CHARACTER);
      }
    }
  }
#endif
//---------------------------------------------------------------------------------------------------------

COLD void LcdDisplay(){
  #if defined(LCD)
    if(millis()-LcdRefresh[0]>LcdRefresh[1] || LcdNeedRefresh == true){

      #if defined(EthModule)
        if(EthLinkStatus==0){
          lcd.setCursor(0, 0);
          lcd.print((char)1);   // EthChar
          lcd.print(F(" Please connect"));
          lcd.setCursor(0, 1);
          lcd.print(F("  ethernet      "));
        }else{
      #endif

      lcd.setCursor(0,0);
      int NameByBcd=0;
      #if defined(MULTI_OUTPUT_BY_BCD)
        if(SelectBank==2){
          NameByBcd=1;
        }else if(SelectBank==4){
          NameByBcd=2;
        }else if(SelectBank==8){
          NameByBcd=3;
        }
      #endif
      #if defined(WATCHDOG)
        if((millis() - WatchdogTimeout[0]) > WatchdogTimeout[1]) {
          lcd.print("CAT timeout");
        }else{
      #endif
          lcd.print(String(ANTname[BAND][NameByBcd]).substring(0, 11));   // crop up to 7 char
          Space(11, String(ANTname[BAND][NameByBcd]).length(), ' ');
      #if defined(WATCHDOG)
        }
      #endif

      #if defined(EthModule)
        lcd.setCursor(0,0);
        // if(RemoteSwLatencyAnsw==1 || (RemoteSwLatencyAnsw==0 && millis() < RemoteSwLatency[0]+RemoteSwLatency[1]*5)){ // if answer ok, or latency measure nod end
        if(RemoteSwLatencyAnsw==1 || (RemoteSwLatencyAnsw==0 && millis() < RemoteSwLatency[0]+500) ){ // if answer ok, or latency measure nod end
          lcd.print(String(ANTname[BAND][NameByBcd]).substring(0, 11));   // crop up to 7 char
          Space(11, String(ANTname[BAND][NameByBcd]).length(), ' ');
        }else{
          lcd.print(F("NetID-"));
          lcd.print(NET_ID, HEX);
          lcd.print(F(" n/a"));
        }
      #endif

      lcd.setCursor(11,0);
      lcd.print(F(" "));
      #if defined(MULTI_OUTPUT_BY_BCD)
        lcd.print(F("Ant"));
        if(SelectBank==0){
          lcd.print(F("-"));
        }else if(SelectBank==1){
          lcd.print(F("1"));
        }else if(SelectBank==2){
          lcd.print(F("2"));
        }else if(SelectBank==4){
          lcd.print(F("3"));
        }else if(SelectBank==8){
          lcd.print(F("4"));
        }
      #endif

      #if defined(PWM_OUT)
        lcd.print(PwmByBand[BAND]);
        lcd.print(F("  "));
      #endif
      #if !defined(PWM_OUT)
        lcd.print(BCDmatrixOUT[3][BAND]);
        lcd.print(BCDmatrixOUT[2][BAND]);
        lcd.print(BCDmatrixOUT[1][BAND]);
        lcd.print(BCDmatrixOUT[0][BAND]);
      #endif

      lcd.setCursor(0,1);
      #if defined(EthModule)
      if(RemoteSwLatencyAnsw==1 || (RemoteSwLatencyAnsw==0 && millis() < RemoteSwLatency[0]+RemoteSwLatency[1]*5)){ // if answer ok, or latency measure nod end
      // if(DetectedRemoteSw[NET_ID][4]!=0 && (RemoteSwLatencyAnsw==1 && millis() > RemoteSwLatency[0]+RemoteSwLatency[1]*5)){
        lcd.print(F("N"));
      }else{
        lcd.print(F("!"));
      }
      #else
      lcd.print(F("B"));
      #endif
      Space(2, String(BAND).length(), '-');
      lcd.print(BAND);
      lcd.print(F(" "));

      #if !defined(INPUT_BCD) && !defined(ICOM_ACC)
        Space(7, String(freq/1000).length(), ' ');
        PrintFreq();
      #endif
        if(freq<100000000){
          lcd.setCursor(4, 1);
        }else{
          lcd.setCursor(3, 1);
        }
        if(PTT==true){
          // lcd.write(byte(0));        // Lock icon
          lcd.print((char)0);
        }else{
          lcd.print(F(" "));        // Lock icon
        }
        #if !defined(INPUT_BCD) && !defined(ICOM_ACC)
          lcd.setCursor(13,1);
          lcd.print(F("kHz"));
        #endif
        #if defined(ICOM_ACC)
          lcd.setCursor(10,1);
          lcd.print(AccVoltage);
          lcd.print(F(" V"));
        #endif

      LcdRefresh[0]=millis();
      LcdNeedRefresh = false;

      #if defined(EthModule)
      }
      #endif
    }
  #endif
}
//---------------------------------------------------------------------------------------------------------

#if defined(LCD)
  void PrintFreq(){
    int longer=String(freq/1000).length();
    if(longer<4){
      lcd.print(F(" "));
      lcd.print(freq);
    }else{
      lcd.print(String(freq/1000).substring(0, longer-3));
      lcd.print(F("."));
      lcd.print(String(freq/1000).substring(longer-3, longer));
    }
  }
#endif
//---------------------------------------------------------------------------------------------------------

HOT void WebServer(){
  #if defined(EthModuleXXX)
    EthernetClient client = server.available();
    if (client) {
      boolean currentLineIsBlank = true;
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          HTTP_req += c;
          if (c == '\n' && currentLineIsBlank) {
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-Type: text/html"));
            client.println(F("Connection: close"));
            client.println();
            client.println(F("<!DOCTYPE html>"));
            client.println(F("<html>"));
            client.println(F("<head>"));
            client.print(F("<title>"));
            client.println(F("Band Decoder</title>"));
            client.print(F("<meta http-equiv=\"refresh\" content=\"10;url=http://"));
            client.print(Ethernet.localIP());
            client.println(F("\">"));
            client.println(F("<link href='http://fonts.googleapis.com/css?family=Roboto+Condensed:300italic,400italic,700italic,400,700,300&subset=latin-ext' rel='stylesheet' type='text/css'>"));
            client.println(F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">"));
            client.println(F("<meta name=\"mobile-web-app-capable\" content=\"yes\">"));
            client.println(F("<style type=\"text/css\">"));
            client.println(F("body {font-family: 'Roboto Condensed',sans-serif,Arial,Tahoma,Verdana;background: #ccc;}"));
            client.println(F("a:link  {color: #888;font-weight: bold;text-decoration: none;}"));
            client.println(F("a:visited  {color: #888;font-weight: bold;text-decoration: none;}"));
            client.println(F("a:hover  {color: #888;font-weight: bold;text-decoration: none;}"));
            client.println(F("input {border: 2px solid #ccc;background: #fff;margin: 10px 5px 0 0;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px;color : #333;}"));
            client.println(F("input:hover {border: 2px solid #080;}"));
            client.println(F("input.g {background: #080;color: #fff;}"));
            client.println(F("input.gr {background: #800;color: #fff;}"));
            client.println(F(".box {border: 2px solid #080;background: #ccc;  line-height: 2; margin: 10px 5px 0 5px;padding: 1px 7px 1px 7px;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px;color : #000;}"));
            client.println(F(".boxr {border: 2px solid #800;background: #ccc; line-height: 2; margin: 10px 5px 0 5px;padding: 1px 7px 1px 7px;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px;color : #000;}"));
            // client.println(F(".ptt {border: 2px solid #800;background: #ccc;margin: 10px 15px 0 10px;padding: 1px 7px 1px 7px;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px;color : #800;}"));
            client.println(F("</style></head><body><p>Input TRX <span class=\"boxr\">"));
            #if defined(INPUT_SERIAL)
              client.print(F("Input serial"));
            #endif
            #if defined(ICOM_CIV)
              client.print(F("ICOM CI-V</span><br>CI-V address <span class=\"box\">"));
              client.print(CIV_ADRESS);
              client.print(F("h"));
            #endif
            #if defined(KENWOOD_PC)
              client.print(F("KENWOOD"));
            #endif
            #if defined(FLEX_6000)
              client.print(F("FLEX-6000"));
            #endif
            #if defined(YAESU_CAT)
              client.print(F("YAESU"));
            #endif
            #if defined(YAESU_CAT_OLD)
              client.print(F("YAESU [Old]"));
            #endif
            #if defined(YAESU_CAT_FT100)
              client.print(F("YAESU [FT100]"));
            #endif
            #if defined(INPUT_BCD)
              client.print(F("BCD"));
            #endif
            #if defined(ICOM_ACC)
              client.print(F("ICOM ACC voltage"));
            #endif
            client.print(F("</span><br>Baudrate <span class=\"box\">"));
            client.print(SERBAUD);
            client.print(F("</span><br>Watchdog second <span class=\"box\">"));
            #if defined(WATCHDOG)
              client.print(WATCHDOG);
              client.print(F("</span><br>Request <span class=\"box\">"));
            #endif
            client.print(REQUEST);
            client.print(F("ms</span><br>Band <span class=\"box\">"));
            client.print(BAND);
            client.print(F("</span><br>Frequency <span class=\"boxr\">"));
            client.print(freq);
            client.print(F("Hz</span><br>Power voltage: <span class=\"box\">"));
            client.print(volt(analogRead(VoltagePin),ResistorCoeficient));
            client.println(F(" V</span></p>"));
            client.println(F("<p><a href=\".\" onclick=\"window.open( this.href, this.href, 'width=220,height=350,left=0,top=0,menubar=no,location=no,status=no' ); return false;\" > split&#8599;</a></p>"));
            client.println(F("</body></html>"));

            // CAT_Serial.print(HTTP_req);
            HTTP_req = "";
            break;
          }
          if (c == '\n') {
            currentLineIsBlank = true;
          }
          else if (c != '\r') {
            currentLineIsBlank = false;
          }
        }
      }
      delay(1);
      client.stop();
    }
  #endif
}
//---------------------------------------------------------------------------------------------------------

COLD void NetId(){
  #if defined(EthModule)
  if(millis()-GetNetIdTimer[0]>GetNetIdTimer[1]){
    if(NET_ID != GetBoardId()){
      NET_ID = GetBoardId();
      TxUDP(ThisDevice, RemoteDevice, 'b', 'r', 'o');
    }
  }
  #endif
}
//---------------------------------------------------------------------------------------------------------

#if defined(EthModule)
COLD byte GetBoardId(){
    byte GetBcd = 0;
    if(analogRead(Id1Pin)<50){  // 17 1023
      GetBcd = GetBcd | (1<<0);    // Set the n-th bit
    }
    if(analogRead(Id2Pin)<50){ // 0 170-970
      GetBcd = GetBcd | (1<<1);    // Set the n-th bit
    }
    if(analogRead(Id3Pin)<50){  // 0 290-830
      GetBcd = GetBcd | (1<<2);    // Set the n-th bit
    }
    return(GetBcd);
    // if(digitalRead(Id4Pin)==0){
    //   NET_ID = NET_ID | (1<<3);    // Set the n-th bit
    // }
  }
#endif
//---------------------------------------------------------------------------------------------------------

COLD float volt(int raw, float divider) {
  // float voltage = (raw * 5.0) / 1024.0 * ResistorCoeficient;
  float voltage = float(raw) * ArefVoltage * divider / 1023.0;
  #if defined(SERIAL_debug)
    CAT_Serial.print(F("Voltage "));
    CAT_Serial.println(voltage);
  #endif
  return voltage;
}
//-------------------------------------------------------------------------------------------------------

COLD void DCinMeasure(){
  if (millis() - VoltageRefresh[0] > VoltageRefresh[1]){
    //DCinVoltage = volt(analogRead(VoltagePin), ResistorCoeficient);
    #if defined(LCD)
      if (DCinVoltage<7){
        lcd.setCursor(3, 0);
        lcd.print(F(" Power LOW!"));
      }else if (DCinVoltage>15){
        lcd.setCursor(2, 0);
        lcd.print(F("Power HIGH!"));
      }
    #endif
    VoltageRefresh[0] = millis();                      // set time mark
  }
}
//---------------------------------------------------------------------------------------------------------

HOT void BandDecoderInput(){
	#if !defined(INPUT_BCD)
		InterruptON(0,0); // ptt, enc
	#endif

	//----------------------------------- Select Bank
	#if defined(MULTI_OUTPUT_BY_BCD) && !defined(PWM_OUT)
    SelectBank=B00000000;
    if(digitalRead(BcdIn1Pin)==0){
      bitSet(SelectBank, 0);
    }else if(digitalRead(BcdIn2Pin)==0){
      bitSet(SelectBank, 1);
    }else if(digitalRead(BcdIn3Pin)==0){
      bitSet(SelectBank, 2);
    }else if(digitalRead(BcdIn4Pin)==0){
      bitSet(SelectBank, 3);
    }
    // if BCD not selected, use first
    if(SelectBank==B00000000){
      SelectBank=B00000001;
    }
    if(SelectBank!=SelectBankPrev){
      bandSET();
      SelectBankPrev=SelectBank;
      LcdNeedRefresh=true;
    }
  	#endif

	//----------------------------------- Input Serial
	#if defined(INPUT_SERIAL)
    while (CAT_Serial.available() > 0) {
        BAND = CAT_Serial.parseInt();
        freq = CAT_Serial.parseInt();
        if (CAT_Serial.read() == '\n') {
            bandSET();
            #if defined(SERIAL_echo)
                serialEcho();
            #endif
        }
    }
  	#endif

	//----------------------------------- Icom ACC
	#if defined(ICOM_ACC)
    AccVoltage = volt(analogRead(ADPin), ResistorCoeficient);
    if (counter == 5) {
        // AccVoltage = float(AccVoltage) * ArefVoltage * Divider / 1023.0;

        //=====[ Icom ACC voltage range ]===========================================================

        if (AccVoltage > 0.73 && AccVoltage < 1.00 ) {BAND=10;}  //   6m   * * * * * * * * * * * * * * * *
        if (AccVoltage > 1.00 && AccVoltage < 1.09 ) {BAND=9;}   //  10m   *           Need              *
        if (AccVoltage > 1.09 && AccVoltage < 1.32 ) {BAND=8;}   //  12m   *    calibrated to your       *
        if (AccVoltage > 1.32 && AccVoltage < 1.55 ) {BAND=7;}   //  15m   *         own ICOM            *
        if (AccVoltage > 1.55 && AccVoltage < 1.77 ) {BAND=6;}   //  17m   *     ----------------        *
        if (AccVoltage > 1.77 && AccVoltage < 2.24 ) {BAND=5;}   //  20m   *    (These values have       *
        if (AccVoltage > 0.10 && AccVoltage < 0.50 ) {BAND=4;}   //  30m   *   been measured by any)     *
        if (AccVoltage > 2.24 && AccVoltage < 2.73 ) {BAND=3;}   //  40m   *          ic-746             *
        if (AccVoltage > 2.73 && AccVoltage < 2.99 ) {BAND=2;}   //  80m   *                             *
        if (AccVoltage > 2.99 && AccVoltage < 4.00 ) {BAND=1;}   // 160m   * * * * * * * * * * * * * * * *
        if (AccVoltage > 0.00 && AccVoltage < 0.10 ) {BAND=0;}   // parking

        //==========================================================================================

        bandSET();                                // set outputs
        delay (20);
    }else{
        if (abs(prevAccVoltage-AccVoltage)>10) {            // average
            //means change or spurious number
            prevAccVoltage=AccVoltage;
        }else {
            counter++;
            prevAccVoltage=AccVol tage;
        }
    }
    #if defined(SERIAL_echo)
        serialEcho();
        CAT_Serial.print(AccVoltage);
        CAT_Serial.println(F(" V"));
        CAT_Serial.flush();
    #endif

    delay(500);                                   // refresh time
  	#endif

	//----------------------------------- Icom CIV
	#if defined(ICOM_CIV)
    if (CAT_Serial.available() > 0) {
        incomingByte = CAT_Serial.read();
        #if defined(DEBUG)
          CAT_Serial.print(incomingByte);
          CAT_Serial.print(F("|"));
          CAT_Serial.println(incomingByte, HEX);
        #endif
        icomSM(incomingByte);
        rdIS="";
        // if(rdI[10]==0xFD){    // state machine end
        if(StateMachineEnd == true){    // state machine end
          StateMachineEnd = false;
          for (int i=9; i>=5; i-- ){
              if (rdI[i] < 10) {            // leading zero
                  rdIS = rdIS + 0;
              }
              rdIS = rdIS + String(rdI[i], HEX);  // append BCD digit from HEX variable to string
          }
          freq = rdIS.toInt();
          // CAT_Serial.println(freq);
          // CAT_Serial.println("-------");
          FreqToBandRules();
          bandSET();

          #if defined(SERIAL_echo)
              serialEcho();
          #endif
          RequestTimeout[0]=millis();
        }
    }
  	#endif

	//----------------------------------- Yaesu BCD
	#if defined(INPUT_BCD) && !defined(PWM_OUT)
    if (millis() - BcdInRefresh[0] > BcdInRefresh[1]){
      BAND = 0;
      if(digitalRead(BcdIn1Pin)==BcdInputFormat){
        BAND = BAND | (1<<3);    // Set the n-th bit
      }
      if(digitalRead(BcdIn2Pin)==BcdInputFormat){
        BAND = BAND | (1<<2);
      }
      if(digitalRead(BcdIn3Pin)==BcdInputFormat){
        BAND = BAND | (1<<1);
      }
      if(digitalRead(BcdIn4Pin)==BcdInputFormat){
        BAND = BAND | (1<<0);
      }
      bandSET();
      #if defined(SERIAL_echo)
          serialEcho();
      #endif
      BcdInRefresh[0]=millis();
    }
	#endif

	//----------------------------------- Kenwood
	#if defined(KENWOOD_PC)
    // Data example
    // IF00007069910     -031000 0006000001 ;  // note the space a the end
    // IF00007069910     +074001 0006000001 ;
    rdKS="";

    //#define DEBUG
		#if defined(DEBUG)
			byte incomingByte = CAT_Serial.read();
			Serial.print((char) incomingByte);
			if (incomingByte == 59)
					Serial.println("");
		#else          	
			//CAT_Serial.readBytesUntil(lf, rdK, 38);       // fill array from serial
			Serial.println(msg);

			//if (rdK[0] == 73 && rdK[1] == 70)
			if (msg[0] == 73 && msg[1] == 70 && strlen(msg) == 37)
			{     
				// filter for " IF stuff 1 " message/ The 1 and space at end are fixed.
				// 36 is 1 
				if (!(String(msg[35]).equals("1")) && String(msg[36]).equals(" "))
				{
						Serial.println(F("*** BADLY FORMATTED DATA - EXITING ***"));    // K3 Extended RSP format (K31): DATA sub-mode, if applicable:	
						return;
				}
				
				//Serial.println(F("\n*****  IF message *****"));
					
				// Frequency is 3-13 - done after RIT	
				// 14-18 are blanks - skip them

				// RIT/XIT
				int RIT_sign;
				// 19 is + or - for RIT direction
				if (String(msg[18]).equals('-'))
					RIT_sign = -1;   // 2 is neg, 0 is positive
				else
					RIT_sign = 1;
				//Serial.print("RIT Sign "); Serial.println(RIT_sign);	

				rdKS="";
				// 20-23 position to RIT
				// Get and update RIT/XIT frequency offset (-9999Hz to +9999Hz)
				for (int i=19; i<=22; i++){          
					rdKS = rdKS + String(msg[i]);   // append variable to string
				}
				int32_t rit_temp;
				rit_temp = rdKS.toInt() * RIT_sign;
				if (rit_temp != bandmem[curr_band].RIT)  // do not update unless it changes
				{
					bandmem[curr_band].XIT = bandmem[curr_band].RIT = rit_temp;  // set both XIT and RIT to the same for the K3
					Serial.print(F("X/RIT Value ")); Serial.println(bandmem[curr_band].RIT);							
				}

				// 24 is RIT enabled status	
				rit_temp = String(msg[23]).toInt(); 	// RIT enabled if == 1							
				if (rit_temp != bandmem[curr_band].RIT_en)
				{
					bandmem[curr_band].RIT_en = rit_temp;
					Serial.print("RIT enabled "); Serial.println(bandmem[curr_band].RIT_en);
					displayRIT();
				}

				// 25 is XIT enabled status
				rit_temp = String(msg[24]).toInt();
				if (rit_temp != bandmem[curr_band].XIT_en)
				{
					bandmem[curr_band].XIT_en = rit_temp; 	// XIT enabled if == 1
					//Serial.print("XIT enabled "); Serial.println(bandmem[curr_band].XIT_en);
					displayXIT();
				}

				rdKS="";
				// This is out of byte order since we want the latest RIT status first
				for (int i=2; i<=12; i++)
				{          // 3-13 position to freq
					rdKS = rdKS + String(msg[i]);   // append variable to string
				}				
				freq = rdKS.toInt();				
				//Serial.print("freq is "); Serial.println(freq);

				if (bandmem[curr_band].RIT_en)
					freq += bandmem[curr_band].RIT;

				FreqToBandRules();
				
				if (freq != bandmem[curr_band].vfo_A_last)
				{
					//Serial.println(F("Update VFO "));
					VFOA = bandmem[curr_band].vfo_A_last = freq;
				}

				// 26 is Tx Rx mode
				extern struct User_Settings user_settings[];
				extern uint8_t user_Profile;
				struct User_Settings *pTX = &user_settings[user_Profile];
				pTX->xmit = String(msg[28]).toInt();    // 1 is Tx, 0 is Rx				
				//Serial.print("Transmit is "); Serial.println(pTX->xmit);
				displayFreq();  // update VFO and TX/RX

				// 30 is mode
				// 1 (LSB), 2 (USB), 3 (CW), 4 (FM), 5 (AM), 6 (DATA), 7 (CW-REV), or 9 (DATA-REV).
				int E_mode = String(msg[29]).toInt();    // mode
				//Serial.print("Radio Mode is "); Serial.println(E_mode);
				int new_mode;
				switch (E_mode)
				{
					case 1: new_mode = LSB; break;
					case 2: new_mode = USB; break;
					case 3: new_mode = CW; break;
					case 4: new_mode = FM; break;
					case 5: new_mode = AM; break;
					case 6: new_mode = DATA; break;
					case 7: new_mode = CW_REV; break;
					case 9: new_mode = DATA_REV; break;
					default: new_mode = USB; break;
				}
				//if (new_mode != bandmem[curr_band].mode_A)
				//{
					//Serial.print("New Mode is "); Serial.println(new_mode);
					bandmem[curr_band].mode_A = new_mode;
					selectMode(new_mode);   // Select the mode for the Active VFO 
					displayMode();
					IF_Center_Request();
				//}

				// 31 is VFO in Rx mode - 0 is VFO A, 1 is VFO B         
				if (String(msg[30]).toInt())
					bandmem[curr_band].VFO_AB_Active = VFO_B;	//  which VFO is Active
				else
					bandmem[curr_band].VFO_AB_Active = VFO_A;
				//Serial.print(F("VFO_AB_Active is ")); Serial.println(bandmem[curr_band].VFO_AB_Active);
				displayVFO_AB();
				
				// 32 is scan in progress
				//int E_scan = String(rdK[31]).toInt();    // scan
				//Serial.print(F("Scan in Progress is ")); Serial.println(E_scan);

				// 33 is split mode (1 is yes, 0 is no)
				int split_temp = String(msg[32]).toInt();
				if (split_temp != bandmem[curr_band].split)     // split
				{
					bandmem[curr_band].split = split_temp;
					//Serial.print("Radio Split Mode is "); Serial.println(bandmem[curr_band].split);
					displaySplit();
				}

				// 34 is 0, or if K22 extended mosde is 1 if change is due to a band change
				//int E_bchg = String(rdK[33]).toInt();    // mode
				//Serial.print(F("Band Change? is ")); Serial.println(E_bchg);				
				
				// 35 is DATA_submode
				//int DATA_submode = String(rdK[34]).toInt();    // K3 Extended RSP format (K31): DATA sub-mode, if applicable:
				//Serial.print(F("DATA subMode is ")); Serial.println(DATA_submode); //  (0=DATA A, 1=AFSK A, 2= FSK D, 3=PSK D)

				//Serial.println(F("*****  End Radio Polling *****"));
				

				//bandSET();  // set outputs relay

				#if defined(SERIAL_echo)
				Serial6.print("FA;");
					serialEcho();
				#endif
			}
			memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
			return;
		#endif
    
  	#endif

	//----------------------------------- FLEX-6700
	#if defined(FLEX_6000)
    // http://www.flexradio.com/downloads/smartsdr-cat-user-guide-pdf/#
    // Data exapmple FA
    // FA00007167500;
    // FA00014150000;
    // Data exapmple IF
    // IF00007151074      000000000030000080;
    // IF000035730000100+0000000000090000000;   when 3.573 MHz
    while (CAT_Serial.available()) {
        rdKS="";
        #if defined(DEBUG)
          byte incomingByte = CAT_Serial.read();
          CAT_Serial.write(incomingByte);
        #else
          CAT_Serial.readBytesUntil(lf, rdK, 14);       // fill array from serial
            if (rdK[0] == 70 && rdK[1] == 65){     // filter
                for (int i=2; i<=12; i++){          // 3-13 position to freq
                    rdKS = rdKS + String(rdK[i]);   // append variable to string
                }
                freq = rdKS.toInt();
                FreqToBandRules();
                bandSET();                                              // set outputs relay

                #if defined(SERIAL_echo)
                    serialEcho();
                #endif
            }
            memset(rdK, 0, sizeof(rdK));   // Clear contents of Buffer
          #endif
    }
  #endif

  //----------------------------------- Yaesu CAT
  #if defined(YAESU_CAT)
  while (CAT_Serial.available()) {
      rdYS="";
      #if defined(DEBUG)
        byte incomingByte = CAT_Serial.read();
        CAT_Serial.write(incomingByte);
      #else
      CAT_Serial.readBytesUntil(lf, rdY, 38);         // fill array from serial
          if (rdY[0] == 73 && rdY[1] == 70){      // filter
              for (int i=5; i<=12; i++){          // 6-13 position to freq
                  rdYS = rdYS + String(rdY[i]);   // append variable to string
              }
              freq = rdYS.toInt();
              FreqToBandRules();
              bandSET();                                              // set outputs relay

              #if defined(SERIAL_echo)
                  serialEcho();
              #endif
          }
          memset(rdY, 0, sizeof(rdY));   // Clear contents of Buffer
      #endif
  }
  #endif
  //----------------------------------- Yaesu CAT OLD
  #if defined(YAESU_CAT_OLD)
  while (CAT_Serial.available()) {
      rdYOS="";
      #if defined(DEBUG)
        byte incomingByte = CAT_Serial.read();
        CAT_Serial.write(incomingByte);
        CAT_Serial.print(F(" "));
      #else
        CAT_Serial.readBytesUntil('240', rdYO, 5);                   // fill array from serial (240 = 0xF0)
        if (rdYO[0] != 0xF0 && rdYO[1] != 0xF0 && rdYO[2] != 0xF0 && rdYO[3] != 0xF0 && rdYO[4] != 0xF0 && rdYO[5] != 0xF0){     // filter
            for (int i=0; i<4; i++ ){
                if (rdYO[i] < 10) {                              // leading zero
                    rdYOS = rdYOS + 0;
                }
                rdYOS = rdYOS + String(rdYO[i], HEX);            // append BCD digit from HEX variable to string
            }
            rdYOS = rdYOS + 0;                                   // append Hz
            freq = rdYOS.toInt();
            FreqToBandRules();
            bandSET();                                                                // set outputs relay

            #if defined(SERIAL_echo)
                serialEcho();
            #endif
        }
        memset(rdYO, 0, sizeof(rdYO));   // Clear contents of Buffer
      #endif
    }
  #endif

  //----------------------------------- Yaesu CAT FT100
  #if defined(YAESU_CAT_FT100)
  union ArrayToInteger {
    byte array[5];
    uint32_t integer;
  };
  ArrayToInteger convert;
  while (CAT_Serial.available()) {
    #if defined(DEBUG)
      byte incomingByte = CAT_Serial.read();
      CAT_Serial.write(incomingByte);
    #else
      int numberOfBytes = CAT_Serial.readBytes(rdYO, 32);
      if(numberOfBytes == 32){
          convert.array[4] = 0;
          convert.array[3] = rdYO[1];
          convert.array[2] = rdYO[2];
          convert.array[1] = rdYO[3];
          convert.array[0] = rdYO[4];
          freq = convert.integer * 1.25;                    //fq data read back from FT-100 is the number of steps in 1.25Hz

          FreqToBandRules();                                // map fq to band
          bandSET();                                        // set outputs relay
      }
      memset(rdYO, 0, sizeof(rdYO));   // Clear contents of Buffer
    #endif
  }
  #endif

  #if !defined(INPUT_BCD)
    //InterruptON(1,1); // ptt, enc
  #endif
}

//---------------------------------------------------------------------------------------------------------

COLD void BandDecoderOutput(){

  //=====[ Output Icom CIV ]=======================
  #if defined(ICOM_CIV_OUT)
      if(freq!= freqPrev1){                    // if change
          txCIV(0, freq, CIV_ADR_OUT);         // 0 - set freq
          freqPrev1 = freq;
      }
  #endif
  //=====[ Output Kenwood PC ]=====================
  #if !defined(REQUEST) && defined(KENWOOD_PC_OUT)
      if(freq != freqPrev2){                     // if change
          String freqPCtx = String(freq);        // to string
          freqPCtx.reserve(12);
          while (freqPCtx.length() < 11) {       // leding zeros
              freqPCtx = 0 + freqPCtx;
          }
         CAT_Serial.print("FA" + freqPCtx + ";");    // sets both VFO
         CAT_Serial.print("FB" + freqPCtx + ";");
//          CAT_Serial.print("FA" + freqPCtx + ";");    // first packet not read every time
         CAT_Serial.flush();
         freqPrev2 = freq;
      }
  #endif
  //=====[ Output Yaesu CAT ]=====================
  #if !defined(REQUEST) && defined(YAESU_CAT_OUT)
      if(freq != freqPrev2){                     // if change
          String freqPCtx = String(freq);        // to string
          while (freqPCtx.length() < 8) {        // leding zeros
              freqPCtx = 0 + freqPCtx;
          }
         CAT_Serial.print("FA" + freqPCtx + ";");    // sets both VFO
         CAT_Serial.print("FB" + freqPCtx + ";");
         CAT_Serial.flush();
         freqPrev2 = freq;
      }
  #endif
  //=====[ Output Yaesu CAT OLD ]=================
  #if !defined(REQUEST) && defined(YAESU_CAT_OUT_OLD)
      if(freq != freqPrev2){                     // if change
          String freqPCtx = String(freq);        // to string
          while (freqPCtx.length() < 8) {        // leding zeros
              freqPCtx = 0 + freqPCtx;
         }
         CAT_Serial.write(1);                        // set freq
         CAT_Serial.flush();
         freqPrev2 = freq;
      }
  #endif

}
//---------------------------------------------------------------------------------------------------------
#ifdef BANDSET
COLD void bandSET() {                                               // set outputs by BAND variable

  if(BAND==0 && previousBAND != 0){    // deactivate PTT
    //digitalWrite(PttOffPin, HIGH);
    PTT = true;
    #if defined(LCD)
      LcdNeedRefresh = true;
    #endif
  }else if(BAND!=0 && previousBAND == 0){    // deactivate PTT
    //digitalWrite(PttOffPin, LOW);
  }

  #if !defined(PTT_BY_BAND)
  if((PTT==false && previousBAND != 0 ) || (PTT==true && previousBAND == 0)){
  #endif
    for (int i = 0; i < NumberOfBoards; i++) {
      ShiftByte[i] = B00000000;
    }

    #if defined(MULTI_OUTPUT_BY_BCD)
      for (int i = 0; i < 17; i++) {   // outputs 1-8
        for (int y = 0; y < 4; y++) { // bcd bit
          if(bitRead(SelectBank,y)==1 && bitRead(matrix[BAND][i],y)==1){
            if(i<8){
              bitSet(ShiftByte[0], i);
            }else{
              bitSet(ShiftByte[1], i-8);
            }
          }
        }
      }
    #else
      for (int i = 0; i < 8; i++) {   // outputs 1-8
        if(matrix[BAND][i]>0){
          ShiftByte[0] = ShiftByte[0] | (1<<i);
        }
      }
      for (int i = 8; i < 16; i++) {   // outputs 9-16
        if(matrix[BAND][i]>0){
          ShiftByte[1] = ShiftByte[1] | (1<<(i-8));
        }
      }
      for (int i = 16; i < 24; i++) {   // outputs 17-24
        if(matrix[BAND][i]>0){
          ShiftByte[2] = ShiftByte[2] | (1<<(i-16));
        }
      }
      for (int i = 24; i < 32; i++) {   // outputs 25-32
        if(matrix[BAND][i]>0){
          ShiftByte[3] = ShiftByte[3] | (1<<(i-24));
        }
      }
      for (int i = 32; i < 40; i++) {   // outputs 33-40
        if(matrix[BAND][i]>0){
          ShiftByte[4] = ShiftByte[4] | (1<<(i-32));
        }
      }
    #endif

    //digitalWrite(ShiftOutLatchPin, LOW);    // ready for receive data
      for (int i = NumberOfBoards; i >0; i--) {   // outputs 9-16
        //shiftOut(ShiftOutDataPin, ShiftOutClockPin, LSBFIRST, ShiftByte[i-1]);
      }
    ////////////////////////////////////////////////         digitalWrite(ShiftOutLatchPin, HIGH);    // switch to output pin

    #if !defined(INPUT_BCD) && !defined(MULTI_OUTPUT_BY_BCD) && !defined(PWM_OUT)
        //bcdOut();
    #endif

    #if defined(EthModule)
      TxUDP(ThisDevice, RemoteDevice, ShiftByte[1], ShiftByte[0], 0x00);
    #endif

    #if defined(LCD)
      LcdNeedRefresh = true;
    #endif
  #if !defined(PTT_BY_BAND)
  }
  #endif

  #if defined(PWM_OUT)
    analogWrite(PwmOutPin, PwmByBand[BAND]);
  #endif

  // #if defined(EthModule)
  //   if(DetectedRemoteSw[NET_ID][4]==0 || RemoteSwLatencyAnsw==0){
  //     //  && millis() < RemoteSwLatency[0]+RemoteSwLatency[1]*5) ){
  //     digitalWrite(PttOffPin, HIGH);
  //     #if defined(UdpBroadcastDebug_debug)
  //       TxBroadcastUdp("BandSet-" + String(DetectedRemoteSw[NET_ID][4]) + "-" + String(RemoteSwLatencyAnsw) );
  //     #endif
  //     PTT = true;
  //     #if defined(LCD)
  //     LcdNeedRefresh = true;
  //     #endif
  //   }
  // #endif
  #if defined(EthModule_XXX)
    #if defined(BcdIpRelay)
      TxUDP(ThisDevice, RemoteDevice, BAND, 0x00, 0x00);
    #else
      byte A = 0x00;
      for (int i = 0; i < 8; i++) {   // outputs 1-8
        if(matrix[BAND][i]==1){
          A = A | (1<<i); // set n-th bit
        }
      }
      byte B = 0x00;
      for (int i = 8; i < 16; i++) {   // outputs 8-16
        if(matrix[BAND][i]==1){
          B = B | (1<<i-8); // set n-th bit
        }
      }
      TxUDP(ThisDevice, RemoteDevice, A, B, 0x00);
    #endif
  #endif

  previousBAND = BAND;
  #if defined(WATCHDOG)
    WatchdogTimeout[0] = millis();                   // set time mark
  #endif
}
#endif
//---------------------------------------------------------------------------------------------------------

COLD void remoteRelay() {
    CAT_Serial.print(1);
    CAT_Serial.print(',');
    CAT_Serial.print(BAND, DEC);
    CAT_Serial.print('\n');
    CAT_Serial.flush();
}
//---------------------------------------------------------------------------------------------------------

COLD void serialEcho() {
    CAT_Serial.print(F("<"));
    CAT_Serial.print(BAND);
    CAT_Serial.print(F(","));
    CAT_Serial.print(freq);
    CAT_Serial.print(F("> "));
    CAT_Serial.println(ShiftByte[0], BIN);
    CAT_Serial.flush();
}
//---------------------------------------------------------------------------------------------------------

COLD void bcdOut(){
    //if (BCDmatrixOUT[0][BAND] == 1){ digitalWrite(BcdIn1Pin, HIGH); }else{ digitalWrite(BcdIn1Pin, LOW);}
    //if (BCDmatrixOUT[1][BAND] == 1){ digitalWrite(BcdIn2Pin, HIGH); }else{ digitalWrite(BcdIn2Pin, LOW);}
    //if (BCDmatrixOUT[2][BAND] == 1){ digitalWrite(BcdIn3Pin, HIGH); }else{ digitalWrite(BcdIn3Pin, LOW);}
    //if (BCDmatrixOUT[3][BAND] == 1){ digitalWrite(BcdIn4Pin, HIGH); }else{ digitalWrite(BcdIn4Pin, LOW);}
}
//---------------------------------------------------------------------------------------------------------

COLD void watchDog() {
  #if defined(WATCHDOG)
    if((millis() - WatchdogTimeout[0]) > WatchdogTimeout[1]) {
      BAND=0;
      freq=0;
      //bandSET();                                 // set outputs
    }
  #endif
}
//---------------------------------------------------------------------------------------------------------

#if defined(ICOM_CIV) || defined(ICOM_CIV_OUT)
/*http://www.plicht.de/ekki/civ/civ-p0a.html
stty -F /dev/ttyUSB3 115200 raw -echo -echoe -echok -echoctl -echoke
echo -n -e '\xFE\xFE\x00\xA2\x00\x80\x48\x06\x44\x01\xFD' > /dev/ttyUSB3
echo -n -e '\xFE\xFE\x00\xA2\x00\x00\x50\x11\x32\x04\xFD' > /dev/ttyUSB3
echo -n -e '\xFE\xFE\x00\xA2\x00\x00\x20\x30\x96\x12\xFD' > /dev/ttyUSB3
FE|FE|0|56|0|70|99|99|52|0|FD
FE|FE|0|56|0|30| 0| 0|53|0|FD
IC9700
FE|FE|0|A2|0|80|48| 6|44|1|FD 2m
FE|FE|0|A2|0| 0|50|11|32|4|FD 70cm
FE|FE|0|A2|0| 0|20|30|96|12|FD 23cm

*/
    int icomSM(byte b){      // state machine
        // This filter solves read from 0x00 0x05 0x03 commands and 00 E0 F1 address used by software
        static bool Band23cm;

        // CAT_Serial.print(b, HEX);
        // CAT_Serial.print(" | ");
        // CAT_Serial.print(state);
        // CAT_Serial.print(" | ");
        // CAT_Serial.println(Band23cm);

        switch (state) {
            case 1: if( b == 0xFE ){ state = 2; rdI[0]=b; rdI[10]=0x00; }; break;
            case 2: if( b == 0xFE ){ state = 3; rdI[1]=b; }else{ state = 1;}; break;
            // addresses that use different software 00-trx, e0-pc-ale, winlinkRMS, f1-winlink trimode
            case 3: if( b == 0x00 || b == 0xE0 || b == 0x0E || b == 0xF1 ){ state = 4; rdI[2]=b;                       // choose command $03
            }else if( b == CIV_ADRESS ){ state = 6; rdI[2]=b;
                    }else if( b == 0xFE ){ state = 3; rdI[1]=b;      // FE (3x reduce to 2x)
                    }else{ state = 1;}; break;                       // or $05

            case 4: if( b == CIV_ADRESS ){ state = 5; rdI[3]=b; }else{ state = 1;}; break;                      // select command $03
            case 5: if( b == 0x00 || b == 0x03 ){state = 8; rdI[4]=b;  // freq
                    }else if( b == 0x04 ){state = 14; rdI[4]=b;        // mode
                    }else if( b == 0xFE ){ state = 2; rdI[0]=b;        // FE
                    }else{ state = 1;}; break;

            case 6: if( b == 0x00 || b == 0xE0 || b == 0xF1 ){ state = 7; rdI[3]=b; }else{ state = 1;}; break;  // select command $05
            case 7: if( b == 0x00 || b == 0x05 ){ state = 8; rdI[4]=b; }else{ state = 1;}; break;

            case 8: if( b <= 0x99 ){state = 9; rdI[5]=b;             // 10Hz 1Hz
                    }else if( b == 0xFE ){ state = 2; rdI[0]=b;      // FE
                    }else{state = 1;}; break;
            case 9: if( b <= 0x99 ){state = 10; rdI[6]=b;            // 1kHz 100Hz
                    }else if( b == 0xFE ){ state = 2; rdI[0]=b;      // FE
                    }else{state = 1;}; break;
           case 10: if( b <= 0x99 ){state = 11; rdI[7]=b;            // 100kHz 10kHz
                    Band23cm = false;
                    }else if( b == 0xFE ){ state = 2; rdI[0]=b;      // FE
                    }else{state = 1;}; break;
           case 11: if( b <= 0x52 || b == 0x96 ){ state = 12; rdI[8]=b;            // 10MHz 1Mhz
                    if(b == 0x96){ Band23cm = true; }
                    }else if( b == 0xFE ){ state = 2; rdI[0]=b;      // FE
                    }else{ state = 1; } break;
           case 12: if( b <= 0x01 || b == 0x04 || (b == 0x12 && Band23cm==true) ){state = 13; rdI[9]=b; // 1GHz 100MHz  <-- 1xx/4xx/12xx MHz limit
                    }else if( b == 0xFE ){ state = 2; rdI[0]=b;      // FE
                    }else{state = 1;}; break;
           case 13: if( b == 0xFD ){state = 1; rdI[10]=b; StateMachineEnd = true;
                    }else if( b == 0xFE ){ state = 2; rdI[0]=b;      // FE
                    }else{state = 1; rdI[10] = 0x00;}; break;

           case 14: if( b <= 0x12 ){state = 15; rdI[5]=b;
                    }else if( b == 0xFE ){ state = 2; rdI[0]=b;      // FE
                    }else{state = 1;}; break;   // Mode
           case 15: if( b <= 0x03 ){state = 16; rdI[6]=b;
                    }else if( b == 0xFE ){ state = 2; rdI[0]=b;      // FE
                    }else{state = 1;}; break;   // Filter
           case 16: if( b == 0xFD ){state = 1; rdI[7]=b;
                    }else if( b == 0xFE ){ state = 2; rdI[0]=b;      // FE
                    }else{state = 1; rdI[7] = 0;}; break;
        }
    }
    //---------------------------------------------------------------------------------------------------------
    bool CheckPartFreqInRange(byte FRQ, int PART){
      bool result=false;
      for (int i=0; i<16; i++){
        if( PartFreqToByte(Freq2Band[i][0],PART)<FRQ && FRQ<PartFreqToByte(Freq2Band[i][1],PART)){
          result=true;
        }
      }
      return result;
    }
    //---------------------------------------------------------------------------------------------------------
    byte PartFreqToByte(uint32_t FREQ, int PART){
      String StrFreq=String(FREQ);
      StrFreq.reserve(10);
      while (StrFreq.length() < 10){                 // leding zeros
        StrFreq = 0 + StrFreq;
      }
      StrFreq=StrFreq.substring(PART*2,PART*2+2);
      byte PartFreq = StrFreq.toInt();
      return PartFreq;
    }
    //---------------------------------------------------------------------------------------------------------

    int txCIV(int commandCIV, uint32_t dataCIVtx, int toAddress) {
        //CAT_Serial.flush();
        CAT_Serial.write(254);                                    // FE
        CAT_Serial.write(254);                                    // FE
        CAT_Serial.write(toAddress);                              // to adress
        CAT_Serial.write(fromAdress);                             // from OE
        CAT_Serial.write(commandCIV);                             // data
        if (dataCIVtx != 0){
            String freqCIVtx = String(dataCIVtx);             // to string
            freqCIVtx.reserve(11);
            String freqCIVtxPart;
            freqCIVtxPart.reserve(11);
            while (freqCIVtx.length() < 10) {                 // leding zeros
                freqCIVtx = 0 + freqCIVtx;
            }
            for (int x=8; x>=0; x=x-2){                       // loop for 5x2 char [xx xx xx xx xx]
                freqCIVtxPart = freqCIVtx.substring(x,x+2);   // cut freq to five part
                    CAT_Serial.write(hexToDec(freqCIVtxPart));    // HEX to DEC, because write as DEC format from HEX variable
            }
        }
        CAT_Serial.write(253);                                    // FD
        // CAT_Serial.flush();
        while(CAT_Serial.available()){        // clear buffer
          CAT_Serial.read();
        }
    }
    //---------------------------------------------------------------------------------------------------------

    unsigned int hexToDec(String hexString) {
        hexString.reserve(2);
        unsigned int decValue = 0;
        int nextInt;
        for (int i = 0; i < hexString.length(); i++) {
            nextInt = int(hexString.charAt(i));
            if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
            if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
            if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
            nextInt = constrain(nextInt, 0, 15);
            decValue = (decValue * 16) + nextInt;
        }
        return decValue;
    }
    //---------------------------------------------------------------------------------------------------------
#endif

COLD void FreqToBandRules(){
         if (freq >=Freq2Band[0][0] && freq <=Freq2Band[0][1] )  {BAND=1;}  // 160m
    else if (freq >=Freq2Band[1][0] && freq <=Freq2Band[1][1] )  {BAND=2;}  //  80m
    else if (freq >=Freq2Band[2][0] && freq <=Freq2Band[2][1] )  {BAND=3;}  //  40m
    else if (freq >=Freq2Band[3][0] && freq <=Freq2Band[3][1] )  {BAND=4;}  //  30m
    else if (freq >=Freq2Band[4][0] && freq <=Freq2Band[4][1] )  {BAND=5;}  //  20m
    else if (freq >=Freq2Band[5][0] && freq <=Freq2Band[5][1] )  {BAND=6;}  //  17m
    else if (freq >=Freq2Band[6][0] && freq <=Freq2Band[6][1] )  {BAND=7;}  //  15m
    else if (freq >=Freq2Band[7][0] && freq <=Freq2Band[7][1] )  {BAND=8;}  //  12m
    else if (freq >=Freq2Band[8][0] && freq <=Freq2Band[8][1] )  {BAND=9;}  //  10m
    else if (freq >=Freq2Band[9][0] && freq <=Freq2Band[9][1] ) {BAND=10;}  //   6m
    else if (freq >=Freq2Band[10][0] && freq <=Freq2Band[10][1] ) {BAND=11;}  // 2m
    else if (freq >=Freq2Band[11][0] && freq <=Freq2Band[11][1] ) {BAND=12;}  // 70cm
    else {BAND=0;}   // out of range
}

void AGC_Decode(void)
{
    rdKS="";
	if (msg[0] == 71 && msg[1] == 84)  // Look for GTxxx i.e. GT002 = AGC Fast, GT004 = AGC Slow
	{     				
		//Serial.println(F("\n*****  AGC Update *****"));

		for (int i=2; i<=4; i++)
		{          // 3-5 position
			rdKS = rdKS + String(msg[i]);   // append variable to string
		}				
		int RadioAGC = rdKS.toInt();	
		Serial.print("Radio AGC is "); Serial.println(RadioAGC);
		if (RadioAGC == 2)
			bandmem[curr_band].agc_mode = AGC_FAST;
		if (RadioAGC == 4)
			bandmem[curr_band].agc_mode = AGC_SLOW;
		if (RadioAGC == 0)
			bandmem[curr_band].agc_mode = AGC_OFF;
		displayAgc();
		memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
		return;					
	}
	memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
}

void Filter_Decode(void)
{
    rdKS="";
	if ((msg[0] == 66 && msg[1] == 87) || (msg[0] == 70 && msg[1] == 87))  // Look for BWxxxx i.e. BW0280 = 2800Hz band filter width messages
	{     				
		//Serial.println(F("\n*****  Filter Width Update *****"));

		for (int i=2; i<=5; i++)
		{          // 3-6 position
			rdKS = rdKS + String(msg[i]);   // append variable to string
		}				
		filterWidth = rdKS.toInt() * 10;	
		Serial.print("Filterwidth is "); Serial.println(filterWidth);
		displayFilter();
		memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
		return;					
	}
	memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
}

void BarGraph_Decode(void)
{
	rdKS="";
	if (msg[0] == 66 && msg[1] == 71)  // Look for BGxxY i.e. BG04R bar graph messages
	{     				
		//Serial.println(F("\n*****  Bar Graph Update *****"));

		for (int i=2; i<=3; i++)
		{          // 3-4 position to freq
			rdKS = rdKS + String(msg[i]);   // append variable to string
		}				
		barGraph = rdKS.toInt();
		
		int xmit;

		if (msg[4] == 'T')  // cheap way to determine if radio is in transmit without a dedicated query
			xmit = ON;
		else 
			xmit = OFF;

		if (xmit != user_settings[user_Profile].xmit)	
		{		
			user_settings[user_Profile].xmit = xmit;
			displayFreq();
		}		
		memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
		return;					
	}
	memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
}

void RadioMode_Decode(void)
{
    rdKS="";
			
	if (msg[0] == 77 && msg[1] == 68)  // Look for MDx i.e. MD1 or MD7
	{     		
		int new_mode = 0;

		//Serial.println(F("\n*****  Radio Mode Selection Update *****"));
		switch (msg[2])
		{
			case 1: new_mode = LSB; break;
			case 2: new_mode = USB; break;
			case 3: new_mode = CW; break;
			case 4: new_mode = FM; break;
			case 5: new_mode = AM; break;
			case 6: new_mode = DATA; break;
			case 7: new_mode = CW_REV; break;
			case 9: new_mode = DATA_REV; break;
			default: new_mode = USB; break;
		}
		Serial.print("New Mode is "); Serial.println(new_mode);
		bandmem[curr_band].mode_A = new_mode;
		selectMode(new_mode);   // Select the mode for the Active VFO 
		IF_Center_Request();    // get the IF shift that occurs on mode changes for panadapters
		displayMode();
		 
		memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
		return;			
	}
	memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
}

void ANT_Decode(void)
{
    rdKS="";
			
	if (msg[0] == 65 && msg[1] == 78)  // Look for ANxxx i.e. AN1 or AN2
	{     				
		//Serial.println(F("\n*****  Antenna Selection Update *****"));
		
		bandmem[curr_band].ant_sw = atoi(&msg[2]);
		displayANT();
		 
		memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
		return;			
	}
	memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
}

// RSP format: FInnnn; where nnnn represents the last 4 digits of the K3’s present I.F. center frequency in Hz
void IF_Center_Decode(void)
{
    rdKS="";
			
	if (msg[0] == 70 && msg[1] == 73)  // Look for FIxxxx i.e. FI8215
	{     				
		//Serial.println(F("\n*****  IF Center Frequency Update *****"));

		for (int i=2; i<=5; i++)
		{          // 3-6 position to freq
			rdKS = rdKS + String(msg[i]);   // append variable to string
		}				
		Fc= rdKS.toInt() - 5000;   // 8.215.000 is normal cntger IF.  5000 is last 4 digits
		
		Serial.print(F("Update Fc ")); Serial.println(Fc);
		selectFrequency(0);
		displayFreq();
		 
		memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
		return;			
	}
	memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
}

void VFOB_Decode(void)
{
    rdKS="";
			
	if (msg[0] == 70 && msg[1] == 66)  // Look for FBxxx i.e. FB00014065940 - look for FB VFO B mnessages
	{     				
		//Serial.println(F("\n*****  VFO B Update *****"));

		for (int i=2; i<=12; i++)
		{          // 3-13 position to freq
			rdKS = rdKS + String(msg[i]);   // append variable to string
		}				
		freq = rdKS.toInt();
		FreqToBandRules();   // not using fore the panadpater but is used for othe parts of the decoder feature set if used.
		if (freq != bandmem[curr_band].vfo_B_last)
		{
			Serial.println(F("Update VFO B"));
			VFOB = bandmem[curr_band].vfo_B_last = freq;
			displayFreq();
		} 
		memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
		return;			
	}
	memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
}

void VFOA_Decode(void)
{
	rdKS="";

	if (msg[0] == 70 && msg[1] == 65)  // Look for FAxxx i.e. FA00014065940 - look for FA VFO B mnessages
	{     				
		//Serial.println(F("\n*****  VFO A Update *****"));

		for (int i=2; i<=12; i++)
		{          // 3-13 position to freq
			rdKS = rdKS + String(msg[i]);   // append variable to string
		}				
		freq = rdKS.toInt();
		FreqToBandRules();   // not using fore the panadpater but is used for othe parts of the decoder feature set if used.
		if (freq != bandmem[curr_band].vfo_A_last)
		{
			Serial.println(F("Update VFO A"));
			VFOA = bandmem[curr_band].vfo_A_last = freq;
			displayFreq();
		}
		memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
		return;			
	}
	memset(msg, 0, sizeof(msg));   // Clear contents of Buffer
}

void AGC_Request(void)	  	// Get the filter width
{
	CAT_Serial.print(F("GT;"));
	CAT_Serial.flush();       	// Waits for the transmission of outgoing serial data to complete
}

void Filter_Request(void)	  	// Get the filter width
{
	CAT_Serial.print(F("BW;"));
	CAT_Serial.flush();       	// Waits for the transmission of outgoing serial data to complete
}

void VFOA_Request(void)			// Get VFO A
{
	CAT_Serial.print(F("FA;"));
	CAT_Serial.flush();       	// Waits for the transmission of outgoing serial data to complete
}

void VFOB_Request(void)			// Get VFO B
{
	CAT_Serial.print(F("FB;"));
	CAT_Serial.flush();       	// Waits for the transmission of outgoing serial data to complete
}

void BarGraph_Request(void)		// Get the bar graph value
{
	//Serial.println(F("Requesting Bar Graph Update"));
	CAT_Serial.print(F("BG;"));
	CAT_Serial.flush();       	// Waits for the transmission of outgoing serial data to complete
}

// IF Center Frewquency: 
// RSP format: FInnnn; where nnnn represents the last 4 digits of the K3’s present I.F. center frequency in Hz
void IF_Center_Request(void)		// Get the IF Center Freq value
{
	//Serial.println(F("Requesting IF Center Freq Update"));
	CAT_Serial.print(F("FI;"));
	CAT_Serial.flush();       	// Waits for the transmission of outgoing serial data to complete
}

void RadioMode_Request(void)		// Get the Mode Selection value
{
	//Serial.println(F("Requesting Mode Selection Update"));
	CAT_Serial.print(F("MD;"));
	CAT_Serial.flush();       	// Waits for the transmission of outgoing serial data to complete
}

void ANT_Request(void)		// Get the Antenna Selection value
{
	//Serial.println(F("Requesting Antenna Selection Update"));
	CAT_Serial.print(F("AN;"));
	CAT_Serial.flush();       	// Waits for the transmission of outgoing serial data to complete
}

int16_t CAT_msgs(void)
{	
	char c = 0;
	static int8_t i = 0;
	int count;
	
	while ((count = CAT_Serial.available()) && ((c = CAT_Serial.read()) != 59))
	{	
		//Serial.print(c);

		if (count > S_BUFF || c < 30)		// bail if we blow out a string, miss a terminator
		{	// clean up for next message
			Serial.print(F("Reached string buffer limit or invalid char - count = "));	Serial.println(count);
			i = 0;
			CAT_Serial.clear();
			return 0;
		}

		msg[i] = c;		// accumulate chars until terminator	
		msg[i+1] = '\0';
		i++;
	}
	
	if (c == 59)				// If terninator, stop, send string off to the right function
	{
		// clean up for next message
		//Serial.println("");
		//Serial.print("msg=");Serial.println(msg);
		//Serial.print("i="); Serial.println(i);
		
		if (!strncmp(msg, "FA", 2))
			VFOA_Decode();
		else if (!strncmp(msg, "FB", 2))
			VFOB_Decode();
		else if (!strncmp(msg, "BW", 2) || !strncmp(msg, "FW", 2))
			Filter_Decode();
		else if (!strncmp(msg, "BG", 2))	
			BarGraph_Decode();
		else if (!strncmp(msg, "IF", 2))	
			BandDecoderInput();
		else if (!strncmp(msg, "GT", 2))	
			AGC_Decode();	
		else if (!strncmp(msg, "AN", 2))	
			ANT_Decode();
		else if (!strncmp(msg, "FI", 2))	
			IF_Center_Decode();
    else if (!strncmp(msg, "MD", 2))	
			RadioMode_Decode();
		i = 0;
		msg[0] = 0;
		return 1;
	}
	return 0;
}
#endif  // ALL_CAT
