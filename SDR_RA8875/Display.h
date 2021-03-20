#include <RA8875.h>
extern RA8875 tft;

extern uint8_t curr_band;   // global tracks our current band setting.  
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint32_t VFOB;
extern struct Band_Memory bandmem[];
extern struct Bandwidth_Settings bw[];
extern struct Standard_Button std_btn[];
extern struct Complex_Button complex_btn[];
extern struct Frequency_Display disp_Freq[];
extern struct User_Settings user_settings[];
extern uint8_t user_Profile;

// function declaration
void draw_2_state_Button(uint8_t button, uint8_t *function_ptr);
void refreshScreen(void);
const char * formatVFO(uint32_t vfo);
void displayTime(void);

int displayColor = RA8875_LIGHT_GREY;
int textcolor = tft.Color565(128, 128, 128);

//////////////////////////////////////////////////////////////

void displayFreq(void)
{ 
	struct User_Settings *pTX 		 = &user_settings[user_Profile];
	struct Frequency_Display *pVAct  = &disp_Freq[0];     // pointer to Active VFO Digits record
	struct Frequency_Display *pMAct  = &disp_Freq[1];     // pointer to Active VFO Label record
	struct Frequency_Display *pVStby = &disp_Freq[2];     // pointer to Standby VFO Digits record
	struct Frequency_Display *pMStby = &disp_Freq[3];     // pointer to Standby VFO Label record

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
	}
	else
	{
		tft.print("B");		
		tft.setFont(pVAct->txt_Font);
		tft.setTextColor(pVAct->txt_clr);
		tft.setCursor(pVAct->bx+pVAct->padx, pVAct->by+pVAct->pady);
		tft.print(formatVFO(VFOB));
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
	char m_str[10];
	uint8_t mode;
	uint16_t x = 10;
	uint16_t y = 110;
	uint16_t w = 60;
	uint16_t h = 20;

	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
		mode = bandmem[curr_band].mode_A;
	else 
		mode = bandmem[curr_band].mode_B;
	sprintf(m_str, "%s", Mode[mode]);
	tft.print(m_str);    // use MODE{band][mode] to retrieve the text}
	//sprintf(m_str, "M:%s", Mode[mode]);
	//strcpy(std_btn[MODE_BTN].label, m_str);
	draw_2_state_Button(MODE_BTN, &mode);
}

void displayFilter(void)
{
	char filt_str[10];
	uint16_t x = 80;
	uint16_t y = 110;
	uint16_t w = 100;
	uint16_t h = 20;

	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	tft.setTextColor(RA8875_LIGHT_ORANGE);	
	sprintf(filt_str, "F: %s%s", filter[bandmem[curr_band].filter].Filter_name,filter[bandmem[curr_band].filter].units);
	tft.print(filt_str);
	//sprintf(std_btn[FILTER_BTN].label, "F:%s", filter[bandmem[curr_band].filter].Filter_name);
	Serial.print("Tune Rate is "); Serial.println(filt_str);
	draw_2_state_Button(FILTER_BTN, &bandmem[curr_band].filter);
}

void displayRate(void)
{
	char r_str[15];
	uint16_t x = 190;
	uint16_t y = 110;
	uint16_t w = 80;
	uint16_t h = 20;

	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2);
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	sprintf(r_str, "R: %s%s", tstep[bandmem[curr_band].tune_step].ts_name, tstep[bandmem[curr_band].tune_step].ts_units);
	tft.print(r_str);
	//sprintf(std_btn[RATE_BTN].label, "R:%s", tstep[bandmem[curr_band].tune_step].ts_name);
	Serial.print("Tune Rate is "); Serial.println(r_str);
	draw_2_state_Button(RATE_BTN, &bandmem[curr_band].tune_step);
}

void displayAttn()
{
	uint16_t x = 296;
	uint16_t y = 110;
	uint16_t w = 40;
	uint16_t h = 20;
	
	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	if (bandmem[curr_band].attenuator)
		tft.setTextColor(RA8875_GREEN);
	else
		tft.setTextColor(RA8875_LIGHT_ORANGE);
	//sprintf(std_btn[ATTEN_BTN].label, "%s", "ATT");
	tft.print(std_btn[ATTEN_BTN].label);
	Serial.print("Atten is "); Serial.println(bandmem[curr_band].attenuator);
	draw_2_state_Button(ATTEN_BTN, &bandmem[curr_band].attenuator);
}

void displayPreamp()
{
	uint16_t x = 358;
	uint16_t y = 110;
	uint16_t w = 40;
	uint16_t h = 20;
	
	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	if (bandmem[curr_band].preamp)
		tft.setTextColor(RA8875_GREEN);
	else
		tft.setTextColor(RA8875_LIGHT_ORANGE);
	//sprintf(std_btn[PREAMP_BTN].label, "%s", "PRE");
	tft.print(std_btn[PREAMP_BTN].label);
	Serial.print("Preamp is "); Serial.println(bandmem[curr_band].preamp);
	draw_2_state_Button(PREAMP_BTN, &bandmem[curr_band].preamp);
}

