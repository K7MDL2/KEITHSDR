//**************************************************************//
/*
File Name : Ra8876_Lite.h                                   
Author    : RAiO Application Team                             
Edit Date : 12/29/2015
Version   : v1.0
*
* Modified Version of: File Name : Ra8876_Lite.h                                   
 *			Author    : RAiO Application Team                             
 *			Edit Date : 09/13/2017
 * 	  	     : For Teensy 3.x and T4
 *                   : By Warren Watson
 *                   : 06/07/2018 - 11/31/2019
 *                   : Copyright (c) 2017-2019 Warren Watson.
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
/*File Name : tft.h                                   
 *          : For Teensy 3.x and T4
 *          : By Warren Watson
 *          : 06/07/2018 - 11/31/2019
 *          : Copyright (c) 2017-2019 Warren Watson.
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
 /***************************************************************
 *  Modified by mjs513 and KurtE and MorganS 
 *  Combined libraries and added functions to be compatible with
 *  other display libraries
 *  See PJRC Forum Thread: https://forum.pjrc.com/threads/58565-RA8876LiteTeensy-For-Teensy-T36-and-T40/page5
 *
 ***************************************************************/

#include "Arduino.h"
#include "SPI.h"
#include "RA8876Registers.h"

#ifndef _RA8876_T3
#define _RA8876_T3

#define USE_FT5206_TOUCH

/* Addins for ILI and GFX Fonts */
#include "ILI9341_fonts.h"

#if !defined(swapvals)
	#if defined(ESP8266)
		#define swapvals(a, b) { int16_t t = a; a = b; b = t; }
		//#define swapvals(a, b) { typeid(a) t = a; a = b; b = t; }
	#else
		#define swapvals(a, b) { typeof(a) t = a; a = b; b = t; }
	#endif
#endif

// Lets see about supporting Adafruit fonts as well?
#if __has_include(<gfxfont.h>)
	#include <gfxfont.h>
#endif

// Lets see about supporting Adafruit fonts as well?
#ifndef _GFXFONT_H_
#define _GFXFONT_H_

/// Font data stored PER GLYPH
typedef struct {
	uint16_t bitmapOffset;     ///< Pointer into GFXfont->bitmap
	uint8_t  width;            ///< Bitmap dimensions in pixels
    uint8_t  height;           ///< Bitmap dimensions in pixels
	uint8_t  xAdvance;         ///< Distance to advance cursor (x axis)
	int8_t   xOffset;          ///< X dist from cursor pos to UL corner
    int8_t   yOffset;          ///< Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct { 
	uint8_t  *bitmap;      ///< Glyph bitmaps, concatenated
	GFXglyph *glyph;       ///< Glyph array
	uint8_t   first;       ///< ASCII extents (first char)
    uint8_t   last;        ///< ASCII extents (last char)
	uint8_t   yAdvance;    ///< Newline distance (y axis)
} GFXfont;

#endif // _GFXFONT_H_ 


// Default to a relatively slow speed for breadboard testing. 
//const ru32 SPIspeed = 47000000;
const ru32 SPIspeed = 3000000;

// Max. size in byte of SDRAM
const uint32_t MEM_SIZE_MAX	= 16l*1024l*1024l;

#if defined(USE_FT5206_TOUCH)
#include <Wire.h>
#endif


class RA8876_t3 : public Print
{
public:
	RA8876_t3(const uint8_t CSp = 10, const uint8_t RSTp = 8, const uint8_t mosi_pin = 11, const uint8_t sclk_pin = 13, const uint8_t miso_pin = 12);

	
	volatile bool	RA8876_BUSY; //This is used to show an SPI transaction is in progress. 
	volatile bool   activeDMA=false; //Unfortunately must be public so asyncEventResponder() can set it
	void textRotate(boolean on);
	void		setRotation(uint8_t rotation); //rotate text and graphics
	uint8_t		getRotation(); //return the current rotation 0-3
	/* Initialize RA8876 */
	boolean begin(uint32_t spi_clock=SPIspeed);
	boolean ra8876Initialize(); 
	boolean ra8876PllInitial (void);
	boolean ra8876SdramInitial(void);
	
	/* Test */
	void Color_Bar_ON(void);
	void Color_Bar_OFF(void);
	
	/*access*/
	void lcdRegWrite(ru8 reg, bool finalize = true);
	void lcdDataWrite(ru8 data, bool finalize = true);
	ru8 lcdDataRead(bool finalize = true);
	ru16 lcdDataRead16bpp(bool finalize = true);
	ru8 lcdStatusRead(bool finalize = true);
	void lcdRegDataWrite(ru8 reg,ru8 data, bool finalize = true);
	ru8 lcdRegDataRead(ru8 reg, bool finalize = true);
	void lcdDataWrite16bbp(ru16 data, bool finalize = true); 
	
	/*Status*/
	void checkWriteFifoNotFull(void);
	void checkWriteFifoEmpty(void);
	void checkReadFifoNotFull(void);
	void checkReadFifoFull(void);
	void checkReadFifoNotEmpty(void);
	void check2dBusy(void);
	boolean checkSdramReady(void);
	ru8 powerSavingStatus(void);
	boolean checkIcReady(void);//
	
