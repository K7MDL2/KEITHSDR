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
	struct Frequency_Display *ptr = disp_Freq;     // pointer to button object passed by calling function
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
	
	tft.fillRect(ptr->bx, ptr->by, ptr->bw, ptr->bh, ptr->bg_clr);

	// Write the Active VFO
	tft.setFont(ptr->vfoActive_LabelFont);
	tft.setCursor(ptr->bx, ptr->by+12);
	tft.setTextColor(ptr->vfoActive_clr);
	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
	{
		tft.print("A:");		
		tft.setFont(ptr->vfoActive_Font);
		tft.setCursor(ptr->bx + ptr->bm, ptr->by+6);		
		tft.print(formatVFO(VFOA));
	}
	else
	{
		tft.print("B:");		
		tft.setFont(ptr->vfoActive_Font);
		tft.setCursor(ptr->bx + ptr->bm, ptr->by+6);
		tft.print(formatVFO(VFOB));
	}
	
	// Write the standby VFO
	tft.setFont(ptr->vfoStandby_LabelFont);
	tft.setCursor(ptr->bx, ptr->by + ptr->bs);
	tft.setTextColor(ptr->vfoStandby_clr);	
	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
	{		
		tft.print("B:");
		tft.setFont(ptr->vfoStandby_Font);
		tft.setCursor(ptr->bx+10 + ptr->bm, ptr->by + ptr->bs-2);
		tft.print(formatVFO(VFOB));
	}
	else 
	{
		tft.print("A:");
		tft.setFont(ptr->vfoStandby_Font);
		tft.setCursor(ptr->bx+10 + ptr->bm, ptr->by + ptr->bs-2);
		tft.print(formatVFO(VFOA));
	}
}

void displayMode(void)
{
	char m_str[10];
	uint8_t mode;
	uint16_t x = 10;
	uint16_t y = 6;
	uint16_t w = 150;
	uint16_t h = 20;

	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_16);
	tft.setCursor(x+1, y+2 );
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
		mode = bandmem[curr_band].mode_A;
	else 
		mode = bandmem[curr_band].mode_B;
	sprintf(m_str, "M: %s", Mode[mode]);
	tft.print(m_str);    // use MODE{band][mode] to retrieve the text}
	sprintf(m_str, "M:%s", Mode[mode]);
	strcpy(std_btn[MODE_BTN].label, m_str);
	draw_2_state_Button(MODE_BTN, &mode);
}

void displayFilter(void)
{
	char filt_str[10];
	uint16_t x = 10;
	uint16_t y = 36;
	uint16_t w = 150;
	uint16_t h = 20;

	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_16);
	tft.setCursor(x+1, y+2 );
	tft.setTextColor(RA8875_LIGHT_ORANGE);	
	sprintf(filt_str, "F: %s%s", filter[bandmem[curr_band].filter].Filter_name,filter[bandmem[curr_band].filter].units);
	tft.print(filt_str);
	sprintf(std_btn[FILTER_BTN].label, "F:%s", filter[bandmem[curr_band].filter].Filter_name);
	draw_2_state_Button(FILTER_BTN, &bandmem[curr_band].filter);
}

void displayRate(void)
{
	char r_str[15];
	uint16_t x = 10;
	uint16_t y = 66;
	uint16_t w = 150;
	uint16_t h = 20;

	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_16);
	tft.setCursor(x+1, y+2);
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	sprintf(r_str, "R: %s%s", tstep[bandmem[curr_band].tune_step].ts_name, tstep[bandmem[curr_band].tune_step].ts_units);
	tft.print(r_str);
	sprintf(std_btn[RATE_BTN].label, "R:%s", tstep[bandmem[curr_band].tune_step].ts_name);
	draw_2_state_Button(RATE_BTN, &bandmem[curr_band].tune_step);
}

void displayAgc(void)
{
	uint16_t x = 10;
	uint16_t y = 96;
	uint16_t w = 100;
	uint16_t h = 20;

	tft.fillRect(x, y, w, h, RA8875_BLACK);
	tft.setFont(Arial_16);
	tft.setCursor(x+1, y+2 );
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	sprintf(std_btn[AGC_BTN].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
	tft.print(std_btn[AGC_BTN].label);
	Serial.print("AGC ="); Serial.println(std_btn[AGC_BTN].label);
	draw_2_state_Button(AGC_BTN, &bandmem[curr_band].agc_mode);
}

void displayTime(void)
{	
	static char time_str[20];
	//uint16_t x = 640;
	//uint16_t y = 6;
	//uint16_t w = 160;
	//uint16_t h = 20;

	//tft.fillRect(x, y, w, h, RA8875_BLACK);
	//tft.setFont(Arial_16);
	//tft.setCursor(x+1, y+2 );
	//tft.setTextColor(RA8875_LIGHT_GREY);
	sprintf(time_str, "UTC:%02d:%02d:%02d", hour(), minute(), second());
	//sprintf(time_str, "UTC:%02d:%02d:%02d", NTP_hour, NTP_min, NTP_sec);
	//Serial.print("UTC Time = "); Serial.println(time_str);
	sprintf(std_btn[UTCTIME_BTN].label, time_str);
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
void displayFine(){draw_2_state_Button(FINE_BTN,  &user_settings[user_Profile].fine);}
void displayNotch(){draw_2_state_Button(NOTCH_BTN,  &user_settings[user_Profile].notch);}
void displayBandDn(){draw_2_state_Button(BANDDN_BTN, &bandmem[curr_band].band_num);}
void displayDisplay(){draw_2_state_Button(DISPLAY_BTN, &display_state);}
void displayXMIT(){draw_2_state_Button(XMIT_BTN, &user_settings[user_Profile].xmit);}
void displayRIT(){draw_2_state_Button(RIT_BTN, &bandmem[curr_band].RIT_en);}
void displayXIT(){draw_2_state_Button(XIT_BTN, &bandmem[curr_band].XIT_en);}
void displayAttn(){draw_2_state_Button(ATTEN_BTN, &bandmem[curr_band].attenuator);}
void displayPreamp(){draw_2_state_Button(PREAMP_BTN, &bandmem[curr_band].preamp);}
void displayMute(){draw_2_state_Button(MUTE_BTN, &user_settings[user_Profile].mute);}
void displayATU(){draw_2_state_Button(ATU_BTN, &bandmem[curr_band].ATU);}
void displayNB(){draw_2_state_Button(NB_BTN, &user_settings[user_Profile].nb_en);}
void displayNR(){draw_2_state_Button(NR_BTN, &user_settings[user_Profile].nr_en);}
void displaySplit(){draw_2_state_Button(SPLIT_BTN, &bandmem[curr_band].split);}
void displayXVTR(){draw_2_state_Button(XVTR_BTN, &bandmem[curr_band].xvtr_en);}
void displayEnet(){draw_2_state_Button(ENET_BTN, &user_settings[user_Profile].enet_output);}
void displayANT(){draw_2_state_Button(ANT_BTN, &bandmem[curr_band].ant_sw);}
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
	
	if(ptr->enabled == 0)
		return;

	if(*function_ptr == 1)
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
	static char vfo_str[15];
	
	uint16_t MHz = (vfo/1000000 % 1000000);
	uint16_t Hz  = (vfo % 1000);
	uint16_t KHz = ((vfo % 1000000) - Hz)/1000;
	sprintf(vfo_str, "%d.%03d.%03d", MHz, KHz, Hz);
	//Serial.print("New VFO: ");Serial.println(vfo_str);
	return vfo_str;
}
