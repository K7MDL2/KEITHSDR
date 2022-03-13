#ifndef _SDR_RA8875_H_
#define _SDR_RA8875_H_

//  SDR_RA8875.h
//
//  Header File for the Main Arduino program file
//
//  Spectrum, Display, full F32 library conversion completed 3/2021. Uses FFTXXXX_IQ_F32 FFT I and Q version files
//      XXXX can be the 256, 1024, 2048 or 4096 versions.
//  Spectrum uses the raw FFT output and is not calibrated.
//
//  Test tones are enabled in spectrum only, not in audio path.
//
#include <Arduino.h>
#include <avr/pgmspace.h>
// #include <T4_PowerButton.h>     // https://github.com/FrankBoesing/T4_PowerButton for the FlexInfo() and Hardfault reporting tools
//  T4_Powerbutton is not compatible with newer versions of Arduino, worked for 1.8.13, does not for 1.8.19.
#include <SPI.h>                // included with Arduino
//#include <SD.h>                 // included with Arduino
#include <Wire.h>               // included with Arduino
#include <WireIMXRT.h>          // gets installed with wire.h
#include <WireKinetis.h>        // included with Arduino
#define  ENCODER_OPTIMIZE_INTERRUPTS  // leave this one here.  Not normally user changed
#include <Encoder.h>            // Internal Teensy library and at C:\Program Files (x86)\Arduino\hardware\teensy\avr\libraries
#include <Metro.h>              // GitHub https://github.com/nusolar/Metro
#include <Audio.h>              // Included with Teensy and at GitHub https://github.com/PaulStoffregen/Audio
#include <OpenAudio_ArduinoLibrary.h> // F32 library located on GitHub. https://github.com/chipaudette/OpenAudio_ArduinoLibrary
#include <InternalTemperature.h>// V2.1.0 @ Github https://github.com/LAtimes2/InternalTemperature
#include <TimeLib.h>            // TODO  - list where to find this

//#define BYPASS_SPECTRUM_MODULE   // debugging temp 

// Below are local project files
#include "RadioConfig.h"        // Our main configuration file
#ifndef BYPASS_SPECTRUM_MODULE
  #include <Spectrum_RA887x.h>    // New K7MDL Spectrum and Waterfall library created Jan 2022
#endif
#include "SDR_Network.h"        // for ethernet UDP remote control and monitoring
#include "Vfo.h"
#include "Display.h"
#include "Tuner.h"
#include "Mode.h"
#include "Smeter.h"
#include "Controls.h"
#include "UserInput.h"          // include after Spectrum_RA8875.h and Display.h
#include "Bandwidth2.h"
#ifdef USE_RS_HFIQ
    #include <SDR_RS_HFIQ.h>   // https://github.com/K7MDL2/Teensy4_USB_Host_RS-HFIQ_Library
#endif

// Simple ways to designate functions to run out of fast or slower memory to help save RAM
#define HOT FASTRUN    __attribute__((hot))
#define COLD FLASHMEM  __attribute__((cold))

