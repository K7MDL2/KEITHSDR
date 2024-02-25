#ifndef _RADIOCONFIG_H_
#define _RADIOCONFIG_H_

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

/*******************************  !!!!!!! IMPORTANT CONFIGURATION NOTE !!!!!!!!!! ****************************************

                        Teensy Configuration Requirement - Please READ!

To compile sucessfully you must configure the Teensy 4.0 or 4.1 USB mode.

For this build it must be set to SERIAL+MIDI+AUDIO.   

The previous builds were Dual Serial. No changing this mode wil result in a complie time error about AudioInputUSB not found

There is only 1 USB serial port active now so debug is globally shut off with '#define DEBUG' to allow the 
OmniRig V1 RS-HFIQ compatible CAT control from an external PC.


******************************  !!!!!!! ********************************** !!!!!! *****************************************/

#define BANNER "Teensy 4 SDR"  // Custom Startup Screen Text
#define CALLSIGN  "K7MDL CN88sf"   // Personalized Startup Screen Text

#define USE_RA8875          // Turns on support for RA8875 LCD Touchscreen Display with FT5204 Touch controller
                            // When commented out it will default to the RA8876 controller and FT5206 touch controller
                            // DEPENDS on correct display controller type connected via 4-wire SPI bus.
                            // For the RA8875 (only) be sure to edit the file
                            //   For Arduino IDE <2.0 C:\Program Files (x86)\Arduino\hardware\teensy\avr\libraries\RA8875\_settings\RA8875UserSettings.h
                            //   For IDE V2.X %AppData%\Local\Arduino15\packages\teensy\hardware\avr\0.58.3\libraries\RA8875\_settings\RA8875UserSettings.h
                            // To enable touch by uncommenting this config item
                            //   #define USE_FT5206_TOUCH//capacitive touch screen
            
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
                            // The Si5351mcu library uses Hz offset, etherkit and others use ppb.

//#define PE4302            // PE4302 Digital step attenuator. 31.5dB in 0.5 steps, only using 1dB steps today
                            // Harmless to leave this defined as long as it is not connected via an I2C port expander
                            // DEPENDS on a PE4302 connected for variable attenuation
                            // MAY DEPEND on the Attenuation relay on a SV1AFN BPF board being turned on.
                            // You can use this without relays or the BPF board 
                            // The RF attenuator bypass relay is turned on and off.  Does not matter if there is a real relay connected or not. 

#define HARDWARE_ATT_SIZE  0   // Fixed attenuator size. 0 is OFF.  >0 == ON.   MAX = 99 (Future use!)
                            // This is used to correct the dBm scale on the spectrum 
                            // Can also fudge it to calibrate the spectrum until a more elegant solution is built

//#define SV1AFN_BPF        // Bandpass filter via I2C port expander.  Will hang if you do not have the port expander.
                            // DEPENDS on SV1AFN BPF board connected via a MCP23017 I2C port expander.

//#define ENET              // Turn off or on ethernet features and hardware. Teeny4.1 has ethernet built in but needs an external connector.
                            // It wants to find a valid link status (Cable connected) or may not be happy.
                            // Configured to use DHCP right now.
                            // DEPENDS on ethernet jack and cable connected
                            // Choose to use DHCP or a static IP address for the SDR

#define USE_DHCP            // Use DHCP rather then define a static IP address (which is Defined further below)

// #define I2C_LCD          // Turn on or off the optional I2C character LCD display.   
                            // Look below to set the i2c address and the size of the display.
                            // DEPENDS on LCD I2C hardware connected or it will hang on I2C comms timeouts

//#define I2C_ENCODERS      // I2C connected encoders. Here we are using the RGB LED version from
                            // GitHub https://github.com/Fattoresaimon/ArduinoDuPPaLib
                            // Hardware verson 2.1, Arduino library version 1.40.

//#define GPIO_ENCODERS     // Use regular (non-I2C) GPIO connected encoders.  If this is defined and there are no encoders connected,
                            // *** AND *** ENET is defined, you will get reboot right after enet initialization completes.
#ifdef GPIO_ENCODERS        // DEPENDS on I2C_ENCODERS for servicing functions, can disable all i2c encoder in config section below
    #define I2C_ENCODERS    
#endif // GPIO_ENCODERS
                            
//#define USE_ENET_PROFILE  // This is inserted here to conveniently turn on ethernet profile for me using 1 setting.
#ifdef USE_ENET_PROFILE     // Depends on ENET
    #define ENET
#endif // USE_ENET_PROFILE

