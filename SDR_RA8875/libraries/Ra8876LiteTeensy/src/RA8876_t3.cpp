//**************************************************************//
//**************************************************************//
/*
 * Ra8876LiteTeensy.cpp
 * Modified Version of: File Name : RA8876_t3.cpp                                   
 *			Author    : RAiO Application Team                             
 *			Edit Date : 09/13/2017
 *			Version   : v2.0  1.modify bte_DestinationMemoryStartAddr bug 
 *                 			  2.modify ra8876SdramInitial Auto_Refresh
 *                 			  3.modify ra8876PllInitial 
 * 	  	      : For Teensy 3.x and T4
 *                    : By Warren Watson
 *                    : 06/07/2018 - 11/31/2019
 *                    : Copyright (c) 2017-2019 Warren Watson.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//**************************************************************//

#include "RA8876_t3.h"
#include "Arduino.h"
#include "SPI.h"
#ifdef SPI_HAS_TRANSFER_ASYNC
#include "EventResponder.h"
#endif

#if defined (USE_FT5206_TOUCH)
	const uint8_t _ctpAdrs = 0x38;
	const uint8_t coordRegStart[5] = {0x03,0x09,0x0F,0x15,0x1B};
	static volatile bool _FT5206_INT = false;
#endif



//**************************************************************//
// Graphic Cursor Image (Busy)
//**************************************************************//
PROGMEM unsigned char gImage_busy_im[256] = { /* 0X00,0X02,0X20,0X00,0X20,0X00, */
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X55,0X55,0X55,0X55,0X6A,0XAA,
0XAA,0XAA,0X54,0X00,0X00,0X05,0X6A,0XAA,0XAA,0XAA,0X55,0X55,0X55,0X55,0X6A,0XAA,
0XAA,0XAA,0X94,0X00,0X00,0X05,0XAA,0XAA,0XAA,0XAA,0X94,0X00,0X00,0X05,0XAA,0XAA,
0XAA,0XAA,0X94,0X44,0X44,0X45,0XAA,0XAA,0XAA,0XAA,0X94,0X11,0X11,0X05,0XAA,0XAA,
0XAA,0XAA,0X95,0X04,0X44,0X15,0XAA,0XAA,0XAA,0XAA,0XA5,0X41,0X10,0X56,0XAA,0XAA,
0XAA,0XAA,0XA9,0X50,0X41,0X5A,0XAA,0XAA,0XAA,0XAA,0XAA,0X54,0X05,0X6A,0XAA,0XAA,
0XAA,0XAA,0XAA,0X94,0X05,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X94,0X05,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0X94,0X45,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X94,0X05,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0X50,0X01,0X6A,0XAA,0XAA,0XAA,0XAA,0XA9,0X40,0X40,0X5A,0XAA,0XAA,
0XAA,0XAA,0XA5,0X00,0X10,0X16,0XAA,0XAA,0XAA,0XAA,0X94,0X00,0X00,0X05,0XAA,0XAA,
0XAA,0XAA,0X94,0X04,0X44,0X05,0XAA,0XAA,0XAA,0XAA,0X94,0X11,0X11,0X05,0XAA,0XAA,
0XAA,0XAA,0X94,0X44,0X44,0X45,0XAA,0XAA,0XAA,0XAA,0X95,0X11,0X11,0X15,0XAA,0XAA,
0XAA,0XAA,0X55,0X55,0X55,0X55,0X6A,0XAA,0XAA,0XAA,0X54,0X00,0X00,0X05,0X6A,0XAA,
0XAA,0XAA,0X55,0X55,0X55,0X55,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
};

//**************************************************************//
// Graphic Cursor Image (Not)
//**************************************************************//
PROGMEM unsigned char gImage_no_im[256] = { /* 0X00,0X02,0X20,0X00,0X20,0X00, */
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X95,0X55,0XAA,0XAA,0XAA,
0XAA,0XAA,0XA9,0X40,0X00,0X5A,0XAA,0XAA,0XAA,0XAA,0X94,0X00,0X00,0X05,0XAA,0XAA,
0XAA,0XAA,0X40,0X00,0X00,0X00,0X6A,0XAA,0XAA,0XA9,0X00,0X15,0X55,0X00,0X1A,0XAA,
0XAA,0XA4,0X00,0X6A,0XAA,0X50,0X06,0XAA,0XAA,0XA4,0X00,0X6A,0XAA,0XA4,0X06,0XAA,
0XAA,0X90,0X00,0X1A,0XAA,0XA9,0X01,0XAA,0XAA,0X90,0X10,0X06,0XAA,0XA9,0X01,0XAA,
0XAA,0X40,0X64,0X01,0XAA,0XAA,0X40,0X6A,0XAA,0X40,0X69,0X00,0X6A,0XAA,0X40,0X6A,
0XAA,0X40,0X6A,0X40,0X1A,0XAA,0X40,0X6A,0XAA,0X40,0X6A,0X90,0X06,0XAA,0X40,0X6A,
0XAA,0X40,0X6A,0XA4,0X01,0XAA,0X40,0X6A,0XAA,0X40,0X6A,0XA9,0X00,0X6A,0X40,0X6A,
0XAA,0X40,0X6A,0XAA,0X40,0X1A,0X40,0X6A,0XAA,0X90,0X1A,0XAA,0X90,0X05,0X01,0XAA,
0XAA,0X90,0X1A,0XAA,0XA4,0X00,0X01,0XAA,0XAA,0XA4,0X06,0XAA,0XA9,0X00,0X06,0XAA,
0XAA,0XA4,0X01,0X6A,0XAA,0X40,0X06,0XAA,0XAA,0XA9,0X00,0X15,0X55,0X00,0X1A,0XAA,
0XAA,0XAA,0X40,0X00,0X00,0X00,0X6A,0XAA,0XAA,0XAA,0X94,0X00,0X00,0X05,0XAA,0XAA,
0XAA,0XAA,0XA9,0X40,0X00,0X5A,0XAA,0XAA,0XAA,0XAA,0XAA,0X95,0X55,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
};

//**************************************************************//
// Graphic Cursor Image (Arrow)
//**************************************************************//
PROGMEM unsigned char gImage_arrow_il[256] = { /* 0X00,0X02,0X20,0X00,0X20,0X00, */
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0X5A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X46,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0X41,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0X40,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0X40,0X01,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X00,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,
0X40,0X00,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X00,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,
0X40,0X00,0X01,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X00,0X00,0X6A,0XAA,0XAA,0XAA,0XAA,
0X40,0X00,0X00,0X1A,0XAA,0XAA,0XAA,0XAA,0X40,0X00,0X00,0X06,0XAA,0XAA,0XAA,0XAA,
0X40,0X00,0X00,0X01,0XAA,0XAA,0XAA,0XAA,0X40,0X00,0X00,0X00,0X6A,0XAA,0XAA,0XAA,
0X40,0X00,0X15,0X55,0X5A,0XAA,0XAA,0XAA,0X40,0X10,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,
0X40,0X64,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,0X41,0XA4,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,
0X46,0XA9,0X01,0XAA,0XAA,0XAA,0XAA,0XAA,0X5A,0XA9,0X01,0XAA,0XAA,0XAA,0XAA,0XAA,
0X6A,0XAA,0X40,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X6A,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0X90,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X1A,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XA4,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XA4,0X06,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XA9,0X5A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
};

//**************************************************************//
// Graphic Cursor Image (Pen)
//**************************************************************//
PROGMEM unsigned char gImage_pen_il[256] = { /* 0X00,0X02,0X20,0X00,0X20,0X00, */
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0X96,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X91,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0XA4,0X15,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XA4,0X00,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,
0XA9,0X01,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XA9,0X00,0X46,0XAA,0XAA,0XAA,0XAA,0XAA,
0XAA,0X40,0X51,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X14,0X6A,0XAA,0XAA,0XAA,0XAA,
0XAA,0XA4,0X05,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XA9,0X01,0X46,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0X40,0X51,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X14,0X6A,0XAA,0XAA,0XAA,
0XAA,0XAA,0XA4,0X05,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XA9,0X01,0X46,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0X40,0X51,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X14,0X69,0XAA,0XAA,
0XAA,0XAA,0XAA,0XA4,0X01,0X14,0X6A,0XAA,0XAA,0XAA,0XAA,0XA9,0X00,0X44,0X1A,0XAA,
0XAA,0XAA,0XAA,0XAA,0X40,0X11,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X04,0X41,0XAA,
0XAA,0XAA,0XAA,0XAA,0XA4,0X01,0X10,0X6A,0XAA,0XAA,0XAA,0XAA,0XA9,0X00,0X44,0X1A,
0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X11,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X04,0X1A,
0XAA,0XAA,0XAA,0XAA,0XAA,0XA4,0X01,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XA9,0X00,0X1A,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X95,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
};

// Create a parameter save structure for all 10 screen pages	
tftSave_t	screenSave1,
			screenSave2,
			screenSave3,
			screenSave4,
			screenSave5,
			screenSave6,
			screenSave7,
			screenSave8,
			screenSave9;
			//screenSave10;
// Create pointers to each screen save structs
tftSave_t *screenPage1 = &screenSave1;
tftSave_t *screenPage2 = &screenSave2;
tftSave_t *screenPage3 = &screenSave3;
tftSave_t *screenPage4 = &screenSave4;
tftSave_t *screenPage5 = &screenSave5;
tftSave_t *screenPage6 = &screenSave6;
tftSave_t *screenPage7 = &screenSave7;
tftSave_t *screenPage8 = &screenSave8;
tftSave_t *screenPage9 = &screenSave9;
//tftSave_t *screenPage10 = &screenSave10; // Not used at this time


	
	int16_t HDW = 1024;
	int16_t VDH = 600;
	
	//Physical size of screen - these numbers won't change even if rotation is applied or status bar occupies some screen area
	int16_t SCREEN_WIDTH  = HDW;
	int16_t SCREEN_HEIGHT = VDH;

#ifdef SPI_HAS_TRANSFER_ASYNC
//**************************************************************//
// If using DMA, must close transaction and de-assert _CS
// after the data has been sent.
//**************************************************************//
void asyncEventResponder(EventResponderRef event_responder) {
  RA8876_t3 *tft = (RA8876_t3*)event_responder.getContext();
  tft->activeDMA = false;
  tft->endSend(true);
}
#endif

//**************************************************************//
// RA8876_t3()
//**************************************************************//
// Create RA8876 driver instance
RA8876_t3::RA8876_t3(const uint8_t CSp, const uint8_t RSTp, const uint8_t mosi_pin, const uint8_t sclk_pin, const uint8_t miso_pin)
{
	_mosi = mosi_pin;
	_miso = miso_pin;
	_sclk = sclk_pin;
	_cs = CSp;
	_rst = RSTp;
}

//**************************************************************//
// Ra8876_begin()
//**************************************************************//
#ifndef FLASHMEM
#define FLASHMEM
#endif
FLASHMEM boolean RA8876_t3::begin(uint32_t spi_clock) 
{ 
  //initialize the bus for Teensy 3.6/4.0
  // Figure out which SPI Buss to use.  
  _SPI_CLOCK = spi_clock;				// #define ILI9341_SPICLOCK 30000000
	if (SPI.pinIsMOSI(_mosi) && ((_miso == 0xff) || SPI.pinIsMISO(_miso)) && SPI.pinIsSCK(_sclk)) {
		_pspi = &SPI;
		_spi_num = 0;          // Which buss is this spi on? 
		#ifdef KINETISK
		_pkinetisk_spi = &KINETISK_SPI0;  // Could hack our way to grab this from SPI object, but...
		//_fifo_full_test = (3 << 12);
		#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)  // Teensy 4.x 
		_pimxrt_spi = &IMXRT_LPSPI4_S;  // Could hack our way to grab this from SPI object, but...
		#else
		_pkinetisl_spi = &KINETISL_SPI0;
		#endif				
	
	#if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1062__) || defined(__MKL26Z64__)
	} else if (SPI1.pinIsMOSI(_mosi) && ((_miso == 0xff) || SPI1.pinIsMISO(_miso)) && SPI1.pinIsSCK(_sclk)) {
		_pspi = &SPI1;
		_spi_num = 1;          // Which buss is this spi on? 
		#ifdef KINETISK
		_pkinetisk_spi = &KINETISK_SPI1;  // Could hack our way to grab this from SPI object, but...
		//_fifo_full_test = (0 << 12);
		#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)  // Teensy 4.x 
		_pimxrt_spi = &IMXRT_LPSPI3_S;  // Could hack our way to grab this from SPI object, but...
		#else
		_pkinetisl_spi = &KINETISL_SPI1;
		#endif				
	#if !defined(__MKL26Z64__)
	} else if (SPI2.pinIsMOSI(_mosi) && ((_miso == 0xff) || SPI2.pinIsMISO(_miso)) && SPI2.pinIsSCK(_sclk)) {
		_pspi = &SPI2;
		_spi_num = 2;          // Which buss is this spi on? 
		#ifdef KINETISK
		_pkinetisk_spi = &KINETISK_SPI2;  // Could hack our way to grab this from SPI object, but...
		//_fifo_full_test = (0 << 12);
		#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)  // Teensy 4.x 
		_pimxrt_spi = &IMXRT_LPSPI1_S;  // Could hack our way to grab this from SPI object, but...
		#endif				
	#endif
	#endif
	} else {
		Serial.println("RA8876_t3: The IO pins on the constructor are not valid SPI pins");
	
		Serial.printf("    mosi:%d miso:%d SCLK:%d CS:%d\n", _mosi, _miso, _sclk, _cs); Serial.flush();
		_errorCode |= (1 << 1);//set
		return false;  // most likely will go bomb

	}
	// Make sure we have all of the proper SPI pins selected.
	_pspi->setMOSI(_mosi);
	_pspi->setSCK(_sclk);
	if (_miso != 0xff) _pspi->setMISO(_miso);

	// And startup SPI...
	_pspi->begin();

	// for this round will punt on trying to use CS as hardware CS...
#ifdef KINETISK
	pinMode(_cs, OUTPUT);
	_csport    = portOutputRegister(digitalPinToPort(_cs));
	_cspinmask = digitalPinToBitMask(_cs);
	*_csport |= _cspinmask;

#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)  // Teensy 4.x 
	// Serial.println("   T4 setup CS/DC"); Serial.flush();
	_csport = portOutputRegister(_cs);
	_cspinmask = digitalPinToBitMask(_cs);
	pinMode(_cs, OUTPUT);	
	DIRECT_WRITE_HIGH(_csport, _cspinmask);
//	maybeUpdateTCR(_tcr_dc_not_assert | LPSPI_TCR_FRAMESZ(7));

#else
	// TLC
	pinMode(_cs, OUTPUT);
	_csport    = portOutputRegister(digitalPinToPort(_cs));
	_cspinmask = digitalPinToBitMask(_cs);
	*_csport |= _cspinmask;
#endif	

	#ifdef SPI_HAS_TRANSFER_ASYNC
		finishedDMAEvent.setContext(this);	// Set the contxt to us
		finishedDMAEvent.attachImmediate(asyncEventResponder);
	#endif
  
	// toggle RST low to reset
	if (_rst < 255) {
		pinMode(_rst, OUTPUT);
		digitalWrite(_rst, HIGH);
		delay(5);
		digitalWrite(_rst, LOW);
		delay(20);
		digitalWrite(_rst, HIGH);
		delay(150);
	}

	#if defined(USE_FT5206_TOUCH)
		_intCTSPin = 255;
		_rstCTSPin = 255;
	#endif

	if(!checkIcReady())
		return false;
	//read ID code must disable pll, 01h bit7 set 0
	lcdRegDataWrite(0x01,0x08);
	delay(1);
	if ((lcdRegDataRead(0xff) != 0x76)&&(lcdRegDataRead(0xff) != 0x77))
		return false;

	// Initialize RA8876 to default settings
	if(!ra8876Initialize())
		return false;
	//------- time for capacitive touch stuff -----------------
	#if defined(USE_FT5206_TOUCH)
		_maxTouch = 5;
		_gesture = 0;
		_currentTouches = 0;
		_currentTouchState = 0;
	#endif


	// return success
	return true;
}

//**************************************************************//
// Initialize RA8876 to default settings.
// Return true on success.
//**************************************************************//
boolean RA8876_t3::ra8876Initialize() {
	
	// Init PLL
	if(!ra8876PllInitial())
		return false;
	// Init SDRAM
	if(!ra8876SdramInitial())
		return false;

	lcdRegWrite(RA8876_CCR);//01h
//  lcdDataWrite(RA8876_PLL_ENABLE<<7|RA8876_WAIT_MASK<<6|RA8876_KEY_SCAN_DISABLE<<5|RA8876_TFT_OUTPUT24<<3
  
  lcdRegWrite(RA8876_CCR);//01h
  lcdDataWrite(RA8876_PLL_ENABLE<<7|RA8876_WAIT_NO_MASK<<6|RA8876_KEY_SCAN_DISABLE<<5|RA8876_TFT_OUTPUT24<<3
  |RA8876_I2C_MASTER_DISABLE<<2|RA8876_SERIAL_IF_ENABLE<<1|RA8876_HOST_DATA_BUS_SERIAL);

  lcdRegWrite(RA8876_MACR);//02h
  lcdDataWrite(RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_LRTB<<1);

  lcdRegWrite(RA8876_ICR);//03h
  lcdDataWrite(RA8877_LVDS_FORMAT<<3|RA8876_GRAPHIC_MODE<<2|RA8876_MEMORY_SELECT_IMAGE);

  lcdRegWrite(RA8876_MPWCTR);//10h
  lcdDataWrite(RA8876_PIP1_WINDOW_DISABLE<<7|RA8876_PIP2_WINDOW_DISABLE<<6|RA8876_SELECT_CONFIG_PIP1<<4
  |RA8876_IMAGE_COLOCR_DEPTH_16BPP<<2|TFT_MODE);

  lcdRegWrite(RA8876_PIPCDEP);//11h
  lcdDataWrite(RA8876_PIP1_COLOR_DEPTH_16BPP<<2|RA8876_PIP2_COLOR_DEPTH_16BPP);
  
  lcdRegWrite(RA8876_AW_COLOR);//5Eh
  lcdDataWrite(RA8876_CANVAS_BLOCK_MODE<<2|RA8876_CANVAS_COLOR_DEPTH_16BPP);
  
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_S0_COLOR_DEPTH_16BPP);//92h
  
  /*TFT timing configure*/
  lcdRegWrite(RA8876_DPCR);//12h
  lcdDataWrite(XPCLK_INV<<7|RA8876_DISPLAY_OFF<<6|RA8876_OUTPUT_RGB);
  
	/* TFT timing configure (1024x600) */
	lcdRegWrite(RA8876_DPCR);//12h
	lcdDataWrite(XPCLK_INV<<7|RA8876_DISPLAY_OFF<<6|RA8876_OUTPUT_RGB);
	
	lcdRegWrite(RA8876_PCSR);//13h
	lcdDataWrite(XHSYNC_INV<<7|XVSYNC_INV<<6|XDE_INV<<5);
    
	lcdHorizontalWidthVerticalHeight(HDW,VDH);
	lcdHorizontalNonDisplay(HND);
	lcdHsyncStartPosition(HST);
	lcdHsyncPulseWidth(HPW);
	lcdVerticalNonDisplay(VND);
	lcdVsyncStartPosition(VST);
	lcdVsyncPulseWidth(VPW);
	
	// Init Global Variables
	_width = 	SCREEN_WIDTH;
	_height = 	SCREEN_HEIGHT;
	_rotation = 0;
	currentPage = PAGE1_START_ADDR; // set default screen page address
	pageOffset = 0;
	gCursorX = 0;
	gCursorY = 0;
	_cursorX = 0;
	_cursorY = 0;
	CharPosX = 0;
	CharPosY = 0;
	_FNTwidth = 8; // Default font width
	_FNTheight = 16; // Default font height;
	_scaleX = 1;
	_scaleY = 1;
	_textMode = true;
	prompt_size = 1 * (_FNTwidth * _scaleX); // prompt ">"
	vdata = 0;  // Used in tft_print()
	leftmarg = 0;
	topmarg = 0;
	rightmarg =  (uint8_t)(_width / (_FNTwidth  * _scaleX));
	bottommarg = (uint8_t)(_height / (_FNTheight * _scaleY));
	_scrollXL = 0;
	_scrollXR = _width;
	_scrollYT = 0;
	_scrollYB = _height;
	_cursorX = _scrollXL;
	_cursorY = _scrollYT;
	CharPosX = _scrollXL;
	CharPosY = _scrollYT;
	tab_size = 8;
	_cursorXsize = _FNTwidth;
	_cursorYsize = _FNTheight;
	_TXTForeColor = COLOR65K_WHITE;
	_TXTBackColor = COLOR65K_DARKBLUE;
	RA8876_BUSY = false;
	//_FNTrender = false;
	/* set-->  _TXTparameters  <--
	0:_extFontRom = false;
	1:_autoAdvance = true;
	2:_textWrap = user defined
	3:_fontFullAlig = false;
	4:_fontRotation = false;//not used
	5:_alignXToCenter = false;
	6:_alignYToCenter = false;
	7: render         = false;
	*/
	_TXTparameters = 0b00000010;
	
	_activeWindowXL = 0;
	_activeWindowXR = _width;
	_activeWindowYT = 0;
	_activeWindowYB = _height;
  
	_portrait = false;

	_backTransparent = false;	
	
	displayOn(true);	// Turn on TFT display
	UDFont = false;     // Turn off user define fonts
	
	// Setup graphic cursor
	Set_Graphic_Cursor_Color_1(0xff); // Foreground color
	Set_Graphic_Cursor_Color_2(0x00); // Outline color
        Disable_Graphic_Cursor(); // Turn off Graphic Cursor
        Disable_Text_Cursor(); // Turn off Text Cursor
	Graphic_cursor_initial();  // Initialize Graphic Cursor
        Select_Graphic_Cursor_2(); // Select Arrow Graphic Cursor
	// Set default foreground and background colors
	setTextColor(_TXTForeColor, _TXTBackColor);	
	// Position text cursor to default
	setTextCursor(_scrollXL, _scrollYT);
	// Setup Text Cursor
	cursorInit();
	// Set Margins to default settings
	setTMargins(0, 0, 0, 0); // Left Side, Top Side, Right Side, Bottom Side

	// This must be called before usage of fillScreen() which calls drawSquareFill().
	// drawSquareFill() is ineffective otherwise. 
	setClipRect();

	// Save Default Screen Parameters
	saveTFTParams(screenPage1);
	saveTFTParams(screenPage2);
	saveTFTParams(screenPage3);
	saveTFTParams(screenPage4);
	saveTFTParams(screenPage5);
	saveTFTParams(screenPage6);
	saveTFTParams(screenPage7);
	saveTFTParams(screenPage8);
	saveTFTParams(screenPage9);
//	saveTFTParams(screenPage10);

	// Initialize all screen colors to default values
	currentPage = 999; // Don't repeat screen page 1 init.
	selectScreen(PAGE1_START_ADDR);	// Init page 1 screen
	fillScreen(COLOR65K_DARKBLUE);     // Not sure why we need to clear the screen twice
	//fillStatusLine(COLOR65K_DARKBLUE);
	selectScreen(PAGE2_START_ADDR);	// Init page 2 screen
	fillScreen(COLOR65K_DARKBLUE);
	//fillStatusLine(COLOR65K_DARKBLUE);
	selectScreen(PAGE3_START_ADDR);	// Init page 3 screen
	fillScreen(COLOR65K_DARKBLUE);
	//fillStatusLine(COLOR65K_DARKBLUE);
	selectScreen(PAGE4_START_ADDR);	// Init page 4 screen
	fillScreen(COLOR65K_DARKBLUE);
	//fillStatusLine(COLOR65K_DARKBLUE);
	selectScreen(PAGE5_START_ADDR);	// Init page 5 screen
	fillScreen(COLOR65K_DARKBLUE);
	//fillStatusLine(COLOR65K_DARKBLUE);
	selectScreen(PAGE6_START_ADDR);	// Init page 6 screen
	fillScreen(COLOR65K_DARKBLUE);
	//fillStatusLine(COLOR65K_DARKBLUE);
	selectScreen(PAGE7_START_ADDR);	// Init page 7 screen
	//fillScreen(COLOR65K_DARKBLUE);
	fillStatusLine(COLOR65K_DARKBLUE);
	selectScreen(PAGE8_START_ADDR);	// Init page 8 screen
	fillScreen(COLOR65K_DARKBLUE);
	//fillStatusLine(COLOR65K_DARKBLUE);
	selectScreen(PAGE9_START_ADDR);	// Init page 9 screen
	fillScreen(COLOR65K_DARKBLUE);
	//fillStatusLine(COLOR65K_DARKBLUE);
	selectScreen(PAGE10_START_ADDR);	// Init page 10 screen
	fillScreen(COLOR65K_DARKBLUE);
	//fillStatusLine(COLOR65K_DARKBLUE);
	selectScreen(PAGE1_START_ADDR); // back to page 1 screen

	// Set graphic mouse cursor to center of screen
	gcursorxy(width() / 2, height() / 2);
		
	setOrigin();
	setTextSize(1, 1);
	
  return true;
}

//**************************************************************//
// Write to a RA8876 register
//**************************************************************//
void RA8876_t3::lcdRegWrite(ru8 reg, bool finalize) 
{
  ru16 _data = (RA8876_SPI_CMDWRITE16 | reg);
  
  startSend();
  _pspi->transfer16(_data);
  endSend(finalize);
}

void RA8876_t3::LCD_CmdWrite(unsigned char cmd)
{	
  startSend();
  _pspi->transfer16(0x00);
  _pspi->transfer(cmd);
  endSend(true);
}

//**************************************************************//
// Write RA8876 Data
//**************************************************************//
void RA8876_t3::lcdDataWrite(ru8 data, bool finalize) 
{
  ru16 _data = (RA8876_SPI_DATAWRITE16 | data);
  startSend();
  _pspi->transfer16(_data);
  endSend(finalize);
}

//**************************************************************//
// Read RA8876 Data
//**************************************************************//
ru8 RA8876_t3::lcdDataRead(bool finalize) 
{
  ru16 _data = (RA8876_SPI_DATAREAD16 | 0x00);
  
  startSend();
  ru8 data = _pspi->transfer16(_data);
  endSend(finalize);
  return data;
}

//**************************************************************//
// Read RA8876 status register
//**************************************************************//
ru8 RA8876_t3::lcdStatusRead(bool finalize) 
{
  startSend();
  ru8 data = _pspi->transfer16(RA8876_SPI_STATUSREAD16);
  endSend(finalize);
  return data;
}

//**************************************************************//
// Write Data to a RA8876 register
//**************************************************************//
void RA8876_t3::lcdRegDataWrite(ru8 reg, ru8 data, bool finalize)
{
  //write the register we wish to write to, then send the data
  //don't need to release _CS between the two transfers
  //ru16 _reg = (RA8876_SPI_CMDWRITE16 | reg);
  //ru16 _data = (RA8876_SPI_DATAWRITE16 | data);
  uint8_t buf[4] = {RA8876_SPI_CMDWRITE, reg, RA8876_SPI_DATAWRITE, data };
  startSend();
  //_pspi->transfer16(_reg);
  //_pspi->transfer16(_data);
  _pspi->transfer(buf, nullptr, 4);
  endSend(finalize);
}

//**************************************************************//
// Read a RA8876 register Data
//**************************************************************//
ru8 RA8876_t3::lcdRegDataRead(ru8 reg, bool finalize)
{
  lcdRegWrite(reg, finalize);
  return lcdDataRead();
}

//**************************************************************//
// support SPI interface to write 16bpp data after Regwrite 04h
//**************************************************************//
void RA8876_t3::lcdDataWrite16bbp(ru16 data, bool finalize) 
{
	startSend();
	_pspi->transfer(RA8876_SPI_DATAWRITE);
	_pspi->transfer16(data);
	endSend(finalize);
}

//**************************************************************//
//RA8876 register 
//**************************************************************//
/*[Status Register] bit7  Host Memory Write FIFO full
0: Memory Write FIFO is not full.
1: Memory Write FIFO is full.
Only when Memory Write FIFO is not full, MPU may write another one pixel.*/ 
//**************************************************************//
void RA8876_t3::checkWriteFifoNotFull(void)
{  ru16 i;  
   for(i=0;i<10000;i++) //Please according to your usage to modify i value.
   {
    if( (lcdStatusRead()&0x80)==0 ){break;}
   }
}

//**************************************************************//
/*[Status Register] bit6  Host Memory Write FIFO empty
0: Memory Write FIFO is not empty.
1: Memory Write FIFO is empty.
When Memory Write FIFO is empty, MPU may write 8bpp data 64
pixels, or 16bpp data 32 pixels, 24bpp data 16 pixels directly.*/
//**************************************************************//
void RA8876_t3::checkWriteFifoEmpty(void)
{ ru16 i;
   for(i=0;i<10000;i++)   //Please according to your usage to modify i value.
   {
    if( (lcdStatusRead()&0x40)==0x40 ){break;}
   }
}

//**************************************************************//
/*[Status Register] bit5  Host Memory Read FIFO full
0: Memory Read FIFO is not full.
1: Memory Read FIFO is full.
When Memory Read FIFO is full, MPU may read 8bpp data 32
pixels, or 16bpp data 16 pixels, 24bpp data 8 pixels directly.*/
//**************************************************************//
void RA8876_t3::checkReadFifoNotFull(void)
{ ru16 i;
  for(i=0;i<10000;i++)  //Please according to your usage to modify i value.
  {if( (lcdStatusRead()&0x20)==0x00 ){break;}}
}

//**************************************************************//
/*[Status Register] bit4   Host Memory Read FIFO empty
0: Memory Read FIFO is not empty.
1: Memory Read FIFO is empty.*/
//**************************************************************//
void RA8876_t3::checkReadFifoNotEmpty(void)
{ ru16 i;
  for(i=0;i<10000;i++)// //Please according to your usage to modify i value. 
  {if( (lcdStatusRead()&0x10)==0x00 ){break;}}
}

//**************************************************************//
/*[Status Register] bit3   Core task is busy
Following task is running:
BTE, Geometry engine, Serial flash DMA, Text write or Graphic write
0: task is done or idle.   1: task is busy
A typical task like drawing a rectangle might take 30-150 microseconds, 
    depending on size
A large filled rectangle might take 3300 microseconds
*****************************************************************/
void RA8876_t3::check2dBusy(void)  
{  ru32 i; 
   for(i=0;i<50000;i++)   //Please according to your usage to modify i value.
   { 
   delayMicroseconds(1);
    if( (lcdStatusRead()&0x08)==0x00 )
    {return;}
   }
   Serial.println("2D ready failed");
}  


//**************************************************************//
/*[Status Register] bit2   SDRAM ready for access
0: SDRAM is not ready for access   1: SDRAM is ready for access*/	
//**************************************************************//
boolean RA8876_t3::checkSdramReady(void)
{ru32 i;
 for(i=0;i<1000000;i++) //Please according to your usage to modify i value.
 { 
   delayMicroseconds(1);
   if( (lcdStatusRead()&0x04)==0x04 )
    {return true;}
    
 }
 return false;
}

//**************************************************************//
/*[Status Register] bit1  Operation mode status
0: Normal operation state  1: Inhibit operation state
Inhibit operation state means internal reset event keep running or
initial display still running or chip enter power saving state.	*/
//**************************************************************//
boolean RA8876_t3::checkIcReady(void)
{ru32 i;
  for(i=0;i<1000000;i++)  //Please according to your usage to modify i value.
   {
     delayMicroseconds(1);
     if( (lcdStatusRead()&0x02)==0x00 )
    {return true;}     
   }
   return false;
}

