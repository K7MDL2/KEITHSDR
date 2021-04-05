//
//  Display.cpp
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"
//#include "Display.h"

#ifdef USE_RA8875
	extern RA8875 tft;
#else 
	extern RA8876_t3 tft;
#endif

#ifdef I2C_LCD
  #include <LiquidCrystal_I2C.h>
  extern LiquidCrystal_I2C lcd;
#endif

#ifdef ENET
  extern uint8_t enet_ready;
#endif

extern uint8_t display_state;   // something to hold the button state for the display pop-up window later.
extern uint8_t curr_band;   // global tracks our current band setting.  
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint32_t VFOB;
extern uint8_t user_Profile;
extern struct Band_Memory bandmem[];
extern struct Filter_Settings filter[];
extern struct Standard_Button std_btn[];
extern struct Label labels[];
extern struct Frequency_Display disp_Freq[];
extern struct User_Settings user_settings[];
extern struct AGC agc_set[];
extern struct NB nb[];
extern struct Modes_List modeList[];
extern struct TuneSteps tstep[];

void _triangle_helper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled);

struct User_Settings *pTX = &user_settings[user_Profile];
struct Frequency_Display *pVAct  = &disp_Freq[0];     // pointer to Active VFO Digits record
struct Frequency_Display *pMAct  = &disp_Freq[1];     // pointer to Active VFO Label record
struct Frequency_Display *pVStby = &disp_Freq[2];     // pointer to Standby VFO Digits record
struct Frequency_Display *pMStby = &disp_Freq[3];     // pointer to Standby VFO Label record

uint8_t	_textMode = false;
uint8_t _portrait = false;
uint16_t _arcAngle_max = ARC_ANGLE_MAX;
uint8_t _arcAngle_offset = ARC_ANGLE_OFFSET;
uint8_t _angle_offset = ANGLE_OFFSET;
uint8_t _color_bpp = 16;
uint8_t _colorIndex = 0;

// Function Declarations
//-------------- COLOR CONVERSION -----------------------------------------------------------
	inline uint16_t Color565(uint8_t r,uint8_t g,uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }
	inline uint16_t Color24To565(int32_t color_) { return ((((color_ >> 16) & 0xFF) / 8) << 11) | ((((color_ >> 8) & 0xFF) / 4) << 5) | (((color_) &  0xFF) / 8);}
	inline uint16_t htmlTo565(int32_t color_) { return (uint16_t)(((color_ & 0xF80000) >> 8) | ((color_ & 0x00FC00) >> 5) | ((color_ & 0x0000F8) >> 3));}
	inline void 	Color565ToRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b){r = (((color & 0xF800) >> 11) * 527 + 23) >> 6; g = (((color & 0x07E0) >> 5) * 259 + 33) >> 6; b = ((color & 0x001F) * 527 + 23) >> 6;}

void displayFreq(void)
{ 
	// bx					// X - upper left corner anchor point
	// by					// Y - upper left corner anchor point
	// bw					// width of whole box
	// bh					// height of whole box
	// br					// radius of corners
    // ol_clr	  			// outline color
	// bg_clr    			// background color
	// vfoActive_clr  		// color of Active VFO numbers
	// vfoActive_LabelFont 	// size of Active VFO Label text
    // vfoActive_Font  		// size of Active VFO numbers
    // vfoStandby_clr  		// color of Standby VFO numbers
	// vfoStandby_LabelFont // size of Standby VFO label text
    // vfoStanby_Font  		// size of Standby VFO numbers
    // TX_clr    			// Color when active VFO is in transmit
	// padx					// horizonal padding from left side of box
	// pady					// vertical padding form top of box
	
	// Put a box around the VFO section (use BLACK to turn it off)
	//tft.drawRect(pVAct->bx-1, pVAct->by-1, pVAct->bw+2, pVAct->bh+pVStby->bh+4, pVAct->box_clr);
	tft.drawLine(0, pVAct->bh+pVStby->bh+12, pMAct->bx+pMAct->bw, pVAct->bh+pVStby->bh+12, RA8875_LIGHT_ORANGE);
	//tft.drawRect(0, 15, 192, 65, RA8875_LIGHT_ORANGE);
	
	// Draw Active VFO box and Label
	tft.fillRect(pVAct->bx, pVAct->by, pVAct->bw, pVAct->bh, pVAct->bg_clr);
	tft.drawRect(pVAct->bx, pVAct->by, pVAct->bw, pVAct->bh, pVAct->ol_clr);
	tft.fillRect(pMAct->bx, pMAct->by, pMAct->bw, pMAct->bh, pMAct->bg_clr);
	tft.drawRect(pMAct->bx, pMAct->by, pMAct->bw, pMAct->bh, pMAct->ol_clr);
	
	// Draw Standby VFO box and Label
	tft.fillRect(pVStby->bx, pVStby->by, pVStby->bw, pVStby->bh, pVStby->bg_clr);
	tft.drawRect(pVStby->bx, pVStby->by, pVStby->bw, pVStby->bh, pVStby->ol_clr);
	tft.fillRect(pMStby->bx, pMStby->by, pMStby->bw, pMStby->bh, pMStby->bg_clr);
	tft.drawRect(pMStby->bx, pMStby->by, pMStby->bw, pMStby->bh, pMStby->ol_clr);

	// Write the Active VFO
	tft.setFont(pMAct->txt_Font);
	tft.setCursor(pMAct->bx+pMAct->padx, pMAct->by+pMAct->pady);
	tft.setTextColor(pMAct->txt_clr);
	if (pTX->xmit && !bandmem[curr_band].split)
		tft.fillRect(pMAct->bx, pMAct->by, pMAct->bw, pMAct->bh, pMAct->TX_clr);
	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
	{
		tft.print("A");		
		tft.setFont(pVAct->txt_Font);
		tft.setTextColor(pVAct->txt_clr);
		tft.setCursor(pVAct->bx+pVAct->padx, pVAct->by+pVAct->pady);		
		tft.print(formatVFO(VFOA));
    #ifdef I2C_LCD
      lcd.setCursor(0,0);
      lcd.print(formatVFO(VFOA));
    #endif
	}
	else
	{
		tft.print("B");		
		tft.setFont(pVAct->txt_Font);
		tft.setTextColor(pVAct->txt_clr);
		tft.setCursor(pVAct->bx+pVAct->padx, pVAct->by+pVAct->pady);
		tft.print(formatVFO(VFOB));
    #ifdef I2C_LCD
      lcd.setCursor(0,0);
      lcd.print(formatVFO(VFOB));
    #endif
	}

	// Write the standby VFO
	tft.setFont(pMStby->txt_Font);
	tft.setCursor(pMStby->bx+pMStby->padx, pMStby->by+pMStby->pady);
	tft.setTextColor(pMStby->txt_clr);	
	if (pTX->xmit && bandmem[curr_band].split)
		tft.fillRect(pMStby->bx, pMStby->by, pMStby->bw, pMStby->bh, pMStby->TX_clr);
	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
	{		
		tft.print("B");
		tft.setFont(pVStby->txt_Font);
		tft.setTextColor(pVStby->txt_clr);
		tft.setCursor(pVStby->bx+pVStby->padx, pVStby->by+pVStby->pady);
		tft.print(formatVFO(VFOB));
	}
	else 
	{
		tft.print("A");
		tft.setFont(pVStby->txt_Font);
		tft.setTextColor(pVStby->txt_clr);
		tft.setCursor(pVStby->bx+pVStby->padx, pVStby->by+pVStby->pady);
		tft.print(formatVFO(VFOA));
	}
}

