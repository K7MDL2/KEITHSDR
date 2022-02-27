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

#define USE_RA8875          // Turns on support for RA8875 LCD TOcuhscreen Display with FT5204 Touch controller
                            // When commented out it will default to the RA8876 controller and FT5206 touch controller
                            // DEPENDS on correct display controller type conencted via 4-wire SPI bus.
                            // UN-comment this line to use RA8876  *** AND in the Spectrum_RA887x.h ***

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

//#define si5351_CORRECTION  0  // frequency correction for the si5351A PLL board crystal or TXCO.
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

//#define I2C_ENCODERS      // I2C connected encoders. Here we are using the RGB LED version from
                            // GitHub https://github.com/Fattoresaimon/ArduinoDuPPaLib
                            // Hardware verson 2.1, Arduino library version 1.40.

//#define MECH_ENCODERS     // Use regular mechanical (non-I2C) connected encoders.  If this is defined and there are no encoders connected,
                            // *** AND *** ENET is defined, you will get reboot right after enet initialization comletes.

//#define USE_ENET_PROFILE  // This is inserted here to conveniently turn on ethernet profile for me using 1 setting.
#ifdef USE_ENET_PROFILE     // Depends on ENET
    #define ENET
#endif // USE_ENET_PROFILE

//#define REMOTE_OPS        // Turn on Remote_Ops ethernet write feature for remote control head dev work.
#ifdef REMOTE_OPS           // Depends on ENET
    #define ENET
#endif  // REMOTE_OPS

//#define TEST_SINEWAVE_SIG       // Turns on sinewave generators for display in the spectrum FFT only.

#define SPECTRUM_PRESET  0        // The spectrum layout record default value.
                                  // 0 is recommended for full screen.
                                  // 5 for smaller 2 window size.

//#define PANADAPTER          // Optimize some settings for panadapter use.  VFO becomes fixed LO at designated frequency
                            // Comment this ouot to dispable all PANADAPTER settings.

#define PANADAPTER_LO   8215000 // Frequency of radio's IF output in Hz. 
                                // For a K3 it is 8215Khz for DATA A mode, 8212.5KHz if USB/LSB
                                // Enabled only when the PANADAPTER define is active. Can be left uncommented.

#define PANADAPTER_MODE_OFFSET_DATA 0   // This is the offset added by the radio in certain modes
                                        // It is usually the Center frequency of the filter
                                        // Enabled only when the PANADAPTER define is active. Can be left uncommented.

//#define PANADAPTER_INVERT // When uncommented, this inverts the tuning direction seen on screen.
                            // Most radio IFs are inverted, though it can change depending on frequency
                            // Enabled only when the PANADAPTER define is active. Can be left uncommented.

//#define ALL_CAT           // Include support for reading radio info for PANADAPTER mode CAT control over serial port or other means
                            // Intended for use in combination with PANADAPTER mode.  
                            // Defining this without the PANADAPTER mode enabled may cause odd effects.
                            // DEPENDS on PANADAPTER mode


#define SCREEN_ROTATION   0 // 0 is normal horizontal landscape orientation  For RA8876 only at this point.
                            // 2 is 180 flip.  This will affect the touch orientation so that must be set to match your display
                            // The 7" RA8876 display has a better off-center viewing angle when horizantal when the touch panel ribbon is at the top.  This requires the touch to be rotated.
                            // The rotation will be 0, touch rotation will be "defined"
                            // When the 7" is vertically mounted the ribbon should be down with Touch Rotation "undefined".
//#define TOUCH_ROTATION    // if not defined (commented out) there is no correction.                        
                            // if defined (uncommented) correction is applied flipping the coordinates top to bottom.

#define VFO_MULT      4     // 4x for QRP-Labs RX, 2x for NT7V QSE/QSD board

#define PTT_INPUT    35     // GPIO digital input pin number for external PTT.  Typically LO (GND) = TX, HI = RX.

#define PTT_OUT1     36     // GPIO digital output pin number for external PTT.  Typically LO (GND) = TX, HI = RX.

#define AUDIOBOOST   (1.0f) // Audio output amp gain.
                            // 0/0 - 32767.0.   0.0 theoretically shuts off flow so should not be used.  
                            // 1.0f is pass through (no gain or loss)
                            // 0 to < 1.0f is attenuation level
                            // > 1.0f is positive gain.  Too high and you can get clipping.  
                            // See AudioAmplifer doc at https://www.pjrc.com/teensy/gui/index.html?info=AudioAmplifier

//#define USE_MIDI  	// Experimental dev work to use Teensy SDR controls to send out MIDI events over USB

#define PHASE_CHANGE_ON     // Switch manual methos fo I2S inut phase correction.  
                            // When enabled use long press on ANT button to cycle thorugh 3 possible
                            // solutons to remove mirror image aka Twin Peaks problem
                            // When commented out use AutoSDRpreProcessor auto correction 

// K7MDL specific Build Configuration rolled up into one #define for easier testing in multiple configurations
//#define K7MDL_BUILD