	// Display
	void displayOn(boolean on);
	void backlight(boolean on);
	void lcdHorizontalWidthVerticalHeight(ru16 width,ru16 height);
	void lcdHorizontalNonDisplay(ru16 numbers);
	void lcdHsyncStartPosition(ru16 numbers);
	void lcdHsyncPulseWidth(ru16 numbers);
	void lcdVerticalNonDisplay(ru16 numbers);
	void lcdVsyncStartPosition(ru16 numbers);
	void lcdVsyncPulseWidth(ru16 numbers);
	void displayImageStartAddress(ru32 addr);
	void displayImageWidth(ru16 width);
	void displayWindowStartXY(ru16 x0,ru16 y0);
	void canvasImageStartAddress(ru32 addr);
	void canvasImageWidth(ru16 width);
	void activeWindowXY(ru16 x0,ru16 y0);
	void activeWindowWH(ru16 width,ru16 height);
	
	/*PWM function*/
	void pwm_Prescaler(ru8 prescaler);
	void pwm_ClockMuxReg(ru8 pwm1_clk_div, ru8 pwm0_clk_div, ru8 xpwm1_ctrl, ru8 xpwm0_ctrl);
	void pwm_Configuration(ru8 pwm1_inverter,ru8 pwm1_auto_reload,ru8 pwm1_start,ru8 
						  pwm0_dead_zone, ru8 pwm0_inverter, ru8 pwm0_auto_reload,ru8 pwm0_start);

	void pwm0_ClocksPerPeriod(ru16 clocks_per_period);
	void pwm0_Duty(ru16 duty);
	void pwm1_ClocksPerPeriod(ru16 clocks_per_period);
	void pwm1_Duty(ru16 duty);
	
	// RAM Access			
	void ramAccessPrepare(void);
	ru8  vmemReadData(ru32 addr);
	ru16 vmemReadData16(ru32 addr);
	void vmemWriteData(ru32 addr, ru8 vmemData);
	void vmemWriteData16(ru32 addr, ru16 vmemData);
	void Memory_Select_SDRAM(void);
	void Memory_Select_Graphic_Cursor_RAM(void);	
	void Memory_Select_CGRAM(void);
	void CGRAM_initial(uint32_t charAddr, const uint8_t *data, uint16_t count);
	void Memory_XY_Mode(void);
	void Memory_Linear_Mode(void);


	/*graphic function*/
	void graphicMode(boolean on);
	void setPixelCursor(ru16 x,ru16 y);
	void drawPixel(ru16 x, ru16 y, ru16 color);
	ru16 getPixel(ru16 x, ru16 y);
	void foreGroundColor16bpp(ru16 color, bool finalize = true);
	void backGroundColor16bpp(ru16 color, bool finalize = true);
	
	
	/*  Picture Functions */
	void putPicture_16bpp(ru16 x,ru16 y,ru16 width, ru16 height);  //not recommended: use BTE instead
	void putPicture_16bppData8(ru16 x,ru16 y,ru16 width, ru16 height, const unsigned char *data);  //not recommended: use BTE instead
	void putPicture_16bppData16(ru16 x,ru16 y,ru16 width, ru16 height, const unsigned short *data);  //not recommended: use BTE instead
	
	
	/* Graphic Cursor Function */
	void Enable_Graphic_Cursor(void);
	void Disable_Graphic_Cursor(void);
	void Select_Graphic_Cursor_1(void);
	void Select_Graphic_Cursor_2(void);
	void Select_Graphic_Cursor_3(void);
	void Select_Graphic_Cursor_4(void);
	void Upload_Graphic_Cursor(uint8_t cursorNum, uint8_t *data);
	//**[40h][41h][42h][43h]**//
	void Graphic_Cursor_XY(int16_t WX,int16_t HY);
	//**[44]**//
	void Set_Graphic_Cursor_Color_1(unsigned char temp);
	//**[45]**//
	void Set_Graphic_Cursor_Color_2(unsigned char temp);
	void Graphic_cursor_initial(void);

	uint32_t boxPut(uint32_t vPageAddr, uint16_t x0, uint16_t y0,uint16_t x1, uint16_t y1, uint16_t dx0, uint16_t dy0);
	uint32_t boxGet(uint32_t vPageAddr, uint16_t x0, uint16_t y0,uint16_t x1, uint16_t y1, uint16_t dx0, uint16_t dy0);

	
	/* Button Functions */
	void initButton(struct Gbuttons *button, uint16_t x, uint16_t y, uint8_t w, uint8_t h,
		uint16_t outline, uint16_t fill, uint16_t textcolor,
		char *label, uint8_t textsize);
	void drawButton(struct Gbuttons *buttons, bool inverted);
	bool buttonContains(struct Gbuttons *buttons,uint16_t x, uint16_t y);
	void buttonPress(struct Gbuttons *buttons, bool p);
	bool buttonIsPressed(struct Gbuttons *buttons);
	bool buttonJustPressed(struct Gbuttons *buttons);
	bool buttonJustReleased(struct Gbuttons *buttons);
	
	
	/*  Font Functions  */
	//**[DBh]~[DEh]**//
	void CGRAM_Start_address(uint32_t Addr);
	void setTextParameter1(ru8 source_select,ru8 size_select,ru8 iso_select);//cch
	void setTextParameter2(ru8 align, ru8 chroma_key, ru8 width_enlarge, ru8 height_enlarge);//cdh
	void genitopCharacterRomParameter(ru8 scs_select, ru8 clk_div, ru8 rom_select, ru8 character_select, ru8 gt_width);//b7h,bbh,ceh,cfh