//#define REMOTE_OPS        // Turn on Remote_Ops ethernet write feature for remote control head dev work.
#ifdef REMOTE_OPS           // Depends on ENET
    #define ENET
#endif  // REMOTE_OPS

//#define TEST_SINEWAVE_SIG   // Turns on sinewave generators for display in the spectrum FFT only.

//#define PANADAPTER          // Optimize some settings for panadapter use.  VFO becomes fixed LO at designated frequency
                              // Comment this out to dispable all PANADAPTER settings.

//#define PANADAPTER_LO   8215000 // Frequency of radio's IF output in Hz. 
                              // For a K3 it is 8215Khz for DATA A mode, 8212.5KHz if USB/LSB
                              // Enabled only when the PANADAPTER define is active. Can be left uncommented.

//#define PANADAPTER_MODE_OFFSET_DATA 0   // This is the offset added by the radio in certain modes
                              // It is usually the Center frequency of the filter
                              // Enabled only when the PANADAPTER define is active. Can be left uncommented.

//#define PANADAPTER_INVERT   // When uncommented, this inverts the tuning direction seen on screen.
                              // Most radio IFs are inverted, though it can change depending on frequency
                              // Enabled only when the PANADAPTER define is active. Can be left uncommented.

//#define PAN_CAT           // ** NOT likely working for now***  Include support for reading radio info for PANADAPTER mode CAT control over serial port or other means
                            // Intended for use in combination with PANADAPTER mode.  
                            // Defining this without the PANADAPTER mode enabled may cause odd effects.
                            // DEPENDS on PANADAPTER mode
                            // APPLIES to PANADAPTER usage only which is using older code that has not tested recently and may not work today

//#define USE_CAT_SER         // Use CAT port for Non-RSHFIQ hardware
                            // If USE_RS_HFIQ set then the CAT port is always enabled (for now)

//#define ALT_CAT_PORT        // Turn on to force the CAT comms to use the same port as Debug for single USB serial port configurations
                            // This allows operation with standard Serial+MIDI+Audio USB type though the 48KHz USB audio will not work right.
                            // APPLIES to all CAT comms regardless of RF hardware

//#define  FT817_CAT          // Use FT-0817 serial interface while in Panadapter mode.

#define SCREEN_ROTATION   0 // 0 is normal horizontal landscape orientation  For RA8876 only at this point.
                            // 2 is 180 flip.  This will affect the touch orientation so that must be set to match your display
                            // The 7" RA8876 display has a better off-center viewing angle when horizantal when the touch panel ribbon is at the top.  This requires the touch to be rotated.
                            // The rotation will be 0, touch rotation will be "defined"
                            // When the 7" is vertically mounted the ribbon should be down with Touch Rotation "undefined".

//#define TOUCH_ROTATION    // if not defined (commented out) there is no correction.                        
                            // if defined (uncommented) correction is applied flipping the coordinates top to bottom.

#define VFO_MULT          4 // 4x for QRP-Labs RX, 2x for NT7V QSE/QSD board

#define RIT_STEP_DEFAULT  1 // step size index from the tstep table.  normally index = 1 -> 10Hz.  

#define XIT_STEP_DEFAULT  1 // step size index from the tstep table.  normally index = 1 -> 10Hz.  

#define VARIABLE_FILTER   1  // when undefined or set to 0, the encoder will cycle through predefined filter widths same as the touch buttons do.  
                             // When active, the encoder (only) will be variable over the allowed range based on mode and change in various step rates according to freqwuncy.
                             //  50Hz < 1KHz, 100Hz 1-3KHz and 200Hz > 3KHz.  Max is 6.  FM is fixed and shows as N/A width.

#define AUDIOBOOST   (1.0f) // Audio output amp gain.
                            // 0/0 - 32767.0.   0.0 theoretically shuts off flow so should not be used.  
                            // 1.0f is pass through (no gain or loss)
                            // 0 to < 1.0f is attenuation level
                            // > 1.0f is positive gain.  Too high and you can get clipping.  
                            // See AudioAmplifer doc at https://www.pjrc.com/teensy/gui/index.html?info=AudioAmplifier

// Choose 1024, 2048, or 4096  for AUDIO audio output- usually defined in the main program
#define FFT_SIZE 4096

// --->>>> Enable one or more FFT pipelines for 2nd window, pan and zoom, if used
// Recommend all 3 be enabled for 3 zoom levels -  At least one must match FFT_SIZE
#define FFT_4096 
#define FFT_2048
#define FFT_1024

