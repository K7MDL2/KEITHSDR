//
//      RadioConfig.h
//
//  This file is the central warehouse for operational parameters.  
//  Key to this operation is are stuctures that hold the majority of current band settings
//  Other key parameters are current settings values which may be modified from the last used value
//  in the structure.  These are usually things like VFO dial settings and should be stored in 
//  EEPROM at some point like band changes when the dial has stopped for a certain length of time
//

#include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
#include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
#include <RA8875.h>           // internal Teensy library with ft5206 cap touch enabled in user_setting.h

#define myDARKGREY  31727u

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
#define VFO_A       1
#define VFO_B       0
#define NBOFF       0
#define NB1         1
#define NB2         2
#define NB3         3
#define NB4         4
#define NROFF       0
#define NR1         1
#define NR2         2
#define NR3         3
#define NR4         4
#define NTCHOFF     0
#define NTCH1       1
#define NTCH2       2



// Our Database of settings. This is the "factory default".  A user copy wil be stored in EEPROM with user changes
#define BANDS   11
struct Band_Memory {
    char        band_name[20];  // Freindly name or label.  Default here but can be changed by user.
    uint32_t    edge_lower;     // band edge limits for TX and for when to change to next band when tuning up or down.
    uint32_t    edge_upper;
    uint32_t    vfo_A_last;     // remember last VFO dial setting in this band
    uint32_t    vfo_B_last;
    uint8_t     VFO_AB_Active;  // Flag to track which has focus. Usesd in RX.  Used in TX for split
    uint8_t     mode_A;         // CW, LSB, USB, DATA.  255 = ignore and use current mode
    uint8_t     mode_B;         // CW, LSB, USB, DATA.  255 = ignore and use current mode
    uint8_t     filter;      // index to Bandwidth selection for this band.
    uint8_t     band_num;       // generally the same as the index but could be used to sort bands differently and skip bands
    uint8_t     tune_step;      // last step rate on this band.  Index to Tune step table  255 = ignore and use current
    uint8_t     agc_mode;       // index to group of AGC settings in another table
    uint8_t     split;          // split active or not. 255 is feature disabled
    uint8_t     RIT_en;         // RIT active. 
    int32_t     RIT;            // RIT value in Hz pos or neg from current VFO
    uint8_t     XIT_en;         // XIT active
    int32_t     XIT;            // XIT value in Hz pos or neg from current VFO
    uint8_t     ATU;            // enable ATU or not. 255 is feature disabled
    uint8_t     ant_sw;         // antenna selector switch. 255 is feature disabled
    uint8_t     preselector;    // preselector band set value. 255 is feature disabled
    uint8_t     attenuator;     // 0 = bypass, >0 is attenuation value to set. 255 is feature disabled
    uint8_t     preamp;         // 0-off, 1 is level 2, level 2, 255 is feature disabled
    uint8_t     xvtr_en;        // use Tranverter Table or not.  Want to be able to edit while disabled so this is sperate from index.
    uint8_t     xvtr_num;       // index to Transverter Table.   
} bandmem[BANDS] = { 
    // name    lower   upper    VFOA     VFOB  VActiv modeA modeB filt   band step  agc SPLIT RT RV XT XV ATU ANT PRESELECT   ATTEN     PREAMP  XVE XV# NBe 
    {"160M", 1800000, 2000000, 1840000, 1860000,VFO_A, LSB, LSB, BW2_8, BAND0,1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 0,   ATTEN_OFF,PREAMP_OFF, 0,  0},
    { "80M", 3500000, 4000000, 3573000, 3830000,VFO_A, LSB, LSB, BW3_2, BAND1,1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 1,    ATTEN_OFF,PREAMP_OFF, 0,  1},
    { "60M", 5350000, 5367000, 5000000, 5366000,VFO_B, USB, USB, BW3_2, BAND2,1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 2,    ATTEN_OFF,PREAMP_OFF, 0,  2},
    { "40M", 7000000, 7300000, 7074000, 7200000,VFO_A,DATA, LSB, BW4_0, BAND3,1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 3,    ATTEN_OFF,PREAMP_OFF, 0,  3},
    { "30M",10100000,10150000,10000000,10136000,VFO_A,DATA, USB, BW3_2, BAND4,1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 4,    ATTEN_OFF,PREAMP_OFF, 0,  4},
    { "20M",14000000,14350000,14074000,14200000,VFO_A,DATA, USB, BW4_0, BAND5,1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 5,    ATTEN_OFF,PREAMP_OFF, 0,  5},
    { "17M",18068000,18168000,18135000,18100000,VFO_A, USB, USB, BW3_2, BAND6,1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 6,    ATTEN_OFF,PREAMP_OFF, 0,  6},
    { "15M",21000000,21450000,21074000,21350000,VFO_A,DATA, USB, BW4_0, BAND7,1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 7,    ATTEN_OFF,PREAMP_OFF, 0,  7},
    { "12M",24890000,24990000,24915000,24904000,VFO_A,  CW, USB, BW1_8, BAND8,1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 8,    ATTEN_OFF,PREAMP_OFF, 0,  8},
    { "10M",28000000,29600000,28074000,28074000,VFO_A,DATA, USB, BW3_2, BAND9,1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 9,    ATTEN_OFF,PREAMP_OFF, 0,  9},
    {  "6M",50000000,54000000,50125000,50313000,VFO_A, USB, USB, BW3_2,BAND10,1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 10,ATTEN_OFF,PREAMP_OFF, 0, 10}
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
    {"AGC- ",2,0,0,-36.0,12.0,6.0},
    {"AGC-S",2,0,0,-36.0,12.0,6.0},
    {"AGC-M",2,0,0,-36.0,12.0,6.0},
    {"AGC-F",2,0,0,-36.0,12.0,6.0}
};