	/*text function*/
	void textMode(boolean on);
	void textColor(ru16 foreground_color,ru16 background_color);
	void setTextCursor(ru16 x,ru16 y);
	void textxy(ru16 x, ru16 y);
	void buildTextScreen(void);
	void setFontSource(uint8_t source);
	//**[5Fh]~[62h]**//
	void linearAddressSet(ru32 addr);
	
	void Enable_Text_Cursor(void);
	void Disable_Text_Cursor(void);
	void Enable_Text_Cursor_Blinking(void);
	void Disable_Text_Cursor_Blinking(void);
	void Blinking_Time_Frames(unsigned char temp);
	void Text_Cursor_H_V(unsigned short WX,unsigned short HY);
	void scroll(void);
	void scrollDown(void);
	void putString(ru16 x0,ru16 y0, const char *str);
	void writeStatusLine(ru16 x0, uint16_t fgcolor, uint16_t bgcolor, const char *str);	
	
	
	/* Screen Functions */
	void selectScreen(uint32_t screenPage);
	void saveTFTParams(tftSave_t *screenSave);
	void restoreTFTParams(tftSave_t *screenSave);
	void update_xy(void);
	void update_tft(uint8_t data);

	void clearActiveScreen(void);
	void clreol(void);
	void clreos(void);
	void clrbol(void);
	void clrbos(void);
	void clrlin(void);
	void clearStatusLine(uint16_t color); 
	
	/* Pseudo Frame Buffer Support */
	void useCanvas(boolean on);
	void updateScreen();
	
	 
	/*draw function*/
	void drawLine(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color);
	void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
	void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
	void drawSquare(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color);
	void drawSquareFill(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 color);
	void drawCircleSquare(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 xr, ru16 yr, ru16 color);
	void drawCircleSquareFill(ru16 x0, ru16 y0, ru16 x1, ru16 y1, ru16 xr, ru16 yr, ru16 color);
	void drawTriangle(ru16 x0,ru16 y0,ru16 x1,ru16 y1,ru16 x2,ru16 y2,ru16 color);
	void drawTriangleFill(ru16 x0,ru16 y0,ru16 x1,ru16 y1,ru16 x2,ru16 y2,ru16 color);
	void drawCircle(ru16 x0,ru16 y0,ru16 r,ru16 color);
	void drawCircleFill(ru16 x0,ru16 y0,ru16 r,ru16 color);
	void drawEllipse(ru16 x0,ru16 y0,ru16 xr,ru16 yr,ru16 color);
	void drawEllipseFill(ru16 x0,ru16 y0,ru16 xr,ru16 yr,ru16 color);
  
