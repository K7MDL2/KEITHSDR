//
//      Controls.cpp
//

#include "SDR_RA8875.h"
#include "RadioConfig.h"

// Using the SV1AFN Band Pass Filter board with modified I2C library for Premp, Attenuator, and for 10 HF bands of BPFs
//#include "SVN1AFN_BandpassFilters.h>""
#ifdef SV1AFN_BPF
  #include <SVN1AFN_BandpassFilters.h>
  extern SVN1AFN_BandpassFilters bpf;
#endif

#ifdef USE_RA8875
	extern RA8875 tft;
#else 
	extern RA8876_t3 tft;
#endif
#ifndef BYPASS_SPECTRUM_MODULE
    extern Spectrum_RA887x spectrum_RA887x;    // Spectrum Display Libary
#endif
extern          uint8_t             display_state;   // something to hold the button state for the display pop-up window later.
extern          uint8_t             curr_band;   // global tracks our current band setting.  
extern          uint32_t            VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern          uint32_t            VFOB;
extern struct   Modes_List          modeList[];
extern struct   Band_Memory         bandmem[];
extern struct   User_Settings       user_settings[];
extern struct   Standard_Button     std_btn[];
extern struct   Label               labels[];
extern struct   Filter_Settings     filter[];
extern struct   AGC                 agc_set[];
extern struct   NB                  nb[];
extern struct   Spectrum_Parms      Sp_Parms_Def[];
extern          uint8_t             user_Profile;
extern          Metro               popup_timer; // used to check for popup screen request
extern AudioControlSGTL5000         codec1;
extern radioNoiseBlanker_F32        NoiseBlanker;
extern AudioEffectCompressor2_F32   compressor1; // Audio Compressor
extern AudioEffectCompressor2_F32   compressor2; // Audio Compressor
extern          uint8_t             popup;
extern          void                RampVolume(float vol, int16_t rampType);
extern volatile int32_t             Freq_Peak;
extern          void                set_MF_Service(uint8_t client_name);
extern          void                unset_MF_Service(uint8_t client_name);
extern          uint8_t             MF_client; // Flag for current owner of MF knob services
extern          float               fft_bin_size;       // = sample_rate_Hz/(FFT_SIZE*2) -  Size of FFT bin in Hz
extern          void                touchBeep(bool enable);
extern          bool                MeterInUse;  // S-meter flag to block updates while the MF knob has control
extern          Metro               MF_Timeout;
extern          bool                MF_default_is_active;
extern          void                TXAudio(int TX);
extern          int32_t 		    ModeOffset;

void Set_Spectrum_Scale(int8_t zoom_dir);
void Set_Spectrum_RefLvl(int8_t zoom_dir);
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
void NB(int8_t toggle);
void Xmit();
void Ant();
void Fine();
void Rate(int8_t direction);
void setMode(int8_t dir);
void AGC();
void Filter(int dir);
void ATU();
void Xvtr();
void Split();
void XIT();
void RIT();
void Preamp(int8_t toggle);
void Atten(int8_t toggle);
void VFO_AB();
void setAtten_dB(int8_t atten);
void setAFgain(int8_t toggle);
void AFgain(int8_t delta);
void setRFgain(int8_t toggle);
void RFgain(int8_t delta);
void setRefLevel(int8_t toggle);
void setNBLevel(int8_t delta);
void RefLevel(int8_t newval);
void TouchTune(int16_t touch_Freq);
void selectStep(uint8_t fndx);
void selectAgc(uint8_t andx);
void clearMeter(void);
void setMeter(uint8_t id);

#ifndef BYPASS_SPECTRUM_MODULE
// Use gestures (pinch) to adjust the the vertical scaling.  This affects both watefall and spectrum.  YMMV :-)
COLD void Set_Spectrum_Scale(int8_t zoom_dir)
{
    //Serial.println(zoom_dir);
    //extern struct Spectrum_Parms Sp_Parms_Def[];    
    //extern Spectrum_RA887x spectrum_RA887x;    // Spectrum Display Libary
    if (Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale > 2.0) 
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale = 0.5;
    if (Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale < 0.5)
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale = 2.0; 
    if (zoom_dir == 1)
    {
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale += 0.1;
        //Serial.println("ZOOM IN");
    }
    else
    {        
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale -= 0.1;
        //Serial.println("ZOOM OUT"); 
    }
    //Serial.println(Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale);
}

// Use gestures to raise and lower the spectrum reference level relative to the bottom of the window (noise floor)
COLD void Set_Spectrum_RefLvl(int8_t zoom_dir)
{
    //Serial.println(zoom_dir);    
    
    if (zoom_dir == 1)
    {
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor -= 1;
        //Serial.print("RefLvl=UP");
    }        
    else
    {
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor += 1;
        //Serial.print("RefLvl=DOWN");
    }
    if (Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor < -400)
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor = -400; 
    if (Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor > 400)
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor = 400;
}
#endif
//
//----------------------------------- Skip to Ham Bands only ---------------------------------
//
// Increment band up or down from present.   To be used with touch or physical band UP/DN buttons.
// A alternate method (not in this function) is to use a band button or gesture to do a pop up selection map.  
// A rotary encoder can cycle through the choices and push to select or just touch the desired band.
//
// --------------------- Change bands using database -----------------------------------
// Returns 0 if cannot change bands
// Returns 1 if success