void displayANT()
{
	uint16_t x = 424;
	uint16_t y = 110;
	uint16_t w = 60;
	uint16_t h = 20;
	
	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	sprintf(std_btn[ANT_BTN].label, "%s%1d", "ANT", bandmem[curr_band].ant_sw);
	tft.print(std_btn[ANT_BTN].label);
	Serial.print("Antenna Switch set to "); Serial.println(std_btn[ANT_BTN].label);
	draw_2_state_Button(ANT_BTN, &bandmem[curr_band].ant_sw);
}

void displayATU()
{
	uint16_t x = 494;
	uint16_t y = 110;
	uint16_t w = 40;
	uint16_t h = 20;
	
	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	if (bandmem[curr_band].ATU)
		tft.setTextColor(RA8875_GREEN);
	else
		tft.setTextColor(RA8875_LIGHT_ORANGE);
	//sprintf(std_btn[ATU_BTN].label, "%s", "ATU");
	tft.print(std_btn[ATU_BTN].label);
	Serial.print("ATU is "); Serial.println(bandmem[curr_band].ATU);
	draw_2_state_Button(ATU_BTN, &bandmem[curr_band].ATU);
}

void displayAgc(void)
{
	uint16_t x = 554;
	uint16_t y = 110;
	uint16_t w = 70;
	uint16_t h = 20;

	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	sprintf(std_btn[AGC_BTN].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
	tft.print(std_btn[AGC_BTN].label);
	Serial.print("AGC ="); Serial.println(std_btn[AGC_BTN].label);
	draw_2_state_Button(AGC_BTN, &bandmem[curr_band].agc_mode);
}

void displayRIT()
{
	uint16_t x = 10;
	uint16_t y = 50; //75;
	uint16_t w = 40;
	uint16_t h = 20;
	
	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	if (bandmem[curr_band].RIT_en)
		tft.setTextColor(RA8875_GREEN);
	else
		tft.setTextColor(RA8875_LIGHT_ORANGE);
	//sprintf(std_btn[RIT_BTN].label, "%s", "RIT");
	tft.print(std_btn[RIT_BTN].label);
	Serial.print("RIT is "); Serial.println(bandmem[curr_band].RIT_en);
	draw_2_state_Button(RIT_BTN, &bandmem[curr_band].RIT_en);
}

void displayXIT()
{
	uint16_t x = 70;
	uint16_t y = 50; //75;
	uint16_t w = 40;
	uint16_t h = 20;
	
	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	if (bandmem[curr_band].XIT_en)
		tft.setTextColor(RA8875_GREEN);
	else
		tft.setTextColor(RA8875_LIGHT_ORANGE);
	//sprintf(std_btn[XIT_BTN].label, "%s", std_btn[XIT_BTN].label);
	tft.print(std_btn[XIT_BTN].label);
	Serial.print("XIT is "); Serial.println(bandmem[curr_band].XIT_en);
	draw_2_state_Button(XIT_BTN, &bandmem[curr_band].XIT_en);
}

void displayFine()
{
	uint16_t x = 136;
	uint16_t y = 50; //75;
	uint16_t w = 45;
	uint16_t h = 20;
	
	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	if (user_settings[user_Profile].fine)
		tft.setTextColor(RA8875_GREEN);
	else
		tft.setTextColor(RA8875_LIGHT_ORANGE);
	//sprintf(std_btn[FINE_BTN].label, "%s", std_btn[FINE_BTN].label);
	tft.print(std_btn[FINE_BTN].label);
	Serial.print("Fine Tune is "); Serial.println(user_settings[user_Profile].fine);
	draw_2_state_Button(FINE_BTN,  &user_settings[user_Profile].fine);
}

void displaySplit()
{
	char sp_label[15];
	uint16_t x = 220;
	uint16_t y = 63;
	uint16_t w = 90;
	uint16_t h = 20;
	
	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	if (bandmem[curr_band].split)
	{
		tft.setTextColor(RA8875_GREEN);
		sprintf(sp_label, "%s %s",  std_btn[SPLIT_BTN].label, ">>>");
	}
	else
	{
		tft.setTextColor(RA8875_LIGHT_ORANGE);
		sprintf(sp_label, "%s %s", std_btn[SPLIT_BTN].label, "Off");
	}
	tft.print(sp_label);
	Serial.print("Split is "); Serial.println(bandmem[curr_band].split);
	draw_2_state_Button(SPLIT_BTN, &bandmem[curr_band].split);
}

void displayNB()
{
	uint16_t x = 10;
	uint16_t y = 25; //45;
	uint16_t w = 40;
	uint16_t h = 20;
	
	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	if (user_settings[user_Profile].nb_en)
		tft.setTextColor(RA8875_GREEN);
	else
		tft.setTextColor(RA8875_LIGHT_ORANGE);
	//sprintf(std_btn[NB_BTN].label, "%s%1d", "NB", user_settings[user_Profile].nb_en);
	tft.print(std_btn[NB_BTN].label);
	Serial.print("NB is "); Serial.println(user_settings[user_Profile].nb_en);
	draw_2_state_Button(NB_BTN, &user_settings[user_Profile].nb_en);
}

void displayNR()
{
	uint16_t x = 70;
	uint16_t y = 25; //45;
	uint16_t w = 40;
	uint16_t h = 20;
	
	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	if (user_settings[user_Profile].nr_en)
		tft.setTextColor(RA8875_GREEN);
	else
		tft.setTextColor(RA8875_LIGHT_ORANGE);
	//sprintf(std_btn[NR_BTN].label, "%s%1d", "NR", user_settings[user_Profile].nr_en);
	tft.print(std_btn[NR_BTN].label);
	Serial.print("NR is "); Serial.println(user_settings[user_Profile].nr_en);
	draw_2_state_Button(NR_BTN, &user_settings[user_Profile].nr_en);
}

void displayNotch()
{
	uint16_t x = 130;
	uint16_t y = 25; //45;
	uint16_t w = 50;
	uint16_t h = 20;
	
	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_14);
	tft.setCursor(x+1, y+2 );
	if (user_settings[user_Profile].notch)
		tft.setTextColor(RA8875_GREEN);
	else
		tft.setTextColor(RA8875_LIGHT_ORANGE);
	//sprintf(std_btn[NOTCH_BTN].label, "%s%1d", "NTCH", user_settings[user_Profile].notch);
	tft.print(std_btn[NOTCH_BTN].label);
	Serial.print("Notch is "); Serial.println(std_btn[NOTCH_BTN].label);
	draw_2_state_Button(NOTCH_BTN,  &user_settings[user_Profile].notch);
}

