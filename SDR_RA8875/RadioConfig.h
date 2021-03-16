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
#define VFO_A       1
#define VFO_B       0

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
    uint8_t     bandwidth;      // index to Bandwidth selection for this band.
    uint8_t     band_num;       // generally the same as the index but could be used to sort bands differently and skip bands
    uint16_t    tune_step;      // last step rate on this band.  Index to Tune step table  255 = ignore and use current
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
    uint8_t     nb_en;          // Noise Blanker mode.  0 is off.  1+ is mode  
} bandmem[BANDS] = { 
    // name    lower   upper    VFOA     VFOB  VActiv modeA modeB bw    band step  agc SPLIT RT RV XT XV ATU ANT PRESELECT   ATTEN     PREAMP  XVE XV# NBe 
    {"160M", 1800000, 2000000, 1840000, 1860000,VFO_A, LSB, LSB, BW2_8, BAND0,4,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, PRESEL1,ATTEN_OFF,PREAMP_OFF, 0,  0, 0},
    { "80M", 3500000, 4000000, 3573000, 3830000,VFO_A, LSB, LSB, BW3_2, BAND1,4,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, PRESEL2,ATTEN_OFF,PREAMP_OFF, 0,  1, 0},
    { "60M", 5000000, 5000000, 5000000, 5000000,VFO_B, USB, LSB, BW3_2, BAND2,4,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, PRESEL3,ATTEN_OFF,PREAMP_OFF, 0,  2, 0},
    { "40M", 7000000, 7300000, 7074000, 7200000,VFO_A,DATA, LSB, BW4_0, BAND3,3,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, PRESEL4,ATTEN_OFF,PREAMP_OFF, 0,  3, 0},
    { "30M",10000000,10200000,10136000,10136000,VFO_A,DATA, USB, BW3_2, BAND4,4,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, PRESEL5,ATTEN_OFF,PREAMP_OFF, 0,  4, 0},
    { "20M",14000000,14350000,14074000,14200000,VFO_A,DATA, USB, BW4_0, BAND5,4,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, PRESEL6,ATTEN_OFF,PREAMP_OFF, 0,  5, 0},
    { "17M",18000000,18150000,18135000,18100000,VFO_A, USB, USB, BW3_2, BAND6,4,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, PRESEL7,ATTEN_OFF,PREAMP_OFF, 0,  6, 0},
    { "15M",21000000,21450000,21074000,21350000,VFO_A,DATA, USB, BW4_0, BAND7,4,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, PRESEL8,ATTEN_OFF,PREAMP_OFF, 0,  7, 0},
    { "12M",24890000,25000000,24915000,24904000,VFO_A,  CW, USB, BW1_8, BAND8,4,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, PRESEL9,ATTEN_OFF,PREAMP_OFF, 0,  8, 0},
    { "10M",28000000,29600000,28074000,28074000,VFO_A,DATA, USB, BW3_2, BAND9,4,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1,PRESEL10,ATTEN_OFF,PREAMP_OFF, 0,  9, 0},
    {  "6M",50000000,54000000,50125000,50313000,VFO_A, USB, USB, BW3_2,BAND10,4,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1,PRESEL11,ATTEN_OFF,PREAMP_OFF, 0, 10, 0}
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
    {"BW 250Hz ",  250,   CW},
    {"BW 500Hz ",  500,   CW},
    {"BW 700Hz ",  700,   CW},
    {"BW 1.0KHz", 1000,   CW},
    {"BW 1.8KHz", 1800,   CW},
    {"BW 2.3KHz", 2300,  USB},
    {"BW 2.8KHz", 2800,  USB},
    {"BW 3.2KHz", 3200,  USB},
    {"BW 4.0KHz", 4000, DATA}
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
};

