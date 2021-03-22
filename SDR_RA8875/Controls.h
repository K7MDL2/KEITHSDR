//
//    Controls.h
//
//    Core functions that control things.  The controller may be a touch event from a button or label object
//      or from a mechanical device or another function in a chain of interdepedencies.
//
//    Many controls have multipe possible controllers so have to adopt an Controller->control->display model
//      allowing parallel control requests.  
//      The control object changes (or requests to change) states, the display only scans and reports the state.
//      It becomes importan to pass through this as remote control and monitoring get built.

// Using the SV1AFN Band Pass Filter board with modified I2C library for Premp, Attenuator, and for 10 HF bands of BPFs
//#include "SVN1AFN_BandpassFilters.h>""
#ifdef SV1AFN_BPF
  extern SVN1AFN_BandpassFilters bpf;
#endif

extern uint8_t curr_band;   // global tracks our current band setting.  
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint32_t VFOB;
extern struct Band_Memory bandmem[];
extern struct User_Settings user_settings[];
extern struct Bandwidth_Settings bw[];
extern uint8_t user_Profile;
extern Metro popup_timer; // used to check for popup screen request
extern AudioControlSGTL5000 codec1;
extern uint8_t popup;
extern void RampVolume(float vol, int16_t rampType);

void Set_Spectrum_Scale(int8_t zoom_dir);
void Set_Spectrum_RefLvl(int8_t zoom_dir);
void Gesture_Handler(uint8_t gesture);
void changeBands(int8_t direction);
void pop_win(uint8_t init);
void Mute();
void Menu();
void Display();
void Band();
void BandDn();
void BandUp();
void Notch();
void Spot();
void Enet();
void NR();
void NB();
void Xmit();
void Ant();
void Fine();
void Rate(int direction);
void setMode();
void AGC();
void Filter();
void ATU();
void Xvtr();
void Split();
void XIT();
void RIT();
void Preamp();
void Atten();
void VFO_AB();
void PAtten_Set(uint8_t atten);

// Use gestures (pinch) to adjust the the vertical scaling.  This affects both watefall and spectrum.  YMMV :-)
void Set_Spectrum_Scale(int8_t zoom_dir)
{
    Serial.println(zoom_dir);
    //extern struct Spectrum_Parms Sp_Parms_Def[];    
    if (Sp_Parms_Def[spectrum_preset].spect_wf_scale > 2.0) 
        Sp_Parms_Def[spectrum_preset].spect_wf_scale = 0.5;
    if (Sp_Parms_Def[spectrum_preset].spect_wf_scale < 0.5)
        Sp_Parms_Def[spectrum_preset].spect_wf_scale = 2.0; 
    if (zoom_dir == 1)
    {
        Sp_Parms_Def[spectrum_preset].spect_wf_scale += 0.1;
        Serial.println("ZOOM IN");
    }
    else
    {        
        Sp_Parms_Def[spectrum_preset].spect_wf_scale -= 0.1;
        Serial.println("ZOOM OUT"); 
    }
    Serial.println(Sp_Parms_Def[spectrum_preset].spect_wf_scale);
}

// Use gestures to raise and lower the spectrum reference level relative to the bottom of the window (noise floor)
void Set_Spectrum_RefLvl(int8_t zoom_dir)
{
    Serial.println(zoom_dir);    
    
    if (zoom_dir == 1)
    {
        Sp_Parms_Def[spectrum_preset].spect_floor -= 10;
        Serial.print("RefLvl=UP");
    }        
    else
    {
        Sp_Parms_Def[spectrum_preset].spect_floor += 10;
        Serial.print("RefLvl=DOWN");
    }
    if (Sp_Parms_Def[spectrum_preset].spect_floor < -400)
        Sp_Parms_Def[spectrum_preset].spect_floor = -400; 
    if (Sp_Parms_Def[spectrum_preset].spect_floor > 400)
        Sp_Parms_Def[spectrum_preset].spect_floor = 400;
}
//
//----------------------------------- Skip to Ham Bands only ---------------------------------
//
// Increment band up or down from present.   To be used with touch or physical band UP/DN buttons.
// A alternate method (not in this function) is to use a band button or gesture to do a pop up selection map.  
// A rotary encoder can cycle through the choices and push to select or just touch the desired band.
//
//
// --------------------- Change bands using database -----------------------------------
// Returns 0 if cannot change bands
// Returns 1 if success