void displayMode(void)
{
	uint8_t mode;

	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
		mode = bandmem[curr_band].mode_A;
	else 
		mode = bandmem[curr_band].mode_B;
	//sprintf(std_btn[MODE_BTN].label, "%s", Mode[mode]);
	sprintf(labels[MODE_LBL].label, "%s", modeList[mode].mode_label);
	drawLabel(MODE_LBL, &mode);
	draw_2_state_Button(MODE_BTN, &mode);
}

void displayFilter(void)
{
	char str[15];

	sprintf(str, "F: %s%s", filter[bandmem[curr_band].filter].Filter_name,filter[bandmem[curr_band].filter].units);	
	Serial.print("Tune Rate is "); Serial.println(str);	
	sprintf(labels[FILTER_LBL].label, "%s", str);
	drawLabel(FILTER_LBL, &bandmem[curr_band].filter);
	draw_2_state_Button(FILTER_BTN, &bandmem[curr_band].filter);
}

void displayRate(void)
{	
	if (bandmem[curr_band].tune_step >= TS_STEPS)
		bandmem[curr_band].tune_step = TS_STEPS-1;
	sprintf(labels[RATE_LBL].label, "R: %s%s", tstep[bandmem[curr_band].tune_step].ts_name, tstep[bandmem[curr_band].tune_step].ts_units);;
	Serial.print("Tune Rate is "); Serial.println(labels[RATE_LBL].label);
	drawLabel(RATE_LBL, &bandmem[curr_band].tune_step);
	draw_2_state_Button(RATE_BTN, &bandmem[curr_band].tune_step);
}

