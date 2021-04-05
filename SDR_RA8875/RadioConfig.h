#ifndef _RADIOCONFIG_H_
#define _RADIOCONFIG_H_

//#include "SDR_RA8875.h"
//
//      RadioConfig.h
//
//  This file is the central warehouse for operational parameters.  
//  Key to this operation is are stuctures that hold the majority of current band settings
//  Other key parameters are current settings values which may be modified from the last used value
//  in the structure.  These are usually things like VFO dial settings and should be stored in 
//  EEPROM at some point like band changes when the dial has stopped for a certain length of time
//
//-----------------------BUILD TIME CONFIGURATION SECTION -------------------------------------
// Individual #defines are here to choose what code features will be compiled into your build.
// Your connected hardware is the primary reason to change these.
// Compiling in code that talks to an I2C device for example will hang if the device is not present.

#define USE_RA8875        // Turns on support for RA8875 LCD TOcuhscreen Display with FT5204 Touch controller
                            // When commented out it will default to the RA8876 controller and FT5206 touch controller
                            // DEPENDS on correct display controller type conencted via 4-wire SPI bus.

//#define OCXO_10MHZ        // Uncomment this line to use a different library that supports External CLKIN for si5351C version PLL boards.
                            // DEPENDS on si5351C version PLL board.  Otherwise uses the standard issue Si5351A PLL

//#define si5351_TCXO       // If your Si5351 PLL module has a TCXO then turn off the load capacitors.
                            // DEPENDS on a modified si5351mcu mod by library by K7MDL.
                            // Alternative is to use the Etherkit library or Adafruit or other. 

#define si5351_XTAL_25MHZ   // This depends on what your PLL uses.
                            // Uncomment this if your Si5351A crysal is 25MHz
                            // Commented out it will use 27MHz in VFO.h
                            // This is ignored if OCXO_10MHz is defined.
                            // DEPENDS on your crystal being 25Mhz

#define si5351_CORRECTION  0  // frequency correction for the si5351A PLL board crystal or TXCO.
                            // The 5351mcu library uses Hz offset, etherkit and others use ppb.

//#define DIG_STEP_ATT        // PE4302 Digital step attenuator. Harmless to leave this defined as long as it is not in the I2C port expander
                            // DEPENDS on a PE4302 connected for variable attenuation
                            // MAY DEPEND on the Attenuation relay on a SV1AFN BPF board being turned on.
                            //   You can use this without relays or the BPF board

//#define SV1AFN_BPF          // Bandpass filter via I2C port expander.  Will hang if you do not have the port expander.
                            // DEPENDS on SV1AFN BPF board connected via a MCP23017 I2C port expander.

//#define ENET              // Turn off or on ethernet features and hardware. Teeny4.1 has ethernet built in but needs an external connector.
                            // It wants to find a valid link status (Cable connected) or may not be happy.
                            // Configured to use DHCP right now.
                            // DEPENDS on ethernet jack and cable connected
                            // Choose to use DHCP or a static IP address for the SDR

#define USE_DHCP            // Use DHCP rather then define a static IP address (which is Defined further below)

// #define I2C_LCD          // Turn of or on the optional I2C character LCD display.   
                            // Look below to set the i2c address of the display and
                            // the size of the display.
                            // DEPENDS on LCD I2C hardware connected or it will hang on I2C comms timeouts

//#define I2C_ENCODERS     // I2C connected encoders. Here we are using the RGB LED version from
                            // GitHub https://github.com/Fattoresaimon/ArduinoDuPPaLib
                            // Hardware verson 2.1, Arduino library version 1.40.

//#define USE_ENET_PROFILE  // This is inserted here to conveniently turn on ethernet profile for me using 1 setting.
#ifdef USE_ENET_PROFILE           // Depends on ENET
    #define ENET
#endif // USE_ENET_PROFILE

//#define REMOTE_OPS        // Turn on Remote_Ops ethernet write feature for remote control head dev work.
#ifdef REMOTE_OPS           // Depends on ENET
    #define ENET