//**************************************************************//
// Initialize PLL
//**************************************************************//
//[05h] [06h] [07h] [08h] [09h] [0Ah]
//------------------------------------//----------------------------------*/
boolean RA8876_t3::ra8876PllInitial(void) 
{
/*(1) 10MHz <= OSC_FREQ <= 15MHz 
  (2) 10MHz <= (OSC_FREQ/PLLDIVM) <= 40MHz
  (3) 250MHz <= [OSC_FREQ/(PLLDIVM+1)]x(PLLDIVN+1) <= 600MHz
PLLDIVM:0
PLLDIVN:1~63
PLLDIVK:CPLL & MPLL = 1/2/4/8.SPLL = 1/2/4/8/16/32/64/128.
ex:
 OSC_FREQ = 10MHz
 Set X_DIVK=2
 Set X_DIVM=0
 => (X_DIVN+1)=(XPLLx4)/10*/
//ru16 x_Divide,PLLC1,PLLC2;
//ru16 pll_m_lo, pll_m_hi;
//ru8 temp;

	// Set tft output pixel clock
		if(SCAN_FREQ>=79)								//&&(SCAN_FREQ<=100))
		{
			lcdRegDataWrite(0x05,0x04);				//PLL Divided by 4
			lcdRegDataWrite(0x06,(SCAN_FREQ*4/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>=63)&&(SCAN_FREQ<=78))
		{
			lcdRegDataWrite(0x05,0x05);				//PLL Divided by 4
			lcdRegDataWrite(0x06,(SCAN_FREQ*8/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>=40)&&(SCAN_FREQ<=62))
		{								  	
			lcdRegDataWrite(0x05,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x06,(SCAN_FREQ*8/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>=32)&&(SCAN_FREQ<=39))
		{								  	
			lcdRegDataWrite(0x05,0x07);				//PLL Divided by 8
			lcdRegDataWrite(0x06,(SCAN_FREQ*16/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>=16)&&(SCAN_FREQ<=31))
		{								  	
			lcdRegDataWrite(0x05,0x16);				//PLL Divided by 16
			lcdRegDataWrite(0x06,(SCAN_FREQ*16/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>=8)&&(SCAN_FREQ<=15))
		{
			lcdRegDataWrite(0x05,0x26);				//PLL Divided by 32
			lcdRegDataWrite(0x06,(SCAN_FREQ*32/OSC_FREQ)-1);
		}
		else if((SCAN_FREQ>0)&&(SCAN_FREQ<=7))
		{
			lcdRegDataWrite(0x05,0x36);				//PLL Divided by 64
//			lcdRegDataWrite(0x06,(SCAN_FREQ*64/OSC_FREQ)-1);
			lcdRegDataWrite(0x06,((SCAN_FREQ*64/OSC_FREQ)-1) & 0x3f);
		}								    
		// Set internal Buffer Ram clock
		if(DRAM_FREQ>=158)							//
		{
			lcdRegDataWrite(0x07,0x02);				//PLL Divided by 4
			lcdRegDataWrite(0x08,(DRAM_FREQ*2/OSC_FREQ)-1);
		}
		else if((DRAM_FREQ>=125)&&(DRAM_FREQ<=157))							
		{
			lcdRegDataWrite(0x07,0x03);				//PLL Divided by 4
			lcdRegDataWrite(0x08,(DRAM_FREQ*4/OSC_FREQ)-1);
		}
		else if((DRAM_FREQ>=79)&&(DRAM_FREQ<=124))					
		{
			lcdRegDataWrite(0x07,0x04);				//PLL Divided by 4
			lcdRegDataWrite(0x08,(DRAM_FREQ*4/OSC_FREQ)-1);
		}
		else if((DRAM_FREQ>=63)&&(DRAM_FREQ<=78))					
		{
			lcdRegDataWrite(0x07,0x05);				//PLL Divided by 4
			lcdRegDataWrite(0x08,(DRAM_FREQ*8/OSC_FREQ)-1);
		}
		else if((DRAM_FREQ>=40)&&(DRAM_FREQ<=62))
		{								  	
			lcdRegDataWrite(0x07,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x08,(DRAM_FREQ*8/OSC_FREQ)-1);
		}
		else if((DRAM_FREQ>=32)&&(DRAM_FREQ<=39))
		{								  	
			lcdRegDataWrite(0x07,0x07);				//PLL Divided by 16
			lcdRegDataWrite(0x08,(DRAM_FREQ*16/OSC_FREQ)-1);
		}
		else if(DRAM_FREQ<=31)
		{
			lcdRegDataWrite(0x07,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x08,(30*8/OSC_FREQ)-1);	//set to 30MHz if out off range
		}
		// Set Core clock
		if(CORE_FREQ>=158)
		{
			lcdRegDataWrite(0x09,0x02);				//PLL Divided by 2
			lcdRegDataWrite(0x0A,(CORE_FREQ*2/OSC_FREQ)-1);
		}
		else if((CORE_FREQ>=125)&&(CORE_FREQ<=157))
		{
			lcdRegDataWrite(0x09,0x03);				//PLL Divided by 4
			lcdRegDataWrite(0x0A,(CORE_FREQ*4/OSC_FREQ)-1);
		}
		else if((CORE_FREQ>=79)&&(CORE_FREQ<=124))					
		{
			lcdRegDataWrite(0x09,0x04);				//PLL Divided by 4
			lcdRegDataWrite(0x0A,(CORE_FREQ*4/OSC_FREQ)-1);
		}
		else if((CORE_FREQ>=63)&&(CORE_FREQ<=78))					
		{
			lcdRegDataWrite(0x09,0x05);				//PLL Divided by 8
			lcdRegDataWrite(0x0A,(CORE_FREQ*8/OSC_FREQ)-1);
		}
		else if((CORE_FREQ>=40)&&(CORE_FREQ<=62))
		{								  	
			lcdRegDataWrite(0x09,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x0A,(CORE_FREQ*8/OSC_FREQ)-1);
		}
		else if((CORE_FREQ>=32)&&(CORE_FREQ<=39))
		{								  	
			lcdRegDataWrite(0x09,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x0A,(CORE_FREQ*8/OSC_FREQ)-1);
		}
		else if(CORE_FREQ<=31)
		{
			lcdRegDataWrite(0x09,0x06);				//PLL Divided by 8
			lcdRegDataWrite(0x0A,(30*8/OSC_FREQ)-1);	//set to 30MHz if out off range
		}

	delay(1);
	lcdRegWrite(0x01);
	delay(2);
	lcdDataWrite(0x80);
	delay(2); //wait for pll stable
	if((lcdDataRead()&0x80)==0x80)
		return true;
	else
		return false; 
}

//**************************************************************//
// Initialize SDRAM
//**************************************************************//
boolean RA8876_t3::ra8876SdramInitial(void)
{
ru8	CAS_Latency;
ru16	Auto_Refresh;

#ifdef IS42SM16160D
  if(DRAM_FREQ<=133)	
  CAS_Latency=2;
  else 				
  CAS_Latency=3;

  Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0xf9);        
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x09);
 #endif

 #ifdef IS42S16320B	
  if(DRAM_FREQ<=133)	
  CAS_Latency=2;
  else 				
  CAS_Latency=3;	
  
  Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x32);	
  lcdRegDataWrite(0xe1,CAS_Latency);
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x09);
 #endif

#ifdef IS42S16400F
  if(DRAM_FREQ<143)	
  CAS_Latency=2;
  else 
  CAS_Latency=3;
  
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x28);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif

 #ifdef M12L32162A
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x08);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x09);
 #endif

 #ifdef M12L2561616A
  CAS_Latency=3;	
  Auto_Refresh=(64*DRAM_FREQ*1000)/(8192);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x31);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif

 #ifdef M12L64164A
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x28);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x09);
 #endif

 #ifdef W9825G6JH
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x31);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif

 #ifdef W9812G6JH
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);	
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x29);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif

 #ifdef MT48LC4M16A
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
	Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x28);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif

 #ifdef K4S641632N
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x28);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
#endif

#ifdef K4S281632K
  CAS_Latency=3;
  Auto_Refresh=(64*DRAM_FREQ*1000)/(4096);	
  Auto_Refresh=Auto_Refresh-2; 
  lcdRegDataWrite(0xe0,0x29);      
  lcdRegDataWrite(0xe1,CAS_Latency);      //CAS:2=0x02，CAS:3=0x03
  lcdRegDataWrite(0xe2,Auto_Refresh);
  lcdRegDataWrite(0xe3,Auto_Refresh>>8);
  lcdRegDataWrite(0xe4,0x01);
 #endif
  return checkSdramReady();
}

//**************************************************************//
// Turn Display ON/Off (true = ON)
//**************************************************************//
void RA8876_t3::displayOn(boolean on)
{
	unsigned char temp;
	
	// Maybe preserve some of the bits. 
	temp = lcdRegDataRead(RA8876_DPCR) & (1 << 3);

  if(on)
   lcdRegDataWrite(RA8876_DPCR, XPCLK_INV<<7|RA8876_DISPLAY_ON<<6|RA8876_OUTPUT_RGB|temp);
  else
   lcdRegDataWrite(RA8876_DPCR, XPCLK_INV<<7|RA8876_DISPLAY_OFF<<6|RA8876_OUTPUT_RGB|temp);
   
  delay(20);
 }

//**************************************************************//
// Turn Backlight ON/Off (true = ON)
//**************************************************************//
void RA8876_t3::backlight(boolean on)
{

  if(on) {
	//Enable_PWM0_Interrupt();
	//Clear_PWM0_Interrupt_Flag();
 	//Mask_PWM0_Interrupt_Flag();
	//Select_PWM0_Clock_Divided_By_2();
 	//Select_PWM0();
 	pwm_ClockMuxReg(0, RA8876_PWM_TIMER_DIV2, 0, RA8876_XPWM0_OUTPUT_PWM_TIMER0);
 	//Enable_PWM0_Dead_Zone();
	//Auto_Reload_PWM0();
	//Start_PWM0();
	pwm_Configuration(RA8876_PWM_TIMER1_INVERTER_OFF, RA8876_PWM_TIMER1_AUTO_RELOAD,RA8876_PWM_TIMER1_STOP,
					RA8876_PWM_TIMER0_DEAD_ZONE_ENABLE, RA8876_PWM_TIMER1_INVERTER_OFF,
					RA8876_PWM_TIMER0_AUTO_RELOAD, RA8876_PWM_TIMER0_START);

	pwm0_Duty(0xffff);

  }
  else 
  {
	pwm_Configuration(RA8876_PWM_TIMER1_INVERTER_OFF, RA8876_PWM_TIMER1_AUTO_RELOAD,RA8876_PWM_TIMER1_STOP,
					RA8876_PWM_TIMER0_DEAD_ZONE_ENABLE, RA8876_PWM_TIMER1_INVERTER_OFF,
					RA8876_PWM_TIMER0_AUTO_RELOAD, RA8876_PWM_TIMER0_STOP);

  }

}