#ifndef BYPASS_SPECTRUM_MODULE
//
//--------------------------------- RA8875 LCD TOUCH DISPLAY INIT & PINS --------------------------
//
//  Usually also defined in main program header file such as RadioConfig.h for SDR_887x program
    #ifdef USE_RA8875
        #define  SCREEN_WIDTH       800 
        #define  SCREEN_HEIGHT      480
        #define  RA8875_INT        14   //any pin
        #define  RA8875_CS         10   //any digital pin
        #define  RA8875_RESET      9    //any pin or nothing!
        #define  MAXTOUCHLIMIT     3    //1...5  using 3 for 3 finger swipes, otherwise 2 for pinches or just 1 for touch
        #include <SPI.h>                // included with Arduino
        #include <RA8875.h>           // internal Teensy library with ft5206 cap touch enabled in user_setting.h
        #include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
        #include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
    #else 
        #define USE_RA8876_t3
        #define  SCREEN_WIDTH      1024 
        #define  SCREEN_HEIGHT     600
        #include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
        #include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
        #include <RA8876_t3.h>           // Github
        #include <FT5206.h>
        //#define  CTP_INT           14   // Use an interrupt capable pin such as pin 2 (any pin on a Teensy)
        //#define  RA8876_CS         10   //any digital pin
        //#define  RA8876_RESET      9    //any pin or nothing!
        //#define  MAXTOUCHLIMIT     3    //1...5  using 3 for 3 finger swipes, otherwise 2 for pinches or just 1 for touch
        
        // From RA8875/_settings/RA8875ColorPresets.h
        // Colors preset (RGB565)
        const uint16_t	RA8875_BLACK            = 0x0000;
        const uint16_t 	RA8875_WHITE            = 0xFFFF;
        const uint16_t	RA8875_RED              = 0xF800;
        const uint16_t	RA8875_GREEN            = 0x07E0;
        const uint16_t	RA8875_BLUE             = 0x001F;
        const uint16_t 	RA8875_CYAN             = RA8875_GREEN | RA8875_BLUE; //0x07FF;
        const uint16_t 	RA8875_MAGENTA          = 0xF81F;
        const uint16_t 	RA8875_YELLOW           = RA8875_RED | RA8875_GREEN; //0xFFE0;  
        const uint16_t 	RA8875_LIGHT_GREY 		= 0xB5B2; // the experimentalist
        const uint16_t 	RA8875_LIGHT_ORANGE 	= 0xFC80; // the experimentalist
        const uint16_t 	RA8875_DARK_ORANGE 		= 0xFB60; // the experimentalist
        const uint16_t 	RA8875_PINK 		    = 0xFCFF; // M.Sandercock
        const uint16_t 	RA8875_PURPLE 		    = 0x8017; // M.Sandercock
        const uint16_t 	RA8875_GRAYSCALE 		= 2113; //grayscale30 = RA8875_GRAYSCALE*30
    #endif // USE_RA8876_t3

    #define myLT_GREY               RA8875_LIGHT_GREY 
    #define myBLUE                  RA8875_BLUE
    #define myBLACK                 RA8875_BLACK
    #define myWHITE                 RA8875_WHITE
    #define myYELLOW                RA8875_YELLOW
    #define myGREEN                 RA8875_GREEN
    #define myRED                   RA8875_RED
    #define myLIGHT_ORANGE          RA8875_LIGHT_ORANGE
    
    const uint16_t myDARK_GREEN         = 0x03C0;
    const uint16_t myVERY_DARK_GREEN    = 0x02C0;
    const uint16_t myDARK_BLUE          = 0x02B0;
    const uint16_t myVERY_DARK_BLUE     = 0x01B0;

    #ifdef USE_RA8875
	//extern RA8875 tft;
    #else 
	//extern RA8876_t3 tft;
    //int16_t	_activeWindowXL = 0;
    //int16_t _activeWindowXR = SCREEN_WIDTH;
    //int16_t	_activeWindowYT = 0;
    //int16_t _activeWindowYB = SCREEN_HEIGHT;
    #endif

    // use the generator function to create 1 set of data to define preset values for window size and placement.  
    // Just copy and paste from the serial terminal into each record row.
    #define PRESETS 12  // number of parameter records with our preset spectrum window values
    ///******************************************************************************************************************************
    //   *************  Set Preset to select your configuration record to use.  The rest are ignored for operation ******************
    ///
    #define SPECTRUM_PRESET 0

#endif // BYPASS_SPECTRUM_MODULE


///////////////////////Set up global variables for Frequency, mode, bandwidth, step
// Index to bandmem table rows.  Use BANDX since rows can be in any order
                    // Band slot index into data table
#define BAND160M      0 // 160M 
#define BAND80M       1 // 80M
#define BAND60M       2 // 60M
#define BAND40M       3 // 40M
#define BAND30M       4 // 30M
#define BAND20M       5 // 20M
#define BAND17M       6 // 17M      
#define BAND15M       7 // 15M
#define BAND12M       8 // 12M
#define BAND10M       9 // 10M
#define BAND6M       10 // 6M
#define BAND2M       11 // 2M
#define PAN_ADAPT    12 // Panadapter IF band