//-------------------------W7PUA Auto I2S phase correction-----------------
//
// Auto I2S alignment error correction (aka Twin Peaks problem)
// Requires 10K resistors on each SGTL5000 codec LineIn pin (L and R) to a common GPIO pin, pin 22 by default, defined in this file
//
//#define W7PUA_I2S_CORRECTION  
//
// Can leave these 2 defined, no effect on other things.
#define PIN_FOR_TP 22       // Teensy pin used for both Codec and I/O pin signal source methods (W7PUA I2S correction)
#define SIGNAL_HARDWARE TP_SIGNAL_IO_PIN  // 10Kohm is for RS-HFIQ which has 100ohm output impedance.  Other audio sources may vary.
// ---------------------------------------

// --------------- Motherboard/Protoboard version --------------------------
// Uncomment one of these to account for Touch interrupt differences, or
// if not using any of these boards, comment them all out to use the default old values
//#define SMALL_PCB_V1 // For the   small motherboard 4/18/2022
//#define V1_4_3_PCB   // For the V1 4.3" motherboard 4/18/2022
//#define V2_4_3_PCB   // For the V2 4.3" motherboard 4/21/2022
//#define V21_7_PCB    // For the V2.1 7" motherboard 12/30/2022
//#define V22_7_PCB    // For the V2.1 7" motherboard 12/30/2022

// ----------------- RS-HFIQ ---------------------------------------------------
//#define USE_RS_HFIQ             // Use the RS-HFIQ 5W SDR transceiver for the RF hardware. Connect via USB Host serial cable.
                                  // CAT port is always enabled for RS-HFIQ module.  
                                  // Set ALT_CAT_PORT for which USB port to use.
//#define NO_RSHFIQ_BLOCKING      // When combined with USE_RS-HFIQ, bypasses wait loops for queries from hardware allowing testing with no hardware connected
//#define RSHFIQ_CAL_OFFSET  (0)  // Fixed offset applied each RS-HFIQ startup to calibrate frequency.

// *****************************************************************************************
//    K7MDL specific Build Configuration rolled up into one #define for easier testing in multiple configurations

//#define K7MDL_BUILD  // This section overrides general settings above to permit fast switching between my current 2 SDR configs, RA8875 and RA8876

//******************************************************************************************

#ifdef K7MDL_BUILD  

    //#undef USE_RA8875   // Controls RA8875 or RA8876 build - Comment this line to choose RA8875, uncomment for RA8876
    
	#ifdef USE_RA8875   // My RA8875 4.3" specific build items
      #define I2C_ENCODERS            // Use I2C connected encoders. 
      #define V2_4_3_PCB              // For the V2 large 4.3" motherboard 4/2022
      #define USE_RS_HFIQ 
      //#ifdef  USE_RS_HFIQ  // use the RS-HFIQ 5W SDR tranciever for the RF hardware. Connect via USB Host serial cable.
        //#undef ALT_CAT_PORT
        //#define ALT_CAT_PORT
        #define RSHFIQ_CAL_OFFSET (-7500)
        //#define NO_RSHFIQ_BLOCKING
      //#endif
    #else // My RA8876 7" specific build items
      #undef  SCREEN_ROTATION
      #define SCREEN_ROTATION     2
      //#define GPIO_ENCODERS           // Requires I2C_Encoders library
      #define I2C_ENCODERS            // Use I2C connected encoders
      #define V22_7_PCB
      //#define V21_7_PCB
      
      //#define USE_RS_HFIQ 
      #ifndef USE_RS_HFIQ
        #define PE4302       // enable the step attenuator - using the ENC3 pins 33-35
        #define SV1AFN_BPF   // for 10-band BPF board
        #undef  VFO_MULT
        #define VFO_MULT           2    // 2 for NT7V board
        #define OCXO_10MHZ   // for Si5351C PLL board
        #define K7MDL_OCXO
        #define USE_CAT_SER  // For now USE_RSHFIQ will overide this so this can be left defined, no problem
        //#define ALT_CAT_PORT  // Use when only 1 USB serial port available (Such as when IO is set to Serial-MIDI-Audio)
      #else      // Use RS-HFIQ
        #undef ALT_CAT_PORT
        //#define ALT_CAT_PORT
        #define RSHFIQ_CAL_OFFSET (-7500) // Example: WWV tunes in high at 10000130Hz.  Subtract (130*100) or -13000.   75Hz high is -7500.
        //#define NO_RSHFIQ_BLOCKING
      #endif
    #endif

    // Config items common or NA to both builds        
    #define USE_DHCP                  // UNCOMMENT this for static IP  
    //#define USE_ENET_PROFILE          // UNCOMMENT to use ENET --AND-- the enet profile
    #ifdef  USE_ENET_PROFILE
      #define ENET
    #endif

    #define W7PUA_I2S_CORRECTION      // Attempt to resolve twin peak problem on SGTL5000 codec chip

    // Experimental features - use only one or none!
    //#define USE_FREQ_SHIFTER    // Experimental to shift the FFT spectrum up away from DC
    //#define USE_FFT_LO_MIXER    // Experimental to shift the FFT spectrum up away from DC
    //#define BETATEST            // audio memory external buffer test using FFT4096 
    //#define USE_MIDI  	        // Experimental dev work to use Teensy SDR controls to send out MIDI events over USB

    #define CESSB   // Beta test for new Weaver method CESSB.  Output is centers on 1350Hz so needs FC offset or other LO shifting
    // Choose one or none of these 3 below.  Selecting **none of these** will default to the CESSB_DIRECT method with Fc = +/-1350
    // DIRECT Mode will place the LO in the audio passband and it may be strong enough to be heard and even beat note against your test tone.
    //#define CESSB_MULTIPLY
    //#define CESSB_2xIQMIXER
    //#define CESSB_IQMIXER
    #define IQ_CORRECTION_WITH_CESSB  // turns on IQ correction to possibly help improve sideband image rejection due to hardware imbalances.

