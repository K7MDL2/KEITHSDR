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
void displayAGC1();
void refreshScreen(void);
const char * formatVFO(uint32_t vfo);

int displayColor = RA8875_LIGHT_GREY;
int textcolor = tft.Color565(128, 128, 128);

//////////////////////////////////////////////////////////////
void displayMode()
{
	int mode;
	tft.fillRect(19, 20, 100, 30, RA8875_BLACK );
	tft.setFont(Arial_18);
	tft.setCursor(20,26);
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
		mode = bandmem[curr_band].mode_A;
	else 
		mode = bandmem[curr_band].mode_B;
	tft.print(Mode[mode]);    // use MODE{band][mode] to retrieve the text}
}

void displayBandwidth()
{
	tft.fillRect(170, 20, 160, 30, RA8875_BLACK);
	tft.setFont(Arial_18);
	tft.setCursor(170, 26 );
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	tft.print(bw[bandmem[curr_band].bandwidth].bw_name);   // use bw[current band.bandwidth index] to retrieve the text}
}

void displayFreq()
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
		tft.setCursor(ptr->bx+28, ptr->by+6);		
		tft.print(formatVFO(VFOA));
	}
	else
	{
		tft.print("B:");		
		tft.setFont(ptr->vfoActive_Font);
		tft.setCursor(ptr->bx+28, ptr->by+6);
		tft.print(formatVFO(VFOB));
	}
	
	// Write the standby VFO
	tft.setFont(ptr->vfoStandby_LabelFont);
	tft.setCursor(ptr->bx, ptr->by+58);
	tft.setTextColor(ptr->vfoStandby_clr);	
	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
	{		
		tft.print("B:");
		tft.setFont(ptr->vfoStandby_Font);
		tft.setCursor(ptr->bx+28, ptr->by+56);
		tft.print(formatVFO(VFOB));
	}
	else 
	{
		tft.print("A:");
		tft.setFont(ptr->vfoStandby_Font);
		tft.setCursor(ptr->bx+28, ptr->by+56);
		tft.print(formatVFO(VFOA));
	}
}

void displayRate()
{
	tft.fillRect(170, 80, 150, 30, RA8875_BLACK);
	tft.setFont(Arial_18);
	tft.setCursor(170,80);
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	tft.print(tstep[bandmem[curr_band].tune_step].ts_name);
}

void displayAgc()
{
	tft.fillRect(19, 70, 120, 40, RA8875_BLACK);
	tft.setFont(Arial_18);
	tft.setCursor(20,80);
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	tft.print(agc_set[bandmem[curr_band].agc_mode].agc_name);
	displayAGC1();
}

void displayMenu(){draw_2_state_Button(MENU_BTN, &std_btn[MENU_BTN].enabled);}
void displayFn(){draw_2_state_Button(FN_BTN, &std_btn[FN_BTN].enabled);}
void displayVFO_AB(){draw_2_state_Button(VFO_AB_BTN, &bandmem[curr_band].VFO_AB_Active);}
void displayBandUp(){draw_2_state_Button(BANDUP_BTN, &bandmem[curr_band].band_num);}
void displayBand(){draw_2_state_Button(BAND_BTN, &bandmem[curr_band].band_num);}
void displayBandDn(){draw_2_state_Button(BANDDN_BTN, &bandmem[curr_band].band_num);}
void displayDisplay(){draw_2_state_Button(DISPLAY_BTN, &display_state);}
void displayRIT(){draw_2_state_Button(RIT_BTN, &bandmem[curr_band].RIT_en);}
void displayAttn(){draw_2_state_Button(ATTEN_BTN, &bandmem[curr_band].attenuator);}
void displayPreamp(){draw_2_state_Button(PREAMP_BTN, &bandmem[curr_band].preamp);}
void displayMute(){draw_2_state_Button(MUTE_BTN, &user_settings[user_Profile].mute);}
void displayATU(){draw_2_state_Button(ATU_BTN, &bandmem[curr_band].ATU);}
void displayNB(){draw_2_state_Button(NB_BTN, &bandmem[curr_band].nb_en);}
void displayAGC1(){draw_2_state_Button(AGC_BTN, &bandmem[curr_band].agc_mode);}
void displaySplit(){draw_2_state_Button(SPLIT_BTN, &bandmem[curr_band].split);}
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
//  Input: 1. fn=0 is normal value to refresh all objects. 
// 		   2. fn=1 to refresh all but the displayFn() since that is the one function that 
// 		        calls this routine and would result in a circular loop.
// 
void displayRefresh(uint8_t fn)
{
    displayFreq();    // display frequency
	displayBandwidth();
	displayMode();
    displayAgc();
	displayRate();
	// Bottom Panel Anchor buttons
	displayMenu();
	displayMute();
	if (!fn) displayFn();   // make fn=1 to call displayFn() to prevent calling itself
	// Panel 1 buttons
	displayDisplay();
	displayRIT();
	displayBandUp();
	displayBandDn();
	//Panel 2 buttons
	displayAttn();
	displayPreamp();
	displaySplit();
	displayBand();
	//Panel 3 buttons
	displayATU();
	displayNB();
	displayXVTR();
	displayEnet();
	
	
	//displayVFO_AB();

    //displayAGC1();
}
 
const char* formatVFO(uint32_t vfo)
{
	static char vfo_str[15];
	
	uint32_t MHz = (vfo/1000000 % 1000000);
	uint32_t Hz  = (vfo % 1000);
	uint32_t KHz = ((vfo % 1000000) - Hz)/1000;
	sprintf(vfo_str, "%d.%03d.%03d", MHz, KHz, Hz);
	Serial.print("New VFO: ");Serial.println(vfo_str);
	return vfo_str;
}