//**************************************************************//
//**************************************************************//
void RA8876_t3::lcdHorizontalWidthVerticalHeight(ru16 width,ru16 height)
{
	unsigned char temp;
	temp=(width/8)-1;
	lcdRegDataWrite(RA8876_HDWR,temp);
	temp=width%8;
	lcdRegDataWrite(RA8876_HDWFTR,temp);
	temp=height-1;
	lcdRegDataWrite(RA8876_VDHR0,temp);
	temp=(height-1)>>8;
	lcdRegDataWrite(RA8876_VDHR1,temp);
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::lcdHorizontalNonDisplay(ru16 numbers)
{
	ru8 temp;
	if(numbers<8)
	{
		lcdRegDataWrite(RA8876_HNDR,0x00);
		lcdRegDataWrite(RA8876_HNDFTR,numbers);
	} else {
		temp=(numbers/8)-1;
		lcdRegDataWrite(RA8876_HNDR,temp);
		temp=numbers%8;
		lcdRegDataWrite(RA8876_HNDFTR,temp);
	}	
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::lcdHsyncStartPosition(ru16 numbers)
{
	ru8 temp;
	if(numbers<8)
	{
		lcdRegDataWrite(RA8876_HSTR,0x00);
	} else {
		temp=(numbers/8)-1;
		lcdRegDataWrite(RA8876_HSTR,temp);
	}
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::lcdHsyncPulseWidth(ru16 numbers)
{
	ru8 temp;
	if(numbers<8)
	{
		lcdRegDataWrite(RA8876_HPWR,0x00);
	} else {
		temp=(numbers/8)-1;
		lcdRegDataWrite(RA8876_HPWR,temp);
	}
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::lcdVerticalNonDisplay(ru16 numbers)
{
	ru8 temp;
	temp=numbers-1;
	lcdRegDataWrite(RA8876_VNDR0,temp);
	lcdRegDataWrite(RA8876_VNDR1,temp>>8);
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::lcdVsyncStartPosition(ru16 numbers)
{
	ru8 temp;
	temp=numbers-1;
	lcdRegDataWrite(RA8876_VSTR,temp);
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::lcdVsyncPulseWidth(ru16 numbers)
{
	ru8 temp;
	temp=numbers-1;
	lcdRegDataWrite(RA8876_VPWR,temp);
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::displayImageStartAddress(ru32 addr)	
{
	lcdRegDataWrite(RA8876_MISA0,addr);//20h
	lcdRegDataWrite(RA8876_MISA1,addr>>8);//21h 
	lcdRegDataWrite(RA8876_MISA2,addr>>16);//22h  
	lcdRegDataWrite(RA8876_MISA3,addr>>24);//23h 
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::displayImageWidth(ru16 width)	
{
	lcdRegDataWrite(RA8876_MIW0,width); //24h
	lcdRegDataWrite(RA8876_MIW1,width>>8); //25h 
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::displayWindowStartXY(ru16 x0,ru16 y0)	
{
	lcdRegDataWrite(RA8876_MWULX0,x0);//26h
	lcdRegDataWrite(RA8876_MWULX1,x0>>8);//27h
	lcdRegDataWrite(RA8876_MWULY0,y0);//28h
	lcdRegDataWrite(RA8876_MWULY1,y0>>8);//29h
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::canvasImageStartAddress(ru32 addr)	
{
	lcdRegDataWrite(RA8876_CVSSA0,addr);//50h
	lcdRegDataWrite(RA8876_CVSSA1,addr>>8);//51h
	lcdRegDataWrite(RA8876_CVSSA2,addr>>16);//52h
	lcdRegDataWrite(RA8876_CVSSA3,addr>>24);//53h  
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::canvasImageWidth(ru16 width)	
{
	lcdRegDataWrite(RA8876_CVS_IMWTH0,width);//54h
	lcdRegDataWrite(RA8876_CVS_IMWTH1,width>>8); //55h
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::activeWindowXY(ru16 x0,ru16 y0)	
{
	lcdRegDataWrite(RA8876_AWUL_X0,x0);//56h
	lcdRegDataWrite(RA8876_AWUL_X1,x0>>8);//57h 
	lcdRegDataWrite(RA8876_AWUL_Y0,y0);//58h
	lcdRegDataWrite(RA8876_AWUL_Y1,y0>>8);//59h 
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::activeWindowWH(ru16 width,ru16 height)	
{
	lcdRegDataWrite(RA8876_AW_WTH0,width);//5ah
	lcdRegDataWrite(RA8876_AW_WTH1,width>>8);//5bh
	lcdRegDataWrite(RA8876_AW_HT0,height);//5ch
	lcdRegDataWrite(RA8876_AW_HT1,height>>8);//5dh  
}

//**************************************************************//
//**************************************************************//
void  RA8876_t3::setPixelCursor(ru16 x,ru16 y)
{
	switch (_rotation) {
		case 1: swapvals(x,y); break;
		case 2: x = _width-x; break;
		case 3: rotateCCXY(x,y); break;
	}
	lcdRegDataWrite(RA8876_CURH0,x); //5fh
	lcdRegDataWrite(RA8876_CURH1,x>>8);//60h
	lcdRegDataWrite(RA8876_CURV0,y);//61h
	lcdRegDataWrite(RA8876_CURV1,y>>8);//62h
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::linearAddressSet(ru32 addr)	
{
	lcdRegDataWrite(RA8876_CURH0,addr); //5fh
	lcdRegDataWrite(RA8876_CURH1,addr>>8); //60h
	lcdRegDataWrite(RA8876_CURV0,addr>>16); //61h
	lcdRegDataWrite(RA8876_CURV1,addr>>24); //62h
}

//**************************************************************//
//**************************************************************//
ru8 RA8876_t3::vmemReadData(ru32 addr)	
{
	ru8 vmemData = 0;
  
	graphicMode(true);
	Memory_Linear_Mode();
	linearAddressSet(addr);
	ramAccessPrepare();
	vmemData = lcdDataRead(); // dummyread to alert SPI
	vmemData = lcdDataRead(); // read byte
	check2dBusy();
	Memory_XY_Mode();
	return vmemData;
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::vmemWriteData(ru32 addr, ru8 vmemData)	
{
	graphicMode(true);
	Memory_Linear_Mode();
	linearAddressSet(addr);
	ramAccessPrepare();
//  checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
	lcdDataWrite(vmemData);
//  checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
	graphicMode(false);
	Memory_XY_Mode();
	graphicMode(false);
}

//**************************************************************//
//**************************************************************//
ru16 RA8876_t3::vmemReadData16(ru32 addr)	
{
	ru16 vmemData = 0;
  
	vmemData = (vmemReadData(addr) & 0xff); // lo byte
	vmemData |= vmemReadData(addr+1) << 8; // hi byte
	return vmemData;
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::vmemWriteData16(ru32 addr, ru16 vmemData)	
{
	vmemWriteData(addr,vmemData); // lo byte
	vmemWriteData(addr+1,vmemData>>8); // hi byte
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::bte_Source0_MemoryStartAddr(ru32 addr)	
{
	lcdRegDataWrite(RA8876_S0_STR0,addr);//93h
	lcdRegDataWrite(RA8876_S0_STR1,addr>>8);//94h
	lcdRegDataWrite(RA8876_S0_STR2,addr>>16);//95h
	lcdRegDataWrite(RA8876_S0_STR3,addr>>24);////96h
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::bte_Source0_ImageWidth(ru16 width)	
{
	lcdRegDataWrite(RA8876_S0_WTH0,width);//97h
	lcdRegDataWrite(RA8876_S0_WTH1,width>>8);//98h
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::bte_Source0_WindowStartXY(ru16 x0,ru16 y0)	
{
	lcdRegDataWrite(RA8876_S0_X0,x0);//99h
	lcdRegDataWrite(RA8876_S0_X1,x0>>8);//9ah
	lcdRegDataWrite(RA8876_S0_Y0,y0);//9bh
	lcdRegDataWrite(RA8876_S0_Y1,y0>>8);//9ch
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::bte_Source1_MemoryStartAddr(ru32 addr)	
{
	lcdRegDataWrite(RA8876_S1_STR0,addr);//9dh
	lcdRegDataWrite(RA8876_S1_STR1,addr>>8);//9eh
	lcdRegDataWrite(RA8876_S1_STR2,addr>>16);//9fh
	lcdRegDataWrite(RA8876_S1_STR3,addr>>24);//a0h
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::bte_Source1_ImageWidth(ru16 width)	
{
	lcdRegDataWrite(RA8876_S1_WTH0,width);//a1h
	lcdRegDataWrite(RA8876_S1_WTH1,width>>8);//a2h
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::bte_Source1_WindowStartXY(ru16 x0,ru16 y0)	
{
	lcdRegDataWrite(RA8876_S1_X0,x0);//a3h
	lcdRegDataWrite(RA8876_S1_X1,x0>>8);//a4h
	lcdRegDataWrite(RA8876_S1_Y0,y0);//a5h
	lcdRegDataWrite(RA8876_S1_Y1,y0>>8);//a6h
}

//**************************************************************//
//**************************************************************//
void  RA8876_t3::bte_DestinationMemoryStartAddr(ru32 addr)	
{
	lcdRegDataWrite(RA8876_DT_STR0,addr);//a7h
	lcdRegDataWrite(RA8876_DT_STR1,addr>>8);//a8h
	lcdRegDataWrite(RA8876_DT_STR2,addr>>16);//a9h
	lcdRegDataWrite(RA8876_DT_STR3,addr>>24);//aah
}

//**************************************************************//
//**************************************************************//
void  RA8876_t3::bte_DestinationImageWidth(ru16 width)	
{
	lcdRegDataWrite(RA8876_DT_WTH0,width);//abh
	lcdRegDataWrite(RA8876_DT_WTH1,width>>8);//ach
}

//**************************************************************//
//**************************************************************//
void  RA8876_t3::bte_DestinationWindowStartXY(ru16 x0,ru16 y0)	
{
	lcdRegDataWrite(RA8876_DT_X0,x0);//adh
	lcdRegDataWrite(RA8876_DT_X1,x0>>8);//aeh
	lcdRegDataWrite(RA8876_DT_Y0,y0);//afh
	lcdRegDataWrite(RA8876_DT_Y1,y0>>8);//b0h
}

//**************************************************************//
//**************************************************************//
void  RA8876_t3::bte_WindowSize(ru16 width, ru16 height)
{
	lcdRegDataWrite(RA8876_BTE_WTH0,width);//b1h
	lcdRegDataWrite(RA8876_BTE_WTH1,width>>8);//b2h
	lcdRegDataWrite(RA8876_BTE_HIG0,height);//b3h
	lcdRegDataWrite(RA8876_BTE_HIG1,height>>8);//b4h
}

//**************************************************************//
// Window alpha allows 2 source images to be blended together
// with a constant blend factor (alpha) over the entire picture.
// Also called "picture mode".
// 
// alpha can be from 0 to 32
//**************************************************************//
void  RA8876_t3::bte_WindowAlpha(ru8 alpha)
{
	lcdRegDataWrite(RA8876_APB_CTRL,alpha);//b5h
}

//**************************************************************//
// These 8 bits determine prescaler value for Timer 0 and 1.*/
// Time base is “Core_Freq / (Prescaler + 1)”*/
//**************************************************************//
void RA8876_t3::pwm_Prescaler(ru8 prescaler)
{
	lcdRegDataWrite(RA8876_PSCLR,prescaler);//84h
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::pwm_ClockMuxReg(ru8 pwm1_clk_div, ru8 pwm0_clk_div, ru8 xpwm1_ctrl, ru8 xpwm0_ctrl)
{
	lcdRegDataWrite(RA8876_PMUXR,pwm1_clk_div<<6|pwm0_clk_div<<4|xpwm1_ctrl<<2|xpwm0_ctrl);//85h
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::pwm_Configuration(ru8 pwm1_inverter,ru8 pwm1_auto_reload,ru8 pwm1_start,ru8 
                       pwm0_dead_zone, ru8 pwm0_inverter, ru8 pwm0_auto_reload,ru8 pwm0_start) {
	lcdRegDataWrite(RA8876_PCFGR,pwm1_inverter<<6|pwm1_auto_reload<<5|pwm1_start<<4|pwm0_dead_zone<<3|
	pwm0_inverter<<2|pwm0_auto_reload<<1|pwm0_start);//86h                
}   

//**************************************************************//
//**************************************************************//
void RA8876_t3::pwm0_Duty(ru16 duty)
{
	lcdRegDataWrite(RA8876_TCMPB0L,duty);//88h 
	lcdRegDataWrite(RA8876_TCMPB0H,duty>>8);//89h 
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::pwm0_ClocksPerPeriod(ru16 clocks_per_period)
{
	lcdRegDataWrite(RA8876_TCNTB0L,clocks_per_period);//8ah
	lcdRegDataWrite(RA8876_TCNTB0H,clocks_per_period>>8);//8bh
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::pwm1_Duty(ru16 duty)
{
	lcdRegDataWrite(RA8876_TCMPB1L,duty);//8ch 
	lcdRegDataWrite(RA8876_TCMPB1H,duty>>8);//8dh
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::pwm1_ClocksPerPeriod(ru16 clocks_per_period)
{
	lcdRegDataWrite(RA8876_TCNTB1L,clocks_per_period);//8eh
	lcdRegDataWrite(RA8876_TCNTB1F,clocks_per_period>>8);//8fh
}

//**************************************************************//
// Call this before reading or writing to SDRAM
//**************************************************************//
void  RA8876_t3::ramAccessPrepare(void)
{
	lcdRegWrite(RA8876_MRWDP); //04h
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::foreGroundColor16bpp(ru16 color, bool finalize)
{
	lcdRegDataWrite(RA8876_FGCR,color>>8, false);//d2h
	lcdRegDataWrite(RA8876_FGCG,color>>3, false);//d3h
	lcdRegDataWrite(RA8876_FGCB,color<<3, finalize);//d4h
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::backGroundColor16bpp(ru16 color, bool finalize)
{
	lcdRegDataWrite(RA8876_BGCR,color>>8, false);//d5h
	lcdRegDataWrite(RA8876_BGCG,color>>3, false);//d6h
	lcdRegDataWrite(RA8876_BGCB,color<<3, finalize);//d7h
}

//***************************************************//
/*                 GRAPHIC FUNCTIONS                 */
//***************************************************//

//**************************************************************//
/* Turn RA8876 graphic mode ON/OFF (True = ON)                  */
/* Inverse of text mode                                         */
//**************************************************************//
void RA8876_t3::graphicMode(boolean on)
{
	if(on)
		lcdRegDataWrite(RA8876_ICR,RA8877_LVDS_FORMAT<<3|RA8876_GRAPHIC_MODE<<2|RA8876_MEMORY_SELECT_IMAGE);//03h  //switch to graphic mode
	else
		lcdRegDataWrite(RA8876_ICR,RA8877_LVDS_FORMAT<<3|RA8876_TEXT_MODE<<2|RA8876_MEMORY_SELECT_IMAGE);//03h  //switch back to text mode

}

//**************************************************************//
/* Read a 16bpp pixel                                           */
//**************************************************************//
ru16 RA8876_t3::readPixel(int16_t x, int16_t y) {
    return getPixel(x, y);
}

ru16 RA8876_t3::getPixel(ru16 x,ru16 y) {
  ru16 rdata = 0;
  selectScreen(currentPage);
  graphicMode(true);
  setPixelCursor(x, y);		          // set memory address
  ramAccessPrepare();			          // Setup SDRAM Access
  lcdDataRead();
  rdata = (lcdDataRead() & 0xff);		// read low byte
  rdata |= lcdDataRead() << 8;	    // add high byte 
 	return rdata;
}

void RA8876_t3::readRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pcolors) {
  for(uint16_t j = y; j < (h + y); j++) {
    for(uint16_t i = x; i < (w + x); i++) {
      *pcolors++ = getPixel(i, j);
    }
  }
}

//**************************************************************//
/* Write a 16bpp pixel                                          */
//**************************************************************//
void  RA8876_t3::drawPixel(ru16 x,ru16 y,ru16 color)
{
	graphicMode(true);
	setPixelCursor(x,y);
	ramAccessPrepare();
	lcdDataWrite(color);
	lcdDataWrite(color>>8);
	//lcdDataWrite16bbp(color);
}

//**************************************************************//
/* Write 16bpp(RGB565) picture data for user operation          */
/* Not recommended for future use - use BTE instead             */
//**************************************************************//
void  RA8876_t3::putPicture_16bpp(ru16 x,ru16 y,ru16 width, ru16 height)
{
	graphicMode(true);
	activeWindowXY(x,y);
	activeWindowWH(width,height);
	setPixelCursor(x,y);
	ramAccessPrepare();
    // Now your program has to send the image data pixels
}

//*******************************************************************//
/* write 16bpp(RGB565) picture data in byte format from data pointer */
/* Not recommended for future use - use BTE instead                  */
//*******************************************************************//
void  RA8876_t3::putPicture_16bppData8(ru16 x,ru16 y,ru16 width, ru16 height, const unsigned char *data)
{
	ru16 i,j;
	graphicMode(true);
	activeWindowXY(x,y);
	activeWindowWH(width,height);
	setPixelCursor(x,y);
	ramAccessPrepare();
	for(j=0;j<height;j++) {
		for(i=0;i<width;i++) {
			//checkWriteFifoNotFull();	//if high speed mcu and without Xnwait check
			lcdDataWrite(*data);
			data++;
			//checkWriteFifoNotFull();	//if high speed mcu and without Xnwait check
			lcdDataWrite(*data);
			data++;
		}
	} 
	checkWriteFifoEmpty();				//if high speed mcu and without Xnwait check
	activeWindowXY(0,0);
    activeWindowWH(_width,_height);
}

//****************************************************************//
/* Write 16bpp(RGB565) picture data word format from data pointer */
/* Not recommended for future use - use BTE instead               */
//****************************************************************//
void  RA8876_t3::putPicture_16bppData16(ru16 x,ru16 y,ru16 width, ru16 height, const unsigned short *data)
{
	ru16 i,j;
	putPicture_16bpp(x, y, width, height);
	for(j=0;j<height;j++) {
		for(i=0;i<width;i++) {
			//checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
			lcdDataWrite16bbp(*data);
			data++;
			//checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
		}
	} 
	checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
	activeWindowXY(0,0);
	activeWindowWH(_width,_height);
}

//***************************************************//
/*                 TEXT FUNCTIONS                    */
//***************************************************//

//**************************************************************//
/* Turn on RA8876 Text Mode, (Same Graphics Mode in reverse)    */
//**************************************************************//
void RA8876_t3::textMode(boolean on)
{
	if(on)
		lcdRegDataWrite(RA8876_ICR,RA8877_LVDS_FORMAT<<3|RA8876_TEXT_MODE<<2|RA8876_MEMORY_SELECT_IMAGE);//03h  //switch to text mode
	else
		lcdRegDataWrite(RA8876_ICR,RA8877_LVDS_FORMAT<<3|RA8876_GRAPHIC_MODE<<2|RA8876_MEMORY_SELECT_IMAGE);//03h  //switch back to graphic mode
}

//**************************************************************//
/* Set text Foreground and Bckground colors (16 bit)            */
//**************************************************************//
void RA8876_t3::textColor(ru16 foreground_color,ru16 background_color)
{
	check2dBusy();
	foreGroundColor16bpp(foreground_color, false);
	backGroundColor16bpp(background_color, true);
	_TXTForeColor = foreground_color;
	_TXTBackColor = background_color;
}

//**************************************************************//
/* Position Text Cursor                                         */
/* in pixel coordinates                                         */
//**************************************************************//
void  RA8876_t3::setTextCursor(ru16 x,ru16 y)
{
	lcdRegDataWrite(RA8876_F_CURX0,x, false); //63h
	lcdRegDataWrite(RA8876_F_CURX1,x>>8, false);//64h
	lcdRegDataWrite(RA8876_F_CURY0,y, false);//65h
	lcdRegDataWrite(RA8876_F_CURY1,y>>8, true);//66h
	_cursorX = x; _cursorY = y; // Update global cursor position variables
}

//***************************************************************//
/* Position text cursor in character units                       */
//***************************************************************//
void RA8876_t3::textxy(ru16 x, ru16 y)
{
	x += (_scrollXL / (_FNTwidth  * _scaleX));
	y += (_scrollYT / (_FNTheight * _scaleY));
	if(x > (_scrollXR / (_FNTwidth  * _scaleX)))
		x = _scrollXR / (_FNTwidth  * _scaleX)-1;
	if(y > (_scrollYB / (_FNTheight  * _scaleY)))
		y = _scrollYB / (_FNTheight  * _scaleY)-1;
	setTextCursor(x * (_FNTwidth  * _scaleX), y * (_FNTheight * _scaleY));
}

//**************************************************************//
/* source_select = 0 : internal CGROM,  source_select = 1: external CGROM, source_select = 2: user-define*/
/* size_select = 0 : 8*16/16*16, size_select = 1 : 12*24/24*24, size_select = 2 : 16*32/32*32  */
/* iso_select = 0 : iso8859-1, iso_select = 1 : iso8859-2, iso_select = 2 : iso8859-4, iso_select = 3 : iso8859-5*/
//**************************************************************//
void RA8876_t3::setTextParameter1(ru8 source_select,ru8 size_select,ru8 iso_select)
{
	lcdRegDataWrite(RA8876_CCR0,source_select<<6|size_select<<4|iso_select); //cch
}

//***********************************************************************************************//
/* align = 0 : full alignment disable, align = 1 : full alignment enable                         */
/* chroma_key = 0 : text with chroma key disable, chroma_key = 1 : text with chroma key enable   */
/* width_enlarge and height_enlarge can be set 0~3, (00b: X1) (01b : X2)  (10b : X3)  (11b : X4) */
//***********************************************************************************************//
void RA8876_t3::setTextParameter2(ru8 align, ru8 chroma_key, ru8 width_enlarge, ru8 height_enlarge)
{
	lcdRegDataWrite(RA8876_CCR1,align<<7|chroma_key<<6|width_enlarge<<2|height_enlarge);//cdh
}

//**************************************************************//
/* Setup SPI to read character rom selected font set            */
//**************************************************************//
void RA8876_t3::genitopCharacterRomParameter(ru8 scs_select, ru8 clk_div, ru8 rom_select, ru8 character_select, ru8 gt_width)
{ 
  if(scs_select==0)
  lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT0<<7|RA8876_SERIAL_FLASH_FONT_MODE<<6|RA8876_SERIAL_FLASH_ADDR_24BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
  if(scs_select==1)
  lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT1<<7|RA8876_SERIAL_FLASH_FONT_MODE<<6|RA8876_SERIAL_FLASH_ADDR_24BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
  
  lcdRegDataWrite(RA8876_SPI_DIVSOR,clk_div);//bbh 
  
  lcdRegDataWrite(RA8876_GTFNT_SEL,rom_select<<5);//ceh
  lcdRegDataWrite(RA8876_GTFNT_CR,character_select<<3|gt_width);//cfh
}

//**************************************************************//
/* Print a string using internal font or external font code     */
//**************************************************************//
void  RA8876_t3::putString(ru16 x0,ru16 y0, const char *str)
{
  textMode(true);
  while(*str != '\0')
  {
    write(*str);
    ++str; 
  } 
}

//**************************************************************//
/* Clear the status line                                        */
//**************************************************************//
void RA8876_t3::clearStatusLine(uint16_t color) {
	int temp_height = _height-STATUS_LINE_HEIGHT-1;
	drawSquareFill(0,temp_height,_width,_height-1,color);
    check2dBusy();
}

//**************************************************************//
/* Write a string to status line using a constant 8x16 font     */
/* x0 is starting position of string                            */
/* fgcolr is foreground color                                   */
/* bgcolor is background color                                  */
/* str is a pointer to a text string                            */
//**************************************************************//
void  RA8876_t3::writeStatusLine(ru16 x0, uint16_t fgcolor, uint16_t bgcolor, const char *str)
{
	uint16_t tempX = _cursorX;
	uint16_t tempY = _cursorY;
	uint16_t tempBGColor = _TXTBackColor;
	uint16_t tempFGColor = _TXTForeColor;
	uint8_t tempScaleX = _scaleX;
	uint8_t tempScaleY = _scaleY;
	uint8_t tempFontWidth = _FNTwidth;
	uint8_t tempFontHeight = _FNTheight;
	uint16_t temp_height = _height;
	_height = SCREEN_HEIGHT-STATUS_LINE_HEIGHT;

	// Set fontsize to a constant value
	if(UDFont) {
		_scaleX = _scaleY = 1;
		_FNTwidth = 8;
		_FNTheight = 16;
	}// else {
//		_FNTwidth = 12;
//		_FNTheight = 24;
//	}

	buildTextScreen();
	textMode(true);
	setTextCursor(x0, _height);
	textColor(fgcolor,bgcolor);
	if(!UDFont) {
		ramAccessPrepare();
		while(*str != '\0')
		{
			checkWriteFifoNotFull();  
			lcdDataWrite(*str);
			++str; 
		} 
		check2dBusy();
	}
	if(UDFont) {
		_scaleX = _scaleY = 1;
		while(*str != '\0')
		{
			CGRAM_Start_address(PATTERN1_RAM_START_ADDR);
			ramAccessPrepare();
			checkWriteFifoNotFull();  
			lcdDataWrite(*str>>8);
			checkWriteFifoNotFull();  
			lcdDataWrite(*str);
			++str; 
			x0+=_FNTwidth*_scaleX;
		} 
		check2dBusy();
	}
	_scaleX = tempScaleX;
	_scaleY = tempScaleY;
	_FNTwidth = tempFontWidth;
	_FNTheight = tempFontHeight;
	_height = temp_height;
	buildTextScreen();
	textMode(false);
	setTextCursor(tempX,tempY);
	textColor(tempFGColor,tempBGColor);
}

// Support function for VT100: Clear to End Of Line
void RA8876_t3::clreol(void)
{
	uint16_t tempX = _cursorX;
	uint16_t tempY = _cursorY;
	drawSquareFill(tempX, _cursorY, _width-1, _cursorY+(_FNTheight*_scaleY), _TXTBackColor);
	setTextCursor(tempX,tempY);
	textColor(_TXTForeColor,_TXTBackColor);
}

// Support function for VT100: Clear to End Of Screen
void RA8876_t3::clreos(void)
{
	uint16_t tempX = _cursorX;
	uint16_t tempY = _cursorY;
	clreol();
	drawSquareFill(0, _cursorY+(_FNTheight*_scaleY), _width-1, _height-1, _TXTBackColor);
	setTextCursor(tempX,tempY);
	textColor(_TXTForeColor,_TXTBackColor);
}

// Support function for VT100: Clear to End Of Line
void RA8876_t3::clrbol(void)
{
	uint16_t tempX = _cursorX;
	uint16_t tempY = _cursorY;
	drawSquareFill(0, _cursorY, tempX, _cursorY+(_FNTheight*_scaleY), _TXTBackColor);
	setTextCursor(tempX,tempY);
	textColor(_TXTForeColor,_TXTBackColor);
}

// Support function for VT100: Clear to begining of Screen
void RA8876_t3::clrbos(void)
{
	uint16_t tempX = _cursorX;
	uint16_t tempY = _cursorY;
	clrbol();
	drawSquareFill(0, 0, _width-1, _cursorY, _TXTBackColor);
	setTextCursor(tempX,tempY);
	textColor(_TXTForeColor,_TXTBackColor);
}

// Support function for VT100: Clear Line
void RA8876_t3::clrlin(void)
{
	uint16_t tempX = _cursorX;
	uint16_t tempY = _cursorY;
	setTextCursor(0,tempY);
	drawSquareFill(0, _cursorY, _width-1, _cursorY+(_FNTheight*_scaleY), _TXTBackColor);
	setTextCursor(tempX,tempY);
	textColor(_TXTForeColor,_TXTBackColor);
}

// Clear current Active Screen. Home Cursor. Set default forground and background colors.
void RA8876_t3::clearActiveScreen(void)
{
	drawSquareFill(0, 0, _width-1, _height-1, _TXTBackColor);
	textColor(_TXTForeColor,_TXTBackColor);
	textxy(0,0);
}

//**************************************************************//
/* For control code 7 (BELL Character in tftPrint())            */
//**************************************************************//
extern void tone(uint8_t pin, uint16_t frequency, uint32_t duration);

//**************************************************************//
/* update display coordinates and scroll screen if needed        */
//**************************************************************//
void RA8876_t3::update_xy(void)
{
   if(_cursorY >= _scrollYB) 
   {
      scroll();
      _cursorY -= (_FNTheight * _scaleY);
	  _cursorX = _scrollXL;
   }
   setTextCursor(_cursorX,_cursorY);
}

//**************************************************************//
/* Write character to active text screen                        */
//**************************************************************//
void RA8876_t3::update_tft(uint8_t data)
{
	CGRAM_Start_address(PATTERN1_RAM_START_ADDR);
	textMode(true);
	setTextCursor(_cursorX,_cursorY);
	ramAccessPrepare();
	checkWriteFifoNotFull();  
	if(!UDFont) {
		lcdDataWrite(data);
	}
	if(UDFont) {
		lcdDataWrite(data>>8);
		checkWriteFifoNotFull();  
		lcdDataWrite(data);
	}
	check2dBusy();
}

//**************************************************************//
/* Select RA8876 fonts or user defined fonts                    */
//**************************************************************//
/*
void RA8876_t3::setFontSource(uint8_t source) {
	switch(source) {
		case 0:
			UDFont = false;
			break;
		case 1:
			UDFont = true;
			break;
		default:
			UDFont = false;
	}
}
*/

//********************************************************************//
/* Set up the current active screen based on global setup parameters  */
// for this active screen page                                        */
//********************************************************************//
void RA8876_t3::buildTextScreen(void)
{
	switch(_FNTheight) {
		case 16:
			_fontheight = 0;
			break;
		case 24:
			_fontheight = 1;
			break;
		case 32:
			_fontheight = 2;
			break;
		default:
			break;
	}

	leftmarg =  (uint8_t)(_scrollXL / (_FNTwidth  * _scaleX));
	topmarg =  (uint8_t)(_scrollYT / (_FNTheight  * _scaleY));
	rightmarg =  (uint8_t)(_scrollXR / (_FNTwidth  * _scaleX));
	bottommarg = (uint8_t)(_scrollYB / (_FNTheight * _scaleY));
	if(!UDFont) {
		setTextParameter1(RA8876_SELECT_INTERNAL_CGROM,_fontheight,RA8876_SELECT_8859_1);//cch
		setTextParameter2(RA8876_TEXT_FULL_ALIGN_ENABLE, RA8876_TEXT_CHROMA_KEY_DISABLE,
						  RA8876_TEXT_WIDTH_ENLARGEMENT_X1,RA8876_TEXT_HEIGHT_ENLARGEMENT_X1);//cdh
	} else {
		setTextParameter1(RA8876_SELECT_USER_DEFINED,RA8876_CHAR_HEIGHT_16,RA8876_SELECT_8859_1);//cch
		setTextParameter2(RA8876_TEXT_FULL_ALIGN_DISABLE, RA8876_TEXT_CHROMA_KEY_DISABLE,
						  _scaleX-1, _scaleY-1);//cdh
	}
	genitopCharacterRomParameter(RA8876_SERIAL_FLASH_SELECT0,RA8876_SPI_DIV4,RA8876_GT30L32S4W,
					  RA8876_ASCII, RA8876_GT_FIXED_WIDTH);
//	Text_Cursor_H_V(_cursorXsize,_cursorYsize); 		// Block cusror
	textColor(_TXTForeColor,_TXTBackColor);
}

//********************************************************************//
/* Set Character Generator Starting RAM address (32 bit aligned)      */
/* RA8876 registers[DBh]~[DEh]                                        */
//********************************************************************//
void RA8876_t3::CGRAM_Start_address(uint32_t Addr)
{

	//CGRAM START ADDRESS [31:0] 
	lcdRegDataWrite(RA8876_CGRAM_STR0, Addr);
	lcdRegDataWrite(RA8876_CGRAM_STR1, Addr >> 8);
	lcdRegDataWrite(RA8876_CGRAM_STR2, Addr >> 16);
	lcdRegDataWrite(RA8876_CGRAM_STR3, Addr >> 24);
}

//********************************************************************//
/* Initialize CGRAM with user defined font                            */
//********************************************************************//
void RA8876_t3::CGRAM_initial(uint32_t charAddr, const uint8_t *data, uint16_t count)
{
  uint16_t i;

  graphicMode(true);//switch to graphic mode
  
  lcdRegWrite(RA8876_AW_COLOR);// 5Eh 
  lcdDataWrite(RA8876_CANVAS_LINEAR_MODE<<2|RA8876_CANVAS_COLOR_DEPTH_8BPP); // set memory to 8bpp and linear mode
  linearAddressSet(charAddr); // Set linear address (32 bit)
  //Set the start address for User Define Font, and write data.
  ramAccessPrepare();
   for(i = 0; i < count; i++)
   {
    checkWriteFifoNotFull();
    lcdDataWrite(*data);
    data++;
   }
   checkWriteFifoEmpty(); // If high speed mcu and without Xnwait check
   
  lcdRegWrite(RA8876_AW_COLOR);//5Eh 
  lcdDataWrite(RA8876_CANVAS_BLOCK_MODE<<2|RA8876_CANVAS_COLOR_DEPTH_16BPP);
}


//***************************************************//
/*                 GRAPHIC FUNCTIONS                    */
//***************************************************//

//***************************************************************************//
/* Initialize graphic cursor RAM with 4 available graphic cursor shapes      */
/* These are 8x32 byte arrays, 256 bytes per image * 4 images (2048 bytes)   */  
//***************************************************************************//
void RA8876_t3::Graphic_cursor_initial(void)
{
	unsigned int i ;

    check2dBusy();
	graphicMode(true);
    Memory_Select_Graphic_Cursor_RAM(); 
	// Pen cursor
    Select_Graphic_Cursor_1();
	ramAccessPrepare();
    for(i=0;i<256;i++)
    {					 
     lcdDataWrite(gImage_pen_il[i]);
    }
	// Standard arrow cursor
    Select_Graphic_Cursor_2();
	ramAccessPrepare();
    for(i=0;i<256;i++)
    {					 
     lcdDataWrite(gImage_arrow_il[i]);
    }
	// Busy cursor
     Select_Graphic_Cursor_3();
	 ramAccessPrepare();
     for(i=0;i<256;i++)
     {					 
      lcdDataWrite(gImage_busy_im[i]);
     }
	// Stop cursor
     Select_Graphic_Cursor_4();
     ramAccessPrepare();
     for(i=0;i<256;i++)
     {					 
      lcdDataWrite(gImage_no_im[i]);
     }
	Memory_Select_SDRAM();// Reselect image SDRAM
    // Set default cursor foreground and outline colors
    Set_Graphic_Cursor_Color_1(0xff); // White foreground color
    Set_Graphic_Cursor_Color_2(0x00); // Background outline color
    Graphic_Cursor_XY(0,0);
}

//***************************************************************************//
/* Initialize graphic cursor RAM with user-supplied data                     */
/* These are 8x32 byte arrays, 256 bytes per image * 4 images (2048 bytes)  */  
/*                                                                           */
/* The 4 colors used in the graphic cursor are:   (Binary)                   */
/*       00  Color 1 (usually white)                                         */
/*       01  Color 2 (usually black)                                         */
/*       10  Background (transparent)                                        */
/*       11  Invert background (annoying flashy, don't normally use it)      */
//***************************************************************************//
void RA8876_t3::Upload_Graphic_Cursor(uint8_t cursorNum, uint8_t *data)
{
	unsigned int i ;

    check2dBusy();
	graphicMode(true);
    Memory_Select_Graphic_Cursor_RAM(); 

	// awkward way to select which cursor to upload, but it works
	switch(cursorNum) {
		case 1: 
		    Select_Graphic_Cursor_1();
			break;
		case 2: 
		    Select_Graphic_Cursor_2();
			break;
		case 3: 
		    Select_Graphic_Cursor_3();
			break;
		case 4: 
		    Select_Graphic_Cursor_4();
			break;
	}

	ramAccessPrepare();
    for(i=0;i<256;i++)
    {					 
     lcdDataWrite(data[i], false);
    }
	Memory_Select_SDRAM();// Reselect image SDRAM
    // Don't set colors or position
}

//***************************************************//
/* Select Character Generator RAM                    */
//***************************************************//
void RA8876_t3::Memory_Select_CGRAM(void) // this may not be right
{
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_ICR); //0x03
//    temp &= cClrb1; // manual says this bit is clear
    temp |= cSetb0;
    temp &= cClrb0; // manual says this bit is set ?? Figure 14-10
	lcdRegDataWrite(RA8876_ICR, temp);
}

//***************************************************//
/* Select display buffer RAM                         */
//***************************************************//
void RA8876_t3::Memory_Select_SDRAM(void)
{
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_ICR); //0x03
    temp &= cClrb1;
    temp &= cClrb0;	// B
	lcdRegDataWrite(RA8876_ICR, temp);
}

//***************************************************//
/* Select graphic cursor RAM                         */
//***************************************************//
void RA8876_t3::Memory_Select_Graphic_Cursor_RAM(void)
{
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_ICR); //0x03
    temp |= cSetb1;
    temp &= cClrb0;
	lcdRegDataWrite(RA8876_ICR, temp);
}

//***************************************************//
/* Select RAM X-Y addressing mode                    */
//***************************************************//
void RA8876_t3::Memory_XY_Mode(void)	
{
/*
Canvas addressing mode
	0: Block mode (X-Y coordination addressing)
	1: linear mode
*/
	unsigned char temp;

	temp = lcdRegDataRead(RA8876_AW_COLOR);
	temp &= cClrb2;
	lcdRegDataWrite(RA8876_AW_COLOR, temp);
}

//***************************************************//
/* Select RAM linear addressing mode                 */
//***************************************************//
void RA8876_t3::Memory_Linear_Mode(void)	
{
/*
Canvas addressing mode
	0: Block mode (X-Y coordination addressing)
	1: linear mode
*/
	unsigned char temp;

	temp = lcdRegDataRead(RA8876_AW_COLOR);
	temp |= cSetb2;
	lcdRegDataWrite(RA8876_AW_COLOR, temp);
}

//***************************************************//
/* Enable Graphic Cursor                             */
//***************************************************//
void RA8876_t3::Enable_Graphic_Cursor(void)	
{
/*
Graphic Cursor Enable
	0 : Graphic Cursor disable.
	1 : Graphic Cursor enable.
*/
	unsigned char temp;
	
	temp = lcdRegDataRead(RA8876_GTCCR); //0x3c
	temp |= cSetb4;
	lcdRegDataWrite(RA8876_GTCCR,temp);
}

//***************************************************//
/* Disable Graphic Cursor                            */
//***************************************************//
void RA8876_t3::Disable_Graphic_Cursor(void)	
{
/*
Graphic Cursor Enable
	0 : Graphic Cursor disable.
	1 : Graphic Cursor enable.
*/
	unsigned char temp;
	
	temp = lcdRegDataRead(RA8876_GTCCR); //0x3c
	temp &= cClrb4;
	lcdRegDataWrite(RA8876_GTCCR,temp);
}

//************************************************/
/* Select one of four available graphic cursors  */
//************************************************/
void RA8876_t3::Select_Graphic_Cursor_1(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
	00b : Graphic Cursor Set 1.
	01b : Graphic Cursor Set 2.
	10b : Graphic Cursor Set 3.
	11b : Graphic Cursor Set 4.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_GTCCR); //0x3c
	temp &= cClrb3;
	temp &= cClrb2;
	lcdRegDataWrite(RA8876_GTCCR,temp);
}

//************************************************/
/* Select one of four available graphic cursors  */
//************************************************/
void RA8876_t3::Select_Graphic_Cursor_2(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
	00b : Graphic Cursor Set 1.
	01b : Graphic Cursor Set 2.
	10b : Graphic Cursor Set 3.
	11b : Graphic Cursor Set 4.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_GTCCR); //0x3c
	temp &= cClrb3;
	temp |= cSetb2;
	lcdRegDataWrite(RA8876_GTCCR,temp);
}

//************************************************/
/* Select one of four available graphic cursors  */
//************************************************/
void RA8876_t3::Select_Graphic_Cursor_3(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
	00b : Graphic Cursor Set 1.
	01b : Graphic Cursor Set 2.
	10b : Graphic Cursor Set 3.
	11b : Graphic Cursor Set 4.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_GTCCR); //0x3c
	temp |= cSetb3;
	temp &= cClrb2;
	lcdRegDataWrite(RA8876_GTCCR,temp);
}

//************************************************/
/* Select one of four available graphic cursors  */
//************************************************/
void RA8876_t3::Select_Graphic_Cursor_4(void)	
{
/*
Graphic Cursor Selection Bit
Select one from four graphic cursor types. (00b to 11b)
	00b : Graphic Cursor Set 1.
	01b : Graphic Cursor Set 2.
	10b : Graphic Cursor Set 3.
	11b : Graphic Cursor Set 4.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_GTCCR); //0x3c
	temp |= cSetb3;
	temp |= cSetb2;
	lcdRegDataWrite(RA8876_GTCCR,temp);
}

//************************************************/
/* Position graphic cursor                       */
//************************************************/
void RA8876_t3::Graphic_Cursor_XY(int16_t WX,int16_t HY)
{
/*
REG[40h] Graphic Cursor Horizontal Location[7:0]
REG[41h] Graphic Cursor Horizontal Location[12:8]
REG[42h] Graphic Cursor Vertical Location[7:0]
REG[43h] Graphic Cursor Vertical Location[12:8]
Reference main Window coordinates.
*/	
	gCursorX = WX;
	gCursorY = HY;

	if(WX < 0 && WX > -32) WX = 0; //cursor partially visible off the left/top of the screen is coerced onto the screen
	if(HY < 0 && HY > -32) HY = 0;

	lcdRegDataWrite(RA8876_GCHP0,WX, false);
	lcdRegDataWrite(RA8876_GCHP1,WX>>8, false);

	lcdRegDataWrite(RA8876_GCVP0,HY, false);
	lcdRegDataWrite(RA8876_GCVP1,HY>>8, true);
}

//************************************************/
/* Set graphic cursor foreground color           */
//************************************************/
void RA8876_t3::Set_Graphic_Cursor_Color_1(unsigned char temp)
{
/*
REG[44h] Graphic Cursor Color 0 with 256 Colors
RGB Format [7:0] = RRRGGGBB.
*/	
	lcdRegDataWrite(RA8876_GCC0,temp);
}

//************************************************/
/* Set graphic cursor outline color              */
//************************************************/
void RA8876_t3::Set_Graphic_Cursor_Color_2(unsigned char temp)
{
/*
REG[45h] Graphic Cursor Color 1 with 256 Colors
RGB Format [7:0] = RRRGGGBB.
*/	
	lcdRegDataWrite(RA8876_GCC1,temp);
}

//************************************************/
/* turn on text cursor                           */
//************************************************/
void RA8876_t3::Enable_Text_Cursor(void)	
{
/*
Text Cursor Enable
	0 : Disable.
	1 : Enable.
Text cursor & Graphic cursor cannot enable simultaneously.
Graphic cursor has higher priority then Text cursor if enabled simultaneously.
*/
	unsigned char temp;
	
	temp = lcdRegDataRead(RA8876_GTCCR); // 0x3c
	temp |= cSetb1;
	lcdRegDataWrite(RA8876_GTCCR,temp);
}

//************************************************/
/* turn off text cursor                          */
//************************************************/
void RA8876_t3::Disable_Text_Cursor(void)	
{
/*
Text Cursor Enable
	0 : Disable.
	1 : Enable.
Text cursor & Graphic cursor cannot enable simultaneously.
Graphic cursor has higher priority then Text cursor if enabled simultaneously.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_GTCCR); // 0x3c
	temp &= cClrb1;
	lcdRegDataWrite(RA8876_GTCCR,temp);
}

//************************************************/
/* Turn on blinking text cursor                  */
//************************************************/
void RA8876_t3::Enable_Text_Cursor_Blinking(void)	
{
/*
Text Cursor Blinking Enable
	0 : Disable.
	1 : Enable.
*/
	unsigned char temp;
	
	temp = lcdRegDataRead(RA8876_GTCCR); // 0x3c
	temp |= cSetb0;
	lcdRegDataWrite(RA8876_GTCCR,temp);
}

//************************************************/
/* Turn off blinking text cursor                  */
//************************************************/
void RA8876_t3::Disable_Text_Cursor_Blinking(void)	
{
/*
Text Cursor Blinking Enable
	0 : Disable.
	1 : Enable.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_GTCCR); // 0x3c
	temp &= cClrb0;
	lcdRegDataWrite(RA8876_GTCCR,temp);
}

//************************************************/
/* Set text cursor blink rate                    */
//************************************************/
void RA8876_t3::Blinking_Time_Frames(unsigned char temp)	
{
/*
Text Cursor Blink Time Setting (Unit: Frame)
	00h : 1 frame time.
	01h : 2 frames time.
	02h : 3 frames time.
	:
	FFh : 256 frames time.
*/
	lcdRegDataWrite(RA8876_BTCR,temp); // 0x3d
}

//************************************************/
/* Set text cursor horizontal and vertical size  */
//************************************************/
void RA8876_t3::Text_Cursor_H_V(unsigned short WX,unsigned short HY)
{
/*
REG[3Eh]
Text Cursor Horizontal Size Setting[4:0]
Unit : Pixel
Zero-based number. Value 0 means 1 pixel.
Note : When font is enlarged, the cursor setting will multiply the
same times as the font enlargement.
REG[3Fh]
Text Cursor Vertical Size Setting[4:0]
Unit : Pixel
Zero-based number. Value 0 means 1 pixel.
Note : When font is enlarged, the cursor setting will multiply the
same times as the font enlargement.
*/
	lcdRegDataWrite(RA8876_CURHS,WX, false); // 0x3e
	lcdRegDataWrite(RA8876_CURVS,HY, true); // 0x3f
}

//************************************************/
/* Turn on color bar test screen                 */
//************************************************/
void RA8876_t3::Color_Bar_ON(void)
{
/*	
Display Test Color Bar
	0b: Disable.
	1b: Enable.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_DPCR); // 0x12
	temp |= cSetb5;
	lcdRegDataWrite(RA8876_DPCR,temp); // 0x12
}

//************************************************/
/* Turn off color bar test screen                */
//************************************************/
void RA8876_t3::Color_Bar_OFF(void)
{
/*	
Display Test Color Bar
	0b: Disable.
	1b: Enable.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_DPCR); // 0x12
	temp &= cClrb5;
	lcdRegDataWrite(RA8876_DPCR,temp); // 0x12
}

//************************************************/
/* Drawing Functions                             */
//************************************************/

//**************************************************************//
/* Draw a line                                                  */
/* x0,y0: Line start coords                                     */
/* x1,y1: Line end coords                                       */
//**************************************************************//
void RA8876_t3::drawLine(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color)
{
	x0 += _originx; x1 += _originx;
	y0 += _originy; y1 += _originy;
	
	if ((x0 == x1 && y0 == y1)) {//Thanks MrTOM
		drawPixel(x0,y0,color);
		return;
	}
	
  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
	switch (_rotation) {
		case 1: swapvals(x0,y0); swapvals(x1,y1); break;
		case 2: x0 = _width-x0; x1 = _width-x1;break;
		case 3: rotateCCXY(x0,y0); rotateCCXY(x1,y1); break;
  	}
  lcdRegDataWrite(RA8876_DLHSR0,x0, false);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8, false);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0, false);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8, false);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x1, false);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x1>>8, false);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y1, false);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y1>>8, false);//6fh        
  lcdRegDataWrite(RA8876_DCR0,RA8876_DRAW_LINE, true);//67h,0x80
}


//**************************************************************//
// Draw a rectangle:
// x0,y0 is upper left start
// x1,y1 is lower right end corner
//**************************************************************//
void RA8876_t3::drawSquare(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color)
{
	x0 += _originx; x1 += _originx;
	y1 += _originy; y1 += _originy;
	int16_t x_end = x1;
	int16_t y_end = y1;
	if((x0 >= _displayclipx2)   || // Clip right
		 (y0 >= _displayclipy2) || // Clip bottom
		 (x_end < _displayclipx1)    || // Clip left
		 (y_end < _displayclipy1))  	// Clip top 
	{
		// outside the clip rectangle
		return;
	}
	if (x0 < _displayclipx1) x0 = _displayclipx1;
	if (y0 < _displayclipy1) y0 = _displayclipy1;
	if (x_end > _displayclipx2) x_end = _displayclipx2;
	if (y_end > _displayclipy2) y_end = _displayclipy2;
	
  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
	switch (_rotation) {
		case 1: swapvals(x0,y0); swapvals(x1,y1); break;
		case 2: x0 = _width-x0; x1 = _width-x1;break;
		case 3: rotateCCXY(x0,y0); rotateCCXY(x1,y1); break;
  	}
  lcdRegDataWrite(RA8876_DLHSR0,x0, false);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8, false);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0, false);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8, false);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x_end, false);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x_end>>8, false);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y1, false);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y_end>>8, false);//6fh     
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_SQUARE, true);//76h,0xa0

}

//**************************************************************//
// Draw a filled rectangle:
// x0,y0 is upper left start
// x1,y1 is lower right end corner
//**************************************************************//
void RA8876_t3::drawSquareFill(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color)
{
//  Serial.printf("DSF:(%d %d)(%d %d) %x\n", x0, y0, x1, y1, color);	
	x0 += _originx; x1 += _originx;
	y1 += _originy; y1 += _originy;
	int16_t x_end = x1;
	int16_t y_end = y1;
	if((x0 >= _displayclipx2)   || // Clip right
		 (y0 >= _displayclipy2) || // Clip bottom
		 (x_end < _displayclipx1)    || // Clip left
		 (y_end < _displayclipy1))  	// Clip top 
	{
		// outside the clip rectangle
		return;
	}
	if (x0 < _displayclipx1) x0 = _displayclipx1;
	if (y0 < _displayclipy1) y0 = _displayclipy1;
	if (x_end > _displayclipx2) x_end = _displayclipx2;
	if (y_end > _displayclipy2) y_end = _displayclipy2;

  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
  switch (_rotation) {
		case 1: swapvals(x0,y0); swapvals(x_end,y_end); break;
		case 2: x0 = _width-x0; x_end = _width-x_end; break;
		case 3: rotateCCXY(x0,y0); rotateCCXY(x_end,y_end); break;
  	}
  lcdRegDataWrite(RA8876_DLHSR0,x0, false);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8, false);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0, false);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8, false);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x_end, false);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x_end>>8, false);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y_end, false);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y_end>>8, false);//6fh     
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_SQUARE_FILL, true);//76h,0xE0  
}

//**************************************************************//
// Draw a round corner rectangle:
// x0,y0 is upper left start
// x1,y1 is lower right end corner
// xr is the major radius of corner (horizontal)
// yr is the minor radius of corner (vertical)
//**************************************************************//
void RA8876_t3::drawCircleSquare(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 xr, ru16 yr, ru16 color)
{
	x0 += _originx; x1 += _originx;
	y1 += _originy; y1 += _originy;
	int16_t x_end = x1;
	int16_t y_end = y1;
	if((x0 >= _displayclipx2)   || // Clip right
		 (y0 >= _displayclipy2) || // Clip bottom
		 (x_end < _displayclipx1)    || // Clip left
		 (y_end < _displayclipy1))  	// Clip top 
	{
		// outside the clip rectangle
		return;
	}
	if (x0 < _displayclipx1) x0 = _displayclipx1;
	if (y0 < _displayclipy1) y0 = _displayclipy1;
	if (x_end > _displayclipx2) x_end = _displayclipx2;
	if (y_end > _displayclipy2) y_end = _displayclipy2;	

  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
	switch (_rotation) {
		case 1: swapvals(x0,y0); swapvals(x_end,y_end); swapvals(xr, yr); break;
		case 2: x0 = _width-x0; x_end = _width-x_end;break;
		case 3: rotateCCXY(x0,y0); rotateCCXY(x_end,y_end);  swapvals(xr, yr);break;
  	}
  lcdRegDataWrite(RA8876_DLHSR0,x0, false);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8, false);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0, false);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8, false);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x_end, false);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x_end>>8, false);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y_end, false);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y_end>>8, false);//6fh    
  lcdRegDataWrite(RA8876_ELL_A0,xr, false);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,xr>>8, false);//79h 
  lcdRegDataWrite(RA8876_ELL_B0,yr, false);//7ah    
  lcdRegDataWrite(RA8876_ELL_B1,yr>>8, false);//7bh
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE_SQUARE, true);//76h,0xb0
}

//**************************************************************//
// Draw a filled round corner rectangle:
// x0,y0 is upper left start
// x1,y1 is lower right end corner
// xr is the major radius of corner (horizontal)
// yr is the minor radius of corner (vertical)
//**************************************************************//
void RA8876_t3::drawCircleSquareFill(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 xr, ru16 yr, ru16 color)
{
	x0 += _originx; x1 += _originx;
	y1 += _originy; y1 += _originy;
	int16_t x_end = x1;
	int16_t y_end = y1;
	if((x0 >= _displayclipx2)   || // Clip right
		 (y0 >= _displayclipy2) || // Clip bottom
		 (x_end < _displayclipx1)    || // Clip left
		 (y_end < _displayclipy1))  	// Clip top 
	{
		// outside the clip rectangle
		return;
	}
	if (x0 < _displayclipx1) x0 = _displayclipx1;
	if (y0 < _displayclipy1) y0 = _displayclipy1;
	if (x_end > _displayclipx2) x_end = _displayclipx2;
	if (y_end > _displayclipy2) y_end = _displayclipy2;
	
  check2dBusy();
  //graphicMode(true);
  foreGroundColor16bpp(color);
	switch (_rotation) {
		case 1: swapvals(x0,y0); swapvals(x_end,y_end); swapvals(xr, yr); break;
		case 2: x0 = _width-x0; x_end = _width-x_end;break;
		case 3: rotateCCXY(x0,y0); rotateCCXY(x_end,y_end);  swapvals(xr, yr);break;
  	}
  lcdRegDataWrite(RA8876_DLHSR0,x0, false);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8, false);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0, false);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8, false);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x_end, false);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x_end>>8, false);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y_end, false);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y_end>>8, false);//6fh    
  lcdRegDataWrite(RA8876_ELL_A0,xr, false);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,xr>>8, false);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,yr, false);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,yr>>8, false);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE_SQUARE_FILL, true);//76h,0xf0
}

//**************************************************************//
// Draw a triangle
// x0,y0 is triangle start
// x1,y1 is triangle second point
// x2,y2 is triangle end point
//**************************************************************//
void RA8876_t3::drawTriangle(ru16 x0,ru16 y0,ru16 x1,ru16 y1,ru16 x2,ru16 y2,ru16 color)
{
  x0 += _originx; x1 += _originx; x2 += _originx; 
  y0 += _originy; y1 += _originy; y2 += _originy;
	
	if (x0 >= _width || x1 >= _width || x2 >= _width) return;
	if (y0 >= _height || y1 >= _height || y2 >= _height) return;
	if (x0 == x1 && y0 == y1 && x0 == x2 && y0 == y2) {			// All points are same
		drawPixel(x0,y0, color);
		return;
	}

	switch (_rotation) {
		case 1: swapvals(x0,y0); swapvals(x1,y1); swapvals(x2,y2);  break;
		case 2: x0 = _width-x0; x1 = _width - x1; x2 = _width - x2; break;
		case 3: rotateCCXY(x0,y0); rotateCCXY(x1,y1); rotateCCXY(x2,y2); break;
  	}

  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,x0, false);//68h point 0
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8, false);//69h point 0
  lcdRegDataWrite(RA8876_DLVSR0,y0, false);//6ah point 0
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8, false);//6bh point 0
  lcdRegDataWrite(RA8876_DLHER0,x1, false);//6ch point 1
  lcdRegDataWrite(RA8876_DLHER1,x1>>8, false);//6dh point 1
  lcdRegDataWrite(RA8876_DLVER0,y1, false);//6eh point 1
  lcdRegDataWrite(RA8876_DLVER1,y1>>8, false);//6fh point 1
  lcdRegDataWrite(RA8876_DTPH0,x2, false);//70h point 2
  lcdRegDataWrite(RA8876_DTPH1,x2>>8, false);//71h point 2
  lcdRegDataWrite(RA8876_DTPV0,y2, false);//72h point 2
  lcdRegDataWrite(RA8876_DTPV1,y2>>8, false);//73h  point 2
  lcdRegDataWrite(RA8876_DCR0,RA8876_DRAW_TRIANGLE, false);//67h,0x82
}

