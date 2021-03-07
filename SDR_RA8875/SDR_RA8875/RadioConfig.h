//
//      RadioConfig.h
//
//  This file is the central warehouse for operational parameters.  
//  Key to this operation is are stuctures that hold the majority of current band settings
//  Other key parameters are current settings values which may be modified from the last used value
//  in the structure.  These are usually things like VFO dial settings and should be stored in 
//  EEPROM at some point like band changes when the dial has stopped for a certain length of time
//
// Soem defines for ease of use 
#define CW          0
#define LSB         1     
#define USB         2
#define DATA        3
#define ON          1
#define OFF         0
#define XVTR        0
#define ANT1        1
#define ANT2        2
#define ANT3        3
#define ATTEN_OFF   0       // Bypass
#define ATTEN_ON    1       // Turn relay on
#define PREAMP_OFF  0       // Bypass
#define PREAMP_ON   1       // Switch relay on
#define PRESEL_BYP  0       // Preselector Fitler or Bypass
#define PRESEL1     1
#define PRESEL2     2
#define PRESEL3     3
#define PRESEL4     4
#define PRESEL5     5
#define PRESEL6     6
#define PRESEL7     7
#define PRESEL8     8   
#define PRESEL9     9
#define PRESEL10    10
#define PRESEL11    11
#define PRESEL12    12
#define PRESEL13    13
#define PRESEL14    14
#define PRESEL15    15
#define PRESEL16    16
#define XVTR1       1       // Transverter band Slot ID
#define XVTR2       2
#define XVTR3       3
#define XVTR4       4
#define XVTR5       5
#define XVTR6       6
#define XVTR7       7
#define XVTR8       8
#define XVTR9       9
#define XVTR10      10
#define XVTR11      11
#define XVTR12      12
#define BAND1       1       // Band slot ID
#define BAND2       2
#define BAND3       3
#define BAND4       4
#define BAND5       5
#define BAND6       6
#define BAND7       7       
#define BAND8       8
#define BAND9       9
#define BAND10      10
#define BAND11      11
#define BAND12      12
#define AGC_OFF     0       // Index to AGC Settings table
#define AGC_SLOW    1
#define AGC_MED     2
#define AGC_FAST    3
#define MIC_OFF     0
#define MIC_ON      1


// Function declarations
void NextBand(void);

// Our Database of settings
#define BANDS   11
struct Band_Memory {
    char        band_name[20];
    float       edge_lower;
    float       edge_upper;
    float       vfo_A_last;
    float       vfo_B_last;
    uint8_t     mode;           // CW, LSB, USB, DATA
    uint8_t     band_num;
    uint8_t     tune_step;
    uint8_t     agc_mode;       // index to group of AGC settings in antoehr table
    uint8_t     split;
    float       RIT;
    float       XIT;
    uint8_t     tuner;          // enable ATU or not
    uint8_t     ant_sw;         // antenna selector switch
    uint8_t     preselector;    // preselector band set value
    uint8_t     attenuator;     // 0 = bypass, >0 is attenuation value to set
    uint8_t     preamp; 
    uint8_t     mic_input_en;   // mic on or off
    float       mic_Gain_last;
    uint8_t     lineIn_en;      // line in on or off
    float       lineIn_Vol_last;
    uint8_t     spkr_en;        // 0 is disable or mute
    float       spkr_Vol_last;  // last seting for unmut or power on (Wen we store in EEPROM)
    uint8_t     lineOut_en;     // line out on or off
    float       lineOut_Vol_last;
    uint8_t     xvtr_en;   // use Tranverter Table or not
    uint8_t     xvtr_num;  // index to Transverter Table 
} static bandmem[BANDS] = {
    {"160M", 1800.0, 2000.0, 1840.0, 1860.0, LSB, BAND1,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL1,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0 },
    { "80M", 3500.0, 4000.0, 3573.0, 3830.0, LSB, BAND2,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL2,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0 },
    { "60M", 5000.0, 5000.0, 5000.0, 5000.0, USB, BAND3,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL3,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0 },
    { "40M", 7000.0, 7300.0, 7040.0, 7200.0,DATA, BAND4,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL4,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0 },
    { "30M",10000.0,10200.0,10136.0,10136.0,DATA, BAND5,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL5,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0 },
    { "20M",14000.0,14350.0,21074.0,14200.0,DATA, BAND6,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL6,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0 },
    { "17M",18000.0,18150.0,18000.0,18000.0, USB, BAND7,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL7,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0 },
    { "15M",21000.0,21450.0,21074.0,21350.0,DATA, BAND8,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL8,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0 },
    { "12M",24890.0,25000.0,24890.0,24920.0, USB, BAND9,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL9,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0 },
    { "10M",28000.0,29600.0,28100.0,28074.0,DATA,BAND10,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1,PRESEL10,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0 },
    {  "6M",50000.0,54000.0,50125.0,50313.0, USB,BAND11,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1,PRESEL11,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0 }
};

