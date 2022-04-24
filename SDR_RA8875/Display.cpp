//
//  Display.cpp
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"
#include "Display.h"
      
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

extern uint8_t 	display_state;   // something to hold the button state for the display pop-up window later.
extern uint8_t 	curr_band;   // global tracks our current band setting.  
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint32_t VFOB;
extern uint8_t 	user_Profile;
extern struct 	Band_Memory bandmem[];
extern struct 	Filter_Settings filter[];
extern struct 	Standard_Button std_btn[];
extern struct 	Label labels[];
extern struct 	Frequency_Display disp_Freq[];
extern struct 	User_Settings user_settings[];
extern struct 	Zoom_Lvl zoom[];
extern struct 	AGC agc_set[];
extern struct 	NB nb[];
extern struct 	Modes_List modeList[];
extern struct 	TuneSteps tstep[];
extern bool    	MeterInUse;  // S-meter flag to block updates while the MF knob has control
extern uint8_t 	MF_client;  // Flag for current owner of MF knob services
extern int32_t 	ModeOffset;

void ringMeter(int val, int minV, int maxV, int16_t x, int16_t y, uint16_t r, const char* units, uint16_t colorScheme,uint16_t backSegColor,int16_t angle,uint8_t inc);
void drawAlert(int x, int y , int side, boolean draw);
void setTextDatum(uint8_t d);
int drawCentreString(const char *string, int dX, int poY, int font);
void setTextPadding(uint16_t x_width);
int16_t textWidth(const char *string, int font);
int drawString(const char *string, int poX, int poY, int font);
unsigned int rainbow(byte value);
void _triangle_helper(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled);
void drawQuad(int16_t x0, int16_t y0,int16_t x1, int16_t y1,int16_t x2, int16_t y2,int16_t x3, int16_t y3, uint16_t color);
void fillQuad(int16_t x0, int16_t y0,int16_t x1, int16_t y1,int16_t x2, int16_t y2, int16_t x3, int16_t y3, uint16_t color, bool triangled) ;

struct User_Settings 	 *pTX    = &user_settings[user_Profile];
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

COLD void displayFreq(void)
{
	static uint8_t 	vfo_active_last = 255;  // use 255 since .VFO_AB_active is using 0 and 1 values
	static uint8_t 	vfo_changed = 0;
	static uint32_t vfo_a_last  = 0;
	static uint32_t vfo_b_last  = 0;
	static uint8_t 	xmit_last   = 0;
	
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

	// Draw the top orange separator line (under the VFO numbers)
	#ifdef USE_RA8875
		tft.drawFastHLine(10, pVAct->bh+pVStby->bh+12, pMAct->bx+pMAct->bw-10, RA8875_LIGHT_ORANGE); // for 8875
	#else
		tft.drawFastHLine(10, pVAct->bh+pVStby->bh+12, pMAct->bx+pMAct->bw+130, RA8875_LIGHT_ORANGE);  // For 8876
	#endif // USE_RA8875
	//tft.drawRect(0, 15, 792, 65, RA8875_LIGHT_ORANGE);  // test box

	// Detect if active VFO changed, or any VFO value, if any did, then draw the changed VFO standby section, else leave it alone to reduce flicker
	if (bandmem[curr_band].VFO_AB_Active != vfo_active_last)
	{
		vfo_active_last = bandmem[curr_band].VFO_AB_Active;  // track VFO swapping
		vfo_changed = 1;  // force stby vfo update
	}
	else
	{
		vfo_active_last = (bandmem[curr_band].VFO_AB_Active);  // Active VFO did not change
	}

	if (pTX->xmit != xmit_last)
	{
		vfo_changed = 1;
		xmit_last =pTX->xmit;
	}

	//Update VFO Markers
	if (vfo_changed)
	{
		// Update the Active VFO Marker
		if (pTX->xmit && !bandmem[curr_band].split)
			tft.fillRect(pMAct->bx, pMAct->by, pMAct->bw, pMAct->bh, pMAct->TX_clr);
		else	
			tft.fillRect(pMAct->bx, pMAct->by, pMAct->bw, pMAct->bh, pMAct->bg_clr);
		tft.drawRect(pMAct->bx, pMAct->by, pMAct->bw, pMAct->bh, pMAct->ol_clr);
		tft.setFont(pMAct->txt_Font);
		tft.setCursor(pMAct->bx+pMAct->padx, pMAct->by+pMAct->pady);
		tft.setTextColor(pMAct->txt_clr);
		if (bandmem[curr_band].VFO_AB_Active == VFO_A)
			tft.print("A");	
		else
			tft.print("B");

		// Update Stby VFO marker
		if (pTX->xmit && bandmem[curr_band].split)
			tft.fillRect(pMStby->bx, pMStby->by, pMStby->bw, pMStby->bh, pMStby->TX_clr);
		else
			tft.fillRect(pMStby->bx, pMStby->by, pMStby->bw, pMStby->bh, pMStby->bg_clr);
		tft.drawRect(pMStby->bx, pMStby->by, pMStby->bw, pMStby->bh, pMStby->ol_clr);
		tft.setFont(pMStby->txt_Font);
		tft.setCursor(pMStby->bx+pMStby->padx, pMStby->by+pMStby->pady);
		tft.setTextColor(pMStby->txt_clr);

		if (bandmem[curr_band].VFO_AB_Active == VFO_A)
			tft.print("B");	
		else
			tft.print("A");
	}

	// Update VFO only if they change	
	// Update the active VFO frequency (top line)
	if  ((bandmem[curr_band].VFO_AB_Active == VFO_A && vfo_a_last != VFOA) || 
		 (bandmem[curr_band].VFO_AB_Active == VFO_B && vfo_b_last != VFOB) ||
		vfo_changed)
	{
		tft.fillRect(pVAct->bx, pVAct->by, pVAct->bw, pVAct->bh, pVAct->bg_clr);
		tft.drawRect(pVAct->bx, pVAct->by, pVAct->bw, pVAct->bh, pVAct->ol_clr);
		tft.setFont(pVAct->txt_Font);
		tft.setCursor(pVAct->bx+pVAct->padx, pVAct->by+pVAct->pady);
		tft.setTextColor(pVAct->txt_clr);
		if (bandmem[curr_band].VFO_AB_Active == VFO_A)
		{
			tft.print(formatVFO(VFOA));
			#ifdef I2C_LCD
				lcd.setCursor(0,0);
				lcd.print(formatVFO(VFOA));
			#endif
		} 
		else
		{
			tft.print(formatVFO(VFOB));
			#ifdef I2C_LCD
				lcd.setCursor(0,0);
				lcd.print(formatVFO(VFOB));
			#endif
		}
	}
	
	if  ((bandmem[curr_band].VFO_AB_Active == VFO_B && vfo_a_last != VFOA) || 
		 (bandmem[curr_band].VFO_AB_Active == VFO_A && vfo_b_last != VFOB) ||
		vfo_changed)
	{
		tft.fillRect(pVStby->bx, pVStby->by, pVStby->bw, pVStby->bh, pVStby->bg_clr);
		tft.drawRect(pVStby->bx, pVStby->by, pVStby->bw, pVStby->bh, pVStby->ol_clr);
		tft.setFont(pVStby->txt_Font);
		tft.setCursor(pVStby->bx+pVStby->padx, pVStby->by+pVStby->pady);
		tft.setTextColor(pVStby->txt_clr);
		if (bandmem[curr_band].VFO_AB_Active == VFO_A)			
			tft.print(formatVFO(VFOB));			
		else
			tft.print(formatVFO(VFOA));	
	}
	
	vfo_changed = 0;
	vfo_a_last = VFOA;  // record to detect a change next time around.
	vfo_b_last = VFOB;
}

