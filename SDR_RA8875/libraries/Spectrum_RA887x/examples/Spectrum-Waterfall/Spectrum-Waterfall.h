#ifndef _SPECTRUM_WATERFALL_H_
#define _SPECTRUM_WATERFALL_H_

//      SPECTRUM-WATERFALL.h
//
//  This is an example file how to use the Spectrum_RA887x library to display a custom
//  sized window containing a sepctrum display and a waterfall display.
//  It takes advantage of the RA887x controller's ability to move blocks of display memory
//  with very few CPU commands enabling rather high resolution scrolling spectrum and waterfall
//  displays possible without undue burden on the host CPU.   
//  This allows CPU cycles to handle other tasks like audio processing and encoders.
// 
//  Content for the display is sourced from DSP audio FFT output data.
//  This library is used in RA8875_SDR project by K7MDL
//  It can be used on Teensy 4.X CPUs together with ether the RA8875 or RA8876 controller based Displays.
//  You need the RA8875 library that is distributed with TeensyDuino
//  or the RA8876 Ra8876LiteTeensy found at https://github.com/wwatson4506/Ra8876LiteTeensy
//  Use the #define below to select the type that matches your display type.
//  More information such as wiring assumptions can be found at my Github WIki and Readme pages
//  https://github.com/K7MDL2/KEITHSDR
//
//  For use in my SDR program I access the VFO dial frequency and calculate frequency markers and
//  peak signal frequencies. For this example I will use constants.  
//  Normally the value would come from VFO encoder + PLL code.
//
//  I have predefined several experimental windows layouts with coloring options located in theg Spectrum_RA887x library files. 
//  Using index 0 as default.  It filles the full width of the display minus borders.
//  Only a few are really useful, others are interesting to look at what is possible. 
//  You can also run 2 windows, perhaps one zoomed in on a 4KHz wide chunk of spectrum, the other larger window covering 20KHz
//
//  There are many ToDos for the this library, a big one is to add pan and zoom.  
//  Right now you can process up to 4096I+Q FFT (2048 data points below and 2048 points above the center frequency)
//  The RA8876 7" is 1024 pixels so today I am only display the center 1000Hz of the available 4096 FFT points.
//
//

#include <Arduino.h>
#include <SPI.h>                // included with Arduino for SPI library
#include <Wire.h>               // included with Arduino or i2c library
#include <WireIMXRT.h>          // gets installed with wire.h
#include <WireKinetis.h>        // included with Arduino
#include <Spectrum_RA887x.h>    // K7MDL Teensy RA887X Spectrum display library

// These are for the included support functions including audio FFT data.   
// Without a working PLL/Rx board there is no FTT data.  One could enabled a test tone or inject music into the audio card line in.
// For this example prgram, you will get a full display with an empty (no signal) spectrum.
#include <Audio.h>              // Included with Teensy and at GitHub https://github.com/PaulStoffregen/Audio
#include <OpenAudio_ArduinoLibrary.h> // F32 library located on GitHub. https://github.com/chipaudette/OpenAudio_ArduinoLibrary
#include <InternalTemperature.h>// V2.1.0 @ Github https://github.com/LAtimes2/InternalTemperature
#include <TimeLib.h>            // TODO  - list where to find this
#include <Metro.h>


// Choose line in or a test sinewave to display something when there is no audio input for the FFT to look at
#define TEST_SINE

//--------------------------------- RA8875 LCD TOUCH DISPLAY INIT & PINS --------------------------
//
#define SPECTRUM_PRESET  0        // The spectrum layout record default value.
                                  // 0 is recommended for full screen.
                                  // 5 for smaller 2 window size.

//#define PANADAPTER          // Optimize some settings for panadapter use.  VFO becomes fixed LO at designated frequency
                            // Comment this ouot to dispable all PANADAPTER settings.

#define PANADAPTER_LO   (8215000u) // Frequency of radio's IF output in Hz. 
                                // For a K3 it is 8215Khz for DATA A mode, 8212.5KHz if USB/LSB
                                // Enabled only when the PANADAPTER define is active. Can be left uncommented.

