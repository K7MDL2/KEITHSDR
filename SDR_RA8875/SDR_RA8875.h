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
#include <T4_PowerButton.h>     // https://github.com/FrankBoesing/T4_PowerButton for the FlexInfo() and Hardfault reporting tools
//#include "avr/pgmspace.h"
#include <SPI.h>                // included with Arduino
#include <SD.h>                 // included with Arduino
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
#include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
#include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
#include <TimeLib.h>
//#include <glcdfont.c>
//#include <ILI9341_fonts.h>
//#include <FiraCode_mono_14.c>   //minipixel
//#include <fonts/FiraCode_mono_16.c>
//#include <fonts/FiraCode_mono_18.c>
//#include <fonts/FiraCode_mono_20.c>
//#include <fonts/FiraCode_mono_24.c>
//#include <fonts/FiraCode_mono_28.c>
//#include <fonts/FiraCode_mono_32.c>
//#include <fonts/FiraCode_mono_40.c>
// Below are local project files
#include "SDR_Network.h"        // for ethernet UDP remote control and monitoring
#include "Spectrum_RA8875.h"    // spectrum
//////#include "Hilbert.h"      // filter coefficients -DO NOT include this file here, included by bandwidth.cpp directly
#include "Vfo.h"
#include "Display.h"
#include "Tuner.h"
#include "Mode.h"
#include "Smeter.h"
#include "CW_Tune.h"
#include "Quadrature.h"
#include "Controls.h"
#include "UserInput.h"          // include after Spectrum_RA8875.h and Display.h
#include "Bandwidth2.h"
#include "SD_Card.h"

///////////////////////Set up global variables for Frequency, mode, bandwidth, step
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

// ------------------------  OPERTIONAL PARAMETER STORAGE --------------------------------------
//
//  Most users will not normally mess around in this section but you can edit some of the table data to refine the default to your liking.  
//  Contained here are per-band settings, User Profile settings (globals), Font type and sizes
//  buttons and indicator colors, button an label locations and more. 
//  Pretty much every global variable that controls a setting is here in a table of some sort. 
//
#define CW          0
#define CW_REV      6
#define LSB         1     
#define USB         2
#define DATA        3
#define DATA_REV    7
#define FM          4
#define AM          5

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
#define myDARKGREY  31727u
// From RA8876_t3/RA8876Registers.h
#define BLACK		    0x0000
#define WHITE		    0xffff
#define RED		  	  0xf800
#define LIGHTRED	  0xfc10
#define CRIMSON		  0x8000
#define GREEN		    0x07e0
#define PALEGREEN	  0x87f0
#define DARKGREEN	  0x0400
#define BLUE		    0x001f
#define LIGHTBLUE	  0x051f
#define SKYBLUE		  0x841f
#define DARKBLUE	  0x0010
#define YELLOW		  0xffe0
#define LIGHTYELLOW	0xfff0
#define DARKYELLOW	0x8400 // mustard
#define CYAN		    0x07ff
#define LIGHTCYAN	  0x87ff
#define DARKCYAN	  0x0410
#define MAGENTA		  0xf81f
#define VIOLET		  0xfc1f
#define BLUEVIOLET	0x8010
#define ORCHID		  0xA145 
// Otehr sources of RGB coplpr definitions
#define NAVY        0x000F
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF
#define ORANGE      0xFD20
#define GREENYELLOW 0xAFE5
#define PINK        0xF81F

