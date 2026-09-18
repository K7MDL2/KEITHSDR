as of 4/30/20
Added frame buffering type support:
1. useCanvas()
2. updateScreen()

modified for printing to screen:
1. print(uint8_t text) -> write(c);
2. printStr(const char * str); -> write(c, string length);
====================================================================
as of 04/29/20
The Ra8876 library redo for the Teensy has been focused on consolidation and essentially refactoring to make it into a more generic libary for library.  For the restructuing of the the library it is currently being tested with the BuyDisplay's ER-TFTM070-6 7 inch LCD module w/touch, https://www.buydisplay.com/spi-7-inch-tft-lcd-dislay-module-1024x600-ra8876-optl-touch-screen-panel.

Chages made so far include the users ability to:
1. Specify the SPI port for the display:
RA8876_t3(const uint8_t CSp = 10, const uint8_t RSTp = 8, const uint8_t mosi_pin = 11, const uint8_t sclk_pin = 13, const uint8_t miso_pin = 12);
pins are defaulted to standard SPI pins.  Currently the Back Light pin is still harde coded in the library to pin 7

2. Command changes for compatibility with other display libraries:
tft_cls -> fillScreen(color)
getGwidth -> width()
getGheight -> height()

3. Added or modified
setCursor(x, y) - added
drawFastHLine - added
setFontSize - modified so you can pass just the size, with a default runflag=false
setTextSize - added, calls setFontSize, only have to pass size, defaults to runflag=false
fillStatusLine(uint16_t color); - used to fill the status line (left it in since i thought it was neat, at least for now)
printStatusLine(uint16_t x0,uint16_t fgColor,uint16_t bgColor, const char *text); - now used to print to the statusline

4. prints to screen changed to use print stream so you can call tft.print, printf, println without having to convert data to strings etc.

5. vt100 - not incorporated for now.

6. All examples have been updated to the new data calls and working with the updated library.





==========================================================================================
Ra8876LiteTeeensy WIP

Teensy RA8876 Driver using BuyDisplay's ER-TFTM101-1 10.1 inch lcd module tft display w/touch
https://www.buydisplay.com/default/serial-spi-i2c-10-1-inch-tft-lcd-module-dislay-w-ra8876-optl-touch-panel

RAIO RA8876 support documentation can be found here:
http://www.raio.com.tw/en/Support_RA887677.html

Tested and working with PJRC Teensy's T36 and T40.
And possibly with modification for TechToy's HDMI Shield. Not Tested.
http://www.techtoys.com.hk/BoardsKits/HDMIshield/HDMIshield.htm

Documentation and drivers for this device can be found at the bottom of
the BuyDisplay WEB page for this display.

The TFT panel I have was ordered setup with:
 - SPI 4 wire 40 pin interface
 - 5V Power Supply (Can be 3.3v if ordered that way)
 - 10.1" Resistive Touch Controller
 - SD Card Pin interface
 - ER3304-1 Font Chip
 
The current Price is about $74.00 not including shipping.

Teensy connections are through a 4 wire SPI port. Other interface options are available.
The RA8876 TFT controller has the same MISO tri-state problem as the RA8875 has. It
needs a 74HCT125 or similar chip if using more than one device on the same SPI port.

Check out Sumotoy's WEB page for a solution to this problem.
https://github.com/sumotoy/RA8875/wiki/Fix-compatibility-with-other-SPI-devices

*** CONNECTING THE TEENSY TO THE ER-TFTM101-1 ***

40 pin dual inline connector pinouts can be found here.
https://www.buydisplay.com/download/interfacing/ER-TFTM101-1_RTP_Interfacing.pdf

TEENSY 36/40                                  ER-TFTM101-1
-------------------------------------------------------------
Pin 10 /CS   --------------------------------> Pin 5  /SCS
Pin 13 SCK   --------------------------------> Pin 8  SCLK
Pin 11 MOSI  --------------------------------> Pin 7  SDI
Pin 12 MISO  <-------------------------------- Pin 6  SDO
Pin 21 RESET --------------------------------> Pin 11 /RESET
Pin 20 BLITE --------------------------------> Pin 14 BL_CONTROL (VDD 3.3V)
-------------------------------------------------------------

*** Touch Screen was only tested on the T36 with the TallDog breakout board.    ***
*** Resistive touch panel uses XPT2046. Used a modified version of XPT2046.cpp  ***
 
                       TOUCH SCREEN
-------------------------------------------------------------
Pin 31 /CS1  --------------------------------> Pin 32 TP_/CS
Pin 32 SCK2  --------------------------------> Pin 35 TP_SLCK
Pin 0  MOSI1 --------------------------------> Pin 34 TP_DIN
Pin 1  MISO1 --------------------------------> Pin 36 TP_DOUT
Pin 24 TSINT <-------------------------------- Pin 33 TS_PEN
--------------------------------------------------------------

ER-TFTM101-1 Ground Pins are 1,2,13,31,39,40. These should all be connectd
to Teensy Grounds.
ER-TFTM101-1 Power Pins (VDD is 5V or 3.3v depending on how it was configured
when ordered) are 3,4,37,38 and all should be connected together.

*** EXAMPLES ***

graphics.ino - Demonstrates the RA8876 accelerated graphics engine.
             - Rectangles, Rectangles with line thickness, Filled Rectangles.
             - Round Rectangles, Round Rectangles with different line thickness,
               Filled Round Rectangles.
             - Circles, Circles with line thickness, Filled Circles.
             - Ellipses, Ellipses with line thickness, Filled Ellipses.
             - Lines.
guages.ino   - This is Sumotoy's RA8875 guages example ported to the RA8876.
piptest.ino  - Picuture In Picture demo. See comments in file.
scroll.ino   - Demo of scrolling a text screen up and then down. It's kind of
             - slow and there is probably a better way of scrolling.
treedee.ino  - This is Sumotoy's RA8875 treedee example ported to the RA8876.
UserDefinedFonts.ino - This is an example of user defined characters on the RA8876.
                     - In this example the font is compiled into the program and
                     - then loaded from flash memory into CGRAM. If in tft.h file
                     - #define USE_FF_FONTLOAD is set from 0 to 1 then the fontload()
                     - function will be enabled. This allows fonts to be loaded from
                     - disk. Currently fontload() is setup to use FatFS but can be changed
                     - to use SDFat or SD. The fontload() function is loacated in tft.cpp.
vt100Demo.ino - This is basically the same as scroll.ino but uses VT100 string command to
              - demonstrate text scrolling.

There are a lot things the RA8876 can do that I have not tried yet such as screen rotation,
mirroring, working with transparency  etc...

The RA8876.pdf manual is a little hard to use but seems complete.

Mountain Hung the Senior Engineer at RAIO is very helpfull and respond quickly to emails.
His Email Address:
mountain@raio.com.tw

Check out the source files for more info:
- Ra8876LiteTeensy.cpp A heavily modified version of the original Ra8876Lite.cpp file.
- tft.cpp Is basically wrapper functions for Ra8876LiteTeensy.cpp.
- vt100.c Is a modified version of a VT100 terminal program by Geoff Graham, April 2014.

Again, this is WIP and is probably just a starting point for those who have better programming
than mine:)