// Zoom level for UI control
#define ZOOMx1      0       // Zoom out the most (fft1024 @ 48K) (aka OFF)  x1 reference
#define ZOOMx2      1       // in between (fft2048)  is x2 of 1024
#define ZOOMx4      2       // Zoom in the most (fft4096 at 96K)   is x4 of 1024
#define ZOOM_NUM    3       // Number of zoom level choiced for menu system

// ------------------------  OPERATIONAL PARAMETER STORAGE --------------------------------------
//
//  Most users will not normally mess around in this section but you can edit some of the table data to refine the default to your liking.  
//  Contained here are per-band settings, User Profile settings (globals), Font type and sizes
//  buttons and indicator colors, button an label locations and more. 
//  Pretty much every global variable that controls a setting is here in a table of some sort. 
//
#define CW          0
#define CW_REV      1
#define USB         2
#define LSB         3     
#define DATA        4
#define DATA_REV    5
#define AM          6
#define FM          7

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
#define AGC_OFF     0       // Index to AGC Settings table
#define AGC_SLOW    1
#define AGC_MED     2
#define AGC_FAST    3
#define MIC_OFF     0
#define MIC_ON      1
#define BW0_25      0
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
#define NB5         4
#define NB6         4

#define NROFF       0
#define NR1         1
#define NR2         2
#define NR3         3
#define NR4         4
#define NTCHOFF     0
#define NTCH1       1
#define NTCH2       2

// Some defines for ease of use 
#define myDARKGREY    31727u
// From RA8876_t3/RA8876Registers.h
#define BLACK		  0x0000
#define WHITE		  0xffff
#define RED		  	  0xf800
#define LIGHTRED	  0xfc10
#define CRIMSON		  0x8000
#define GREEN		  0x07e0
#define PALEGREEN	  0x87f0
#define DARKGREEN	  0x0400
#define BLUE		  0x001f
#define LIGHTBLUE	  0x051f
#define SKYBLUE		  0x841f
#define DARKBLUE	  0x0010
#define YELLOW		  0xffe0
#define LIGHTYELLOW	  0xfff0
#define DARKYELLOW	  0x8400 // mustard
#define CYAN		  0x07ff
#define LIGHTCYAN	  0x87ff
#define DARKCYAN	  0x0410
#define MAGENTA		  0xf81f
#define VIOLET		  0xfc1f
#define BLUEVIOLET	  0x8010
#define ORCHID		  0xA145 
// Other sources of RGB color definitions
#define NAVY          0x000F
#define MAROON        0x7800
#define PURPLE        0x780F
#define OLIVE         0x7BE0
#define LIGHTGREY     0xC618
#define DARKGREY      0x7BEF
#define ORANGE        0xFD20
#define GREENYELLOW   0xAFE5
#define PINK          0xF81F

// This group defines the number of records in each structure
#define MODES_NUM   8
#define FREQ_DISP_NUM  4
#define BANDS       12
#define XVTRS       12
#define TS_STEPS    6
#define FILTER      9
#define AGC_SET_NUM 4
#define NB_SET_NUM  7
#define USER_SETTINGS_NUM 3
#define LABEL_NUM   22      // number of labels in the table
#define STD_BTN_NUM 35      // number of buttons in the table

// Alternative to #define XXX_BTN is use "const int XXX_BTN" or enum to create index names to the table.
// enum Button_List {FN_BTN, MODE_BTN, FILTER_BTN, ATTEN_BTN, PREAMP_BTN, };
// using #define method as it is easiet to relate the purpose and more obvious which row it is mapped to.
#define PANEL_ROWS  7       // Set Number of panels + 2.  0 is disable, 1 is not used.
                            // numbers 2 and up are the panel index number (panel number -2) for the panel to display.