//**************************************************************//
// Draw a filled triangle
// x0,y0 is triangle start
// x1,y1 is triangle second point
// x2,y2 is triangle end point
//**************************************************************//
void RA8876_t3::drawTriangleFill(ru16 x0,ru16 y0,ru16 x1,ru16 y1,ru16 x2,ru16 y2,ru16 color)
{
  x0 += _originx; x1 += _originx; x2 += _originx; 
  y0 += _originy; y1 += _originy; y2 += _originy;
	
	if (x0 >= _width || x1 >= _width || x2 >= _width) return;
	if (y0 >= _height || y1 >= _height || y2 >= _height) return;
	if (x0 == x1 && y0 == y1 && x0 == x2 && y0 == y2) {			// All points are same
		drawPixel(x0,y0, color);
		return;
	}
	
	switch (_rotation) {
		case 1: swapvals(x0,y0); swapvals(x1,y1); swapvals(x2,y2);  break;
		case 2: x0 = _width-x0; x1 = _width - x1; x2 = _width - x2; break;
		case 3: rotateCCXY(x0,y0); rotateCCXY(x1,y1); rotateCCXY(x2,y2); break;
  	}

  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
  lcdRegDataWrite(RA8876_DLHSR0,x0, false);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x0>>8, false);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y0, false);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y0>>8, false);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x1, false);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x1>>8, false);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y1, false);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y1>>8, false);//6fh  
  lcdRegDataWrite(RA8876_DTPH0,x2, false);//70h
  lcdRegDataWrite(RA8876_DTPH1,x2>>8, false);//71h
  lcdRegDataWrite(RA8876_DTPV0,y2, false);//72h
  lcdRegDataWrite(RA8876_DTPV1,y2>>8, false);//73h  
  lcdRegDataWrite(RA8876_DCR0,RA8876_DRAW_TRIANGLE_FILL, true);//67h,0xa2
}

//**************************************************************//
// Draw a circle
// x,y is center point of circle
// r is radius of circle
// See page 59 of RA8876.pdf for information on drawing arc's. (1 of 4 quadrants at a time only) 
//**************************************************************//
void RA8876_t3::drawCircle(ru16 x0,ru16 y0,ru16 r,ru16 color)
{
  x0 += _originx;
  y0 += _originy;
  
	if (r < 1) r = 1;
	if (r < 2) {//NEW
		drawPixel(x0,y0,color);
		return;
	}
	if (r > _height / 2) r = (_height / 2) - 1;//this is the (undocumented) hardware limit of RA8875
	
  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
	switch (_rotation) {
		case 1: swapvals(x0,y0);  break;
		case 2: x0 = _width-x0;   break;
		case 3: rotateCCXY(x0,y0);  break;
  	}

  lcdRegDataWrite(RA8876_DEHR0,x0, false);//7bh
  lcdRegDataWrite(RA8876_DEHR1,x0>>8, false);//7ch
  lcdRegDataWrite(RA8876_DEVR0,y0, false);//7dh
  lcdRegDataWrite(RA8876_DEVR1,y0>>8, false);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,r, false);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,r>>8, false);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,r, false);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,r>>8, false);//7ah
//  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_BOTTOM_LEFT_CURVE);//76h,0x90 (arc test)
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE, true);//76h,0x80
}

//**************************************************************//
// Draw a filled circle
// x,y is center point of circle
// r is radius of circle
// See page 59 of RA8876.pdf for information on drawing arc's. (1 of 4 quadrants at a time only)
//**************************************************************//
void RA8876_t3::drawCircleFill(ru16 x0,ru16 y0,ru16 r,ru16 color)
{
  x0 += _originx;
  y0 += _originy;
  
	if (r < 1) r = 1;
	if (r < 2) {//NEW
		drawPixel(x0,y0,color);
		return;
	}
	if (r > _height / 2) r = (_height / 2) - 1;//this is the (undocumented) hardware limit of RA8875
	
  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
	switch (_rotation) {
		case 1: swapvals(x0,y0);  break;
		case 2: x0 = _width-x0;   break;
		case 3: rotateCCXY(x0,y0);  break;
  	}
  lcdRegDataWrite(RA8876_DEHR0,x0, false);//7bh
  lcdRegDataWrite(RA8876_DEHR1,x0>>8, false);//7ch
  lcdRegDataWrite(RA8876_DEVR0,y0, false);//7dh
  lcdRegDataWrite(RA8876_DEVR1,y0>>8, false);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,r, false);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,r>>8, false);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,r, false);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,r>>8, false);//7ah
//  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_BOTTOM_LEFT_CURVE_FILL);//76h,0xd0 (arc test)
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE_FILL, false);//76h,0xc0
}

//**************************************************************//
// Draw an ellipse
// x0,y0 is ellipse start
// xr is ellipse x radius, major axis
// yr is ellipse y radius, minor axis
//**************************************************************//
void RA8876_t3::drawEllipse(ru16 x0,ru16 y0,ru16 xr,ru16 yr,ru16 color)
{
	
  x0 += _originx;
  y0 += _originy;
  
	//if (_portrait) {
	//	swapvals(x0,y0);
	//	swapvals(xr,yr);
	//	if (longAxis > _height/2) longAxis = (_height / 2) - 1;
	//	if (shortAxis > _width/2) shortAxis = (_width / 2) - 1;
	//} else {
		if (xr > _width/2) xr = (_width / 2) - 1;
		if (yr > _height/2) yr = (_height / 2) - 1;
	//}
	if (xr == 1 && yr == 1) {
		drawPixel(x0,y0,color);
		return;
	}
	
	
  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
	switch (_rotation) {
		case 1: swapvals(x0,y0);  swapvals(xr,yr); break;
		case 2: x0 = _width-x0;   break;
		case 3: rotateCCXY(x0,y0); swapvals(xr,yr); break;
  	}
  lcdRegDataWrite(RA8876_DEHR0,x0, false);//7bh
  lcdRegDataWrite(RA8876_DEHR1,x0>>8, false);//7ch
  lcdRegDataWrite(RA8876_DEVR0,y0, false);//7dh
  lcdRegDataWrite(RA8876_DEVR1,y0>>8, false);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,xr, false);//77h  
  lcdRegDataWrite(RA8876_ELL_A1,xr>>8, false);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,yr, false);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,yr>>8, false);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_ELLIPSE, true);//76h,0x80
}

//**************************************************************//
// Draw an filled ellipse
// x0,y0 is ellipse start
// x1,y1 is ellipse x radius
// x2,y2 is ellipse y radius
//**************************************************************//
void RA8876_t3::drawEllipseFill(ru16 x0,ru16 y0,ru16 xr,ru16 yr,ru16 color)
{
  x0 += _originx;
  y0 += _originy;
  
	//if (_portrait) {
	//	swapvals(xCenter,yCenter);
	//	swapvals(longAxis,shortAxis);
	//	if (longAxis > _height/2) longAxis = (_height / 2) - 1;
	//	if (shortAxis > _width/2) shortAxis = (_width / 2) - 1;
	//} else {
		if (xr > _width/2) xr = (_width / 2) - 1;
		if (yr > _height/2) yr = (_height / 2) - 1;
	//}
	if (xr == 1 && yr == 1) {
		drawPixel(x0,y0,color);
		return;
	}
	
  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
	switch (_rotation) {
		case 1: swapvals(x0,y0);  swapvals(xr,yr); break;
		case 2: x0 = _width-x0;   break;
		case 3: rotateCCXY(x0,y0); swapvals(xr,yr); break;
  	}
  lcdRegDataWrite(RA8876_DEHR0,x0, false);//7bh
  lcdRegDataWrite(RA8876_DEHR1,x0>>8, false);//7ch
  lcdRegDataWrite(RA8876_DEVR0,y0, false);//7dh
  lcdRegDataWrite(RA8876_DEVR1,y0>>8, false);//7eh
  lcdRegDataWrite(RA8876_ELL_A0,xr, false);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,xr>>8, false);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,yr, false);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,yr>>8, false);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_ELLIPSE_FILL, true);//76h,0xc0
}

//*************************************************************//
// Scroll Screen up
//*************************************************************//
void RA8876_t3::scroll(void) { // No arguments for now
	bteMemoryCopy(currentPage,SCREEN_WIDTH, _scrollXL, _scrollYT+(_FNTheight*_scaleY),	//Source
				  currentPage,SCREEN_WIDTH, _scrollXL, pageOffset+_scrollYT,	//Desination
				  _scrollXR-_scrollXL, _scrollYB-_scrollYT-(_FNTheight*_scaleY)); //Copy Width, Height
	// Clear bottom text line
	drawSquareFill(_scrollXL, _scrollYB-(_FNTheight*_scaleY), _scrollXR-1, _scrollYB-1, _TXTBackColor);
	textColor(_TXTForeColor,_TXTBackColor);
}

//*************************************************************//
// Scroll Screen down
//*************************************************************//
void RA8876_t3::scrollDown(void) { // No arguments for now
	bteMemoryCopy(currentPage,SCREEN_WIDTH, _scrollXL, _scrollYT,	//Source
				  PAGE10_START_ADDR,SCREEN_WIDTH, _scrollXL, _scrollYT,	//Desination
				  _scrollXR-_scrollXL, (_scrollYB-_scrollYT)-(_FNTheight*_scaleY)); //Copy Width, Height
	bteMemoryCopy(PAGE10_START_ADDR,SCREEN_WIDTH, _scrollXL, _scrollYT,	//Source
				  currentPage,SCREEN_WIDTH, _scrollXL, _scrollYT+(_FNTheight*_scaleY),	//Desination
				  _scrollXR-_scrollXL, (_scrollYB-_scrollYT)-_FNTheight); //Copy Width, Height
	// Clear top text line
	drawSquareFill(_scrollXL, _scrollYT, _scrollXR-1,_scrollYT+(_FNTheight*_scaleY), _TXTBackColor);
	textColor(_TXTForeColor,_TXTBackColor);

}

//*************************************************************//
// Put a section of current screen page to another screen page.
// x0,y0 is upper left start coordinates of section
// x1,y1 is lower right coordinates of section
// dx0,dy0 is start coordinates of destination screen page
//*************************************************************//
uint32_t RA8876_t3::boxPut(uint32_t vPageAddr, uint16_t x0, uint16_t y0,
							 uint16_t x1, uint16_t y1,
							 uint16_t dx0,uint16_t dy0) {

	bteMemoryCopy(currentPage, SCREEN_WIDTH, x0, y0,
    vPageAddr, SCREEN_WIDTH, dx0, dy0,
    x1-x0, y1-y0);
	return vPageAddr;
}

//*************************************************************//
// get a section of another screen page to current screen page.
// x0,y0 is upper left start coordinates of section
// x1,y1 is lower right coordinates of section
// dx0,dy0 is start coordinates of current screen page
//*************************************************************//
uint32_t RA8876_t3::boxGet(uint32_t vPageAddr, uint16_t x0, uint16_t y0,
							 uint16_t x1, uint16_t y1,
							 uint16_t dx0,uint16_t dy0) {

	bteMemoryCopy(vPageAddr, SCREEN_WIDTH, x0, y0,
	              currentPage, SCREEN_WIDTH, dx0, dy0,
	              x1-dx0, y1-dy0);
	return vPageAddr;
}

/*BTE function*/
//**************************************************************//
// Copy from place to place in memory (eg. from a non-displayed page to the current page)
//**************************************************************//
void RA8876_t3::bteMemoryCopy(ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
								ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,
								ru16 copy_width,ru16 copy_height)
{
  check2dBusy();
  graphicMode(true);
  bte_Source0_MemoryStartAddr(s0_addr);
  bte_Source0_ImageWidth(s0_image_width);
  bte_Source0_WindowStartXY(s0_x,s0_y);

//  bte_Source1_MemoryStartAddr(PAGE3_START_ADDR);
//  bte_Source1_ImageWidth(des_image_width);
//  bte_Source1_WindowStartXY(des_x,des_y);

  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(copy_width,copy_height); 

  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_CODE_12<<4|RA8876_BTE_MEMORY_COPY_WITH_ROP);//91h
//  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_CODE_12<<4|3);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,(RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP) & 0x7f);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
} 

//**************************************************************//
// Memory copy with Raster OPeration blend from two sources
// One source may be the destination, if blending with something already on the screen
//**************************************************************//
void RA8876_t3::bteMemoryCopyWithROP(
	ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
	ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,
    ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,
    ru16 copy_width,ru16 copy_height,ru8 rop_code)
{
  check2dBusy();
  graphicMode(true);
  bte_Source0_MemoryStartAddr(s0_addr);
  bte_Source0_ImageWidth(s0_image_width);
  bte_Source0_WindowStartXY(s0_x,s0_y);
  bte_Source1_MemoryStartAddr(s1_addr);
  bte_Source1_ImageWidth(s1_image_width);
  bte_Source1_WindowStartXY(s1_x,s1_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(copy_width,copy_height);
  lcdRegDataWrite(RA8876_BTE_CTRL1,rop_code<<4|RA8876_BTE_MEMORY_COPY_WITH_ROP);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
} 
//**************************************************************//
// Memory copy with one color set to transparent
//**************************************************************//
void RA8876_t3::bteMemoryCopyWithChromaKey(
		ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
		ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,
		ru16 copy_width, ru16 copy_height, ru16 chromakey_color)
{
  check2dBusy();
  graphicMode(true);
  bte_Source0_MemoryStartAddr(s0_addr);
  bte_Source0_ImageWidth(s0_image_width);
  bte_Source0_WindowStartXY(s0_x,s0_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(copy_width,copy_height);
  backGroundColor16bpp(chromakey_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_MEMORY_COPY_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
}
//**************************************************************//
// Blend two source images with a simple transparency 0-32
//**************************************************************//
void RA8876_t3::bteMemoryCopyWindowAlpha(
		ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
		ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,
		ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,
		ru16 copy_width, ru16 copy_height, ru8 alpha)
{
  check2dBusy();
  graphicMode(true);
  bte_Source0_MemoryStartAddr(s0_addr);
  bte_Source0_ImageWidth(s0_image_width);
  bte_Source0_WindowStartXY(s0_x,s0_y);
  bte_Source1_MemoryStartAddr(s1_addr);
  bte_Source1_ImageWidth(s1_image_width);
  bte_Source1_WindowStartXY(s1_x,s1_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(copy_width,copy_height);
  bte_WindowAlpha(alpha);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_MEMORY_COPY_WITH_OPACITY);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
}

//**************************************************************//
// Send data from the microcontroller to the RA8876
// Does a Raster OPeration to combine with an image already in memory
// For a simple overwrite operation, use ROP 12
//**************************************************************//
void RA8876_t3::bteMpuWriteWithROPData8(ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,ru32 des_addr,ru16 des_image_width,
ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru8 rop_code,const unsigned char *data)
{
  bteMpuWriteWithROP(s1_addr, s1_image_width, s1_x, s1_y, des_addr, des_image_width, des_x, des_y, width, height, rop_code);
  
  startSend();
  _pspi->transfer(RA8876_SPI_DATAWRITE);

#ifdef SPI_HAS_TRANSFER_ASYNC
  activeDMA = true;
  _pspi->transfer(data, NULL, width*height*2, finishedDMAEvent);
#else
  //If you try _pspi->transfer(data, length) then this tries to write received data into the data buffer
  //but if we were given a PROGMEM (unwriteable) data pointer then _pspi->transfer will lock up totally.
  //So we explicitly tell it we don't care about any return data.
  _pspi->transfer(data, NULL, width*height*2);
  endSend(true);
#endif
}
//**************************************************************//
// For 16-bit byte-reversed data.
// Note this is 4-5 milliseconds slower than the 8-bit version above
// as the bulk byte-reversing SPI transfer operation is not available
// on all Teensys.
//**************************************************************//
void RA8876_t3::bteMpuWriteWithROPData16(ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,ru32 des_addr,ru16 des_image_width,
ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru8 rop_code,const unsigned short *data)
{
  ru16 i,j;
  bteMpuWriteWithROP(s1_addr, s1_image_width, s1_x, s1_y, des_addr, des_image_width, des_x, des_y, width, height, rop_code);

  startSend();
  _pspi->transfer(RA8876_SPI_DATAWRITE);
  
  for(j=0;j<height;j++)
  {
    for(i=0;i<width;i++)
    {
	  _pspi->transfer16(*data);	  
      data++;
    }
  } 
  
  endSend(true);
}
//**************************************************************//
//write data after setting, using lcdDataWrite() or lcdDataWrite16bbp()
//**************************************************************//
void RA8876_t3::bteMpuWriteWithROP(ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,ru32 des_addr,ru16 des_image_width,ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru8 rop_code)
{
  check2dBusy();
  graphicMode(true);
  bte_Source1_MemoryStartAddr(s1_addr);
  bte_Source1_ImageWidth(s1_image_width);
  bte_Source1_WindowStartXY(s1_x,s1_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  lcdRegDataWrite(RA8876_BTE_CTRL1,rop_code<<4|RA8876_BTE_MPU_WRITE_WITH_ROP);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
}

//**************************************************************//
// Send data from the microcontroller to the RA8876
// Does a chromakey (transparent color) to combine with the image already in memory
//**************************************************************//
void RA8876_t3::bteMpuWriteWithChromaKeyData8(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 chromakey_color,const unsigned char *data)
{
  bteMpuWriteWithChromaKey(des_addr, des_image_width, des_x, des_y, width, height, chromakey_color);  

  startSend();
  _pspi->transfer(RA8876_SPI_DATAWRITE);

#ifdef SPI_HAS_TRANSFER_ASYNC
  activeDMA = true;
  _pspi->transfer(data, NULL, width*height*2, finishedDMAEvent);
#else
  _pspi->transfer(data, NULL, width*height*2);
  endSend(true);
#endif
}
//**************************************************************//
// Chromakey for 16-bit byte-reversed data. (Slower than 8-bit.)
//**************************************************************//
void RA8876_t3::bteMpuWriteWithChromaKeyData16(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height, ru16 chromakey_color,const unsigned short *data)
{
  ru16 i,j;
  bteMpuWriteWithChromaKey(des_addr, des_image_width, des_x, des_y, width, height, chromakey_color);
  
  startSend();
  _pspi->transfer(RA8876_SPI_DATAWRITE);
  for(j=0;j<height;j++)
  {
    for(i=0;i<width;i++)
    {
	  _pspi->transfer16(*data);
      data++;
    }
  } 
  endSend(true);
}

//**************************************************************//
//write data after setting, using lcdDataWrite() or lcdDataWrite16bbp()
//**************************************************************//
void RA8876_t3::bteMpuWriteWithChromaKey(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 chromakey_color)
{
  check2dBusy();
  graphicMode(true);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  backGroundColor16bpp(chromakey_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_MPU_WRITE_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
}

//**************************************************************//
//**************************************************************//
void RA8876_t3::bteMpuWriteColorExpansionData(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 foreground_color,ru16 background_color,const unsigned char *data)
{
  ru16 i,j;
  check2dBusy();
  graphicMode(true);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  foreGroundColor16bpp(foreground_color);
  backGroundColor16bpp(background_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_BUS_WIDTH8<<4|RA8876_BTE_MPU_WRITE_COLOR_EXPANSION);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
  
  for(i=0;i< height;i++)
  {	
    for(j=0;j< ((width+7)/8);j++)
    {
      lcdDataWrite(*data, false);
      data++;
    }
  }
  lcdStatusRead();
}
//**************************************************************//
//write data after setting, using lcdDataWrite() or lcdDataWrite16bbp()
//**************************************************************//
void RA8876_t3::bteMpuWriteColorExpansion(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 foreground_color,ru16 background_color)
{
  check2dBusy();
  graphicMode(true);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height); 
  foreGroundColor16bpp(foreground_color);
  backGroundColor16bpp(background_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_BUS_WIDTH8<<4|RA8876_BTE_MPU_WRITE_COLOR_EXPANSION);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
}
//**************************************************************//
/*background_color do not set the same as foreground_color*/
//**************************************************************//
void RA8876_t3::bteMpuWriteColorExpansionWithChromaKeyData(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 foreground_color,ru16 background_color, const unsigned char *data)
{
  ru16 i,j;
  check2dBusy();
  graphicMode(true);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  foreGroundColor16bpp(foreground_color);
  backGroundColor16bpp(background_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_BUS_WIDTH8<<4|RA8876_BTE_MPU_WRITE_COLOR_EXPANSION_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
  
  for(i=0;i< height;i++)
  {	
    for(j=0;j< ((width+7)/8);j++)
    {
      lcdDataWrite(*data, false);
      data++;
    }
  }
  lcdStatusRead();
}
//**************************************************************//
/*background_color do not set the same as foreground_color*/
//write data after setting, using lcdDataWrite() or lcdDataWrite16bbp()
//**************************************************************//
void RA8876_t3::bteMpuWriteColorExpansionWithChromaKey(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 foreground_color,ru16 background_color)
{
  check2dBusy();
  graphicMode(true);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  foreGroundColor16bpp(foreground_color);
  backGroundColor16bpp(background_color);
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_BUS_WIDTH8<<4|RA8876_BTE_MPU_WRITE_COLOR_EXPANSION_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4);//90h
  ramAccessPrepare();
}
//**************************************************************//
//**************************************************************//
void  RA8876_t3::btePatternFill(ru8 p8x8or16x16, ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
                                 ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height)
{ 
  check2dBusy();
  graphicMode(true);
  bte_Source0_MemoryStartAddr(s0_addr);
  bte_Source0_ImageWidth(s0_image_width);
  bte_Source0_WindowStartXY(s0_x,s0_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height); 
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_CODE_12<<4|RA8876_BTE_PATTERN_FILL_WITH_ROP);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h
  
  if(p8x8or16x16 == 0)
    lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4|RA8876_PATTERN_FORMAT8X8);//90h
  else
    lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4|RA8876_PATTERN_FORMAT16X16);//90h
   
}
//**************************************************************//
//**************************************************************//
void  RA8876_t3::btePatternFillWithChromaKey(ru8 p8x8or16x16, ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 chromakey_color)
{
  check2dBusy();
  graphicMode(true);
  bte_Source0_MemoryStartAddr(s0_addr);
  bte_Source0_ImageWidth(s0_image_width);
  bte_Source0_WindowStartXY(s0_x,s0_y);
  bte_DestinationMemoryStartAddr(des_addr);
  bte_DestinationImageWidth(des_image_width);
  bte_DestinationWindowStartXY(des_x,des_y);
  bte_WindowSize(width,height);
  backGroundColor16bpp(chromakey_color); 
  lcdRegDataWrite(RA8876_BTE_CTRL1,RA8876_BTE_ROP_CODE_12<<4|RA8876_BTE_PATTERN_FILL_WITH_CHROMA);//91h
  lcdRegDataWrite(RA8876_BTE_COLR,RA8876_S0_COLOR_DEPTH_16BPP<<5|RA8876_S1_COLOR_DEPTH_16BPP<<2|RA8876_DESTINATION_COLOR_DEPTH_16BPP);//92h

  if(p8x8or16x16 == 0)
    lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4|RA8876_PATTERN_FORMAT8X8);//90h
  else
    lcdRegDataWrite(RA8876_BTE_CTRL0,RA8876_BTE_ENABLE<<4|RA8876_PATTERN_FORMAT16X16);//90h

}

 /*DMA Function*/
 //**************************************************************//
 /*If used 32bit address serial flash through ra8876, must be set command to serial flash to enter 4bytes mode first.
 only needs set one times after power on */
 //**************************************************************//
 void  RA8876_t3::setSerialFlash4BytesMode(ru8 scs_select)
 {
  if(scs_select==0)
  {
  lcdRegDataWrite( RA8876_SPIMCR2, RA8876_SPIM_NSS_SELECT_0<<5|RA8876_SPIM_MODE0);//b9h
  lcdRegDataWrite( RA8876_SPIMCR2, RA8876_SPIM_NSS_SELECT_0<<5|RA8876_SPIM_NSS_ACTIVE<<4|RA8876_SPIM_MODE0);//b9h 
  lcdRegWrite( RA8876_SPIDR);//b8h
  delay(1);
  lcdDataWrite(0xB7);//
  delay(1);
  lcdRegDataWrite( RA8876_SPIMCR2, RA8876_SPIM_NSS_SELECT_0<<5|RA8876_SPIM_NSS_INACTIVE<<4|RA8876_SPIM_MODE0);//b9h 
  }
  if(scs_select==1)
  {
  lcdRegDataWrite( RA8876_SPIMCR2 ,RA8876_SPIM_NSS_SELECT_1<<5|RA8876_SPIM_MODE0);//b9h
  lcdRegDataWrite( RA8876_SPIMCR2, RA8876_SPIM_NSS_SELECT_1<<5|RA8876_SPIM_NSS_ACTIVE<<4|RA8876_SPIM_MODE0);//b9h
  lcdRegWrite( RA8876_SPIDR);//b8h
  delay(1);
  lcdDataWrite(0xB7);//
  delay(1);
  lcdRegDataWrite( RA8876_SPIMCR2, RA8876_SPIM_NSS_SELECT_1<<5|RA8876_SPIM_NSS_INACTIVE<<4|RA8876_SPIM_MODE0);//b9h 
  } 
 }
//**************************************************************//
/* scs = 0 : select scs0, scs = 1 : select scs1, */
//**************************************************************//
 void  RA8876_t3::dma_24bitAddressBlockMode(ru8 scs_select,ru8 clk_div,ru16 x0,ru16 y0,ru16 width,ru16 height,ru16 picture_width,ru32 addr)
 {
   if(scs_select==0)
    lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT0<<7|RA8876_SERIAL_FLASH_DMA_MODE<<6|RA8876_SERIAL_FLASH_ADDR_24BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
   if(scs_select==1)
    lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT1<<7|RA8876_SERIAL_FLASH_DMA_MODE<<6|RA8876_SERIAL_FLASH_ADDR_24BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
  
  lcdRegDataWrite(RA8876_SPI_DIVSOR,clk_div);//bbh  
  lcdRegDataWrite(RA8876_DMA_DX0,x0);//c0h
  lcdRegDataWrite(RA8876_DMA_DX1,x0>>8);//c1h
  lcdRegDataWrite(RA8876_DMA_DY0,y0);//c2h
  lcdRegDataWrite(RA8876_DMA_DY1,y0>>8);//c3h 
  lcdRegDataWrite(RA8876_DMAW_WTH0,width);//c6h
  lcdRegDataWrite(RA8876_DMAW_WTH1,width>>8);//c7h
  lcdRegDataWrite(RA8876_DMAW_HIGH0,height);//c8h
  lcdRegDataWrite(RA8876_DMAW_HIGH1,height>>8);//c9h 
  lcdRegDataWrite(RA8876_DMA_SWTH0,picture_width);//cah
  lcdRegDataWrite(RA8876_DMA_SWTH1,picture_width>>8);//cbh 
  lcdRegDataWrite(RA8876_DMA_SSTR0,addr);//bch
  lcdRegDataWrite(RA8876_DMA_SSTR1,addr>>8);//bdh
  lcdRegDataWrite(RA8876_DMA_SSTR2,addr>>16);//beh
  lcdRegDataWrite(RA8876_DMA_SSTR3,addr>>24);//bfh 
  
  lcdRegDataWrite(RA8876_DMA_CTRL,RA8876_DMA_START);//b6h 
  check2dBusy(); 
 }
 //**************************************************************//
/* scs = 0 : select scs0, scs = 1 : select scs1, */
//**************************************************************//
 void  RA8876_t3::dma_32bitAddressBlockMode(ru8 scs_select,ru8 clk_div,ru16 x0,ru16 y0,ru16 width,ru16 height,ru16 picture_width,ru32 addr)
 {
   if(scs_select==0)
    lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT0<<7|RA8876_SERIAL_FLASH_DMA_MODE<<6|RA8876_SERIAL_FLASH_ADDR_32BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
   if(scs_select==1)
    lcdRegDataWrite(RA8876_SFL_CTRL,RA8876_SERIAL_FLASH_SELECT1<<7|RA8876_SERIAL_FLASH_DMA_MODE<<6|RA8876_SERIAL_FLASH_ADDR_32BIT<<5|RA8876_FOLLOW_RA8876_MODE<<4|RA8876_SPI_FAST_READ_8DUMMY);//b7h
  
  lcdRegDataWrite(RA8876_SPI_DIVSOR,clk_div);//bbh 
  
  lcdRegDataWrite(RA8876_DMA_DX0,x0);//c0h
  lcdRegDataWrite(RA8876_DMA_DX1,x0>>8);//c1h
  lcdRegDataWrite(RA8876_DMA_DY0,y0);//c2h
  lcdRegDataWrite(RA8876_DMA_DY1,y0>>8);//c3h 
  lcdRegDataWrite(RA8876_DMAW_WTH0,width);//c6h
  lcdRegDataWrite(RA8876_DMAW_WTH1,width>>8);//c7h
  lcdRegDataWrite(RA8876_DMAW_HIGH0,height);//c8h
  lcdRegDataWrite(RA8876_DMAW_HIGH1,height>>8);//c9h 
  lcdRegDataWrite(RA8876_DMA_SWTH0,picture_width);//cah
  lcdRegDataWrite(RA8876_DMA_SWTH1,picture_width>>8);//cbh 
  lcdRegDataWrite(RA8876_DMA_SSTR0,addr);//bch
  lcdRegDataWrite(RA8876_DMA_SSTR1,addr>>8);//bdh
  lcdRegDataWrite(RA8876_DMA_SSTR2,addr>>16);//beh
  lcdRegDataWrite(RA8876_DMA_SSTR3,addr>>24);//bfh  
  
  lcdRegDataWrite(RA8876_DMA_CTRL,RA8876_DMA_START);//b6h 
  check2dBusy(); 
 }
 
//**************************************************************//
// Setup PIP Windows ( 2 PIP Windows Avaiable)
//**************************************************************//
void RA8876_t3::PIP (
		 unsigned char On_Off // 0 : disable PIP, 1 : enable PIP, 2 : To maintain the original state
		,unsigned char Select_PIP // 1 : use PIP1 , 2 : use PIP2
		,unsigned long PAddr //start address of PIP
		,unsigned short XP //coordinate X of PIP Window, It must be divided by 4.
		,unsigned short YP //coordinate Y of PIP Window, It must be divided by 4.
		,unsigned long ImageWidth //Image Width of PIP (recommend = canvas image width)
		,unsigned short X_Dis //coordinate X of Display Window
		,unsigned short Y_Dis //coordinate Y of Display Window
		,unsigned short X_W //width of PIP and Display Window, It must be divided by 4.
		,unsigned short Y_H //height of PIP and Display Window , It must be divided by 4.
		)
{
	if(Select_PIP == 1 )  
	{
	Select_PIP1_Parameter();
	}
	if(Select_PIP == 2 )  
	{
	Select_PIP2_Parameter();
	}
	PIP_Display_Start_XY(X_Dis,Y_Dis);
	PIP_Image_Start_Address(PAddr);
	PIP_Image_Width(ImageWidth);
	PIP_Window_Image_Start_XY(XP,YP);
	PIP_Window_Width_Height(X_W,Y_H);

	if(On_Off == 0)
    {
  		if(Select_PIP == 1 )  
		{ 
		Disable_PIP1();	
		}
		if(Select_PIP == 2 )  
		{
		Disable_PIP2();
		}
    }

    if(On_Off == 1)
    {
  		if(Select_PIP == 1 )  
		{ 
		Enable_PIP1();	
		}
		if(Select_PIP == 2 )  
		{
		Enable_PIP2();
		}
    }
}

//[2Ah][2Bh][2Ch][2Dh]=========================================================================
void RA8876_t3::PIP_Display_Start_XY(unsigned short WX,unsigned short HY)	
{
/*
Reference Main Window coordination.
Unit: Pixel
It must be divisible by 4. PWDULX Bit [1:0] tie to ¡§0¡š internally.
X-axis coordination should less than horizontal display width.
According to bit of Select Configure PIP 1 or 2 Window¡Šs parameters. 
Function bit will be configured for relative PIP window.
*/
	lcdRegDataWrite(RA8876_PWDULX0, WX);    // [2Ah] PIP Window Display Upper-Left corner X-coordination [7:0]
	lcdRegDataWrite(RA8876_PWDULX1, WX>>8); // [2Bh] PIP Window Display Upper-Left corner X-coordination [12:8]


/*
Reference Main Window coordination.
Unit: Pixel
Y-axis coordination should less than vertical display height.
According to bit of Select Configure PIP 1 or 2 Window¡Šs parameters.
Function bit will be configured for relative PIP window.
*/
	lcdRegDataWrite(RA8876_PWDULY0, HY);    // [2Ch] PIP Window Display Upper-Left corner Y-coordination [7:0]
	lcdRegDataWrite(RA8876_PWDULY1, HY>>8); // [2Dh] PIP Window Display Upper-Left corner Y-coordination [12:8]
}

//[2Eh][2Fh][30h][31h]=========================================================================
void RA8876_t3::PIP_Image_Start_Address(unsigned long Addr)	
{
	lcdRegDataWrite(RA8876_PISA0, Addr); // [2Eh] PIP Image Start Address[7:2]
	lcdRegDataWrite(RA8876_PISA1, Addr>>8); // [2Fh] PIP Image Start Address[15:8]
	lcdRegDataWrite(RA8876_PISA2, Addr>>16); // [30h] PIP Image Start Address [23:16]
	lcdRegDataWrite(RA8876_PISA3, Addr>>24); // [31h] PIP Image Start Address [31:24]
}

//[32h][33h]=========================================================================
void RA8876_t3::PIP_Image_Width(unsigned short WX)	
{
/*
Unit: Pixel.
It must be divisible by 4. PIW Bit [1:0] tie to ¡§0¡š internally.
The value is physical pixel number.
This width should less than horizontal display width.
According to bit of Select Configure PIP 1 or 2 Window¡Šs parameters.
Function bit will be configured for relative PIP window.
*/
	lcdRegDataWrite(RA8876_PIW0, WX);     // [32h] PIP Image Width [7:0]
	lcdRegDataWrite(RA8876_PIW1, WX>>8);  // [33h] PIP Image Width [12:8]
}