#ifdef K7MDL_BUILD 
    #ifdef USE_RA8875 
      //#undef USE_RA8875            // UN-comment this line to use RA8876  *** AND in the Spectrum_RA887x.h ***
    #endif
    #ifndef USE_RA8875
      #define TOUCH_ROTATION          // Rotate for the RA8876 for better view angle and no touch coordnmate correction required.
    #endif
    #define I2C_ENCODERS            // Use I2C connected encoders.
    //#define MECH_ENCODERS            // Use regular (or non-I2C) connected encoders.  If this is defined and there are no encoders connected,
                                      // *** AND *** ENET is defined, you will get reboot right after enet initialization completes.
    //#define OCXO_10MHZ              // Switch to etherkits library and set to use ext ref input at 10MHz
    //#define K7MDL_OCXO              // use the si5351 C board with 10Mhz OCXO
    //#define si5351_TCXO               // Set load cap to 0pF for TCXO
    #ifdef si5351_TCXO                // etherkits TCXO Si5351A board (25MHz)
      #define si5351_CORRECTION 0     // for TCXO whcih has been adjusted or corrected in other ways
    #else      
      #define si5351_CORRECTION 1726  // for standard crystal PLL +1726 for my 4.3" with cheap crystal Si5351a at 80F ambient
    #endif
    #define si5351_XTAL_25MHZ         // Choose 25MHz tcxo or crystal, else 27Mhz is default
    #ifdef VFO_MULT 
      #undef VFO_MULT                 // undefine so we can redefine it without error msg
      #define VFO_MULT            2   // 2 for NT7V board, 4 for QRP labs RX board
    #endif
    #undef USE_DHCP                
    #define USE_DHCP                  // UNCOMMENT this for static IP  
    //#define USE_ENET_PROFILE          // UNCOMMENT to use ENET
    #ifdef USE_ENET_PROFILE
      #define ENET
    #endif
    //#define REMOTE_OPS              // Experimentatl.  Can dump out FFT data
    //#define SV1AFN_BPF              // Use the BPF board
    //#define DIG_STEP_ATT            // Use the step atten usually the PE4302 board
    //#define PANADAPTER              // Enable panadapter mode
    #ifdef PANADAPTER
      #define ALL_CAT                 // Band decoder library - reads radio info only for many radios by many means, voltage, serial, bcd input
      //#define FT817_CAT             // FT-817 control library - does full control and monitor for the FT-817
      //#define PANADAPTER_INVERT     // Invert spectrum for inverted IF tuning direction
    #endif
    #undef AUDIOBOOST
    #define AUDIOBOOST   (1.0f)
    // Experimental features - use only one or none!
    //#define USE_FREQ_SHIFTER // Experimental to shift the FFT spectrum up away from DC
    //#define USE_FFT_LO_MIXER    // Experimental to shift the FFT spectrum up away from DC
#endif  // K7MDL_BUILD
//
//--------------------------USER HARDWARE AND PREFERENCES---------------------------------------
//
// ---------------------------------------ENCODERS----------------------------------------------
// 
// Choose your actual pin assignments for any you may have.
// VFO Encoder (not I2C)
#define VFO_ENC_PIN_A   4
#define VFO_ENC_PIN_B   5

#define VFO_PPR 6  // for VFO A/B Tuning encoder. This scales the PPR to account for high vs low PPR encoders.  600ppr is very fast at 1Hz steps, worse at 10Khz!
// I find a value of 60 works good for 600ppr. 30 should be good for 300ppr, 1 or 2 for typical 24-36 ppr encoders. Best to use even numbers above 1. 

// I2C connected encoders use this this pin to signal interrupts
// Knob assignments are the user_settings database                                                                                                                ither I2C or GPIO connected.  I no encoder is used, comment out I2C_ENCODER to prevent hangs on I2C comms
#ifdef I2C_ENCODERS
  #define I2C_INT_PIN     29
  #define MF_ENC_ADDR     (0x61)  	/* Address 0x61 only - Jumpers A0, A5 and A6 are soldered.*/
  #define ENC2_ADDR       (0x62)  	/* Address 0x62 only - Jumpers A1, A5 and A6 are soldered.*/
  //#define ENC3_ADDR     (0x63)  	/* Address 0x63 only - Jumpers A0, A1, A5 and A6 are soldered.*/     
#else
  #define MF_ENC_PIN_A    40   // list pins for any non I2C aux encoders.
  #define MF_ENC_PIN_B    39
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
//
//------------------------------------  Ethernet UDP messaging section --------------------------
//
#ifdef ENET
    #include <NativeEthernet.h>
    #include <NativeEthernetUdp.h>
    
    // Choose or create your desired time zone offset or use 0 for UTC.
    #define MYTZ -8
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

// -----------------------------   PANADAPTER CAT INTERFACES  -------------------------------------------
//
#ifdef  FT817_CAT
  #define HWSERIAL Serial1 // Teensy hardware Serial or USB Serial port. Set this to the hardware serial port you wish to use
  #include <ft817.h>
  #include "SDR_CAT.h"
#endif  // FT817_CAT

#ifdef ALL_CAT
  #include "SDR_CAT.h"
#endif

#endif //_RADIOCONFIG_H_