// This group defines thw number of records in each structure
#define MODES_NUM   8
#define FREQ_DISP_NUM  4
#define BANDS       12
#define XVTRS       12
#define TS_STEPS    6
#define FILTER      9
#define AGC_SET_NUM 4
#define NB_SET_NUM  7
#define USER_SETTINGS_NUM 3
#define LABEL_NUM   20      // number of labels in the table
#define STD_BTN_NUM 33      // number of buttons in the table

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
#define FINE_BTN    21      // Fine and coarse rate
#define SPLIT_BTN   22      // ON/OFF
#define DISPLAY_BTN 23      // Invoke PopUp (On), close popup (Off)
#define VFO_AB_BTN  24      // ON/(A)/OFF(B)   Will go away soon. 
//Panel 5
#define ENET_BTN    25      // turn on and off the enet data output (does not enable/disable the enet hardware) 
#define XVTR_BTN    26      // not implemented yet
#define RFGAIN_BTN  27      // Sets digital RF level
#define REFLVL_BTN  28      // Sets the Spectrum Noise floor.
#define AFGAIN_BTN  29     // Sets digital AF level

// Not in a Panel
#define UTCTIME_BTN 30      // NTP UTC time when ethernet (and internet) is available 
#define SMETER_BTN  31      // Box for the Smeter.  Can be a meter for any use.  Can touch the meter to configure maybe
#define SPECTUNE_BTN 32     // Convertes a touch in the spectrum window to a frequency to tune too.

// The #define button numbers act as the ID of possible owners of MF knob services
#define MFTUNE      50      // Fake button so the MF knob can tune the VFO since there is no button
                            // Make sure this does have the same value as any buttom #defne value. 50 is safe.
#define MFNONE      0       // No active MF knob client.  0 is safe.
#define myDARKGREY  31727u
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
    uint8_t     attenuator;     // 0 = bypass, >0 is attenuation value to set.
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
    uint8_t     last_band;          // index into band memeory table to recall last settings  - this might get moved out later
    uint8_t     mute;               // Current status of Mute 
    uint8_t     mic_input_en;       // mic on or off
    float       mic_Gain_last;      // last used mic gain on this band
    uint8_t     lineIn_level;       // codec line in max level. Range 0 to 15.  0 => 3.12Vp-p, 15 => 0.24Vp-p sensitivity. Set this to max allowed.
    uint8_t     rfGain_en;          // 0-100 (% of lineIn level). Last used line-in setting on this band.
    uint8_t     rfGain;             // 0-100 (% of lineIn level). Last used line-in setting on this band.
    uint8_t     spkr_en;            // 0 is disable or mute. 1= mono, 2= stereo. 3= sound effect 1 and so on. 255 is ignore and use current setting
    uint8_t     afGain_en;          // 0-100 Last setting for unmute or power on (When we store in EEPROM). scaled to 0.0 to 1.0.
    uint8_t     afGain;             // 0-100 Last setting for unmute or power on (When we store in EEPROM). scaled to 0.0 to 1.0.
    uint8_t     lineOut_level;      // line out off = 0. Level = Range 13 to 31.  13 => 3.16Vp-p, 31=> 1.16Vp-p
    uint8_t     lineOut_Vol_last;   // last line out setting used on this band. Intended for data mode line level separate from speaker volume
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
    float32_t      nb_threshold;       // threshold recommended to be between 1.5 and 20, closer to 3 maybe best.
    uint16_t        nb_nAnticipation;   // nAnticipation is 1 to 125
    uint16_t        nb_decay;           // Decay is 1 to 10.
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
    uint8_t  mode_num;
    char   mode_label[8];
};

enum Label_List {BAND_LBL, MODE_LBL, FILTER_LBL, RATE_LBL, AGC_LBL, ANT_LBL, ATTEN_LBL, PREAMP_LBL, ATU_LBL, RIT_LBL, XIT_LBL, FINE_LBL, NB_LBL, NR_LBL, NOTCH_LBL, SPLIT_LBL, MUTE_LBL, XMIT_LBL, XVTR_LBL, REFLVL_LBL, SPOT_LBL};
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
*/

// Simple ways to designate functions to run out of fast or slower memory to help save RAM
#define HOT FASTRUN    __attribute__((hot))
#define COLD FLASHMEM  __attribute__((cold))

#endif //_SDR_RA8875_H_