struct User_Settings user_settings[USER_SETTINGS_NUM] = {
    //Profile name  spect mn  pop uc1 uc2 uc3 lastB  mute  mic_En  micG  LIn LInVol SpkEn SpkVol LoEn LoVol enet
    {"User Config #1", 10, 0,  0,  0,  0,  0, BAND3,  OFF, MIC_OFF, 1.0,  ON,    14,   ON,   0.5,  ON,  12,    1},
    {"User Config #2",  1, 0,  0,  0,  0,  0, BAND2,  OFF, MIC_OFF, 1.0,  ON,    14,   ON,   0.5,  ON,  12,    0},
    {"User Config #3",  6, 0,  0,  0,  0,  0, BAND6,  OFF, MIC_OFF, 1.0,  ON,    14,   ON,   0.5,  ON,  12,    0}
};

struct Standard_Button {
    uint8_t enabled;        // enable or disable this button. UserInput() will look for mach corondinate and skip if disabled.
                            // Enable if currently showing button, disable if not. used to share screen coordinates with replacement buttons.
    uint8_t pressed;        // used to optionally track the pressed state of the button
    uint16_t bx;
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

#define STD_BTN_NUM 15      // number of buttons in the table
#define PANEL_ROWS  5       // 5-2 = Panel #.  0 is disable, 1 is not used, 2 3, and 4 values are Panel to display.
//  There are 6 100px wide buttons that can swap places, enabled/dispable by the function button for a row
//Anchor buttons normally stay put
#define MENU_BTN    0       // invoke a pop up (ON) for atten slider or MF knob adjustment
#define MUTE_BTN    1       // On off
#define FN_BTN      2       // Swaps out buttons in bottom row.  This stays for each row.
//Panel 1  These swap out
#define DISPLAY_BTN 3       // Invoke PopUp (On), close popup (Off)
#define RIT_BTN     4       // not implemented yet  
#define BANDDN_BTN  5       // this will change to a copmplex button soon
#define BANDUP_BTN  6       // this will change to a copmplex button soon
//Panel 2
#define ATTEN_BTN   7       // invoke a pop up (ON) for atten slider or MF knob adjustment
#define PREAMP_BTN  8       // On off
#define SPLIT_BTN   9       // ON/OFF
#define BAND_BTN    10      // Pops up Band Window.  Touch or MF can change bands
//Panel 3
#define ATU_BTN     11      // not implemented yet
#define NB_BTN      12      // not implemented yet
#define XVTR_BTN    13      // not implemented yet
#define VFO_AB_BTN  14      // ON/(A)/OFF(B)   Will go away soon.    

struct Standard_Button std_btn[STD_BTN_NUM] = {
  //  en  prsd   x   y    w    h   r   outline_color      txtcolor           on_color     off_color  padx pady    label
    { ON, OFF,   1, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 23, 20, "Menu\0"},
    { ON, OFF, 118, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 23, 20, "Mute\0"},
    {  2, OFF, 235, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK,RA8875_BLACK, 23, 20, "Fn 1\0"},
    //Panel 1
    { ON, OFF, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK,  9, 20, "Display\0"},
    { ON, OFF, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 30, 20, "RIT\0"},
    { ON, OFF, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 16, 20, "Band -\0"},
    { ON, OFF, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 16, 20, "Band +\0"},
    //Panel 2
    {OFF, OFF, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 21, 20, "Atten\0"},
    {OFF, OFF, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 31, 20, "Pre\0"},
    {OFF, OFF, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 23, 20, "Split\0"},    
    {OFF, OFF, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 23, 20, "Band\0"},   
    //Panel 3
    {OFF, OFF, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 21, 20, "ATU\0"},
    {OFF, OFF, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 31, 20, "NB\0"},
    {OFF, OFF, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 23, 20, "Xvtr\0"},
    {OFF, OFF, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 30, 20, "A/B\0"}    
};

struct Complex_Button {
    uint8_t enabled;        // enable or disable this button. UserInput() will look for mach corondinate and skip if disabled.
                            // Enable if currently showing button, disable if not. used to share screen coordinates with replacement buttons.
    uint8_t pressed;        // used to optionally track the pressed state of the button
	uint16_t bx;
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

#define COMPLEX_BTN_NUM 5       // number of buttons in the table

#define MODE_BTN    0       // index to button
#define FILTER_BTN  1       // will display the current FILTER value in the button
#define AGC_BTN     2       // will display the AGC value in the button
#define RATE_BTN    3       // will display the RATE (step) value in the button
#define NR_BTN      4       // will display the NR value in the button


// Complex button can update their labels to show a value or text.  For example AGC_S, changes to AGC_OFF
struct Complex_Button complex_btn[COMPLEX_BTN_NUM] = {
  // En  Prsd   x   y    w    h   r   outline_color      txtcolor           on_color     off_color  padx pady    label
    {ON, OFF,   1, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 21, 20, "Atten\0"},
    {ON, OFF, 118, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 31, 20, "Pre\0"},
    {ON, OFF, 235, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 23, 20, "Mute\0"},
    {ON, OFF, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 26, 20, "ATU\0"},
    {ON, OFF, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE, RA8875_BLACK, 23, 20, "AGC\0"}
};

struct Frequency_Display {
    uint16_t bx;        // X - upper left corner anchor point
	uint16_t by;        // Y - upper left corner anchor point
	uint16_t bw;        // width of whole box
	uint16_t bh;        // height of whole box
	uint16_t br;        // radius of corners
    uint16_t ol_clr;    // outline color
    uint16_t bg_clr;    // background color
    uint16_t vfoActive_clr;  // color of VFOA numbers
    const ILI9341_t3_font_t vfoActive_LabelFont;    // size of Active VFO Label text
    const ILI9341_t3_font_t vfoActive_Font;         // size of Active VFO numbers
    uint16_t vfoStandby_clr;                        // color of VFOB numbers
    const ILI9341_t3_font_t vfoStandby_LabelFont;   // size of Standby VFO label text
    const ILI9341_t3_font_t vfoStandby_Font;        // size of VFOB numbers
    uint16_t TX_clr;    // Color when active VFO is in transmit
	uint16_t padx;      // horizonal padding from left side of box
	uint16_t pady;      // vertical padding form top of box
    //uint32_t d_vfoa = ptr->d_vfoa;     // Text to display for the button label  Use spaces to center
    //uint32_t d_vfob = ptr->d_vfob;     // Text to display for the button label  Use spaces to center    
} disp_Freq[1] = {
    // x   y    w    h   r   outline_clr          bg_clr      vfoActive_clr     vfoAct_LblFnt  vActFnt     vfoStby_clr  vStbyLblFnt   vStbyFnt  TX_clr     padx  pady
    {400,  0, 240,  90,  0, RA8875_LIGHT_GREY, RA8875_BLACK,  RA8875_LIGHT_GREY,    Arial_20,  Arial_32, RA8875_MAGENTA,   Arial_16,  Arial_20, RA8875_RED,  0,   20}
};

 uint8_t display_state;   // something to hold the button state for the display pop-up window later.

//
//------------------------------------  Ethernet UDP messaging section --------------------------
//
#define ENET              // Include support for ethernet
#ifdef ENET
    #include <NativeEthernet.h>
    #include <NativeEthernetUdp.h>
    // Enter a MAC address and IP address for your controller below. MAC not required for Teensy cause we are using TeensyMAC function.
    // The IP address will be dependent on your local network:  don't need this since we can automatically figure ou tthe mac
    //byte mac[] = {
    //  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEC
    //};
    IPAddress ip(192, 168, 1, 237);    // Our static IP address.  Could use DHCP but preferring static address.
    unsigned int localPort = 7943;     // local port to LISTEN for the remote display/Desktop app

    //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    IPAddress remote_ip(192, 168, 1, 7);  // destination  IP (desktop app or remote display Arduino
    unsigned int remoteport = 7942;    // the destination port to SENDTO (a remote display or Desktop app)
#endif
//
//------------------------------------ End of Ethernet UDP messaging section --------------------------
//