#endif  // REMOTE_OPS

//#define TEST_SINEWAVE_SIG // Turns on sinewave generators for display in the spectrum FFT only.

#define SPECTRUM_PRESET  5  // The spectrum layout record default value.  

// K7MDL specific Build Configuration rolled up into one #define
//#define K7MDL_BUILD
//
#ifdef K7MDL_BUILD 
    #ifdef USE_RA8875 
     //#undef USE_RA8875            // UNcomment this line to use RA8876
    #endif
    #define I2C_ENCODERS
    //#define OCXO_10MHZ            // Switch to etherkits library and set to use ext ref input at 10MHz
    #define si5351_TCXO             // set load cap to 0pF for TCXO
    #define si5351_XTAL_25MHZ       // choose 25MHz tcxo or crystal, else 27Mhz
    #define USE_DHCP
    #define ENET
    #define USE_ENET_PROFILE
    //#define REMOTE_OPS
    #define SV1AFN_BPF              // use the BPF board
    #define DIG_STEP_ATT            // USe the step atten
#endif  // K7MDL_BUILD
//
//--------------------------USER HARDWARE AND PREFERENCES---------------------------------------
//
// ---------------------------------------ENCODERS----------------------------------------------
// 
// Choose your actual pin assignments for any you may have.
// VFO Encoder (not I2C)
#define VFO_ENC_PIN_A 4
#define VFO_ENC_PIN_B 5

#define VFO_PPR 6  // for VFO A/B Tuning encoder. This scales the PPR to account for high vs low PPR encoders.  600ppr is very fast at 1Hz steps, worse at 10Khz!
// I find a value of 60 works good for 600ppr. 30 should be good for 300ppr, 1 or 2 for typical 24-36 ppr encoders. Best to use even numbers above 1. 

// I2C connected encoders use this this pin to signal interrupts
// Knob assignments are the user_settings database                                                                                                                ither I2C or GPIO connected.  I no encoder is used, comment out I2C_ENCODER to prevent hangs on I2C comms
#ifdef I2C_ENCODERS
  #define I2C_INT_PIN   29
  #define MF_ENC_ADDR  (0x61)  	/* Address 0x61 only - Jumpers A0, A5 and A6 are soldered.*/
  #define ENC2_ADDR    (0x62)  	/* Address 0x62 only - Jumpers A1, A5 and A6 are soldered.*/
  //#define ENC3_ADDR    (0x63)  	/* Address 0x63 only - Jumpers A0, A1, A5 and A6 are soldered.*/     
#else
  #define MF_ENC_PIN_A 40   // list pins for any non I2C aux encoders.
  #define MF_ENC_PIN_B 39
#endif // I2C_ENCODERS

// -------------------------  PE4302 6 bit Digital Step Attenuator -----------------------------
//      Digital step attenuator.  0-31.5dB in 0.5dB steps. Connected via I2C port expander.
//      Could use the 3 left over pins on the MCP23017 I2C port expander servicing the SV1AFN preselector module.
//      For now using Teensy 4.1 pins 30-32.
//      
#ifdef DIG_STEP_ATT
  #define Atten_CLK       31
  #define Atten_DATA      32
  #define Atten_LE        30
#endif  // DIG_STEP_ATT

//
//--------------------------------- RA8875 LCD TOUCH DISPLAY INIT & PINS --------------------------
//

#ifdef USE_RA8875
  #define  SCREEN_WIDTH      800 
  #define  SCREEN_HEIGHT     480
  #define  RA8875_INT        14   //any pin
  #define  RA8875_CS         10   //any digital pin
  #define  RA8875_RESET      9    //any pin or nothing!
  #define  MAXTOUCHLIMIT     3    //1...5  using 3 for 3 finger swipes, otherwise 2 for pinches or just 1 for touch
  #include <SPI.h>                // included with Arduino
  //#include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
  //#include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
  #include <RA8875.h>           // internal Teensy library with ft5206 cap touch enabled in user_setting.h