void changeBands(int8_t direction)  // neg value is down.  Can jump multiple bandswith value > 1.
{
    
    // TODO search bands column for match toaccount for mapping that does not start with 0 and bands could be in odd order and disabled.
    //Serial.print("\nCurrent Band is "); Serial.println(bandmem[curr_band].band_name);
    bandmem[curr_band].vfo_A_last = VFOA;
    bandmem[curr_band].vfo_B_last = VFOB;

    // Deal with transverters later probably increase BANDS count to cover all transverter bands to (if enabled).
    int8_t target_band = bandmem[curr_band + direction].band_num;
    
    Serial.print("Target Band is "); Serial.println(target_band);

    if (target_band > BAND9)    // go to bottom band
        target_band = BAND9;    // 0 is not used
    if (target_band < BAND0)    // go to top most band  -  
        target_band = BAND0;    // 0 is not used so do not have to adjsut with a -1 here

    Serial.print("Corrected Target Band is "); Serial.println(target_band);    
  
//TODO check if band is active and if not, skip down to next until we find one active in the bandmap    
    RampVolume(0.0f, 2);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    curr_band = target_band;    // Set out new band
    VFOA = bandmem[curr_band].vfo_A_last;  // up the last used frequencies
    VFOB = bandmem[curr_band].vfo_B_last;
    Serial.print("New Band is "); Serial.println(bandmem[curr_band].band_name);     
    delay(20);  // small delay for audio ramp to work
    selectFrequency(0);  // change band and preselector
    selectBandwidth(bandmem[curr_band].filter);
    selectMode(0);  // no change just set for the active VFO
    Rate(0);
    displayRefresh();
    selectAgc(bandmem[curr_band].agc_mode);
    RampVolume(1.0f, 2);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp" 
}

void pop_win(uint8_t init)
{
    if(init)
    {
        popup_timer.interval(300);
        tft.setActiveWindow(200, 600, 160, 360);
        tft.fillRoundRect(200,160, 400, 200, 20, RA8875_LIGHT_GREY);
        tft.drawRoundRect(200,160, 400, 200, 20, RA8875_RED);
        tft.setTextColor(RA8875_BLUE);
        tft.setCursor(CENTER, CENTER, true);
        tft.print("this is a future keyboard");
        delay(1000);
        tft.fillRoundRect(200,160, 400, 200, 20, RA8875_LIGHT_ORANGE);
        tft.drawRoundRect(200,160, 400, 200, 20, RA8875_RED);
        tft.setCursor(CENTER, CENTER, true);
        tft.print("Thanks for watching, GoodBye!");
        delay(600);
        popup = 0;
   // }
   // else 
   // {
        tft.fillRoundRect(200,160, 400, 290, 20, RA8875_BLACK);
        tft.setActiveWindow();
        popup = 0;   // resume our normal schedule broadcast
        popup_timer.interval(65000);
        drawSpectrumFrame(user_settings[user_Profile].sp_preset);
        displayRefresh();
    }
}

//
//  -----------------------   Button Functions --------------------------------------------
//   Called by Touch, Encoder, or Switch events
//
    
// MODE button
void setMode()
{
    selectMode(1);   // Increment the mode for the Active VFO 
    Serial.print("Set Mode");  
    displayMode();
}