// per-band settings for common user adjsutments that are band dependent. The index is the band number.
struct Spectrum_Settings {
    uint16_t    Ref_level;      //Spectrum common adjustments due to noise level and scale/gain choices during operation.
    uint16_t    Span;
    float       scale;
};

#define FILTER 9
struct Filter_Settings {
    char        Filter_name[12];    // display name for UI
    uint16_t    Width;             //bandwidth in HZ
    char        units[4];
    uint8_t     pref_mode;      // preferred mode when enabled (future use)
} filter[FILTER] = {
    {"250",   250,  "Hz",   CW},
    {"500",   500,  "Hz",   CW},
    {"700",   700,  "Hz",   CW},
    {"1.00",  1000, "KHz",   CW},
    {"1.80",  1800, "KHz",   CW},
    {"2.30",  2300, "KHz",  USB},
    {"2.80",  2800, "KHz",  USB},
    {"3.20",  3200, "KHz",  USB},
    {"4.00",  4200, "KHz", DATA}
};

char Mode[4][5] = {"CW", "LSB", "USB", "DATA"};

#define TS_STEPS 5
struct TuneSteps {
    char        ts_name[12];    // display name for UI
    char        ts_units[4];    // units for display HZ or KHz
    uint16_t    step;             //bandwidth in HZ
    uint8_t     pref_mode;      // preferred mode when enabled (future use)
} tstep[TS_STEPS] = {
    {"1 ",   "Hz",     1,  CW},
    {"10 ",  "Hz",    10,  CW},
    {"100 ", "Hz",   100,  CW},
    {"1.0", "KHz",  1000,  CW},
    {"2.5", "KHz",  2500, USB},
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
    uint8_t     mute;               // Current status of Mute 
    uint8_t     mic_input_en;       // mic on or off
    float       mic_Gain_last;      // last used mic gain on this band
    uint8_t     lineIn_en;          // line in on or off. Range 0 to 15.  0 => 3.12Vp-p, 15 => 0.24Vp-p sensitivity
    uint8_t     lineIn_Vol_last;    // last used line in setting on this band. 255 is ignore and use current value
    uint8_t     spkr_en;            // 0 is disable or mute. 1= mono, 2= stereo. 3= sound effect 1 and so on. 255 is ignore and use current setting
    float       spkr_Vol_last;      // last setting for unmute or power on (When we store in EEPROM)
    uint8_t     lineOut_en;         // line out on or off. Range 13 to 31.  13 => 3.16Vp-p, 31=> 1.16Vp-p
    uint8_t     lineOut_Vol_last;   // last line out setting used on this band. 255 is ignore and use the current value.
    uint8_t     enet_enabled;       // Turn on ethernet features
    uint8_t     enet_output;        // Allow ethernet data to flow (if enet is enabled)
    uint8_t     nb_en;              // Noise Blanker mode.  0 is off.  1+ is mode
    uint8_t     nr_en;              // Noise Reduction.  0 is off.  1+ is mode
    uint8_t     spot;               // Spot  0 is off.  1+ is mode
    uint16_t    pitch;              // Pitch  0 is off.  1+ is mode
    uint8_t     notch;              // Notch mode.  0 is off.  1+ is mode
    uint8_t     xmit;               // xmit state.  0 is off.  1+ is mode
    uint8_t     fine;               // Fine tune state.  0 is off.  1+ is mode
    uint8_t     VFO_last;           // Track the last known state of the VFO A/B feature - either on A or B
};