COLD void changeBands(int8_t direction)  // neg value is down.  Can jump multiple bandswith value > 1.
{
    // TODO search bands column for match to account for mapping that does not start with 0 and bands could be in odd order and disabled.
    //Serial.print("\nCurrent Band is "); Serial.println(bandmem[curr_band].band_name);
    bandmem[curr_band].vfo_A_last = VFOA;
    bandmem[curr_band].vfo_B_last = VFOB;

    // Deal with transverters later probably increase BANDS count to cover all transverter bands to (if enabled).
    int8_t target_band = bandmem[curr_band].band_num + direction;
    
    //Serial.print("Target Band is "); Serial.println(target_band);

    if (target_band > BAND11)    // go to bottom band
        target_band = BAND11;    // 0 is not used
    if (target_band < BAND0)    // go to top most band  -  
        target_band = BAND0;    // 0 is not used so do not have to adjsut with a -1 here

    //Serial.print("Corrected Target Band is "); Serial.println(target_band);    
  
//TODO check if band is active and if not, skip down to next until we find one active in the bandmap    
    ///RampVolume(0.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    #ifndef PANADAPTER    
        curr_band = target_band;    // Set out new band
    #endif
    VFOA = bandmem[curr_band].vfo_A_last;  // up the last used frequencies
    VFOB = bandmem[curr_band].vfo_B_last;
    
    //Serial.print("New Band is "); Serial.println(bandmem[curr_band].band_name);     
    // delay(20);  // small delay for audio ramp to work
    selectFrequency(0);  // change band and preselector
    Atten(-1);      // -1 sets to database state. 2 is toggle state. 0 and 1 are Off and On.  Operate relays if any.
    selectBandwidth(bandmem[curr_band].filter);
     //dB level is set elsewhere and uses value in the dB in this function.
    Preamp(-1);     // -1 sets to database state. 2 is toggle state. 0 and 1 are Off and On.  Operate relays if any.
    //selectMode(0);  
    setMode(0);     // 0 is set value in database for both VFOs
    RefLevel(0);    // 0 just updates things to be current value
    RFgain(0);
    ///AFgain(0);
    setNBLevel(0);
#ifndef BYPASS_SPECTRUM_MODULE
    spectrum_RA887x.drawSpectrumFrame(user_settings[user_Profile].sp_preset);
#endif
    //Rate(0); Not needed
    //Ant() when there is hardware to setup in the future
    //ATU() when there is hardware to setup in the future
    //
    //   insert any future features, software or hardware, that need to be altered      
    //
    selectAgc(bandmem[curr_band].agc_mode);
    displayRefresh();
    ///RampVolume(1.0f, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp" 
    Atten(-1);      // -1 sets to database state. 2 is toggle state. 0 and 1 are Off and On.  Operate relays if any.
    //AFgain(0);  // Set RX audio level back to last position on RX
}

COLD void pop_win(uint8_t init)
{
    return;
    if(init)
    {
        popup_timer.interval(300);
        #ifdef USE_RA8875
            tft.setActiveWindow(200, 600, 160, 360);
            tft.fillRoundRect(200,160, 400, 200, 20, RA8875_LIGHT_GREY);
            tft.drawRoundRect(200,160, 400, 200, 20, RA8875_RED);
        #else
            tft.activeWindowXY(200, 160);
            tft.activeWindowWH(400,200);
            tft.fillRoundRect(200,160, 400, 200, 20, 20, RA8875_LIGHT_GREY);
            tft.drawRoundRect(200,160, 400, 200, 20, 20, RA8875_RED);
        #endif
        tft.setTextColor(RA8875_BLUE);
        tft.setCursor(CENTER, CENTER, true);
        tft.print(F("this is a future keyboard"));
        delay(1000);
        #ifdef USE_RA8875
            tft.fillRoundRect(200,160, 400, 200, 20, RA8875_LIGHT_ORANGE);
            tft.drawRoundRect(200,160, 400, 200, 20, RA8875_RED);
        #else
            tft.fillRoundRect(200,160, 400, 200, 20, 20, RA8875_LIGHT_ORANGE);
            tft.drawRoundRect(200,160, 400, 200, 20, 20, RA8875_RED);
        #endif
        tft.setCursor(CENTER, CENTER, true);
        tft.print(F("Thanks for watching, GoodBye!"));
        delay(600);
        popup = 0;
   // }
   // else 
   // {
        #ifdef USE_RA8875
            tft.fillRoundRect(200,160, 400, 290, 20, RA8875_BLACK);
            tft.setActiveWindow();
        #else
            tft.fillRoundRect(200, 160, 400, 290, 20, 20, RA8875_BLACK);
            tft.activeWindowXY(200,160);
            tft.activeWindowWH(400,290);
        #endif

        popup = 0;   // resume our normal schedule broadcast
        popup_timer.interval(65000);
#ifndef BYPASS_SPECTRUM_MODULE        
        spectrum_RA887x.drawSpectrumFrame(user_settings[user_Profile].sp_preset);
#endif        
        displayRefresh();
    }
}

