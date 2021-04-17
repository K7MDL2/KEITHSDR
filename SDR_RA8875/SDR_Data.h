//
//  SDR_Data_RA8876.h
//
//

#include "SDR_RA8875.h"

#ifndef _SDR_DATA_RA8876_H_
#define _SDR_DATA_RA8876_H_
#include "RadioConfig.h"

struct Band_Memory bandmem[BANDS] = { 
    // name    lower   upper    VFOA     VFOB  VActiv modeA modeB filt   band  ts  agc  SPLIT RT RV XT XV ATU ANT BPF  ATTEN   att_DB  PREAMP    XVE XV#  SpRef
    {  "IF", 8200000, 8300000, 8215000, 8215000,VFO_A, USB, LSB, BW2_8, BAND0, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 0, ATTEN_OFF, 20, PREAMP_OFF,  0,  0,  8},
    {"160M", 1800000, 2000000, 1840000, 1860000,VFO_A, LSB, LSB, BW2_8, BAND1, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 0, ATTEN_OFF, 20, PREAMP_OFF,  0,  0,  8},
    { "80M", 3500000, 4000000, 3573000, 3830000,VFO_A,DATA, LSB, BW3_2, BAND2, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 1, ATTEN_OFF, 10, PREAMP_OFF,  0,  1,  5},
    { "60M", 4990000, 5367000, 5000000, 5366000,VFO_A, USB, USB, BW3_2, BAND3, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 2,  ATTEN_ON,  6, PREAMP_OFF,  0,  2,  8},
    #ifdef PANADAPTER
    { "40M", 8000000, 8500000, 8215000, 8215000,VFO_A, USB, USB, BW3_2, BAND4, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 3,  ATTEN_ON,  5,  PREAMP_ON,  0,  3,  8},
    #else 
    { "40M", 7000000, 7300000, 7074000, 8215000,VFO_A,DATA, LSB, BW4_0, BAND4, 1,AGC_FAST, ON,OFF,0,OFF,0, ON,ANT2, 3,  ATTEN_ON,  2, PREAMP_OFF,  0,  3,  8},
    #endif
    { "30M", 9990000,10150000,10000000,10136000,VFO_A,DATA, USB, BW3_2, BAND5, 1,AGC_SLOW,OFF, ON,0,OFF,0,OFF,ANT1, 4,  ATTEN_ON,  1,  PREAMP_ON,  0,  4,  8},
    { "20M",14000000,14350000,14074000,14200000,VFO_A,DATA, USB, BW4_0, BAND6, 1,AGC_SLOW,OFF,OFF,0, ON,0,OFF,ANT2, 5,  ATTEN_ON,  7,  PREAMP_ON,  0,  5,  8},
    { "17M",18068000,18168000,18135000,18100000,VFO_B,DATA, USB, BW3_2, BAND7, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 6,  ATTEN_ON,  7,  PREAMP_ON,  0,  6,  8},
    { "15M",21100000,21450000,21074000,21350000,VFO_A,DATA, USB, BW4_0, BAND8, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 7,  ATTEN_ON, 10,  PREAMP_ON,  0,  7,  8},
    { "12M",24890000,24990000,24915000,24904000,VFO_B, USB,  CW, BW0_7, BAND9, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 8,  ATTEN_ON,  4,  PREAMP_ON,  0,  8,  8},
    { "10M",28000000,29600000,28074000,28074000,VFO_A,DATA, USB, BW3_2,BAND10, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 9,  ATTEN_ON,  2,  PREAMP_ON,  0,  9,  8},
    {  "6M",50000000,54000000,50125000,50313000,VFO_A, USB,DATA, BW3_2,BAND11, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 10, ATTEN_ON,  4,  PREAMP_ON,  0, 10,  8}
};