//  There are 6 100px wide buttons that can swap places, enabled/dispable by the function button for a row
//Anchor buttons normally stay put
//Panel 1  Fn is an anchor, the rest swap out
#define FN_BTN      0       // Swaps out buttons in bottom row.  This stays for each row.
#define MODE_BTN    1       // index to button
#define FILTER_BTN  2       // will display the current FILTER value in the button
#define RATE_BTN    3       // will display the RATE (step) value in the button
#define ATTEN_BTN   4       // invoke a pop up (ON) for atten slider or MF knob adjustment
#define PREAMP_BTN  5       // On off
#define BAND_BTN    6       // Pops up Band Window.  Touch or MF can change bands
//Panel 2  These swap out
#define NB_BTN      7       // not implemented yet
#define NR_BTN      8       // will display the NR value in the button
#define NOTCH_BTN   9       // will display the NR value in the button
#define AGC_BTN     10      // will display the AGC value in the button
#define ZOOM_BTN    11      // Sets Zoom level
#define PAN_BTN     12      // Enables PAN spectrum shift
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
#define FINE_BTN    21      // Fine and coarse rate
#define SPLIT_BTN   22      // ON/OFF
#define DISPLAY_BTN 23      // Invoke PopUp (On), close popup (Off)
#define VFO_AB_BTN  24      // ON/(A)/OFF(B)   Will go away soon. 
//Panel 5
#define ENET_BTN    25      // turn on and off the enet data output (does not enable/disable the enet hardware) 
#define XVTR_BTN    26      // not implemented yet
#define RFGAIN_BTN  27      // Sets digital RF level
#define REFLVL_BTN  28      // Sets the Spectrum Noise floor.
#define AFGAIN_BTN  29      // Sets digital AF level
#define MUTE_BTN    30      // Audio headphone jack output on/off

// Not in a Panel
#define UTCTIME_BTN 31      // NTP UTC time when ethernet (and internet) is available 
#define SMETER_BTN  32      // Box for the Smeter.  Can be a meter for any use.  Can touch the meter to configure maybe
#define SPECTUNE_BTN 33     // Converts a touch in the spectrum window to a frequency to tune too.
#define SPOT_BTN    34       // will display the NR value in the button

// The #define button numbers act as the ID of possible owners of MF knob services
#define MFTUNE      50      // Fake button so the MF knob can tune the VFO since there is no button
                            // Make sure this does have the same value as any buttom #defne value. 50 is safe.
#define MFNONE      0       // No active MF knob client.  0 is safe.

// Our Database of settings. This is the "factory default".  A user copy will be stored in EEPROM with user changes
struct Band_Memory {
    char        band_name[20];  // Freindly name or label.  Default here but can be changed by user.
    uint32_t    edge_lower;     // band edge limits for TX and for when to change to next band when tuning up or down.
    uint32_t    edge_upper;
    uint32_t    vfo_A_last;     // remember last VFO dial setting in this band
    uint32_t    vfo_B_last;
    uint8_t     VFO_AB_Active;  // Flag to track which has focus. Usesd in RX.  Used in TX for split
    uint8_t     mode_A;         // CW, LSB, USB, DATA.  
    uint8_t     mode_B;         // CW, LSB, USB, DATA.  
    uint8_t     filter;         // index to Bandwidth selection for this band.
    uint8_t     band_num;       // generally the same as the index but could be used to sort bands differently and skip bands
    uint8_t     tune_step;      // last step rate on this band.  Index to Tune step table 
    uint8_t     agc_mode;       // index to group of AGC settings in another table
    uint8_t     split;          // split active or not. 0 is off
    uint8_t     RIT_en;         // RIT active. 
    int32_t     RIT;            // RIT value in Hz pos or neg from current VFO
    uint8_t     XIT_en;         // XIT active
    int32_t     XIT;            // XIT value in Hz pos or neg from current VFO
    uint8_t     ATU;            // enable ATU or not.
    uint8_t     ant_sw;         // antenna selector switch.
    uint8_t     preselector;    // preselector band set value.
    uint8_t     attenuator;     // 0 = button ON/OFF
    uint8_t     attenuator_byp; // 0 = bypass, 1 is attenuation hardware active
    uint8_t     attenuator_dB;  // 0 is attenuation value to set.
    uint8_t     preamp;         // 0-off, 1 is level 2, level 2
    uint8_t     xvtr_en;        // use Tranverter Table or not.  Want to be able to edit while disabled so this is sperate from index.
    uint8_t     xvtr_num;       // index to Transverter Table.
    int16_t     sp_ref_lvl;     // per band spectrum reference level.  Overides the spectrum module level by copying this value into it.
};

