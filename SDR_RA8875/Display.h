#include <RA8875.h>
extern RA8875 tft;

extern uint8_t curr_band;   // global tracks our current band setting.  
extern volatile uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern volatile uint32_t VFOB;
extern struct Band_Memory bandmem[];
extern struct Bandwidth_Settings bw[];
extern struct Standard_Button std_btn[];
extern struct User_Settings user_settings[];
extern uint8_t user_Profile;

// function declaration
void draw_Std_Button(uint8_t button, uint8_t *function_ptr);

int displayColor = RA8875_LIGHT_GREY;
int textcolor = tft.Color565(128, 128, 128);

//////////////////////////////////////////////////////////////
void displayMode()
{
	tft.fillRect(19, 30, 100, 30, RA8875_BLACK );
	tft.setFont(Arial_18);
	tft.setCursor(20,36);
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	tft.print(Mode[bandmem[curr_band].mode]);    // use MODE{band][mode] to retrieve the text}
}

void displayBandwidth()
{
	tft.fillRect(129, 30, 160, 30, RA8875_BLACK);
	tft.setFont(Arial_18);
	tft.setCursor(130, 36 );
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	tft.print(bw[bandmem[curr_band].bandwidth].bw_name);   // use bw[current band.bandwidth index] to retrieve the text}
}

void displayFreq()
{ 
	tft.fillRect(305, 0, 215, 40, RA8875_BLACK);
	tft.setFont(Arial_32);
	tft.setCursor(306,36);
	tft.setTextColor(RA8875_LIGHT_ORANGE);
	tft.print(float(VFOA)/1000,3);
}

void displayStep()
{
	tft.fillRect(524, 30, 150, 30, RA8875_BLACK);
	tft.setFont(Arial_18);
	tft.setCursor(525,36);
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
}

void displayAttn()
{
    draw_Std_Button(ATTEN_BTN, &bandmem[curr_band].attenuator);
}

void displayPreamp()
{
    draw_Std_Button(PREAMP_BTN, &bandmem[curr_band].preamp);
}

void displayMute()
{
    draw_Std_Button(MUTE_BTN, &user_settings[user_Profile].mute);
}

void displayATU()
{
    draw_Std_Button(ATU_BTN, &bandmem[curr_band].ATU);
}

void displayAGC1()
{
    draw_Std_Button(AGC_BTN, &bandmem[curr_band].agc_mode);
}

void displaySplit()
{
    draw_Std_Button(SPLIT_BTN, &bandmem[curr_band].split);
}

void displayXVTR()
{
    draw_Std_Button(XVTR_BTN, &bandmem[curr_band].xvtr_en);
}


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
void draw_Std_Button(uint8_t button, uint8_t *function_ptr) 
{
    struct Standard_Button *ptr = std_btn + button;     // pointer to button object passed by calling function
	uint16_t bx     = ptr->bx;
	uint16_t by     = ptr->by;
	uint16_t bw     = ptr->bw;
	uint16_t bh     = ptr->bh;
	uint16_t br     = ptr->br;
    uint16_t ol_clr = ptr->outline_color;
    uint16_t txtclr = ptr->txtclr; 
    uint16_t on_clr = ptr->on_color;
    uint16_t off_clr= ptr->off_color;

	if(*function_ptr == 1)
		tft.fillRoundRect(bx,by,bw,bh,br,on_clr);
	if(*function_ptr == 0)
    {
  	    tft.fillRoundRect(bx,by,bw,bh, br, off_clr );
		tft.drawRoundRect(bx,by,bw,bh,br,ol_clr);
    }
    tft.setTextColor(txtclr);
	tft.setFont(Arial_18);
	tft.setTextColor(txtclr); 
    tft.setCursor(bx+10,by+20);
    tft.print(ptr->label);
}

 