//
//  -----------------------   Button Functions --------------------------------------------
//   Called by Touch, Encoder, or Switch events

// ---------------------------Mode() ----------------------------------
//   Input: 0 = set to current value in the database (circular rotation through all modes)
//          1 = increment the mode
//         -1 = decrement the mode
    
// MODE button
COLD void setMode(int8_t dir)
{
	uint8_t mndx;

	if (bandmem[curr_band].VFO_AB_Active == VFO_A)  // get Active VFO mode
		mndx = bandmem[curr_band].mode_A;			
	else
		mndx = bandmem[curr_band].mode_B;
	
	mndx += dir; // Make the change

  	if (mndx > FM)		// Validate change and fix if needed
   		mndx=0;
	if (mndx < CW)
		mndx = CW;

    selectMode(mndx);   // Select the mode for the Active VFO 
    
	if (bandmem[curr_band].VFO_AB_Active == VFO_A)   // Store our mode for the active VFO
		bandmem[curr_band].mode_A = mndx;
	else	
		bandmem[curr_band].mode_B = mndx;    
    
    // Update the filter setting per mode 
    Filter(2);
    //Serial.println("Set Mode");  
    displayMode();
    selectFrequency(0);  // Call in case a mode change requires a frequency offset
}

// ---------------------------Filter() ---------------------------
//   Input: 0 = step to next based on last direction (starts upwards).  Ramps up then down then up.
//          1 = step up 1 filter (wider)
//         -1 = step down 1 filter (narrower)
//          2 = use last filter width used in this mode (from modeList table)
//      This is mode-aware. In non-CW modes we will only cycle through SSB width filters as set in the filter tables

// FILTER button
COLD void Filter(int dir)
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

    if (_mode == CW || _mode == CW_REV)  // CW modes
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
    
    if (dir == 2) // Use last filter width used in this mode, ignore the rest (hopefully it is valid)
    {    
        if (modeList[_mode].Width <= BW4_0 && modeList[_mode].Width >= 0)
            _bndx = modeList[_mode].Width;
    }
    else
        modeList[_mode].Width = _bndx;  //if filter changed without a mode change, store it in last use per mode table.

    selectBandwidth(_bndx);
    //Serial.print("Set Filter to ");
    //Serial.println(bandmem[curr_band].filter);
    displayFilter();
}

// ---------------------------Rate() ---------------------------
//   Input: 0 = step to next based on last direction (starts upwards).  Ramps up then down then up.
//          1 = step up 1 tune rate
//         -1 = step down 1 tune step rate
//      If FINE is OFF, we will not use 1Hz. If FINE = ON we only use 1 and 10Hz steps.

// RATE button
COLD void Rate(int8_t swiped)
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

		if (_fndx >= 3)   //TS_STEPS-1)
		{
			_fndx = 3;  //TS_STEPS-1;
			direction = -1;
		}
		
		if (swiped == 0)
			_fndx += direction; // Index our step up or down
		else	
			_fndx += swiped;  // forces a step higher or lower then current
		
		if (_fndx >= 3)  //TS_STEPS-1)   // ensure we are still in range
			_fndx = 3;  //TS_STEPS - 1;  // just in case it over ranges, bad stuff happens when it does
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
     
    //Serial.print("Set Rate to ");
    //Serial.println(tstep[bandmem[curr_band].tune_step].ts_name);
    displayRate();
}

