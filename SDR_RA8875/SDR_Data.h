//
//  SDR_Data_RA8876.h
//
//
#ifndef _SDR_DATA_RA8876_H_
#define _SDR_DATA_RA8876_H_

#include "RadioConfig.h"
#include "SDR_RA8875.h"

struct Band_Memory bandmem[BANDS] = { 
    // name    lower   upper    VFOA     VFOB  VActiv modeA modeB filt   band  ts  agc  SPLIT RT RV XT XV ATU ANT BPF  ATTEN   att_DB  PREAMP    XVE XV#  SpRef
    {  "IF", 8200000, 8300000, 8215000, 8215000,VFO_A, USB, LSB, BW2_8, BAND0, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 0, ATTEN_OFF, 20, PREAMP_OFF,  0,  0,  8},
    {"160M", 1800000, 2000000, 1840000, 1860000,VFO_A,DATA, LSB, BW4_0, BAND1, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 0, ATTEN_OFF, 20, PREAMP_OFF,  0,  0,  8},
    { "80M", 3500000, 4000000, 3573000, 3868000,VFO_A,DATA, LSB, BW3_2, BAND2, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 1,  ATTEN_ON, 10, PREAMP_OFF,  0,  1,  5},
    { "60M", 4990000, 5367000, 5000000, 5366000,VFO_A, USB, USB, BW3_2, BAND3, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 2,  ATTEN_ON,  6, PREAMP_OFF,  0,  2,  8},
    #ifdef PANADAPTER
    { "40M", 8000000, 8500000, 8215000, 8215000,VFO_A, USB, USB, BW3_2, BAND4, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 3,  ATTEN_ON,  5,  PREAMP_ON,  0,  3,  8},
    #else 
    { "40M", 7000000, 7300000, 7074000, 7200000,VFO_A,DATA, LSB, BW4_0, BAND4, 3,AGC_SLOW, ON,OFF,0,OFF,0, ON,ANT2, 3,  ATTEN_ON,  2, PREAMP_OFF,  0,  3,  8},
    #endif
    { "30M", 9990000,10150000,10000000,10136000,VFO_A,DATA, USB, BW3_2, BAND5, 1,AGC_SLOW,OFF, ON,0,OFF,0,OFF,ANT1, 4,  ATTEN_ON,  1,  PREAMP_ON,  0,  4,  8},
    { "20M",14000000,14350000,14074000,14200000,VFO_A,DATA, USB, BW4_0, BAND6, 3,AGC_SLOW,OFF,OFF,0, ON,0,OFF,ANT2, 5,  ATTEN_ON,  7,  PREAMP_ON,  0,  5,  8},
    { "17M",18068000,18168000,18100000,18135000,VFO_B,DATA, USB, BW3_2, BAND7, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 6,  ATTEN_ON,  7,  PREAMP_ON,  0,  6,  8},
    { "15M",21100000,21450000,21074000,21350000,VFO_A,DATA, USB, BW4_0, BAND8, 3,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 7,  ATTEN_ON, 10,  PREAMP_ON,  0,  7,  8},
    { "12M",24890000,24990000,24915000,24904000,VFO_B, USB, USB, BW3_2, BAND9, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 8,  ATTEN_ON,  4,  PREAMP_ON,  0,  8,  8},
    { "10M",28000000,29600000,28074000,28200000,VFO_A,DATA, USB, BW4_0,BAND10, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 9,  ATTEN_ON,  2,  PREAMP_ON,  0,  9,  8},
    {  "6M",50000000,54000000,50125000,50313000,VFO_A, USB,DATA, BW3_2,BAND11, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 10, ATTEN_ON,  4,  PREAMP_ON,  0, 10,  8}
};