#endif  // K7MDL_BUILD


// *****************************************************************************************
//      BANDMAP - sets BAND ENABLE/DISABLE
//  Specify what bands should be skipped. Set to 0 to skip.  1 to enable
//  For RS-HFIQ (80M-10M) users 160M is disabled. Everything 6M and up is set to 0 unless you have a Xvtr active
// *****************************************************************************************
#ifdef USE_RS_HFIQ
  #define ENABLE_160M_BAND  0
#else
  #define ENABLE_160M_BAND  1
#endif
#define ENABLE_80M_BAND   1
#define ENABLE_60M_BAND   1
#define ENABLE_40M_BAND   1
#define ENABLE_30M_BAND   1
#define ENABLE_20M_BAND   1
#define ENABLE_17M_BAND   1
#define ENABLE_15M_BAND   1
#define ENABLE_12M_BAND   1
#define ENABLE_10M_BAND   1

// These are transverter bands common to all RF hardware that covers HF bands to 30MHz.
// The default IF is 10M band defined in the bandmem table in SDR_DATA.h
#define ENABLE_6M_BAND    0  // if you hardware does 6M then edit the bandmem table in SDR_DATA.h
#define ENABLE_144_BAND   0
#define ENABLE_222_BAND   0
#define ENABLE_432_BAND   0
#define ENABLE_902_BAND   0
#define ENABLE_1296_BAND  0
#define ENABLE_2304_BAND  0
#define ENABLE_2400_BAND  0
#define ENABLE_3400_BAND  0
#define ENABLE_5760_BAND  0
#define ENABLE_10G_BAND   0
#define ENABLE_24G_BAND   0
#define ENABLE_47G_BAND   0
#define ENABLE_76G_BAND   0
#define ENABLE_122G_BAND  0

//--------------------------USER HARDWARE AND PREFERENCES---------------------------------------
//
// ---------------------------------------ENCODERS----------------------------------------------
// 
// Choose your actual pin assignments for any you may have.

// VFO Encoder (not I2C).  uses PCB jack labeled ENC1.
#define VFO_PPR 36  // for VFO A/B Tuning encoder. This scales the PPR to account for high vs low PPR encoders.  600ppr is very fast at 1Hz steps, worse at 10Khz!
// I find a value of 60 works good for 600ppr. 30 should be good for 300ppr, 1 or 2 for typical 24-36 ppr encoders. Best to use even numbers above 1. 

#if defined(SMALL_PCB_V1)     // Generic compact display to Teensy Adapter, any size display
  #define I2C_INT_PIN            36
	#define GPIO_VFO_PIN_A          3     // used for VFO
	#define GPIO_VFO_PIN_B          4
	#define GPIO_ENC2_PIN_A        30     // Encoder 2
	#define GPIO_ENC2_PIN_B        31
	#define GPIO_ENC2_PIN_SW       32
	#define GPIO_ENC3_PIN_A        34     // Encoder 3
	#define GPIO_ENC3_PIN_B        35
	#define GPIO_ENC3_PIN_SW       33
	#define PTT_INPUT     		    255   	// GPIO digital input pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define PTT_OUT1      		    255   	// GPIO digital output pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define GPIO_SW1_PIN          255   	// pin assignment for external switches. When enabled, these will be scanned and software debounced
	#define GPIO_SW2_PIN          255   	// Rev 2 PCBs have an 8x2 header U7 that has Teensy GPIO pins 0-7 on it.  
	#define GPIO_SW3_PIN          255		  // Pins 0 and 1 I try to reserve for hardware serial port duties so assigning pins 2 through 7.
	#define GPIO_SW4_PIN          255
	#define GPIO_SW5_PIN          255
  #define GPIO_SW6_PIN          255     // There are only 8 GPIO on the header connector so disable this one and use for GPIO_ANT_PIN output.
	#define GPIO_ANT_PIN          255		  // 255 for unused pins.  When used this is configured as an output instead of input for the GPIO_SWx_PINs
