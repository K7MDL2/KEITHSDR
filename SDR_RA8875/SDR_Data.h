//
//  SDR_Data_RA8876.h
//
//
#ifndef _SDR_DATA_RA8876_H_
#define _SDR_DATA_RA8876_H_

#include "RadioConfig.h"
#include "SDR_RA8875.h"

struct Band_Memory bandmem[BANDS] = { 
    // name    lower   upper       VFOA   Md_A    VFOA-1  md1     VFOA-2  md2    VFOB   modeB filt   band      ts   agc  SPLIT RT RV XT XV ATU ANT BPF  ATTEN   AttByp att_DB  PREAMP    XVE XV#  SpRef
    {"160M", 1800000,  2000000,  1840000, DATA,  1860000, LSB,   1910000, LSB,  1860000, LSB, BW4_0, BAND160M,  1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 0,  ATTEN_OFF,  1,   80,  PREAMP_OFF,  0,  0,  5},
    { "80M", 3500000,  4000000,  3573000, DATA,  3868000, LSB,   3813000, LSB,  3868000, LSB, BW3_2, BAND80M,   1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 1,  ATTEN_OFF,  1,   40,  PREAMP_OFF,  0,  1,  5},
    { "60M", 4990000,  5405000,  5000000, AM,    5287200, LSB,   5364700, LSB,  5405000, USB, BW3_2, BAND60M,   1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 2,  ATTEN_OFF,  1,   40,  PREAMP_OFF,  0,  2,  5},
    #ifdef PANADAPTER
    {  "IF", 8200000,  8300000,  8215000, DATA,  8215000, USB,   8215000, USB,  8215000, LSB, BW2_8, PAN_ADAPT, 1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 0,  ATTEN_OFF,  0,   50,  PREAMP_OFF,  0,  0,  5}
    #else 
    { "40M", 7000000,  7300000,  7074000, DATA,  7030000, CW,    7200000, LSB,  7200000, LSB, BW4_0, BAND40M,   3,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT2, 3,  ATTEN_OFF,  1,   20,  PREAMP_OFF,  0,  3,  5},
    #endif
    { "30M", 9990000, 10150000, 10000000, AM,   10136000, DATA, 10130000, CW,  10136000, USB, BW3_2, BAND30M,   1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 4,  ATTEN_OFF,  1,   10,  PREAMP_OFF,  0,  4,  5},
    { "20M",14000000, 14350000, 14074000, DATA, 14030000, CW ,  14200000, USB, 14200000, USB, BW4_0, BAND20M,   3,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT2, 5,  ATTEN_OFF,  1,   50,  PREAMP_OFF,  0,  5,  5},
    { "17M",18068000, 18168000, 18100000, DATA, 18135000, USB,  18090000, CW,  18135000, USB, BW3_2, BAND17M,   1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 6,  ATTEN_OFF,  1,   50,  PREAMP_OFF,  0,  6,  5},
    { "15M",21000000, 21450000, 21074000, DATA, 21030000, CW,   21300000, USB, 21350000, USB, BW4_0, BAND15M,   3,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 7,  ATTEN_OFF,  0,    0,  PREAMP_OFF,  0,  7,  5},
    { "12M",24890000, 24990000, 24915000, USB,  24892000, CW,   24950000, USB, 24904000, USB, BW3_2, BAND12M,   1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 8,  ATTEN_OFF,  0,    0,  PREAMP_ON,   0,  8,  5},
    { "10M",28000000, 29600000, 28074000, DATA, 28200000, USB,  29400000, USB, 28200000, USB, BW4_0, BAND10M,   1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1, 9,  ATTEN_OFF,  0,    0,  PREAMP_ON,   0,  9,  5},
    {  "6M",50000000, 54000000, 50125000, USB,  50313000, DATA, 50100000, CW,  50313000, DATA,BW3_2, BAND6M,    1,AGC_SLOW,OFF,OFF,0,OFF,0,OFF,ANT1,10,  ATTEN_OFF,  0,    0,  PREAMP_OFF,  0, 10,  5},
};

// Fields are label text, enable band, Xvtr band ID, RF frequency, IF frequency, power, calibration offset, Band decoder output pattern
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