COLD void displayMode(void)
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

COLD void displayFilter(void)
{
	char str[15];

	#ifdef PANADAPTER
		extern int16_t filterWidth;
		sprintf(str, "F: %dHz", filterWidth);	
	#else
		sprintf(str, "F: %s%s", filter[bandmem[curr_band].filter].Filter_name,filter[bandmem[curr_band].filter].units);	
	#endif
	//Serial.print(F("Filter is ")); Serial.println(str);
	sprintf(labels[FILTER_LBL].label, "%s", str);
	drawLabel(FILTER_LBL, &bandmem[curr_band].filter);
	draw_2_state_Button(FILTER_BTN, &bandmem[curr_band].filter);
}

COLD void displayRate(void)
{	
	if (bandmem[curr_band].tune_step >= TS_STEPS)
		bandmem[curr_band].tune_step = TS_STEPS-1;
	sprintf(labels[RATE_LBL].label, "R: %s%s", tstep[bandmem[curr_band].tune_step].ts_name, tstep[bandmem[curr_band].tune_step].ts_units);;
	//Serial.print(F("Tune Rate is ")); Serial.println(labels[RATE_LBL].label);
	drawLabel(RATE_LBL, &bandmem[curr_band].tune_step);
	draw_2_state_Button(RATE_BTN, &bandmem[curr_band].tune_step);
}