#elif defined(V1_4_3_PCB)     // V1.0 4.3" Display PCB
  #define I2C_INT_PIN            36
	#define GPIO_VFO_PIN_A          4     // used for VFO
	#define GPIO_VFO_PIN_B          3     // Can swap A and B to get direction correct without rewiring
	#define GPIO_ENC2_PIN_A        30     // GPIO Encoder 2.
	#define GPIO_ENC2_PIN_B        31
	#define GPIO_ENC2_PIN_SW       32
	#define GPIO_ENC3_PIN_A        33     // GPIO Encoder 3
	#define GPIO_ENC3_PIN_B        34
	#define GPIO_ENC3_PIN_SW       35
	#define PTT_INPUT     	      255   	// GPIO digital input pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define PTT_OUT1      		    255   	// GPIO digital output pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define GPIO_SW1_PIN          255   	// pin assignment for external switches. When enabled, these will be scanned and software debounced
	#define GPIO_SW2_PIN          255   	// Rev 2 PCBs have an 8x2 header U7 that has Teensy GPIO pins 0-7 on it.  
	#define GPIO_SW3_PIN          255		  // Pins 0 and 1 I try to reserve for hardware serial port duties so assigning pins 2 through 7.
	#define GPIO_SW4_PIN          255     // 255 for unused pins
	#define GPIO_SW5_PIN          255     // 255 for unused pins
  #define GPIO_SW6_PIN          255     // There are only 8 GPIO on the header connector so disable this one and use for GPIO_ANT_PIN output.
	#define GPIO_ANT_PIN          255		  // 255 for unused pins.  When used this is configured as an output instead of input for the GPIO_SWx_PINs
#elif defined (V2_4_3_PCB)    // V2.0 4.3" Display PCB
  #define I2C_INT_PIN            17
	#define GPIO_VFO_PIN_A         15     // used for VFO
	#define GPIO_VFO_PIN_B         16
	#define GPIO_ENC2_PIN_A        36     // Encoder 2
	#define GPIO_ENC2_PIN_B        37
	#define GPIO_ENC2_PIN_SW       38
	#define GPIO_ENC3_PIN_A        35     // Encoder 3
	#define GPIO_ENC3_PIN_B        34
	#define GPIO_ENC3_PIN_SW       33
	#define PTT_INPUT     			    0   	// GPIO digital input pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define PTT_OUT1      			    1   	// GPIO digital output pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define GPIO_SW1_PIN            3     // pin assignment for external switches. When enabled, these will be scanned and software debounced
	#define GPIO_SW2_PIN            4     // Rev 2 PCBs have an 8x2 header U7 that has Teensy GPIO pins 0-7 on it.  
	#define GPIO_SW3_PIN            5     // Pins 0 and 1 I try to reserve for hardware serial port duties so assigning pins 2 through 7.
	#define GPIO_SW4_PIN            6     // 255 for unused pins
	#define GPIO_SW5_PIN           26     // 255 for unused pins
  #define GPIO_SW6_PIN          255     // There are only 8 GPIO on the header connector so disable this one and use for GPIO_ANT_PIN output.
	#define GPIO_ANT_PIN            2		  // Used as an output for Ant relay 1/2
  #define GPIO_GPS_TX_PIN        28     // GPS Jack CN6.  Goes to Serial Port 7 or can be used as generic GPIO.
  #define GPIO_GPS_RX_PIN        29     // GPS Jack CN6.
  #define GPIO_GSP_GPIO_PIN      30     // GPS Jack CN6.
  #elif defined (V21_7_PCB)    // V2.1 7" Display PCB, can also mount the 4.3" RA8875 onto it.
  #define I2C_INT_PIN            17
	#define GPIO_VFO_PIN_A         16     // used for VFO
	#define GPIO_VFO_PIN_B         15
	#define GPIO_ENC2_PIN_A        36     // Encoder 2.   conflicts with I2C encoders if they are enabled
	#define GPIO_ENC2_PIN_B        37
	#define GPIO_ENC2_PIN_SW       38
	#define GPIO_ENC3_PIN_A        35     // Encoder 3
	#define GPIO_ENC3_PIN_B        34
	#define GPIO_ENC3_PIN_SW       33
	#define PTT_INPUT     			    1   	// buffered GPIO digital input pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define PTT_OUT1      			    2   	// buffered GPIO digital output pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define GPIO_SW1_PIN            3   	// pin assignment for external switches. When enabled, these will be scanned and software debounced
	#define GPIO_SW2_PIN            4   	// Rev 2 PCBs have an 8x2 header U7 that has Teensy GPIO pins 0-7 on it.  
	#define GPIO_SW3_PIN            5		  // Pins 0 and 1 I try to reserve for hardware serial port duties so assigning pins 2 through 7.
	#define GPIO_SW4_PIN            6     // these are 3.3V!
	#define GPIO_SW5_PIN           26
  #define GPIO_SW6_PIN          255     // 0   // There are only 8 GPIO on the header connector so disable this one and use for GPIO_ANT_PIN output.
	#define GPIO_ANT_PIN            0	    // 255 // Used as an output for Ant relay 1/2
  #define GPIO_GPS_TX_PIN        28     // GPS Jack CN6.  Goes to Serial Port 7 or can be used as generic GPIO.
  #define GPIO_GPS_RX_PIN        29     // GPS Jack CN6.
  #define GPIO_GSP_GPIO_PIN      30     // GPS Jack CN6.