// FILTER button
void Filter(int dir)
{ 
    static int direction = -1;
    int _bndx = bandmem[curr_band].filter; // Change Bandwidth  - cycle down then back to the top
    
    uint8_t _mode;

    // 1. Limit to allowed step range
    // 2. Cycle up and at top, cycle back down, do nto roll over.
    if (_bndx <= 0)
    {
        _bndx = -1;
        direction = 1;   // cycle upwards
    }

    if (_bndx >= FILTER-1)
    {
        _bndx = FILTER-1;
        direction = -1;
    }

    if (bandmem[curr_band].VFO_AB_Active == VFO_A)
        _mode = bandmem[curr_band].mode_A;
    else
        _mode = bandmem[curr_band].mode_B;

    if (_mode == CW)  // CW modes
    {
        if (_bndx > FILTER-1)   // go to bottom band   
        {
            _bndx = FILTER-1;
            direction = -1;     // cycle downwards
        }
        if (_bndx <=BW0_25)
        {
            _bndx = BW0_25;          
            direction = 1;      // cycle upwards
        }
    }
    else  // Non-CW modes
    {
        if (_bndx > FILTER-1)   // go to bottom band  
        { 
            _bndx = FILTER-1;    
            direction = -1;     // cycle downwards
        }
        if (_bndx <= BW1_8)
        {
            _bndx = BW1_8;
            direction = 1;      // cycle upwards
        }
    }

        if (dir == 0)
            _bndx += direction; // Index our step up or down
		else	
			_bndx += dir;  // forces a step higher or lower then current
		
    selectBandwidth(_bndx);
    Serial.print("Set Filter to ");
    Serial.println(bandmem[curr_band].filter);
    displayFilter();
}

// RATE button
void Rate(int swiped)
{
    static int direction = 1;
	int _fndx = bandmem[curr_band].tune_step;

	if (user_settings[user_Profile].fine == 0)
	{
		// 1. Limit to allowed step range
		// 2. Cycle up and at top, cycle back down, do nto roll over.
		if (_fndx <= 1)
		{
			_fndx = 1;
			direction = 1;   // cycle upwards
		}

		if (_fndx >= TS_STEPS-1)
		{
			_fndx = TS_STEPS-1;
			direction = -1;
		}
		
		if (swiped == 0)
			_fndx += direction; // Index our step up or down
		else	
			_fndx += swiped;  // forces a step higher or lower then current
		
		if (_fndx > TS_STEPS-1)   // ensure we are still in range
			_fndx = TS_STEPS - 1;  // just in case it over ranges, bad stuff happens when it does
		if (_fndx < 1)
			_fndx = 1;  // just in case it over ranges, bad stuff happens when it does		
	}
	if (user_settings[user_Profile].fine && swiped == -1)  // 1 Hz steps
		bandmem[curr_band].tune_step = 0;   // set to 1 hz steps
	else if (user_settings[user_Profile].fine && swiped == 1)
		bandmem[curr_band].tune_step = 1;    // normally swiped is +1 or -1
	else if (user_settings[user_Profile].fine && swiped == 0)
	{
		if (_fndx > 0)
			_fndx = 0;			
		else
			_fndx = 1;
		bandmem[curr_band].tune_step = _fndx;
	}
	else 
		bandmem[curr_band].tune_step = _fndx;  // Fine tunig mode is off, allow all steps 10hz and higher
        
    Serial.print("Set Rate to ");
    Serial.println(tstep[bandmem[curr_band].tune_step].ts_name);
    displayRate();
}