//[34h][35h][36h][37h]=========================================================================
void RA8876_t3::PIP_Window_Image_Start_XY(unsigned short WX,unsigned short HY)	
{
/*
Reference PIP Image coordination.
Unit: Pixel
It must be divisible by 4. PWIULX Bit [1:0] tie to ¡§0¡š internally.
X-axis coordination plus PIP image width cannot large than 8188.
According to bit of Select Configure PIP 1 or 2 Window¡Šs parameters. 
Function bit will be configured for relative PIP window.
*/
	lcdRegDataWrite(RA8876_PWIULX0, WX);    // [34h] PIP 1 or 2 Window Image Upper-Left corner X-coordination [7:0]
	lcdRegDataWrite(RA8876_PWIULX1, WX>>8); // [35h] PIP Window Image Upper-Left corner X-coordination [12:8]

/*
Reference PIP Image coordination.
Unit: Pixel
Y-axis coordination plus PIP window height should less than 8191.
According to bit of Select Configure PIP 1 or 2 Window¡Šs parameters. 
Function bit will be configured for relative PIP window.
*/
	lcdRegDataWrite(RA8876_PWIULY0, HY);    // [36h] PIP Windows Display Upper-Left corner Y-coordination [7:0]
	lcdRegDataWrite(RA8876_PWIULY1, HY>>8); // [37h] PIP Windows Image Upper-Left corner Y-coordination [12:8]

}

//[38h][39h][3Ah][3Bh]=========================================================================
void RA8876_t3::PIP_Window_Width_Height(unsigned short WX,unsigned short HY)	
{
/*

Unit: Pixel.
It must be divisible by 4. The value is physical pixel number.
Maximum value is 8188 pixels.
According to bit of Select Configure PIP 1 or 2 Window¡Šs parameters. 
Function bit will be configured for relative PIP window.
*/
	lcdRegDataWrite(RA8876_PWW0, WX);    // [38h] PIP Window Width [7:0]
	lcdRegDataWrite(RA8876_PWW1, WX>>8); // [39h] PIP Window Width [10:8]

/*
Unit: Pixel
The value is physical pixel number. Maximum value is 8191 pixels.
According to bit of Select Configure PIP 1 or 2 Window¡Šs parameters. 
Function bit will be configured for relative PIP window.
*/
	lcdRegDataWrite(RA8876_PWH0, HY);    // [3Ah] PIP Window Height [7:0]
	lcdRegDataWrite(RA8876_PWH1, HY>>8); // [3Bh] PIP Window Height [10:8]

}

//[10h]=========================================================================
// Turn on PIP window 1
void RA8876_t3::Enable_PIP1(void)
{
/*
PIP 1 window Enable/Disable
0 : PIP 1 window disable.
1 : PIP 1 window enable
PIP 1 window always on top of PIP 2 window.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MPWCTR); // 0x10
	temp |= cSetb7;
	lcdRegDataWrite(RA8876_MPWCTR,temp);

}

// Turn off PIP window 1
void RA8876_t3::Disable_PIP1(void)
{
/*
PIP 1 window Enable/Disable
0 : PIP 1 window disable.
1 : PIP 1 window enable
PIP 1 window always on top of PIP 2 window.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MPWCTR); // 0x10
    temp &= cClrb7;
	lcdRegDataWrite(RA8876_MPWCTR,temp);
}

// Turn on PIP window 2
void RA8876_t3::Enable_PIP2(void)
{
/*
PIP 2 window Enable/Disable
0 : PIP 2 window disable.
1 : PIP 2 window enable
PIP 1 window always on top of PIP 2 window.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MPWCTR); // 0x10
	temp |= cSetb6;
	lcdRegDataWrite(RA8876_MPWCTR,temp);
}

// Turn off PIP window 1
void RA8876_t3::Disable_PIP2(void)
{
/*
PIP 2 window Enable/Disable
0 : PIP 2 window disable.
1 : PIP 2 window enable
PIP 1 window always on top of PIP 2 window.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MPWCTR); // 0x10
    temp &= cClrb6;
	lcdRegDataWrite(RA8876_MPWCTR,temp);
}

void RA8876_t3::Select_PIP1_Parameter(void)
{
/*
0: To configure PIP 1¡Šs parameters.
1: To configure PIP 2¡Šs parameters..
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MPWCTR); // 0x10
    temp &= cClrb4;
	lcdRegDataWrite(RA8876_MPWCTR,temp);
}

void RA8876_t3::Select_PIP2_Parameter(void)
{
/*
0: To configure PIP 1¡Šs parameters.
1: To configure PIP 2¡Šs parameters..
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MPWCTR); // 0x10
	temp |= cSetb4;
	lcdRegDataWrite(RA8876_MPWCTR,temp);
}
void RA8876_t3::Select_Main_Window_8bpp(void)
{
/*
Main Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MPWCTR); // 0x10
    temp &= cClrb3;
    temp &= cClrb2;
	lcdRegDataWrite(RA8876_MPWCTR,temp);
}
void RA8876_t3::Select_Main_Window_16bpp(void)
{
/*
Main Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MPWCTR); // 0x10
    temp &= cClrb3;
    temp |= cSetb2;
	lcdRegDataWrite(RA8876_MPWCTR,temp);
}
void RA8876_t3::Select_Main_Window_24bpp(void)
{
/*
Main Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MPWCTR); // 0x10
	temp |= cSetb3;
	lcdRegDataWrite(RA8876_MPWCTR,temp);
}

void RA8876_t3::Select_LCD_Sync_Mode(void)
{
/*
To Control panel's synchronous signals
0: Sync Mode: Enable XVSYNC, XHSYNC, XDE
1: DE Mode: Only XDE enable, XVSYNC & XHSYNC in idle state
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MPWCTR); // 0x10
    temp &= cClrb0;
	lcdRegDataWrite(RA8876_MPWCTR,temp);
}
void RA8876_t3::Select_LCD_DE_Mode(void)
{
/*
To Control panel's synchronous signals
0: Sync Mode: Enable XVSYNC, XHSYNC, XDE
1: DE Mode: Only XDE enable, XVSYNC & XHSYNC in idle state
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MPWCTR); // 0x10
	temp |= cSetb0;
	lcdRegDataWrite(RA8876_MPWCTR,temp);
}

//[11h]=========================================================================
void RA8876_t3::Select_PIP1_Window_8bpp(void)
{
/*
PIP 1 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_PIPCDEP); // 0x11
    temp &= cClrb3;
    temp &= cClrb2;
	lcdRegDataWrite(RA8876_PIPCDEP,temp);
}

void RA8876_t3::Select_PIP1_Window_16bpp(void)
{
/*
PIP 1 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_PIPCDEP); // 0x11
    temp &= cClrb3;
    temp |= cSetb2;
	lcdRegDataWrite(RA8876_PIPCDEP,temp);
}

void RA8876_t3::Select_PIP1_Window_24bpp(void)
{
/*
PIP 1 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_PIPCDEP); // 0x11
    temp |= cSetb3;
	lcdRegDataWrite(RA8876_PIPCDEP,temp);
}

void RA8876_t3::Select_PIP2_Window_8bpp(void)
{
/*
PIP 2 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_PIPCDEP); // 0x11
    temp &= cClrb1;
    temp &= cClrb0;
	lcdRegDataWrite(RA8876_PIPCDEP,temp);
}

void RA8876_t3::Select_PIP2_Window_16bpp(void)
{
/*
PIP 2 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_PIPCDEP); // 0x11
    temp &= cClrb1;
    temp |= cSetb0;
	lcdRegDataWrite(RA8876_PIPCDEP,temp);
}

void RA8876_t3::Select_PIP2_Window_24bpp(void)
{
/*
PIP 2 Window Color Depth Setting
00b: 8-bpp generic TFT, i.e. 256 colors.
01b: 16-bpp generic TFT, i.e. 65K colors.
1xb: 24-bpp generic TFT, i.e. 1.67M colors.
*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_PIPCDEP); // 0x11
    temp |= cSetb1;
//    temp &= cClrb0;
	lcdRegDataWrite(RA8876_PIPCDEP,temp);
}

/********************************************************/
// Select a screen page (Buffer) 1 to 9.
// ALT + (F1 to F9) using USBHost_t36 Keyboard Driver
// and my USBKeyboard host driver.
// Also, STBASIC Commands: screen 0 to screen 8
void RA8876_t3::selectScreen(uint32_t screenPage) {
	check2dBusy();
	tftSave_t *tempSave, *tempRestore;
	// Don't Select the current screen page
	if(screenPage == currentPage)
		return;
	switch(currentPage) {
		case PAGE1_START_ADDR:
			tempSave = screenPage1; 
			break;
		case PAGE2_START_ADDR:
			tempSave = screenPage2; 
			break;
		case PAGE3_START_ADDR:
			tempSave = screenPage3; 
			break;
		case PAGE4_START_ADDR:
			tempSave = screenPage4; 
			break;
		case PAGE5_START_ADDR:
			tempSave = screenPage5; 
			break;
		case PAGE6_START_ADDR:
			tempSave = screenPage6; 
			break;
		case PAGE7_START_ADDR:
			tempSave = screenPage7; 
			break;
		case PAGE8_START_ADDR:
			tempSave = screenPage8; 
			break;
		case PAGE9_START_ADDR:
			tempSave = screenPage9; 
			break;
//		case PAGE10_START_ADDR:
//			tempSave = screenPage10; 
//			break;
		default:
			tempSave = screenPage1; 
	}
	// Copy back selected screen page parameters
	switch(screenPage) {
		case PAGE1_START_ADDR:
			tempRestore = screenPage1; 
			break;
		case PAGE2_START_ADDR:
			tempRestore = screenPage2; 
			break;
		case PAGE3_START_ADDR:
			tempRestore = screenPage3; 
			break;
		case PAGE4_START_ADDR:
			tempRestore = screenPage4; 
			break;
		case PAGE5_START_ADDR:
			tempRestore = screenPage5; 
			break;
		case PAGE6_START_ADDR:
			tempRestore = screenPage6; 
			break;
		case PAGE7_START_ADDR:
			tempRestore = screenPage7; 
			break;
		case PAGE8_START_ADDR:
			tempRestore = screenPage8; 
			break;
		case PAGE9_START_ADDR:
			tempRestore = screenPage9; 
			break;
//		case PAGE10_START_ADDR:
//			tempRestore = screenPage10; 
//			break;
		default:
			tempRestore = screenPage1; 
	}
	// Save current screen page parameters 
	saveTFTParams(tempSave);
	// Restore selected screen page parameters
	restoreTFTParams(tempRestore);
	// Setup params to rebuild selected screen
	pageOffset = screenPage;
	currentPage = screenPage;
	displayImageStartAddress(currentPage);
	displayImageWidth(_width);
	displayWindowStartXY(0,0);
	canvasImageStartAddress(currentPage);
	canvasImageWidth(_width);
	activeWindowXY(0,0);
	activeWindowWH(_width,_height); 

	setTextCursor(_cursorX, _cursorY);
	textColor(_TXTForeColor,_TXTBackColor);
	// Rebuild the display
	buildTextScreen();
	
}

// Save current screen page parameters 
void RA8876_t3::saveTFTParams(tftSave_t *screenSave) {
	screenSave->width			= _width;
	screenSave->height			= _height;
	screenSave->cursorX			= _cursorX;
	screenSave->cursorY			= _cursorY;
	screenSave->scaleX			= _scaleX;
	screenSave->scaleY			= _scaleY;
	screenSave->FNTwidth		= _FNTwidth;
	screenSave->FNTheight		= _FNTheight;
	screenSave->fontheight		= _fontheight;
	screenSave->fontSource      = currentFont;
	screenSave->cursorXsize		= _cursorXsize;
	screenSave->cursorYsize		= _cursorYsize;
// Text Sreen Vars
	screenSave->prompt_size		= prompt_size; // prompt ">"
	screenSave->prompt_line		= prompt_line; // current text prompt row
	screenSave->vdata			= vdata;
	screenSave->leftmarg		= leftmarg;
	screenSave->topmarg			= topmarg;
	screenSave->rightmarg		= rightmarg;
	screenSave->bottommarg		= bottommarg;
	screenSave->tab_size		= tab_size;
	screenSave->CharPosX		= CharPosX;
	screenSave->CharPosY		= CharPosY;
	screenSave->UDFont  		= UDFont;

//scroll vars ----------------------------
	screenSave->scrollXL		= _scrollXL;
	screenSave->scrollXR		= _scrollXR;
	screenSave->scrollYT		= _scrollYT;
	screenSave->scrollYB		= _scrollYB;
	screenSave->TXTForeColor	= _TXTForeColor;
    screenSave->TXTBackColor	= _TXTBackColor;
}

// Restore selected screen page parameters
void RA8876_t3::restoreTFTParams(tftSave_t *screenSave) {
	 _width = screenSave->width;
	 _height = screenSave->height;
	 _cursorX = screenSave->cursorX;
	 _cursorY = screenSave->cursorY;
	 _scaleX = screenSave->scaleX;
	 _scaleY = screenSave->scaleY;
	 _FNTwidth = screenSave->FNTwidth;
	 _FNTheight = screenSave->FNTheight;
	 _fontheight = screenSave->fontheight;
	 currentFont = screenSave->fontSource;
	 _cursorXsize = screenSave->cursorXsize;		
	 _cursorYsize = screenSave->cursorYsize;		
	// Text Sreen Vars
	 prompt_size = screenSave->prompt_size; // prompt ">"
	 prompt_line = screenSave->prompt_line; // current text prompt row
	 vdata = screenSave->vdata;			
	 leftmarg = screenSave->leftmarg;		
	 topmarg = screenSave->topmarg;		
	 rightmarg = screenSave->rightmarg;
	 bottommarg = screenSave->bottommarg;		
	 tab_size =	screenSave->tab_size;		
	 CharPosX = screenSave->CharPosX;		
	 CharPosY = screenSave->CharPosY;		
	 UDFont = screenSave->UDFont;		
	//scroll vars -----------------
	 _scrollXL = screenSave->scrollXL;
	 _scrollXR = screenSave->scrollXR;
	 _scrollYT = screenSave->scrollYT;
	 _scrollYB = screenSave->scrollYB;
	 _TXTForeColor = screenSave->TXTForeColor;
	 _TXTBackColor = screenSave->TXTBackColor;
}

void RA8876_t3::useCanvas(boolean on) {
  if(on) {
    displayImageStartAddress(PAGE1_START_ADDR);
    displayImageWidth(_width);
    displayWindowStartXY(0,0);
    
    canvasImageStartAddress(PAGE2_START_ADDR);
    canvasImageWidth(_width);
    activeWindowXY(0, 0);
    activeWindowWH(_width, _height);
    currentPage = PAGE2_START_ADDR;
    pageOffset = PAGE2_START_ADDR;
    check2dBusy();
    ramAccessPrepare();
    
  } else {
    displayImageStartAddress(PAGE1_START_ADDR);
    displayImageWidth(_width);
    displayWindowStartXY(0,0);
    
    canvasImageStartAddress(PAGE1_START_ADDR);
    canvasImageWidth(_width);
    activeWindowXY(0, 0);
    activeWindowWH(_width, _height);
    currentPage = PAGE1_START_ADDR;
    pageOffset = PAGE1_START_ADDR;
    check2dBusy();
    ramAccessPrepare();
    
  }
}

void RA8876_t3::updateScreen() {
	bteMemoryCopy(PAGE2_START_ADDR,_width,0,0,
				  PAGE1_START_ADDR,_width, 0,0,
				 _width,_height);
}

// Setup text cursor
void RA8876_t3::cursorInit(void)
{
	_cursorXsize = _FNTwidth;
	_cursorYsize = _FNTheight-1;
	Text_Cursor_H_V(_cursorXsize,_cursorYsize); 		// Block cusror
	Blinking_Time_Frames(20);		// Set blink rate
	Enable_Text_Cursor_Blinking();	// Turn blinking cursor on
}

//void RA8876_t3::setCursor(uint16_t x, uint16_t y)
//{
//	setTextCursor(x, y);
//}


/**************************************************************************/
/*!   
		Set the Text position for write Text only.
		Parameters:
		x:horizontal in pixels or CENTER(of the screen)
		y:vertical in pixels or CENTER(of the screen)
		autocenter:center text to choosed x,y regardless text lenght
		false: |ABCD
		true:  AB|CD
		NOTE: works with any font
*/
/**************************************************************************/
void RA8876_t3::setCursor(int16_t x, int16_t y, bool autocenter) 
{
	if(_use_default) {
		//setTextCursor(x, y);
		//return;
	}
	
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	
	_absoluteCenter = autocenter;
	
	if (_portrait) {//rotation 1,3
		//if (_use_default) swapvals(x,y);
		if (y == CENTER) {//swapped OK
			y = _width/2;
			if (!autocenter) {
				_relativeCenter = true;
				_TXTparameters |= (1 << 6);//set x flag
			}
		}
		if (x == CENTER) {//swapped
			x = _height/2;
			if (!autocenter) {
				_relativeCenter = true;
				_TXTparameters |= (1 << 5);//set y flag
			}
		}
	} else {//rotation 0,2
		if (x == CENTER) {
			x = _width/2;
			if (!autocenter) {
				_relativeCenter = true;
				_TXTparameters |= (1 << 5);
			}
		}
		if (y == CENTER) {
			y = _height/2;
			if (!autocenter) {
				_relativeCenter = true;
				_TXTparameters |= (1 << 6);
			}
		}
	}
	//TODO: This one? Useless?
	if (bitRead(_TXTparameters,2) == 0){//textWrap
		if (x >= _width) x = _width-1;
		if (y >= _height) y = _height-1;
	}
	
	_cursorX = x;
	_cursorY = y;

	//if _relativeCenter or _absoluteCenter do not apply to registers yet!
	// Have to go to _textWrite first to calculate the lenght of the entire string and recalculate the correct x,y
	if (_relativeCenter || _absoluteCenter) return;
	if (bitRead(_TXTparameters,7) == 0) _textPosition(x,y,false);
}

/**************************************************************************/
/*!   
		Give you back the current text cursor position by reading inside RA8875
		Parameters:
		x: horizontal pos in pixels
		y: vertical pos in pixels
		note: works also with rendered fonts
		USE: xxx.getCursor(myX,myY);
*/
/**************************************************************************/
void RA8876_t3::getCursor(int16_t &x, int16_t &y) 
{
		uint8_t t1,t2,t3,t4;
		t1 = lcdRegDataRead(RA8876_F_CURX0);
		t2 = lcdRegDataRead(RA8876_F_CURX1);
		t3 = lcdRegDataRead(RA8876_F_CURY0);
		t4 = lcdRegDataRead(RA8876_F_CURY1);
		x = (t2 << 8) | (t1 & 0xFF);
		y = (t4 << 8) | (t3 & 0xFF);
		//if (_portrait && _use_default) swapvals(x,y);
	
}

int16_t RA8876_t3::getCursorX(void)
{
	//if (_portrait && _use_default) return _cursorY;
	return _cursorX;
}

int16_t RA8876_t3::getCursorY(void)
{
	//if (_portrait && _use_default) return _cursorX;
	return _cursorY;
}

//Set Graphic Mode Margins (pixel based)
void RA8876_t3::setMargins(uint16_t xl, uint16_t yt, uint16_t xr, uint16_t yb) {
	_scrollXL = xl;
	_scrollYT = yt;
	_scrollXR = xr;
	_scrollYB = yb;
	buildTextScreen();	
	setTextCursor(    _scrollXL,    _scrollYT);
}

//Set Text Mode Margins (font size based)
void RA8876_t3::setTMargins(uint16_t xl, uint16_t yt, uint16_t xr, uint16_t yb) {
	_scrollXL = xl*    _FNTwidth;
	_scrollYT = yt*    _FNTheight;
	_scrollXR = _width-(xr*    _FNTwidth);
	_scrollYB = _height-(yb*    _FNTheight);
	buildTextScreen();	
	setTextCursor(    _scrollXL,    _scrollYT);
}

// Set text prompt size (font size based)
void RA8876_t3::setPromptSize(uint16_t ps) {
	prompt_size = ps*(    _FNTwidth *     _scaleX) +     _scrollXL; // Default prompt ">"
}

// Clear current screen to background 'color'
void RA8876_t3::fillScreen(uint16_t color) {
	drawSquareFill(_scrollXL, _scrollYT, _scrollXR, _scrollYB, color);
	check2dBusy();  //must wait for fill to finish before setting foreground color
	textColor(_TXTForeColor,_TXTBackColor);
	setTextCursor(_scrollXL,_scrollYT);
}

// Clear Status Line to background 'color'
void RA8876_t3::fillStatusLine(uint16_t color) {
	uint16_t temp = _TXTBackColor;
	clearStatusLine(color);
	_TXTBackColor = temp;
	textColor(_TXTForeColor,temp);
}

//**************************************************************//
// Note that, unlike the RA88875, this does not set BG transparent!
//**************************************************************//
void RA8876_t3::setTextColor(uint16_t color)
{
 	check2dBusy();  // don't change colors until not busy
 	foreGroundColor16bpp(color);
	_backTransparent = true;  // used for ILI and GFX Fonts
	_TXTForeColor = color;

}

//**************************************************************//
//**************************************************************//
void RA8876_t3::setBackGroundColor(uint16_t color)
{
	backGroundColor16bpp(color);
}

// Set text foreground + background colors
void RA8876_t3::setTextColor(uint16_t fgc, uint16_t bgc) {
	if(_use_ili_font){
	  //for anti-alias font only
	  // pre-expand colors for fast alpha-blending later
	  textcolorPrexpanded = (fgc | (fgc << 16)) & 0b00000111111000001111100000011111;
	  textbgcolorPrexpanded = (bgc | (bgc << 16)) & 0b00000111111000001111100000011111;
	}  
	
	textColor(fgc,bgc);
	_backTransparent = false;  // used for ILI and GFX Fonts
}


// Send a character directly to video memory (No control ASCII codes are processed).
void RA8876_t3::tftRawWrite(uint8_t data) {
	rawPrint(data);
}
	
// Send a string to the status line
void RA8876_t3::printStatusLine(uint16_t x0,uint16_t fgColor,uint16_t bgColor, const char *text) {
	
	if(_use_gfx_font || _use_ili_font) {
		setTextColor(fgColor, bgColor);
		setTextCursor(x0, _height-STATUS_LINE_HEIGHT);
		printf("%s", text);
	} else {
		writeStatusLine(x0*(_FNTwidth*_scaleX), fgColor, bgColor, text);	
	}
}

// Load a user defined font from memory to RA8876 Character generator RAM
uint8_t RA8876_t3::fontLoadMEM(char *fontsrc) {
	CGRAM_initial(PATTERN1_RAM_START_ADDR, (uint8_t *)fontsrc, 16*256);
	return 0;
}

// Set USE_FF_FONTLOAD in h to 1 to use fontload function.
// Requires FatFS or Can be modified to use SDFat or SD.
#if USE_FF_FONTLOAD == 1
#include "ff.h"
// Load a user defined font from a file to RA8876 Character generator RAM
uint8_t RA8876_t3::fontLoad(char *fontfile) {
    FIL fsrc;      /* File object */
    FRESULT fresult = FR_OK;          /* FatFs function common result code */
    uint br = 0;
	uint8_t fontdata[4096];	// Buffer is setup to use 8x16 fonts.
							// Bigger fonts can be used.
    /* Open source file */
    fresult = f_open(&fsrc, fontfile, FA_READ);
    if (fresult)
		return 1; /* return any error */
    for (;;) {
        fresult = f_read(&fsrc, fontdata, sizeof fontdata, &br);  /* Read a chunk of source file */
        if (fresult || br == 0) break; /* error or eof */
    }
    /* Close open file */
    f_close(&fsrc);
	// Initialize CGRAM with font loaded into fontdata buffer.
	CGRAM_initial(PATTERN1_RAM_START_ADDR, fontdata, 16*256);
	return 0;
}
#endif

// Select internal fonts or user defined fonts (0 for internal or 1 for user defined)
void RA8876_t3::setFontSource(uint8_t source) {
	
	switch(source) {
	case 0:
		_scaleX = 1;
		_scaleY = 1;
		_setFNTdimensions(0);
		UDFont = false;
		break;
	case 1:
		if((_FNTheight != 16) && (_FNTwidth != 8)) {
			_FNTheight = 16;
			_FNTwidth  = 8;
		}
		UDFont = true;
		_setFNTdimensions(1);
		break;
	default:
		UDFont = false;	
	}
	//Ra8876_Lite::setFontSource(source);
	// Rebuild current screen page
	buildTextScreen();
}

// Set fontsize for fonts, currently 0 to 2 (Internal and User Defined)
boolean RA8876_t3::setFontSize(uint8_t scale, boolean runflag) {
	switch(scale) {
		
		case 0:
			if(UDFont) { // User Defined Fonts
				_scaleY = 1;
				_scaleX = 1;
			} else {
				_FNTheight = 16;
				_FNTwidth  = 8;
			}
			break;
		case 1:
			if(UDFont) {
				_FNTheight = 16;
				_FNTwidth  = 8;
				_scaleY = 2;
				_scaleX = 2;
			} else {
				_FNTheight = 24;
				_FNTwidth  = 12;
			}
			break;
		case 2:
			if(UDFont) {
				_FNTheight = 16;
				_FNTwidth  = 8;
				_scaleY = 3;
				_scaleX = 3;
			} else {
				_FNTheight = 32;
				_FNTwidth  = 16;
			}
			break;
		default:
			return true;
			break;
		setTextCursor(_scrollXL,_scrollYT);
	}
	// Rebuild current screen page
	buildTextScreen();
	cursorInit();
	if(runflag == false) {
		drawSquareFill(0, 0, _width-1, _height-1, _TXTBackColor);
		//have to wait for fill to finish before changing the foreground color...
		check2dBusy();
		textColor(_TXTForeColor,_TXTBackColor);
		setTextCursor(0,0);
	}
	rightmarg = (uint8_t)(_width / (_FNTwidth * _scaleX));
	bottommarg = (uint8_t)(_height / (_FNTheight * _scaleY));
	
	return false;
}

// Text cursor on or off
void RA8876_t3::setCursorMode(boolean mode) {

	if(mode == false)
		Disable_Text_Cursor();	// No cursor
	else
		Enable_Text_Cursor();	// Blinking block cursor
}

// Set text cursor type
void RA8876_t3::setCursorType(uint8_t type) {

	switch(type) {
		case 0:
			_cursorYsize = _FNTheight;
			Text_Cursor_H_V(_cursorXsize, _cursorYsize); 		// Block cursor
			break;
		case 1:
			_cursorYsize = 2;
			Text_Cursor_H_V(_cursorXsize, _cursorYsize); 		// Underline cursor
			break;
		case 2:
			_cursorYsize = _FNTheight;
			Text_Cursor_H_V(2, _cursorYsize); 						// I-beam cursor
			break;
		default:
			break;
	}
}

// Set text cursor blink mode (On or Off)
void RA8876_t3::setCursorBlink(boolean onOff) {
	if(onOff)
		Enable_Text_Cursor_Blinking();
	else
		Disable_Text_Cursor_Blinking();
}
	
// If you're looking to set text write position in characters within the current scroll window then use textxy()

// Get cursor X position
int16_t RA8876_t3::getTextX(void) {
	return (_cursorX / _FNTwidth); //getCursorX() / getFontWidth();
}	
	
// Get cursor Y position
int16_t RA8876_t3::getTextY(void) {
	return (_cursorY / _FNTheight);//getCursorY() / getFontHeight();
}	

// Get current text screen width (pixel * current font width)
int16_t RA8876_t3::getTwidth(void) {
	return (rightmarg - leftmarg)-1;
}

// Get current text screen height (pixel * current font height)
int16_t RA8876_t3::getTheight(void) {
	return (bottommarg - topmarg)-1;
}

// Get current screen foreground color
uint16_t RA8876_t3::getTextFGC() {
	return _TXTForeColor;
}

// Get current screen backround color
uint16_t RA8876_t3::getTextBGC(void) {
	return _TXTBackColor;
}


// Get font vertical size (in pixels)
uint8_t RA8876_t3::getFontHeight(void) {
	return _FNTheight * _scaleY;
}

// Get font width size (in pixels)
uint8_t RA8876_t3::getFontWidth(void) {
	return _FNTwidth * _scaleX;
}


// Draw a rectangle. Note: damages text color register
void RA8876_t3::drawRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color) {
	x += _originx;
	y += _originy;
	int16_t x_end = x+w-1;
	int16_t y_end = y+h-1;
	if((x >= _displayclipx2)   || // Clip right
		 (y >= _displayclipy2) || // Clip bottom
		 (x_end < _displayclipx1)    || // Clip left
		 (y_end < _displayclipy1))  	// Clip top 
	{
		// outside the clip rectangle
		return;
	}
	if (x < _displayclipx1) x = _displayclipx1;
	if (y < _displayclipy1) y = _displayclipy1;
	if (x_end > _displayclipx2) x_end = _displayclipx2;
	if (y_end > _displayclipy2) y_end = _displayclipy2;
	
  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
  switch (_rotation) {
  	case 1: swapvals(x,y); swapvals(x_end, y_end); break;
  	case 2: x = _width-x; x_end = _width - x_end;; break;
  	case 3: rotateCCXY(x,y); rotateCCXY(x_end, y_end); break;
  }

  lcdRegDataWrite(RA8876_DLHSR0,x, false);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x>>8, false);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y, false);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y>>8, false);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x_end, false);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x_end>>8, false);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y_end, false);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y_end>>8, false);//6fh     
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_SQUARE, true);//76h,0xa0
}

// Draw a filled rectangle. Note: damages text color register
void RA8876_t3::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color) {
	x += _originx;
	y += _originy;
	int16_t x_end = x+w-1;
	int16_t y_end = y+h-1;
	if((x >= _displayclipx2)   || // Clip right
		 (y >= _displayclipy2) || // Clip bottom
		 (x_end < _displayclipx1)    || // Clip left
		 (y_end < _displayclipy1))  	// Clip top 
	{
		// outside the clip rectangle
		return;
	}
	if (x < _displayclipx1) x = _displayclipx1;
	if (y < _displayclipy1) y = _displayclipy1;
	if (x_end > _displayclipx2) x_end = _displayclipx2;
	if (y_end > _displayclipy2) y_end = _displayclipy2;
	
  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
  switch (_rotation) {
  	case 1: swapvals(x,y); swapvals(x_end, y_end); break;
  	case 2: x = _width-x; x_end = _width - x_end;; break;
  	case 3: rotateCCXY(x,y); rotateCCXY(x_end, y_end); break;
  }

  lcdRegDataWrite(RA8876_DLHSR0,x, false);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x>>8, false);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y, false);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y>>8, false);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x_end, false);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x_end>>8, false);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y_end, false);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y_end>>8, false);//6fh   
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_SQUARE_FILL, true);//76h,0xE0  

}

// fillRectHGradient	- fills area with horizontal gradient
void RA8876_t3::fillRectHGradient(int16_t x, int16_t y, int16_t w, int16_t h,
                                            uint16_t color1, uint16_t color2) {
  x += _originx;
  y += _originy;

  // Rectangular clipping
  if ((x >= _displayclipx2) || (y >= _displayclipy2))
      return;
  if (x < _displayclipx1) {
      w -= (_displayclipx1 - x);
      x = _displayclipx1;
  }
  if (y < _displayclipy1) {
      h -= (_displayclipy1 - y);
      y = _displayclipy1;
  }
  if ((x + w - 1) >= _displayclipx2)
      w = _displayclipx2 - x;
  if ((y + h - 1) >= _displayclipy2)
      h = _displayclipy2 - y;

  int16_t r1, g1, b1, r2, g2, b2, dr, dg, db, r, g, b;
  uint16_t color;
  color565toRGB14(color1, r1, g1, b1);
  color565toRGB14(color2, r2, g2, b2);
  dr = (r2 - r1) / w;
  dg = (g2 - g1) / w;
  db = (b2 - b1) / w;
  r = r1;
  g = g1;
  b = b1;
  
  check2dBusy();
  graphicMode(true);
  for (uint16_t j = h; j > 0; j--) { //y
      for (uint16_t i = w; i > 0; i--) { //x
          color = RGB14tocolor565(r, g, b);
          drawPixel(x + i, y + j, color);
          r += dr;
          g += dg;
          b += db;
      }
      //color = RGB14tocolor565(r, g, b);
      //drawPixel(x, y, color);
      r = r1;
      g = g1;
      b = b1;
  }
}
    