#elif defined (V22_7_PCB)    // V2.2 7" Display PCB, can also mount a RA8875 4.3" display
  #define I2C_INT_PIN            17
	#define GPIO_VFO_PIN_A         16     // used for VFO
	#define GPIO_VFO_PIN_B         15
	#define GPIO_ENC2_PIN_A        36     // Encoder 2
	#define GPIO_ENC2_PIN_B        37
	#define GPIO_ENC2_PIN_SW       38
	#define GPIO_ENC3_PIN_A        35     // Encoder 3  // For K7MDL build using this jack for step attenuator
	#define GPIO_ENC3_PIN_B        34
	#define GPIO_ENC3_PIN_SW       33
	#define PTT_INPUT     			   40   	// buffered GPIO digital input pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define PTT_OUT1      			   41   	// buffered GPIO digital output pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define GPIO_SW1_PIN            3   	// pin assignment for external switches. When enabled, these will be scanned and software debounced
	#define GPIO_SW2_PIN            4   	// Rev 2 PCBs have an 8x2 header U7 that has Teensy GPIO pins 0-7 on it.  
	#define GPIO_SW3_PIN            5		  // Pins 0 and 1 I try to reserve for hardware serial port duties so assigning pins 2 through 7.
	#define GPIO_SW4_PIN            6     // assign the rest of the pins
	#define GPIO_SW5_PIN            2
  #define GPIO_SW6_PIN           26     // There are 8 GPIO pins on the header connector
	#define GPIO_ANT_PIN           31	    // New in V2.2. Buffered output for Ant relay 1/2.  V2.2 PCB has dedicated buffered output on pin 31.  Can also access at the wirepad next to D32
  #define GPIO_SPARE1_PIN         0     // Placeholder for unused pin on the GPIO header.  I try to save these for hardware UART duty
  #define GPIO_SPARE2_PIN         1     // Placeholder for unused pin on the GPIO header.  I try to save these for hardware UART duty
  #define GPIO_SPARE3_PIN        32     // New i n V2.2. Wirepad for D32 at top of board above CPU.  
  #define GPIO_GPS_TX_PIN        28     // GPS Jack CN6.  Goes to Serial Port 7 or can be used as generic GPIO.
  #define GPIO_GPS_RX_PIN        29     // GPS Jack CN6.   Can also use this port for Panadapter CAT control connection
  #define GPIO_GSP_GPIO_PIN      30     // GPS Jack CN6.
#else // else old proto board assignments
  #define I2C_INT_PIN            29
	#define GPIO_VFO_PIN_A          4     // used for VFO
	#define GPIO_VFO_PIN_B          5
	#define GPIO_ENC2_PIN_A        30     // GPIO Encoder 2
	#define GPIO_ENC2_PIN_B        31
	#define GPIO_ENC2_PIN_SW       32
	#define GPIO_ENC3_PIN_A        33     // GPIO Encoder 3
	#define GPIO_ENC3_PIN_B        34
	#define GPIO_ENC3_PIN_SW       35
	#define PTT_INPUT     	      255   	// GPIO digital input pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define PTT_OUT1      		    255   	// GPIO digital output pin number for external PTT.  Typically LO (GND) = TX, HI = RX.
	#define GPIO_SW1_PIN          255   	// pin assignment for external switches. When enabled, these will be scanned and software debounced
	#define GPIO_SW2_PIN          255   	// Rev 2 PCBs have an 8x2 header U7 that has Teensy GPIO pins 0-7 on it.  
	#define GPIO_SW3_PIN          255		  // Pins 0 and 1 I try to reserve for hardware serial port duties so assigning pins 2 through 7.
	#define GPIO_SW4_PIN          255     // 255 for unused pins
	#define GPIO_SW5_PIN          255     // 255 for unused pins
  #define GPIO_SW6_PIN          255     // 255 for unused pins
	#define GPIO_ANT_PIN          255	    // 255 for unused pins