struct Transverter {
    char        xvtr_name[20];
    uint8_t     xvtr_en;
    uint8_t     xvtr_num;
    uint16_t    RF;
    uint16_t    IF;
    float       pwr_set;
    float       offset;  
    uint8_t     decoder_pattern;
};

struct Standard_Button {
    uint8_t  enabled;       // ON - enabled. Enable or disable this button. UserInput() will look for matched coordinate and skip if disabled.                            
    uint8_t  show;          // ON= Show key. 0 = Hide key. Used to Hide a button without disabling it. Useful for swapping panels of buttons.
    uint8_t  Panelnum;      // Panel to display this button in, if any
    uint16_t bx;            // coordinates used by both touch and display systems
	uint16_t by;
	uint16_t bw;
	uint16_t bh;            // height 
	uint16_t br;            // radius of roundRect corners
    uint16_t outline_color; // used for button outline color
    uint16_t txtclr;        // used for button text color
    uint16_t on_color;      // fill color when button is ON state
    uint16_t off_color;     // fill color when button is OFF state
    uint16_t padx;          // # of pixels to pad label text horizontally shifting right to center the text 
    uint16_t pady;          // # of pixels to pad label text vertically shifting text down to center the text 
    char     label[20];     // Text to display for the button label. Use padx and pady to center..
};

 //UserInput() will look for a matching touch event coordinate if SHOW = ON. Will skip if SHOW of OFF and keep looking for coordinate matches.  
 //DisplayXXX() also looks at SHOW to decide whether to draw something. 
 //
struct Label {
    uint8_t  enabled;       // Not used for Labels today. Can be used for state tracking. 
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
};

struct User_Settings {
    char        configset_name[20]; // friendly anme for this record
    uint16_t    sp_preset;          // Sets the Spectrum module layout preset
    uint8_t     main_page;          // stores index to page settings table
    uint8_t     band_popup;         // index to band selection pop-up page layout preference
    uint8_t     usrcfgpage_1;       // index to user configuration page layout
    uint8_t     usrcfgpage_2;       // index to user configuration page layout
    uint8_t     usrcfgpage_3;       // index to user configuration page layout
    uint8_t     last_band;          // index into band memory table to recall last settings  - this might get moved out later
    uint8_t     mute;               // Current status of Mute 
    uint8_t     mic_input_en;       // mic on or off
    float       mic_Gain_level;     // 0-100 Mic gain level in percent. Applied to LineOut_TX level.
    uint8_t     lineIn_level;       // codec line in max level. Range 0 to 15.  0 => 3.12Vp-p, 15 => 0.24Vp-p sensitivity. Set this to max allowed.
    uint8_t     rfGain_en;          // Shows when RF gain setting value is displayed or not on meter face
    uint8_t     rfGain;             // 0-100 (% of lineIn level). Last used line-in setting on this band.
    uint8_t     spkr_en;            // 0 is disable or mute. 1= mono, 2= stereo. 3= sound effect 1 and so on. 255 is ignore and use current setting
    uint8_t     afGain_en;          // Shows when AF gain setting value is displayed or not on meter face
    uint8_t     afGain;             // 0-100 Last setting for unmute or power on (When we store in EEPROM). scaled to 0.0 to 1.0.
    uint8_t     lineOut_RX;         // Output Level when in Receive. (Line out to headphone jack) off = 0. Level = Range 13 to 31.  13 => 3.16Vp-p, 31=> 1.16Vp-p
    uint8_t     lineOut_TX;         // Output level when in Transmit (LineOut to TX board)
    uint8_t     enet_enabled;       // Allow initialization and operation of ethernet hardware
    uint8_t     enet_output;        // Allow ethernet data to flow (if enet is enabled)
    uint8_t     nb_en;              // Noise Blanker mode.  0 is off.  1+ is mode
    uint8_t     nb_level;           // 0 to NB_SET_NUM records in the table
    uint8_t     nr_en;              // Noise Reduction.  0 is off.  1+ is mode
    uint8_t     spot;               // Spot  0 is off.  1+ is mode
    float       rogerBeep_Vol;      // feedback beeps level  Range 0.0 to 1.0. 
    uint16_t    pitch;              // Pitch  0 is off.  1+ is mode
    uint8_t     notch;              // Notch mode.  0 is off.  1+ is mode
    uint8_t     xmit;               // xmit state.  0 is off.  1+ is mode
    uint8_t     fine;               // Fine tune state.  0 is off.  1+ is mode
    uint8_t     VFO_last;           // Track the last known state of the VFO A/B feature - either on A or B
    uint8_t     default_MF_client;  // The default "client" assignment for the the MF Knob.
    uint8_t     encoder1_client;    // The "client" action for one of the encoder knobs - Set to 0 if not encoder is wired up
    uint8_t     encoder2_client;    // The "client" action for one of the encoder knobs - Set to 0 if not encoder is wired up
    uint8_t     encoder3_client;    // The "client" action for one of the encoder knobs - Set to 0 if not encoder is wired up
    uint8_t     zoom_level;         // 0 - 2.  Zoom level memory.  x1, x2, x4 
    uint8_t     pan_state;          // 0 = OFF, 1 = ON
    uint8_t     pan_level;          // 0-100 converts to pan range of -0.50 to 0.50 for the pan memory.  0  is centered.
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
};