struct User_Settings user_settings[USER_SETTINGS_NUM] = {
    //Profile name  spect mn  pop uc1 uc2 uc3 lastB  mute  mic_En  micG  LIn LInVol SpkEn SpkVol LoEn LoVol enet  enout  nben   nren  spot pitch   notch xmit fine VFO-AB
    {"User Config #1", 10, 0, OFF,  0,  0,  0, BAND3,  OFF, MIC_OFF, 1.0,  ON,    14,   ON,   0.5,  ON,  12,   ON,  OFF,   NB1,   NR2,  OFF,  600,   NTCH1, OFF, OFF,   0},
    {"User Config #2", 10, 0, OFF,  0,  0,  0, BAND2,  OFF, MIC_OFF, 1.0,  ON,    14,   ON,   0.5,  ON,  12,  OFF,  OFF,   NB2,   NR3,  OFF,  600, NTCHOFF, OFF, OFF,   0},
    {"User Config #3",  6, 0, OFF,  0,  0,  0, BAND6,  OFF, MIC_OFF, 1.0,  ON,    14,   ON,   0.5,  ON,  12,  OFF,  OFF, NBOFF, NROFF,  OFF,  600, NTCHOFF, OFF, OFF,   0}
};

struct Standard_Button {
    uint8_t  enabled;       // ON - enabled. Enable or disable this button. UserInput() will look for matched coordinate and skip if disabled.                            
    uint8_t  show;          // ON= Show key. 0 = Hide key. Used to Hide a button without disabling it. Useful for swapping panels of buttons.
    uint16_t bx;            // coordinates used by both touch and display systems
	uint16_t by;
	uint16_t bw;
	uint16_t bh;            // height 
	uint16_t br;            // radius of roundRect corners
    uint16_t outline_color; // used for button outline color
    uint16_t txtclr;        // used for button text color
    uint16_t on_color;      // fill color when button is ON state
    uint16_t off_color;     // fill color when button is OFF state
    uint16_t padx;       // # of pixels to pad label text horizontally shifting right to center the text 
    uint16_t pady;       // # of pixels to pad label text vertically shifting text down to center the text 
    char     label[20];     // Tesxt to display for the button label  Use spaces to center
};

#define STD_BTN_NUM 28      // number of buttons in the table
#define PANEL_ROWS  6       // 5-2 = Panel #.  0 is disable, 1 is not used, 2 3, and 4 values are Panel to display.
//  There are 6 100px wide buttons that can swap places, enabled/dispable by the function button for a row
//Anchor buttons normally stay put
//Panel 1  Fn is an anchor, the rest swap out
#define FN_BTN      0       // Swaps out buttons in bottom row.  This stays for each row.
#define MODE_BTN    1       // index to button
#define FILTER_BTN  2       // will display the current FILTER value in the button
#define ATTEN_BTN   3       // invoke a pop up (ON) for atten slider or MF knob adjustment
#define PREAMP_BTN  4       // On off
#define RATE_BTN    5       // will display the RATE (step) value in the button
#define BAND_BTN    6       // Pops up Band Window.  Touch or MF can change bands
//Panel 2  These swap out
#define NB_BTN      7       // not implemented yet
#define NR_BTN      8       // will display the NR value in the button
#define SPOT_BTN    9       // will display the NR value in the button
#define NOTCH_BTN   10      // will display the NR value in the button
#define AGC_BTN     11      // will display the AGC value in the button
#define MUTE_BTN    12      // On off
//Panel 3
#define MENU_BTN    13      // invoke a pop up (ON) for atten slider or MF knob adjustment
#define ANT_BTN     14      // Antenna switch
#define ATU_BTN     15      // not implemented yet
#define XMIT_BTN    16      // not implemented yet
#define BANDDN_BTN  17      // this will change to a copmplex button soon
#define BANDUP_BTN  18      // this will change to a copmplex button soon
//Panel 4
#define RIT_BTN     19      // not implemented yet 
#define XIT_BTN     20      // not implemented yet 
#define VFO_AB_BTN  21      // ON/(A)/OFF(B)   Will go away soon. 
#define FINE_BTN    22      // Fine and coarse rate
#define DISPLAY_BTN 23      // Invoke PopUp (On), close popup (Off)
#define SPLIT_BTN   24      // ON/OFF
//Panel Spare
#define ENET_BTN    25      // turn on and off the enet data output (does not enable/disable the enet hardware) 
#define XVTR_BTN    26      // not implemented yet
#define UTCTIME_BTN 27      // NTP UTC time when ethernet (and internet) is available 