#endif

#ifndef K7MDL_BUILD

		// Assign 0 to disable, assign a unique number to identify and match the table ID field. 
  // Coordinate this assignment with any i2c encoders
  // VFO Enable is slot 0 by convention, value is ignored.
	#define GPIO_VFO_ENABLE         1     // VFO encoder always enabled.  Normally set to 1 for Touch tuning (via drag) to work in case there are no encoders.
  #ifdef GPIO_ENCODERS
	 #define GPIO_ENC2_ENABLE       0     // Aux GPIO encoder, 0 disables, >0 enables, make unique to place into table row
	 #define GPIO_ENC3_ENABLE       0     // Aux GPIO encoder, 0 disables, >0 enables, make unique to place into table row
  #else
   #define GPIO_ENC2_ENABLE       0     // Aux GPIO encoder, 0 disables, >0 enables, make unique to place into table row
	 #define GPIO_ENC3_ENABLE       0     // Aux GPIO encoder, 0 disables, >0 enables, make unique to place into table row
	#endif
  #define GPIO_SW1_ENABLE         0     // GPIO switch, 0 disables, >0 enables, make unique to pla
	#define GPIO_SW2_ENABLE         0     // GPIO switch, 0 disables, >0 enables, make unique to place into table row
	#define GPIO_SW3_ENABLE         0     // GPIO switch, 0 disables, >0 enables, make unique to place into table row
  #define GPIO_SW4_ENABLE         0     // GPIO switch, 0 disables, >0 enables, make unique to place into table row
	#define GPIO_SW5_ENABLE         0     // GPIO switch, 0 disables, >0 enables, make unique to place into table row
  #define GPIO_SW6_ENABLE         0     // GPIO switch, 0 disables, >0 enables, make unique to place into table row
	#define GPIO_ANT_ENABLE         0     // GPIO Output, 0 disables, any value > 0 enables

	// I2C connected encoders use this this pin to signal interrupts
	// Knob assignments are the user_settings database 
	// While there are up to 6 i2c encoders possible, the encoder table and support functions
	//   only know about 7 encoders, the 1st is always the gpio VFO.
	//   If any GPIO aux encoders are defined, the total must be <=7  (6 aux plus 1 VFO) so some wil be disabled
	#define I2C_ENC1_ENABLE         0    // 0 is Disabled, > 0 to activate - set value to row number in Encoder_List table
	#define I2C_ENC2_ENABLE         0
	#define I2C_ENC3_ENABLE         0    
	#define I2C_ENC4_ENABLE         0    // 0 is disabled
	#define I2C_ENC5_ENABLE         0
	#define I2C_ENC6_ENABLE         0