PROGMEM struct Transverter xvtr[XVTRS] = {
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

#ifdef USE_RA8875

struct Standard_Button std_btn[STD_BTN_NUM] = {
  //  en  show  pnl   x   y    w    h   r   outline_color      txtcolor           on_color     off_color  padx pady    label
    {  2,  ON,   0,   1, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 25, 20, "Fn 1\0"},
    { ON,  ON,   1, 118, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 20, 20, "Mode\0"},
    { ON,  ON,   1, 235, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 21, 20, "Filter\0"},
    { ON,  ON,   1, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  6, 20, "Atten\0"},
    { ON,  ON,   1, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  5, 20, "Preamp\0"},
    { ON,  ON,   1, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 24, 20, "Rate\0"},
    { ON,  ON,   1, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 20, 20, "Band\0"},
    //Panel 2
    { ON, OFF,   2, 118, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 22, 20, "NB\0"},
    { ON, OFF,   2, 235, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 32, 20, "NR\0"},
    { ON, OFF,   2, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 23, 20, "Spot\0"},
    { ON, OFF,   2, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 18, 20, "Notch\0"},
    { ON, OFF,   2, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK,  8, 20, "AGC- \0"},
    { ON, OFF,   2, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Mute\0"},
    //Panel 3
    { ON, OFF,   3, 118, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 18, 20, "Menu\0"},
    { ON, OFF,   3, 235, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 18, 20, "ANT  \0"},
    { ON, OFF,   3, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 25, 20, "ATU\0"},
    { ON, OFF,   3, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 22, 20, "XMIT\0"},
    { ON, OFF,   3, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 12, 20, "Band -\0"},
    { ON, OFF,   3, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 12, 20, "Band +\0"},
    //Panel 4
    { ON, OFF,   4, 118, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 31, 20, "RIT\0"},
    { ON, OFF,   4, 235, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 31, 20, "XIT\0"},   
    { ON, OFF,   4, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 26, 20, "Fine\0"},
    { ON, OFF,   4, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Split\0"},
    { ON, OFF, 200, 583, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK,  9, 20, "Display\0"},//  One off cheat to allow the Display button to show in 2 rows, 4 and 5.
    { ON, OFF,   4, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLUE,  31, 20, "A/B\0"},
    //Panel 5
    { ON, OFF,   5, 118, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Enet\0"},
    { ON, OFF,   5, 235, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 30, 20, "Xvtr\0"},
    { ON, OFF,   5, 350, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  9, 20, "RF:\0"},
    {OFF, OFF,   5, 467, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 14, 20, "RefLvl\0"},
    // Placeholder for display key to eb active here also
    { ON, OFF,   5, 699, 419, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  9, 20, "AF:\0"},
    //use outside of panel in upper right of screen.  Show wil be turned off when there is no clock time source to display
    { ON,  ON,   0, 630,   1, 170, 36,  3, RA8875_BLACK,      RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 16, 10, "UTC:\0"},  // For RA8875 4.3"
    { ON,  ON,   0, 645,  40, 140, 85,  8, RA8875_LIGHT_GREY, RA8875_BLUE,       RA8875_BLACK, RA8875_BLACK, 7,  10, ""},  // S/MF meter for RA8875 4.3"
    { ON, OFF,   0,   0, 200, 800,180, 20, RA8875_BLACK,      RA8875_BLACK,      RA8875_BLACK, RA8875_BLACK,  9, 20, ""}  // Spectrum TouchTune area definition.
};

#else

struct Standard_Button std_btn[STD_BTN_NUM] = {
  //  en  show  pnl   x   y    w    h   r   outline_color      txtcolor           on_color     off_color  padx pady    label
    {  2,  ON,   0,   1, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 25, 20, "Fn 1\0"},
    { ON,  ON,   1, 118, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 20, 20, "Mode\0"},
    { ON,  ON,   1, 235, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 21, 20, "Filter\0"},
    { ON,  ON,   1, 467, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  6, 20, "Atten\0"},
    { ON,  ON,   1, 583, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  5, 20, "Preamp\0"},
    { ON,  ON,   1, 350, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 24, 20, "Rate\0"},
    { ON,  ON,   1, 699, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 20, 20, "Band\0"},
    //Panel 2
    { ON, OFF,   2, 118, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 22, 20, "NB\0"},
    { ON, OFF,   2, 235, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 32, 20, "NR\0"},
    { ON, OFF,   2, 467, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 23, 20, "Spot\0"},
    { ON, OFF,   2, 350, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 18, 20, "Notch\0"},
    { ON, OFF,   2, 583, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK,  8, 20, "AGC- \0"},
    { ON, OFF,   2, 699, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Mute\0"},
    //Panel 3
    { ON, OFF,   3, 118, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 18, 20, "Menu\0"},
    { ON, OFF,   3, 235, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 18, 20, "ANT  \0"},
    { ON, OFF,   3, 350, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 25, 20, "ATU\0"},
    { ON, OFF,   3, 467, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 22, 20, "XMIT\0"},
    { ON, OFF,   3, 583, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 12, 20, "Band -\0"},
    { ON, OFF,   3, 699, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 12, 20, "Band +\0"},
    //Panel 4
    { ON, OFF,   4, 118, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 31, 20, "RIT\0"},
    { ON, OFF,   4, 235, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 31, 20, "XIT\0"},   
    { ON, OFF,   4, 350, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 26, 20, "Fine\0"},
    { ON, OFF,   4, 467, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Split\0"},
    { ON, OFF, 200, 583, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK,  9, 20, "Display\0"},//  One off cheat to allow the Display button to show in 2 rows, 4 and 5.
    { ON, OFF,   4, 699, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLUE,  31, 20, "A/B\0"},
    //Panel 5
    { ON, OFF,   5, 118, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 23, 20, "Enet\0"},
    { ON, OFF,   5, 235, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 30, 20, "Xvtr\0"},
    { ON, OFF,   5, 350, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  9, 20, "RF:\0"},
    {OFF, OFF,   5, 467, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK, 14, 20, "RefLvl\0"},
    // Placeholder for display key to eb active here also
    { ON, OFF,   5, 699, 539, 100, 60, 20, RA8875_LIGHT_GREY, RA8875_LIGHT_GREY, RA8875_BLUE,  RA8875_BLACK,  9, 20, "AF:\0"},
    //use outside of panel in upper right of screen.  Show wil be turned off when there is no clock time source to display    
    { ON,  ON,   0, 865,   1, 170, 36,  3, RA8875_BLACK,      RA8875_LIGHT_GREY, RA8875_BLACK, RA8875_BLACK, 16, 10, "UTC:\0"},  // for RA8876 7.0"
    { ON,  ON,   0, 880,  41, 140, 85,  8, RA8875_LIGHT_GREY, RA8875_BLUE,       RA8875_BLACK, RA8875_BLACK,  7, 10, ""},  // for RA8876 7.0" S/MF Meter hotspot
    { ON, OFF,   0,   0, 190,1023,320, 20, RA8875_BLACK,      RA8875_BLACK,      RA8875_BLACK, RA8875_BLACK,  9, 20, ""}  // Spectrum TouchTune hotspot area definition.

    // For the above TouchTune hotspot box set the top and bottom some margin away from the touch labels and touch buttons
};

#endif  // USE_RA8875

// Labels are screen display objects that are used mostly to show status.
// They may be used for touch events, usually in combination with a button.  Take NR for example
//    NR has a button and a screen label. The NR button is not always visible so a label is usually permanent
//    but is too small for relaible touch events.
// A button may be used as a label also but carries some extra baggage. The Clock button is a good example,
//    it supports touch events later to maybe pop up a menu of clock settings.
// The UserInput.h "Button handler" will scan all reported touch events x,y coordinates for a match to any touch enabled buttons and labels.

struct Label labels[LABEL_NUM] = {
  //  en  show   x   y    w    h   r   outline_color     on_txtclr      on_color    off_txtclr     off_color  padx pady  label 
    {OFF, OFF,   0,   0,  40, 30, 3, RA8875_BLACK, RA8875_BLUE,  RA8875_BLACK,   RA8875_CYAN,  RA8875_BLACK, 3, 7, "B:\0"},   
    {OFF,  ON,  20, 110,  60, 30, 3, RA8875_BLACK, RA8875_YELLOW,RA8875_BLACK,   RA8875_YELLOW,RA8875_BLACK, 3, 7, "Mode\0"}, //Set SHOW to ON if you want this label to be drawn on screen.
    {OFF,  ON, 130, 110, 105, 30, 3, RA8875_BLACK, RA8875_CYAN,  RA8875_BLACK,   RA8875_CYAN,  RA8875_BLACK, 3, 7, "F:\0"},
    {OFF,  ON, 280, 110,  96, 30, 3, RA8875_BLACK, RA8875_BLUE,  RA8875_BLACK, RA8875_LIGHT_ORANGE, RA8875_BLACK, 3, 7, "R:\0"},
    {OFF,  ON, 430, 110,  72, 30, 3, RA8875_BLACK, RA8875_LIGHT_ORANGE, RA8875_BLACK, RA8875_BLUE, RA8875_BLACK, 3, 7, "AGC-\0"},
    {OFF,  ON, 560, 110,  60, 30, 3, RA8875_BLACK, RA8875_RED,   RA8875_BLACK,   RA8875_YELLOW,RA8875_BLACK, 3, 7, "ANT-\0"}, 
    {OFF,  ON,  10,  15,  48, 22, 3, RA8875_BLACK, RA8875_CYAN,  RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "ATT\0"},
    {OFF,  ON,  70,  15,  48, 22, 3, RA8875_BLACK, RA8875_BLACK, RA8875_BLUE,     myDARKGREY,  RA8875_BLACK, 6, 4, "Pre\0"},
    {OFF,  ON, 130,  15,  48, 22, 3, RA8875_CYAN,  RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 6, 4, "ATU\0"},
    {OFF,  ON,  10,  40,  48, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 5, 4, "RIT\0"},
    {OFF,  ON,  70,  40,  48, 22, 3, RA8875_BLACK, RA8875_RED,   RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 8, 4, "XIT\0"},
    {OFF,  ON, 130,  40,  60, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 5, 4, "Fine\0"},
    {OFF,  ON,  10,  65,  48, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "NB-\0"},
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
    {OFF, OFF, 699, 419, 100, 22, 3, RA8875_BLACK, RA8875_GREEN, RA8875_BLACK,    myDARKGREY,  RA8875_BLACK, 3, 4, "RefLvl\0"}  // No label on screen for this today
};

struct User_Settings user_settings[USER_SETTINGS_NUM] = {                      
    //Profile name  spect mn  pop uc1 uc2 uc3 lastB  mute  mic_En  micG LInLvl rfgen rfGain SpkEn  afgen afGain LoEn LoVol enet  enout  nben  nblvl nren  spot rbeep pitch  notch  xmit fine VFO-AB  DefMFknob   enc1          enc2          enc3    
    {"User Config #1", 10, 0, OFF,  0,  0,  0, BAND3,  OFF, MIC_OFF, 1.0,  15,   OFF,   100,   ON,   OFF,    80,  ON,  22,   ON,  OFF,  OFF,  NB5,  OFF,  OFF,  0.02,  600, NTCHOFF, OFF, OFF,   0,  MFTUNE,     MFTUNE,      REFLVL_BTN,  ATTEN_BTN}, // if no encoder is present assign it to 0 and it will be skipped. 
    {"User Config #2", 10, 0, OFF,  0,  0,  0, BAND2,  OFF, MIC_OFF, 1.0,  15,   OFF,   100,   ON,   OFF,    80,  ON,  22,  OFF,  OFF,  OFF,  NB2,  NR3,  OFF,  0.02,  600, NTCHOFF, OFF, OFF,   0,  MFTUNE,     MFTUNE,      RFGAIN_BTN,  AFGAIN_BTN},
    {"PanAdapter #1",  10, 0, OFF,  0,  0,  0, BAND0,  OFF, MIC_OFF, 1.0,  15,   OFF,   100,   ON,   OFF,    80,  ON,  22,   ON,  OFF,  OFF,  NB1,  OFF,  OFF,  0.02,  600, NTCHOFF, OFF, OFF,   0,  MFTUNE,     REFLVL_BTN,  REFLVL_BTN,  RFGAIN_BTN}
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

PROGMEM struct AGC agc_set[AGC_SET_NUM] = {
    {"AGC- ",2,0,0,-36.0,12.0,6.0},
    {"AGC-S",2,0,0,-36.0,12.0,6.0},
    {"AGC-M",2,0,0,-36.0,12.0,6.0},
    {"AGC-F",2,0,0,-36.0,12.0,6.0}
};

// Settings ranges
// threshold recommended to be between 1.5 and 20, closer to 3 maybe best
// nAnticipation is 1 to 125
// Decay is 1 to 10
PROGMEM struct NB nb[NB_SET_NUM] = {
    {"",    1.0,   1,  1},  // use "" to leave the number blank like AGC, suggesting it is off.
    {"1",   2.0,   5,  2},  // values suggested for test in the source code   
    {"2",   4.0,   5,  8},  
    {"3",   7.0,   2,  4},
    {"4",  15.0,  40,  8},    
    {"5",  20.0,  60, 10},  
    {"6",  80.0, 100,  4}
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
    {"4.00",  4200, "KHz", DATA}
};

PROGMEM struct TuneSteps  tstep[TS_STEPS] = {
    {"1 ",   "Hz",     1,  CW},
    {"10 ",  "Hz",    10, USB},
    {"100 ", "Hz",   100, USB},
    {"1.0", "KHz",  1000, USB},
    {"2.5", "KHz",  2500, USB},
    {"5.0", "KHz",  5000, USB}
};

PROGMEM struct Modes_List modeList[MODES_NUM] = {
    {0, "CW"},
    {1, "LSB"},
    {2, "USB"},
    {3, "DATA"}
 };

#endif //  _SDR_DATA_RA8876_H_ 