#define PANADAPTER_MODE_OFFSET_DATA 0   // This is the offset added by the radio in certain modes
                                        // It is usually the Center frequency of the filter
                                        // Enabled only when the PANADAPTER define is active. Can be left uncommented.

//#define PANADAPTER_INVERT // When uncommented, this inverts the tuning direction seen on screen.
                            // Most radio IFs are inverted, though it can change depending on frequency
                            // Enabled only when the PANADAPTER define is active. Can be left uncommented.

#define SCREEN_ROTATION   0 // 0 is normal horizontal landscape orientation  For RA8876 only at this point.
                            // 2 is 180 flip.  This will affect the touch orientation so that must be set to match your display
                            // The 7" RA8876 display has a better off-center viewing angle when horizantal when the touch panel ribbon is at the top.  This requires the touch to be rotated.
                            // The rotation will be 0, touch rotation will be "defined"
                            // When the 7" is vertically mounted the ribbon should be down with Touch Rotation "undefined".

//#define USE_RA8875          // Turns on support for RA8875 LCD Touchscreen Display with FT5204 Touch controller
                            // When commented out it will default to the RA8876 controller and FT5206 touch controller
                            // DEPENDS on correct display controller type connected via 4-wire SPI bus.
                            // UN-comment this line to use RA8876  *** AND in the Spectrum_RA887x.h ***

//==================================== Frequency Set ==========================================
#ifdef PANADAPTER
  #define VFOA PANADAPTER_LO
  #define VFOB PANADAPTER_LO
#else
  #define VFOA  (14074000u)
  #define VFOB   (7074000u)
#endif
    
// Some defines for ease of use 
#define myDARKGREY  31727u

// From RA8876_t3/RA8876Registers.h
#define BLACK       0x0000
#define WHITE       0xffff
#define RED         0xf800
#define LIGHTRED    0xfc10
#define CRIMSON     0x8000
#define GREEN       0x07e0
#define PALEGREEN   0x87f0
#define DARKGREEN   0x0400
#define BLUE        0x001f
#define LIGHTBLUE   0x051f
#define SKYBLUE     0x841f
#define DARKBLUE    0x0010
#define YELLOW      0xffe0
#define LIGHTYELLOW 0xfff0
#define DARKYELLOW  0x8400 // mustard
#define CYAN        0x07ff
#define LIGHTCYAN   0x87ff
#define DARKCYAN    0x0410
#define MAGENTA     0xf81f
#define VIOLET      0xfc1f
#define BLUEVIOLET  0x8010
#define ORCHID      0xA145 
// Other sources of RGB color definitions
#define NAVY        0x000F
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define DARKGREY    0x7BEF
#define ORANGE      0xFD20
#define GREENYELLOW 0xAFE5
#define PINK        0xF81F

#ifdef USE_RA8875
  #define  SCREEN_WIDTH      800 
  #define  SCREEN_HEIGHT     480
  #define  RA8875_INT        14   //any pin
  #define  RA8875_CS         10   //any digital pin
  #define  RA8875_RESET      9    //any pin or nothing!
  #define  MAXTOUCHLIMIT     3    //1...5  using 3 for 3 finger swipes, otherwise 2 for pinches or just 1 for touch
  #include <RA8875.h>             // internal Teensy library with ft5206 cap touch enabled in user_setting.h
  #include <SPI.h>                // included with Arduino
  #include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
  #include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
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
  #include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
  #include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
  #include <RA8876_t3.h>           // Github
  #include <FT5206.h>
  #define  CTP_INT           14   // Use an interrupt capable pin such as pin 2 (any pin on a Teensy)
  #define  RA8876_CS         10   //any digital pin
  #define  RA8876_RESET      9    //any pin or nothing!
  #define  MAXTOUCHLIMIT     3    //1...5  using 3 for 3 finger swipes, otherwise 2 for pinches or just 1 for touch
#endif // USE_RA8876_t3

bool        enable_printCPUandMemory = false;
void        togglePrintMemoryAndCPU(void) { enable_printCPUandMemory = !enable_printCPUandMemory; };
int32_t     Freq_Peak = 0;

#endif  // end of SPECTRUM_WATERFALL