COLD void displayAgc(void)
{	
	sprintf(std_btn[AGC_BTN].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
	sprintf(labels[AGC_LBL].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
	//Serial.print("AGC ="); Serial.println(std_btn[AGC_BTN].label);
	drawLabel(AGC_LBL, &bandmem[curr_band].agc_mode);
	draw_2_state_Button(AGC_BTN, &bandmem[curr_band].agc_mode);
}

COLD void displayANT(void)
{	
	sprintf(std_btn[ANT_BTN].label, "%s%1d", "ANT", bandmem[curr_band].ant_sw);
	sprintf(labels[ANT_LBL].label, "%s%1d", "ANT", bandmem[curr_band].ant_sw);
	//Serial.print(F("Antenna Switch set to ")); Serial.println(std_btn[ANT_BTN].label);
	drawLabel(ANT_LBL, &bandmem[curr_band].ant_sw);
	draw_2_state_Button(ANT_BTN, &bandmem[curr_band].ant_sw);
}

void displayPan(void)
{	
	char string[80];   // print format stuff
	sprintf(std_btn[PAN_BTN].label, "%s%3d", "Pan:", user_settings[user_Profile].pan_level-50);
	//sprintf(labels[PAN_BTN].label, "%s%3d", "PN:", user_settings[user_Profile].pan_level);
	//drawLabel(PAN_BTN, &user_settings[user_Profile].pan_level);
    if (MF_client == PAN_BTN) 
	{ 
		//MF_default_is_active = false;
		sprintf(string, "Pan:%3d", user_settings[user_Profile].pan_level-50);
		MeterInUse = true;
		displayMeter(user_settings[user_Profile].pan_level/10, string, 5);   // val, string label, color scheme
	}
	//Serial.print(F("Pan set to ")); Serial.println(std_btn[PAN_BTN].label);
	draw_2_state_Button(PAN_BTN, &user_settings[user_Profile].pan_state);
  #ifdef I2C_LCD
    lcd.setCursor(10,1);
    lcd.print(std_btn[PAN_BTN].label);
  #endif
}

COLD void displayRFgain(void)
{	
	char string[80];   // print format stuff
	sprintf(std_btn[RFGAIN_BTN].label, "%s%3d", "RF:", user_settings[user_Profile].rfGain);
	//sprintf(labels[RFGAIN_LBL].label, "%s%3d", "RF:", user_settings[user_Profile].rfGain);
	//drawLabel(RFGAIN_LBL, &user_settings[user_Profile].rfGain);
    if (MF_client == RFGAIN_BTN) 
	{ 
		//MF_default_is_active = false;
		sprintf(string, " RF:%d", user_settings[user_Profile].rfGain);
		MeterInUse = true;
		displayMeter(user_settings[user_Profile].rfGain/10, string, 5);   // val, string label, color scheme
	}
	//Serial.print(F("RF Gain set to ")); Serial.println(std_btn[RFGAIN_BTN].label);
	draw_2_state_Button(RFGAIN_BTN, &user_settings[user_Profile].rfGain_en);
  #ifdef I2C_LCD
    lcd.setCursor(10,1);
    lcd.print(std_btn[RFGAIN_BTN].label);
  #endif
}

COLD void displayAFgain(void)
{	
	char string[80];   // print format stuff
	sprintf(std_btn[AFGAIN_BTN].label, "%s%3d", "AF:", user_settings[user_Profile].afGain);
	//sprintf(labels[AFGAIN_LBL].label, "%s%3d", "AF:", user_settings[user_Profile].afGain);
	//drawLabel(AFGAIN_LBL, &user_settings[user_Profile].afGain);
	if (MF_client == AFGAIN_BTN) 
	{ 
		sprintf(string, " AF:%d", user_settings[user_Profile].afGain);
        MeterInUse = true;
    	displayMeter(user_settings[user_Profile].afGain/10, string, 5);   // val, string label, color scheme        
	}
	//Serial.print(F("AF Gain set to ")); Serial.println(std_btn[AFGAIN_BTN].label);
	draw_2_state_Button(AFGAIN_BTN, &user_settings[user_Profile].afGain_en);
  #ifdef I2C_LCD  
    lcd.setCursor(0,1);
    lcd.print(std_btn[AFGAIN_BTN].label);
  #endif
}

COLD void displayAttn()
{
	char string[80];   // print format stuff
	sprintf(std_btn[ATTEN_BTN].label, "%s%3d", "ATT:", bandmem[curr_band].attenuator_dB);
	//Serial.print(F("Atten is ")); Serial.println(bandmem[curr_band].attenuator);
	if (MF_client == ATTEN_BTN) 
	{ 
		sprintf(string, "ATT:%d", bandmem[curr_band].attenuator_dB);
        MeterInUse = true;
    	displayMeter(bandmem[curr_band].attenuator_dB/10, string, 5);   // val, string label, color scheme               
	}
	drawLabel(ATTEN_LBL, &bandmem[curr_band].attenuator_byp);
	draw_2_state_Button(ATTEN_BTN, &bandmem[curr_band].attenuator_byp);
}

COLD void displayPreamp()
{
	//Serial.print(F("Preamp is ")); Serial.println(bandmem[curr_band].preamp);
	drawLabel(PREAMP_LBL, &bandmem[curr_band].preamp);
	draw_2_state_Button(PREAMP_BTN, &bandmem[curr_band].preamp);
}

COLD void displayATU()
{
	//Serial.print(F("ATU is ")); Serial.println(bandmem[curr_band].ATU);
	drawLabel(ATU_LBL, &bandmem[curr_band].ATU);
	draw_2_state_Button(ATU_BTN, &bandmem[curr_band].ATU);
}

COLD void displayRIT()
{
	//Serial.print(F("RIT is ")); Serial.println(bandmem[curr_band].RIT_en);
	drawLabel(RIT_LBL, &bandmem[curr_band].RIT_en);
	draw_2_state_Button(RIT_BTN, &bandmem[curr_band].RIT_en);
}

COLD void displayXIT()
{
	//Serial.print(F("XIT is ")); Serial.println(bandmem[curr_band].XIT_en);
	drawLabel(XIT_LBL, &bandmem[curr_band].XIT_en);
	draw_2_state_Button(XIT_BTN, &bandmem[curr_band].XIT_en);
}

COLD void displayFine()
{
	//Serial.print(F("Fine Tune is ")); Serial.println(user_settings[user_Profile].fine);
	drawLabel(FINE_LBL, &user_settings[user_Profile].fine);
	draw_2_state_Button(FINE_BTN,  &user_settings[user_Profile].fine);
}

COLD void displayNB()
{
	char string[80];   // print format stuff
	sprintf(std_btn[NB_BTN].label, "NB:%1d", user_settings[user_Profile].nb_level);
    //sprintf(labels[NB_LBL].label,  "NB%s", nb[user_settings[user_Profile].nb_level].nb_name);
	//Serial.print(F("NB is ")); Serial.print(user_settings[user_Profile].nb_en);
	//Serial.print(F("   NB Level is ")); Serial.println(user_settings[user_Profile].nb_level);
	if (MF_client == NB_BTN) 
	{ 
		sprintf(string, "  NB:%1d", user_settings[user_Profile].nb_level);
        MeterInUse = true;
    	displayMeter((int) user_settings[user_Profile].nb_level*1.7, string, 5);   // val, string label, color scheme        
	}
	drawLabel(NB_LBL, &user_settings[user_Profile].nb_level);
	draw_2_state_Button(NB_BTN, &user_settings[user_Profile].nb_en);
}

COLD void displayZoom()
{
	// Enable the label draw if a screen icon is used
    //sprintf(labels[ZOOM_LBL].label,  "Zoom:%d", user_settings[user_Profile].zoom_level);
	//drawLabel(ZOOM_LBL, &user_settings[user_Profile].zoom_level);

	//Serial.print(F("Zoom is ")); Serial.println(zoom[user_settings[user_Profile].zoom_level].zoom_name);
	sprintf(std_btn[ZOOM_BTN].label, "Zoom%s", zoom[user_settings[user_Profile].zoom_level].zoom_name);
	draw_2_state_Button(ZOOM_BTN, &user_settings[user_Profile].zoom_level);
}

COLD void displayRefLevel()
{
	char string[80];   // print format stuff
	if (MF_client == REFLVL_BTN) 
	{ 
		sprintf(string, "Lvl:%d", bandmem[curr_band].sp_ref_lvl);
		MeterInUse = true; 
		displayMeter((bandmem[curr_band].sp_ref_lvl+50)/10, string, 5);   // val, string label, color scheme
	}
	draw_2_state_Button(REFLVL_BTN, &std_btn[REFLVL_BTN].enabled); 
}

COLD void displayNR()
{
	//Serial.print(F("NR is ")); Serial.println(user_settings[user_Profile].nr_en);
	drawLabel(NR_LBL, &user_settings[user_Profile].nr_en);
	draw_2_state_Button(NR_BTN, &user_settings[user_Profile].nr_en);
}

COLD void displayNotch()
{
	//Serial.print(F("Notch is ")); Serial.println(std_btn[NOTCH_BTN].label);
	drawLabel(NOTCH_LBL, &user_settings[user_Profile].notch);
	draw_2_state_Button(NOTCH_BTN,  &user_settings[user_Profile].notch);
}

COLD void displaySplit()
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
	//Serial.print(F("Split is ")); Serial.println(bandmem[curr_band].split);
	drawLabel(SPLIT_LBL, &bandmem[curr_band].split);
	draw_2_state_Button(SPLIT_BTN, &bandmem[curr_band].split);
}

COLD void displayTime(void)
{
	#ifdef ENET
	sprintf(std_btn[UTCTIME_BTN].label, "UTC:%02d:%02d:%02d", hour(), minute(), second());
	#else
	sprintf(std_btn[UTCTIME_BTN].label, "Local:%02d:%02d:%02d", hour(), minute(), second());
	#endif
	//tft.print(std_btn[UTCTIME_BTN].label);
	//Serial.println("UTC Time = "); Serial.println(std_btn[UTCTIME_BTN].label);
	draw_2_state_Button(UTCTIME_BTN, &std_btn[UTCTIME_BTN].show);	
}

// val = bar graph value (0 to 10 range), string is the meter text, color set
COLD void displayMeter(int val, const char *string, uint16_t colorscheme)
{
	#ifdef USE_RA8875
	   	ringMeter(val, 0, 10, std_btn[SMETER_BTN].bx+20, std_btn[SMETER_BTN].by+10, std_btn[SMETER_BTN].bh-50, string, colorscheme, 1, 90, 8);
	#else
		ringMeter(val, 0, 10, std_btn[SMETER_BTN].bx+20, std_btn[SMETER_BTN].by+10, std_btn[SMETER_BTN].bh-50, string, colorscheme, 1, 90, 8);
	#endif
	static uint8_t startup_flag = 0;
	if (startup_flag == 0)
	{
		draw_2_state_Button(SMETER_BTN, &std_btn[SMETER_BTN].show);
		startup_flag = 1;  // only draw this box on startup then write smeter direct. The button fills the inside each update so cannot use it.
	}
}

// These buttons have no associated labels so are simply button updates
COLD void displayMenu() 	{draw_2_state_Button(MENU_BTN, &std_btn[MENU_BTN].enabled);				 }
COLD void displayFn() 		{draw_2_state_Button(FN_BTN, &std_btn[FN_BTN].enabled);					 }
COLD void displayVFO_AB() 	{draw_2_state_Button(VFO_AB_BTN, &bandmem[curr_band].VFO_AB_Active);	 }
COLD void displayBandUp() 	{draw_2_state_Button(BANDUP_BTN, &bandmem[curr_band].band_num);			 }
COLD void displayBand() 	{draw_2_state_Button(BAND_BTN, &bandmem[curr_band].band_num);			 }
COLD void displaySpot() 	{draw_2_state_Button(SPOT_BTN,  &user_settings[user_Profile].spot);		 }
COLD void displayBandDn()	{draw_2_state_Button(BANDDN_BTN, &bandmem[curr_band].band_num);			 }
COLD void displayDisplay()	{draw_2_state_Button(DISPLAY_BTN, &display_state);						 }
COLD void displayXMIT()		{draw_2_state_Button(XMIT_BTN, &user_settings[user_Profile].xmit);		 }
COLD void displayMute()		{draw_2_state_Button(MUTE_BTN, &user_settings[user_Profile].mute);		 }
COLD void displayXVTR()		{draw_2_state_Button(XVTR_BTN, &bandmem[curr_band].xvtr_en);			 }
COLD void displayEnet()		{draw_2_state_Button(ENET_BTN, &user_settings[user_Profile].enet_output);}

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
COLD void draw_2_state_Button(uint8_t button, uint8_t *function_ptr) 
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

COLD void drawLabel(uint8_t lbl_num, uint8_t *function_ptr)
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
COLD const char* formatVFO(uint32_t vfo)
{
	static char vfo_str[25] = {""};
	if (ModeOffset < -1 || ModeOffset > 1)
		vfo += ModeOffset;  // Account for pitch offset when in CW mode, not others
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
// In theory every button and label can be called here in any order.  
// The table Panelnum and Panelpos control the position.  Show control visibility.
// When a panel is active, the button for tha panel are flipped to show=ON, all other are set to show=OFF
// 
COLD void displayRefresh(void)
{
	// Bottom Panel Anchor button
	displayFn();   // make fn=1 to call displayFn() to prevent calling itself
    displayFreq();    // display frequency
	#ifdef ENET
	if (enet_ready)
		displayTime();
	#endif
	//displayMeter();
	// Panel 1 buttons
	displayMode();
	displayFilter();
	displayRate();
	displayAttn();
	displayPreamp();
	displayBand();
	//Panel 2 buttons
	displayNB();
	displayNR();
	displayNotch();
	displayAgc();
	displayZoom();
	displayPan();
	//Panel 3 buttons
	displayMenu();
	displayANT();
	displayATU();
	displayXMIT();
	displayBandDn();
	displayBandUp();
	//Panel 4 buttons
	displayRIT();
	displayXIT();
	displayFine();
	displaySplit();
	displayDisplay();
	displayVFO_AB();
	//Panel 5 buttons
	displayEnet();
	displayXVTR();
	displayRFgain();
	displayRefLevel();
	displayAFgain();
	displayMute();
	
	//displaySpot(); // spare
}

//#ifndef USE_RA8875

/*
 An example showing 'ring' analogue meter on a RA8875/8876 TFT
 color screen

 Needs Fonts 2, 4 and 7 (also Font 6 if using a large size meter)
 */

// Meter colour schemes
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5

uint16_t  textcolor   = 0xFFFF;
uint16_t  textbgcolor = 0x0000;
uint16_t _width    = tft.width();
uint16_t _height   = tft.height();
uint8_t textsize  = 1;
uint8_t padX = 0;
bool textwrap  = true;
uint8_t textdatum = 0; // Left text alignment is default
uint32_t runTime = -99999;       // time for next update

int reading = 0; // Value to be displayed
int d = 0; // Variable used for the sinewave test waveform
boolean alert = 0;
int8_t ramp = 1;

//These enumerate the text plotting alignment (reference datum point)
#define TL_DATUM 0 // Top left (default)
#define TC_DATUM 1 // Top centre
#define TR_DATUM 2 // Top right
#define ML_DATUM 3 // Middle left
#define CL_DATUM 3 // Centre left, same as above
#define MC_DATUM 4 // Middle centre
#define CC_DATUM 4 // Centre centre, same as above
#define MR_DATUM 5 // Middle right
#define CR_DATUM 5 // Centre right, same as above
#define BL_DATUM 6 // Bottom left
#define BC_DATUM 7 // Bottom centre
#define BR_DATUM 8 // Bottom right

uint16_t  addr_row = 0xFFFF;
uint16_t  addr_col = 0xFFFF;
uint16_t  win_xe = 0xFFFF;
uint16_t  win_ye = 0xFFFF;

#ifdef LOAD_GLCD
  fontsloaded = 0x0002; // Bit 1 set
#endif

#ifdef LOAD_FONT2
  fontsloaded |= 0x0004; // Bit 2 set
#endif

#ifdef LOAD_FONT4
  fontsloaded |= 0x0010; // Bit 4 set
#endif

#ifdef LOAD_FONT6
  fontsloaded |= 0x0040; // Bit 6 set
#endif

#ifdef LOAD_FONT7
  fontsloaded |= 0x0080; // Bit 7 set
#endif

#ifdef LOAD_FONT8
  fontsloaded |= 0x0100; // Bit 8 set
#endif

typedef struct {
  const unsigned char *chartbl;
  const unsigned char *widthtbl;
  unsigned       char height;
} fontinfo;

// This is a structure to conveniently hold infomation on the fonts
// Stores font character image address pointer, width table and height

const fontinfo fontdata [] = {
   { 0, 0, 0 },
   { 0, 0, 8 }
};

/**************************************************************************/
/*!
	  calculate a grandient color
	  return a spectrum starting at blue to red (0...127)
*/
/**************************************************************************/
COLD uint16_t grandient(uint8_t val)
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
COLD uint16_t colorInterpolation(uint8_t r1,uint8_t g1,uint8_t b1,uint8_t r2,uint8_t g2,uint8_t b2,uint16_t pos,uint16_t div)
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
COLD void ringMeter(int val, int minV, int maxV, int16_t x, int16_t y, uint16_t r, const char* units, uint16_t colorScheme,uint16_t backSegColor,int16_t angle,uint8_t inc)
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
	for (int16_t i = -angle; i < angle; i += inc) 
	{
		colour = RA8875_BLACK;
		switch (colorScheme) 
		{
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

		if (i < curAngle) 
		{ 
			// Fill in coloured segments with 2 triangles
			switch (colorScheme) 
			{
				case 0: colour = RA8875_RED; break; // Fixed colour
				case 1: colour = RA8875_GREEN; break; // Fixed colour
				case 2: colour = RA8875_BLUE; break; // Fixed colour
				case 3: colour = rainbow(map(i, -angle, angle, 0, 127)); break; // Full spectrum blue to red
				case 4: colour = rainbow(map(i, -angle, angle, 70, 127)); break; // Green to red (high temperature etc)
				case 5: colour = rainbow(map(i, -angle, angle, 127, 63)); break; // Red to green (low battery etc)
			   default: colour = RA8875_BLUE; break; // Fixed colour
			}
			tft.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
			tft.fillTriangle(x1, y1, x2, y2, x3, y3, colour);
			//static int16_t text_color = colour; // Save the last colour drawn
		}
		else // Fill in blank segments
		{
			tft.fillTriangle(x0, y0, x1, y1, x2, y2, BLACK);
			tft.fillTriangle(x1, y1, x2, y2, x3, y3, BLACK);
		}

	}
	// Convert value to a string
	char buf[10];
	byte len = 3; if (val > 999) len = 5;
	dtostrf(val, len, 0, buf);
	buf[len] = ' '; buf[len+1] = 0; // Add blanking space and terminator, helps to centre text too!
	//Set the text colour to default
	tft.setTextSize(1);
	tft.setFont(Arial_14);
	/*   Not using this overange feature - could change the S-Unit text color though
	if (val<minV || val>maxV) 
	{
		drawAlert(x,y+90,50,1);
	}
	else 
	{
		drawAlert(x,y+90,50,0);
	}
	*/
	tft.setTextColor(BLUE, BLACK);
	// Uncomment next line to set the text colour to the last segment value!
	//tft.setTextColor(colour, RA8875_BLACK);
	x -= 32;
	y -= 8;
	tft.setCursor(x,y);
	tft.fillRect(x,y,75,14,BLACK);  // Clear text space
	tft.print(units);

	//setTextDatum(MC_DATUM);
	// Print value, if the meter is large then use big font 8, othewise use 4
	//if (r > 84) 
	//{
		//setTextPadding(55*3); // Allow for 3 digits each 55 pixels wide
		//drawString(buf, x, y, 8); // Value in middle
	//}
	//else 
	//{
		//setTextPadding(3 * 14); // Allow for 3 digits each 14 pixels wide
		//setTextPadding(55*3); // Allow for 3 digits each 55 pixels wide
		//drawString(buf, x, y, 4); // Value in middle
	//}

	//tft.setTextSize(1);
	//setTextPadding(0);
	// Print units, if the meter is large then use big font 4, othewise use 2
	//tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
	//if (r > 84)
	//{ 
		//tft.printf(buf, x, y, 4); // Value in middle
		//drawString(units, x, y + 60, 4); // Units display
		//tft.setCursor(x,y);
		//tft.print(units);
	//}
	//else 
	//{
		//tft.printf(units, x, y + 15, 2); // Units display
		//drawString(units, x, y + 15, 2); // Units display
		//tft.setCursor(x,y);
		//tft.print(units);
	//}
}

/**************************************************************************/
/*!
      Draw a quadrilateral by connecting 4 points
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
COLD void drawQuad(int16_t x0, int16_t y0,int16_t x1, int16_t y1,int16_t x2, int16_t y2,int16_t x3, int16_t y3, uint16_t color) 
{
	tft.drawLine(x0, y0, x1, y1, color);//low 1
	tft.drawLine(x1, y1, x2, y2, color);//high 1
	tft.drawLine(x2, y2, x3, y3, color);//high 2
	tft.drawLine(x3, y3, x0, y0, color);//low 2
}

COLD void drawAlert(int x, int y , int side, boolean draw)
{
  if (draw && !alert) {
    tft.fillTriangle(x, y, x+30, y+47, x-30, y+47, rainbow(95));
    tft.setTextColor(RA8875_BLACK);
    drawCentreString("!", x, y + 6, 4);
    alert = 1;
  }
  else if (!draw) {
    tft.fillTriangle(x, y, x+30, y+47, x-30, y+47, RA8875_BLACK);
    alert = 0;
  }
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
COLD unsigned int rainbow(byte value)
{
  // Value is expected to be in range 0-127
  // The value is converted to a spectrum colour from 0 = blue through to 127 = red

  byte red = 0; // Red is the top 5 bits of a 16 bit colour value
  byte green = 0;// Green is the middle 6 bits
  byte blue = 0; // Blue is the bottom 5 bits

  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// #########################################################################
COLD float sineWave(int phase) {
  return sin(phase * 0.0174532925);
}

/***************************************************************************************
** Function name:           setTextDatum
** Description:             Set the text position reference datum
***************************************************************************************/
COLD void setTextDatum(uint8_t d)
{
  textdatum = d;
}

/***************************************************************************************
** Function name:           setTextPadding
** Description:             Define padding width (aids erasing old text and numbers)
***************************************************************************************/
COLD void setTextPadding(uint16_t x_width)
{
  padX = x_width;
}

/***************************************************************************************
** Function name:           drawString
** Description :            draw string with padding if it is defined
***************************************************************************************/
COLD int drawString(const char *string, int poX, int poY, int font)
{
  	int16_t sumX = 0;
  	uint8_t padding = 1;
  	unsigned int cheight = 0;
	_width = tft.width();
	_height = tft.height();

  	if (textdatum || padX)
  	{
    	// Find the pixel width of the string in the font
    	unsigned int cwidth  = textWidth(string, font);

    	// Get the pixel height of the font
    	cheight = pgm_read_byte( &fontdata[font].height ) * textsize;

    switch(textdatum) {
      case TC_DATUM:
        poX -= cwidth/2;
        padding = 2;
        break;
      case TR_DATUM:
        poX -= cwidth;
        padding = 3;
        break;
      case ML_DATUM:
        poY -= cheight/2;
        padding = 1;
        break;
      case MC_DATUM:
        poX -= cwidth/2;
        poY -= cheight/2;
        padding = 2;
        break;
      case MR_DATUM:
        poX -= cwidth;
        poY -= cheight/2;
        padding = 3;
        break;
      case BL_DATUM:
        poY -= cheight;
        padding = 1;
        break;
      case BC_DATUM:
        poX -= cwidth/2;
        poY -= cheight;
        padding = 2;
        break;
      case BR_DATUM:
        poX -= cwidth;
        poY -= cheight;
        padding = 3;
        break;
    }
	Serial.println("PARMS=");
	Serial.println(_width);
Serial.println(_height);
    // Check coordinates are OK, adjust if not
    if (poX < 0) poX = 0;
    if (poX+cwidth>_width)   poX = _width - cwidth;
    if (poY < 0) poY = 0;
    if (poY+cheight>_height) poY = _height - cheight;
  }
Serial.println(poX);
Serial.println(poY);
  //while (*string) sumX += drawChar(*(string++), poX+sumX, poY, font);
  tft.setCursor(poX+sumX, poY);
  tft.setFont(Arial_14);
  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
  while (*string) sumX += tft.print(*(string++));
//#define PADDING_DEBUG

#ifndef PADDING_DEBUG
  if((padX>sumX) && (textcolor!=textbgcolor))
  {
    int padXc = poX+sumX; // Maximum left side padding
    switch(padding) {
      case 1:
        tft.fillRect(padXc,poY,padX-sumX,cheight, textbgcolor);
        break;
      case 2:
        tft.fillRect(padXc,poY,(padX-sumX)>>1,cheight, textbgcolor);
        padXc = (padX-sumX)>>1;
        if (padXc>poX) padXc = poX;
        tft.fillRect(poX - padXc,poY,(padX-sumX)>>1,cheight, textbgcolor);
        break;
      case 3:
        if (padXc>padX) padXc = padX;
        tft.fillRect(poX + sumX - padXc,poY,padXc-sumX,cheight, textbgcolor);
        break;
    }
  }
#else

  // This is debug code to show text (green box) and blanked (white box) areas
  // to show that the padding areas are being correctly sized and positioned
  if((padX>sumX) && (textcolor!=textbgcolor))
  {
    int padXc = poX+sumX; // Maximum left side padding
    tft.drawRect(poX,poY,sumX,cheight, GREEN);
    switch(padding) {
      case 1:
        tft.drawRect(padXc,poY,padX-sumX,cheight, WHITE);
        break;
      case 2:
        tft.drawRect(padXc,poY,(padX-sumX)>>1, cheight, WHITE);
        padXc = (padX-sumX)>>1;
        if (padXc>poX) padXc = poX;
        tft.drawRect(poX - padXc,poY,(padX-sumX)>>1,cheight, WHITE);
        break;
      case 3:
        if (padXc>padX) padXc = padX;
        tft.drawRect(poX + sumX - padXc,poY,padXc-sumX,cheight, WHITE);
        break;
    }
  }
#endif

return sumX;
}

/***************************************************************************************
** Function name:           drawCentreString
** Descriptions:            draw string centred on dX
***************************************************************************************/
COLD int drawCentreString(const char *string, int dX, int poY, int font)
{
  byte tempdatum = textdatum;
  int sumX = 0;
  textdatum = TC_DATUM;
  sumX = drawString(string, dX, poY, font);
  textdatum = tempdatum;
  return sumX;
}

/***************************************************************************************
** Function name:           drawRightString
** Descriptions:            draw string right justified to dX
***************************************************************************************/
COLD int drawRightString(const char *string, int dX, int poY, int font)
{
  byte tempdatum = textdatum;
  int sumX = 0;
  textdatum = TR_DATUM;
  sumX = drawString(string, dX, poY, font);
  textdatum = tempdatum;
  return sumX;
}

/***************************************************************************************
** Function name:           textWidth
** Description:             Return the width in pixels of a string in a given font
***************************************************************************************/
COLD int16_t textWidth(const char *string, int font)
{
  unsigned int str_width  = 0;
  char uniCode;
  char *widthtable;

  if (font>1 && font<9)
  //widthtable = (char *)pgm_read_word( &(fontdata[font].widthtbl ) ) - 32; //subtract the 32 outside the loop
  widthtable = (char *)( &(fontdata[font].widthtbl ) ) - 32; //subtract the 32 outside the loop
  else return 0;

  while (*string)
  {
    uniCode = *(string++);
#ifdef LOAD_GLCD
    if (font == 1) str_width += 6;
    else
#endif
    str_width += (int) (widthtable + uniCode); // Normally we need to subract 32 from uniCode
  }
  return str_width * textsize;
}

/***************************************************************************************
** Function name:           fillCircle
** Description:             draw a filled circle
***************************************************************************************/
COLD void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  tft.drawFastVLine(x0, y0 - r, r + r + 1, color);
  //fillCircleHelper(x0, y0, r, 3, 0, color);
}

/***************************************************************************************
** Function name:           fillCircleHelper
** Description:             Support function for filled circle drawing
***************************************************************************************/

// Used to do circles and roundrects
COLD void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color)
{
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -r - r;
  int16_t x     = 0;

  delta++;
  while (x < r) {
    if (f >= 0) {
      r--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
      tft.drawFastVLine(x0 + x, y0 - r, r + r + delta, color);
      tft.drawFastVLine(x0 + r, y0 - x, x + x + delta, color);
    }
    if (cornername & 0x2) {
      tft.drawFastVLine(x0 - x, y0 - r, r + r + delta, color);
      tft.drawFastVLine(x0 - r, y0 - x, x + x + delta, color);
    }
  }
}