void displayTime(void)
{	
	sprintf(std_btn[UTCTIME_BTN].label, "UTC:%02d:%02d:%02d", hour(), minute(), second());
	//tft.print(std_btn[UTCTIME_BTN].label);
	//Serial.println("UTC Time = "); Serial.println(std_btn[UTCTIME_BTN].label);
	draw_2_state_Button(UTCTIME_BTN, &std_btn[UTCTIME_BTN].show);	
}

void displayMenu(){draw_2_state_Button(MENU_BTN, &std_btn[MENU_BTN].enabled);}
void displayFn(){draw_2_state_Button(FN_BTN, &std_btn[FN_BTN].enabled);}
void displayVFO_AB(){draw_2_state_Button(VFO_AB_BTN, &bandmem[curr_band].VFO_AB_Active);}
void displayBandUp(){draw_2_state_Button(BANDUP_BTN, &bandmem[curr_band].band_num);}
void displayBand(){draw_2_state_Button(BAND_BTN, &bandmem[curr_band].band_num);}
void displaySpot(){draw_2_state_Button(SPOT_BTN,  &user_settings[user_Profile].spot);}

void displayBandDn(){draw_2_state_Button(BANDDN_BTN, &bandmem[curr_band].band_num);}
void displayDisplay(){draw_2_state_Button(DISPLAY_BTN, &display_state);}
void displayXMIT(){draw_2_state_Button(XMIT_BTN, &user_settings[user_Profile].xmit);}
void displayMute(){draw_2_state_Button(MUTE_BTN, &user_settings[user_Profile].mute);}

void displayXVTR(){draw_2_state_Button(XVTR_BTN, &bandmem[curr_band].xvtr_en);}
void displayEnet(){draw_2_state_Button(ENET_BTN, &user_settings[user_Profile].enet_output);}

//
//------------------------------------  drawButton ------------------------------------------------------------------------
//
//  Input:  1. Button ID
//          2. pointer to structure where state is stored
//
//  Usage:  The structure table has teh button properties.  Just pass the index and state.
//
//  Notes:  Only handles 2 states today.  
//
// TODO: change to handle multiple states such as RF gain, AF gain, or triple state, maybe info text like level.  Could be a new function.
//
void draw_2_state_Button(uint8_t button, uint8_t *function_ptr) 
{
    struct Standard_Button *ptr = std_btn + button;     // pointer to button object passed by calling function
	
	//if(ptr->enabled == 0)
	if(ptr->show == 0)
		return;

	if(*function_ptr > 0)
	{
		tft.fillRoundRect(ptr->bx,ptr->by,ptr->bw,ptr->bh,ptr->br,ptr->on_color);
		tft.drawRoundRect(ptr->bx,ptr->by,ptr->bw,ptr->bh,ptr->br,ptr->outline_color);
	}	
	else  //(*function_ptr == 0)
    {
  	    tft.fillRoundRect(ptr->bx,ptr->by,ptr->bw,ptr->bh, ptr->br, ptr->off_color );
		tft.drawRoundRect(ptr->bx,ptr->by,ptr->bw,ptr->bh,ptr->br,ptr->outline_color);
    }
    tft.setTextColor(ptr->txtclr);
	tft.setFont(Arial_18);
	tft.setTextColor(ptr->txtclr); 
    tft.setCursor(ptr->bx+ptr->padx,ptr->by+ptr->pady);
    tft.print(ptr->label);
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
	if (enet_ready)
		displayTime();
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
	//displayEnet();	
}

//
//    formatvfo()
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

