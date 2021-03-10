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
#define BAND0       0       // Band slot ID
#define BAND1       1       
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
#define BW0_25       0
#define BW0_5       1
#define BW0_7       2
#define BW1_0       3
#define BW1_8       4
#define BW2_3       5
#define BW2_8       6
#define BW3_2       7
#define BW4_0       8

// Our Database of settings. This is the "factory default".  A user copy wil be stored in EEPROM with user changes
#define BANDS   11
struct Band_Memory {
    char        band_name[20];  // Freindly name or label.  Default here but can be changed by user.
    uint32_t    edge_lower;     // band edge limits for TX and for when to change to next band when tuning up or down.
    uint32_t    edge_upper;
    uint32_t    vfo_A_last;     // remember last VFO dial setting in this band
    uint32_t    vfo_B_last;
    uint8_t     mode;           // CW, LSB, USB, DATA.  255 = ignore and use current mode
    uint8_t     bandwidth;      // index to Bandwidth selection for this band.
    uint8_t     band_num;       // generally the same as the index but could be used to sort bands differently and skip bands
    uint16_t    tune_step;      // last step rate on this band.  Index to Tune step table  255 = ignore and use current
    uint8_t     agc_mode;       // index to group of AGC settings in another table
    uint8_t     split;          // split active or not. 255 is feature disabled
    uint32_t    RIT;            // RIT active.  255 is feature disabled
    uint32_t    XIT;            // XIT active.  255 is feature disabled
    uint8_t     ATU;            // enable ATU or not. 255 is feature disabled
    uint8_t     ant_sw;         // antenna selector switch. 255 is feature disabled
    uint8_t     preselector;    // preselector band set value. 255 is feature disabled
    uint8_t     attenuator;     // 0 = bypass, >0 is attenuation value to set. 255 is feature disabled
    uint8_t     preamp;         // 0-off, 1 is level 2, level 2, 255 is feature disabled
    uint8_t     mic_input_en;   // mic on or off
    float       mic_Gain_last;  // last used mic gain on this band
    uint8_t     lineIn_en;      // line in on or off
    uint8_t     lineIn_Vol_last;// last used line in setting on this band. 255 is ignore and use current value
    uint8_t     spkr_en;        // 0 is disable or mute. 1= mono, 2= stereo. 3= sound effect 1 and so on. 255 is ignore and use current setting
    float       spkr_Vol_last;  // last setting for unmute or power on (When we store in EEPROM)
    uint8_t     lineOut_en;     // line out on or off
    uint8_t     lineOut_Vol_last;// last line out setting used on this band. 255 is ignore and use the current value.
    uint8_t     xvtr_en;        // use Tranverter Table or not.  Want to be able to edit while disabled so this is sperate from index.
    uint8_t     xvtr_num;       // index to Transverter Table.  
    uint8_t     spectrum_idx;   // index to spectrum settings table for per band settings like ref level and span
} bandmem[BANDS] = { 
    // name    lower   upper    VFOA     VFOB    mode bw     band step  agc SPLIT RIT XIT ATU ANT PRESELECT   ATTEN     PREAMP     MIC   MG  LI LG SP VOL LO LOG X XV SPC
    {"160M", 1800000, 2000000, 1840000, 1860000, LSB, BW2_8, BAND0,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL1,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.5,ON,20,OFF,0, 0 },
    { "80M", 3500000, 4000000, 3573000, 3830000, LSB, BW3_2, BAND1,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL2,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0, 1 },
    { "60M", 5000000, 5000000, 5000000, 5000000, USB, BW3_2, BAND2,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL3,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0, 2 },
    { "40M", 7000000, 7300000, 7074000, 7200000,DATA, BW4_0, BAND3,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL4,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0, 3 },
    { "30M",10000000,10200000,10136000,10136000,DATA, BW3_2, BAND4,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL5,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0, 4 },
    { "20M",14000000,14350000,14074000,14200000,DATA, BW4_0, BAND5,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL6,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0, 5 },
    { "17M",18000000,18150000,18135000,18100000, USB, BW3_2, BAND6,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL7,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0, 6 },
    { "15M",21000000,21450000,21074000,21350000,DATA, BW4_0, BAND7,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL8,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0, 7 },
    { "12M",24890000,25000000,24915000,24904000,  CW, BW1_8, BAND8,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1, PRESEL9,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0, 8 },
    { "10M",28000000,29600000,28074000,28074000,DATA, BW3_2, BAND9,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1,PRESEL10,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0, 9 },
    {  "6M",50000000,54000000,50125000,50313000, USB, BW3_2,BAND10,4,AGC_SLOW,OFF,OFF,OFF,OFF,ANT1,PRESEL11,ATTEN_OFF,PREAMP_OFF,MIC_OFF,1.0,ON,15,ON,0.7,ON,20,OFF,0,10 }
};

