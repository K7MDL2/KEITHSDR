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
    
    Serial.print("\nTarget Band is "); Serial.println(target_band);

    if (target_band > BAND9)    // go to bottom band
        target_band = BAND9;    // 0 is not used
    if (target_band < BAND0)    // go to top most band  -  
        target_band = BAND0;    // 0 is not used so do not have to adjsut with a -1 here

    Serial.print("\nCorrected Target Band is "); Serial.println(target_band);    
  
//TODO check if band is active and if not, skip down to next until we find one active in the bandmap    
    RampVolume(0.0f, 2);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    curr_band = target_band;    // Set out new band
    VFOA = bandmem[curr_band].vfo_A_last;
    VFOB = bandmem[curr_band].vfo_B_last;
    Serial.print("New Band is "); Serial.println(bandmem[curr_band].band_name);     
    delay(20);
    selectFrequency(0); 
    selectBandwidth(bandmem[curr_band].filter);
    selectMode(0);  // no change just set for the active VFO
    selectStep(0);
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
void Filter()
{ 
    selectBandwidth(bandmem[curr_band].filter - 1); // Change Bandwidth  - cycle down then back to the top
    Serial.print("Set Filter to ");
    Serial.println(bandmem[curr_band].filter); 
    displayFilter();
}

// RATE button
void Rate(int direction)
{
    selectStep(direction);        
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
    selectStep(0);   //bandmem[curr_band].ant_sw = ON;   
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