struct AGC {
    char        agc_name[10];
    uint8_t     agc_maxGain;
    uint8_t     agc_response;
    uint8_t     agc_hardlimit;    // autoVolumeControl(maxGain, response, hardLimit, threshold, attack, decay);
    float       agc_threshold;
    float       agc_attack;
    float       agc_decay;
};

// Noise Blanker Settings
struct NB {
    char            nb_name[10];        // A friendly name for display
    float32_t       nb_threshold;       // threshold recommended to be between 1.5 and 20, closer to 3 maybe best.
    uint16_t        nb_nAnticipation;   // nAnticipation is 1 to 125
    uint16_t        nb_decay;           // Decay is 1 to 10.
};

// Zoom Level Info table
struct Zoom_Lvl {
    char            zoom_name[10];        // A friendly name for display  x1, x2 or x4, etc
    uint8_t         zoom_factor;         // Factor equates to 1, 2 ,or 4
};

// per-band settings for common user adjustments that are band dependent. The index is the band number.
//struct Spectrum_Settings {
//    uint16_t    Ref_level;      // Spectrum common adjustments due to noise level and scale/gain choices during operation.
//    uint16_t    Span;
//    float       scale;
//};

struct Filter_Settings {
    char        Filter_name[12];   // display name for UI
    uint16_t    Width;             // bandwidth in HZ
    char        units[4];
    uint8_t     filter_type;       // preferred mode when enabled (future use)
};

struct TuneSteps {
    char        ts_name[12];    // display name for UI
    char        ts_units[4];    // units for display HZ or KHz
    uint16_t    step;           // bandwidth in HZ
    uint8_t     pref_mode;      // preferred mode when enabled (future use)
};

struct Modes_List {
    uint8_t     mode_num;
    char        mode_label[8];
    uint16_t    Width;             // bandwidth in HZ - look up matching width in Filter table when changing modes
};

enum Label_List {BAND_LBL, MODE_LBL, FILTER_LBL, RATE_LBL, AGC_LBL, ANT_LBL, ATTEN_LBL, PREAMP_LBL, ATU_LBL, RIT_LBL, XIT_LBL, FINE_LBL, NB_LBL, NR_LBL, NOTCH_LBL, SPLIT_LBL, MUTE_LBL, XMIT_LBL, XVTR_LBL, REFLVL_LBL, SPOT_LBL, ZOOM_LBL};
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
#define REFLVL_LBL  19      // not implemented yet
#define SPOT_LBL    20      // not implemented yet
#define ZOOM_LBL    21      // not implemented yet
*/

#endif //_SDR_RA8875_H_