// Shared button placement for both RA8875 800x480 and RA8876 1024x600 displays
#ifdef USE_RA8875   // These rows differ between display sizes. 
    // Button Position variables for easy bulk size, place and move.
    const uint16_t x_0 = 1; // reserved for Menu row select button (Fucntion button)
    const uint16_t x_1 = 118;  // x_ valuesa are the left side refernce for button. combine with y_1 for anchor point in upper left.
    const uint16_t x_2 = 235;
    const uint16_t x_3 = 350;
    const uint16_t x_4 = 467;
    const uint16_t x_5 = 583;
    const uint16_t x_6 = 699;
    // rest of standard button dimensions
    const uint16_t y_1 = 419;  // upper position in px.  A Ra8875 4.3" display is 800px wide x 460px high
    const uint16_t w_1 = 100;  // button width px
    const uint16_t h_1 = 60;   // button height px
    const uint16_t r_1 = 20;   // button corner radius in px

    //#define STD_BTN_NUM 46      // number of buttons in the table **defined in SDR_RA8875.h**
    struct Standard_Button std_btn[STD_BTN_NUM] = 
    {
        // Special button for function button is presented on every row to advance to the next row.
        // Enable value is used to store th actiuve panel number for the function button operation. Starts at offset of 2. (2-1 to get panel 1 for example)   Do not change this value
        {  2,  ON, 0, 0, x_0, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 25, 20, "Fn 1\0"},
        
        // Panel 1  - this table is organized in sorted order for convenience but can be in any order using Panelnum an Panelpos
        // Show == ON unhides ths button for display on a panel.   Enable usually tracks the on/off state of the button.
        //en show pnl pos  x   y    w    h    r   outline_clr txtclr    on_clr off_clr padx pady label
        { ON,  ON, 1, 1, x_1, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Mode\0"},
        { ON,  ON, 1, 2, x_2, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 21, 20, "Filter\0"},
        { ON,  ON, 1, 3, x_3, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 24, 20, "Rate\0"},
        { ON,  ON, 1, 4, x_4, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  4, 20, "ATT\0"},
        { ON,  ON, 1, 5, x_5, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  5, 20, "Preamp\0"},
        { OFF, ON, 1, 6, x_6, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band\0"},
        //Panel 2
        { ON, OFF, 2, 1, x_1, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 22, 20, "NB\0"},
        { ON, OFF, 2, 2, x_2, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 32, 20, "NR\0"},
        { ON, OFF, 2, 3, x_3, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 18, 20, "Notch\0"},
        { ON, OFF, 2, 4, x_4, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK,  8, 20, "AGC- \0"},
        { ON, OFF, 2, 5, x_5, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  7, 20, "Zoom\0"},
        { ON, OFF, 2, 6, x_6, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  5, 20, "Pan\0"},
        //Panel 3
        { ON, OFF, 3, 1, x_1, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 18, 20, "Menu\0"},
        { ON, OFF, 3, 2, x_2, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 18, 20, "ANT  \0"},
        { ON, OFF, 3, 3, x_3, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 25, 20, "ATU\0"},
        { ON, OFF, 3, 4, x_4, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 22, 20, "XMIT\0"},
        { ON, OFF, 3, 5, x_5, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 12, 20, "Band -\0"},
        { ON, OFF, 3, 6, x_6, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 12, 20, "Band +\0"},
        //Panel 4
        { ON, OFF, 4, 1, x_1, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 31, 20, "RIT\0"},
        { ON, OFF, 4, 2, x_2, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 31, 20, "XIT\0"},   
        { ON, OFF, 4, 3, x_3, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 26, 20, "Fine\0"},
        { ON, OFF, 4, 4, x_4, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 23, 20, "Split\0"},
        { ON, OFF, 4, 5, x_5, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK,  9, 20, "Display\0"},
        { ON, OFF, 4, 6, x_6, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLUE,  31, 20, "A/B\0"},
        //Panel 5
        { ON, OFF, 5, 1, x_1, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 23, 20, "Enet\0"},
        { ON, OFF, 5, 2, x_2, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 30, 20, "Xvtr\0"},
        { ON, OFF, 5, 3, x_3, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  9, 20, "RF:\0"},
        {OFF, OFF, 5, 4, x_4, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 14, 20, "RefLvl\0"},
        { ON, OFF, 5, 5, x_5, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  9, 20, "AF:\0"},
        { ON, OFF, 5, 6, x_6, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 23, 20, "Mute\0"},

        // Panelpos ==255 means it is a special type button not on a panel
        //use outside of panel in upper right of screen.  Show wil be turned off when there is no clock time source to display
        { ON,  ON, 0, 255, 630,   1, 170,  36,   3, BLACK,      LIGHTGREY,  BLACK, BLACK, 16, 10, "UTC:\0"},  // For RA8875 4.3"
        { ON,  ON, 0, 255, 645,  40, 140,  85,   8, LIGHTGREY,  BLUE,       BLACK, BLACK,  7, 10, ""},  // S/MF meter for RA8875 4.3"
        { ON, OFF, 0, 255,   0, 200, 800, 180, r_1, BLACK,      BLACK,      BLACK, BLACK,  9, 20, ""},  // Spectrum TouchTune area definition.

        // This defines the Band Select window and buttons
        #define BS_ANCHOR_X 100
        #define BS_ANCHOR_Y 80
        { OFF, OFF, 0, 255, BS_ANCHOR_X, BS_ANCHOR_Y, 600, 280, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK,  9, 20, "Band Select"},   // Band Select menu Window
        // Spare
        //    { ON, OFF, 2, 255, 467, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 23, 20, "Spot\0"},
        
        // These are the Band buttons.  Use panel_num 100
        // Can use either x and y, or use the pos_num to set the displayed order as we fit buttons into the window
        #define BSX_0  (BS_ANCHOR_X+20)
        #define BSX_1  (BS_ANCHOR_X+20+(w_1+14))
        #define BSX_2 ((BS_ANCHOR_X+20+(w_1+14)*2))
        #define BSX_3 ((BS_ANCHOR_X+20+(w_1+14)*3))
        #define BSX_4 ((BS_ANCHOR_X+20+(w_1+14)*4))

        #define BSY_0 (BS_ANCHOR_Y+80)
        #define BSY_1 (BS_ANCHOR_Y+80+(h_1+40))
        #define BSY_2 (BS_ANCHOR_Y+80+(h_1+40)*2)
        
        // This group is used for the Band Select Menu Window buttons.
        // 255 is disabled and will be skipped over. panelnum == 100 is the Band select Window. panelpos # is the first button to draw.
        { OFF, OFF, 100, 255, BSX_0, BSY_0, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 10, 20, "160M\0"},
        { OFF, OFF, 100,   0, BSX_1, BSY_0, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "80M\0"},   
        { OFF, OFF, 100,   1, BSX_2, BSY_0, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "60M\0"},
        { OFF, OFF, 100,   2, BSX_3, BSY_0, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "40M\0"},
        { OFF, OFF, 100,   2, BSX_4, BSY_0, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "30M\0"},
        { OFF, OFF, 100,   3, BSX_0, BSY_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "20M\0"},
        { OFF, OFF, 100,   4, BSX_1, BSY_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 24, 20,  "17M\0"},
        { OFF, OFF, 100,   5, BSX_2, BSY_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "15M\0"},
        { OFF, OFF, 100,   6, BSX_3, BSY_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "12M\0"},
        { OFF, OFF, 100,   7, BSX_4, BSY_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "10M\0"},
        { OFF, OFF, 100, 255, BSX_0, BSY_2, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 30, 30,   "6M\0"}
    };
#else   // Use the below RA8876 layout
    // Button Position variables for easy bulk size, place and move.
    const uint16_t x_0 = 1; // reserved for Menu row select button (Fucntion button)
    const uint16_t x_1 = 118;  // x_ valuesa are the left side refernce for button. combine with y_1 for anchor point in upper left.
    const uint16_t x_2 = 235;
    const uint16_t x_3 = 350;
    const uint16_t x_4 = 467;
    const uint16_t x_5 = 583;
    const uint16_t x_6 = 699;
    const uint16_t x_7 = 815;
    const uint16_t x_8 = 931;
    // rest of standard button dimensions
    const uint16_t y_1 = 539;
    const uint16_t w_1 = 100;  // button width px
    const uint16_t h_1 = 60;   // button height px
    const uint16_t r_1 = 20;   // button corner radius in px
    //#define STD_BTN_NUM 49      // number of buttons in the table **defined in SDR_RA8875.h**
    struct Standard_Button std_btn[STD_BTN_NUM] = 
    {
        // Special button for function button is presented on every row to advance to the next row.
        // Enable value is used to store th actiuve panel number for the function button operation. Starts at offset of 2. (2-1 to get panel 1 for example)   Do not change this value
        {  2,  ON, 0, 0, x_0, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 25, 20, "Fn 1\0"},
        
        // Panel 1  - this table is organized in sorted order for convenience but can be in any order using Panelnum an Panelpos
        // Show == ON unhides ths button for display on a panel.   Enable usually tracks the on/off state of the button.
        //en show pnl pos  x   y    w    h    r   outline_clr txtclr    on_clr off_clr padx pady label
        { ON,  ON, 1, 1, x_1, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Mode\0"},
        { ON,  ON, 1, 2, x_2, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 21, 20, "Filter\0"},
        { ON,  ON, 1, 3, x_3, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 24, 20, "Rate\0"},
        { ON,  ON, 1, 4, x_4, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  4, 20, "ATT\0"},
        { ON,  ON, 1, 5, x_5, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  5, 20, "Preamp\0"},
        { OFF, ON, 1, 6, x_6, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band\0"},
        //{ OFF, ON, 1, 7, x_7, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band1\0"},
        //{ OFF, ON, 1, 8, x_8, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band\0"},
        //Panel 2
        { ON, OFF, 2, 1, x_1, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 22, 20, "NB\0"},
        { ON, OFF, 2, 2, x_2, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 32, 20, "NR\0"},
        { ON, OFF, 2, 3, x_3, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 18, 20, "Notch\0"},
        { ON, OFF, 2, 4, x_4, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK,  8, 20, "AGC- \0"},
        { ON, OFF, 2, 5, x_5, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  7, 20, "Zoom\0"},
        { ON, OFF, 2, 6, x_6, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  5, 20, "Pan\0"},
        //{ OFF, ON, 2, 7, x_7, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band2\0"},
        //{ OFF, ON, 2, 8, x_8, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band\0"},
        //Panel 3
        { ON, OFF, 3, 1, x_1, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 18, 20, "Menu\0"},
        { ON, OFF, 3, 2, x_2, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 18, 20, "ANT  \0"},
        { ON, OFF, 3, 3, x_3, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 25, 20, "ATU\0"},
        { ON, OFF, 3, 4, x_4, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 22, 20, "XMIT\0"},
        { ON, OFF, 3, 5, x_5, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 12, 20, "Band -\0"},
        { ON, OFF, 3, 6, x_6, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 12, 20, "Band +\0"},
        //{ OFF, ON, 3, 7, x_7, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band3\0"},
        //{ OFF, ON, 3, 8, x_8, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band\0"},
        //Panel 4
        { ON, OFF, 4, 1, x_1, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 31, 20, "RIT\0"},
        { ON, OFF, 4, 2, x_2, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 31, 20, "XIT\0"},   
        { ON, OFF, 4, 3, x_3, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 26, 20, "Fine\0"},
        { ON, OFF, 4, 4, x_4, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 23, 20, "Split\0"},
        { ON, OFF, 4, 5, x_5, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK,  9, 20, "Display\0"},
        { ON, OFF, 4, 6, x_6, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLUE,  31, 20, "A/B\0"},
        //{ OFF, ON, 4, 7, x_7, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band4\0"},
        //{ OFF, ON, 4, 8, x_8, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band\0"},
        //Panel 5
        { ON, OFF, 5, 1, x_1, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 23, 20, "Enet\0"},
        { ON, OFF, 5, 2, x_2, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 30, 20, "Xvtr\0"},
        { ON, OFF, 5, 3, x_3, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  9, 20, "RF:\0"},
        { OFF,OFF, 5, 4, x_4, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 14, 20, "RefLvl\0"},
        { ON, OFF, 5, 5, x_5, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK,  9, 20, "AF:\0"},
        { ON, OFF, 5, 6, x_6, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLUE,  BLACK, 23, 20, "Mute\0"},
        //{ OFF, ON, 5, 7, x_7, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band5\0"},
        //{ OFF, ON, 5, 8, x_8, y_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK, 20, 20, "Band\0"},
        // End of panel buitton definitions

        //  These are special button types
        { ON,  ON, 0, 255, 865,   1, 170,  36,   3, BLACK,      LIGHTGREY,  BLACK, BLACK, 16, 10, "UTC:\0"},  // for RA8876 7.0"
        { ON,  ON, 0, 255, 880,  41, 140,  85,   8, LIGHTGREY,  BLUE,       BLACK, BLACK,  7, 10, ""},  // for RA8876 7.0" S/MF Meter hotspot
        { ON, OFF, 0, 255,   0, 190,1023, 320, r_1, BLACK,      BLACK,      BLACK, BLACK,  9, 20, ""},  // Spectrum TouchTune hotspot area definition.
        // For the above TouchTune hotspot box set the top and bottom some margin away from the touch labels and touch buttons
        
        // Define the Band Stack window and buttons
        #define BS_ANCHOR_X 210
        #define BS_ANCHOR_Y 160
        // This defines the Band Select window
        { OFF,OFF, 0, 255, BS_ANCHOR_X, BS_ANCHOR_Y, 600, 280, r_1, LIGHTGREY, LIGHTGREY, BLACK, BLACK,  9, 20, "Band Select"},   // Band Select menu Window

        // These are the Band buttons.  Use panel_num 100
        // Can use either x and y, or use the pos_num to set the displayed order as we fit buttons into the window
        #define BSX_0  (BS_ANCHOR_X+20)
        #define BSX_1  (BS_ANCHOR_X+20+(w_1+14))
        #define BSX_2 ((BS_ANCHOR_X+20+(w_1+14)*2))
        #define BSX_3 ((BS_ANCHOR_X+20+(w_1+14)*3))
        #define BSX_4 ((BS_ANCHOR_X+20+(w_1+14)*4))

        #define BSY_0 (BS_ANCHOR_Y+80)
        #define BSY_1 (BS_ANCHOR_Y+80+(h_1+40))
        #define BSY_2 (BS_ANCHOR_Y+80+(h_1+40)*2)
        
        // This group is used for the Band Select Menu Window buttons.
        // 255 is disabled and will be skipped over. panelnum == 100 is the Band select Window. panelpos # is the first button to draw.
        { OFF, OFF, 100, 255, BSX_0, BSY_0, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 10, 20, "160M\0"},
        { OFF, OFF, 100,   0, BSX_1, BSY_0, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "80M\0"},   
        { OFF, OFF, 100,   1, BSX_2, BSY_0, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "60M\0"},
        { OFF, OFF, 100,   2, BSX_3, BSY_0, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "40M\0"},
        { OFF, OFF, 100,   2, BSX_4, BSY_0, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "30M\0"},
        { OFF, OFF, 100,   3, BSX_0, BSY_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "20M\0"},
        { OFF, OFF, 100,   4, BSX_1, BSY_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 24, 20,  "17M\0"},
        { OFF, OFF, 100,   5, BSX_2, BSY_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "15M\0"},
        { OFF, OFF, 100,   6, BSX_3, BSY_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "12M\0"},
        { OFF, OFF, 100,   7, BSX_4, BSY_1, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 26, 20,  "10M\0"},
        { OFF, OFF, 100, 255, BSX_0, BSY_2, w_1, h_1, r_1, LIGHTGREY, LIGHTGREY, NAVY, NAVY, 30, 30,   "6M\0"}
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
    {OFF, OFF,   0,   0,  40, 29, 3, BLACK, BLUE,  BLACK, CYAN,       BLACK, 3, 7, "B:\0"},   
    {OFF,  ON,  20, 110,  90, 28, 3, BLACK, YELLOW,BLACK, YELLOW,     BLACK, 3, 7, "Mode\0"}, //Set SHOW to ON if you want this label to be drawn on screen.
    {OFF,  ON, 130, 110, 105, 28, 3, BLACK, CYAN,  BLACK, CYAN,       BLACK, 3, 7, "F:\0"},
    {OFF,  ON, 280, 110,  96, 28, 3, BLACK, BLUE,  BLACK, ORANGE,     BLACK, 3, 7, "R:\0"},
    {OFF,  ON, 430, 110,  72, 28, 3, BLACK, ORANGE,BLACK, BLUE,       BLACK, 3, 7, "AGC-\0"},
    {OFF,  ON, 560, 110,  60, 28, 3, BLACK, RED,   BLACK, YELLOW,     BLACK, 3, 7, "ANT-\0"}, 
    {OFF,  ON,  10,  15,  48, 22, 3, BLACK, CYAN,  BLACK, myDARKGREY, BLACK, 3, 4, "ATT\0"},
    {OFF,  ON,  70,  15,  48, 22, 3, BLACK, BLACK, BLUE,  myDARKGREY, BLACK, 6, 4, "Pre\0"},
    {OFF,  ON, 130,  15,  48, 22, 3, CYAN,  GREEN, BLACK, myDARKGREY, BLACK, 6, 4, "ATU\0"},
    {OFF,  ON,  10,  40,  48, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 5, 4, "RIT\0"},
    {OFF,  ON,  70,  40,  48, 22, 3, BLACK, RED,   BLACK, myDARKGREY, BLACK, 8, 4, "XIT\0"},
    {OFF,  ON, 130,  40,  60, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 5, 4, "Fine\0"},
    {OFF,  ON,  10,  65,  48, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 8, 4, "NB\0"},
    {OFF,  ON,  70,  65,  48, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 9, 4, "NR\0"},
    {OFF,  ON, 130,  65,  60, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 2, 4, "Notch\0"},
    #ifdef USE_RA8875
    {OFF,  ON, 220,  65,  90, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 3, 4, "Split\0"},
    #else
    {OFF,  ON, 320,  65,  90, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 3, 4, "Split\0"},
    #endif
    {OFF, OFF, 699, 419, 100, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 3, 4, "Mute\0"}, // No label on screen for this today
    {OFF, OFF, 467, 419, 100, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 3, 4, "XMIT\0"}, // No label on screen for this today
    {OFF, OFF, 583, 419, 100, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 3, 4, "Xvtr\0"}, // No label on screen for this today
    {OFF, OFF, 699, 419, 100, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 3, 4, "RefLvl\0"},  // No label on screen for this today
    {OFF, OFF, 583, 419, 100, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 2, 4, "Zoom\0"}, // No label on screen for this today
    {OFF, OFF, 699, 419, 100, 22, 3, BLACK, GREEN, BLACK, myDARKGREY, BLACK, 3, 4, "Pan\0"} // No label on screen for this today
};

struct User_Settings user_settings[USER_SETTINGS_NUM] = {                      
//Profile name    sp_preset mn  sub_VFO  sv_md uc1 uc2 uc3  lastB   mute  mic_En  micG LInLvl rfg_en rfGain SpkEn afgen afGain LoRX LoTX enet  enout  nben   nblvl  nren  spot  rbeep pitch   notch  xmit fine VFO-AB DefMFknob  enc1   enc1_sw   enc1_swl     enc2     enc2_sw   enc2_swl  enc3     enc3_sw   enc3_swl    enc4    enc4_sw enc4_swl   enc5        enc5_sw   enc5_swl enc6        enc6_sw     enc6_swl    Zoom_lvl panEn panlvl
    {"ENET ON Config",    0, 0, 28000000, USB, 0,  0,  0, BAND80M,   OFF, MIC_ON,  76.0,  15,   OFF,   100,   ON,   OFF, 100,  16,  16,   ON,  OFF,  OFF,  NBOFF,  OFF,  OFF,  0.02,  600, NTCHOFF, OFF, OFF,   0,    MFTUNE,   MFTUNE, RATE_BTN, FILTER_BTN, RFGAIN_BTN, MODE_BTN, FINE_BTN, PAN_BTN, ZOOM_BTN, VFO_AB_BTN, NB_BTN, NR_BTN, NOTCH_BTN, AFGAIN_BTN, MUTE_BTN, RIT_BTN, REFLVL_BTN, BANDUP_BTN, BANDDN_BTN, ZOOMx1, OFF, 50}, // if no encoder is present assign it to 0 and it will be skipped. 
    {"User Config #2",    0, 0, 14200000, USB, 0,  0,  0, BAND30M,   OFF, MIC_ON,  50.0,  15,   OFF,   100,   ON,   OFF, 100,  22,  16,  OFF,  OFF,  OFF,  NBOFF,  OFF,  OFF,  0.02,  600, NTCHOFF, OFF, OFF,   0,    MFTUNE,   MFTUNE, RATE_BTN, FILTER_BTN, RFGAIN_BTN, MODE_BTN, FINE_BTN, PAN_BTN, ZOOM_BTN, VFO_AB_BTN, NB_BTN, NR_BTN, NOTCH_BTN, AFGAIN_BTN, MUTE_BTN, RIT_BTN, REFLVL_BTN, BANDUP_BTN, BANDDN_BTN, ZOOMx1, OFF, 50},
    {"PanAdapter Config", 0, 0, 1420000,  USB, 0,  0,  0, PAN_ADAPT, OFF, MIC_OFF, 76.0,  15,   OFF,   100,   ON,   OFF, 100,  16,  16,  OFF,  OFF,  OFF,  NBOFF,  OFF,  OFF,  0.02,  600, NTCHOFF, OFF, OFF,   0,    MFTUNE,   MFTUNE, RATE_BTN, FILTER_BTN, RFGAIN_BTN, MODE_BTN, FINE_BTN, PAN_BTN, ZOOM_BTN, VFO_AB_BTN, NB_BTN, NR_BTN, NOTCH_BTN, AFGAIN_BTN, MUTE_BTN, RIT_BTN, REFLVL_BTN, BANDUP_BTN, BANDDN_BTN, ZOOMx1, OFF, 50}
};

struct Frequency_Display disp_Freq[FREQ_DISP_NUM] = {
    // x   y    w    h    r   bs  bm   outline_clr     box_clr              bg_clr      txt_clr              txt_Fnt   TX_clr     padx  pady
    #ifdef USE_RA8875
    {204,  1, 380,  50,   3,  0,  0, BLACK,      LIGHTGREY,  BLACK,  LIGHTGREY,  Arial_40, RED,   4,   4}, // VFO Active Digits
    {588,  6,  40,  40,   3,  0,  0, LIGHTGREY,  BLACK,      BLACK,  GREEN,      Arial_24, RED,   9,   7}, // VFO Active Label
    {310, 53, 274,  40,   3,  0,  0, BLACK,      BLACK,      BLACK,  myDARKGREY, Arial_28, RED,   6,   6}, // VFO Stby Digits
    {588, 53,  40,  40,   3,  0,  0, LIGHTGREY,  BLACK,      BLACK,  myDARKGREY, Arial_24, RED,   9,   7}  // VFO Stby Label
    #else
    {304,  1, 380,  50,   3,  0,  0, BLACK,      LIGHTGREY,  BLACK,  LIGHTGREY,  Arial_40, RED,   4,   4}, // VFO Active Digits
    {688,  6,  40,  40,   3,  0,  0, LIGHTGREY,  BLACK,      BLACK,  GREEN,      Arial_24, RED,   9,   7}, // VFO Active Label
    {410, 53, 274,  40,   3,  0,  0, BLACK,      BLACK,      BLACK,  myDARKGREY, Arial_28, RED,   6,   6}, // VFO Stby Digits
    {688, 53,  40,  40,   3,  0,  0, LIGHTGREY,  BLACK,      BLACK,  myDARKGREY, Arial_24, RED,   9,   7}  // VFO Stby Label
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
#define PRESETS 1  // number of parameter records with our preset spectrum window values
struct Spectrum_Parms Sp_Parms_Def[PRESETS] = { // define default sets of spectrum window parameters, mostly for easy testing but could be used for future custom preset layout options
        //W        LE  RE CG                                         x   y   w  h  c sp st clr sc mode      scal reflvl wfrate
    #ifdef USE_RA8875
        {798,0, 0,  0,798,398,14,8,157,179,179,408,400,110,111,289,289,  0,153,799,256,50,20,6,240,1.0,0.9,1,20, 5, 0}      // Default layout for 4.3" RA8875 800 x480
    #else
        {1020,1,1,  1,1021,510,14,8,143,165,165,528,520,142,213,307,307,  0,139,1022,390,40,20,6,890,1.5,0.9,1,20,10, 80}   // Default layout for 7" RA8876 1024 x 600
    #endif        
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