// AGC button
COLD void AGC()
{
    selectAgc(bandmem[curr_band].agc_mode + 1);            
    //Serial.print("Set AGC to ");
    //Serial.println(bandmem[curr_band].agc_mode);            
    sprintf(std_btn[AGC_BTN].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
    sprintf(labels[AGC_LBL].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
    displayAgc();
}

// MUTE
COLD void Mute()
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
COLD void Menu()
{   
    //popup = 1;
    //pop_win(1);
#ifndef BYPASS_SPECTRUM_MODULE
    Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_colortemp += 10;
    if (Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_colortemp > 10000)
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_colortemp = 1;              
#endif
    //Serial.print("spectrum_wf_colortemp = ");
    //Serial.println(Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_colortemp); 
    displayMenu();
    //Serial.println("Menu Pressed");
}

// VFO A/B
COLD void VFO_AB()
{
    // feedback beep
    touchBeep(true);  // a timer will shut it off.

    if (bandmem[curr_band].VFO_AB_Active == VFO_A)
    {
        bandmem[curr_band].VFO_AB_Active = VFO_B;
        selectMode(bandmem[curr_band].mode_B);
    }
    else if (bandmem[curr_band].VFO_AB_Active == VFO_B)
    {
        bandmem[curr_band].VFO_AB_Active = VFO_A;
        selectMode(bandmem[curr_band].mode_A);
    }
    VFOA = bandmem[curr_band].vfo_A_last;
    VFOB = bandmem[curr_band].vfo_B_last;
    selectFrequency(0);
    displayVFO_AB();
    displayMode();
    //Serial.print("Set VFO_AB_Active to ");
    //Serial.println(bandmem[curr_band].VFO_AB_Active,DEC);
}

// ATT
//   toogle = -1 sets attenuator state to current database value. Used for startup or changing bands.
//   toogle = 0 sets attenuator state off
//   toogle = 1 sets attenuator state on
//   toogle = 2 toggles attenuator state
//   dB = 1-31.  Set att relay on and set attenuattion level.  0-31 is a valid range to set but we will only use 1-31
//
COLD void Atten(int8_t toggle)
{
    Serial.print("toggle = "); Serial.println(toggle);
    // Set the attenuation level from the value in the database
    #ifdef DIG_STEP_ATT 
      setAtten_dB(bandmem[curr_band].attenuator_dB);  // set attenuator level to value in database for this band
    #endif   

    if (toggle == 2)    // toggle if ordered, else just set to current state such as for startup.
    {
        if (bandmem[curr_band].attenuator)  // toggle the attenuator tracking state
            toggle = 0;
        else 
            toggle = 1;
    }
    
    if (toggle == 1)    // toggle is 1, turn on Atten
    {
        if (bandmem[curr_band].attenuator == ATTEN_ON && MeterInUse)  // if already on, assume this was called by a log press and we want to turn off the meter and but not the feature.
            clearMeter();
        else
        {
            bandmem[curr_band].attenuator = ATTEN_ON;  // le the attenuator tracking state to ON
            setMeter(ATTEN_BTN);
        }
    }

    if (toggle == 0) // || toggle == -1)
    {   
        bandmem[curr_band].attenuator = ATTEN_OFF;  // set attenuator tracking state to OFF
        clearMeter(); 
    }
    
    #ifdef SV1AFN_BPF
      //if (bandmem[curr_band].attenuator == ATTEN_OFF)
        //Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor += bandmem[curr_band].attenuator_dB;  // reset back to normal
      //else 
        //Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor -= bandmem[curr_band].attenuator_dB;  // raise floor up due to reduced signal levels coming in

      RampVolume(0.0, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
      bpf.setAttenuator((bool) bandmem[curr_band].attenuator);  // Turn attenuator relay on or off
      RampVolume(1.0, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    #endif

    displayAttn();
    //Serial.print("Set Attenuator Relay to ");
    //Serial.print(bandmem[curr_band].attenuator);
    //Serial.print(" Atten_dB is ");
    //Serial.print(bandmem[curr_band].attenuator_dB);
    //Serial.print(" and Ref Level is ");
    //Serial.println(Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor);
}

// PREAMP button
//   toogle = 0 sets Preamp state off
//   toogle = 1 sets Preamp state on
//   toogle = 2 toggles Preamp state
//   toogle = -1 or any value other than 0-2 sets Preamp state to current database value. Used for startup or changing bands.
//
COLD void Preamp(int8_t toggle)
{    
    if (toggle == 2)    // toggle state
    {
        if (bandmem[curr_band].preamp == PREAMP_ON)
            bandmem[curr_band].preamp = PREAMP_OFF;
        else 
            bandmem[curr_band].preamp = PREAMP_ON;
    }
    else if (toggle == 1)  // set to ON
        bandmem[curr_band].preamp = PREAMP_ON;
    else if (toggle == 0)  // set to OFF
        bandmem[curr_band].preamp = PREAMP_OFF;
    // any other value of toggle pass through with unchanged state, jsut set the relays to current state
    
    #ifdef SV1AFN_BPF
      //if (bandmem[curr_band].preamp == PREAMP_OFF)
      //  Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor -= 30;  // reset back to normal
      //else 
      //  Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor += 30;  // lower floor due to increased signal levels coming in
 
      RampVolume(0.0, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
      bpf.setPreamp((bool) bandmem[curr_band].preamp);
      RampVolume(1.0, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    #endif
    
    displayPreamp();
    //Serial.print("Set Preamp to ");
    //Serial.println(bandmem[curr_band].preamp);
}

// RIT button
COLD void RIT()
{
    if (bandmem[curr_band].RIT_en == ON)
        bandmem[curr_band].RIT_en = OFF;
    else if (bandmem[curr_band].RIT_en == OFF)
        bandmem[curr_band].RIT_en = ON;
    displayRIT();
    //Serial.print("Set RIT to ");
    //Serial.println(bandmem[curr_band].RIT_en);
}
    
// XIT button
COLD void XIT()
{
    if (bandmem[curr_band].XIT_en == ON)
        bandmem[curr_band].XIT_en = OFF;
    else if (bandmem[curr_band].XIT_en == OFF)
        bandmem[curr_band].XIT_en = ON;
    displayXIT();
    //Serial.print("Set XIT to ");
    //Serial.println(bandmem[curr_band].XIT_en);
}

// SPLIT button
COLD void Split()
{
    if (bandmem[curr_band].split == ON)
        bandmem[curr_band].split = OFF;
    else if (bandmem[curr_band].split == OFF)
        bandmem[curr_band].split = ON;
    displaySplit();
    displayFreq();
    //Serial.print("Set Split to ");
    //Serial.println(bandmem[curr_band].split);

}

// XVTR button
COLD void Xvtr()
{
    if (bandmem[curr_band].xvtr_en == ON)
        bandmem[curr_band].xvtr_en = OFF;
    else if (bandmem[curr_band].xvtr_en == OFF)
        bandmem[curr_band].xvtr_en = ON;
    //
    //   Insert any future hardware setup calls
    //
    displayXVTR();
    //Serial.print("Set Xvtr Enable to ");
    //Serial.println(bandmem[curr_band].xvtr_en);
}

// ATU button
COLD void ATU()
{    
    if (bandmem[curr_band].ATU == ON)
        bandmem[curr_band].ATU = OFF;
    else if (bandmem[curr_band].ATU == OFF)
        bandmem[curr_band].ATU = ON;    
    //
    //   Insert any future ATU hardware setup calls
    //
    displayATU();
    //Serial.print("Set ATU to ");
    //Serial.println(bandmem[curr_band].ATU);
}

// Fine button
COLD void Fine()
{
    extern uint8_t enc_ppr_response;        

    if (user_settings[user_Profile].fine == ON)
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
    
    //Serial.print("Set Fine to ");
    //Serial.println(user_settings[user_Profile].fine);
}

// ANT button
COLD void Ant()
{
    if (bandmem[curr_band].ant_sw== 1)
        bandmem[curr_band].ant_sw = 2;
    else if (bandmem[curr_band].ant_sw == 2)
        bandmem[curr_band].ant_sw = 1;
    displayANT();
    //Serial.print("Set Ant Sw to ");
    //Serial.println(bandmem[curr_band].ant_sw);

#ifdef DIG_STEP_ATT  // for testing only
// FOR TEST of Attenuator settings
static int i=1;
i = bandmem[curr_band].attenuator_dB +1;
if (i> 31)
    i=1;
if (i< 1)
    i= 31;
setAtten_dB(i);
bandmem[curr_band].attenuator_dB = i;
//Serial.print("TEST: Manually Set Atten value to "); Serial.println(i);
#endif

}

// AF GAIN button activate control
COLD void setAFgain(int8_t toggle)
{
    if (toggle == 2)    // toggle if ordered, else just set to current state such as for startup.
    {
        if (user_settings[user_Profile].afGain_en)  // toggle the tracking state
            toggle = 0;
        else 
            toggle = 1;
    }

    if (toggle == 1)      // Set button to on to track as active 
    {
        if ( user_settings[user_Profile].afGain_en == ON && MeterInUse)  // if already on, assume this was called by a long press and we want to turn off the meter and but not the feature.
            clearMeter();
        else
        {
            user_settings[user_Profile].afGain_en = ON;  // set the af tracking state to ON
            setMeter(AFGAIN_BTN);
        }
    }
    
    if (toggle == 0 || toggle == -1)
    {
        user_settings[user_Profile].afGain_en = OFF;
        MeterInUse = false;
        if (toggle != -1)
            clearMeter();
    }

    //Serial.print(" AF Gain ON/OFF set to  "); 
    //Serial.println(user_settings[user_Profile].afGain_en);
    displayAFgain();
}

// AFGain Adjust
//  Input:   0 no change
//          -X Reduce by X%
//          +X Increase by X%
//
//   Request a new volume as a percentage up or down.
//   To jump to Full volume use 100;
//   To jump to Min volume use -100;
//   Normally just ask for +1 or -1
//       
COLD void AFgain(int8_t delta)
{
    float _afLevel;

    ///if (MF_client == AFGAIN_BTN)
    //    set_MF_Service(AFGAIN_BTN);  // reset encoder counter and set up for next read if any until another functionm takes ownership

    _afLevel = user_settings[user_Profile].afGain;   // Get last absolute volume setting as a value 0-100

//Serial.print(" TEST AF Level requested "); Serial.println(_afLevel);

    _afLevel += delta;      // convert percentage request to a single digit float

//Serial.print(" TEST AF Level absolute "); Serial.println(_afLevel);

    if (_afLevel > 100)         // Limit the value between 0.0 and 1.0 (100%)
        _afLevel = 100;
    if (_afLevel < 1)
        _afLevel = 1;    // do not use 0 to prevent divide/0 error

    user_settings[user_Profile].afGain = _afLevel;  // was 3 finger swipe down
    RampVolume(_afLevel/100, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    //Serial.print(" Volume set to  "); 
    //Serial.println(_afLevel);
    displayAFgain();
}

// RF GAIN button activate control
COLD void setRFgain(int8_t toggle)
{
    if (toggle == 2)    // toggle if ordered, else just set to current state such as for startup.
    {
        if (user_settings[user_Profile].rfGain_en)  // toggle the attenuator tracking state
            toggle = 0;
        else 
            toggle = 1;
    }
    
    if (toggle == 1)      // Set button to on to track as active 
    {
        user_settings[user_Profile].rfGain_en = ON;        
        setMeter(RFGAIN_BTN);
    }
    
    if (toggle == 0 || toggle == -1)
    {
        user_settings[user_Profile].rfGain_en = OFF;
        MeterInUse = false;
        if (toggle != -1)
            clearMeter();
    }

    //Serial.print(" RF Gain ON/OFF set to  "); 
    //Serial.println(user_settings[user_Profile].rfGain_en);
    displayRFgain();
}

// RF GAIN Adjust
//
//  Input:   0 no change
//          -X Reduce by X%
//          +X Increase by X%
//
COLD void RFgain(int8_t delta)
{
    float _rfLevel;

    _rfLevel = user_settings[user_Profile].rfGain;   // Get last absolute volume setting as a value 0-100

//Serial.print(" TEST RF Level "); Serial.println(_rfLevel);

    _rfLevel += delta;      // convert percentage request to a single digit float

//Serial.print(" TEST RF Level "); Serial.println(_rfLevel);

    if (_rfLevel > 100)         // Limit the value between 0.0 and 1.0 (100%)
        _rfLevel = 100;
    if (_rfLevel < 1)
        _rfLevel = 1;    // do not use 0 to prevent divide/0 error

    user_settings[user_Profile].rfGain = _rfLevel;  // 
    codec1.lineInLevel(user_settings[user_Profile].lineIn_level * user_settings[user_Profile].rfGain/100); 
    //Serial.print("CodecLine IN level set to "); 
    //Serial.println(user_settings[user_Profile].lineIn_level * user_settings[user_Profile].rfGain/100);
    //Serial.print("RF Gain level set to  "); 
    //Serial.println(_rfLevel);
    displayRFgain();
}

// XMIT button
COLD void Xmit()
{
    if (user_settings[user_Profile].xmit == ON)
    {
        user_settings[user_Profile].xmit = OFF;
        digitalWrite(PTT_OUT1, HIGH);
        TXAudio(0);  // enable line input to pass to headphone jack on audio card, set audio levels
    }
    else if (user_settings[user_Profile].xmit == OFF)
    {
        user_settings[user_Profile].xmit = ON;
        digitalWrite(PTT_OUT1, LOW);
        TXAudio(1);  // enable mic input to pass to line out on audio card, set audio levels
    }
    
    displayXMIT();
    displayFreq();
    //Serial.print("Set XMIT to ");
    //Serial.println(user_settings[user_Profile].xmit);
}

// NB button
COLD void NB(int8_t toggle)
{
    //char string[80];   // print format stuff
    if (toggle == 2)    // toggle if ordered, else just set to current state such as for startup.
    {
        if (user_settings[user_Profile].nb_en)  // toggle the attenuator tracking state
            toggle = 0;
        else 
            toggle = 1;
    }
    if (toggle == 1)
    {   
        if (user_settings[user_Profile].nb_en == ON && MeterInUse)  // if already on, assume this was called by a log press and we want to turn off the meter and but not the feature.
            clearMeter();
        else
        {
            user_settings[user_Profile].nb_en = ON;
            setMeter(NB_BTN);
        }
    }
    
    if (toggle == 0) 
    {
        user_settings[user_Profile].nb_en = OFF;
        if (toggle != -1)
            clearMeter();
        setNBLevel(0);     // update to current setting but setNBLevel will see it is turned off and bypass the NB component
    }
    //Serial.print("Set NB to ");
    //Serial.println(user_settings[user_Profile].nb_en);
    displayNB();
}

// Adjust the NB level. NB() turn on and off only calling this to initiialize the current level with delta = 0    
COLD void setNBLevel(int8_t delta)
{
    float _nbLevel;

    _nbLevel = user_settings[user_Profile].nb_level;   // Get last known value

//Serial.print(" TEST NB Level "); Serial.println(_nbLevel);

    _nbLevel += delta;      // increment up or down

//Serial.print(" TEST NB Level "); Serial.println(_nbLevel);

    if (_nbLevel >= NB_SET_NUM-1)         // Limit the value
        _nbLevel = NB_SET_NUM-1;
    if (_nbLevel <= 1)
        _nbLevel = 1;

    user_settings[user_Profile].nb_level = _nbLevel;  // We have our new table index value

    if (user_settings[user_Profile].nb_en > OFF)   // Adjust the value if on
    {
        NoiseBlanker.showError(1);
        NoiseBlanker.useTwoChannel(true);  // true enables a path through the "I"  or left side for I and Q
        NoiseBlanker.enable(true); // turn on NB for I
        // void setNoiseBlanker(float32_t _threshold, uint16_t _nAnticipation, uint16_t _nDecay)
        NoiseBlanker.setNoiseBlanker(nb[user_settings[user_Profile].nb_level].nb_threshold,
          nb[user_settings[user_Profile].nb_level].nb_nAnticipation,
          nb[user_settings[user_Profile].nb_level].nb_decay);   // for I
        // threshold recommended to be between 1.5 and 20, closer to 3 maybe best.
        // nAnticipation is 1 to 125
        // Decay is 1 to 10.
    }
    else  // NB is disabled so bypass 
    {
        NoiseBlanker.enable(false);  //  NB block is bypassed
    }
    
    //Serial.print("NB level set to  "); 
    //Serial.println(_nbLevel);
    displayNB();
}

// NR button
COLD void NR()
{
    if (user_settings[user_Profile].nr_en > NROFF)
        user_settings[user_Profile].nr_en = NROFF;
    else if (user_settings[user_Profile].nr_en == NROFF)
        user_settings[user_Profile].nr_en = NR1;
    displayNR();
    //Serial.print("Set NR to ");
    //Serial.println(user_settings[user_Profile].nr_en);
}

// Enet button
COLD void Enet()
{
    if (user_settings[user_Profile].enet_output == ON)
        user_settings[user_Profile].enet_output = OFF;
    else if (user_settings[user_Profile].enet_output == OFF)
        user_settings[user_Profile].enet_output = ON;
    displayEnet();
    //Serial.print("Set Ethernet to ");
    //Serial.println(user_settings[user_Profile].enet_output);
}

// Spot button
COLD void Spot()
{
    if (user_settings[user_Profile].spot == OFF)
        user_settings[user_Profile].spot = ON;
    else
        user_settings[user_Profile].spot = OFF;
    displaySpot();
    //Serial.print("Set Spot to ");
    //Serial.println(user_settings[user_Profile].spot);
}

// REF LEVEL button activate control
COLD void setRefLevel(int8_t toggle)
{
    if (toggle == 2)    // toggle if ordered, else just set to current state such as for startup.
    {
        if (std_btn[REFLVL_BTN].enabled)  // toggle the attenuator tracking state
            toggle = 0;
        else 
            toggle = 1;
    }
    
    if (toggle == 1)      // Set button to on to track as active 
    {
        std_btn[REFLVL_BTN].enabled = ON;
        if (MF_client != REFLVL_BTN) 
        {
            set_MF_Service(REFLVL_BTN);  // reset encoder counter and set up for next read if any until another functionm takes ownership
            MF_default_is_active = false;
        }
        //Serial.print("Set REFLVL to ON ");
        //Serial.print(std_btn[REFLVL_BTN].enabled);
    }
    
    if (toggle == 0 || toggle == -1)
    {
        std_btn[REFLVL_BTN].enabled = OFF;
        MeterInUse = false; 
        if (toggle != -1)
        {       
            set_MF_Service(user_settings[user_Profile].default_MF_client);  // will turn off the button, if any, and set the default as new owner.
            MF_default_is_active = true;
        }
        //Serial.print("Set REFLVL to OFF ");
        //Serial.print(std_btn[REFLVL_BTN].enabled);
    }

    displayRefLevel();
    //Serial.print(" and Ref Level is ");
    //Serial.println(Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor);
}

// Ref Level adjust
// Pass the desired new absolute value.  It will be limited to allowable range of -110 to -220
COLD void RefLevel(int8_t newval)
{
    bandmem[curr_band].sp_ref_lvl += newval;
    if (bandmem[curr_band].sp_ref_lvl > 50)
        bandmem[curr_band].sp_ref_lvl = 50; 
    if (bandmem[curr_band].sp_ref_lvl < -50)
        bandmem[curr_band].sp_ref_lvl = -50; 
#ifndef BYPASS_SPECTRUM_MODULE        
    Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor = bandmem[curr_band].sp_ref_lvl;
#endif
    displayRefLevel();
    //Serial.print("Set Reference Level to ");
    //Serial.println(Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor);
}

// Notch button
COLD void Notch()
{
    if (user_settings[user_Profile].notch== ON)
        user_settings[user_Profile].notch = OFF;
    else if (user_settings[user_Profile].notch == OFF)
        user_settings[user_Profile].notch = ON;
    displayNotch();
    //Serial.print("Set Notch to ");
    //Serial.println(user_settings[user_Profile].notch);
}

// BAND UP button
COLD void BandUp()
{
    changeBands(1);
    displayBandUp();
    //Serial.print("Set Band UP to ");
    //Serial.println(bandmem[curr_band].band_num,DEC);
}

// BAND DOWN button
COLD void BandDn()
{
    //Serial.println("BAND DN");
    changeBands(-1);
    displayBandDn();
    //Serial.print("Set Band DN to ");
    //Serial.println(bandmem[curr_band].band_num,DEC);
}

// BAND button
COLD void Band()
{
    //popup = 1;
    //pop_win(1);
    changeBands(1);  // increment up 1 band for now until the pop up windows buttons and/or MF are working
    displayBand();
    //Serial.print("Set Band to ");
    //Serial.println(bandmem[curr_band].band_num,DEC);
}

// DISPLAY button
COLD void Display()
{   
#ifndef BYPASS_SPECTRUM_MODULE
    if (Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_dot_bar_mode)
    {
        display_state = 0;
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_dot_bar_mode = 0;
    }
    else 
    {
        display_state = 1;
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_dot_bar_mode = 1;
    }
    spectrum_RA887x.drawSpectrumFrame(user_settings[user_Profile].sp_preset);
#endif
    //popup = 1;
    //pop_win(1);
    displayDisplay();
    //Serial.print("Set Display Button to ");
    //Serial.println(display_state);
}

COLD void TouchTune(int16_t touch_Freq)
{
#ifndef BYPASS_SPECTRUM_MODULE
    touch_Freq -= Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_width/2;     // adjust coordinate relative to center 
    int32_t _newfreq = touch_Freq * fft_bin_size*2;  // convert touch X coordinate to a frequency and jump to it.    
    
    //Serial.print("TouchTune(r) frequency is ");
    
    if (bandmem[curr_band].VFO_AB_Active == VFO_A)
    {
        VFOA += _newfreq;
        if (abs((int32_t) VFOA - (int32_t) Freq_Peak) < 800)
        {
            if (bandmem[curr_band].mode_A == CW || bandmem[curr_band].mode_A == CW_REV)
                VFOA = Freq_Peak - ModeOffset;  //user_settings[user_Profile].pitch;
            else
                VFOA = Freq_Peak;
        }
        //Serial.println(formatVFO(VFOA));
    }
    else
    {
        VFOB += _newfreq; 
        if (abs((int32_t) VFOB - (int32_t) Freq_Peak) < 800)
        {
            if (bandmem[curr_band].mode_B == CW || bandmem[curr_band].mode_B == CW_REV)
                VFOB = Freq_Peak - ModeOffset; //user_settings[user_Profile].pitch;
            else
                VFOB = Freq_Peak;
        }
        //Serial.println(formatVFO(VFOB));
    }
#endif    
    selectFrequency(0);
    displayFreq();
}

/*******************************************************************************
* Function Name: setAtten_dB()
********************************************************************************
*
* Summary:
* Main function performs following functions:
* 1: Configures the solid state attenuator by shifting 16 bits of address and
*    atten level in LSB first.
* 
* Parameters:
*  atten = attenuation level to set in range of 0 to 31 (in dB)
*
* Return:
*  None.
*
*******************************************************************************/
COLD void setAtten_dB(int8_t atten)
{
    //Serial.print("Requested new attenuator value is "); Serial.println(atten);
    if(atten > 31) 
        atten = 31;
    if(atten <=0 )
        atten = 0;
    bandmem[curr_band].attenuator_dB = atten;
    
    //Serial.print("Setting attenuator value to "); Serial.println(bandmem[curr_band].attenuator_dB);
    displayAttn();  // update the button value
    
    #ifdef DIG_STEP_ATT
    char    atten_str[8] = {'\0'};
    char    atten_data[8] = {'\0'};
    uint8_t   i;

    atten *= 2; //shift the value x2 so the LSB controls the 0.5 step.  We are not using the 0.5 today.
    /* Convert to 8 bits of  0 and 1 format */
    itoa(atten, atten_str, 2);
    
    // pad with leading 0s as needed.  6 bits for the PE4302
    for(i=0;(i<6-strlen(atten_str));i++)
    {
        atten_data[i]='0';
    }
    strncat(atten_data, atten_str, strlen(atten_str));

    //  LE = 0 to allow writing data into shift register
    digitalWrite(Atten_LE,   (uint8_t) OFF);
    digitalWrite(Atten_DATA, (uint8_t) OFF);
    digitalWrite(Atten_CLK,  (uint8_t) OFF);
    delayMicroseconds(10);
    //  Now loop for 6 bits, set data on Data pin and toggle Clock pin.  
    //    Start with the MSB first so start at the left end of the string   
    for(i=0;i<6;i++)
    {
        // convert ascii 0 or 1 to a decimal 0 or 1 
        digitalWrite(Atten_DATA, (uint8_t) atten_data[i]-48);
        delayMicroseconds(10);
        digitalWrite(Atten_CLK,  (uint8_t) ON);
        delayMicroseconds(10);
        digitalWrite(Atten_CLK,  (uint8_t) OFF); 
        delayMicroseconds(10);
    }
    //  Toggle LE pin to latch the data and set the new attenuation value in the hardware
    digitalWrite(Atten_LE, (uint8_t) ON);
    delayMicroseconds(10);
    digitalWrite(Atten_LE, (uint8_t) OFF);

    return;    
#endif  // DIG_STEP_ATT
}

COLD void selectStep(uint8_t fndx)
{ 
		if (fndx <= 1)
		{
			fndx = 0;
		}

		if (fndx >= TS_STEPS-1)
		{
			fndx = TS_STEPS-1;
		}
	//  remove the int fndx arg if using a global fndx
	bandmem[curr_band].tune_step = fndx;
	displayRate();
}

COLD void selectAgc(uint8_t andx)
{

	// TODO   Put in some code for custom AGC. Use the AGC table of settings.
    if (andx >= AGC_SET_NUM)
      	andx = AGC_OFF; 		// Cycle around

	if (andx <  AGC_OFF)
    	andx = AGC_SET_NUM - 1;		// Cycle around
		
  	bandmem[curr_band].agc_mode = andx;

    if (andx == AGC_OFF)
        codec1.autoVolumeDisable();
    else
    {
        struct AGC *pAGC = &agc_set[andx];
        codec1.autoVolumeControl(
            pAGC->agc_maxGain,
            pAGC->agc_response,
            pAGC->agc_hardlimit,
            pAGC->agc_threshold,
            pAGC->agc_attack,
            pAGC->agc_decay);
        codec1.autoVolumeEnable();
    }
 	//displayAgc();
}

// Turns meter off
void clearMeter(void)
{
    Serial.println("Turn OFF meter");
    MeterInUse = false;
    set_MF_Service(user_settings[user_Profile].default_MF_client);  // will turn off the button, if any, and set the default as new owner.
    MF_default_is_active = true;
}

// Turns meter on and assigns function to the new MF focus
void setMeter(uint8_t id)
{
    if (MF_client != id) 
    {
        Serial.println("Turn ON meter");
        set_MF_Service(id);  // reset encoder counter and set up for next read if any until another functionm takes ownership
        MF_default_is_active = false;  
    }
}