struct AudioSettings {

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

#define AGC_SET_NUM 4
struct AGC {
    char        agc_name[10];
    uint8_t     agc_maxGain;
    uint8_t     agc_response;
    uint8_t     agc_hardlimit;    ///autoVolumeControl(maxGain, response, hardLimit, threshold, attack, decay);
    float       agc_threshold;
    float       agc_attack;
    float       agc_decay;
} agc_set[AGC_SET_NUM] = {
    {"AGC-OFF",2,0,0,-36.0,12.0,6.0},
    {"AGC-S  ",2,0,0,-36.0,12.0,6.0},
    {"AGC-M  ",2,0,0,-36.0,12.0,6.0},
    {"AGC-F  ",2,0,0,-36.0,12.0,6.0}
};

#define USER_SETTINGS_NUM 3
struct User_Settings {
    char        configset_name[20]; // friendly anme for this record
    uint16_t    sp_preset;    // Sets the Spectrum module layout preset
    uint8_t     main_page;          // stores index to page settings table
    uint8_t     band_popup;         // index to band selection pop-up page layout preference
    uint8_t     usrcfgpage_1;       // index to user configuration page layout
    uint8_t     usrcfgpage_2;       // index to user configuration page layout
    uint8_t     usrcfgpage_3;       // index to user configuration page layout
    uint8_t     last_band;          // index into band memeory table to recall last settings  - this might get moved out later
} usr_set[USER_SETTINGS_NUM] = {
    {"User Config #1", 10, 0, 0, 0, 0, 0, BAND3},
    {"User Config #2", 1, 0, 0, 0, 0, 0, BAND2},
    {"User Config #3", 6, 0, 0, 0, 0, 0, BAND6}
};


// per-band settings for common user adjsutments that are band dependent. The index is the band number.
struct Spectrum_Settings {
    uint16_t    Ref_level;      //Spectrum common adjustments due to noise level and scale/gain choices during operation.
    uint16_t    Span;
    float       scale;
};

#define BWSTEPS 9
struct Bandwidth_Settings {
    char        bw_name[12];    // display name for UI
    uint16_t    bw;             //bandwidth in HZ
    uint8_t     pref_mode;      // preferred mode when enabled (future use)
} bw[BWSTEPS] = {
    {"BW 250Hz ", 250, CW},
    {"BW 500Hz ", 500, CW},
    {"BW 700Hz ", 700, CW},
    {"BW 1.0KHz", 500, CW},
    {"BW 1.8KHz", 500, CW},
    {"BW 2.3KHz", 500, USB},
    {"BW 2.8KHz", 500, USB},
    {"BW 3.2KHz", 500, USB},
    {"BW 4.0KHz",4000, DATA}
};

char Mode[4][5] = {"CW", "LSB", "USB", "DATA"};

#define TS_STEPS 8
struct TuneSteps {
    char        ts_name[12];    // display name for UI
    uint16_t    step;             //bandwidth in HZ
    uint8_t     pref_mode;      // preferred mode when enabled (future use)
} tstep[TS_STEPS] = {
    {"Ts 1Hz ",       1,  CW},
    {"Ts 10Hz ",     10,  CW},
    {"Ts 100Hz ",   100,  CW},
    {"Ts 250Hz ",   250,  CW},
    {"Ts 1.0KHz",  1000,  CW},
    {"Ts 2.5KHz",  2500, USB},
    {"Ts 5.0KHz",  5000, USB},
    {"Ts 10.0KHz",10000, USB}
};