  /* New Functions for 2024 */
  void readRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *pcolors);
  uint16_t readPixel(int16_t x, int16_t y);
  void fillRectHGradient(int16_t x, int16_t y, int16_t w, int16_t h,
                                            uint16_t color1, uint16_t color2);
  void fillRectVGradient(int16_t x, int16_t y, int16_t w, int16_t h,
                                            uint16_t color1, uint16_t color2);
	 
	/*BTE function*/
	void bte_Source0_MemoryStartAddr(ru32 addr);
	void bte_Source0_ImageWidth(ru16 width);
	void bte_Source0_WindowStartXY(ru16 x0,ru16 y0);
	void bte_Source1_MemoryStartAddr(ru32 addr);
	void bte_Source1_ImageWidth(ru16 width);
	void bte_Source1_WindowStartXY(ru16 x0,ru16 y0);
	void bte_DestinationMemoryStartAddr(ru32 addr);
	void bte_DestinationImageWidth(ru16 width);
	void bte_DestinationWindowStartXY(ru16 x0,ru16 y0);
	void bte_WindowSize(ru16 width, ru16 height);
	void bte_WindowAlpha(ru8 alpha);
	
	void bteMemoryCopy(ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,ru32 des_addr,ru16 des_image_width, 
					   ru16 des_x,ru16 des_y,ru16 copy_width,ru16 copy_height);
	void bteMemoryCopyWithROP(ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,
							   ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 copy_width,ru16 copy_height,ru8 rop_code);
	void bteMemoryCopyWithChromaKey(ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
								   ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 copy_width,ru16 copy_height,ru16 chromakey_color);
	void bteMemoryCopyWindowAlpha(ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
									ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,
								   ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 copy_width,ru16 copy_height,ru8 alpha);
	void bteMpuWriteWithROPData8(ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,ru32 des_addr,ru16 des_image_width,
							ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru8 rop_code,const unsigned char *data);
	void bteMpuWriteWithROPData16(ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,ru32 des_addr,ru16 des_image_width,
							ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru8 rop_code,const unsigned short *data);
	bool DMAFinished() {return !activeDMA;}
	void bteMpuWriteWithROP(ru32 s1_addr,ru16 s1_image_width,ru16 s1_x,ru16 s1_y,ru32 des_addr,ru16 des_image_width,
							ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru8 rop_code);                     
	void bteMpuWriteWithChromaKeyData8(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 chromakey_color,
								 const unsigned char *data);
	void bteMpuWriteWithChromaKeyData16(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 chromakey_color,
								 const unsigned short *data);
	void bteMpuWriteWithChromaKey(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 chromakey_color);
	void bteMpuWriteColorExpansionData(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 foreground_color,ru16 background_color,const unsigned char *data);
	void bteMpuWriteColorExpansion(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 foreground_color,ru16 background_color);
	void bteMpuWriteColorExpansionWithChromaKeyData(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,
												ru16 foreground_color,ru16 background_color,const unsigned char *data);
	void bteMpuWriteColorExpansionWithChromaKey(ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,
												ru16 width,ru16 height,ru16 foreground_color,ru16 background_color);

	void btePatternFill(ru8 p8x8or16x16, ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
					   ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height);
	void btePatternFillWithChromaKey(ru8 p8x8or16x16, ru32 s0_addr,ru16 s0_image_width,ru16 s0_x,ru16 s0_y,
									ru32 des_addr,ru16 des_image_width, ru16 des_x,ru16 des_y,ru16 width,ru16 height,ru16 chromakey_color);
									
	/*DMA function*/
	void setSerialFlash4BytesMode(ru8 scs_select);
	void dma_24bitAddressBlockMode(ru8 scs_selct,ru8 clk_div,ru16 x0,ru16 y0,ru16 width,ru16 height,ru16 picture_width,ru32 addr);
	void dma_32bitAddressBlockMode(ru8 scs_selct,ru8 clk_div,ru16 x0,ru16 y0,ru16 width,ru16 height,ru16 picture_width,ru32 addr);

	
	/* PIP window funtions */
	void PIP
	(
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
	);

	void PIP_Display_Start_XY(unsigned short WX,unsigned short HY);	
	void PIP_Image_Start_Address(unsigned long Addr);	
	void PIP_Image_Width(unsigned short WX);	
	void PIP_Window_Image_Start_XY(unsigned short WX,unsigned short HY);	
	void PIP_Window_Width_Height(unsigned short WX,unsigned short HY);	

	//**[10h]**//
	void Enable_PIP1(void);
	void Disable_PIP1(void);
	void Enable_PIP2(void);
	void Disable_PIP2(void);
	void Select_PIP1_Parameter(void);
	void Select_PIP2_Parameter(void);
	void Select_Main_Window_8bpp(void);
	void Select_Main_Window_16bpp(void);
	void Select_Main_Window_24bpp(void);
	void Select_LCD_Sync_Mode(void);
	void Select_LCD_DE_Mode(void);
	//**[11h]**//
	void Select_PIP1_Window_8bpp(void);
	void Select_PIP1_Window_16bpp(void);
	void Select_PIP1_Window_24bpp(void);
	void Select_PIP2_Window_8bpp(void);
	void Select_PIP2_Window_16bpp(void);
	void Select_PIP2_Window_24bpp(void);
	
	/****************************************/
	
	void tftRawWrite(uint8_t data);
	void printStatusLine(uint16_t x0,uint16_t fgColor,uint16_t bgColor, const char *text);
	void fillScreen(uint16_t color);
	void fillStatusLine(uint16_t color);
	void setTextColor(uint16_t color);
	void setBackGroundColor(uint16_t color);
	void setTextColor(uint16_t fgc, uint16_t bgc);
	int16_t getTextX(void);

	void setMargins(uint16_t xl, uint16_t yt, uint16_t xr, uint16_t yb);
	void setTMargins(uint16_t xl, uint16_t yt, uint16_t xr, uint16_t yb);
	void setPromptSize(uint16_t ps);
	uint8_t fontLoad(char *fontfile);
	uint8_t fontLoadMEM(char *fontsrc);
	//void setFontSource(uint8_t source);
	boolean setFontSize(uint8_t scale, boolean runflag=false);
	//void setTextSize(uint8_t scale, boolean runflag=false) { setFontSize(scale, runflag);}
	int16_t getTextY(void);
	int16_t getTwidth(void);
	int16_t getTheight(void);
	void setCursorMode(boolean mode);
	void setCursorType(uint8_t type);
	void setCursorBlink(boolean onOff);
	uint16_t getTextFGC(void);
	uint16_t getTextBGC(void);
	int16_t width(void) { return _width; }
	int16_t height(void) { return _height; }
	uint8_t getFontHeight(void);
	uint8_t getFontWidth(void);
	

	void drawRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color);
	void fillRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color);

	void writeRect(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors);
	// Experiment to see if we get significant speed ups for images if they are already pre processed to 
	// draw in current/specified orientation. 
	uint16_t *rotateImageRect(int16_t w, int16_t h, const uint16_t *pcolors, int16_t rotation=-1);
	void writeRotatedRect(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *pcolors);
    // writeRect8BPP - 	write 8 bit per pixel paletted bitmap
    //					bitmap data in array at pixels, one byte per
    // pixel
    //					color palette data in array at palette
    void writeRect8BPP(int16_t x, int16_t y, int16_t w, int16_t h,
                       const uint8_t *pixels, const uint16_t *palette);

    // writeRect4BPP - 	write 4 bit per pixel paletted bitmap
    //					bitmap data in array at pixels, 4 bits per
    // pixel
    //					color palette data in array at palette
    //					width must be at least 2 pixels
    void writeRect4BPP(int16_t x, int16_t y, int16_t w, int16_t h,
                       const uint8_t *pixels, const uint16_t *palette);

    // writeRect2BPP - 	write 2 bit per pixel paletted bitmap
    //					bitmap data in array at pixels, 4 bits per
    // pixel
    //					color palette data in array at palette
    //					width must be at least 4 pixels
    void writeRect2BPP(int16_t x, int16_t y, int16_t w, int16_t h,
                       const uint8_t *pixels, const uint16_t *palette);

    // writeRect1BPP - 	write 1 bit per pixel paletted bitmap
    //					bitmap data in array at pixels, 4 bits per
    // pixel
    //					color palette data in array at palette
    //					width must be at least 8 pixels
    void writeRect1BPP(int16_t x, int16_t y, int16_t w, int16_t h,
                       const uint8_t *pixels, const uint16_t *palette);

    // writeRectNBPP - 	write N(1, 2, 4, 8) bit per pixel paletted bitmap
    //					bitmap data in array at pixels
    //  Currently writeRect1BPP, writeRect2BPP, writeRect4BPP use this to do all
    //  of the work.
    //
    void writeRectNBPP(int16_t x, int16_t y, int16_t w, int16_t h,
                       uint8_t bits_per_pixel, const uint8_t *pixels,
                       const uint16_t *palette);



	void drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t xr, uint16_t yr, uint16_t color);
	void fillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t xr, uint16_t yr, uint16_t color);
	void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
	void fillEllipse(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color);
	void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
	void cursorInit(void);
	//void setCursor(uint16_t x, uint16_t y);
	void gCursorSet(boolean gCursorEnable, uint8_t gcursortype, uint8_t gcursorcolor1, uint8_t gcursorcolor2);
	void gcursorxy(uint16_t gcx, uint16_t gcy);
	uint16_t GetGCursorX() {return gCursorX;}
	uint16_t GetGCursorY() {return gCursorY;}


	void touchEnable(boolean enabled);
	void readTouchADC(uint16_t *x, uint16_t *y);

	
	boolean TStouched(void);
	void getTSpoint(uint16_t *x, uint16_t *y);
	#if defined (USE_FT5206_TOUCH)
	bool 		touched(bool safe=false);
	void 		setTouchLimit(uint8_t limit);//5 for FT5206, 1 for  RA8875
	uint8_t 	getTouchLimit(void);

	void		setWireObject(TwoWire *wire) {_wire = wire; }
	void		useCapINT(const uint8_t INTpin=2,const uint8_t RSTPin=255);
	void 		enableCapISR(bool force = false); 
	void	 	updateTS(void);
	uint8_t 	getGesture(void);
	uint8_t 	getTouches(void);
	uint8_t 	getTouchState(void);
	uint8_t 	getTScoordinates(uint16_t (*touch_coordinates)[2]);
	void  		printTSRegisters(Print &pr, uint8_t start, uint8_t count);
	#endif


	void putPicture(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const unsigned char *data);

	void scrollUp(void);
  
  // Pass 8-bit (each) R,G,B, get back 16-bit packed color
  static uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
      return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }

  // color565toRGB		- converts 565 format 16 bit color to RGB
  static void color565toRGB(uint16_t color, uint8_t &r, uint8_t &g,
                            uint8_t &b) {
      r = (color >> 8) & 0x00F8;
      g = (color >> 3) & 0x00FC;
      b = (color << 3) & 0x00F8;
  }

  // color565toRGB14		- converts 16 bit 565 format color to 14 bit RGB (2
  // bits clear for math and sign)
  // returns 00rrrrr000000000,00gggggg00000000,00bbbbb000000000
  // thus not overloading sign, and allowing up to double for additions for
  // fixed point delta
  static void color565toRGB14(uint16_t color, int16_t &r, int16_t &g,
                              int16_t &b) {
      r = (color >> 2) & 0x3E00;
      g = (color << 3) & 0x3F00;
      b = (color << 9) & 0x3E00;
  }
  
  // RGB14tocolor565		- converts 14 bit RGB back to 16 bit 565 format
  // color
  static uint16_t RGB14tocolor565(int16_t r, int16_t g, int16_t b) {
      return (((r & 0x3E00) << 2) | ((g & 0x3F00) >> 3) | ((b & 0x3E00) >> 9));
  }
	