// AGC button
void AGC()
{
    selectAgc(bandmem[curr_band].agc_mode + 1);            
    Serial.print("Set AGC to ");
    Serial.println(bandmem[curr_band].agc_mode);            
    sprintf(std_btn[AGC_BTN].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
    sprintf(labels[AGC_LBL].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
    displayAgc();
}

// MUTE
void Mute()
{  
    if (user_settings[user_Profile].spkr_en)
    {
        if (!user_settings[user_Profile].mute)
        {
            RampVolume(0.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"           
            user_settings[user_Profile].mute = ON;
        }
        else    
        {    //codec1.muteHeadphone();
            RampVolume(1.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"                        
            user_settings[user_Profile].mute = OFF;        
        }
        displayMute();
    }        
}

// MENU
void Menu()
{   
    popup = 1;
    pop_win(1);
    Sp_Parms_Def[spectrum_preset].spect_wf_colortemp += 10;
    if (Sp_Parms_Def[spectrum_preset].spect_wf_colortemp > 10000)
        Sp_Parms_Def[spectrum_preset].spect_wf_colortemp = 1;              
    Serial.print("spectrum_wf_colortemp = ");
    Serial.println(Sp_Parms_Def[spectrum_preset].spect_wf_colortemp); 
    displayMenu();
    Serial.println("Menu Pressed");
}

// VFO A/B
void VFO_AB()
{
    if (bandmem[curr_band].VFO_AB_Active == VFO_A)
    {
        bandmem[curr_band].VFO_AB_Active = VFO_B;
    }
    else if (bandmem[curr_band].VFO_AB_Active == VFO_B)
    {
        bandmem[curr_band].VFO_AB_Active = VFO_A;
    }
    VFOA = bandmem[curr_band].vfo_A_last;
    VFOB = bandmem[curr_band].vfo_B_last;
    selectFrequency(0);
    selectMode(0);  // No change to mode, jsut set for active VFO
    displayVFO_AB();
    Serial.print("Set VFO_AB_Active to ");
    Serial.println(bandmem[curr_band].VFO_AB_Active,DEC);
}

// ATT
void Atten()
{   
    if (bandmem[curr_band].attenuator == ATTEN_ON)
        bandmem[curr_band].attenuator = ATTEN_OFF;
    else if (bandmem[curr_band].attenuator == ATTEN_OFF)
        bandmem[curr_band].attenuator = ATTEN_ON;
    #ifdef SV1AFN_BPF
      bpf.setAttenuator((bool) bandmem[curr_band].attenuator);
    #endif
    displayAttn();
    Serial.print("Set Attenuator to ");
    Serial.println(bandmem[curr_band].attenuator);
}

// PREAMP button
void Preamp()
{
    if (bandmem[curr_band].preamp == PREAMP_ON)
        bandmem[curr_band].preamp = PREAMP_OFF;
    else if (bandmem[curr_band].preamp == PREAMP_OFF)
        bandmem[curr_band].preamp = PREAMP_ON;
    #ifdef SV1AFN_BPF
      bpf.setPreamp((bool) bandmem[curr_band].preamp);
    #endif
    displayPreamp();
    Serial.print("Set Preamp to ");
    Serial.println(bandmem[curr_band].preamp);
}

// RIT button
void RIT()
{
    if (bandmem[curr_band].RIT_en == ON)
        bandmem[curr_band].RIT_en = OFF;
    else if (bandmem[curr_band].RIT_en == OFF)
        bandmem[curr_band].RIT_en = ON;
    displayRIT();
    Serial.print("Set RIT to ");
    Serial.println(bandmem[curr_band].RIT_en);
}
    
// XIT button
void XIT()
{
    if (bandmem[curr_band].XIT_en == ON)
        bandmem[curr_band].XIT_en = OFF;
    else if (bandmem[curr_band].XIT_en == OFF)
        bandmem[curr_band].XIT_en = ON;
    displayXIT();
    Serial.print("Set XIT to ");
    Serial.println(bandmem[curr_band].XIT_en);
}

// SPLIT button
void Split()
{
    if (bandmem[curr_band].split == ON)
        bandmem[curr_band].split = OFF;
    else if (bandmem[curr_band].split == OFF)
        bandmem[curr_band].split = ON;
    displaySplit();
    displayFreq();
    Serial.print("Set Split to ");
    Serial.println(bandmem[curr_band].split);

}

// XVTR button
void Xvtr()
{
    if (bandmem[curr_band].xvtr_en== ON)
        bandmem[curr_band].xvtr_en = OFF;
    else if (bandmem[curr_band].xvtr_en == OFF)
        bandmem[curr_band].xvtr_en = ON;
    displayXVTR();
    Serial.print("Set Xvtr Enable to ");
    Serial.println(bandmem[curr_band].xvtr_en);
}

// ATU button
void ATU()
{
    if (bandmem[curr_band].ATU== ON)
        bandmem[curr_band].ATU = OFF;
    else if (bandmem[curr_band].ATU == OFF)
        bandmem[curr_band].ATU = ON;
    displayATU();
    Serial.print("Set ATU to ");
    Serial.println(bandmem[curr_band].ATU);
}

// Fine button
void Fine()
{
    extern uint8_t enc_ppr_response;        

    if (user_settings[user_Profile].fine== ON)
    {
        user_settings[user_Profile].fine = OFF;
        enc_ppr_response /= 1.4;
    }
    else if (user_settings[user_Profile].fine == OFF)
    {
        user_settings[user_Profile].fine = ON;
        enc_ppr_response *= 1.4;
    }
    Rate(0);   
    displayFine();
    displayRate();
    
    Serial.print("Set Fine to ");
    Serial.println(user_settings[user_Profile].fine);
}

// ANT button
void Ant()
{
    if (bandmem[curr_band].ant_sw== 1)
        bandmem[curr_band].ant_sw = 2;
    else if (bandmem[curr_band].ant_sw == 2)
        bandmem[curr_band].ant_sw = 1;
    displayANT();
    Serial.print("Set Ant Sw to ");
    Serial.println(bandmem[curr_band].ant_sw);
}

// XMIT button
void Xmit()
{
    if (user_settings[user_Profile].xmit== ON)
        user_settings[user_Profile].xmit = OFF;
    else if (user_settings[user_Profile].xmit == OFF)
        user_settings[user_Profile].xmit = ON;
    displayXMIT();
    displayFreq();
    Serial.print("Set XMIT to ");
    Serial.println(user_settings[user_Profile].xmit);
}

// NB button
void NB()
{
    if (user_settings[user_Profile].nb_en > NBOFF)
        user_settings[user_Profile].nb_en = NBOFF;
    else if (user_settings[user_Profile].nb_en == NBOFF)
        user_settings[user_Profile].nb_en = NB1;
    displayNB();
    Serial.print("Set NB to ");
    Serial.println(user_settings[user_Profile].nb_en);
}
    
// NR button
void NR()
{
    if (user_settings[user_Profile].nr_en > NROFF)
        user_settings[user_Profile].nr_en = NROFF;
    else if (user_settings[user_Profile].nr_en == NROFF)
        user_settings[user_Profile].nr_en = NR1;
    displayNR();
    Serial.print("Set NR to ");
    Serial.println(user_settings[user_Profile].nr_en);
}

// Enet button
void Enet()
{
    if (user_settings[user_Profile].enet_output== ON)
        user_settings[user_Profile].enet_output = OFF;
    else if (user_settings[user_Profile].enet_output == OFF)
        user_settings[user_Profile].enet_output = ON;
    displayEnet();
    Serial.print("Set Ethernet to ");
    Serial.println(user_settings[user_Profile].enet_output);
}

// Spot button
void Spot()
{
    if (user_settings[user_Profile].spot== ON)
        user_settings[user_Profile].spot = OFF;
    else if (user_settings[user_Profile].spot == OFF)
        user_settings[user_Profile].spot = ON;
    displaySpot();
    Serial.print("Set Spot to ");
    Serial.println(user_settings[user_Profile].spot);
// adjust ref Floor   
Sp_Parms_Def[spectrum_preset].spect_floor += 5;
if (Sp_Parms_Def[spectrum_preset].spect_floor > -130)
    Sp_Parms_Def[spectrum_preset].spect_floor = -220; 
//Serial.println(Sp_Parms_Def[spectrum_preset].spect_floor);
}

// Notch button
void Notch()
{
    if (user_settings[user_Profile].notch== ON)
        user_settings[user_Profile].notch = OFF;
    else if (user_settings[user_Profile].notch == OFF)
        user_settings[user_Profile].notch = ON;
    displayNotch();
    Serial.print("Set Notch to ");
    Serial.println(user_settings[user_Profile].notch);
}

// BAND UP button
void BandUp()
{
    changeBands(1);
    displayBandUp();
    Serial.print("Set Band UP to ");
    Serial.println(bandmem[curr_band].band_num,DEC);
}

// BAND DOWN button
void BandDn()
{
    Serial.println("BAND DN");
    changeBands(-1);
    displayBandDn();
    Serial.print("Set Band DN to ");
    Serial.println(bandmem[curr_band].band_num,DEC);
}

// BAND button
void Band()
{
    popup = 1;
    pop_win(1);
    changeBands(1);  // increment up 1 band for now until the pop up windows buttons and/or MF are working
    displayBand();
    Serial.print("Set Band to ");
    Serial.println(bandmem[curr_band].band_num,DEC);
}

// DISPLAY button
void Display()
{   
    if (Sp_Parms_Def[spectrum_preset].spect_dot_bar_mode)
    {
        display_state = 0;
        Sp_Parms_Def[spectrum_preset].spect_dot_bar_mode = 0;
    }
    else 
    {
        display_state = 1;
        Sp_Parms_Def[spectrum_preset].spect_dot_bar_mode = 1;
    }
    popup = 1;
    pop_win(1);
    displayDisplay();
    Serial.print("Set Display Button to ");
    Serial.println(display_state);
}

/*******************************************************************************
* Function Name: PAtten_Set
********************************************************************************
*
* Summary:
* Main function performs following functions:
* 1: Configures the solid state atenuator by shifitng 16 bit of address and atten in LSB first.
* 
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void PAtten_Set(uint8_t atten)
{
    uint8_t   i;
    char    atten_str[17] = {'\0'};
    char    atten_str1[17] = {'\0'};
    char    atten_data[17] = {'\0'};
    char    addr[17] = {"00000000\0"};   /*   Board is fixed at addr = 000 */
    
    /* atten = CfgTblArr[Band_in_Use].Attenuator_dB;  */
    if(atten > 31) 
        atten = 31;
    if(atten <= 0)
        atten = 0;
    atten = atten * 4;
    /* Convert to 8 bits of  0 and 1 format */
    itoa(atten, atten_str, 2);
    
    /* pad with leading 0s as needed */
    for(i=0;(i<8-strlen(atten_str));i++)
    {
        atten_str1[i]='0';
    }
    strncat(atten_str1, atten_str, strlen(atten_str));
    
    /* Now build the string, address byte on left, atten byte on right (LSB) */
    strncat(atten_data, addr, 8);
    strncat(atten_data, atten_str1, 8);
          
    /*  LE = 0 to allow writing data into shift register */
    //PAtten_LE_Write(0);
    
    /*  Now loop for 16 bits, set data on Data pin and toggle Clock pin.  
        Start with the LSB first so start at the end of the string  */    
    for(i=16;i>0;--i)
    {
        //PAtten_Data_Write(atten_data[i-1]-48);   /*  convert ascii 0 or 1 to decimal 0 or 1 */
        //PAtten_Clock_Write(1);
        delay(1);
        //PAtten_Clock_Write(0); 
        delay(1);
    }
    
    /*  Toggle LE pin to latch the data and set the new attenuation value in the hardware (chip = Perigrine PE43703)  */
    //PAtten_LE_Write(1);
    delay(1);
    //PAtten_LE_Write(0);
    
    return;    
}