#else // If RA8876 is not used then assume the RA8876_t3 1024x600 is.
//
//
//
//--------------------------------- RA8876 LCD TOUCH DISPLAY INIT & PINS --------------------------
//
#define USE_RA8876_t3
//
#define  SCREEN_WIDTH      1024 
#define  SCREEN_HEIGHT     600
//#include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
//#include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
#include <RA8876_t3.h>           // Github
#include <FT5206.h>
#define  CTP_INT           14   // Use an interrupt capable pin such as pin 2 (any pin on a Teensy)
#define  RA8876_CS         10   //any digital pin
#define  RA8876_RESET      9    //any pin or nothing!
#define  MAXTOUCHLIMIT     3    //1...5  using 3 for 3 finger swipes, otherwise 2 for pinches or just 1 for touch

// From RA8875/_settings/RA8875ColorPresets.h
// Colors preset (RGB565)
const uint16_t	RA8875_BLACK            = 0x0000;
const uint16_t 	RA8875_WHITE            = 0xFFFF;
const uint16_t	RA8875_RED              = 0xF800;
const uint16_t	RA8875_GREEN            = 0x07E0;
const uint16_t	RA8875_BLUE             = 0x001F;
const uint16_t 	RA8875_CYAN             = RA8875_GREEN | RA8875_BLUE;//0x07FF;
const uint16_t 	RA8875_MAGENTA          = 0xF81F;
const uint16_t 	RA8875_YELLOW           = RA8875_RED | RA8875_GREEN;//0xFFE0;  
const uint16_t 	RA8875_LIGHT_GREY 		  = 0xB5B2; // the experimentalist
const uint16_t 	RA8875_LIGHT_ORANGE 	  = 0xFC80; // the experimentalist
const uint16_t 	RA8875_DARK_ORANGE 		  = 0xFB60; // the experimentalist
const uint16_t 	RA8875_PINK 			      = 0xFCFF; // M.Sandercock
const uint16_t 	RA8875_PURPLE 			    = 0x8017; // M.Sandercock
const uint16_t 	RA8875_GRAYSCALE 		    = 2113;//grayscale30 = RA8875_GRAYSCALE*30
#endif // USE_RA8876_t3
//
//
//------------------------------------  Ethernet UDP messaging section --------------------------
//
#ifdef ENET
    #include <NativeEthernet.h>
    #include <NativeEthernetUdp.h>
    
    // Choose or create your desired time zone offset or use 0 for UTC.
    #define MYTZ 0
    // here are some example values
    //  1 Central European Time
    //  0 UTC
    // -5 Eastern Standard Time (USA)
    // -4 Eastern Daylight Time (USA)
    // -8 Pacific Standard Time (USA)
    // -7 Pacific Daylight Time (USA)
    
    // If NOT using DHCP then assign a static IP address for the SDR   
    #ifndef USE_DHCP
    // The IP Address is ignored if using DHCP
    // IP address is defined in SDR_Network.cpp 
    #endif // USE_DHCP
    
    #define MY_LOCAL_PORTNUM 7943;     // local port the SDR will LISTEN on for any remote display/Desktop app

    #ifdef REMOTE_OPS
      // This is for later remote operation usage
      // IP address is defined in SDR_Network.cpp 
      #define MY_REMOTE_PORTNUM 7942;         // The destination port to SENDTO (a remote display or Desktop app)
    #endif // REMOTE_OPS
//
//------------------------------------ End of Ethernet UDP messaging section --------------------------
//
#endif  // ENET

// -----------------------------   I2C LCD Display  -------------------------------------------
//
//  An optional I2C character LCD Display can be connected to the Teensy as well, and used
//  to display just about any value you might need, such as RF or AF Gain, or signal strength.
//  Make sure all i2c devices have unique address or problems will ensue.
//
#ifdef I2C_LCD
  #define LCD_ADR     0x27
  #define LCD_COL     20
  #define LCD_LINES   2
#endif //I2C_LCD

#endif //_RADIOCONFIG_H_