#define XVTRS 12
struct Transverter {
    char        xvtr_name[20];
    uint8_t     xvtr_en;
    uint8_t     xvtr_num;
    uint16_t    RF;
    uint16_t    IF;
    float       pwr_set;
    float       offset;  
    uint8_t     decoder_pattern;
} xvtr[XVTRS] = {
    {"50",      OFF,  XVTR1,    50,   28, 0.50, 0.0, XVTR1},
    {"144",     OFF,  XVTR2,   144,   28, 0.50, 0.0, XVTR2},
    {"222",     OFF,  XVTR3,   222,   28, 0.50, 0.0, XVTR3},
    {"432",     OFF,  XVTR4,   432,   28, 0.50, 0.0, XVTR4},
    {"902",     OFF,  XVTR5,   902,   28, 0.50, 0.0, XVTR5},
    {"1296",    OFF,  XVTR6,  1296,   28, 0.50, 0.0, XVTR6},
    {"2304",    OFF,  XVTR7,  2304,  144, 0.50, 0.0, XVTR7},
    {"3456",    OFF,  XVTR8,  3456,  144, 0.50, 0.0, XVTR8},
    {"5760",    OFF,  XVTR9,  5760,  430, 0.50, 0.0, XVTR9},
    {"10368",   OFF, XVTR10, 10368,  144, 0.50, 0.0, XVTR10},
    {"24192",   OFF, XVTR11, 24182, 1296, 0.50, 0.0, XVTR11},
    {"47100",   OFF, XVTR12, 47100, 1296, 0.50, 0.0, XVTR12}
};

#define AGS_SET_NUM 4
struct AGC {
    char        agc_name[10];
    uint8_t     agc_maxGain;
    uint8_t     agc_response;
    uint8_t     agc_hardlimit;    ///autoVolumeControl(maxGain, response, hardLimit, threshold, attack, decay);
    float       agc_threshold;
    float       agc_attack;
    float       agc_decay;
} agc_set[AGS_SET_NUM] = {
    { "AGC OFF",2,0,0,-36.0,12.0,6.0},
    {"AGC SLOW",2,0,0,-36.0,12.0,6.0},
    { "AGC MED",2,0,0,-36.0,12.0,6.0},
    {"AGC FAST",2,0,0,-36.0,12.0,6.0}
};

#define USER_SETTINGS_NUM 3
struct User_Settings {
    char        configset_name[20]; // friendly anme for this record
    uint16_t    spectrum_preset;    // Sets the Spectrum module layout preset
    uint8_t     main_page;          // stores index to page settings table
    uint8_t     band_popup;         // index to band selection pop-up page layout
    uint8_t     usrcfgpage_1;       // index to user configuration page layout
    uint8_t     usrcfgpage_2;       // index to user configuration page layout
    uint8_t     usrcfgpage_3;       // index to user configuration page layout
    uint8_t     last_band;          // index into band memeory table to recall last settings  - this might get moved out later
} usr_set[USER_SETTINGS_NUM] = {
    {"User Config #1", 9, 0, 0, 0, 0, 0, BAND1},
    {"User Config #2", 1, 0, 0, 0, 0, 0, BAND2},
    {"User Config #3", 6, 0, 0, 0, 0, 0, BAND6}
};


//
//----------------------------------- Skip to Ham Bands only ---------------------------------
//
// Increment band up or down from present.   To be used with touch or physical band UP/DN buttons.
// A alternate method (not in this function) is to use a band button or gesture to do a pop up selection map.  
// A rotary encoder can cycle through the choices and push to select or just touch the desired band.
//


//  TODO   Revamp this to use the new band settings structures
void NextBand(void)
{
    float new_band_freq = 0;
    //float offset = 0;
    float band = 0;                              
                // non-direct band change commands, usually from sources like WSJT-X where a frequency is provided rather than specific band info.
    if (band < 1800000)
        new_band_freq = 1800000.0f;
    else if ( 1800000 < band && band < 3500000)
        new_band_freq = 1800000.0f;
    else if ( 3500000 < band && band < 5000000)
        new_band_freq = 3500000.0f;
    else if ( 5000000 < band && band < 7000000)
        new_band_freq = 5000000.0f;
    else if ( 7000000 < band && band < 10000000)
        new_band_freq = 7000000.0f;
    else if (10000000 < band && band < 14000000)
        new_band_freq = 10000000.0f;
    else if (14000000 < band && band < 18000000)
        new_band_freq = 14000000.0f;
    else if (18000000 < band && band < 21000000)
        new_band_freq = 18000000.0f;
    else if (21000000 < band && band < 24800000)
     new_band_freq = 21000000.0f;
    else if (24800000 < band && band < 28000000)
        new_band_freq = 24800000.0f;
    else if (28000000 < band && band < 32000000)
        new_band_freq = 28000000.0f;
    else if (band > 32000000)
        new_band_freq = 32000000;
    
    return;

}