//Hack fonts - RA8875 additional functions
	// setOrigin sets an offset in display pixels where drawing to (0,0) will appear
	// for example: setOrigin(10,10); drawPixel(5,5); will cause a pixel to be drawn at hardware pixel (15,15)
	void setOrigin(int16_t x = 0, int16_t y = 0) { 
		_originx = x; _originy = y; 
		//if (Serial) Serial.printf("Set Origin %d %d\n", x, y);
		updateDisplayClip();
	}
	void getOrigin(int16_t* x, int16_t* y) { *x = _originx; *y = _originy; }

	// setClipRect() sets a clipping rectangle (relative to any set origin) for drawing to be limited to.
	// Drawing is also restricted to the bounds of the display

	void setClipRect(int16_t x1, int16_t y1, int16_t w, int16_t h) 
		{ _clipx1 = x1; _clipy1 = y1; _clipx2 = x1+w; _clipy2 = y1+h; 
			//if (Serial) Serial.printf("Set clip Rect %d %d %d %d\n", x1, y1, w, h);
			updateDisplayClip();
		}
	void setClipRect() {
			 _clipx1 = 0; _clipy1 = 0; _clipx2 = _width; _clipy2 = _height; 
			//if (Serial) Serial.printf("clear clip Rect\n");
			 updateDisplayClip(); 
		}
		
	bool _invisible = false; 
	bool _standard = true; // no bounding rectangle or origin set. 

	inline void updateDisplayClip() {
		_displayclipx1 = max((int16_t)0,min((int16_t)(_clipx1+_originx),width()));
		_displayclipx2 = max((int16_t)0,min((int16_t)(_clipx2+_originx),width()));

		_displayclipy1 = max((int16_t)0,min((int16_t)(_clipy1+_originy),height()));
		_displayclipy2 = max((int16_t)0,min((int16_t)(_clipy2+_originy),height()));
		_invisible = (_displayclipx1 == _displayclipx2 || _displayclipy1 == _displayclipy2);
		_standard =  (_displayclipx1 == 0) && (_displayclipx2 == _width) && (_displayclipy1 == 0) && (_displayclipy2 == _height);
		if (Serial) {
			//Serial.printf("UDC (%d %d)-(%d %d) %d %d\n", _displayclipx1, _displayclipy1, _displayclipx2, 
			//	_displayclipy2, _invisible, _standard);

		}
	}

	// BUGBUG:: two different versions as some places used signed others use unsigned...
	inline void rotateCCXY(int16_t &x, int16_t &y) {
		int16_t yt;
	  	yt = y; y = x; x = _height - yt; 
	}

	inline void rotateCCXY(ru16 &x, ru16 &y) {
		ru16 yt;
	  	yt = y; y = x; x = _height - yt; 
	}

	
	inline void setFontDef() { 
		_use_default = 1; 
		//if(_portrait && (!_use_gfx_font || !_use_ili_font)) {
		//	_cursorX += _cursorY;
		//_cursorY -= _cursorX;
		//}
		_use_ili_font=0; 
		_use_gfx_font=0;
		_use_int_font=1;
		_use_tfont=0;
		setActiveWindow();
		_textPosition(_cursorX, _cursorY, false);
		};
	void setFont(const ILI9341_t3_font_t &f);
    void setFont(const GFXfont *f = NULL);
	void setFontAdafruit(void) { setFont(); }
	void drawFontChar(unsigned int c);
	void drawGFXFontChar(unsigned int c);
	
    void setTextSize(uint8_t sx, uint8_t sy);
	void inline setTextSize(uint8_t s) { setTextSize(s,s); }
	
	void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);
	void inline drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) 
	    { drawChar(x, y, c, color, bg, size);}
	void drawFontBits(bool opaque, uint32_t bits, uint32_t numbits, int32_t x, int32_t y, uint32_t repeat);
	void Pixel(int16_t x, int16_t y, uint16_t color);
	
	void charBounds(char c, int16_t *x, int16_t *y,
		int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);
    void getTextBounds(const uint8_t *buffer, uint16_t len, int16_t x, int16_t y,
      int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void getTextBounds(const char *string, int16_t x, int16_t y,
      int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
    void getTextBounds(const String &str, int16_t x, int16_t y,
      int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
	int16_t strPixelLen(const char * str);

	void drawFontPixel( uint8_t alpha, uint32_t x, uint32_t y );

	void setCursor(int16_t x, int16_t y, bool autocenter=false);

	void getCursor(int16_t &x, int16_t &y); 
	int16_t getCursorX(void);
	int16_t getCursorY(void);
	
//. From Onewire utility files
	#if defined(__IMXRT1052__) || defined(__IMXRT1062__)  // Teensy 4.x
	void DIRECT_WRITE_LOW(volatile uint32_t * base, uint32_t mask)  __attribute__((always_inline)) {
		*(base+34) = mask;
	}
	void DIRECT_WRITE_HIGH(volatile uint32_t * base, uint32_t mask)  __attribute__((always_inline)) {
		*(base+33) = mask;
	}
	#endif

	//SPI Functions - should these be private?
	inline __attribute__((always_inline)) 
	void startSend(){
		#ifdef SPI_HAS_TRANSFER_ASYNC
		while(activeDMA) {}; //wait forever while DMA is finishing- can't start a new transfer
		#endif
		if(!RA8876_BUSY) {
	        RA8876_BUSY = true;
			_pspi->beginTransaction(SPISettings(_SPI_CLOCK, MSBFIRST, SPI_MODE0));
		}
		#if defined(__IMXRT1052__) || defined(__IMXRT1062__)  // Teensy 4.x 
		DIRECT_WRITE_LOW(_csport, _cspinmask);
		#else
			*_csport  &= ~_cspinmask;
		#endif
	}

	inline __attribute__((always_inline)) 
	void endSend(bool finalize){
		#if defined(__IMXRT1052__) || defined(__IMXRT1062__)  // Teensy 4.x 
		DIRECT_WRITE_HIGH(_csport, _cspinmask);
		#else
		*_csport |= _cspinmask;
		#endif
		if(finalize) {
			_pspi->endTransaction();
			RA8876_BUSY = false;
		}
	} 
	
	// overwrite print functions:
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buffer, size_t size);
	
	size_t rawPrint(uint8_t text);
	
	void LCD_CmdWrite(unsigned char cmd);

	
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    RA8876 Parameters
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	uint32_t			currentPage;
	uint32_t			lastPage;
	uint32_t			pageOffset;
	uint8_t				currentFont;
	
	
using Print::write;

private:
	// int _xnscs, _xnreset;
	int _mosi;
	int _miso;
	int _sclk;
	int _cs;
	int _rst;
	int	_errorCode;
	SPIClass *_pspi = nullptr;
//	SPIClass::SPI_Hardware_t *_spi_hardware;

  	uint8_t   	_spi_num;         	// Which buss is this spi on? 
	uint32_t 	_SPI_CLOCK;			// #define ILI9341_SPICLOCK 30000000
	uint32_t	_SPI_CLOCK_READ; 	//#define ILI9341_SPICLOCK_READ 2000000

#if defined(KINETISK)
 	KINETISK_SPI_t *_pkinetisk_spi;
#elif defined(__IMXRT1052__) || defined(__IMXRT1062__)  // Teensy 4.x
 	IMXRT_LPSPI_t *_pimxrt_spi;

#elif defined(KINETISL)
 	KINETISL_SPI_t *_pkinetisl_spi;
#endif

#ifdef SPI_HAS_TRANSFER_ASYNC
	EventResponder finishedDMAEvent;
#endif
	// add support to allow only one hardware CS (used for dc)
#if defined(__IMXRT1052__) || defined(__IMXRT1062__)  // Teensy 4.x
    uint32_t _cspinmask;
    volatile uint32_t *_csport;
    uint32_t _spi_tcr_current;
#else
    uint8_t _cspinmask;
    volatile uint8_t *_csport;
#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    RA8876 Parameters
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//uint32_t			currentPage;
	//uint32_t			lastPage;
	//uint32_t			pageOffset;
	//uint8_t				currentFont;

	// Text Sreen Vars
	uint8_t				vdata;
	uint8_t 			leftmarg;
	uint8_t 			topmarg;
	uint8_t 			rightmarg;
	uint8_t 			bottommarg;
	uint8_t 			tab_size;
	uint16_t 			prompt_size; // prompt ">"
	uint16_t 			prompt_line; // current text prompt row
	uint16_t 			CharPosX, CharPosY;
	boolean 			UDFont;

	//scroll vars ----------------------------
	uint16_t	_scrollXL,_scrollXR,_scrollYT,_scrollYB;
	uint16_t	_TXTForeColor;
	uint16_t	_TXTBackColor;
	
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    Font Parameters
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//index:x -> w,h,baselineLowOffset,baselineTopOffset,variableWidth

	// Font variables
	boolean						_textMode;
	int16_t						_width, 			  _height;
	int16_t						_cursorX, 		  _cursorY;
	int16_t						gCursorX, 		  gCursorY;
	uint8_t						_scaleX,			  _scaleY;
	bool						_scaling;
	uint8_t 					_cursorXsize;
	uint8_t 					_cursorYsize;
	uint8_t						_FNTwidth, 		  _FNTheight;
	uint8_t						_FNTspacing;
	uint8_t						_FNTinterline;
	int							 _spaceCharWidth;
	uint8_t 					_fontheight;
	
 //HACK
	uint8_t		_use_ili_font = 0;
	uint8_t 	_use_gfx_font = 0;
	uint8_t 	_use_tfont = 0;
	uint8_t		_use_int_font = 0;
	uint8_t 	_use_default = 1;
	uint8_t 	textsize, textsize_x, textsize_y;
	uint16_t 	textcolor, textbgcolor; 
	//anti-alias font
	uint8_t fontbpp = 1;
	uint8_t fontbppindex = 0;
	uint8_t fontbppmask = 1;
	uint8_t fontppb = 8;
	uint8_t* fontalphalut;
	float fontalphamx = 1;
	
	bool						_backTransparent;

	//centering -------------------------------
	bool						_relativeCenter;
	bool						_absoluteCenter;
	bool						_portrait;
	uint8_t						_rotation;
	int16_t  					_activeWindowXL,	_activeWindowXR;
	int16_t  					_activeWindowYT,_activeWindowYB;

	volatile uint8_t			  _TXTparameters;
	/* It contains several parameters in one byte
	bit			 parameter
	0	->		_extFontRom 		i's using an ext rom font
	1	->		_autoAdvance		after a char the pointer move ahead
	2	->		_textWrap
	3	->		_fontFullAlig		
	4	->		_fontRotation       (actually not used)
	5	->		_alignXToCenter;
	6	->		_alignYToCenter;
	7	->		_renderFont active;
	*/
	
	// Hack to see about combining outputs for speed.
	int16_t _combine_x_start = 0;
	int16_t _combine_y = 0;
	int16_t _combine_count = 0;
	uint16_t _combine_color = 0;
	
	/* Private Functions */
	uint32_t fetchbit(const uint8_t *p, uint32_t index);
	uint32_t fetchbits_unsigned(const uint8_t *p, uint32_t index, uint32_t required);
	uint32_t fetchbits_signed(const uint8_t *p, uint32_t index, uint32_t required);
	void 	 _fontWrite(const uint8_t* buffer, uint16_t len);
	
	/**
	 * Found in a pull request for the Adafruit framebuffer library. Clever!
	 * https://github.com/tricorderproject/arducordermini/pull/1/files#diff-d22a481ade4dbb4e41acc4d7c77f683d
	 * Converts  0000000000000000rrrrrggggggbbbbb
	 *     into  00000gggggg00000rrrrr000000bbbbb
	 * with mask 00000111111000001111100000011111
	 * This is useful because it makes space for a parallel fixed-point multiply
	 * This implements the linear interpolation formula: result = bg * (1.0 - alpha) + fg * alpha
	 * This can be factorized into: result = bg + (fg - bg) * alpha
	 * alpha is in Q1.5 format, so 0.0 is represented by 0, and 1.0 is represented by 32
	 * @param	fg		Color to draw in RGB565 (16bit)
	 * @param	bg		Color to draw over in RGB565 (16bit)
	 * @param	alpha	Alpha in range 0-255
	 **/
	uint16_t alphaBlendRGB565( uint32_t fg, uint32_t bg, uint8_t alpha )
	 __attribute__((always_inline)) {
	 	alpha = ( alpha + 4 ) >> 3; // from 0-255 to 0-31
		bg = (bg | (bg << 16)) & 0b00000111111000001111100000011111;
		fg = (fg | (fg << 16)) & 0b00000111111000001111100000011111;
		uint32_t result = ((((fg - bg) * alpha) >> 5) + bg) & 0b00000111111000001111100000011111;
		return (uint16_t)((result >> 16) | result); // contract result
	}
	
	/**
	 * Same as above, but fg and bg are premultiplied, and alpah is already in range 0-31
	 */
	uint16_t alphaBlendRGB565Premultiplied( uint32_t fg, uint32_t bg, uint8_t alpha )
	 __attribute__((always_inline)) {
		uint32_t result = ((((fg - bg) * alpha) >> 5) + bg) & 0b00000111111000001111100000011111;
		return (uint16_t)((result >> 16) | result); // contract result
	}
	
	uint32_t fetchpixel(const uint8_t *p, uint32_t index, uint32_t x);

	inline void combineAndDrawPixel(int16_t x, int16_t y, uint16_t color) {
		if (_combine_count && (color == _combine_color)) _combine_count++;
		else {
			if (_combine_count)drawLine(_combine_x_start, _combine_y, _combine_x_start+_combine_count-1, _combine_y, _combine_color);
			_combine_x_start = x;
			_combine_y = y;
			_combine_count = 1;
			_combine_color = color;
		}
	}

	inline void forceCombinedPixelsOut() {
		if (_combine_count)fillRect(_combine_x_start, _combine_y, _combine_count, 1, _combine_color);
		_combine_count = 0;
	}
	
	//rotation functions
	void MemWrite_Left_Right_Top_Down(void);
	void MemWrite_Right_Left_Top_Down(void);
	void MemWrite_Top_Down_Left_Right(void);
	void MemWrite_Down_Top_Left_Right(void);
	void VSCAN_T_to_B(void);
	void VSCAN_B_to_T(void);


protected:
	/* Hack for ILIXXXX and GFX fonts */
 	uint32_t textcolorPrexpanded, textbgcolorPrexpanded;
	boolean wrap; // If set, 'wrap' text at right edge of display
	const ILI9341_t3_font_t *font;
	
	int16_t  _clipx1, _clipy1, _clipx2, _clipy2;
	int16_t  _originx, _originy;
	int16_t  _displayclipx1, _displayclipy1, _displayclipx2, _displayclipy2;
	
	uint8_t		 _FNTbaselineLow, 	  _FNTbaselineTop;
	
 	// GFX Font support
	const GFXfont *gfxFont = nullptr;
	int8_t _gfxFont_min_yOffset = 0;
	
	// Opaque font chracter overlap?
	unsigned int _gfx_c_last;
	int16_t   _gfx_last__cursorX, _gfx_last__cursorY;
	int16_t	 _gfx_last_char_x_write = 0;
	uint16_t _gfx_last_char_textcolor;
	uint16_t _gfx_last_char_textbgcolor;
	bool gfxFontLastCharPosFG(int16_t x, int16_t y);

	void 		_textPosition(int16_t x, int16_t y,bool update);
	void 		_setFNTdimensions(uint8_t index);
	int16_t 	_STRlen_helper(const char* buffer,uint16_t len=0);

	/* Set ActiveWindow functions to match RA8875 */
	void setActiveWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB);
	void setActiveWindow(void);
	void _updateActiveWindow(bool full);

	#if defined(USE_FT5206_TOUCH)
	bool					_useISR;
	uint8_t				    _maxTouch;//5 on FT5206, 1 on resistive
	uint8_t					_intCTSPin;
	uint8_t					_rstCTSPin;
	uint8_t 				_cptRegisters[31];
	uint8_t					_gesture;
	uint8_t					_currentTouches;//0...5
	uint8_t					_currentTouchState;//0,1,2
	void 					_initializeFT5206(void);
	void 					_sendRegFT5206(uint8_t reg,const uint8_t val);
	void 					_disableCapISR(void);
	volatile boolean	  	_needCTS_ISRrearm;
	static void 			cts_isr(void);
	TwoWire 				 *_wire=&Wire;
	#endif	


};

#endif