void displayAgc(void)
{	
	sprintf(std_btn[AGC_BTN].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
	sprintf(labels[AGC_LBL].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
	Serial.print("AGC ="); Serial.println(std_btn[AGC_BTN].label);
	drawLabel(AGC_LBL, &bandmem[curr_band].agc_mode);
	draw_2_state_Button(AGC_BTN, &bandmem[curr_band].agc_mode);
}

void displayANT(void)
{	
	sprintf(std_btn[ANT_BTN].label, "%s%1d", "ANT", bandmem[curr_band].ant_sw);
	sprintf(labels[ANT_LBL].label, "%s%1d", "ANT", bandmem[curr_band].ant_sw);
	Serial.print("Antenna Switch set to "); Serial.println(std_btn[ANT_BTN].label);
	drawLabel(ANT_LBL, &bandmem[curr_band].ant_sw);
	draw_2_state_Button(ANT_BTN, &bandmem[curr_band].ant_sw);
}

void displayRFgain(void)
{	
	sprintf(std_btn[RFGAIN_BTN].label, "%s%3d", "RF:", user_settings[user_Profile].rfGain);
	//sprintf(labels[RFGAIN_LBL].label, "%s%3d", "RF:", user_settings[user_Profile].rfGain);
	//drawLabel(RFGAIN_LBL, &user_settings[user_Profile].rfGain);
	Serial.print("RF Gain set to "); Serial.println(std_btn[RFGAIN_BTN].label);
	draw_2_state_Button(RFGAIN_BTN, &user_settings[user_Profile].rfGain_en);
  #ifdef I2C_LCD
    lcd.setCursor(10,1);
    lcd.print(std_btn[RFGAIN_BTN].label);
  #endif
}

void displayAFgain(void)
{	
	sprintf(std_btn[AFGAIN_BTN].label, "%s%3d", "AF:", user_settings[user_Profile].afGain);
	//sprintf(labels[AFGAIN_LBL].label, "%s%3d", "AF:", user_settings[user_Profile].afGain);
	//drawLabel(AFGAIN_LBL, &user_settings[user_Profile].afGain);
	Serial.print("AF Gain set to "); Serial.println(std_btn[AFGAIN_BTN].label);
	draw_2_state_Button(AFGAIN_BTN, &user_settings[user_Profile].afGain_en);
  #ifdef I2C_LCD  
    lcd.setCursor(0,1);
    lcd.print(std_btn[AFGAIN_BTN].label);
  #endif
}

void displayAttn()
{
	sprintf(std_btn[ATTEN_BTN].label, "%s%3d", "ATT:", bandmem[curr_band].attenuator_dB);
	Serial.print("Atten is "); Serial.println(bandmem[curr_band].attenuator);
	drawLabel(ATTEN_LBL, &bandmem[curr_band].attenuator);
	draw_2_state_Button(ATTEN_BTN, &bandmem[curr_band].attenuator);
}

void displayPreamp()
{
	Serial.print("Preamp is "); Serial.println(bandmem[curr_band].preamp);
	drawLabel(PREAMP_LBL, &bandmem[curr_band].preamp);
	draw_2_state_Button(PREAMP_BTN, &bandmem[curr_band].preamp);
}

void displayATU()
{
	Serial.print("ATU is "); Serial.println(bandmem[curr_band].ATU);
	drawLabel(ATU_LBL, &bandmem[curr_band].ATU);
	draw_2_state_Button(ATU_BTN, &bandmem[curr_band].ATU);
}

void displayRIT()
{
	Serial.print("RIT is "); Serial.println(bandmem[curr_band].RIT_en);
	drawLabel(RIT_LBL, &bandmem[curr_band].RIT_en);
	draw_2_state_Button(RIT_BTN, &bandmem[curr_band].RIT_en);
}

void displayXIT()
{
	Serial.print("XIT is "); Serial.println(bandmem[curr_band].XIT_en);
	drawLabel(XIT_LBL, &bandmem[curr_band].XIT_en);
	draw_2_state_Button(XIT_BTN, &bandmem[curr_band].XIT_en);
}

void displayFine()
{
	Serial.print("Fine Tune is "); Serial.println(user_settings[user_Profile].fine);
	drawLabel(FINE_LBL, &user_settings[user_Profile].fine);
	draw_2_state_Button(FINE_BTN,  &user_settings[user_Profile].fine);
}

void displayNB()
{
	sprintf(std_btn[NB_BTN].label, "NB-%s", nb[user_settings[user_Profile].nb_level].nb_name);
    sprintf(labels[NB_LBL].label,  "NB-%s", nb[user_settings[user_Profile].nb_level].nb_name);
	Serial.print("NB is "); Serial.print(user_settings[user_Profile].nb_en);
	Serial.print("   NB Level is "); Serial.println(user_settings[user_Profile].nb_level);
	drawLabel(NB_LBL, &user_settings[user_Profile].nb_en);
	draw_2_state_Button(NB_BTN, &user_settings[user_Profile].nb_en);
}

void displayNR()
{
	Serial.print("NR is "); Serial.println(user_settings[user_Profile].nr_en);
	drawLabel(NR_LBL, &user_settings[user_Profile].nr_en);
	draw_2_state_Button(NR_BTN, &user_settings[user_Profile].nr_en);
}

void displayNotch()
{
	Serial.print("Notch is "); Serial.println(std_btn[NOTCH_BTN].label);
	drawLabel(NOTCH_LBL, &user_settings[user_Profile].notch);
	draw_2_state_Button(NOTCH_BTN,  &user_settings[user_Profile].notch);
}

void displaySplit()
{
	char sp_label[15];

	if (bandmem[curr_band].split)
	{
		tft.setTextColor(RA8875_GREEN);
		sprintf(sp_label, "%s %s",  std_btn[SPLIT_BTN].label, ">>>");
		sprintf(labels[SPLIT_LBL].label, "%s",  sp_label);
	}
	else
	{
		tft.setTextColor(myDARKGREY);
		sprintf(sp_label, "%s %s", std_btn[SPLIT_BTN].label, "Off");
		sprintf(labels[SPLIT_LBL].label, "%s",  sp_label);
	}
	Serial.print("Split is "); Serial.println(bandmem[curr_band].split);
	drawLabel(SPLIT_LBL, &bandmem[curr_band].split);
	draw_2_state_Button(SPLIT_BTN, &bandmem[curr_band].split);
}

void displayTime(void)
{
	sprintf(std_btn[UTCTIME_BTN].label, "UTC:%02d:%02d:%02d", hour(), minute(), second());
	//tft.print(std_btn[UTCTIME_BTN].label);
	//Serial.println("UTC Time = "); Serial.println(std_btn[UTCTIME_BTN].label);
	draw_2_state_Button(UTCTIME_BTN, &std_btn[UTCTIME_BTN].show);	
}

// These buttons have no associated labels so are simply button updates
void displayMenu() 		{draw_2_state_Button(MENU_BTN, &std_btn[MENU_BTN].enabled);				}
void displayFn() 		{draw_2_state_Button(FN_BTN, &std_btn[FN_BTN].enabled);					}
void displayVFO_AB() 	{draw_2_state_Button(VFO_AB_BTN, &bandmem[curr_band].VFO_AB_Active);	}
void displayBandUp() 	{draw_2_state_Button(BANDUP_BTN, &bandmem[curr_band].band_num);			}
void displayBand() 		{draw_2_state_Button(BAND_BTN, &bandmem[curr_band].band_num);			}
void displaySpot() 		{draw_2_state_Button(SPOT_BTN,  &user_settings[user_Profile].spot);		}
void displayBandDn()	{draw_2_state_Button(BANDDN_BTN, &bandmem[curr_band].band_num);			}
void displayDisplay()	{draw_2_state_Button(DISPLAY_BTN, &display_state);						}
void displayXMIT()		{draw_2_state_Button(XMIT_BTN, &user_settings[user_Profile].xmit);		}
void displayMute()		{draw_2_state_Button(MUTE_BTN, &user_settings[user_Profile].mute);		}
void displayXVTR()		{draw_2_state_Button(XVTR_BTN, &bandmem[curr_band].xvtr_en);			}
void displayEnet()		{draw_2_state_Button(ENET_BTN, &user_settings[user_Profile].enet_output); }
void displayRefLevel()  {draw_2_state_Button(REFLVL_BTN, &std_btn[REFLVL_BTN].enabled); 		}

//
//------------------------------------  drawButton ------------------------------------------------------------------------
//
//  Input:  1. Button ID
//          2. pointer to structure where state is stored
//
//  Usage:  The structure table has teh button properties.  Just pass the index and state.
//
//  Notes:  Default is to handle 2 states today. However, the control function can change the buttom text
//			and set a value in the button's enabled field to track states.
//
void draw_2_state_Button(uint8_t button, uint8_t *function_ptr) 
{
    struct Standard_Button *ptr = std_btn + button;     // pointer to button object passed by calling function
	
	if(ptr->show)
	{
		#ifdef USE_RA8875
		if(*function_ptr > 0)
			tft.fillRoundRect(ptr->bx, ptr->by, ptr->bw, ptr->bh, ptr->br, ptr->on_color);		
		else  //(*function_ptr == 0)		
			tft.fillRoundRect(ptr->bx, ptr->by, ptr->bw, ptr->bh, ptr->br, ptr->off_color );					
		tft.drawRoundRect(ptr->bx, ptr->by, ptr->bw, ptr->bh, ptr->br, ptr->outline_color);
		#else
		if(*function_ptr > 0)
			tft.fillRoundRect(ptr->bx, ptr->by, ptr->bw, ptr->bh, ptr->br, ptr->br, ptr->on_color);		
		else  //(*function_ptr == 0)		
			tft.fillRoundRect(ptr->bx, ptr->by, ptr->bw, ptr->bh, ptr->br, ptr->br, ptr->off_color );					
		tft.drawRoundRect(ptr->bx, ptr->by, ptr->bw, ptr->bh, ptr->br, ptr->br, ptr->outline_color);
		#endif
		tft.setTextColor(ptr->txtclr);
		if (button == UTCTIME_BTN)
			tft.setFont(Arial_16);
		else
			tft.setFont(Arial_18);
		tft.setTextColor(ptr->txtclr); 
		tft.setCursor(ptr->bx+ptr->padx, ptr->by+ptr->pady);
		tft.print(ptr->label);
	}
}

void drawLabel(uint8_t lbl_num, uint8_t *function_ptr)
{
	struct Label *plabel = labels + lbl_num;

	if (plabel->show)
	{
		#ifdef USE_RA8875
			if(*function_ptr > 0)
			{
				tft.fillRoundRect(plabel->x, plabel->y, plabel->w, plabel->h, plabel->r, plabel->on_color);
				tft.setTextColor(plabel->on_txtclr); 
			}
			else
			{
				tft.fillRoundRect(plabel->x, plabel->y, plabel->w, plabel->h, plabel->r, plabel->off_color);	
				tft.setTextColor(plabel->off_txtclr); 
			}
			tft.drawRoundRect(plabel->x,plabel->y, plabel->w, plabel->h, plabel->r, plabel->outline_color);	
		#else   // must be RA8876_t3
			if(*function_ptr > 0)
			{
				tft.fillRoundRect(plabel->x, plabel->y, plabel->w, plabel->h, plabel->r, plabel->r, plabel->on_color);
				tft.setTextColor(plabel->on_txtclr); 
			}
			else
			{
				tft.fillRoundRect(plabel->x, plabel->y, plabel->w, plabel->h, plabel->r, plabel->r, plabel->off_color);	
				tft.setTextColor(plabel->off_txtclr); 
			}
			tft.drawRoundRect(plabel->x,plabel->y, plabel->w, plabel->h, plabel->r, plabel->r, plabel->outline_color);	
		#endif
		
		tft.setFont(Arial_14);
		tft.setCursor(plabel->x+plabel->padx, plabel->y+plabel->pady);
		tft.print(plabel->label);
	}
	return;
}

//
//    formatVFO()
//
const char* formatVFO(uint32_t vfo)
{
	static char vfo_str[25];
	
	uint16_t MHz = (vfo/1000000 % 1000000);
	uint16_t Hz  = (vfo % 1000);
	uint16_t KHz = ((vfo % 1000000) - Hz)/1000;
	sprintf(vfo_str, "%6d.%03d.%03d", MHz, KHz, Hz);
	//sprintf(vfo_str, "%13s", "45.123.123");
	//Serial.print("New VFO: ");Serial.println(vfo_str);
	return vfo_str;
}

//
//----------------------------------- Refresh screen -----------------------------------
//
//  Usage: This function calls all of the displayXXX() functions to easily refresh the
//			screen except for the spectrum display module.
// 
void displayRefresh(void)
{
	// Bottom Panel Anchor button
	displayFn();   // make fn=1 to call displayFn() to prevent calling itself
    displayFreq();    // display frequency
	#ifdef ENET
	if (enet_ready)
		displayTime();
	#endif
	// Panel 1 buttons
	displayMode();
	displayFilter();
	displayAttn();
	displayPreamp();
	displayRate();
	displayBand();
	//Panel 2 buttons
	displayNB();
	displayNR();
	displaySpot();
	displayNotch();
	displayAgc();
	displayMute();
	//Panel 3 buttons
	displayMenu();
	displayANT();
	displayATU();
	displayXMIT();
	displayBandUp();
	displayBandDn();
	//Panel 4 buttons
	displayRIT();
	displayXIT();
	displayVFO_AB();
	displayFine();
	displayDisplay();
	displaySplit();
	//Panel 5 buttons
	displayRFgain();
	displayAFgain();
	displayEnet();
	displayXVTR();
	displayRefLevel();
}

#ifndef USE_RA8875
/**************************************************************************/
/*!
	  calculate a grandient color
	  return a spectrum starting at blue to red (0...127)
*/
/**************************************************************************/
uint16_t grandient(uint8_t val)
{
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t q = val / 32;
	switch(q){
		case 0:
			r = 0; g = 2 * (val % 32); b = 31;
		break;
		case 1:
			r = 0; g = 63; b = 31 - (val % 32);
		break;
		case 2:
			r = val % 32; g = 63; b = 0;
		break;
		case 3:
			r = 31; g = 63 - 2 * (val % 32); b = 0;
		break;
	}
	return (r << 11) + (g << 5) + b;
}

/**************************************************************************/
/*!
	  interpolate 2 r,g,b colors
	  return a 16bit mixed color between the two
	  Parameters:
	  r1.
	  g1:
	  b1:
	  r2:
	  g2:
	  b2:
	  pos:0...div (mix percentage) (0:color1, div:color2)
	  div:divisions between color1 and color 2
*/
/**************************************************************************/
uint16_t colorInterpolation(uint8_t r1,uint8_t g1,uint8_t b1,uint8_t r2,uint8_t g2,uint8_t b2,uint16_t pos,uint16_t div)
{
    if (pos == 0) return Color565(r1,g1,b1);
    if (pos >= div) return Color565(r2,g2,b2);
	float pos2 = (float)pos/div;
	return Color565(
				(uint8_t)(((1.0 - pos2) * r1) + (pos2 * r2)),
				(uint8_t)((1.0 - pos2) * g1 + (pos2 * g2)),
				(uint8_t)(((1.0 - pos2) * b1) + (pos2 * b2))
	);
}

#ifdef LATER1
/**************************************************************************/
/*!
      ringMeter 
	  (adapted from Alan Senior (thanks man!))
	  it create a ring meter with a lot of personalizations,
	  it return the width of the gauge so you can use this value
	  for positioning other gauges near the one just created easily
	  Parameters:
	  val:  your value
	  minV: the minimum value possible
	  maxV: the max value possible
	  x:    the position on x axis
	  y:    the position on y axis
	  r:    the radius of the gauge (minimum 50)
	  units: a text that shows the units, if "none" all text will be avoided
	  scheme:0...7 or 16 bit color (not BLACK or WHITE)
	  0:red
	  1:green
	  2:blue
	  3:blue->red
	  4:green->red
	  5:red->green
	  6:red->green->blue
	  7:cyan->green->red
	  8:black->white linear interpolation
	  9:violet->yellow linear interpolation
	  or
      RGB565 color (not BLACK or WHITE)
	  backSegColor: the color of the segments not active (default BLACK)
	  angle:		90 -> 180 (the shape of the meter, 90:halfway, 180:full round, 150:default)
	  inc: 			5...20 (5:solid, 20:sparse divisions, default:10)
*/
/**************************************************************************/
void ringMeter(int val, int minV, int maxV, int16_t x, int16_t y, uint16_t r, const char* units, uint16_t colorScheme,uint16_t backSegColor,int16_t angle,uint8_t inc)
{
	if (inc < 5) inc = 5;
	if (inc > 20) inc = 20;
	if (r < 50) r = 50;
	if (angle < 90) angle = 90;
	if (angle > 180) angle = 180;
	int curAngle = map(val, minV, maxV, -angle, angle);
	uint16_t colour;
	x += r;
	y += r;   // Calculate coords of centre of ring
	uint16_t w = r / 4;    // Width of outer ring is 1/4 of radius
	const uint8_t seg = 5; // Segments are 5 degrees wide = 60 segments for 300 degrees
	// Draw colour blocks every inc degrees
	for (int16_t i = -angle; i < angle; i += inc) {
		colour = RA8875_BLACK;
		switch (colorScheme) {
			case 0:
				colour = RA8875_RED;
				break; // Fixed colour
			case 1:
				colour = RA8875_GREEN;
				break; // Fixed colour
			case 2:
				colour = RA8875_BLUE;
				break; // Fixed colour
			case 3:
				colour = grandient(map(i, -angle, angle, 0, 127));
				break; // Full spectrum blue to red
			case 4:
				colour = grandient(map(i, -angle, angle, 63, 127));
				break; // Green to red (high temperature etc)
			case 5:
				colour = grandient(map(i, -angle, angle, 127, 63));
				break; // Red to green (low battery etc)
			case 6:
				colour = grandient(map(i, -angle, angle, 127, 0));
				break; // Red to blue (air cond reverse)
			case 7:
				colour = grandient(map(i, -angle, angle, 35, 127));
				break; // cyan to red 
			case 8:
				colour = colorInterpolation(0,0,0,255,255,255,map(i,-angle,angle,0,w),w);
				break; // black to white
			case 9:
				colour = colorInterpolation(0x80,0,0xC0,0xFF,0xFF,0,map(i,-angle,angle,0,w),w);
				break; // violet to yellow
			default:
				if (colorScheme > 9){
					colour = colorScheme;
				} else {
					colour = RA8875_BLUE;
				}
				break; // Fixed colour
		}
		// Calculate pair of coordinates for segment start
		float xStart = cos((i - 90) * 0.0174532925);
		float yStart = sin((i - 90) * 0.0174532925);
		uint16_t x0 = xStart * (r - w) + x;
		uint16_t y0 = yStart * (r - w) + y;
		uint16_t x1 = xStart * r + x;
		uint16_t y1 = yStart * r + y;

		// Calculate pair of coordinates for segment end
		float xEnd = cos((i + seg - 90) * 0.0174532925);
		float yEnd = sin((i + seg - 90) * 0.0174532925);
		int16_t x2 = xEnd * (r - w) + x;
		int16_t y2 = yEnd * (r - w) + y;
		int16_t x3 = xEnd * r + x;
		int16_t y3 = yEnd * r + y;

		if (i < curAngle) { // Fill in coloured segments with 2 triangles
			fillQuad(x0, y0, x1, y1, x2, y2, x3, y3, colour, false);
		} else {// Fill in blank segments
			fillQuad(x0, y0, x1, y1, x2, y2, x3, y3, backSegColor, false);
		}
	}

/**************************************************************************/
/*!
      Draw filled circle
	  Parameters:
      x0: The 0-based x location of the center of the circle
      y0: The 0-based y location of the center of the circle
      r: radius
      color: RGB565 color
*/
/**************************************************************************/
/*
void RA8875::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	_center_helper(x0,y0);
	if (r <= 0) return;
	_circle_helper(x0, y0, r, color, true);
}
*/
		
	//inline __attribute__((always_inline)) 	
	void _center_helper(int16_t &x, int16_t &y)
		__attribute__((always_inline)) {
			if (x == CENTER) x = _width/2;
			if (y == CENTER) y = _height/2;
	}

void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	_center_helper(x0,y0);
	if (r < 1) return;
	if (r == 1) {
		drawPixel(x0,y0,color);
		return;
	}
	_circle_helper(x0, y0, r, color, true);
}

/**************************************************************************/
/*!
      Draw a quadrilater by connecting 4 points
	  Parameters:
	  x0:
	  y0:
	  x1:
	  y1:
	  x2:
	  y2:
	  x3:
	  y3:
      color: RGB565 color
*/
/**************************************************************************/
void drawQuad(int16_t x0, int16_t y0,int16_t x1, int16_t y1,int16_t x2, int16_t y2,int16_t x3, int16_t y3, uint16_t color) 
{
	drawLine(x0, y0, x1, y1, color);//low 1
	drawLine(x1, y1, x2, y2, color);//high 1
	drawLine(x2, y2, x3, y3, color);//high 2
	drawLine(x3, y3, x0, y0, color);//low 2
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+					GEOMETRIC PRIMITIVE HELPERS STUFF								 +
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/**************************************************************************/
/*!
      helper function for circles
	  [private]
*/
/**************************************************************************/
void _circle_helper(int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled)//0.69b32 fixed an undocumented hardware limit
{
	if (_portrait) swapvals(x0,y0);//0.69b21

	if (r < 1) r = 1;
	if (r < 2) {//NEW
		drawPixel(x0,y0,color);
		return;
	}
	if (r > RA8876_HEIGHT / 2) r = (RA8876_HEIGHT / 2) - 1;//this is the (undocumented) hardware limit of RA8875
	
	if (_textMode) _setTextMode(false);//we are in text mode?
	#if defined(USE_RA8876_SEPARATE_TEXT_COLOR)
		_TXTrecoverColor = true;
	#endif
	if (color != _foreColor) setForegroundColor(color);//0.69b30 avoid several SPI calls
	
	_writeRegister(RA8876_DCHR0,    x0 & 0xFF);
	_writeRegister(RA8876_DCHR0 + 1,x0 >> 8);

	_writeRegister(RA8876_DCVR0,    y0 & 0xFF);
	_writeRegister(RA8876_DCVR0 + 1,y0 >> 8);	   
	_writeRegister(RA8876_DCRR,r); 

	writeCommand(RA8876_DCR);
	#if defined(_FASTCPU)
		_slowDownSPI(true);
	#endif
	filled == true ? _writeData(RA8876_DCR_CIRCLE_START | RA8876_DCR_FILL) : _writeData(RA8876_DCR_CIRCLE_START | RA8876_DCR_NOFILL);
	_waitPoll(RA8876_DCR, RA8876_DCR_CIRCLE_STATUS, _RA8876_WAITPOLL_TIMEOUT_DCR_CIRCLE_STATUS);//ZzZzz
	#if defined(_FASTCPU)
		_slowDownSPI(false);
	#endif
}


/**************************************************************************/
/*!
		helper function for rects (filled or not)
		[private]
*/
/**************************************************************************/
/*
void RA8875::_rect_helper(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled)
{
	if (w < 0 || h < 0) return;//why draw invisible rects?(MrTOM temp fix)
	if (w >= _width) return;
	if (h >= _height) return;
	
	if (_portrait) {swapvals(x,y); swapvals(w,h);}

	_checkLimits_helper(x,y);

	if (_textMode) _setTextMode(false);//we are in text mode?
	#if defined(USE_RA8875_SEPARATE_TEXT_COLOR)
		_TXTrecoverColor = true;
	#endif
	if (color != _foreColor) setForegroundColor(color);
	
	_line_addressing(x,y,w,h);

	writeCommand(RA8875_DCR);
	filled == true ? _writeData(0xB0) : _writeData(0x90);
	_waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}
*/

void _rect_helper(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled)
{
	if (_portrait) {swapvals(x1,y1); swapvals(x2,y2);}
	if ((x1 < 0 && x2 < 0) || (x1 >= RA8875_WIDTH && x2 >= RA8875_WIDTH) ||
	    (y1 < 0 && y2 < 0) || (y1 >= RA8875_HEIGHT && y2 >= RA8875_HEIGHT))
		return;	// All points are out of bounds, don't draw anything

	_checkLimits_helper(x1,y1);	// Truncate rectangle that is off screen, still draw remaining rectangle
	_checkLimits_helper(x2,y2);

	if (_textMode) _setTextMode(false);	//we are in text mode?
	#if defined(USE_RA8875_SEPARATE_TEXT_COLOR)
		_TXTrecoverColor = true;
	#endif
	if (color != _foreColor) setForegroundColor(color);
	
	if (x1==x2 && y1==y2)		// Width & height can still be 1 pixel, so render as a pixel
		drawPixel(x1,y1,color);
	else {
		_line_addressing(x1,y1,x2,y2);

		writeCommand(RA8876_DCR);
		filled == true ? _writeData(0xB0) : _writeData(0x90);
		_waitPoll(RA8876_DCR, RA8876_DCR_LINESQUTRI_STATUS, _RA8876_WAITPOLL_TIMEOUT_DCR_LINESQUTRI_STATUS);
	}
}


/**************************************************************************/
/*!
      helper function for triangles
	  [private]
*/
/**************************************************************************/
void _triangle_helper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled)
{
	if (x0 >= _width || x1 >= _width || x2 >= _width) return;
	if (y0 >= _height || y1 >= _height || y2 >= _height) return;
	
	if (_portrait) {swapvals(x0,y0); swapvals(x1,y1); swapvals(x2,y2);}
	/*
	if (x0 == x1 && y0 == y1){
		drawLine(x0, y0, x2, y2,color);
		return;
	} else if (x0 == x2 && y0 == y2){
		drawLine(x0, y0, x1, y1,color);
		return;
	} else if (x0 == x1 && y0 == y1 && x0 == x2 && y0 == y2) {//new
        drawPixel(x0, y0, color);
		return;
	}
	*/
	/*
	if (y0 > y1) {swapvals(y0, y1); swapvals(x0, x1);}			// Sort points from Y < to >
	if (y1 > y2) {swapvals(y2, y1); swapvals(x2, x1);}
	if (y0 > y1) {swapvals(y0, y1); swapvals(x0, x1);}
	*/
/*	
	if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
		int16_t a, b;
        a = b = x0;
		if (x1 < a) {     
			a = x1;
		} else if (x1 > b) {
			b = x1;
		}
        if (x2 < a) { 
			a = x2;
		} else if (x2 > b) {
			b = x2;
		}
        drawFastHLine(a, y0, b-a+1, color);
        return;
    }
*/	

	// Avoid drawing lines here due to hardware bug in certain circumstances when a
	// specific shape triangle is drawn after a line. This bug can still happen, but
	// at least the user has control over fixing it.
	// Not drawing a line here is slower, but drawing a non-filled "triangle" is
	// slightly faster than a filled "triangle".
	//
	// bug example: tft.drawLine(799,479, 750,50, RA8875_BLUE)
	//              tft.fillTriangle(480,152, 456,212, 215,410, RA8875_GREEN)
	// MrTom
	//
	if (x0 == x1 && y0 == y1 && x0 == x2 && y0 == y2) {			// All points are same
		drawPixel(x0,y0, color);
		return;
	} else if ((x0 == x1 && y0 == y1) || (x0 == x2 && y0 == y2) || (x1 == x2 && y1 == y2)){
		filled = false;									// Two points are same
	} else if (x0 == x1 && x0 == x2){
		filled = false;									// Vertical line
	} else if (y0 == y1 && y0 == y2){
		filled = false;									// Horizontal line
	}
	if (filled){
		if (_check_area(x0,y0, x1,y1, x2,y2) < 0.9) {
			filled = false;			// Draw non-filled triangle to avoid filled triangle bug when two vertices are close together.
		}
	}

	if (_textMode) _setTextMode(false);//we are in text mode?
	
	#if defined(USE_RA8876_SEPARATE_TEXT_COLOR)
		_TXTrecoverColor = true;
	#endif
	if (color != _foreColor) setForegroundColor(color);//0.69b30 avoid several SPI calls
	
	//_checkLimits_helper(x0,y0);
	//_checkLimits_helper(x1,y1);
	
	_line_addressing(x0,y0,x1,y1);
	//p2

	_writeRegister(RA8876_DTPH0,    x2 & 0xFF);
	_writeRegister(RA8876_DTPH0 + 1,x2 >> 8);
	_writeRegister(RA8876_DTPV0,    y2 & 0xFF);
	_writeRegister(RA8876_DTPV0 + 1,y2 >> 8);
	
	writeCommand(RA8876_DCR);
	filled == true ? _writeData(0xA1) : _writeData(0x81);
	
	_waitPoll(RA8876_DCR, RA8876_DCR_LINESQUTRI_STATUS, _RA8876_WAITPOLL_TIMEOUT_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!
		Graphic line addressing helper
		[private]
*/
/**************************************************************************/
void _line_addressing(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	//X0
	_writeRegister(RA8876_DLHSR0,    x0 & 0xFF);
	_writeRegister(RA8876_DLHSR0 + 1,x0 >> 8);
	//Y0
	_writeRegister(RA8876_DLVSR0,    y0 & 0xFF);
	_writeRegister(RA8876_DLVSR0 + 1,y0 >> 8);
	//X1
	_writeRegister(RA8876_DLHER0,    x1 & 0xFF);
	_writeRegister(RA8876_DLHER0 + 1,x1 >> 8);
	//Y1
	_writeRegister(RA8876_DLVER0,    y1 & 0xFF);
	_writeRegister(RA8876_DLVER0 + 1,y1 >> 8);
}

/**************************************************************************/
/*!	
		curve addressing helper
		[private]
*/
/**************************************************************************/
void _curve_addressing(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	//center
	_writeRegister(RA8876_DEHR0,    x0 & 0xFF);
	_writeRegister(RA8876_DEHR0 + 1,x0 >> 8);
	_writeRegister(RA8876_DEVR0,    y0 & 0xFF);
	_writeRegister(RA8876_DEVR0 + 1,y0 >> 8);
	//long,short ax
	_writeRegister(RA8876_ELL_A0,    x1 & 0xFF);
	_writeRegister(RA8876_ELL_A0 + 1,x1 >> 8);
	_writeRegister(RA8876_ELL_B0,    y1 & 0xFF);
	_writeRegister(RA8876_ELL_B0 + 1,y1 >> 8);
}

/**************************************************************************/
/*!	
		sin e cos helpers
		[private]
*/
/**************************************************************************/
float _cosDeg_helper(float angle)
{
	float radians = angle / (float)360 * 2 * PI;
	return cos(radians);
}

float _sinDeg_helper(float angle)
{
	float radians = angle / (float)360 * 2 * PI;
	return sin(radians);
}

/**************************************************************************/
/*!	
		change the arc default parameters
*/
/**************************************************************************/
void setArcParams(float arcAngleMax, int arcAngleOffset)
{
	_arcAngle_max = arcAngleMax;
	_arcAngle_offset = arcAngleOffset;
}

/**************************************************************************/
/*!	
		change the angle offset parameter from default one
*/
/**************************************************************************/
void setAngleOffset(int16_t angleOffset)
{
	_angle_offset = ANGLE_OFFSET + angleOffset;
}

/**************************************************************************/
/*! PRIVATE
		Write in a register
		Parameters:
		reg: the register
		val: the data
*/
/**************************************************************************/
void _writeRegister(const uint8_t reg, uint8_t val) 
{
	writeCommand(reg);
	_writeData(val);
}


/**************************************************************************/
/*!
		Write data
		Parameters:
		d: the data
*/
/**************************************************************************/
void _writeData(uint8_t data) 
{
	startSend();
	#if defined(___DUESTUFF) && defined(SPI_DUE_MODE_EXTENDED)
		SPI.transfer(_cs, RA8875_DATAWRITE, SPI_CONTINUE); 
		SPI.transfer(_cs, data, SPI_LAST);
	#else
		#if (defined(__AVR__) && defined(_FASTSSPORT)) || defined(SPARK)
			_spiwrite(RA8875_DATAWRITE);
			_spiwrite(data);
		#else
			#if defined(__MK64FX512__) || defined(__MK66FX1M0__)  || defined(__IMXRT1062__) || defined(__MKL26Z64__)
				_pspi->transfer(RA8876_DATAWRITE);
				_pspi->transfer(data);
			#else
				SPI.transfer(RA8875_DATAWRITE);
				SPI.transfer(data);
			#endif
		#endif
	#endif
	_endSend();
}

/**************************************************************************/
/*! PRIVATE
		Write a command
		Parameters:
		d: the command
*/
/**************************************************************************/
void writeCommand(const uint8_t d) 
{
	startSend();
	#if defined(___DUESTUFF) && defined(SPI_DUE_MODE_EXTENDED)
		SPI.transfer(_cs, RA8875_CMDWRITE, SPI_CONTINUE); 
		SPI.transfer(_cs, d, SPI_LAST);
	#else
		#if (defined(__AVR__) && defined(_FASTSSPORT)) || defined(SPARK)
			_spiwrite(RA8875_CMDWRITE);
			_spiwrite(d);
		#else
			#if defined(__MK64FX512__) || defined(__MK66FX1M0__)  || defined(__IMXRT1062__) || defined(__MKL26Z64__)
				_pspi->transfer(RA8875_CMDWRITE);
				_pspi->transfer(d);
			#else
				SPI.transfer(RA8875_CMDWRITE);
				SPI.transfer(d);
			#endif
		#endif
	#endif
	_endSend();
}

/**************************************************************************/
/*!
      Draw a filled quadrilater by connecting 4 points
	  Parameters:
	  x0:
	  y0:
	  x1:
	  y1:
	  x2:
	  y2:
	  x3:
	  y3:
      color: RGB565 color
	  triangled: if true a full quad will be generated, false generate a low res quad (faster)
	  *NOTE: a bug in _triangle_helper create some problem, still fixing....
*/
/**************************************************************************/
void fillQuad(int16_t x0, int16_t y0,int16_t x1, int16_t y1,int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint16_t color, bool triangled) 
{
	  _triangle_helper(x0, y0, x1, y1, x2, y2, color,true);
	  if (triangled) _triangle_helper(x2, y2, x3, y3, x0, y0, color,true);
      _triangle_helper(x1, y1, x2, y2, x3, y3, color,true);
}


/**************************************************************************/
/*!
      Fill the ActiveWindow by using a specified RGB565 color
	  Parameters:
	  color: RGB565 color (default=BLACK)
*/
/**************************************************************************/
void fillWindow(uint16_t color)
{  
	_line_addressing(0,0,RA8876_WIDTH-1, RA8876_HEIGHT-1);
	setForegroundColor(color);
	writeCommand(RA8876_DCR);
	_writeData(0xB0);
	_waitPoll(RA8876_DCR, RA8876_DCR_LINESQUTRI_STATUS, _RA887_WAITPOLL_TIMEOUT_DCR_LINESQUTRI_STATUS);
	#if defined(USE_RA8876_SEPARATE_TEXT_COLOR)
		_TXTrecoverColor = true;
	#endif
}

/**************************************************************************/
/*!
      clearScreen it's different from fillWindow because it doesn't depends
	  from the active window settings so it will clear all the screen.
	  It should be used only when needed since it's slower than fillWindow.
	  parameter:
	  color: 16bit color (default=BLACK)
*/
/**************************************************************************/
void clearScreen(uint16_t color)//0.69b24
{  
	setActiveWindow();
	fillWindow(color);
}

/**************************************************************************/
/*!
      Draw circle
	  Parameters:
      x0: The 0-based x location of the center of the circle
      y0: The 0-based y location of the center of the circle
      r: radius
      color: RGB565 color
*/
/**************************************************************************/
void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	_center_helper(x0,y0);
	if (r < 1) return;
	if (r < 2) {
		drawPixel(x0,y0,color);
		return;
	}
	_circle_helper(x0, y0, r, color, false);
}


/**************************************************************************/
/*!
      Draw filled circle
	  Parameters:
      x0: The 0-based x location of the center of the circle
      y0: The 0-based y location of the center of the circle
      r: radius
      color: RGB565 color
*/
/**************************************************************************/
/*
void RA8875::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	_center_helper(x0,y0);
	if (r <= 0) return;
	_circle_helper(x0, y0, r, color, true);
}
*/

void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	_center_helper(x0,y0);
	if (r < 1) return;
	if (r == 1) {
		drawPixel(x0,y0,color);
		return;
	}
	_circle_helper(x0, y0, r, color, true);
}

/**************************************************************************/
/*!
      Draw a filled curve
      Parameters:
      xCenter:]   x location of the ellipse center
      yCenter:   y location of the ellipse center
      longAxis:  Size in pixels of the long axis
      shortAxis: Size in pixels of the short axis
      curvePart: Curve to draw in clock-wise dir: 0[180-270�],1[270-0�],2[0-90�],3[90-180�]
      color: RGB565 color
*/
/**************************************************************************/
void fillCurve(int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color)
{
	curvePart = curvePart % 4; //limit to the range 0-3
	if (_portrait) {//fix a problem with rotation
		if (curvePart == 0) {
			curvePart = 2;
		} else if (curvePart == 2) {
			curvePart = 0;
		}
	}
	_ellipseCurve_helper(xCenter, yCenter, longAxis, shortAxis, curvePart, color, true);
}

#endif // LATER



#endif  // USE_RA8875