void RA8876_t3::fillRectVGradient(int16_t x, int16_t y, int16_t w, int16_t h,
                                            uint16_t color1, uint16_t color2) {
    x += _originx;
    y += _originy;

    // Rectangular clipping
    if ((x >= _displayclipx2) || (y >= _displayclipy2))
        return;
    if (x < _displayclipx1) {
        w -= (_displayclipx1 - x);
        x = _displayclipx1;
    }
    if (y < _displayclipy1) {
        h -= (_displayclipy1 - y);
        y = _displayclipy1;
    }
    if ((x + w - 1) >= _displayclipx2)
        w = _displayclipx2 - x;
    if ((y + h - 1) >= _displayclipy2)
        h = _displayclipy2 - y;

    int16_t r1, g1, b1, r2, g2, b2, dr, dg, db, r, g, b;
    color565toRGB14(color1, r1, g1, b1);
    color565toRGB14(color2, r2, g2, b2);
    dr = (r2 - r1) / h;
    dg = (g2 - g1) / h;
    db = (b2 - b1) / h;
    r = r1;
    g = g1;
    b = b1;
    
    check2dBusy();
    graphicMode(true);
    
    for (uint16_t j = h; j > 0; j--) {
        uint16_t color = RGB14tocolor565(r, g, b);
        for (uint16_t i = w; i > 0; i--) {
          drawPixel(x + i , y + j, color);
        }
        //drawPixel(x + i , y + j, color);
        r += dr;
        g += dg;
        b += db;
    }

}


void RA8876_t3::writeRect(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors) {
	uint16_t start_x = (x != CENTER) ? x : (_width - w) / 2;
	uint16_t start_y = (y != CENTER) ? y : (_height - h) / 2;

	switch (_rotation) {
		case 0: // we will just hand off for now to 
				// unrolled to bte call
				//Using the BTE function is faster and will use DMA if available
			    bteMpuWriteWithROPData8(currentPage, width(), start_x, start_y,  //Source 1 is ignored for ROP 12
                              currentPage, width(), start_x, start_y, w, h,     //destination address, pagewidth, x/y, width/height
                              RA8876_BTE_ROP_CODE_12,
                              ( const unsigned char *)pcolors);
			break;
		case 1:
			{
				while (h) {
					//Serial.printf("DP %x, %d, %d %d\n", rotated_row, h, start_x, y);
				    bteMpuWriteWithROPData8(currentPage, height(), start_y, start_x,  //Source 1 is ignored for ROP 12
	                              currentPage, height(),  start_y, start_x, 1, w,     //destination address, pagewidth, x/y, width/height
	                              RA8876_BTE_ROP_CODE_12,
	                              ( const unsigned char *)pcolors);
				    start_y++;
				    h--;
				    pcolors += w;

				}
			}

			break;
		case 2:
			{
				uint16_t *rotated_buffer_alloc = (uint16_t*)malloc(w * 2 + 32);
				if (!rotated_buffer_alloc) return; // failed to allocate. 
			    uint16_t *rotated_buffer = (uint16_t *)(((uintptr_t)rotated_buffer_alloc + 32) & ~((uintptr_t)(31)));
				// unrolled to bte call
				//Using the BTE function is faster and will use DMA if available
				// We reverse the colors in the row...
				// lets reverse data per row...
				while (h) {
					for (int i = 0; i < w; i++) rotated_buffer[w-i-1] = *pcolors++;
				    bteMpuWriteWithROPData8(currentPage, width(), start_x, start_y,  //Source 1 is ignored for ROP 12
                              currentPage, width(), (width()- w) - start_x , start_y, w, 1,     //destination address, pagewidth, x/y, width/height
                              RA8876_BTE_ROP_CODE_12,
                              ( const unsigned char *)rotated_buffer);
				    start_y++;
				    h--;
				}
				#ifdef SPI_HAS_TRANSFER_ASYNC
				while(activeDMA) {}; //wait forever while DMA is finishing- can't start a new transfer
				#endif
				free((void*)rotated_buffer_alloc);
				endSend(true);
			}
			break;
		case 3:
			{
			    start_y += h;
				while (h) {
					//Serial.printf("DP %x, %d, %d %d\n", rotated_row, h, start_x, y);
				    bteMpuWriteWithROPData8(currentPage, height(), start_y, start_x,  //Source 1 is ignored for ROP 12
                           currentPage, height(), height() - start_y, start_x, 1, w,     //destination address, pagewidth, x/y, width/height
	                              RA8876_BTE_ROP_CODE_12,
	                              ( const unsigned char *)pcolors);
				    start_y--;
				    h--;
				    pcolors += w;
				}
			}
			break;
	}
}

uint16_t *RA8876_t3::rotateImageRect(int16_t w, int16_t h, const uint16_t *pcolors, int16_t rotation) 
{
	uint16_t *rotated_colors_alloc = (uint16_t *)malloc(w * h *2+32);
	int16_t x, y;
	if (!rotated_colors_alloc) 
		return nullptr; 
    uint16_t *rotated_colors_aligned = (uint16_t *)(((uintptr_t)rotated_colors_alloc + 32) & ~((uintptr_t)(31)));

	if ((rotation < 0) || (rotation > 3)) rotation = _rotation;  // just use current one. 
	Serial.printf("rotateImageRect %d %d %d %x %x\n", w, h, rotation, pcolors, rotated_colors_aligned);
	switch (rotation) {
		case 0:
			memcpy((uint8_t *)rotated_colors_aligned, (uint8_t*)pcolors, w*h*2);
			break;
		case 1:
			// reorder 
			for(y = 0; y < h; y++) {
				for(x = 0; x < w; x++) {
					rotated_colors_aligned[x*h+y] =*pcolors++;
				}
			}
			break;
		case 2:
			int y_offset;
			for(y = 0; y < h; y++) {
				y_offset = y*w;	// reverse the rows
				for(x = 1; x <= w; x++) {
					rotated_colors_aligned[y_offset + (w-x)] = *pcolors++;
				}
			}
			break;
		case 3:
			// reorder 
			for(y = 1; y <= h; y++) {
				for(x = 0; x < w; x++) {
					rotated_colors_aligned[x*h+(h-y)] = *pcolors++;
				}
			}
			break;
	}

	return rotated_colors_alloc;
}

// This one assumes that the data was previously arranged such that you can just ROP it out...
void RA8876_t3::writeRotatedRect(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors) 
{

	Serial.printf("writeRotatedRect %d %d %d %d (%x)\n", x, y, w, h, pcolors);
	uint16_t start_x = (x != CENTER) ? x : (_width - w) / 2;
	uint16_t start_y = (y != CENTER) ? y : (_height - h) / 2;

    uint16_t *pcolors_aligned = (uint16_t *)(((uintptr_t)pcolors + 32) & ~((uintptr_t)(31)));
	switch (_rotation) {
		case 0: 
		case 2:
			// Same as normal writeRect
		    bteMpuWriteWithROPData8(currentPage, width(), start_x, start_y,  //Source 1 is ignored for ROP 12
                          currentPage, width(), start_x, start_y, w, h,     //destination address, pagewidth, x/y, width/height
                          RA8876_BTE_ROP_CODE_12,
                          ( const unsigned char *)pcolors_aligned);
			break;
		case 1:
		case 3:
		    bteMpuWriteWithROPData8(currentPage, height(), start_y, start_x,  //Source 1 is ignored for ROP 12
                          currentPage, height(),  start_y, start_x, h, w,     //destination address, pagewidth, x/y, width/height
                          RA8876_BTE_ROP_CODE_12,
                          ( const unsigned char *)pcolors_aligned);

			break;
	}

}

void RA8876_t3::writeRect8BPP(int16_t x, int16_t y, int16_t w, int16_t h,
                                        const uint8_t *pixels,
                                        const uint16_t *palette) {
    // Serial.printf("\nWR8: %d %d %d %d %x\n", x, y, w, h, (uint32_t)pixels);
    
    if (x == CENTER) x = (_width - w) / 2;
    if (y == CENTER) y = (_height - h) / 2;

    x += _originx;
    y += _originy;
    int16_t w_pixels = w; // initial w passed in...

    uint16_t x_clip_left =
        0; // How many entries at start of colors to skip at start of row
    uint16_t x_clip_right =
        0; // how many color entries to skip at end of row for clipping
    // Rectangular clipping

    // See if the whole thing out of bounds...
    if ((x >= _displayclipx2) || (y >= _displayclipy2))
        return;
    if (((x + w) <= _displayclipx1) || ((y + h) <= _displayclipy1))
        return;

    // In these cases you can not do simple clipping, as we need to synchronize
    // the colors array with the
    // We can clip the height as when we get to the last visible we don't have to
    // go any farther.
    // also maybe starting y as we will advance the color array.
    if (y < _displayclipy1) {
        int dy = (_displayclipy1 - y);
        h -= dy;
        pixels += (dy * w); // Advance color array to
        y = _displayclipy1;
    }

    if ((y + h - 1) >= _displayclipy2)
        h = _displayclipy2 - y;

    // For X see how many items in color array to skip at start of row and
    // likewise end of row
    if (x < _displayclipx1) {
        x_clip_left = _displayclipx1 - x;
        w -= x_clip_left;
        x = _displayclipx1;
    }
    if ((x + w - 1) >= _displayclipx2) {
        x_clip_right = w;
        w = _displayclipx2 - x;
        x_clip_right -= w;
    }
    const uint8_t *pixels_row_start = pixels; // remember our starting position offset into row

    uint16_t buffer[w] __attribute__((aligned(32)));

    for (; h > 0; h--) {
        pixels = pixels_row_start;            // setup for this row
        uint16_t *pb = buffer;
        for (int i = 0; i < w; i++) {*pb++ = palette[*pixels++]; }

        writeRect(x, y, w, 1, buffer);
        y++;
        pixels_row_start += w_pixels;
    }
}



// writeRect4BPP -  write 4 bit per pixel paletted bitmap
//          bitmap data in array at pixels, 4 bits per
// pixel
//          color palette data in array at palette
//          width must be at least 2 pixels
void RA8876_t3::writeRect4BPP(int16_t x, int16_t y, int16_t w, int16_t h,
                                        const uint8_t *pixels,
                                        const uint16_t *palette) {
    // Simply call through our helper
    writeRectNBPP(x, y, w, h, 4, pixels, palette);
}

// writeRect2BPP -  write 2 bit per pixel paletted bitmap
//          bitmap data in array at pixels, 4 bits per
// pixel
//          color palette data in array at palette
//          width must be at least 4 pixels
void RA8876_t3::writeRect2BPP(int16_t x, int16_t y, int16_t w, int16_t h,
                                        const uint8_t *pixels,
                                        const uint16_t *palette) {
    // Simply call through our helper
    writeRectNBPP(x, y, w, h, 2, pixels, palette);
}

///============================================================================
// writeRect1BPP -  write 1 bit per pixel paletted bitmap
//          bitmap data in array at pixels, 4 bits per
// pixel
//          color palette data in array at palette
//          width must be at least 8 pixels
void RA8876_t3::writeRect1BPP(int16_t x, int16_t y, int16_t w, int16_t h,
                                        const uint8_t *pixels,
                                        const uint16_t *palette) {
    // Simply call through our helper
    writeRectNBPP(x, y, w, h, 1, pixels, palette);
}

///============================================================================
// writeRectNBPP -  write N(1, 2, 4, 8) bit per pixel paletted bitmap
//          bitmap data in array at pixels
//  Currently writeRect1BPP, writeRect2BPP, writeRect4BPP use this to do all of
//  the work.
void RA8876_t3::writeRectNBPP(int16_t x, int16_t y, int16_t w, int16_t h,
                                        uint8_t bits_per_pixel, const uint8_t *pixels,
                                        const uint16_t *palette) {
    // Serial.printf("\nWR8: %d %d %d %d %x\n", x, y, w, h, (uint32_t)pixels);
    x += _originx;
    y += _originy;
    uint8_t pixels_per_byte = 8 / bits_per_pixel;
    uint16_t count_of_bytes_per_row =
        (w + pixels_per_byte - 1) /
        pixels_per_byte; // Round up to handle non multiples
    uint8_t row_shift_init =
        8 - bits_per_pixel;                             // We shift down 6 bits by default
    uint8_t pixel_bit_mask = (1 << bits_per_pixel) - 1; // get mask to use below
    // Rectangular clipping

    // See if the whole thing out of bounds...
    if ((x >= _displayclipx2) || (y >= _displayclipy2))
        return;
    if (((x + w) <= _displayclipx1) || ((y + h) <= _displayclipy1))
        return;

    // In these cases you can not do simple clipping, as we need to synchronize
    // the colors array with the
    // We can clip the height as when we get to the last visible we don't have to
    // go any farther.
    // also maybe starting y as we will advance the color array.
    // Again assume multiple of 8 for width
    if (y < _displayclipy1) {
        int dy = (_displayclipy1 - y);
        h -= dy;
        pixels += dy * count_of_bytes_per_row;
        y = _displayclipy1;
    }

    if ((y + h - 1) >= _displayclipy2)
        h = _displayclipy2 - y;

    // For X see how many items in color array to skip at start of row and
    // likewise end of row
    if (x < _displayclipx1) {
        uint16_t x_clip_left = _displayclipx1 - x;
        w -= x_clip_left;
        x = _displayclipx1;
        // Now lets update pixels to the rigth offset and mask
        uint8_t x_clip_left_bytes_incr = x_clip_left / pixels_per_byte;
        pixels += x_clip_left_bytes_incr;
        row_shift_init =
            8 -
            (x_clip_left - (x_clip_left_bytes_incr * pixels_per_byte) + 1) *
                bits_per_pixel;
    }

    if ((x + w - 1) >= _displayclipx2) {
        w = _displayclipx2 - x;
    }

    const uint8_t *pixels_row_start = pixels; // remember our starting position offset into row


    uint16_t buffer[w] __attribute__((aligned(32)));
    for (; h > 0; h--) {
        pixels = pixels_row_start;            // setup for this row
        uint8_t pixel_shift = row_shift_init; // Setup mask
        uint16_t *pb = buffer;
        for (int i = 0; i < w; i++) {
            *pb++ = palette[((*pixels) >> pixel_shift) & pixel_bit_mask];
            if (!pixel_shift) {
                pixel_shift = 8 - bits_per_pixel; // setup next mask
                pixels++;
            } else {
                pixel_shift -= bits_per_pixel;
            }
        }
        writeRect(x, y, w, 1, buffer);
        y++;
        pixels_row_start += count_of_bytes_per_row;
    }
}




// Draw a round rectangle. 
void RA8876_t3::drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t xr, uint16_t yr, uint16_t color) {
	x += _originx;
	y += _originy;
	int16_t x_end = x+w-1;
	int16_t y_end = y+h-1;
	if((x >= _displayclipx2)   || // Clip right
		 (y >= _displayclipy2) || // Clip bottom
		 (x_end < _displayclipx1)    || // Clip left
		 (y_end < _displayclipy1))  	// Clip top 
	{
		// outside the clip rectangle
		return;
	}
	if (x < _displayclipx1) x = _displayclipx1;
	if (y < _displayclipy1) y = _displayclipy1;
	if (x_end > _displayclipx2) x_end = _displayclipx2;
	if (y_end > _displayclipy2) y_end = _displayclipy2;
	//drawCircleSquare(x, y, x_end, y_end, xr, yr, color);
	
  check2dBusy();
  graphicMode(true);
  foreGroundColor16bpp(color);
	switch (_rotation) {
		case 1: swapvals(x,y); swapvals(x_end, y_end); swapvals(xr, yr); break;
		case 2:  x = _width-x; x_end = _width - x_end;   break;
		case 3: rotateCCXY(x,y); rotateCCXY(x_end, y_end); swapvals(xr, yr); break;
  	}
  lcdRegDataWrite(RA8876_DLHSR0,x, false);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x>>8, false);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y, false);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y>>8, false);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x_end, false);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x_end>>8, false);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y_end, false);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y_end>>8, false);//6fh    
  lcdRegDataWrite(RA8876_ELL_A0,xr, false);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,xr>>8, false);//79h 
  lcdRegDataWrite(RA8876_ELL_B0,yr, false);//7ah    
  lcdRegDataWrite(RA8876_ELL_B1,yr>>8, false);//7bh
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE_SQUARE, true);//76h,0xb0
	
}

// Draw a filed round rectangle.
void RA8876_t3::fillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t xr, uint16_t yr, uint16_t color) {
		x += _originx;
	y += _originy;
	int16_t x_end = x+w-1;
	int16_t y_end = y+h-1;
	if((x >= _displayclipx2)   || // Clip right
		 (y >= _displayclipy2) || // Clip bottom
		 (x_end < _displayclipx1)    || // Clip left
		 (y_end < _displayclipy1))  	// Clip top 
	{
		// outside the clip rectangle
		return;
	}
	if (x < _displayclipx1) x = _displayclipx1;
	if (y < _displayclipy1) y = _displayclipy1;
	if (x_end > _displayclipx2) x_end = _displayclipx2;
	if (y_end > _displayclipy2) y_end = _displayclipy2;
	
  check2dBusy();
  //graphicMode(true);
  foreGroundColor16bpp(color);
	switch (_rotation) {
		case 1: swapvals(x,y); swapvals(x_end, y_end); swapvals(xr, yr); break;
		case 2:  x = _width-x; x_end = _width - x_end;   break;
		case 3: rotateCCXY(x,y); rotateCCXY(x_end, y_end); swapvals(xr, yr); break;
  	}

  lcdRegDataWrite(RA8876_DLHSR0,x, false);//68h
  lcdRegDataWrite(RA8876_DLHSR1,x>>8, false);//69h
  lcdRegDataWrite(RA8876_DLVSR0,y, false);//6ah
  lcdRegDataWrite(RA8876_DLVSR1,y>>8, false);//6bh
  lcdRegDataWrite(RA8876_DLHER0,x_end, false);//6ch
  lcdRegDataWrite(RA8876_DLHER1,x_end>>8, false);//6dh
  lcdRegDataWrite(RA8876_DLVER0,y_end, false);//6eh
  lcdRegDataWrite(RA8876_DLVER1,y_end>>8, false);//6fh    
  lcdRegDataWrite(RA8876_ELL_A0,xr, false);//77h    
  lcdRegDataWrite(RA8876_ELL_A1,xr>>8, false);//78h 
  lcdRegDataWrite(RA8876_ELL_B0,yr, false);//79h    
  lcdRegDataWrite(RA8876_ELL_B1,yr>>8, false);//7ah
  lcdRegDataWrite(RA8876_DCR1,RA8876_DRAW_CIRCLE_SQUARE_FILL, true);//76h,0xf0
}

// Enable Touch Screen.
void RA8876_t3::touchEnable(boolean enabled) {
//		touchEnable(enabled);
}

// Return true if screen touched else false
boolean touchDetect(boolean autoclear) {
//	return touchDetect(autoclear);
return false; // return false for now
}

// Draw a filled circle
void RA8876_t3::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	drawCircleFill(x0, y0, r, color);
}

// Draw a filled ellipse.
void RA8876_t3::fillEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color) {
	drawEllipseFill(xCenter, yCenter, longAxis, shortAxis, color);
}

// Draw a filled triangle
void RA8876_t3::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
	drawTriangleFill(x0, y0, x1, y1, x2, y2, color);
}

// Read touch screen ADC
void RA8876_t3::readTouchADC(uint16_t *x, uint16_t *y) {
//	touchReadAdc(x, y);//we using 10bit adc data here
}

// Setup graphic mouse cursor
void RA8876_t3::gCursorSet(boolean gCursorEnable, uint8_t gcursortype, uint8_t gcursorcolor1, uint8_t gcursorcolor2) {
	if (!gCursorEnable)
		Disable_Graphic_Cursor();
	else
		Enable_Graphic_Cursor();
	switch(gcursortype) {
		case 1:
			Select_Graphic_Cursor_1(); // PEN
			break;
		case 2:
			Select_Graphic_Cursor_2(); // ARROW
			break;
		case 3:
			Select_Graphic_Cursor_3(); // HOUR GLASS
			break;
		case 4:
			Select_Graphic_Cursor_4(); // ERROR SYMBOL
			break;
		default:
			Select_Graphic_Cursor_2();
	}
	Set_Graphic_Cursor_Color_1(gcursorcolor1);
	Set_Graphic_Cursor_Color_2(gcursorcolor2);
}

// Set graphic cursor position on screen
void RA8876_t3::gcursorxy(uint16_t gcx, uint16_t gcy) {
	Graphic_Cursor_XY(gcx, gcy);
}

//==========================================================================================
//= The following Graphic Button functions are based on Adafruits Graphic button libraries =
//==========================================================================================
// Initialize a graphic button. 
void RA8876_t3::initButton(struct Gbuttons *buttons, uint16_t x, uint16_t y, uint8_t w, uint8_t h,
 uint16_t outline, uint16_t fill, uint16_t textcolor,
 char *label, uint8_t textsize)
{
	buttons->x = x;
	buttons->y = y;
	buttons->w = w;
	buttons->h = h;
	buttons->outlinecolor = outline;
	buttons->fillcolor = fill;
	buttons->textcolor = textcolor;
	buttons->textsize  = textsize;
	strncpy(buttons->label, label, 9);
	buttons->label[9] = 0;
}

// Draw graphic button. (not completely implemented yet)
void RA8876_t3::drawButton(struct Gbuttons *buttons, boolean inverted) {
  uint16_t fill, outline;
//  uint16_t text;
  uint16_t fgcolor = _TXTForeColor;	
  uint16_t bgcolor = _TXTBackColor;	

  if(!inverted) {
    fill    = buttons->fillcolor;
    outline = buttons->outlinecolor;
//    text    = buttons->textcolor;
  } else {
    fill    = buttons->textcolor;
    outline = buttons->outlinecolor;
//    text    = buttons->fillcolor;
  }
  drawCircleSquareFill(buttons->x, buttons->y, buttons->w+buttons->x, buttons->h+buttons->y,
					   min(buttons->w,buttons->h)/4, min(buttons->w,buttons->h)/4, fill);

  drawCircleSquare(buttons->x, buttons->y, buttons->w+buttons->x, buttons->h+buttons->y,
				   min(buttons->w,buttons->h)/4, min(buttons->w,buttons->h)/4, outline);

//  _gfx->setCursor(_x - strlen(_label)*3*_textsize, _y-4*_textsize);
//  _gfx->setTextColor(text);
//  _gfx->setTextSize(_textsize);
//  _gfx->print(_label);
  textColor(fgcolor,bgcolor);	// restore colors	
}

// Check the button to see if it is within the range of selection.
boolean RA8876_t3::buttonContains(struct Gbuttons *buttons, uint16_t x, uint16_t y) {
  if ((x < buttons->x) || (x > (buttons->x + buttons->w))) return false;
  if ((y < buttons->y) || (y > (buttons->y + buttons->h))) return false;
  return true;
}

// Signal state of buttons: true = pressed, false = released
void RA8876_t3::buttonPress(struct Gbuttons *buttons, boolean p) {
  buttons->laststate = buttons->currstate;
  buttons->currstate = p;
}

// Check the current state of a button: pressed/released
boolean RA8876_t3::buttonIsPressed(struct Gbuttons *buttons) {
	return buttons->currstate;
}

// Check if button was just pressed
boolean RA8876_t3::buttonJustPressed(struct Gbuttons *buttons) {
	return (buttons->currstate && !buttons->laststate);
 }

// Check if button was just released
boolean RA8876_t3::buttonJustReleased(struct Gbuttons *buttons) {
	return (!buttons->currstate && buttons->laststate);
}
//==========================================================================================

// Check for Touch Screen being touched
boolean RA8876_t3::TStouched(void ) {
//    return touch.isTouching();
return false;
}

// Get Touch Screen x/y coords.
void RA8876_t3::getTSpoint(uint16_t *x, uint16_t *y) {
//	touch.getPosition(x,y);
}

// Put a picture on the screen using raw picture data
// This is a simplified wrapper - more advanced uses (such as putting data onto a page other than current) 
//   should use the underlying BTE functions.
void RA8876_t3::putPicture(ru16 x, ru16 y, ru16 w, ru16 h, const unsigned char *data) {
	//The putPicture_16bppData8 function in the base class is not ideal - it damages the activeWindow setting
	//It also is harder to make it DMA.
	//Ra8876_Lite::putPicture_16bppData8(x, y, w, h, data);

	//Using the BTE function is faster and will use DMA if available
    bteMpuWriteWithROPData8(currentPage, width(), x, y,  //Source 1 is ignored for ROP 12
                              currentPage, width(), x, y, w, h,     //destination address, pagewidth, x/y, width/height
                              RA8876_BTE_ROP_CODE_12,
                              data);
}


// Scroll the screen up one text line
void RA8876_t3::scrollUp(void ) {
	scroll();
}

//=======================================================================
//= RA8876 BTE functions (Block Transfer Engine)                        =
//=======================================================================
// Copy box size part of the current screen page to another screen page
/*uint32_t RA8876_t3::tft_boxPut(uint32_t vPageAddr, uint16_t x0, uint16_t y0,
					uint16_t x1, uint16_t y1, uint16_t dx0, uint16_t dy0) {
	return boxPut(vPageAddr, x0, y0, x1, y1, dx0, dy0);
}

// Copy a box size part of another screen page to the current screen page
uint32_t RA8876_t3::tft_boxGet(uint32_t vPageAddr, uint16_t x0, uint16_t y0,
					uint16_t x1, uint16_t y1, uint16_t dx0, uint16_t dy0) {
	return boxGet(vPageAddr, x0, y0, x1, y1, dx0, dy0);
}
*/

//=======================================================================
//= PIP window function (Display one of two or both PIP windows)        =
//=======================================================================
/*void RA8876_t3::tft_PIP (
 unsigned char On_Off // 0 : disable PIP, 1 : enable PIP, 2 : To maintain the original state
,unsigned char Select_PIP // 1 : use PIP1 , 2 : use PIP2
,unsigned long PAddr //start address of PIP
,unsigned short XP //coordinate X of PIP Window, It must be divided by 4.
,unsigned short YP //coordinate Y of PIP Window, It must be divided by 4.
,unsigned long ImageWidth //Image Width of PIP (recommend = canvas image width)
,unsigned short X_Dis //coordinate X of Display Window
,unsigned short Y_Dis //coordinate Y of Display Window
,unsigned short X_W //width of PIP and Display Window, It must be divided by 4.
,unsigned short Y_H //height of PIP and Display Window , It must be divided by 4.
) {
	PIP (On_Off, Select_PIP, PAddr, XP, YP, ImageWidth, X_Dis, Y_Dis, X_W, Y_H);
}
*/


void RA8876_t3::_fontWrite(const uint8_t* buffer, uint16_t len)
{
	if(_use_default) {
		//if (_FNTgrandient) _FNTgrandient = false;//cannot use this with write
		//_textWrite((const char *)buffer, len);
		//Serial.printf("Default: %c, %d, %d\n", c, _cursorX, _cursorY);
		return;
	}
	// Lets try to handle some of the special font centering code that was done for default fonts.
	if (_absoluteCenter || _relativeCenter ) {
		int16_t x, y;
	  	uint16_t strngWidth, strngHeight;
	  	getTextBounds(buffer, len, 0, 0, &x, &y, &strngWidth, &strngHeight);
	  	//Serial.printf("_fontwrite bounds: %d %d %u %u\n", x, y, strngWidth, strngHeight);
	  	// Note we may want to play with the x ane y returned if they offset some
		if (_absoluteCenter && strngWidth > 0){//Avoid operations for strngWidth = 0
			_absoluteCenter = false;
			_cursorX = _cursorX - ((x + strngWidth) / 2);
			_cursorY = _cursorY - ((y + strngHeight) / 2);
		} else if (_relativeCenter && strngWidth > 0){//Avoid operations for strngWidth = 0
			_relativeCenter = false;
			if (bitRead(_TXTparameters,5)) {//X = center
				_cursorX = (_width / 2) - ((x + strngWidth) / 2);
				_TXTparameters &= ~(1 << 5);//reset
			}
			if (bitRead(_TXTparameters,6)) {//Y = center
				_cursorY = (_height / 2) - ((y + strngHeight) / 2) ;
				_TXTparameters &= ~(1 << 6);//reset
			}
		}
	}

	while(len) {
		uint8_t c = *buffer++;
		if (font) {
			//Serial.printf("ILI: %c, %d, %d\n", c, _cursorX, _cursorY);
			if (c == '\n') {
				//_cursorY += font->line_space;
				//_cursorX  = 0;
			} else {
			  if (c == 13) {
				_cursorY += font->line_space;
				_cursorX  = 0;
			  } else {
				drawFontChar(c);
			  }
			}
		} else if (gfxFont)  {
			//Serial.printf("GFX: %c, %d, %d\n", c, _cursorX, _cursorY);
			if (c == '\n') {
	            _cursorY += (int16_t)textsize_y * gfxFont->yAdvance;
				_cursorX  = 0;
			} else {
				drawGFXFontChar(c);
			}
		} else {
			if (c == '\n') {
				_cursorY += textsize_y*8;
				_cursorX  = 0;
			} else if (c == '\r') {
				// skip em
			} else {
				drawChar(_cursorX, _cursorY, c, _TXTForeColor, _TXTBackColor, textsize_x, textsize_y);
				_cursorX += textsize_x*6;
				if (wrap && (_cursorX > (_width - textsize_x*6))) {
					_cursorY += textsize_y*6;
					_cursorX = 0;
				}
			}
		}
		len--;
	}
}


//#include "glcdfont.c"
extern "C" const unsigned char glcdfont[];

// Draw a character
void RA8876_t3::drawChar(int16_t x, int16_t y, unsigned char c,
			    uint16_t fgcolor, uint16_t bgcolor, uint8_t size_x, uint8_t size_y)
{
	if((x >= _width)            || // Clip right
	   (y >= _height)           || // Clip bottom
	   ((x + 6 * size_x - 1) < 0) || // Clip left  TODO: is this correct?
	   ((y + 8 * size_y - 1) < 0))   // Clip top   TODO: is this correct?
		return;
  // BUGBUG check to see if we are opaque or not.
	//Serial.printf("drawchar %d %d %c %x %x %d %d %x\n", x, y, c, fgcolor, bgcolor, size_x, size_y, _backTransparent);
	if (_backTransparent) {
		// This transparent approach is only about 20% faster
		if ((size_x == 1) && (size_y == 1)) {
			uint8_t mask = 0x01;
			int16_t xoff, yoff;
			for (yoff=0; yoff < 8; yoff++) {
				uint8_t line = 0;
				for (xoff=0; xoff < 5; xoff++) {
					if (glcdfont[c * 5 + xoff] & mask) line |= 1;
					line <<= 1;
				}
				line >>= 1;
				xoff = 0;
				while (line) {
					if (line == 0x1F) {
						drawFastHLine(x + xoff, y + yoff, 5, fgcolor);
						break;
					} else if (line == 0x1E) {
						drawFastHLine(x + xoff, y + yoff, 4, fgcolor);
						break;
					} else if ((line & 0x1C) == 0x1C) {
						drawFastHLine(x + xoff, y + yoff, 3, fgcolor);
						line <<= 4;
						xoff += 4;
					} else if ((line & 0x18) == 0x18) {
						drawFastHLine(x + xoff, y + yoff, 2, fgcolor);
						line <<= 3;
						xoff += 3;
					} else if ((line & 0x10) == 0x10) {
						drawPixel(x + xoff, y + yoff, fgcolor);
						line <<= 2;
						xoff += 2;
					} else {
						line <<= 1;
						xoff += 1;
					}
				}
				mask = mask << 1;
			}
		} else {
			uint8_t mask = 0x01;
			int16_t xoff, yoff;
			for (yoff=0; yoff < 8; yoff++) {
				uint8_t line = 0;
				for (xoff=0; xoff < 5; xoff++) {
					if (glcdfont[c * 5 + xoff] & mask) line |= 1;
					line <<= 1;
				}
				line >>= 1;
				xoff = 0;
				while (line) {
					if (line == 0x1F) {
						fillRect(x + xoff * size_x, y + yoff * size_y,
							5 * size_x, size_y, fgcolor);
						break;
					} else if (line == 0x1E) {
						fillRect(x + xoff * size_x, y + yoff * size_y,
							4 * size_x, size_y, fgcolor);
						break;
					} else if ((line & 0x1C) == 0x1C) {
						fillRect(x + xoff * size_x, y + yoff * size_y,
							3 * size_x, size_y, fgcolor);
						line <<= 4;
						xoff += 4;
					} else if ((line & 0x18) == 0x18) {
						fillRect(x + xoff * size_x, y + yoff * size_y,
							2 * size_x, size_y, fgcolor);
						line <<= 3;
						xoff += 3;
					} else if ((line & 0x10) == 0x10) {
						fillRect(x + xoff * size_x, y + yoff * size_y,
							size_x, size_y, fgcolor);
						line <<= 2;
						xoff += 2;
					} else {
						line <<= 1;
						xoff += 1;
					}
				}
				mask = mask << 1;
			}
		}
	} else {
		// This solid background approach is about 5 time faster
		uint8_t xc, yc;
		uint8_t xr, yr;
		uint8_t mask = 0x01;

		// We need to offset by the origin.
		x+=_originx;
		y+=_originy;
		int16_t x_char_start = x;  // remember our X where we start outputting...

		if((x >= _displayclipx2)            || // Clip right
			 (y >= _displayclipy2)           || // Clip bottom
			 ((x + 6 * size_x - 1) < _displayclipx1) || // Clip left  TODO: this is not correct
			 ((y + 8 * size_y - 1) < _displayclipy1))   // Clip top   TODO: this is not correct
			return;


    int16_t w =  6 * size_x;
    int16_t h = 8 * size_y;
    int16_t y_char_top = y; // remember the y
      uint16_t char_buffer[w * h];
      uint16_t color;
      uint16_t *pfbPixel_row = char_buffer;
      for (yc = 0; (yc < 8) && (y < _displayclipy2); yc++) {
        for (yr = 0; (yr < size_y) && (y < _displayclipy2); yr++) {
          x = x_char_start; // get our first x position...
          if (y >= _displayclipy1) {
            uint16_t *pfbPixel = pfbPixel_row;
            for (xc = 0; xc < 5; xc++) {
              if (glcdfont[c * 5 + xc] & mask) {
                color = fgcolor;
              } else {
                color = bgcolor;
              }
              for (xr = 0; xr < size_x; xr++) {
                if ((x >= _displayclipx1) && (x < _displayclipx2)) {
                  *pfbPixel = color;
                }
                pfbPixel++;
                x++;
              }
            }
            for (xr = 0; xr < size_x; xr++) {
              if ((x >= _displayclipx1) && (x < _displayclipx2)) {
                *pfbPixel = bgcolor;
              }
              pfbPixel++;
              x++;
            }
          }
          pfbPixel_row += w; // setup pointer to
          y++;
        }
        mask = mask << 1;
      }
      writeRect(x_char_start,y_char_top, w, h, char_buffer);
    //writecommand_last(ILI9488_NOP);
		//_endSend();
	}
}