#else // Do K7MDL Dev Build

	// Assign 0 to disable, assign a unique number to identify and match the table ID field. 
  // Coordinate this assignment with any i2c encoders
  // VFO Enable is slot 0 by convention, value is ignored.
	#define GPIO_VFO_ENABLE         1     // VFO encoder always enabled.  Normally set to 1 for Touch tuning (via drag) to work in case there are no encoders.
  #ifdef GPIO_ENCODERS
	 #define GPIO_ENC2_ENABLE       1     // Aux GPIO encoder, 0 disables, >0 enables, make unique to place into table row
	 #define GPIO_ENC3_ENABLE       0     // Aux GPIO encoder, 0 disables, >0 enables, make unique to place into table row
  #else
   #define GPIO_ENC2_ENABLE       0     // Aux GPIO encoder, 0 disables, >0 enables, make unique to place into table row
	 #define GPIO_ENC3_ENABLE       0     // for K7MDL build using this jack for PE4302 Step attenuator
	#endif
  #define GPIO_SW1_ENABLE         6     // GPIO switch, 0 disables, >0 enables, make unique to pla
	#define GPIO_SW2_ENABLE         7     // GPIO switch, 0 disables, >0 enables, make unique to place into table row
	#define GPIO_SW3_ENABLE         8     // GPIO switch, 0 disables, >0 enables, make unique to place into table row
	#define GPIO_SW4_ENABLE         0     // GPIO switch, 0 disables, >0 enables, make unique to place into table row
	#define GPIO_SW5_ENABLE         0     // GPIO switch, 0 disables, >0 enables, make unique to place into table row
  #define GPIO_SW6_ENABLE         0     // GPIO switch, 0 disables, >0 enables, make unique to place into table row
	#define GPIO_ANT_ENABLE         1     // GPIO Output, 0 disables, any value > 0 enables

	// I2C connected encoders use this this pin to signal interrupts
	// Knob assignments are the user_settings database 
	// While there are up to 6 i2c encoders possible, the encoder table and support functions
	//   only know about 7 encoders, the 1st is always the gpio VFO.
	//   If any GPIO aux encoders are defined, the total must be <=7  (6 aux plus 1 VFO) so some wil be disabled
  #ifdef I2C_ENCODERS
    #define I2C_ENC1_ENABLE         2 //2     // 0 is Disabled, > 0 to activate - set value to row number in Encoder_List table
    #define I2C_ENC2_ENABLE         3 //3
    #if defined K7MDL_BUILD && defined USE_RA8875
      #define I2C_ENC3_ENABLE         0 //4    
      #define I2C_ENC4_ENABLE         0 //5    // 0 is disabled
    #else
      #define I2C_ENC3_ENABLE         4    
      #define I2C_ENC4_ENABLE         5
    #endif
    #define I2C_ENC5_ENABLE         0
    #define I2C_ENC6_ENABLE         0
  #else
    #define I2C_ENC1_ENABLE         0//2 //2     // 0 is Disabled, > 0 to activate - set value to row number in Encoder_List table
    #define I2C_ENC2_ENABLE         0//4 //3
    #define I2C_ENC3_ENABLE         0//3 //4    
    #define I2C_ENC4_ENABLE         0 //5    // 0 is disabled
    #define I2C_ENC5_ENABLE         0
    #define I2C_ENC6_ENABLE         0
  #endif

#endif // K7MDL_BUILD
                                                                                                               
#ifdef I2C_ENCODERS
  #if I2C_ENC1_ENABLE > 0
    #define I2C_ENC1_ADDR       (0x61)  	/* Address 0x61 only - Jumpers A0, A5 and A6 are soldered.*/
  #endif
  #if I2C_ENC2_ENABLE > 0
    #define I2C_ENC2_ADDR       (0x62)  	/* Address 0x62 only - Jumpers A1, A5 and A6 are soldered.*/
  #endif
  #if I2C_ENC3_ENABLE > 0
    #define I2C_ENC3_ADDR       (0x63)  	/* Address 0x63 only - Jumpers A0, A1, A5 and A6 are soldered.*/  
  #endif
  #if I2C_ENC4_ENABLE > 0
    #define I2C_ENC4_ADDR       (0x64)  	/* Address 0x64 only - Jumpers A2, A5 and A6 are soldered.*/  
  #endif
  #if I2C_ENC5_ENABLE > 0
    #define I2C_ENC5_ADDR       (0x65)  	/* Address 0x65 only - Jumpers A0, A2, A5 and A6 are soldered.*/  
  #endif
  #if I2C_ENC6_ENABLE > 0
    #define I2C_ENC6_ADDR       (0x66)  	/* Address 0x66 only - Jumpers A1, A2, A5 and A6 are soldered.*/   
  #endif
#endif // I2C_ENCODERS

// -------------------------  PE4302 6 bit Digital Step Attenuator -----------------------------
//      Digital step attenuator.  0-31.5dB in 0.5dB steps. Connected via I2C port expander.
//      Could use the 3 left over pins on the MCP23017 I2C port expander servicing the SV1AFN preselector module.
//      For now using Teensy 4.1 pins 30-32.
//      
#ifdef PE4302   // for V2.2 board I am not using an encoder on ENC3 so using the ENC3 pins for the Step Attenuator
  #define Atten_CLK       GPIO_GPS_RX_PIN
  #define Atten_DATA      GPIO_GPS_TX_PIN
  #define Atten_LE        GPIO_GSP_GPIO_PIN
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
  #define PAN_CAT
  #define HWSERIAL Serial1 // Teensy hardware Serial or USB Serial port. Set this to the hardware serial port you wish to use
  #include <ft817.h>
  #include "SDR_CAT.h"
#endif  // FT817_CAT

#ifdef PAN_CAT
  #include "SDR_CAT.h"
#endif

#endif //_RADIOCONFIG_H_