/***************************************************************************************
** Function name:           drawCircle
** Description:             Draw a circle outline
***************************************************************************************/
COLD void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = - r - r;
  int16_t x = 0;

  //fastSetup();

  tft.drawPixel(x0 + r, y0  , color);
  tft.drawPixel(x0 - r, y0  , color);
  tft.drawPixel(x0  , y0 - r, color);
  tft.drawPixel(x0  , y0 + r, color);

  while (x < r) {
    if (f >= 0) {
      r--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    tft.drawPixel(x0 + x, y0 + r, color);
    tft.drawPixel(x0 - x, y0 + r, color);
    tft.drawPixel(x0 - x, y0 - r, color);
    tft.drawPixel(x0 + x, y0 - r, color);

    tft.drawPixel(x0 + r, y0 + x, color);
    tft.drawPixel(x0 - r, y0 + x, color);
    tft.drawPixel(x0 - r, y0 - x, color);
    tft.drawPixel(x0 + r, y0 - x, color);
  }
}

/***************************************************************************************
** Function name:           drawCircleHelper
** Description:             Support function for circle drawing
***************************************************************************************/
COLD void drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color)
{
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;

  while (x < r) {
    if (f >= 0) {
      r--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4) {
      tft.drawPixel(x0 + x, y0 + r, color);
      tft.drawPixel(x0 + r, y0 + x, color);
    }
    if (cornername & 0x2) {
      tft.drawPixel(x0 + x, y0 - r, color);
      tft.drawPixel(x0 + r, y0 - x, color);
    }
    if (cornername & 0x8) {
      tft.drawPixel(x0 - r, y0 + x, color);
      tft.drawPixel(x0 - x, y0 + r, color);
    }
    if (cornername & 0x1) {
      tft.drawPixel(x0 - r, y0 - x, color);
      tft.drawPixel(x0 - x, y0 - r, color);
    }
  }
}

//	#endif // ifndef USE_RA8875