struct Standard_Button std_btn[STD_BTN_NUM] = {
  //  en  show   x   y    w    h   r   outline_color      txtcolor           on_color     off_color  padx pady    label
    {  2,  ON,   1, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 25, 20, "Fn 1\0"},
    { ON,  ON, 118, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 20, 20, "Mode\0"},
    { ON,  ON, 235, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 21, 20, "Filter\0"},
    { ON,  ON, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 27, 20, "ATT\0"},
    { ON,  ON, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 24, 20, "PRE\0"},
    { ON,  ON, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 24, 20, "Rate\0"},
    { ON,  ON, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 22, 20, "Band\0"},
    //Panel 2
    { ON, OFF, 118, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 32, 20, "NB\0"},
    { ON, OFF, 235, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 32, 20, "NR\0"},
    { ON, OFF, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 14, 20, "RefLvl\0"},
    { ON, OFF, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 18, 20, "Notch\0"},
    { ON, OFF, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK,  8, 20, "AGC- \0"},
    { ON, OFF, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Mute\0"},
    //Panel 3
    { ON, OFF, 118, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 18, 20, "Menu\0"},
    { ON, OFF, 235, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 18, 20, "ANT  \0"},
    { ON, OFF, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 25, 20, "ATU\0"},
    { ON, OFF, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 22, 20, "XMIT\0"},
    { ON, OFF, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 12, 20, "Band -\0"},
    { ON, OFF, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 12, 20, "Band +\0"},
    //Panel 4
    { ON, OFF, 118, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 31, 20, "RIT\0"},
    { ON, OFF, 235, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 31, 20, "XIT\0"},
    { ON, OFF, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLUE,  31, 20, "A/B\0"},
    { ON, OFF, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 26, 20, "Fine\0"},
    { ON, OFF, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK,  9, 20, "Display\0"},
    { ON, OFF, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Split\0"},
    //Panel Spare
    {OFF, OFF, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Enet\0"},
    {OFF, OFF, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Xvtr\0"},
    { ON,  ON, 630,   1, 170, 36,  3, RA8875_BLACK,      RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 16, 10, "UTC:\0"}   
};


// Labels are screen display objects that are used mostly to show status.
// They may be used for touch events, usually in combination with a button.  Take NR for example
//    NR has a button and a screen label. The NR button is not always visible so a label is usually permanent
//    but is too small for relaible touch events.
// A button may be used as a label also but carries some extra baggage. The Clock button is a good example,
//    it supports touch events later to maybe pop up a menu of clock settings.
// The UserInput.h "Button handler" will scan all reported touch events x,y coordinates for a match to any touch enabled buttons and labels.
enum Label_List {BAND_LBL, MODE_LBL, FILTER_LBL, RATE_LBL, AGC_LBL, ANT_LBL, ATTEN_LBL, PREAMP_LBL, ATU_LBL, RIT_LBL, XIT_LBL, FINE_LBL, NB_LBL, NR_LBL, NOTCH_LBL, SPLIT_LBL, MUTE_LBL, XMIT_LBL, XVTR_LBL};

/*
#define BAND_LBL    0       // Band label (if used)
#define MODE_LBL    1       // index to button
#define FILTER_LBL  2       // will display the current FILTER value in the button
#define RATE_LBL    3       // will display the RATE (step) value in the button
#define AGC_LBL     4       // will display the AGC value in the button
#define ANT_LBL     5       // Antenna switch
#define ATTEN_LBL   6       // invoke a pop up (ON) for atten slider or MF knob adjustment
#define PREAMP_LBL  7       // On off
#define ATU_LBL     8       // not implemented yet
#define RIT_LBL     9       // not implemented yet 
#define XIT_LBL     10      // not implemented yet 
#define FINE_LBL    11      // Fine and coarse rate
#define NB_LBL      12      // not implemented yet
#define NR_LBL      13      // will display the NR value in the button
#define NOTCH_LBL   14      // will display the NR value in the button
#define SPLIT_LBL   15      // ON/OFF
#define MUTE_LBL    16      // On off
#define XMIT_LBL    17      // not implemented yet
#define XVTR_LBL    18      // not implemented yet
*/
#define LABEL_NUM 19      // number of labels in the table

struct Label {
    uint8_t  enabled;       // Not used for Labels today. ON - enabled. Enable or disable this Label. UserInput() will look for touchevent coordinate and skip if disabled.
    uint8_t  show;          // ON= Show key. 0 = Hide key. Used to Hide a label without disabling it.
    uint16_t x;             // coordinates used by both touch and display systems
	uint16_t y;
	uint16_t w;
	uint16_t h;             // height 
	uint16_t r;             // radius of roundRect corners (if used)
    uint16_t outline_color; // used for label outline color
    uint16_t on_txtclr;     // used for label text color
    uint16_t on_color;      // fill color when label is ON state
    uint16_t off_txtclr;    // used for label text color
    uint16_t off_color;     // fill color when label is OFF state
    uint16_t padx;          // # of pixels to pad label text horizontally shifting right to center the text 
    uint16_t pady;          // # of pixels to pad label text vertically shifting text down to center the text 
    char     label[20];     // Text to display for the label. Use padx and pady to center
} labels[LABEL_NUM] = {
  //  en  show   x   y    w    h   r   outline_color     on_txtclr      on_color    off_txtclr     off_color  padx pady  label 
    {OFF, OFF,   0,   0,  40, 30, 3, RA8875_BLACK, RA8875_BLUE,  RA8875_BLACK,   RA8875_CYAN,  RA8875_BLACK, 3, 7, "B:\0"},   
    { ON,  ON,  20, 105,  60, 30, 3, RA8875_BLACK, RA8875_YELLOW,RA8875_BLACK,   RA8875_YELLOW,RA8875_BLACK, 3, 7, "Mode\0"},
    { ON,  ON, 130, 105, 105, 30, 3, RA8875_BLACK, RA8875_CYAN,  RA8875_BLACK,   RA8875_CYAN,  RA8875_BLACK, 3, 7, "F:\0"},
    { ON,  ON, 280, 105,  96, 30, 3, RA8875_BLACK, RA8875_BLUE,  RA8875_BLACK,   RA8875_LIGHT_ORANGE, RA8875_BLACK, 3, 7, "R:\0"},
    { ON,  ON, 430, 105,  72, 30, 3, RA8875_BLACK, RA8875_LIGHT_ORANGE,  RA8875_BLACK,   RA8875_BLUE, RA8875_BLACK, 3, 7, "AGC-\0"},
    { ON,  ON, 560, 105,  60, 30, 3, RA8875_BLACK, RA8875_RED,   RA8875_BLACK,    RA8875_YELLOW,RA8875_BLACK, 3, 7, "ANT-\0"}, 
    { ON,  ON,  10,  15,  48, 22, 3, RA8875_BLACK, RA8875_CYAN,  RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "ATT\0"},
    { ON,  ON,  70,  15,  48, 22, 3, RA8875_BLACK, RA8875_BLACK, RA8875_BLUE,     myDARKGREY,  RA8875_BLACK, 3, 4, "PRE\0"},
    { ON,  ON, 130,  15,  48, 22, 3, RA8875_CYAN,  RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 6, 4, "ATU\0"},
    { ON,  ON,  10,  40,  48, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 5, 4, "RIT\0"},
    { ON,  ON,  70,  40,  48, 22, 3, RA8875_BLACK, RA8875_RED  , RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 8, 4, "XIT\0"},
    { ON,  ON, 130,  40,  60, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 5, 4, "Fine\0"},
    { ON,  ON,  10,  65,  48, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 9, 4, "NB\0"},
    { ON,  ON,  70,  65,  48, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 9, 4, "NR\0"},
    { ON,  ON, 130,  65,  60, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 2, 4, "Notch\0"},
    { ON,  ON, 220,  65,  90, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "Split\0"},
    {OFF, OFF, 699, 419, 100, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "Mute\0"},
    {OFF, OFF, 467, 419, 100, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "XMIT\0"},
    {OFF, OFF, 583, 419, 100, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "Xvtr\0"}
   

};

struct Frequency_Display {
    uint16_t bx;        // X - upper left corner anchor point
	uint16_t by;        // Y - upper left corner anchor point
	uint16_t bw;        // width of whole box
	uint16_t bh;        // height of whole box
	uint16_t br;        // radius of corners
    uint16_t bs;        // spacing between the VFO numerals
    uint16_t bm;        // marker spacing between VFO letter and digits  (A: 123.456.789)
    uint16_t ol_clr;    // VFO box outline color
    uint16_t box_clr;   // Color of outline around entire VFO region
    uint16_t bg_clr;    // background color
    uint16_t txt_clr;   // color of Active VFO numbers
    const ILI9341_t3_font_t txt_Font;    // size of Active VFO Label text
    uint16_t TX_clr;    // Color when active VFO is in transmit
	uint16_t padx;      // horizonal padding from left side of box
	uint16_t pady;      // vertical padding form top of box
} disp_Freq[4] = {
    // x   y    w    h    r   bs  bm   outline_clr     box_clr              bg_clr      txt_clr              txt_Fnt   TX_clr     padx  pady
    {204,  1, 380,  50,   3,  0,  0, RA8875_BLACK,      RA8875_LIGHT_GREY, RA8875_BLACK,  RA8875_LIGHT_GREY, Arial_40, RA8875_RED,   4,   4}, // VFO Active Digits
    {588,  6,  40,  40,   3,  0,  0, RA8875_LIGHT_GREY, RA8875_BLACK,      RA8875_BLACK,  RA8875_GREEN,      Arial_24, RA8875_RED,   9,   7}, // VFO Active Label
    {310, 53, 274,  40,   3,  0,  0, RA8875_BLACK,      RA8875_BLACK,      RA8875_BLACK,  myDARKGREY,        Arial_28, RA8875_RED,   6,   6}, // VFO Stby Digits
    {588, 53,  40,  40,   3,  0,  0, RA8875_LIGHT_GREY, RA8875_BLACK,      RA8875_BLACK,  myDARKGREY,        Arial_24, RA8875_RED,   9,   7}  // VFO Stby Label
};

uint8_t display_state;   // something to hold the button state for the display pop-up window later.

//
//------------------------------------  Ethernet UDP messaging section --------------------------
//
#define ENET              // Include support for ethernet
#ifdef ENET
    #include <NativeEthernet.h>
    #include <NativeEthernetUdp.h>
    
    //uint32_t    fstep       = 10;       // sets the tuning increment to 10Hz
    //uint8_t NTP_hour;  //NTP time 
    //uint8_t NTP_min;
    //uint8_t NTP_sec;
    time_t prevDisplay = 0; // when the digital clock was displayed
    //const int timeZone = 1;     // Central European Time
    const int timeZone = 0;     // UTC
    //const int timeZone = -5;  // Eastern Standard Time (USA)
    //const int timeZone = -4;  // Eastern Daylight Time (USA)
    //const int timeZone = -8;  // Pacific Standard Time (USA)
    //const int timeZone = -7;  // Pacific Daylight Time (USA)

    // Enter a MAC address and IP address for your controller below. MAC not required for Teensy cause we are using TeensyMAC function.
    // The IP address will be dependent on your local network:  don't need this since we can automatically figure ou tthe mac
    //byte mac[] = {
    //  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEC
    //};
    
    // Choose to use DHCP or a static IP address for the SDR
    #define USE_DHCP        
    #ifndef USE_DHCP
    // The IP Address is ignored if using DHCP
        IPAddress ip(192, 168, 1, 237);    // Our static IP address.  Could use DHCP but preferring static address.
    #endif
    unsigned int localPort = 7943;     // local port to LISTEN for the remote display/Desktop app

    //#define REMOTE_OPS  - conditionally defined in main header file for now
    #ifdef REMOTE_OPS
    // This is for later remote operaion usage
    //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    IPAddress remote_ip(192, 168, 1, 7);  // destination IP (desktop app or remote display Arduino)
    unsigned int remoteport = 7942;    // the destination port to SENDTO (a remote display or Desktop app)
    #endif 

    // This is for the NTP service to update the clock when connected to the internet
    unsigned int localPort_NTP = 8888;       // local port to listen for UDP packets
    const char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server
#endif
//
//------------------------------------ End of Ethernet UDP messaging section --------------------------
//