struct Transverter xvtr[XVTRS] = {
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

// Button Position variables for easy bulk size, place and move.
const uint16_t x_0 = 1; // reserved for Menu row select button (Fucntion button)
const uint16_t x_1 = 118;
const uint16_t x_2 = 235;
const uint16_t x_3 = 350;
const uint16_t x_4 = 467;
const uint16_t x_5 = 583;
const uint16_t x_6 = 699;
// rest of standard button dimensions
#ifdef USE_RA8875 
    const uint16_t y_1 = 419;
#else
    const uint16_t y_1 = 539;
#endif
const uint16_t w_1 = 100;
const uint16_t h_1 = 60;
const uint16_t r_1 = 20;

// Shared button placement for both RA8875 800x480 and RA8876 1024x600 displays
struct Standard_Button std_btn[STD_BTN_NUM] = {
  //  en  show  pnl   x   y    w    h    r   outline_color      txtcolor           on_color     off_color     padx pady  label
    {  2,  ON,   0, x_0, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 25, 20, "Fn 1\0"},
    { ON,  ON,   1, x_1, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 20, 20, "Mode\0"},
    { ON,  ON,   1, x_2, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 21, 20, "Filter\0"},
    { ON,  ON,   1, x_3, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 24, 20, "Rate\0"},
    { ON,  ON,   1, x_4, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  4, 20, "ATT\0"},
    { ON,  ON,   1, x_5, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  5, 20, "Preamp\0"},
    { ON,  ON,   1, x_6, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 20, 20, "Band\0"},
    //Panel 2
    { ON, OFF,   2, x_1, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 22, 20, "NB\0"},
    { ON, OFF,   2, x_2, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 32, 20, "NR\0"},
    { ON, OFF,   2, x_3, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 18, 20, "Notch\0"},
    { ON, OFF,   2, x_4, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK,  8, 20, "AGC- \0"},
    { ON, OFF,   2, x_5, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  7, 20, "Zoom\0"},
    { ON, OFF,   2, x_6, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  5, 20, "Pan\0"},
    //Panel 3
    { ON, OFF,   3, x_1, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 18, 20, "Menu\0"},
    { ON, OFF,   3, x_2, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 18, 20, "ANT  \0"},
    { ON, OFF,   3, x_3, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 25, 20, "ATU\0"},
    { ON, OFF,   3, x_4, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 22, 20, "XMIT\0"},
    { ON, OFF,   3, x_5, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 12, 20, "Band -\0"},
    { ON, OFF,   3, x_6, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 12, 20, "Band +\0"},
    //Panel 4
    { ON, OFF,   4, x_1, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 31, 20, "RIT\0"},
    { ON, OFF,   4, x_2, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 31, 20, "XIT\0"},   
    { ON, OFF,   4, x_3, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 26, 20, "Fine\0"},
    { ON, OFF,   4, x_4, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Split\0"},
    { ON, OFF,   4, x_5, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK,  9, 20, "Display\0"},
    { ON, OFF,   4, x_6, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLUE,  31, 20, "A/B\0"},
    //Panel 5
    { ON, OFF,   5, x_1, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Enet\0"},
    { ON, OFF,   5, x_2, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 30, 20, "Xvtr\0"},
    { ON, OFF,   5, x_3, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  9, 20, "RF:\0"},
    {OFF, OFF,   5, x_4, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 14, 20, "RefLvl\0"},
    { ON, OFF,   5, x_5, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  9, 20, "AF:\0"},
    { ON, OFF,   5, x_6, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Mute\0"},
    
#ifdef USE_RA8875   // These rows differ between display sizes. 
    //use outside of panel in upper right of screen.  Show wil be turned off when there is no clock time source to display
    { ON,  ON,   0, 630,   1, 170, 36,  3, RA8875_BLACK,      RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 16, 10, "UTC:\0"},  // For RA8875 4.3"
    { ON,  ON,   0, 645,  40, 140, 85,  8, RA8875_LIGHT_GREY, RA8875_BLUE,       RA8875_BLACK, RA8875_BLACK, 7,  10, ""},  // S/MF meter for RA8875 4.3"
    { ON, OFF,   0,   0, 200, 800,180, r_1, RA8875_BLACK,      RA8875_BLACK,      RA8875_BLACK, RA8875_BLACK,  9, 20, ""}  // Spectrum TouchTune area definition.
};
// Spare
//    { ON, OFF,   2, 467, y_1, w_1, h_1, r_1, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 23, 20, "Spot\0"},
#else
    
    { ON,  ON,   0, 865,   1, 170, 36,  3, RA8875_BLACK,      RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 16, 10, "UTC:\0"},  // for RA8876 7.0"
    { ON,  ON,   0, 880,  41, 140, 85,  8, RA8875_LIGHT_GREY, RA8875_BLUE,       RA8875_BLACK, RA8875_BLACK,  7, 10, ""},  // for RA8876 7.0" S/MF Meter hotspot
    { ON, OFF,   0,   0, 190,1023,320, 20, RA8875_BLACK,      RA8875_BLACK,      RA8875_BLACK, RA8875_BLACK,  9, 20, ""}  // Spectrum TouchTune hotspot area definition.
    // For the above TouchTune hotspot box set the top and bottom some margin away from the touch labels and touch buttons
};
#endif  // USE_RA8875

// Labels are screen display objects that are used mostly to show status.
// They may be used for touch events, usually in combination with a button.  Take NR for example
//    NR has a button and a screen label. The NR button is not always visible so a label is usually permanent
//    but is too small for reliable touch events.
// A button may be used as a label also but carries some extra baggage. The Clock button is a good example,
//    it supports touch events later to maybe pop up a menu of clock settings.
// The UserInput.h "Button handler" will scan all reported touch events x,y coordinates for a match to any touch enabled buttons and labels.

struct Label labels[LABEL_NUM] = {
  //  en  show   x   y    w    h   r   outline_color     on_txtclr      on_color    off_txtclr     off_color  padx pady  label 
    {OFF, OFF,   0,   0,  40, 29, 3, RA8875_BLACK, RA8875_BLUE,  RA8875_BLACK,   RA8875_CYAN,  RA8875_BLACK, 3, 7, "B:\0"},   
    {OFF,  ON,  20, 110,  90, 28, 3, RA8875_BLACK, RA8875_YELLOW,RA8875_BLACK,   RA8875_YELLOW,RA8875_BLACK, 3, 7, "Mode\0"}, //Set SHOW to ON if you want this label to be drawn on screen.
    {OFF,  ON, 130, 110, 105, 28, 3, RA8875_BLACK, RA8875_CYAN,  RA8875_BLACK,   RA8875_CYAN,  RA8875_BLACK, 3, 7, "F:\0"},
    {OFF,  ON, 280, 110,  96, 28, 3, RA8875_BLACK, RA8875_BLUE,  RA8875_BLACK, RA8875_LIGHT_ORANGE, RA8875_BLACK, 3, 7, "R:\0"},
    {OFF,  ON, 430, 110,  72, 28, 3, RA8875_BLACK, RA8875_LIGHT_ORANGE, RA8875_BLACK, RA8875_BLUE, RA8875_BLACK, 3, 7, "AGC-\0"},
    {OFF,  ON, 560, 110,  60, 28, 3, RA8875_BLACK, RA8875_RED,   RA8875_BLACK,   RA8875_YELLOW,RA8875_BLACK, 3, 7, "ANT-\0"}, 
    {OFF,  ON,  10,  15,  48, 22, 3, RA8875_BLACK, RA8875_CYAN,  RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "ATT\0"},
    {OFF,  ON,  70,  15,  48, 22, 3, RA8875_BLACK, RA8875_BLACK, RA8875_BLUE,     myDARKGREY,  RA8875_BLACK, 6, 4, "Pre\0"},
    {OFF,  ON, 130,  15,  48, 22, 3, RA8875_CYAN,  RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 6, 4, "ATU\0"},
    {OFF,  ON,  10,  40,  48, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 5, 4, "RIT\0"},
    {OFF,  ON,  70,  40,  48, 22, 3, RA8875_BLACK, RA8875_RED,   RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 8, 4, "XIT\0"},
    {OFF,  ON, 130,  40,  60, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 5, 4, "Fine\0"},
    {OFF,  ON,  10,  65,  48, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 8, 4, "NB\0"},
    {OFF,  ON,  70,  65,  48, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 9, 4, "NR\0"},
    {OFF,  ON, 130,  65,  60, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 2, 4, "Notch\0"},
    #ifdef USE_RA8875
    {OFF,  ON, 220,  65,  90, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "Split\0"},
    #else
    {OFF,  ON, 320,  65,  90, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "Split\0"},
    #endif
    {OFF, OFF, 699, 419, 100, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "Mute\0"}, // No label on screen for this today
    {OFF, OFF, 467, 419, 100, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "XMIT\0"}, // No label on screen for this today
    {OFF, OFF, 583, 419, 100, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "Xvtr\0"}, // No label on screen for this today
    {OFF, OFF, 699, 419, 100, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "RefLvl\0"},  // No label on screen for this today
    {OFF, OFF, 583, 419, 100, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 2, 4, "Zoom\0"}, // No label on screen for this today
    {OFF, OFF, 699, 419, 100, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "Pan\0"} // No label on screen for this today
};

struct User_Settings user_settings[USER_SETTINGS_NUM] = {                      
//Profile name    sp_preset mn  pop uc1 uc2 uc3 lastB  mute  mic_En  micG LInLvl rfg_en rfGain SpkEn afgen afGain LoRX LoTX enet  enout  nben   nblvl  nren  spot  rbeep pitch   notch  xmit fine VFO-AB  DefMFknob    enc1         enc2         enc3      Zoom_lvl panEn panlvl
    {"ENET ON Config",    0, 0, OFF,  0,  0,  0, BAND2,  OFF, MIC_ON,  30.0,  15,   OFF,    80,   ON,   OFF,   0,  15,  13,   ON,  OFF,  OFF,  NBOFF,  OFF,  OFF,  0.02,  600, NTCHOFF, OFF, OFF,   0,    MFTUNE,     MFTUNE,      AFGAIN_BTN,  ATTEN_BTN,   ZOOMx1, OFF, 50}, // if no encoder is present assign it to 0 and it will be skipped. 
    {"User Config #2",    0, 0, OFF,  0,  0,  0, BAND4,  OFF, MIC_ON,  30.0,  15,   OFF,    80,   ON,   OFF,   0,  15,  13,  OFF,  OFF,  OFF,  NBOFF,  OFF,  OFF,  0.02,  600, NTCHOFF, OFF, OFF,   0,    MFTUNE,     MFTUNE,      AFGAIN_BTN,  RFGAIN_BTN,  ZOOMx1, OFF, 50},
    {"PanAdapter Config", 0, 0, OFF,  0,  0,  0, BAND0,  OFF, MIC_OFF, 30.0,  15,   OFF,    80,   ON,   OFF,   0,  15,  13,  OFF,  OFF,  OFF,  NBOFF,  OFF,  OFF,  0.02,  600, NTCHOFF, OFF, OFF,   0,    MFTUNE,     REFLVL_BTN,  REFLVL_BTN,  RFGAIN_BTN,  ZOOMx1, OFF, 50}
};

struct Frequency_Display disp_Freq[FREQ_DISP_NUM] = {
    // x   y    w    h    r   bs  bm   outline_clr     box_clr              bg_clr      txt_clr              txt_Fnt   TX_clr     padx  pady
    #ifdef USE_RA8875
    {204,  1, 380,  50,   3,  0,  0, RA8875_BLACK,      RA8875_LIGHT_GREY, RA8875_BLACK,  RA8875_LIGHT_GREY, Arial_40, RA8875_RED,   4,   4}, // VFO Active Digits
    {588,  6,  40,  40,   3,  0,  0, RA8875_LIGHT_GREY, RA8875_BLACK,      RA8875_BLACK,  RA8875_GREEN,      Arial_24, RA8875_RED,   9,   7}, // VFO Active Label
    {310, 53, 274,  40,   3,  0,  0, RA8875_BLACK,      RA8875_BLACK,      RA8875_BLACK,  myDARKGREY,        Arial_28, RA8875_RED,   6,   6}, // VFO Stby Digits
    {588, 53,  40,  40,   3,  0,  0, RA8875_LIGHT_GREY, RA8875_BLACK,      RA8875_BLACK,  myDARKGREY,        Arial_24, RA8875_RED,   9,   7}  // VFO Stby Label
    #else
    {304,  1, 380,  50,   3,  0,  0, RA8875_BLACK,      RA8875_LIGHT_GREY, RA8875_BLACK,  RA8875_LIGHT_GREY, Arial_40, RA8875_RED,   4,   4}, // VFO Active Digits
    {688,  6,  40,  40,   3,  0,  0, RA8875_LIGHT_GREY, RA8875_BLACK,      RA8875_BLACK,  RA8875_GREEN,      Arial_24, RA8875_RED,   9,   7}, // VFO Active Label
    {410, 53, 274,  40,   3,  0,  0, RA8875_BLACK,      RA8875_BLACK,      RA8875_BLACK,  myDARKGREY,        Arial_28, RA8875_RED,   6,   6}, // VFO Stby Digits
    {688, 53,  40,  40,   3,  0,  0, RA8875_LIGHT_GREY, RA8875_BLACK,      RA8875_BLACK,  myDARKGREY,        Arial_24, RA8875_RED,   9,   7}  // VFO Stby Label
    #endif  // USE_RA8875
};

// (   name,  maxGain, response, hardLimit, threshold,   attack,  decay);
// ( "AGC-x", 0-2,     0-3,      0-1,       0 to -96.0,  x.0,     x.0)
// gain 0(0dB), 1(6.0dB), 2(12dB) max gain applied for expanding
// response 0(0ms), 1(25ms), 2(50ms), 3(100ms) integration time for compressor - larger allows short term peaks to pass through
// hardlimit 0 = softknee compressor - progressively compresses signals louder than threshold
// hardlimit 1 = hard compressor used - all values above threshold are same loudness
// threshold float in range of 0.0 to -96.0dBFS where -18.0dBFS is typical
// attack float controls rate of decrease in gain when signal is over threshold - in dB/s
// decay how fast gain is restored once level drops below threshold in dB/s - typically set longer than attack value
PROGMEM struct AGC agc_set[AGC_SET_NUM] = {
    //2,1,0,-5,0.5,0.5 from example file
    {"AGC- ", 0, 0, 0,  0, 0.0f,  0.0f},  
    {"AGC-S", 1, 1, 0, -5, 0.2f,  0.1f},
    {"AGC-M", 1, 0, 0, -10, 0.4f,  0.3f},
    {"AGC-F", 1, 0, 0, -16, 0.8f,  0.6f}
};

// Settings ranges.5 and 20, closer to 3 maybe best
// nAnticipation is 1 to 125
// Decay is 1 to 10
PROGMEM struct NB nb[NB_SET_NUM] = {
    {"-0",   1.0,   1,  1},  // use "" to leave the number blank like AGC, suggesting it is off.
    {"-1",   2.0,   5,  2},  // values suggested for test in the source code   
    {"-2",   4.0,   5,  8},  
    {"-3",   7.0,   2,  4},
    {"-4",  15.0,  40,  8},    
    {"-5",  20.0,  60, 10},  
    {"-6",  80.0, 100,  4}
};

// Zoom Level table
PROGMEM struct Zoom_Lvl zoom[ZOOM_NUM] = {
    {":1",   1},  // x1 1024 FFT
    {":2",   2},  // x2 2048 FFT
    {":4",   4}   // x4 4096 FFT
};

PROGMEM struct Filter_Settings filter[FILTER] = {
    {"250",   250,  "Hz",    CW},
    {"500",   500,  "Hz",    CW},
    {"700",   700,  "Hz",    CW},
    {"1.00",  1000, "KHz",   CW},
    {"1.80",  1800, "KHz",  USB},
    {"2.30",  2300, "KHz",  USB},
    {"2.80",  2800, "KHz",  USB},
    {"3.20",  3200, "KHz",  USB},
    {"4.00",  4000, "KHz", DATA}
};

PROGMEM struct TuneSteps  tstep[TS_STEPS] = {
    {"1",   "Hz",      1, CW},
    {"10",  "Hz",     10, USB},
    {"100", "Hz",    100, USB},
    {"1.0", "KHz",  1000, USB},
    {"2.5", "KHz",  2500, USB},
    {"5.0", "KHz",  5000, USB}
};

//Remember filter per mode  Last field "Width" is writable
struct Modes_List modeList[MODES_NUM] = {
    {0, "CW    ", BW0_7},
    {1, "CW-R  ", BW0_7},
    {2, "USB   ", BW2_8},
    {3, "LSB   ", BW2_8},
    {4, "DATA  ", BW3_2},
    {5, "DATA-R", BW3_2},
    {6, "AM    ", BW4_0},
    {7, "FM    ", BW4_0}
 };

// Use the generator function to create 1 set of data to define preset values for window size and placement.  
// Just copy and paste from the serial terminal into each record row.
#define PRESETS 12  // number of parameter records with our preset spectrum window values
struct Spectrum_Parms Sp_Parms_Def[PRESETS] = { // define default sets of spectrum window parameters, mostly for easy testing but could be used for future custom preset layout options
        //W        LE  RE CG                                         x   y   w  h  c sp st clr sc mode      scal reflvl wfrate
    #ifdef USE_RA8875
        {798,0, 0,  0,798,398,14,8,157,179,179,408,400,110,111,289,289,  0,153,799,256,50,20,6,240,1.0,0.9,1,20, 8, 0},
        {500,2,49,150,650,400,14,8,133,155,155,478,470, 94,221,249,249,130,129,540,350,30,25,2,550,1.0,0.9,1,30, 8, 90}, // hal
        {796,2, 2,  2,798,400,14,8,143,165,165,408,400, 94,141,259,259,  0,139,800,270,40,20,2,310,1.0,0.9,1,40, 5, 90},
        {500,2,49,150,650,400,14,8,133,155,155,478,470, 94,221,249,249,130,129,540,350,30,25,2,550,1.0,0.9,1,30, 8, 70}, // hal
    #else
        {1020,1,1,  1,1021,510,14,8,143,165,165,528,520,142,213,307,307,  0,139,1022,390,40,20,6,890,1.5,0.9,1,20,10, 80},
        { 508,1,1,  1, 509,254,14,8,214,236,236,528,520,113,171,349,349,  0,210, 510,319,40,20,2,310,1.0,0.9,0,40, 8,100},
        { 508,1,1,513,1021,766,14,8,214,236,236,528,520,113,171,349,349,512,210, 510,319,40,20,2,310,1.0,0.9,1,40, 8,100},
        { 298,1,1,601, 899,749,14,8,304,326,326,499,491, 99, 66,425,425,600,300, 300,200,60,20,2,310,1.0,0.9,0,40, 6,100},
    #endif        
        {512,2,43,143,655,399,14,8,354,376,376,479,471, 57, 38,433,433,100,350,599,130,60,25,2,340,1.7,0.9,0,60, 8, 80},  // Small wide bottom screen area to fit under pop up wndows.
        {498,1, 1,  1,499,249,14,8,143,165,165,408,400, 94,141,259,259,  0,139,500,270,40,20,2,310,1.0,0.9,0,40, 6,100},    //smaller centered
        {198,1, 1,551,749,649,14,8,183,205,205,408,400,136, 59,341,341,550,179,200,230,70,20,2,310,1.0,0.9,1,40, 0,100},  // low wide high gain
        {500,2, 2,150,650,400,14,8,133,155,155,418,410,102,153,257,257,130,129,540,290,40,25,2,320,1.0,0.9,1,30, 8, 75},     //60-100 good
        {512,2,43,143,655,399,14,8,223,245,245,348,340, 57, 38,302,302,100,219,599,130,60,25,2,310,1.7,0.9,0,60, 8,100},
        {396,2, 2,102,498,300,14,8,243,265,265,438,430, 99, 66,364,364,100,239,400,200,60,25,2,310,1.7,0.9,0,40, 8,100},
        {512,2,43,143,655,399,14,8,183,205,205,478,470,106,159,311,311,100,179,599,300,40,25,2,450,0.7,0.9,1,40, 8, 40},
        {796,2, 2,  2,798,400,14,8,183,205,205,478,470,106,159,311,311,  0,179,800,300,40,25,5,440,1.0,0.9,0,40, 8, 30}
};

// To create new layout records for the above table the below paramaters.
// The library utility fucntion Spectrum_Parm_Generator() is called at startup and writes out a complete layout record to serial debug screen
// This structure is defined in the Spectrum_RA887x library .h file.
// Cut and paste the debug data line into the table above.  Adjust the PRESETS value as needed.
// The user_profile table specifies which layout to actually use from the table above.
// This data only generates the data row for cut and paste.  Some day it could be real time. 
// This is primarily an aid to fit new screen sizes if not already defined above.

struct New_Spectrum_Layout Custom_Layout[1] = {      // Temp storage for generating new layouts    
    0,        // spectrum_x >  0 to width of display - window width. Must fit within the button frame edges left and right
                    // ->Pay attention to the fact that position X starts with 0 so 100 pixels wide makes the right side value of x=99.
    153,      // spectrum_y0 to vertical height of display - height of your window. Odd Numbers are best if needed to make the height an even number and still fit on the screen
    256,      // spectrum_height Total height of the window. Even numbers are best. (height + Y) cannot exceed height of the display or the window will be off screen.
    50,       // spectrum_center Value 0 to 100.  Smaller value = biggger waterfall. Specifies the relative size (%) between the spectrum and waterfall areas by moving the dividing line up or down as a percentage
                    // Smaller value makes spectrum smaller, waterfall bigger
    799,      // spectrum_width Total width of window. Even numbers are best. 552 is minimum to fit a full 512 pixel graph plus the min 20 pixel border used on each side. Can be smaller but will reduce graph area
    20,       // spectrum_span Value in KHz.  Ths will be the maximum span shown in the display graphs.  
                    // The graph code knows how many Hz per bin so will scale down to magnify a smaller range.
                    // Max value and resolutoin (pixels per bin) is dependent on sample frequency
                    // 25000 is max for 1024 FFT with 500 bins at 1:1 bins per pixel
                    // 12500 would result in 2 pixels per bin. Bad numbers here should be corrected to best fit by the function
    2,        // spectrum_wf_style Range 1- 6. Specifies the Waterfall style.
    330,      // spectrum_wf_colortemp Range 1 - 1023. Specifies the waterfall color temperature to tune it to your liking
    1.0f,      // spectrum_wf_scal e0.0f to 40.0f. Specifies thew waterfall zoom level - may be redundant when Span is worked out later.
    0.9f,      // spectrum_LPFcoeff 1.0f to 0.0f. Data smoothing
    1,        // spectrum_dot_bar_mode 0=bar, 1=Line. Spectrum box
    40,       // spectrum_sp_scale 10 to 80. Spectrum scale factor in dB. This is the height of the scale (if possible by windows sizes). Will plot the spectrum window of values between the floor and the scale value creating a zoom effect.
    -175,     // spectrum_floor 0 to -150. The reference point for plotting values.  Anything signal value > than this (less negative) will be plotted until stronger than the window height*scale factor.
    70        // spectrum_wf_rate window update rate in ms.  25 is fast enough to see dit and dahs well    
};

#endif //  _SDR_DATA_RA8876_H_ 