void RA8876_t3::setFont(const ILI9341_t3_font_t &f) {
	_use_default = 0;
	if(_portrait && !_use_gfx_font) {
		_cursorY += _cursorX;
		_cursorX -= _cursorY;
	}
	_use_ili_font = 1;
	_use_gfx_font = 0;
	_use_int_font = 1;
	_use_tfont = 0;
	
	_gfx_last_char_x_write = 0;	// Don't use cached data here
	font = &f;
	if (gfxFont) {
        _cursorY -= 6;
		gfxFont = NULL;
	}
	fontbpp = 1;
	// Calculate additional metrics for Anti-Aliased font support (BDF extn v2.3)
	if (font && font->version==23){
		fontbpp = (font->reserved & 0b000011)+1;
		fontbppindex = (fontbpp >> 2)+1;
		fontbppmask = (1 << (fontbppindex+1))-1;
		fontppb = 8/fontbpp;
		fontalphamx = 31/((1<<fontbpp)-1);
		// Ensure text and bg color are different. Note: use setTextColor to set actual bg color
		if (_TXTForeColor == _TXTBackColor) _TXTBackColor = (_TXTForeColor==0x0000)?0xFFFF:0x0000;	}

}


// Maybe support GFX Fonts as well?
void RA8876_t3::setFont(const GFXfont *f) {
	_use_default = 0;
	if(_portrait && !_use_ili_font) {
		_cursorY += _cursorX;
		_cursorX -= _cursorY;
	}
	_use_gfx_font = 1;
	_use_ili_font = 0;
	_use_int_font = 0;
	_use_tfont = 0;
	font = NULL;	// turn off the other font... 
	_gfx_last_char_x_write = 0;	// Don't use cached data here
	if (f == gfxFont) return;	// same font or lack of so can bail.

    if(f) {            // Font struct pointer passed in?
        if(!gfxFont) { // And no current font struct?
            // Switching from classic to new font behavior.
            // Move cursor pos down 6 pixels so it's on baseline.
            _cursorY += 6;
        }

        // Test wondering high and low of Ys here... 
        int8_t miny_offset = 0;
#if 1
        for (uint8_t i=0; i <= (f->last - f->first); i++) {
        	if (f->glyph[i].yOffset < miny_offset) {
        		miny_offset = f->glyph[i].yOffset;
        	}
        }
#else        
        int max_delta = 0;
        uint8_t index_min = 0;
        uint8_t index_max = 0;

        int8_t minx_offset = 127;
        int8_t maxx_overlap = 0;
        uint8_t indexx_min = 0;
        uint8_t indexx_max = 0;
        for (uint8_t i=0; i <= (f->last - f->first); i++) {
        	if (f->glyph[i].yOffset < miny_offset) {
        		miny_offset = f->glyph[i].yOffset;
        		index_min = i;
        	}

        	if (f->glyph[i].xOffset < minx_offset) {
        		minx_offset = f->glyph[i].xOffset;
        		indexx_min = i;
        	}
        	if ( (f->glyph[i].yOffset + f->glyph[i].height) > max_delta) {
        		max_delta = (f->glyph[i].yOffset + f->glyph[i].height);
        		index_max = i;
        	}
        	int8_t x_overlap = f->glyph[i].xOffset + f->glyph[i].width - f->glyph[i].xAdvance;
        	if (x_overlap > maxx_overlap) {
        		maxx_overlap = x_overlap;
        		indexx_max = i;
        	}
        }
        Serial.printf("Set GFX Font(%x): Y: %d %d(%c) %d(%c) X: %d(%c) %d(%c)\n", (uint32_t)f, f->yAdvance, 
        	miny_offset, index_min + f->first, max_delta, index_max + f->first,
        	minx_offset, indexx_min + f->first, maxx_overlap, indexx_max + f->first);
#endif
        _gfxFont_min_yOffset = miny_offset;	// Probably only thing we need... May cache? 

    } else if(gfxFont) { // NULL passed.  Current font struct defined?
        // Switching from new to classic font behavior.
        // Move cursor pos up 6 pixels so it's at top-left of char.
        _cursorY -= 6;
    }
    gfxFont = f;
}

	
void RA8876_t3::drawFontChar(unsigned int c)
{
	uint32_t bitoffset = 0;
	const uint8_t *data;

	//Serial.printf("drawFontChar(%c) %d (%d, %d) %x %x %x\n", c, c, _cursorX, _cursorY, _TXTBackColor, _TXTForeColor, _backTransparent);

	if (c >= font->index1_first && c <= font->index1_last) {
		bitoffset = c - font->index1_first;
		bitoffset *= font->bits_index;
	} else if (c >= font->index2_first && c <= font->index2_last) {
		bitoffset = c - font->index2_first + font->index1_last - font->index1_first + 1;
		bitoffset *= font->bits_index;
	} 

	// TODO: implement sparse unicode
	
	//Serial.printf("  index =  %d\n", fetchbits_unsigned(font->index, bitoffset, font->bits_index));
	data = font->data + fetchbits_unsigned(font->index, bitoffset, font->bits_index);

	uint32_t encoding = fetchbits_unsigned(data, 0, 3);
	if (encoding != 0) return;
	uint32_t width = fetchbits_unsigned(data, 3, font->bits_width);
	bitoffset = font->bits_width + 3;
	uint32_t height = fetchbits_unsigned(data, bitoffset, font->bits_height);
	bitoffset += font->bits_height;
	//Serial.printf("  size =   %d,%d\n", width, height);
	//Serial.printf("  line space = %d\n", font->line_space);
	//Serial.printf("  cap height = %d\n", font->cap_height);

	int32_t xoffset = fetchbits_signed(data, bitoffset, font->bits_xoffset);
	bitoffset += font->bits_xoffset;
	int32_t yoffset = fetchbits_signed(data, bitoffset, font->bits_yoffset);
	bitoffset += font->bits_yoffset;
	//Serial.printf("  offset = %d,%d\n", xoffset, yoffset);

	uint32_t delta = fetchbits_unsigned(data, bitoffset, font->bits_delta);
	bitoffset += font->bits_delta;
	//Serial.printf("  delta =  %d\n", delta);
	
	//horizontally, we draw every pixel, or none at all
	if (_cursorX < 0) _cursorX = 0;
	int32_t origin_x = _cursorX + xoffset;
	if (origin_x < 0) {
		_cursorX -= xoffset;
		origin_x = 0;
	}
	if (origin_x + (int)width > _width) {
		if (!wrap) return;
		origin_x = 0;
		if (xoffset >= 0) {
			_cursorX = 0;
		} else {
			_cursorX = -xoffset;
		}
		_cursorY += font->line_space;
	}

	if (_cursorY >= _height) return;

	// vertically, the top and/or bottom can be clipped
	int32_t origin_y = _cursorY + font->cap_height - height - yoffset;

	// TODO: compute top skip and number of lines
	int32_t linecount = height;
	//uint32_t loopcount = 0;
	int32_t y = origin_y;
	
	bool opaque = !_backTransparent; //(_TXTBackColor != _TXTForeColor);


	// Going to try a fast Opaque method which works similar to drawChar, which is near the speed of writerect
	if (!opaque) {
		// Anti-alias support
		if (fontbpp>1){
			// This branch should, in most cases, never happen. This is because if an
			// anti-aliased font is being used, textcolor and textbgcolor should always
			// be different. Even though an anti-alised font is being used, pixels in this
			// case will all be solid because pixels are rendered on same colour as themselves!
			// This won't look very good.
			bitoffset = ((bitoffset + 7) & (-8)); // byte-boundary
			uint32_t xp = 0;
			uint8_t halfalpha = 1<<(fontbpp-1);
			while (linecount) {
				uint32_t x = 0;
				while(x<width) {
					// One pixel at a time, either on (if alpha > 0.5) or off
					if (fetchpixel(data, bitoffset, xp)>=halfalpha){
						Pixel(origin_x + x,y, _TXTForeColor);
					}
					bitoffset += fontbpp;
					x++;
					xp++;
				}
				y++;
				linecount--;
			}
		}
		// Soild pixels
		else {
			while (linecount > 0) {
				//Serial.printf("    linecount = %d\n", linecount);
				uint32_t n = 1;
				if (fetchbit(data, bitoffset++) != 0) {
					n = fetchbits_unsigned(data, bitoffset, 3) + 2;
					bitoffset += 3;
				}
				uint32_t x = 0;
				do {
					int32_t xsize = width - x;
					if (xsize > 32) xsize = 32;
					uint32_t bits = fetchbits_unsigned(data, bitoffset, xsize);
					//Serial.printf("    multi line %d %d %x\n", n, x, bits);
					drawFontBits(opaque, bits, xsize, origin_x + x, y, n);
					bitoffset += xsize;
					x += xsize;
				} while (x < width);


				y += n;
				linecount -= n;
				//if (++loopcount > 100) {
					//Serial.println("     abort draw loop");
					//break;
				//}
			}
		}
	} else {  //Opaque
		if (fontbpp>1){
			// Now opaque mode... 
			// Now write out background color for the number of rows above the above the character
			// figure out bounding rectangle... 
			// In this mode we need to update to use the offset and bounding rectangles as we are doing it it direct.
			// also update the Origin 
			int cursor_x_origin = _cursorX + _originx;
			int cursor_y_origin = _cursorY + _originy;
			_cursorX += _originx;
			_cursorY += _originy;

			int start_x = (origin_x < cursor_x_origin) ? origin_x : cursor_x_origin; 	
			if (start_x < 0) start_x = 0;
			
			int start_y = (origin_y < cursor_y_origin) ? origin_y : cursor_y_origin; 
			if (start_y < 0) start_y = 0;
			int end_x = cursor_x_origin + delta; 
			if ((origin_x + (int)width) > end_x)
				end_x = origin_x + (int)width;
			if (end_x >= _displayclipx2)  end_x = _displayclipx2;	
			int end_y = cursor_y_origin + font->line_space; 
			if ((origin_y + (int)height) > end_y)
				end_y = origin_y + (int)height;
			if (end_y >= _displayclipy2) end_y = _displayclipy2;	
			end_x--;	// setup to last one we draw
			end_y--;
			int start_x_min = (start_x >= _displayclipx1) ? start_x : _displayclipx1;
			int start_y_min = (start_y >= _displayclipy1) ? start_y : _displayclipy1;

			// See if anything is in the display area.
			if((end_x < _displayclipx1) ||(start_x >= _displayclipx2) || (end_y < _displayclipy1) || (start_y >= _displayclipy2)) {
				_cursorX += delta;	// could use goto or another indent level...
				return;
			}

			//Serial.printf("drawFontChar(%c) %d\n", c, c);
			//Serial.printf("  size =   %d,%d\n", width, height);
			//Serial.printf("  line space = %d\n", font->line_space);
			//Serial.printf("  offset = %d,%d\n", xoffset, yoffset);
			//Serial.printf("  delta =  %d\n", delta);
			//Serial.printf("  cursor = %d,%d\n", _cursorX, _cursorY);
			//Serial.printf("  origin = %d,%d\n", origin_x, origin_y);

			//Serial.printf("  Bounding: (%d, %d)-(%d, %d)\n", start_x, start_y, end_x, end_y);
			//Serial.printf("  mins (%d %d),\n", start_x_min, start_y_min);

			//}
			// Anti-aliased font

			//Serial.printf("SetAddr %d %d %d %d\n", start_x_min, start_y_min, end_x, end_y);
			// output rectangle we are updating... We have already clipped end_x/y, but not yet start_x/y
			//setActiveWindow( start_x, start_y_min, end_x, end_y);
			//setActiveWindow( start_x, end_x, start_y_min, end_y);
			int screen_y = start_y_min;
			int screen_x;

			// Clear above character
			while (screen_y < origin_y) {
				for (screen_x = start_x_min; screen_x <= end_x; screen_x++) {
					drawPixel(screen_x, screen_y, _TXTBackColor);
				}
				screen_y++;
			}
				
			screen_y = origin_y;
			bitoffset = ((bitoffset + 7) & (-8)); // byte-boundary
			int glyphend_x = origin_x + width;
			uint32_t xp = 0;
			while (linecount) {
				screen_x = start_x;
				while(screen_x<=end_x) {
					// XXX: I'm sure clipping could be done way more efficiently than just chekcing every single pixel, but let's just get this going
					if ((screen_x >= _displayclipx1) && (screen_x < _displayclipx2) && (screen_y >= _displayclipy1) && (screen_y < _displayclipy2)) {
						// Clear before or after pixel
						if ((screen_x<origin_x) || (screen_x>=glyphend_x)){
							drawPixel(screen_x, screen_y, _TXTBackColor);
						}
						// Draw alpha-blended character
						else{
							uint8_t alpha = fetchpixel(data, bitoffset, xp);
							drawPixel(screen_x, screen_y, alphaBlendRGB565Premultiplied( textcolorPrexpanded, textbgcolorPrexpanded, (uint8_t)(alpha * fontalphamx) ) );
							bitoffset += fontbpp;
							xp++;
						}
					} // clip
					screen_x++;
				}
				screen_y++;
				linecount--;
			}
			
			// clear below character
			//setActiveWindow();  //have to do this otherwise it gets clipped
			fillRect(_cursorX, screen_y, delta, _cursorY + font->line_space - screen_y, _TXTBackColor);

		} // anti-aliased

		// 1bpp
		else {
			// Now opaque mode... 
			// Now write out background color for the number of rows above the above the character
			// figure out bounding rectangle... 
			// In this mode we need to update to use the offset and bounding rectangles as we are doing it it direct.
			// also update the Origin 
			fillRect(_cursorX, _cursorY, delta, y - _cursorY, _TXTBackColor);
			while (linecount > 0) {
				//Serial.printf("    linecount = %d\n", linecount);
				uint32_t n = 1;
				if (fetchbit(data, bitoffset++) != 0) {
					n = fetchbits_unsigned(data, bitoffset, 3) + 2;
					bitoffset += 3;
				}
				uint32_t x = 0;
				fillRect(_cursorX, y, origin_x - _cursorX, n, _TXTBackColor);
				do {
					int32_t xsize = width - x;
					if (xsize > 32) xsize = 32;
					uint32_t bits = fetchbits_unsigned(data, bitoffset, xsize);
					//Serial.printf("    multi line %d %d %x\n", n, x, bits);
					drawFontBits(opaque, bits, xsize, origin_x + x, y, n);
					bitoffset += xsize;
					x += xsize;
				} while (x < width);

				if ((width+xoffset) < delta) {
					fillRect(origin_x + x, y, delta - (width+xoffset), n, _TXTBackColor);
				}
				y += n;
				linecount -= n;
				//if (++loopcount > 100) {
					//Serial.println("     abort draw loop");
					//break;
				//}
			}
			fillRect(_cursorX, y, delta, _cursorY + font->line_space - y, _TXTBackColor);
		}
	}
	// Increment to setup for the next character.
	_cursorX += delta;
}

//strPixelLen			- gets pixel length of given ASCII string
int16_t RA8876_t3::strPixelLen(const char * str)
{
//	Serial.printf("strPixelLen %s\n", str);
	if (!str) return(0);
	if (gfxFont) 
	{
		// BUGBUG:: just use the other function for now... May do this for all of them...
	  int16_t x, y;
	  uint16_t w, h;
	  getTextBounds(str, _cursorX, _cursorY, &x, &y, &w, &h);
	  return w;
	}

	uint16_t len=0, maxlen=0;
	while (*str)
	{
		if (*str=='\n')
		{
			if ( len > maxlen )
			{
				maxlen=len;
				len=0;
			}
		}
		else
		{
			if (!font)
			{
				len+=textsize_x*6;
			}
			else
			{

				uint32_t bitoffset;
				const uint8_t *data;
				uint16_t c = *str;

//				Serial.printf("char %c(%d)\n", c,c);

				if (c >= font->index1_first && c <= font->index1_last) {
					bitoffset = c - font->index1_first;
					bitoffset *= font->bits_index;
				} else if (c >= font->index2_first && c <= font->index2_last) {
					bitoffset = c - font->index2_first + font->index1_last - font->index1_first + 1;
					bitoffset *= font->bits_index;
				} else if (font->unicode) {
					continue;
				} else {
					continue;
				}
				//Serial.printf("  index =  %d\n", fetchbits_unsigned(font->index, bitoffset, font->bits_index));
				data = font->data + fetchbits_unsigned(font->index, bitoffset, font->bits_index);

				uint32_t encoding = fetchbits_unsigned(data, 0, 3);
				if (encoding != 0) continue;
//				uint32_t width = fetchbits_unsigned(data, 3, font->bits_width);
//				Serial.printf("  width =  %d\n", width);
				bitoffset = font->bits_width + 3;
				bitoffset += font->bits_height;

//				int32_t xoffset = fetchbits_signed(data, bitoffset, font->bits_xoffset);
//				Serial.printf("  xoffset =  %d\n", xoffset);
				bitoffset += font->bits_xoffset;
				bitoffset += font->bits_yoffset;

				uint32_t delta = fetchbits_unsigned(data, bitoffset, font->bits_delta);
				bitoffset += font->bits_delta;
//				Serial.printf("  delta =  %d\n", delta);

				len += delta;//+width-xoffset;
//				Serial.printf("  len =  %d\n", len);
				if ( len > maxlen )
				{
					maxlen=len;
//					Serial.printf("  maxlen =  %d\n", maxlen);
				}
			
			}
		}
		str++;
	}
//	Serial.printf("Return  maxlen =  %d\n", maxlen);
	return( maxlen );
}

void RA8876_t3::charBounds(char c, int16_t *x, int16_t *y,
  int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy) {


  	//uint8_t offset, _x, _y;
	//int charW = 0;

	// BUGBUG:: Not handling offset/clip
    if (font) {
        if(c == '\n') { // Newline?
            *x  = 0;    // Reset x to zero, advance y by one line
            *y += font->line_space;
        } else if(c != '\r') { // Not a carriage return; is normal char
			uint32_t bitoffset;
			const uint8_t *data;
			if (c >= font->index1_first && c <= font->index1_last) {
				bitoffset = c - font->index1_first;
				bitoffset *= font->bits_index;
			} else if (c >= font->index2_first && c <= font->index2_last) {
				bitoffset = c - font->index2_first + font->index1_last - font->index1_first + 1;
				bitoffset *= font->bits_index;
			} else if (font->unicode) {
				return; // TODO: implement sparse unicode
			} else {
				return;
			}
			//Serial.printf("  index =  %d\n", fetchbits_unsigned(font->index, bitoffset, font->bits_index));
			data = font->data + fetchbits_unsigned(font->index, bitoffset, font->bits_index);

			uint32_t encoding = fetchbits_unsigned(data, 0, 3);
			if (encoding != 0) return;
			uint32_t width = fetchbits_unsigned(data, 3, font->bits_width);
			bitoffset = font->bits_width + 3;
			uint32_t height = fetchbits_unsigned(data, bitoffset, font->bits_height);
			bitoffset += font->bits_height;
			//Serial.printf("  size =   %d,%d\n", width, height);
			//Serial.printf("  line space = %d\n", font->line_space);

			int32_t xoffset = fetchbits_signed(data, bitoffset, font->bits_xoffset);
			bitoffset += font->bits_xoffset;
			int32_t yoffset = fetchbits_signed(data, bitoffset, font->bits_yoffset);
			bitoffset += font->bits_yoffset;

			uint32_t delta = fetchbits_unsigned(data, bitoffset, font->bits_delta);
			bitoffset += font->bits_delta;

            int16_t
                    x1 = *x + xoffset,
                    y1 = *y + yoffset,
                    x2 = x1 + width,
                    y2 = y1 + height;

		    if(wrap && (x2 > _width)) {
	            *x  = 0; // Reset x to zero, advance y by one line
	            *y += font->line_space;
	            x1 = *x + xoffset,
	            y1 = *y + font->cap_height - height - yoffset,
	            x2 = x1 + width,
	            y2 = y1 + height;
        	}
            if(x1 < *minx) *minx = x1;
            if(y1 < *miny) *miny = y1;
            if(x2 > *maxx) *maxx = x2;
            if(y2 > *maxy) *maxy = y2;
            *x += delta;	// ? guessing here...
        }
    } 

    else if(gfxFont) {

        if(c == '\n') { // Newline?
            *x  = 0;    // Reset x to zero, advance y by one line
            *y += textsize_y * gfxFont->yAdvance;
        } else if(c != '\r') { // Not a carriage return; is normal char
            uint8_t first = gfxFont->first,
                    last  = gfxFont->last;
            if((c >= first) && (c <= last)) { // Char present in this font?
    			GFXglyph *glyph  = gfxFont->glyph + (c - first);
                uint8_t gw = glyph->width,
                        gh = glyph->height,
                        xa = glyph->xAdvance;
                int8_t  xo = glyph->xOffset,
                        yo = glyph->yOffset + gfxFont->yAdvance/2;
                if(wrap && ((*x+(((int16_t)xo+gw)*textsize_x)) > _width)) {
                    *x  = 0; // Reset x to zero, advance y by one line
                    *y += textsize_y * gfxFont->yAdvance;
                }
                int16_t tsx = (int16_t)textsize_x,
                        tsy = (int16_t)textsize_y,
                        x1 = *x + xo * tsx,
                        y1 = *y + yo * tsy,
                        x2 = x1 + gw * tsx - 1,
                        y2 = y1 + gh * tsy - 1;
                if(x1 < *minx) *minx = x1;
                if(y1 < *miny) *miny = y1;
                if(x2 > *maxx) *maxx = x2;
                if(y2 > *maxy) *maxy = y2;
                *x += xa * tsx;
            }
        }
    }
}

// Add in Adafruit versions of text bounds calculations. 
void RA8876_t3::getTextBounds(const uint8_t *buffer, uint16_t len, int16_t x, int16_t y,
      int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    *x1 = x;
    *y1 = y;
    *w  = *h = 0;

    int16_t minx = _width, miny = _height, maxx = -1, maxy = -1;

    while(len--)
        charBounds(*buffer++, &x, &y, &minx, &miny, &maxx, &maxy);

    if(maxx >= minx) {
        *x1 = minx;
        *w  = maxx - minx + 1;
    }
    if(maxy >= miny) {
        *y1 = miny;
        *h  = maxy - miny + 1;
    }

}

void RA8876_t3::getTextBounds(const char *str, int16_t x, int16_t y,
        int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    uint8_t c; // Current character

    *x1 = x;
    *y1 = y;
    *w  = *h = 0;

    int16_t minx = _width, miny = _height, maxx = -1, maxy = -1;

    while((c = *str++))
        charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);

    if(maxx >= minx) {
        *x1 = minx;
        *w  = maxx - minx + 1;
    }
    if(maxy >= miny) {
        *y1 = miny;
        *h  = maxy - miny + 1;
    }
}

void RA8876_t3::getTextBounds(const String &str, int16_t x, int16_t y,
        int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    if (str.length() != 0) {
        getTextBounds(const_cast<char*>(str.c_str()), x, y, x1, y1, w, h);
    }
}


void RA8876_t3::drawFontBits(bool opaque, uint32_t bits, uint32_t numbits, int32_t x, int32_t y, uint32_t repeat)
{
	//Serial.printf("    drawFontBits: %d %x %x (%d %d) %u\n", opaque, bits, numbits, x, y, repeat);
	if (bits == 0) {
		if (opaque) {
			fillRect(x, y, numbits, repeat, _TXTBackColor);
		}
	} else {
		int32_t x1 = x;
		uint32_t n = numbits;
		int w;
		int bgw;

		w = 0;
		bgw = 0;

		do {
			n--;
			if (bits & (1 << n)) {
				if (bgw>0) {
					if (opaque) {
						fillRect(x1 - bgw, y, bgw, repeat, _TXTBackColor);
						//Serial.printf("        BG fillrect: %d %d %d %d %x\n", x1 - bgw, y, bgw, repeat, _TXTBackColor);
					}
					bgw=0;
				}
				w++;
			} else {
				if (w>0) {
					fillRect(x1 - w, y, w, repeat, _TXTForeColor);
					//Serial.printf("        FG fillrect: %d %d %d %d %x\n", x1 - w, y, w, repeat, _TXTForeColor);
					w = 0;
				}
				bgw++;
			}
			x1++;
		} while (n > 0);

		if (w > 0) {
			fillRect(x1 - w, y, w, repeat, _TXTForeColor);
			//Serial.printf("        FG fillrect: %d %d %d %d %x\n", x1 - w, y, w, repeat, _TXTForeColor);
		}

		if (bgw > 0) {
			if (opaque) {
				fillRect(x1 - bgw, y, bgw, repeat, _TXTBackColor);
				//Serial.printf("        BG fillrect: %d %d %d %d %x\n", x1 - bgw, y, bgw, repeat, _TXTBackColor);
			}
		}
	}
}

void RA8876_t3::drawGFXFontChar(unsigned int c) {
	// Lets do Adafruit GFX character output here as well
    if(c == '\r') 	 return;
    // Some quick and dirty tests to see if we can
    uint8_t first = gfxFont->first;
    if((c < first) || (c > gfxFont->last)) return; 

    GFXglyph *glyph  = gfxFont->glyph + (c - first);
    uint8_t   w     = glyph->width,
              h     = glyph->height;
	//Serial.printf("w = %d, h = %d\n", w, h);
    //if((w == 0) || (h == 0))  return;  // Is there an associated bitmap?

    int16_t xo = glyph->xOffset; // sic
    int16_t yo = glyph->yOffset + gfxFont->yAdvance/2;

    if(wrap && ((_cursorX + textsize_x * (xo + w)) > _width)) {
        _cursorX  = 0;
        _cursorY += (int16_t)textsize_y * gfxFont->yAdvance;
    }

    // Lets do the work to output the font character
    uint8_t  *bitmap = gfxFont->bitmap;

    uint16_t bo = glyph->bitmapOffset;
    uint8_t  xx, yy, bits = 0, bit = 0;
    //Serial.printf("DGFX_char: %c (%d,%d) : %u %u %u %u %d %d %x %x %d %d\n", c, _cursorX, _cursorY, w, h,  
    //			glyph->xAdvance, gfxFont->yAdvance, xo, yo, _TXTForeColor, _TXTBackColor, textsize_x, textsize_y);  Serial.flush();

    if (_backTransparent) {

	     //Serial.printf("DGFXChar: %c %u, %u, wh:%d %d o:%d %d\n", c, _cursorX, _cursorY, w, h, xo, yo);
	    // Todo: Add character clipping here

    	// NOTE: Adafruit GFX does not support Opaque font output as there
    	// are issues with proportionally spaced characters that may overlap
    	// So the below is not perfect as we may overwrite a small portion 
    	// of a letter with the next one, when we blank out... 
    	// But: I prefer to let each of us decide if the limitations are
    	// worth it or not.  If Not you still have the option to not
    	// Do transparent mode and instead blank out and blink...

	    for(yy=0; yy<h; yy++) {
			uint8_t w_left = w;
	    	xx = 0;
	        while (w_left) {
	            if(!(bit & 7)) {
	                bits = bitmap[bo++];
	            }
				// Could try up to 8 bits at time, but start off trying up to 4
	            uint8_t xCount;
	            if ((w_left >= 8) && ((bits & 0xff) == 0xff)) {
	            	xCount = 8;
	            	//Serial.print("8");
	                fillRect(_cursorX+(xo+xx)*textsize_x, _cursorY+(yo+yy)*textsize_y,
	                      xCount * textsize_x, textsize_y, _TXTForeColor);
	            } else if ((w_left >= 4) && ((bits & 0xf0) == 0xf0)) {
	            	xCount = 4;
	            	//Serial.print("4");
	                fillRect(_cursorX+(xo+xx)*textsize_x, _cursorY+(yo+yy)*textsize_y,
	                      xCount * textsize_x, textsize_y, _TXTForeColor);
	            } else if ((w_left >= 3) && ((bits & 0xe0) == 0xe0)) {
	            	//Serial.print("3");
	            	xCount = 3;
	                fillRect(_cursorX+(xo+xx)*textsize_x, _cursorY+(yo+yy)*textsize_y,
                      xCount * textsize_x, textsize_y, _TXTForeColor);
	            } else if ((w_left >= 2) && ((bits & 0xc0) == 0xc0)) {
	            	//Serial.print("2");
	            	xCount = 2;
	                fillRect(_cursorX+(xo+xx)*textsize_x, _cursorY+(yo+yy)*textsize_y,
	                      xCount * textsize_x, textsize_y, _TXTForeColor);
	            } else {
	            	xCount = 1;
	            	if(bits & 0x80) {
		                if((textsize_x == 1) && (textsize_y == 1)){
		                    drawPixel(_cursorX+xo+xx, _cursorY+yo+yy, _TXTForeColor);
		                } else {
							fillRect(_cursorX+(xo+xx)*textsize_x, _cursorY+(yo+yy)*textsize_y,textsize_x, textsize_y, _TXTForeColor);
		                }
		            }
	            }
	            xx += xCount;
	            w_left -= xCount;
	            bit += xCount;
	            bits <<= xCount;
			}
	    }
    	_gfx_last_char_x_write = 0;
	} else {
		// To Do, properly clipping and offsetting...
		// This solid background approach is about 5 time faster
		// Lets calculate bounding rectangle that we will update
		// We need to offset by the origin.

		// We are going direct so do some offsets and clipping
		int16_t x_offset_cursor = _cursorX + _originx;	// This is where the offseted cursor is.
		int16_t x_start = x_offset_cursor;  // I am assuming no negative x offsets.
		int16_t x_end = x_offset_cursor + (glyph->xAdvance * textsize_x);
		if (glyph->xAdvance < (xo + w)) x_end = x_offset_cursor + ((xo + w)* textsize_x);  // BUGBUG Overlflows into next char position.
		int16_t x_left_fill = x_offset_cursor + xo * textsize_x;
		int16_t x;
		if (xo < 0) { 
			// Unusual character that goes back into previous character
			//Serial.printf("GFX Font char XO < 0: %c %d %d %u %u %u\n", c, xo, yo, w, h, glyph->xAdvance );
			x_start += xo * textsize_x;
			x_left_fill = 0;	// Don't need to fill anything here... 
		}

		int16_t y_start = _cursorY + _originy + (_gfxFont_min_yOffset * textsize_y)+ gfxFont->yAdvance*textsize_y/2;  // UP to most negative value.
		int16_t y_end = y_start +  gfxFont->yAdvance * textsize_y;  // how far we will update
		int16_t y = y_start;
		//int8_t y_top_fill = (yo - _gfxFont_min_yOffset) * textsize_y;	 // both negative like -10 - -16 = 6...
		int8_t y_top_fill = (yo - gfxFont->yAdvance/2 - _gfxFont_min_yOffset) * textsize_y;


		// See if anything is within clip rectangle, if not bail
		if((x_start >= _displayclipx2)   || // Clip right
			 (y_start >= _displayclipy2) || // Clip bottom
			 (x_end < _displayclipx1)    || // Clip left
			 (y_end < _displayclipy1))  	// Clip top 
		{
			// But remember to first update the cursor position
			_cursorX += glyph->xAdvance * (int16_t)textsize_x;
			Serial.printf("CLIPPED RETURN XY(%d %d %d %d) CLIP(%d %d %d %d)\n", 
				x_start, x_end, y_start, y_end, 
				_displayclipx1, _displayclipx2, _displayclipy1, _displayclipy2);
			return;
		}

		// If our y_end > _displayclipy2 set it to _displayclipy2 as to not have to test both  Likewise X
		if (y_end > _displayclipy2) y_end = _displayclipy2;
		if (x_end > _displayclipx2) x_end = _displayclipx2;
		// If we get here and 
		if (_gfx_last__cursorY != (_cursorY + _originy))  _gfx_last_char_x_write = 0;

		// lets try to output text in one output rectangle
		//Serial.printf("    SPI (%d %d) (%d %d)\n", x_start, y_start, x_end, y_end);Serial.flush();
		// compute the actual region we will output given 
		//_startSend();
	
		//setActiveWindow((x_start >= _displayclipx1) ? x_start : _displayclipx1, 
		//		(y_start >= _displayclipy1) ? y_start : _displayclipy1, 
		//		x_end  - 1,  y_end - 1); 
		//writeCommand(RA8875_MRWC);
		//Serial.printf("SetAddr: %u %u %u %u\n", (x_start >= _displayclipx1) ? x_start : _displayclipx1, 
		//		(y_start >= _displayclipy1) ? y_start : _displayclipy1, 
		//		x_end  - 1,  y_end - 1); 
		// First lets fill in the top parts above the actual rectangle...
		//Serial.printf("    y_top_fill %d x_left_fill %d\n", y_top_fill, x_left_fill);
		while (y_top_fill--) {
			if ( (y >= _displayclipy1) && (y < _displayclipy2)) {
				for (int16_t xx = x_start; xx < x_end; xx++) {
					if (xx >= _displayclipx1) {
						combineAndDrawPixel(xx, y, gfxFontLastCharPosFG(xx,y)? _gfx_last_char_textcolor : (xx < x_offset_cursor)? _gfx_last_char_textbgcolor : _TXTBackColor);
					}
				}					
			}
			forceCombinedPixelsOut();
			y++;
		}
		//Serial.println("    After top fill"); Serial.flush();
		// Now lets output all of the pixels for each of the rows.. 
		for(yy=0; (yy<h) && (y < _displayclipy2); yy++) {
			uint16_t bo_save = bo;
			uint8_t bit_save = bit;
			uint8_t bits_save = bits;
			for (uint8_t yts = 0; (yts < textsize_y) && (y < _displayclipy2); yts++) {
				// need to repeat the stuff for each row...
				bo = bo_save;
				bit = bit_save;
				bits = bits_save;
				x = x_start;
				if (y >= _displayclipy1) {
					while (x < x_left_fill) {
						if ( (x >= _displayclipx1) && (x < _displayclipx2)) {
							// Don't need to check if we are in previous char as in this case x_left_fill is set to 0...
							combineAndDrawPixel(x, y, gfxFontLastCharPosFG(x,y)? _gfx_last_char_textcolor :  _TXTBackColor);
						}
						x++;
					}
			        for(xx=0; xx<w; xx++) {
			            if(!(bit++ & 7)) {
			                bits = bitmap[bo++];
			            }
			            for (uint8_t xts = 0; xts < textsize_x; xts++) {
							if ( (x >= _displayclipx1) && (x < _displayclipx2)) {
								if (bits & 0x80) 
									combineAndDrawPixel(x, y, _TXTForeColor); 
								else  
									combineAndDrawPixel(x, y,gfxFontLastCharPosFG(x,y)? _gfx_last_char_textcolor : (x < x_offset_cursor)? _gfx_last_char_textbgcolor : _TXTBackColor);		
							}
			            	x++;	// remember our logical position...
			            }
			            bits <<= 1;
			        }
			        // Fill in any additional bg colors to right of our output
			        while (x < x_end) {
						if (x >= _displayclipx1) {
			        		combineAndDrawPixel(x, y, gfxFontLastCharPosFG(x,y)? _gfx_last_char_textcolor : (x < x_offset_cursor)? _gfx_last_char_textbgcolor : _TXTBackColor);
			        	}
			        	x++;
			        }
		    	}
				forceCombinedPixelsOut();
	        	y++;	// remember which row we just output
		    }
	    }
	    // And output any more rows below us...
	  	//Serial.println("    Bottom fill"); Serial.flush();
		while (y < y_end) {
			if (y >= _displayclipy1) {
				for (int16_t xx = x_start; xx < x_end; xx++) {
					if (xx >= _displayclipx1)  {
						combineAndDrawPixel(xx ,y, gfxFontLastCharPosFG(xx,y)? _gfx_last_char_textcolor : (xx < x_offset_cursor)? _gfx_last_char_textbgcolor : _TXTBackColor);
					}
				}
			}
			forceCombinedPixelsOut();
			y++;
		}
		//writecommand_last(ILI9488_NOP);
		//_endSend();
		//setActiveWindow();

		_gfx_c_last = c; 
		_gfx_last__cursorX = _cursorX + _originx;  
		_gfx_last__cursorY = _cursorY + _originy; 
		_gfx_last_char_x_write = x_end; 
		_gfx_last_char_textcolor = _TXTForeColor; 
		_gfx_last_char_textbgcolor = _TXTBackColor;
		
	}

    _cursorX += glyph->xAdvance * (int16_t)textsize_x;
}


// Some fonts overlap characters if we detect that the previous 
// character wrote out more width than they advanced in X direction
// we may want to know if the last character output a FG or BG at a position. 
	// Opaque font chracter overlap?
//	unsigned int _gfx_c_last;
//	int16_t   _gfx_last__cursorX, _gfx_last__cursorY;
//	int16_t	 _gfx_last_x_overlap = 0;
	
bool RA8876_t3::gfxFontLastCharPosFG(int16_t x, int16_t y) {
    GFXglyph *glyph  = gfxFont->glyph + (_gfx_c_last -  gfxFont->first);

    uint8_t   w     = glyph->width,
              h     = glyph->height;


    int16_t xo = glyph->xOffset; // sic
    int16_t yo = glyph->yOffset + gfxFont->yAdvance/2;
    if (x >= _gfx_last_char_x_write) return false; 	// we did not update here...
    if (y < (_gfx_last__cursorY + (yo*textsize_y)))  return false;  // above
    if (y >= (_gfx_last__cursorY + (yo+h)*textsize_y)) return false; // below


    // Lets compute which Row this y is in the bitmap
    int16_t y_bitmap = (y - ((_gfx_last__cursorY + (yo*textsize_y))) + textsize_y - 1) / textsize_y;
    int16_t x_bitmap = (x - ((_gfx_last__cursorX + (xo*textsize_x))) + textsize_x - 1) / textsize_x;
    uint16_t  pixel_bit_offset = y_bitmap * w + x_bitmap;

    return ((gfxFont->bitmap[glyph->bitmapOffset + (pixel_bit_offset >> 3)]) & (0x80 >> (pixel_bit_offset & 0x7)));
}


void RA8876_t3::setTextSize(uint8_t s_x, uint8_t s_y) {
    textsize_x = (s_x > 0) ? s_x : 1;
    textsize_y = (s_y > 0) ? s_y : 1;
	_gfx_last_char_x_write = 0;	// Don't use cached data here
}

void RA8876_t3::drawFontPixel( uint8_t alpha, uint32_t x, uint32_t y ){
	// Adjust alpha based on the number of alpha levels supported by the font (based on bpp)
	// Note: Implemented look-up table for alpha, but made absolutely no difference in speed (T3.6)
	alpha = (uint8_t)(alpha * fontalphamx);
	uint32_t result = ((((textcolorPrexpanded - textbgcolorPrexpanded) * alpha) >> 5) + textbgcolorPrexpanded) & 0b00000111111000001111100000011111;
	Pixel(x,y,(uint16_t)((result >> 16) | result));
}


void RA8876_t3::Pixel(int16_t x, int16_t y, uint16_t color)
 {
	x+=_originx;
	y+=_originy;

	if((x < _displayclipx1) ||(x >= _displayclipx2) || (y < _displayclipy1) || (y >= _displayclipy2)) return;

	setActiveWindow(x, y, x, y);
	//writeCommand(RA8875_MRWC);
	drawPixel(x, y, color);
}


uint32_t RA8876_t3::fetchbit(const uint8_t *p, uint32_t index)
{
	if (p[index >> 3] & (1 << (7 - (index & 7)))) return 1;
	return 0;
}

uint32_t RA8876_t3::fetchbits_unsigned(const uint8_t *p, uint32_t index, uint32_t required)
{
	uint32_t val = 0;
	do {
		uint8_t b = p[index >> 3];
		uint32_t avail = 8 - (index & 7);
		if (avail <= required) {
			val <<= avail;
			val |= b & ((1 << avail) - 1);
			index += avail;
			required -= avail;
		} else {
			b >>= avail - required;
			val <<= required;
			val |= b & ((1 << required) - 1);
			break;
		}
	} while (required);
	return val;
}

uint32_t RA8876_t3::fetchbits_signed(const uint8_t *p, uint32_t index, uint32_t required)
{
	uint32_t val = fetchbits_unsigned(p, index, required);
	if (val & (1 << (required - 1))) {
		return (int32_t)val - (1 << required);
	}
	return (int32_t)val;
}

uint32_t RA8876_t3::fetchpixel(const uint8_t *p, uint32_t index, uint32_t x)
{
	// The byte
	uint8_t b = p[index >> 3];
	// Shift to LSB position and mask to get value
	uint8_t s = ((fontppb-(x % fontppb)-1)*fontbpp);
	// Mask and return
	return (b >> s) & fontbppmask;
}


/**************************************************************************/
/*!
      for compatibility with popular Adafruit_GFX
	  draws a single vertical line
	  Parameters:
	  x: horizontal start
	  y: vertical start
	  h: height
	  color: RGB565 color
*/
/**************************************************************************/
void RA8876_t3::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	if (h < 1) h = 1;
	h < 2 ? drawPixel(x,y,color) : drawLine(x, y, x, (y+h)-1, color);
}

/**************************************************************************/
/*!
      for compatibility with popular Adafruit_GFX
	  draws a single orizontal line
	  Parameters:
	  x: horizontal start
	  y: vertical start
	  w: width
	  color: RGB565 color
*/
/**************************************************************************/
void RA8876_t3::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	if (w < 1) w = 1;
	w < 2 ? drawPixel(x,y,color) : drawLine(x, y, (w+x)-1, y, color);
}

/**************************************************************************/
/*!		
		Set the Active Window
	    Parameters:
		XL: Horizontal Left
		XR: Horizontal Right
		YT: Vertical TOP
		YB: Vertical Bottom
*/
/**************************************************************************/
void RA8876_t3::setActiveWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB)
{
	//if (_portrait){ swapvals(XL,YT); swapvals(XR,YB);}

//	if (XR >= SCREEN_WIDTH) XR = SCREEN_WIDTH;
//	if (YB >= SCREEN_HEIGHT) YB = SCREEN_HEIGHT;
	
	_activeWindowXL = XL; _activeWindowXR = XR;
	_activeWindowYT = YT; _activeWindowYB = YB;
	_updateActiveWindow(false);
}

/**************************************************************************/
/*!		
		Set the Active Window as FULL SCREEN
*/
/**************************************************************************/
void RA8876_t3::setActiveWindow(void)
{
	_activeWindowXL = 0; _activeWindowXR = _width;
	_activeWindowYT = 0; _activeWindowYB = _height;
	//if (_portrait){swapvals(_activeWindowXL,_activeWindowYT); swapvals(_activeWindowXR,_activeWindowYB);}
	_updateActiveWindow(true);
}

/**************************************************************************/
/*!
		this update the RA8875 Active Window registers
		[private]
*/
/**************************************************************************/
void RA8876_t3::_updateActiveWindow(bool full)
{
	if (full){
		// X
		activeWindowXY(0, 0);
		activeWindowWH(_width, _height);;
	} else {
		activeWindowXY(_activeWindowXL, _activeWindowYT);
		activeWindowWH(_activeWindowXR-_activeWindowXL, _activeWindowYB-_activeWindowYT);		
	}
}

/**************************************************************************/
/*!   
		Set the x,y position for text only
		Parameters:
		x: horizontal pos in pixels
		y: vertical pos in pixels
		update: true track the actual text position internally
		note: not active with rendered fonts, just set x,y internal tracked param
		[private]
*/
/**************************************************************************/
void RA8876_t3::_textPosition(int16_t x, int16_t y, bool update)
{
	lcdRegDataWrite(RA8876_F_CURX0,x, false); //63h
	lcdRegDataWrite(RA8876_F_CURX1,x>>8, false);//64h
	lcdRegDataWrite(RA8876_F_CURY0,y, false);//65h
	lcdRegDataWrite(RA8876_F_CURY1,y>>8, true);//66h
	if (update){ _cursorX = x; _cursorY = y;}
}

void RA8876_t3::_setFNTdimensions(uint8_t index) 
{
	_FNTwidth 		= 	fontDimPar[index][0];
	_FNTheight 		= 	fontDimPar[index][1];
	_FNTbaselineLow  = 	fontDimPar[index][2];
	_FNTbaselineTop  = 	fontDimPar[index][3];
}


//**************************************************************//
/* Print a character to current active screen                   */
/* This function processes most ASCII control codes and will    */
/* scroll the screen up when text position is past bottom line  */
//**************************************************************//
// overwrite functions from class Print:
size_t RA8876_t3::write(uint8_t c) {
	return write(&c, 1);
}

size_t RA8876_t3::write(const uint8_t *buffer, size_t size) {
  if(_use_default){
	size_t cb = size;
	// Lets try to handle some of the special font centering code that was done for default fonts.
	if (_absoluteCenter || _relativeCenter ) {
		int16_t x, y;
	  	uint16_t strngWidth, strngHeight;
	  	getTextBounds(buffer, cb, 0, 0, &x, &y, &strngWidth, &strngHeight);
	  	Serial.printf("_fontwrite bounds: %d %d %u %u\n", x, y, strngWidth, strngHeight);
	  	// Note we may want to play with the x ane y returned if they offset some
		if (_absoluteCenter && strngWidth > 0){//Avoid operations for strngWidth = 0
			_absoluteCenter = false;
			_cursorX = _cursorX - (x + strngWidth / 2);
			_cursorY = _cursorY - (y + strngHeight / 2);
		} else if (_relativeCenter && strngWidth > 0){//Avoid operations for strngWidth = 0
			_relativeCenter = false;
			if (bitRead(_TXTparameters,5)) {//X = center
				_cursorX = (_width / 2) - (x + strngWidth / 2);
				_TXTparameters &= ~(1 << 5);//reset
			}
			if (bitRead(_TXTparameters,6)) {//Y = center
				_cursorY = (_height / 2) - (y + strngHeight / 2) ;
				_TXTparameters &= ~(1 << 6);//reset
			}
		}
	}
	
	while (cb) {
		uint8_t text = *buffer++;
		cb--;
		vdata = text;
		if (text == 13){//'\r'
			//Ignore carriage-return
		} else if(text == '\n') {
			_cursorY += (_FNTheight * _scaleY);
			prompt_line = _cursorY + _scrollYT;
			_cursorX = _scrollXL;
			update_xy();
		} else if(text == 127) { // Destructive Backspace
				//if((_cursorX > (prompt_size +_scrollXL)) || (_cursorY > (prompt_line + _scrollYT)))
				{
					_cursorX-=(_FNTwidth * _scaleX);
					update_xy();
					vdata = 0x20;
					update_tft(vdata);
					update_xy();
				}
				if((_cursorY > (prompt_line + _scrollYT)) && ((_cursorX + _scrollXL) < 0)) {
					_cursorY -= (_FNTheight * _scaleY);
					_cursorX = _width-(_FNTwidth * _scaleX);
					if(_FNTwidth == 12)
						_cursorX -= 4; // 1024 / 12 = 85.3 so adjust
					update_xy();

				}
		} else if(text == 0x09) { // TAB Character
				_cursorX += (tab_size*(_FNTwidth * _scaleX));
				update_xy();
		} else if(text == 0x07) { // BELL Character
				tone(35,1000,500); // Need Pin variable for tone output (now pin #35)
		} else if(text == 0x0c) { // form feed
			drawSquareFill(_scrollXL, _scrollYT, _scrollXR, _scrollYB, _TXTBackColor);
			textColor(_TXTForeColor,_TXTBackColor);
			setTextCursor(_scrollXL,_scrollYT);
		} else {
			textColor(_TXTForeColor,_TXTBackColor);
			update_tft(vdata);
			_cursorX += (_FNTwidth * _scaleX);
			switch(_FNTwidth) {
				case 12: // Font width is 12, 1024 / 12 =  85.3, have to
						 // subtract 12 to keep within screen width.
					if(_cursorX >= _scrollXR-(_FNTwidth * _scaleX)) {
					_cursorY += (_FNTheight * _scaleY);
					_cursorX = _scrollXL;
				}
				break;
				default:
					if(_cursorX >= _scrollXR) {
					_cursorY += (_FNTheight * _scaleY);
					_cursorX = _scrollXL;
				}
			}
			update_xy();
		}
	}
	return 1;
  } else {
	  _fontWrite(buffer, size);
	  return 1;
  }
}

//************************************************//
/* Print a character to current active screen.    */
/* This function does not process ASCII control   */
/* codes and will scroll the screen up when text  */
/* position is past bottom line.                  */
//************************************************//
size_t RA8876_t3::rawPrint(uint8_t text) {
		update_tft(text);
		_cursorX += (_FNTwidth * _scaleX);
		switch(_FNTwidth) {
			case 12: // Font width is 12, 1024 / 12 =  85.3, have to
					 // subtract 12 to keep within screen width.
				if(_cursorX >= _scrollXR-(_FNTwidth * _scaleX)) {
				_cursorY += (_FNTheight * _scaleY);
				_cursorX = _scrollXL;
			}
			break;
			default:
				if(_cursorX >= _scrollXR) {
				_cursorY += (_FNTheight * _scaleY);
				_cursorX = _scrollXL;
			}
		}
		update_xy();
	return 1;
}


//**************************************************************//
/* JB ADD FOR ROTATING TEXT                                     */
/* Turn RA8876 text rotate mode ON/OFF (True = ON)              */
//**************************************************************//
void RA8876_t3::setRotation(uint8_t rotation) //rotate text and graphics
{
	_rotation = rotation & 0x3;
	uint8_t macr_settings;

	switch (_rotation) {
		case 0:
			_width = 	SCREEN_WIDTH;
			_height = 	SCREEN_HEIGHT;
			_portrait = false;
			VSCAN_T_to_B();
			macr_settings = RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_LRTB<<1;
			break;
		case 1:
			_portrait = true;
			_width = 	SCREEN_HEIGHT;
			_height = 	SCREEN_WIDTH;
			VSCAN_B_to_T();
			macr_settings = RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_RLTB<<1;
			break;
		case 2: 
			_width = 	SCREEN_WIDTH;
			_height = 	SCREEN_HEIGHT;
			_portrait = false;
			VSCAN_B_to_T();
			macr_settings = RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_RLTB<<1;
			break;
		case 3: 
			_portrait = true;
			_width = 	SCREEN_HEIGHT;
			_height = 	SCREEN_WIDTH;
			VSCAN_T_to_B();
			macr_settings = RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_LRTB<<1;
			//VSCAN_T_to_B();
			//macr_settings = RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_BTLR<<1;
			break;
	}
	lcdRegWrite(RA8876_MACR);//02h
	lcdDataWrite(macr_settings);

	_scrollXL = 0;
	_scrollXR = _width;
	_scrollYT = 0;
	_scrollYB = _height;
	
	_updateActiveWindow(false);

 	setClipRect();
	setOrigin();
	Serial.println("Rotate: After Origins"); Serial.flush();

}

/**************************************************************************/
/*!
      Get rotation setting
*/
/**************************************************************************/
uint8_t RA8876_t3::getRotation()
{
	return _rotation;

}

//**************************************************************//
/* JB ADD FOR ROTATING TEXT                                     */
/* Turn RA8876 text rotate mode ON/OFF (True = ON)              */
//**************************************************************//
void RA8876_t3::textRotate(boolean on)
{
    if(on)
    {
        lcdRegDataWrite(RA8876_CCR1, RA8876_TEXT_ROTATION<<4);//cdh
    }
    else
    {
        lcdRegDataWrite(RA8876_CCR1, RA8876_TEXT_NO_ROTATION<<4);//cdh
    }
}



void RA8876_t3::MemWrite_Left_Right_Top_Down(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
00b: Left .. Right then Top ..Bottom.
Ignored if canvas in linear addressing mode.		*/
	unsigned char temp;
	
	temp = lcdRegDataRead(RA8876_MACR);
	Serial.println(temp, BIN);
	temp &= cClrb2;
	temp &= cClrb1;
	Serial.println(temp, BIN);
	lcdRegDataWrite(RA8876_MACR, temp);
	
	temp = lcdRegDataRead(RA8876_MACR);
	Serial.println(temp, BIN);	
}

void RA8876_t3::MemWrite_Right_Left_Top_Down(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
01b: Right .. Left then Top .. Bottom.
Ignored if canvas in linear addressing mode.		*/
	unsigned char temp;

	temp = lcdRegDataRead(RA8876_MACR);
	Serial.println(temp, BIN);
	
	temp &= cClrb2;
	temp |= cSetb1;
	Serial.println(temp, BIN);
	lcdRegDataWrite(RA8876_MACR, temp);
	
	temp = lcdRegDataRead(RA8876_MACR);
	Serial.println(temp, BIN);
	
	
}

void RA8876_t3::MemWrite_Top_Down_Left_Right(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
10b: Top .. Bottom then Left .. Right.
Ignored if canvas in linear addressing mode.		*/
	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MACR);
	Serial.println(temp, BIN);
	//lcdRegWrite(RA8876_MACR);//02h
	//lcdDataWrite(RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_LRTB<<1);

	//temp = RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_BTLR<<4|RA8876_WRITE_MEMORY_BTLR<<1;
	temp = RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_TBLR<<1;
	Serial.println(temp, BIN);
	lcdRegDataWrite(RA8876_MACR, temp);
	
	temp = lcdRegDataRead(RA8876_MACR);
	Serial.println(temp, BIN);
/*
	unsigned char temp;
	lcdDataWrite(0x02);
	temp = lcdDataRead();
	temp |= cSetb2;
    temp &= cClrb1;
	lcdDataWrite(temp, true);
	*/

}

void RA8876_t3::MemWrite_Down_Top_Left_Right(void)
{
/* Host Write Memory Direction (Only for Graphic Mode)
11b: Bottom .. Top then Left .. Right.
Ignored if canvas in linear addressing mode.		*/

	unsigned char temp;
	temp = lcdRegDataRead(RA8876_MACR);
	Serial.println(temp, BIN);
	//lcdRegWrite(RA8876_MACR);//02h
	//lcdDataWrite(RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_LRTB<<1);

	//temp = RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_BTLR<<4|RA8876_WRITE_MEMORY_BTLR<<1;
	temp = RA8876_DIRECT_WRITE<<6|RA8876_READ_MEMORY_LRTB<<4|RA8876_WRITE_MEMORY_BTLR<<1;
	Serial.println(temp, BIN);
	lcdRegDataWrite(RA8876_MACR, temp);
	
	temp = lcdRegDataRead(RA8876_MACR);
	Serial.println(temp, BIN);

}

void RA8876_t3::VSCAN_T_to_B(void)
{
/*	
Vertical Scan direction
0 : From Top to Bottom
1 : From bottom to Top
PIP window will be disabled when VDIR set as 1.
*/
	unsigned char temp;
	
	temp = lcdRegDataRead(RA8876_DPCR);
	temp &= cClrb3;
	lcdRegDataWrite(RA8876_DPCR, temp);

}

void RA8876_t3::VSCAN_B_to_T(void)
{
/*	
Vertical Scan direction
0 : From Top to Bottom
1 : From bottom to Top
PIP window will be disabled when VDIR set as 1.
*/
  
	unsigned char temp, temp_in;
	
	temp_in =  temp = lcdRegDataRead(RA8876_DPCR);
	temp |= cSetb3;
	lcdRegDataWrite(RA8876_DPCR, temp);
	Serial.printf("call vscan_b_to_t %x %x\n", temp_in, temp);

}

#if defined(USE_FT5206_TOUCH)
/**************************************************************************/
/*!
	The FT5206 Capacitive Touch driver uses a different INT pin than RA8876_t3
	and it's not controlled by RA chip of course, so we need a separate code
	for that purpose, no matter we decide to use an ISR or digitalRead.
	no matter if we will use ISR or DigitalRead
	Parameters:
	INTpin: it's the pin where we listen to ISR
	RSTpin: it's the optional pin to maybe reset the CTP controller
	This last parameter it's used only when decide to use an ISR.
*/
/**************************************************************************/
void RA8876_t3::useCapINT(const uint8_t INTpin,const uint8_t RSTPin) 
{
	_intCTSPin = INTpin;
	_rstCTSPin = RSTPin;
	pinMode(INTpin ,INPUT_PULLUP);
	_wire->begin();
	_wire->setClock(400000UL); // Set I2C frequency to 400kHz

	if (_rstCTSPin != 255) {
		pinMode(_rstCTSPin, OUTPUT);
		digitalWrite(_rstCTSPin, HIGH);
		delay(1);
		digitalWrite(_rstCTSPin, LOW);
		delayMicroseconds(750);
		digitalWrite(_rstCTSPin, HIGH);
	}
	delay(10);
	_initializeFT5206();//initialize FT5206 controller
}

/**************************************************************************/
/*!
		Since FT5206 uses a different INT pin, we need a separate isr routine. 
		[private]
*/
/**************************************************************************/
void RA8876_t3::cts_isr(void)
{
	_FT5206_INT = true;
}

/**************************************************************************/
/*!
		Enable the ISR, after this any falling edge on
		_intCTSPin pin will trigger ISR
		Parameters:
		force: if true will force attach interrup
		NOTE:
		if parameter _needCTS_ISRrearm = true will rearm interrupt
*/
/**************************************************************************/
void RA8876_t3::enableCapISR(bool force) 
{
	if (force || _needCTS_ISRrearm){
		_needCTS_ISRrearm = false;
		attachInterrupt(digitalPinToInterrupt(_intCTSPin),cts_isr,FALLING);
		_FT5206_INT = false;
		_useISR = true;
	}
}

/**************************************************************************/
/*!
		Disable ISR [FT5206 version]
		Works only if previously enabled or do nothing.
*/
/**************************************************************************/
void RA8876_t3::_disableCapISR(void) 
{
	if (_useISR){
		detachInterrupt(digitalPinToInterrupt(_intCTSPin));
		_FT5206_INT = false;
		_useISR = false;
	}
}

/**************************************************************************/
/*!
		Print out some of the FT5206 registers for debug
*/
/**************************************************************************/
void RA8876_t3::printTSRegisters(Print &pr, uint8_t start, uint8_t count)
{
	// guessing. from registers pdf
	_wire->beginTransmission(_ctpAdrs);
	_wire->write(start);
	_wire->endTransmission(_ctpAdrs);

	pr.println("\nRA8876 CTP Registers");
	_wire->requestFrom((uint8_t)_ctpAdrs, count); //get 31 registers
	uint8_t index = 0xff;
	while(_wire->available()) {
		if ((++index & 0xf) == 0) pr.printf("\n  %02x:", start);
		pr.printf(" %02x", _wire->read());
		start++;
	}
	pr.println();
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+							     TOUCH SCREEN COMMONS						         +
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

/**************************************************************************/
/*!
		Checks an interrupt has occurred. return true if yes.
		Designed to run in loop.
		It works with ISR or DigitalRead methods
		Parameters:
		safe: true  (detach interrupt routine, has to be re-engaged manually!)
			  false (
*/
/**************************************************************************/
bool RA8876_t3::touched(bool safe)
{
	if (_useISR){//using interrupts
			_needCTS_ISRrearm = safe;
			if (_FT5206_INT)			{
			//there was an interrupt
					if (_needCTS_ISRrearm){//safe was true, detach int
						_disableCapISR();
					} else {//safe was false, do not detatch int and clear INT flag
							_FT5206_INT = false;
					}
					return true;
			}
			return false;
	} else {//not use ISR, digitalRead method
			return false;
	}
}

void RA8876_t3::setTouchLimit(uint8_t limit)
{
	if (limit > 5) limit = 5;//max 5 allowed
	_maxTouch = limit;
}

uint8_t RA8876_t3::getTouchLimit(void)
{
	return _maxTouch;
}

/**************************************************************************/
/*!
		Initialization sequence of the FT5206
*/
/**************************************************************************/
static const uint8_t _FT5206REgisters[9] = {
	0x16,0x3C,0xE9,0x01,0x01,0xA0,0x0A,0x06,0x28
};

void RA8876_t3::_initializeFT5206(void)
{
	uint8_t i;
	for (i=0x80;i<0x89;i++){
		_sendRegFT5206(i,_FT5206REgisters[i-0x80]);
	}
	_sendRegFT5206(0x00,0x00);//Device Mode


}

/**************************************************************************/
/*!
		Communicate with FT5206
*/
/**************************************************************************/
void RA8876_t3::_sendRegFT5206(uint8_t reg,const uint8_t val)
{
	_wire->beginTransmission(_ctpAdrs);
	_wire->write(reg);
	_wire->write(val);
	_wire->endTransmission(_ctpAdrs);
}


/**************************************************************************/
/*!
		This is the function that update the current state of the Touch Screen
		It's developed for use in loop
*/
/**************************************************************************/
void RA8876_t3::updateTS(void)
{
    _wire->requestFrom((uint8_t)_ctpAdrs,(uint8_t)31); //get 31 registers
    uint8_t index = 0;
    while(_wire->available()) {
      _cptRegisters[index++] = _wire->read();//fill registers
    }
	_currentTouches = _cptRegisters[0x02] & 0xF;
	if (_currentTouches > _maxTouch) _currentTouches = _maxTouch;
	_gesture = _cptRegisters[0x01];
	if (_maxTouch < 2) _gesture = 0;
	uint8_t temp = _cptRegisters[0x03];
	_currentTouchState = 0;
	if (!bitRead(temp,7) && bitRead(temp,6)) _currentTouchState = 1;//finger up
	if (bitRead(temp,7) && !bitRead(temp,6)) _currentTouchState = 2;//finger down
}


/**************************************************************************/
/*!
		It gets coordinates out from data collected by updateTS function
		Actually it will not communicate with FT5206, just reorder collected data
		so it MUST be used after updateTS!
*/
/**************************************************************************/
uint8_t RA8876_t3::getTScoordinates(uint16_t (*touch_coordinates)[2])
{
	uint8_t i;
	if (_currentTouches < 1) return 0;
 	for (i=1;i<=_currentTouches;i++){
		switch(_rotation){
			case 0://ok
				//touch_coordinates[i-1][0] = _width - (((_cptRegisters[coordRegStart[i-1]] & 0x0f) << 8) | _cptRegisters[coordRegStart[i-1] + 1]) / (4096/_width);
				//touch_coordinates[i-1][1] = (((_cptRegisters[coordRegStart[i-1]] & 0x0f) << 8) | _cptRegisters[coordRegStart[i-1] + 1]) / (4096/_height);
				touch_coordinates[i-1][0] = ((_cptRegisters[coordRegStart[i-1]] & 0x0f) << 8) | _cptRegisters[coordRegStart[i-1] + 1];
				touch_coordinates[i-1][1] = ((_cptRegisters[coordRegStart[i-1] + 2] & 0x0f) << 8) | _cptRegisters[coordRegStart[i-1] + 3];
			break;
			case 1://ok
				touch_coordinates[i-1][0] = (((_cptRegisters[coordRegStart[i-1] + 2] & 0x0f) << 8) | _cptRegisters[coordRegStart[i-1] + 3]);
				touch_coordinates[i-1][1] = (_width - 1) - (((_cptRegisters[coordRegStart[i-1]] & 0x0f) << 8) | _cptRegisters[coordRegStart[i-1] + 1]);
			break;
			case 2://ok
				touch_coordinates[i-1][0] = (_width - 1) - (((_cptRegisters[coordRegStart[i-1]] & 0x0f) << 8) | _cptRegisters[coordRegStart[i-1] + 1]);
				touch_coordinates[i-1][1] = (_height - 1) -(((_cptRegisters[coordRegStart[i-1] + 2] & 0x0f) << 8) | _cptRegisters[coordRegStart[i-1] + 3]);
			break;
			case 3://ok
				touch_coordinates[i-1][0] = (_height - 1) - (((_cptRegisters[coordRegStart[i-1] + 2] & 0x0f) << 8) | _cptRegisters[coordRegStart[i-1] + 3]);
				touch_coordinates[i-1][1] = (((_cptRegisters[coordRegStart[i-1]] & 0x0f) << 8) | _cptRegisters[coordRegStart[i-1] + 1]);
			break;
		}
		if (i == _maxTouch) return i;
	} 
    return _currentTouches;
}

/**************************************************************************/
/*!
		Gets the current Touch State, must be used AFTER updateTS!
*/
/**************************************************************************/
uint8_t RA8876_t3::getTouchState(void)
{
	return _currentTouchState;
}

/**************************************************************************/
/*!
		Gets the number of touches, must be used AFTER updateTS!
		Return 0..5
		0: no touch
*/
/**************************************************************************/
uint8_t RA8876_t3::getTouches(void)
{
	return _currentTouches;
}

/**************************************************************************/
/*!
		Gets the gesture, if identified, must be used AFTER updateTS!
*/
/**************************************************************************/
uint8_t RA8876_t3::getGesture(void)
{
	//if gesture is up, down, left or right, juggle with bit2 & bit3 to match rotation
	if((_gesture >> 4) & 1){
		switch(_rotation){
			case 0:
				_gesture ^= 1 << 2;
				if(!((_gesture >> 2) & 1)) _gesture ^= 1 << 3;
			break;
			case 1:
				_gesture ^= 1 << 3;
			break;
			case 2:
				_gesture ^= 1 << 2;
				if((_gesture >> 2) & 1) _gesture ^= 1 << 3;
			break;
		}
	}
	return _gesture;
}
#endif
