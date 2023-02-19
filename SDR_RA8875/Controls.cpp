//
//      Controls.cpp
//

#include "RadioConfig.h"
#include "SDR_RA8875.h"

#ifdef USE_RS_HFIQ
    // init the RS-HFIQ library
    extern SDR_RS_HFIQ RS_HFIQ;
    extern void send_fixed_cmd_to_RSHFIQ(const char* str);
#endif

// Using the SV1AFN Band Pass Filter board with modified I2C library for Premp, Attenuator, and for 10 HF bands of BPFs
// #include "SVN1AFN_BandpassFilters.h>""
#ifdef SV1AFN_BPF
    #include "SVN1AFN_BandpassFilters.h"
    extern SVN1AFN_BandpassFilters bpf;
#endif

#ifdef USE_RA8875
    extern RA8875 tft;
#else
    extern RA8876_t3 tft;
#endif

extern uint8_t display_state; // something to hold the button state for the display pop-up window later.
extern uint8_t curr_band;     // global tracks our current band setting.
extern uint64_t VFOA;         // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint64_t VFOB;
extern struct Modes_List modeList[];
extern struct Band_Memory bandmem[];
extern struct User_Settings user_settings[];
extern struct Standard_Button std_btn[];
extern struct Label labels[];
extern struct Filter_Settings filter[];
extern struct AGC agc_set[];
extern struct NB nb[];
extern struct Spectrum_Parms Sp_Parms_Def[];
extern struct Zoom_Lvl zoom[];
extern struct EncoderList encoder_list[];
extern struct TuneSteps tstep[];
extern struct Transverter xvtr[];
extern uint8_t user_Profile;
extern Metro popup_timer; // used to check for popup screen request
extern Metro TX_Timeout;  // RunawayTX timeout
extern AudioControlSGTL5000 codec1;
extern radioNoiseBlanker_F32 NoiseBlanker;
extern AudioEffectCompressor2_F32 compressor1; // Audio Compressor
extern AudioEffectCompressor2_F32 compressor2; // Audio Compressor
extern uint8_t popup;
extern void RampVolume(float vol, int16_t rampType);
extern volatile uint64_t Freq_Peak;
extern void set_MF_Service(uint8_t client_name);
extern void unset_MF_Service(uint8_t client_name);
extern uint8_t MF_client;  // Flag for current owner of MF knob services
extern float fft_bin_size; // = sample_rate_Hz/(FFT_SIZE*2) -  Size of FFT bin in Hz
extern void touchBeep(bool enable);
extern bool MeterInUse; // S-meter flag to block updates while the MF knob has control
extern Metro MF_Timeout;
extern bool MF_default_is_active;
extern uint8_t default_MF_slot;
extern void TX_RX_Switch(bool TX, uint8_t mode_sel, bool b_Mic_On, bool b_USBIn_On, bool b_ToneA, bool b_ToneB, float TestTone_Vol);
extern int32_t ModeOffset;
extern AudioMixer4_F32 I_Switch;
extern AudioMixer4_F32 Q_Switch;
extern AudioLMSDenoiseNotch_F32 LMS_Notch;
extern bool TwoToneTest;
extern uint16_t fft_size;
extern int16_t fft_bins;
extern void Change_FFT_Size(uint16_t new_size, float new_sample_rate_Hz);
extern float zoom_in_sample_rate_Hz;
extern float sample_rate_Hz;
#ifdef USE_FREQ_SHIFTER
    extern AudioEffectFreqShiftFD_OA_F32 FFT_SHIFT_I;
    extern AudioEffectFreqShiftFD_OA_F32 FFT_SHIFT_Q;
#endif
#ifdef USE_FFT_LO_MIXER
    extern RadioIQMixer_F32 FFT_LO_Mixer_I;
    extern RadioIQMixer_F32 FFT_LO_Mixer_Q;
#endif
extern float pan;
extern uint64_t xvtr_offset; // LO to apply to actual PLL freq
extern int16_t rit_offset;        // global RIT offset value in Hz.
extern int16_t xit_offset;        // global XIT offset value in Hz.
extern int16_t rit_offset_last;   // global RIT offset value in Hz.
extern int16_t xit_offset_last;   // global RIT offset value in Hz.
extern void update_icon_outline(void);
extern void ringMeter(int val, int minV, int maxV, int16_t x, int16_t y, uint16_t r, const char* units, uint16_t colorScheme, uint16_t backSegColor, int16_t angle, uint8_t inc);

void Set_Spectrum_Scale(int8_t zoom_dir);
void Set_Spectrum_RefLvl(int8_t zoom_dir);
void changeBands(int8_t direction);
void pop_win_up(uint8_t win_num);
void pop_win_down(uint8_t win_num);
void Mute();
void Menu();
void Display();
void Band(uint8_t new_band);
void BandDn();
void BandUp();
void Notch();
void Spot();
void Enet();
void setNR();
void setNB(int8_t toggle);
void Xmit(uint8_t state);
void Ant();
void Fine();
void Rate(int8_t dir);
void setMode(int8_t dir);
void AGC(int8_t dir);
void Filter(int8_t dir);
void Variable_Filter(int8_t dir);
void ATU();
void Split();
void setXIT(int8_t toggle);
void XIT(int8_t delta);
void setRIT(int8_t toggle);
void RIT(int8_t delta);
void Preamp(int8_t toggle);
void setAtten(int8_t toggle);
void VFO_AB();
void Atten(int8_t delta);
void setAFgain(int8_t toggle);
void AFgain(int8_t delta);
void setRFgain(int8_t toggle);
void RFgain(int8_t delta);
void setRefLevel(int8_t toggle);
void NBLevel(int8_t delta);
void RefLevel(int8_t newval);
void TouchTune(int16_t touch_Freq);
void selectStep(uint8_t fndx);
void selectAgc(uint8_t andx);
void clearMeter(void);
void setMeter(uint8_t id);
void Zoom(int8_t dir);
void setZoom(int8_t toggle);
void PAN(int8_t delta);
void setPAN(int8_t toggle);
void digital_step_attenuator_PE4302(int16_t _atten); // Takes a 0 to 100 input, converts to the appropriate hardware steps such as 0-31dB in 1 dB steps
void setEncoderMode(uint8_t role);

#ifndef BYPASS_SPECTRUM_MODULE
// Use gestures (pinch) to adjust the the vertical scaling.  This affects both watefall and spectrum.  YMMV :-)
COLD void Set_Spectrum_Scale(int8_t zoom_dir)
{
    // DPRINTLN(zoom_dir);
    // extern struct Spectrum_Parms Sp_Parms_Def[];
    // extern Spectrum_RA887x spectrum_RA887x;    // Spectrum Display Libary
    if (Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale > 2.0)
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale = 0.5;
    if (Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale < 0.5)
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale = 2.0;
    if (zoom_dir == 1)
    {
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale += 0.1;
        // DPRINTLN("ZOOM IN");
    }
    else
    {
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale -= 0.1;
        // DPRINTLN("ZOOM OUT");
    }
    // DPRINTLN(Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_scale);
}

// Use gestures to raise and lower the spectrum reference level relative to the bottom of the window (noise floor)
COLD void Set_Spectrum_RefLvl(int8_t zoom_dir)
{
    // DPRINTLN(zoom_dir);

    if (zoom_dir == 1)
    {
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor -= 1;
        // DPRINTLNF("RefLvl=UP");
    }
    else
    {
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor += 1;
        // DPRINTLNF("RefLvl=DOWN");
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

COLD void changeBands(int8_t direction) // neg value is down.  Can jump multiple bandswith value > 1.
{
    int8_t target_band;
    // TODO search bands column for match to account for mapping that does not start with 0 and bands could be in odd order and disabled.

    DPRINTF("\nChangeBand: Incoming Band is "); DPRINTLN(bandmem[curr_band].band_name); DPRINTF("  Current Freq is "); DPRINT(VFOA); DPRINTF("  Current Last_VFOA is "); DPRINTLN(bandmem[curr_band].vfo_A_last);

    Split(0);

    target_band = bandmem[curr_band].band_num + direction;
    // target_band = curr_band + direction;
    DPRINTF("ChangeBand: Proposed Target Band is "); DPRINTLN(bandmem[target_band].band_name);

    uint16_t top_band    = BANDS-1;
    uint16_t bottom_band = 0;

    // See if the proposed new band is enabled in the bandmap or not.  If not skip to the next one and check again until true.
    while (1)  // lop around up or down until we find a valid band to switch to.
    {
        if (target_band > top_band)
            target_band = bottom_band;

        if (target_band < bottom_band) // bottom
            target_band = top_band;

        if (bandmem[target_band].bandmap_en)
        {
            DPRINTF("ChangeBand: Target band index "); DPRINT(target_band); DPRINTF(" is in the Bandmap. Target band is "); DPRINTLN(bandmem[target_band].band_name);
            break;
        }
        else
        {
            DPRINTF("ChangeBand: Target band "); DPRINT(bandmem[target_band].band_name); DPRINTLNF(" NOT in Bandmap. Trying next Band.");
            target_band += direction; // maintain change direction up or down.
            if (direction == 0) 
                target_band +=1;  // force a search of current is invalid, possible if VFO set externally to disabled band
        }
    }

    curr_band = target_band; // We have a good band so can new band

    if (direction != 0)
        VFOA = bandmem[curr_band].vfo_A_last; // up the last used frequencies
    else
        if (!find_new_band(VFOA, curr_band))  // returns 0 when out of band
            VFOA = bandmem[curr_band].vfo_A_last;   // keep last valid frequency

    DPRINTF("ChangeBand: New Band is "); DPRINT(bandmem[curr_band].band_name); DPRINTF("  New VFOA is "); DPRINTLN(VFOA);
    
    // Calculate frequency difference between the designated xvtr IF band's lower edge and the current VFO band's lower edge (the LO frequency).
    if (bandmem[curr_band].xvtr_IF)
        xvtr_offset = bandmem[curr_band].edge_lower - bandmem[bandmem[curr_band].xvtr_IF].edge_lower; // if band is 144 then PLL will be set to VFOA-xvtr_offset
    else
        xvtr_offset = 0;
    DPRINTF("ChangeBand: xvtr_offset is "); DPRINTLN(xvtr_offset);

    DPRINTLNF("ChangeBand: Set RIT if On");
    if (bandmem[curr_band].RIT_en)
        setRIT(1); // turn on if it was on before.   Also calls selectFrequency(0);
    else
        setRIT(0); // turn off if it was off before on this new band

    // Example how and when to output new Band Decoer pattern on output IO pins. Pins assignments TBD.
    // Set the new band decoder output pattern for this band change
// uint16_t BandDecodePatternByte = bandmem[curr_band].bandDecode;

    codec1.muteHeadphone(); // remove audio thumps during hardware transients

    DPRINTLNF("ChangeBand: Set other per band settings");
    // Split(0);
    setAtten(-1); // -1 sets to database state. 2 is toggle state. 0 and 1 are Off and On.  Operate relays if any.
    selectBandwidth(bandmem[curr_band].filter);
    // dB level is set elsewhere and uses value in the dB in this function.
    Preamp(-1); // -1 sets to database state. 2 is toggle state. 0 and 1 are Off and On.  Operate relays if any.
    // selectMode(0);
    DPRINTLNF("ChangeBand: Set Mode");
    setMode(2);  // 0 is set value in database for both VFOs
    RefLevel(0); // 0 just updates things to be current value
    RFgain(0);
    AFgain(0);
    NBLevel(0); // 0 just updates things to be current value
    ATU(-1);    // -1 sets to database state. 2 is toggle state. 0 and 1 are Off and On.

    #ifndef BYPASS_SPECTRUM_MODULE
        drawSpectrumFrame(user_settings[user_Profile].sp_preset);
    #endif
    // Rate(0); Not needed
    // Ant() when there is hardware to setup in the future
    // ATU() when there is hardware to setup in the future
    //
    //    insert any future features, software or hardware, that need to be altered
    //
    selectAgc(bandmem[curr_band].agc_mode);
    displayRefresh();
    codec1.unmuteHeadphone(); // reduce audio thump from hardware transitions
    DPRINTLNF("ChangeBand: Complete\n");
}

//
//  -----------------------   Button Functions --------------------------------------------
//   Called by Touch, Encoder, or Switch events
//

// ---------------------------setMode() ---------------------------
//   Input: 0 = step to next based on last direction (starts upwards).  Ramps up then down then up.
//          1 = step up 1 mode
//         -1 = step down 1 mode
//          2 = use last mode
//
COLD void setMode(int8_t dir)
{
    static int8_t direction = 1;
    int8_t _mndx;

    _mndx = (int8_t)bandmem[curr_band].mode_A; // get current mode

    if (dir != 2)
    {
        // 1. Limit to allowed step range
        // 2. Cycle up and at top, cycle back down, do nto roll over.
        if (_mndx <= 0)
        {
            _mndx     = 0; // mode = 0 then change to -1 will add direction == +1 to get 0 later.
            direction = 1; // cycle upwards
        }

        if (_mndx >= MODES_NUM - 1)
        {
            _mndx     = MODES_NUM - 1;
            direction = -1; // go downward
        }

        if (dir == 0)
            _mndx += direction; // Index our step up or down, if dir == 0 then no change to current value
        else
            _mndx += dir; // forces a step higher or lower then current

        if (_mndx > MODES_NUM - 1) // limit in case of encoder counts
            _mndx = MODES_NUM - 1;

        if (_mndx < 0) // limit in case of encoder counts
            _mndx = 0;

        bandmem[curr_band].mode_A = (uint8_t)_mndx; // store it
    }

    selectMode((uint8_t)_mndx); // Select the mode for the Active VFO

    // Update the filter setting per mode
    Filter(2);
    // DPRINTF("Set Mode: ");  DPRINTLN(bandmem[curr_band].mode_A);
    displayMode();
    selectFrequency(0); // Call in case a mode change requires a frequency offset
}

// ---------------------------Filter() ---------------------------
//   Input: 0 = step to next based on last direction (starts upwards).  Ramps up then down then up.
//          1 = step up 1 filter (wider)
//         -1 = step down 1 filter (narrower)
//          2 = use last filter width used in this mode (from modeList table)
//      This is mode-aware. In non-CW modes we will only cycle through SSB width filters as set in the filter tables

// FILTER button
COLD void Filter(int8_t dir)
{
    static int8_t direction = 1;
    int16_t _bw             = bandmem[curr_band].filter; // Change Bandwidth  - cycle down then back to the top

    uint8_t _mode;

    // 1. Limit to allowed step range
    // 2. Cycle up and at top, cycle back down, do nto roll over.
    if (_bw <= 0)
    {
        _bw       = -1;
        direction = 1; // cycle upwards
    }

    if (_bw >= FILTER - 1)
    {
        _bw       = FILTER - 1;
        direction = -1;
    }

    _mode = bandmem[curr_band].mode_A;

    if (_mode == FM)
    {
        _bw = modeList[FM].Width;
    }
    else if (_mode == CW || _mode == CW_REV) // CW modes
    {
        if (_bw > FILTER - 1) // go to bottom band
        {
            _bw       = FILTER - 1;
            direction = -1; // cycle downwards
        }
        if (_bw <= BW0_25)
        {
            _bw       = BW0_25 + 1;
            direction = 1; // cycle upwards
        }
    }
    else // Non-CW modes
    {
        if (_bw > FILTER - 1) // go to bottom band
        {
            _bw       = FILTER - 1;
            direction = -1; // cycle downwards
        }
        if (_bw <= BW1_8)
        {
            _bw       = BW1_8 + 1;
            direction = 1; // cycle upwards
        }
    }

    if (_mode != FM)
    {
        if (dir == 0)
            _bw += direction; // Index our step up or down
        else
            _bw += dir; // forces a step higher or lower then current

        if (_bw > FILTER - 1) // limit in case of encoder counts
            _bw = FILTER - 1;

        if (_bw < 0) // limit in case of encoder counts
            _bw = 0;

        // DPRINT("_bw=");DPRINT(_bw);
    }

    if (dir == 2) // Use last filter width used in this mode, ignore the rest (hopefully it is valid)
    {
        if (modeList[_mode].Width < FILTER && modeList[_mode].Width >= 0)
            _bw = (int16_t)modeList[_mode].Width;
    }
    else
        modeList[_mode].Width = (uint16_t)_bw; // if filter changed without a mode change, store it in last use per mode table.

    bandmem[curr_band].var_filter = filter[modeList[_mode].Width].Width; // update the variable filter setting with new bvalue from button
    selectBandwidth((uint8_t)_bw);
    // DPRINTF("Set Filter to "); DPRINTLN(bandmem[curr_band].filter);
    displayFilter();
}

// ---------------------------Variable_Filter() ---------------------------
//          positive counts = Make filter wider   - step rate 100Hz
//          negative counts = Make filter narrower
//          2 = use last filter width used in this mode (from modeList table)
//      This is mode-aware. In non-CW modes we will only cycle through SSB width filters as set in the filter tables
//
//  FILTER button
COLD void Variable_Filter(int8_t dir)
{
    extern int16_t filterCenter;
    extern int16_t filterBandwidth;
    extern void SetFilter(void);
    int16_t var_bw            = bandmem[curr_band].var_filter;
    uint16_t var_filt_max     = 6000; // FM is NA (fixed value)
    uint16_t var_filt_min_CW  = 50;
    uint16_t var_filt_min_SSB = 150;
    extern AudioEffectGain_F32 Amp1_L; // Some well placed gain stages
    extern AudioEffectGain_F32 Amp1_R; // Some well placed gain stages

    var_bw = bandmem[curr_band].var_filter; // get current filter width value

    if (var_bw < 1000)        // 100Hz steps above 1KHz, 50Hz below
        var_bw += (dir * 50); // 50Hz per step
    else if (var_bw < 3000)
        var_bw += (dir * 100); // 100Hz per step
    else
        var_bw += (dir * 200); // 200Hz per step up to max

    if (var_bw >= var_filt_max)
        var_bw = var_filt_max;

    if (bandmem[curr_band].mode_A == FM)
    {
        var_bw = modeList[FM].Width;
    }
    else if (bandmem[curr_band].mode_A <= CW_REV)
    {
        if (var_bw <= var_filt_min_CW)
            var_bw = var_filt_min_CW;

        filterCenter                  = user_settings[user_Profile].pitch; // Use pitch since this is a CW filter
        filterBandwidth               = var_bw;
        bandmem[curr_band].var_filter = (uint16_t)var_bw; // Store our new filter width

        float CW_boost = 0.0f;
        if (var_bw < 51)
            CW_boost = 18.0f;
        else if (var_bw < 101)
            CW_boost = 16.0f;
        else if (var_bw < 251)
            CW_boost = 14.0f;
        else if (var_bw < 501)
            CW_boost = 12.0f;
        else if (var_bw < 701)
            CW_boost = 9.0f;
        else if (var_bw < 1001)
            CW_boost = 6.0f;
        else
            CW_boost = 0.0f;

        AudioNoInterrupts();
        Amp1_L.setGain_dB(AUDIOBOOST + CW_boost); // Adjustable fixed output boost in dB.
        Amp1_R.setGain_dB(AUDIOBOOST + CW_boost);
        AudioInterrupts();

        // DPRINTF("Set CW Variable Filter = "); DPRINTLN(var_bw);
    }
    else
    {
        if (var_bw <= var_filt_min_SSB)
            var_bw = var_filt_min_SSB;
        ;

        filterCenter                  = (var_bw / 2) + var_filt_min_SSB;
        filterBandwidth               = var_bw;
        bandmem[curr_band].var_filter = (uint16_t)var_bw; // Store our new filter width

        AudioNoInterrupts();
        Amp1_L.setGain_dB(AUDIOBOOST); // Adjustable fixed output boost in dB.
        Amp1_R.setGain_dB(AUDIOBOOST);
        AudioInterrupts();

        // DPRINTF("Set Variable Filter to "); DPRINTLN(bandmem[curr_band].var_filter);
    }
    // ToDo: Se tthe butomn filter index to closest value of the curent variable fil;ter to permit mixed operation of encoder and button

    SetFilter();
    displayVarFilter();
}

// ---------------------------Rate() ---------------------------
//   Input: 0 = step to next based on last direction (starts upwards).  Ramps up then down then up.
//          1 = step up 1 tune rate
//         -1 = step down 1 tune step rate
//      If FINE is OFF, we will not use 1Hz. If FINE = ON we only use 1 and 10Hz steps.

// RATE button
COLD void Rate(int8_t dir)
{
    static int direction = 1;
    int _tstep           = bandmem[curr_band].tune_step;

    if (user_settings[user_Profile].fine == 0)
    {
        // 1. Limit to allowed step range
        // 2. Cycle up and at top, cycle back down, do nto roll over.
        if (_tstep <= 1)
        {
            _tstep    = 1;
            direction = 1; // cycle upwards
        }

        if (_tstep >= 3) // TS_STEPS-1)
        {
            _tstep    = 3; // TS_STEPS-1;
            direction = -1;
        }

        if (dir == 0)
            _tstep += direction; // Index our step up or down
        else
            _tstep += dir; // forces a step higher or lower then current

        if (_tstep >= 3) // TS_STEPS-1)   // ensure we are still in range
            _tstep = 3;  // TS_STEPS - 1;  // just in case it over ranges, bad stuff happens when it does
        if (_tstep < 1)
            _tstep = 1; // just in case it over ranges, bad stuff happens when it does
    }

    if (user_settings[user_Profile].fine && dir == -1) // 1 Hz steps
        bandmem[curr_band].tune_step = 0;              // set to 1 hz steps
    else if (user_settings[user_Profile].fine && dir == 1)
        bandmem[curr_band].tune_step = 1; // normally swiped is +1 or -1
    else if (user_settings[user_Profile].fine && dir == 0)
    {
        if (_tstep > 0)
            _tstep = 0;
        else
            _tstep = 1;
        bandmem[curr_band].tune_step = _tstep;
    }
    else
        bandmem[curr_band].tune_step = _tstep; // Fine tunig mode is off, allow all steps 10hz and higher

    // DPRINT("Set Rate to ");
    // DPRINTLN(tstep[bandmem[curr_band].tune_step].ts_name);
    displayRate();
}

// AGC button
// 2 = cycle around
// -1, 0, +1  cycle up or down or stay the same
COLD void AGC(int8_t dir)
{
    int8_t a_md = bandmem[curr_band].agc_mode;
    a_md += dir;
    if (a_md < 0) a_md = 0;
    if (a_md > AGC_SET_NUM - 1) a_md = AGC_SET_NUM - 1;

    if (dir == 2)
        selectAgc(bandmem[curr_band].agc_mode + 1);
    else
    {
        bandmem[curr_band].agc_mode = (uint8_t)a_md;
        selectAgc(bandmem[curr_band].agc_mode);
    }
    // DPRINTF("Set AGC to "); DPRINTLN(bandmem[curr_band].agc_mode);
    sprintf(std_btn[AGC_BTN].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
    sprintf(labels[AGC_LBL].label, "%s", agc_set[bandmem[curr_band].agc_mode].agc_name);
    displayAgc();
}

// MUTE button
COLD void Mute()
{
    // float _afLevel = (float) user_settings[user_Profile].afGain/100;

    if (user_settings[user_Profile].spkr_en)
    {
        if (!user_settings[user_Profile].mute)
        {
            RampVolume(0.0f, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
            user_settings[user_Profile].mute = ON;
        }
        else
        { // codec1.muteHeadphone();
            // RampVolume(_afLevel, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
            user_settings[user_Profile].mute = OFF;
            AFgain(0);
        }
        displayMute();
    }
}

// MENU
COLD void Menu()
{
    pop_win_up(SPECTUNE_BTN);
#ifndef BYPASS_SPECTRUM_MODULE
    Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_colortemp += 10;
    if (Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_colortemp > 10000)
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_colortemp = 1;
#endif
    // tft.fillRect(t_ptr->bx, t_ptr->by, t_ptr->bw, t_ptr->bh, RA8875_BLACK);
    tft.setFont(Arial_24);
    tft.setTextColor(BLUE);
    tft.setCursor(CENTER, CENTER, true);
    tft.print(F("this is a future keyboard"));
    delay(1000);
    // DPRINT("spectrum_wf_colortemp = ");
    // DPRINTLN(Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_wf_colortemp);
    pop_win_down(SPECTUNE_BTN); // remove window, restore old screen info, clear popup flag and timer
    displayMenu();
    DPRINTLN("Menu Pressed");
}

// VFO A/B - swap VFO A and B values.
COLD void VFO_AB(void)
{
    // feedback beep
    touchBeep(true); // a timer will shut it off.

    // collect some settings in prep for swapping
    uint64_t old_VFOA  = VFOA;
    uint8_t old_A_mode = bandmem[curr_band].mode_A;
    // uint8_t  old_A_band = curr_band;
    uint64_t old_VFOB  = user_settings[user_Profile].sub_VFO;
    uint8_t old_B_mode = user_settings[user_Profile].sub_VFO_mode;

#ifdef USE_RS_HFIQXXX
    // Compute the band index for the new target band an ensure it is in limits
    // if (RS_HFIQ.find_new_band(old_VFOB, &curr_band))  // return the updated band index for the new freq
#else
    if (find_new_band(old_VFOB, curr_band))
#endif
    {
        // all good, now start swapping
        // DPRINT("\nStart Swapping -  VFO A: ");
        // DPRINT(old_VFOB);
        // DPRINT("  VFO A Mode: ");
        // DPRINT(old_B_mode);
        // DPRINT(" VFO A band: ");
        // DPRINTLN(curr_band);
        VFOA = bandmem[curr_band].vfo_A_last = old_VFOB; // Update VFOA to new freq, then update the band index to match
        bandmem[curr_band].mode_A            = old_B_mode;
        selectMode(old_B_mode); // copy to VFOA mode and apply

        // DPRINT("Stash sub_VFO values - VFO B: ");
        // DPRINT(old_VFOA);
        // DPRINT("  VFO B Mode: ");
        // DPRINT(old_A_mode);
        // DPRINT(" VFO B band: ");
        // DPRINTLN(old_A_band);
        VFOB = user_settings[user_Profile].sub_VFO = old_VFOA;   // save A into the database
        user_settings[user_Profile].sub_VFO_mode   = old_A_mode; // Udpate VFOB
    }
    selectFrequency(0);
    changeBands(0);
    displayVFO_AB();
    displayMode();
    DPRINT("Set VFO_A to ");
    DPRINTLN(VFOA);
    DPRINT("Set VFO_B to ");
    DPRINTLN(VFOB);
}

//  setAtten 
//   -1 sets attenuator state to current database value. Used for startup or changing bands.
//    0 sets attenuator state off
//    1 sets attenuator state on
//    2 toggles attenuator state
//    3 sets meter active and allow adjustment via MF knob
COLD void setAtten(int8_t toggle)
{
    if (bandmem[curr_band].attenuator_byp == 1 && toggle == 3)
        toggle = 1;

    if (toggle == 2) // toggle if ordered, else just set to current state such as for startup.
    {
        if (bandmem[curr_band].attenuator_byp == 1) // toggle the attenuator tracking state
            toggle = 0;
        else
            toggle = 1;
    }

    if (toggle == 1) // toggle is 1, turn on Atten
    {
        bandmem[curr_band].attenuator_byp = 1;        // Turn relay ON for hardware attenuator
        bandmem[curr_band].attenuator     = ATTEN_ON; // le the attenuator tracking state to ON
        MeterInUse                        = true;
        setMeter(ATTEN_BTN);
    }

    if (toggle == 0)
    {
        bandmem[curr_band].attenuator_byp = 0;         // Turn relay off bypassing hardware attenuator
        bandmem[curr_band].attenuator     = ATTEN_OFF; // set attenuator tracking state to OFF
    }

    if (toggle == 0 || toggle == -1)
    {
        MeterInUse = false;
        if (toggle != -1) clearMeter();
    }

    // Set the attenuation level from the value in the database
    Atten(0);  // 0 = no change to set attenuator level to value in database for this band

#ifdef SV1AFN_BPF
    // if (bandmem[curr_band].attenuator == ATTEN_OFF)
    // Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor += bandmem[curr_band].attenuator_dB;  // reset back to normal
    // else
    // Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor -= bandmem[curr_band].attenuator_dB;  // raise floor up due to reduced signal levels coming in

    codec1.muteHeadphone();
    //codec1.lineInLevel(0); // Audio out to Line-Out and TX board
    delay(50);
    //RampVolume(0.0, 1); //  0 = loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    if (bandmem[curr_band].attenuator_byp)
        bpf.setAttenuator(true); // Turn attenuator relay and status icon on or off
    else
        bpf.setAttenuator(false); // Turn attenuator relay and status icon on or off
    //RampVolume(user_settings[user_Profile].afGain, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    //RampVolume(0.01f, 0);   // Instant off.  0 to 1.0f for full scale.
    delay(50);
    codec1.unmuteHeadphone(); // Audio out to Line-Out and TX board
    AFgain(0); //   Reset afGain to last used to bypass thumps
#endif

    displayAttn();

    // DPRINTF("Set Attenuator Relay to "); DPRINT(bandmem[curr_band].attenuator_byp); DPRINTF(" Atten_dB is "); DPRINTLN(bandmem[curr_band].attenuator_dB);
    // DPRINTF(" and Ref Level is "); DPRINTLN(Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor);
}

/*******************************************************************************
 * Function Name: Atten()
 ********************************************************************************
 *
 * Summary:
 * Main function performs following functions:
 * 1: Configures the solid state attenuator by shifting 16 bits of address and
 *    atten level in LSB first.
 *
 * Parameters:
 *  atten = attenuation level to set in range of 0 to 100% (0 to 31 (in dB))
 *
 * Return:
 *  None.
 *
 *******************************************************************************/
COLD void Atten(int8_t delta)
{
    int8_t _atten = bandmem[curr_band].attenuator_dB;

    _atten += delta * 3;

    if (_atten > 100) // Keep in 0-100 range
        _atten = 100;
    if (_atten <= 0)
        _atten = 0;
    bandmem[curr_band].attenuator_dB = _atten; // Assign new valid value

    // DPRINT("Setting attenuator value to "); DPRINTLN(bandmem[curr_band].attenuator_dB);

    displayAttn(); // update the button value

    // CALL HARDWARE SPECIFIC ATENUATOR or FIXED ATTEN HERE
    // This is for the PE4302 only.
    digital_step_attenuator_PE4302(_atten); // Takes a 0 to 100 input, converts to the appropriate hardware steps such as 0-31dB in 1 dB steps
}

// PREAMP button
//   toggle = 0 sets Preamp state off
//   toggle = 1 sets Preamp state on
//   toggle = 2 toggles Preamp state
//   toggle = -1 or any value other than 0-2 sets Preamp state to current database value. Used for startup or changing bands.
//
COLD void Preamp(int8_t toggle)
{
    if (toggle == 2) // toggle state
    {
        if (bandmem[curr_band].preamp == PREAMP_ON)
            bandmem[curr_band].preamp = PREAMP_OFF;
        else
            bandmem[curr_band].preamp = PREAMP_ON;
    }
    else if (toggle == 1) // set to ON
        bandmem[curr_band].preamp = PREAMP_ON;
    else if (toggle == 0) // set to OFF
        bandmem[curr_band].preamp = PREAMP_OFF;
        // any other value of toggle pass through with unchanged state, jsut set the relays to current state

#ifdef SV1AFN_BPF
    // if (bandmem[curr_band].preamp == PREAMP_OFF)
    //   Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor -= 30;  // reset back to normal
    // else
    //   Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor += 30;  // lower floor due to increased signal levels coming in

    //RampVolume(0.0f, 0);   //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    codec1.muteHeadphone();
    delay(50);
    uint8_t rfg_temp = user_settings[user_Profile].rfGain;
    RFgain(-1);
    bpf.setPreamp((bool)bandmem[curr_band].preamp);
    delay(50);
    RFgain(rfg_temp);
    //RampVolume(0.01f, 0);   // Instant off.  0 to 1.0f for full scale.
    codec1.unmuteHeadphone(); // Audio out to Line-Out and TX board
    AFgain(0); //   Reset afGain to last used to bypass thumps
#endif

    displayPreamp();
    // DPRINTF("Set Preamp to "); DPRINTLN(bandmem[curr_band].preamp);
}

// RIT button
// Input:
// -1 = Clear MF knob and meter, use current value
//  0 = OFF - turn off button highlight color
//  1 = ON  - turn off button highlight and center pan window
//  2 = Toggle RIT ON/OFF state.
//  3 = Zero RIT offset value
COLD void setRIT(int8_t toggle)
{
    // global rit_offset is used add to VFOA when displaying or reporting or setting frequency.
    // It never changes VFOA value to keep memory and band change complications.
    // Will be toggled between 0 and the offset value when active.
    // This eliminates the need to test for RIT at every VFOA handling point
    if (toggle == 3 && bandmem[curr_band].RIT_en == ON) // Adjust the RIT value if active
    {
        toggle = 1;
    }
    else if (toggle == 3 && bandmem[curr_band].RIT_en == OFF) // Zero the RIT value
    {
        rit_offset_last = rit_offset = 0;
        RIT(0); // update frequency offset
    }

    if (toggle == 2) // toggle if ordered, else just set to current state such as for startup.
    {
        if (bandmem[curr_band].RIT_en == ON) // toggle the tracking state
            toggle = 0;
        else
            toggle = 1;
    }

    if (toggle == 1) // Turn on RIT and set to last stored offset
    {
        bandmem[curr_band].RIT_en = ON;
        rit_offset                = rit_offset_last; // restore last used value
        MeterInUse                = true;
        setMeter(RIT_BTN);
        RIT(0);
    }

    if (toggle == 0) // prevent acting on multiple calls
    {
        bandmem[curr_band].RIT_en = OFF;
        rit_offset                = 0; // now clear it
        RIT(0);
    }

    if (toggle == 0 || toggle == -1)
    {
        MeterInUse = false;
        if (toggle != -1) clearMeter();
    }

    // DPRINTF("setRIT: Set RIT ON/OFF to "); DPRINTLN(bandmem[curr_band].RIT_en);
    // DPRINTF("setRIT: Set RIT OFFSET to "); DPRINT(rit_offset); DPRINTF("  rit_offset_last = "); DPRINTLN(rit_offset_last);
}

// RIT offset control
COLD void RIT(int8_t delta)
{

    int16_t _offset;

    if (bandmem[curr_band].RIT_en == ON)
    {
        _offset = rit_offset; // Get cuurent value
        _offset += delta * tstep[user_settings[user_Profile].RIT_ts].step;

        if (_offset > 9999) // Limit the value between -10K and +10K
            _offset = 9999;
        if (_offset < -9999)
            _offset = -9999;

        rit_offset = _offset; // store in the global RIT offset value
        if (bandmem[curr_band].RIT_en == ON)
            rit_offset_last = rit_offset; // only store when RIT is active, prevent false zero for things like delayed calls from S-meter box updates

        // DPRINTF(RIT: Set RIT OFFSET to "); DPRINT(rit_offset); DPRINTF("  rit_offset_last = "); DPRINTLN(rit_offset_last);
    }
    selectFrequency(0); // no base freq change, just correct for RIT offset
    displayFreq();
    displayRIT();
}

// XIT button
// Input:
// -1 = Clear MF knob and meter, use current value
//  0 = OFF - turn off button highlight color
//  1 = ON  - turn off button highlight and center pan window
//  2 = Toggle XIT ON/OFF state.
//  3 = Zero XIT offset value
COLD void setXIT(int8_t toggle)
{
    // global xit_offset is used add to VFOA when displaying or reporting or setting frequency.
    // It never changes VFOA value to keep memory and band change complications.
    // Will be toggled between 0 and the offset value when active.
    // This eliminates the need to test for XIT at every VFO handling point
    if (toggle == 3 && bandmem[curr_band].XIT_en == ON) // Adjust the RIT value if active
    {
        toggle = 1;
    }
    else if (toggle == 3 && bandmem[curr_band].XIT_en == OFF) // Zero the RIT value
    {
        xit_offset_last = xit_offset = 0;
        XIT(0); // update frequency offset
    }

    if (toggle == 2) // toggle if ordered, else just set to current state such as for startup.
    {
        if (bandmem[curr_band].XIT_en == ON) // toggle the tracking state
            toggle = 0;
        else
            toggle = 1;
    }

    if (toggle == 1) // Turn on XIT and set to last stored offset
    {
        bandmem[curr_band].XIT_en = ON;
        xit_offset                = xit_offset_last; // restore last used value
        MeterInUse                = true;
        setMeter(XIT_BTN);
        XIT(0);
    }

    if (toggle == 0) // prevent acting on multiple calls
    {
        bandmem[curr_band].XIT_en = OFF;
        xit_offset                = 0; // now clear it
        XIT(0);
    }

    if (toggle == 0 || toggle == -1)
    {
        MeterInUse = false;
        if (toggle != -1) clearMeter();
    }

    if (toggle == 3) // Zero the RIT value
    {
        xit_offset_last = xit_offset = 0;
        XIT(0); // update frequency
    }

    // DPRINTF("setXIT: Set XIT ON/OFF to "); DPRINTLN(bandmem[curr_band].XIT_en);
    // DPRINTF("setXIT: Set XIT OFFSET to "); DPRINT(xit_offset); DPRINTF("  xit_offset_last = "); DPRINTLN(xit_offset_last);
}

// XIT offset control
COLD void XIT(int8_t delta)
{

    int16_t _offset;

    if (bandmem[curr_band].XIT_en == ON)
    {
        _offset = xit_offset; // Get cuurent value
        _offset += delta * tstep[user_settings[user_Profile].XIT_ts].step;

        if (_offset > 9999) // Limit the value between +/- 10KHz
            _offset = 9999;
        if (_offset < -9999)
            _offset = -9999;

        xit_offset = _offset; // store in the global XIT offset value
        if (bandmem[curr_band].XIT_en == ON)
            xit_offset_last = xit_offset; // only store when XIT is active, prevent false zero for things like delayed calls from S-meter box updates

        //DPRINTF("XIT: Set XIT OFFSET to "); DPRINT(xit_offset); DPRINTF("  xit_offset_last = "); DPRINTLN(xit_offset_last);
    }
    selectFrequency(0); // no base freq change, just correct for RIT offset
    displayFreq();
    displayXIT();
}

// SPLIT button
//   state = 0 sets Split state off
//   state = 1 sets Split state on
//   state = 2 toggles Split state
COLD void Split(uint8_t state)
{
    if (state == 0)
        bandmem[curr_band].split = OFF;
    if (state == 1)
        bandmem[curr_band].split = ON;
    if (state == 2)
    {
        if (bandmem[curr_band].split == ON)
            bandmem[curr_band].split = OFF;
        else if (bandmem[curr_band].split == OFF)
            bandmem[curr_band].split = ON;
    }
    displaySplit();
    displayFreq();
    // DPRINTF("Set Split to "); DPRINTLN(bandmem[curr_band].split);
}

// ATU button
// 0 = OFF  1 = ON   2 = Toggle  -1 = update to database state
COLD void ATU(uint8_t state)
{
    if ((bandmem[curr_band].ATU == ON && state == 2) || state == 0)
        bandmem[curr_band].ATU = OFF;
    else if ((bandmem[curr_band].ATU == OFF && state == 2) || state == 1)
        bandmem[curr_band].ATU = ON;

    if (bandmem[curr_band].ATU == ON)
        TwoToneTest = ON; // For test turn Two Tone test on and off.  When off the Mic is enabled.
    else
        TwoToneTest = OFF; // For test turn Two Tone test on and off.  When off the Mic is enabled.
    //
    //   Insert any future ATU hardware setup calls
    //

    displayATU();
    // DPRINTF("Set ATU to "); DPRINTLN(bandmem[curr_band].ATU);
}

// setZoom()
//   toogle = 0 sets Zoom state to current database value. Used for startup or changing bands.
//   toogle = -1 deregisters Zoom MF control
//   toogle = 1 sets Zoom state on
//   toogle = 2 toggles Zoom state
COLD void setZoom(int8_t toggle)
{
    // DPRINT("toggle = "); DPRINTLN(toggle);

    if (toggle == 2) // toggle if ordered, else just set to current state such as for startup.
    {
        if (user_settings[user_Profile].zoom_level >= 2) // toggle the tracking state
            user_settings[user_Profile].zoom_level = 0;
        else
            user_settings[user_Profile].zoom_level += 1;
        toggle = 1;
    }

    if (toggle == 1) // Set button to on to track as active
    {
        MeterInUse = true;
        setMeter(ZOOM_BTN);
        Zoom(0);
    }

    if (toggle == 0 || toggle == -1)
    {
        if (toggle == 0) Zoom(0);
        MeterInUse = false;
        if (toggle != -1) clearMeter();
    }

    // displayZoom();
    // DPRINTF("Set Zoom to "); DPRINTLN(user_settings[user_Profile].zoom_level);
}

// ---------------------------Zoom() ---------------------------
//   Input: 2 = step to next based on last direction (starts upwards).  Ramps up then down then up.
//          1 = step up 1 (zoomed in more)
//         -1 = step down 1 (zoom out more )
//          0 = use last zoom level used from user profile
//   Zoom levels are x1, x2 and x4  Off is same as x1.
//
COLD void Zoom(int8_t dir)
{
    static int8_t direction = -1;                                     // remember last direction
    int8_t _zoom_Level      = user_settings[user_Profile].zoom_level; // Get last known value from user profile

    if (dir != 0)
    {
        if (dir >= 2)
            dir = 1;

        // 1. Limit to allowed step range
        // 2. Cycle up and at top, cycle back down, do not roll over.
        if (_zoom_Level <= 0)
        {
            _zoom_Level = 0;
            direction   = 1; // cycle upwards
        }
        if (_zoom_Level >= ZOOM_NUM - 1)
        {
            _zoom_Level = ZOOM_NUM - 1;
            direction   = -1;
        }
        if (dir == 0)
            _zoom_Level += direction; // Index our step up or down
        else
            _zoom_Level += dir; // forces a step higher or lower then current
        // Ensure we have legal values
        if (_zoom_Level <= 0)
            _zoom_Level = 0;
        if (_zoom_Level >= ZOOM_NUM - 1)
            _zoom_Level = ZOOM_NUM - 1;
        user_settings[user_Profile].zoom_level = (uint8_t)_zoom_Level; // We have our new table index value
    }

    switch (_zoom_Level)
    {
        #ifdef FFT_1024
                case ZOOMx1:
                    Change_FFT_Size(1024, sample_rate_Hz);
                    break; // Zoom farthest in
        #endif
        #ifdef FFT_2048
                case ZOOMx2:
                    Change_FFT_Size(2048, sample_rate_Hz);
                    break; // Zoom farthest in
        #endif
        #ifdef FFT_4096
                case ZOOMx4:
                    Change_FFT_Size(4096, sample_rate_Hz);
                    break; // Zoom farthest in
        #endif
        default:
            Change_FFT_Size(FFT_SIZE, sample_rate_Hz);
            break; // Zoom farthest in
    }

    // DPRINTF("Zoom level set to  "); DPRINTLN(zoom[_zoom_Level].zoom_name);
    displayZoom();
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

    // DPRINTF("Set Fine to "); DPRINTLN(user_settings[user_Profile].fine);
}

// ANT button
// Toggle between antenna 1 and 2.  Turns GPIO_ANT_PIN ON and OFF to control a relay.
COLD void Ant()
{
    if (bandmem[curr_band].ant_sw == 1)
    {
        bandmem[curr_band].ant_sw = 2;
        if (GPIO_ANT_ENABLE)
            digitalWrite(GPIO_ANT_PIN, 1);
    }
    else if (bandmem[curr_band].ant_sw == 2)
    {
        bandmem[curr_band].ant_sw = 1;
        if (GPIO_ANT_ENABLE)
            digitalWrite(GPIO_ANT_PIN, 0);
    }

    displayANT();
    // DPRINTF("Set Ant Sw to "); DPRINTLN(bandmem[curr_band].ant_sw);
}

// AF GAIN button activate control
COLD void setAFgain(int8_t toggle)
{
    if (toggle == 2) // toggle if ordered, else just set to current state such as for startup.
    {
        if (user_settings[user_Profile].afGain_en) // toggle the tracking state
            toggle = 0;
        else
            toggle = 1;
    }

    if (toggle == 1) // Set button to on to track as active
    {
        if (user_settings[user_Profile].afGain_en == ON && MeterInUse) // if already on, assume this was called by a long press and we want to turn off the meter and but not the feature.
            clearMeter();
        else
        {
            user_settings[user_Profile].afGain_en = ON; // set the af tracking state to ON
            MeterInUse = true;
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

    // DPRINT(" AF Gain ON/OFF set to  ");
    // DPRINTLN(user_settings[user_Profile].afGain_en);
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

    // LineOutLevel is 0 to 31 with 0-12 clipping.  So 13-31 is usable range.  This scale is inverted.  13 is loudest, 31 lowest output.
    if (user_settings[user_Profile].xmit == OFF)
        _afLevel = user_settings[user_Profile].afGain; // Get last absolute volume setting as a value 0-100
    else
        _afLevel = user_settings[user_Profile].mic_Gain_level; // Get last absolute volume setting as a value 0-100

    // DPRINT(" TEST AF Level requested "); DPRINTLN(_afLevel);

    _afLevel += delta * 2; // convert percentage request to a single digit float

    // DPRINT(" TEST AF Level absolute "); DPRINTLN(_afLevel);

    if (_afLevel > 100) // Limit the value between 0.0 and 1.0 (100%)
        _afLevel = 100;
    if (_afLevel < 1)
        _afLevel = 1; // do not use 0 to prevent divide/0 error

    // LineOutLevel is 0 to 31 with 0-12 clipping.  So 13-31 is usable range.  This scale is inverted.  13 is loudest, 31 lowest output.
    if (user_settings[user_Profile].xmit == OFF)
    {
        user_settings[user_Profile].afGain = _afLevel; // update memory
        // codec1.lineOutLevel(user_settings[user_Profile].lineOut_RX * _afLevel/100); // skip when in TX to act as Mic Level Adjust control
    }
    else // Control Mic Gain and Power out
    {
        //   Temp mic/lineout level control until a real control is created for these
        _afLevel                                   = 100;                  // force to 100%
        user_settings[user_Profile].mic_Gain_level = _afLevel;             // 0 to 100 mic gain range
        codec1.micGain(user_settings[user_Profile].mic_Gain_level * 0.63); // adjust for 0 to 63dB
        // codec1.micGain(63);  // adjust for 0 to 63dB
        codec1.lineOutLevel(user_settings[user_Profile].lineOut_TX * _afLevel / 100); // skip when in TX to act as Mic Level Adjust control
        // codec1.lineOutLevel(14); // skip when in TX to act as Mic Level Adjust control
        DPRINT("Mic Gain (0-63dB)= ");
        DPRINTLN(_afLevel * 0.63);
        DPRINT("Power Out(0-100%)= ");
        DPRINTLN(_afLevel);
        codec1.muteHeadphone();
    }
    // Convert linear pot to audio taper pot behavior
    // Use new afLevel
    float val = log10f(_afLevel) / 2.0f;
    // DPRINT(" Volume set to  log = ");
    // DPRINTLN(val);

#ifdef USE_FFT_LO_MIXERxxxx
    AudioNoInterrupts();
    FFT_LO_Mixer_I.frequency((_afLevel)*200.0f);
    FFT_LO_Mixer_Q.frequency((_afLevel)*200.0f);
    AudioInterrupts();
#endif

    // RampVolume handles the scaling. Must set the LineOutLevel to the desired max though.
    RampVolume((float)val, 2); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    // DPRINT(" Volume set to  ");
    // DPRINTLN(_afLevel);
    displayAFgain();
}

// RF GAIN button activate control
COLD void setRFgain(int8_t toggle)
{
    if (toggle == 2) // toggle if ordered, else just set to current state such as for startup.
    {
        if (user_settings[user_Profile].rfGain_en) // toggle the attenuator tracking state
            toggle = 0;
        else
            toggle = 1;
    }

    if (toggle == 1) // Set button to on to track as active
    {
        user_settings[user_Profile].rfGain_en = ON;
        MeterInUse                            = true;
        setMeter(RFGAIN_BTN);
    }

    if (toggle == 0 || toggle == -1)
    {
        user_settings[user_Profile].rfGain_en = OFF;
        MeterInUse                            = false;
        if (toggle != -1)
            clearMeter();
    }

    // DPRINT(" RF Gain ON/OFF set to  ");
    // DPRINTLN(user_settings[user_Profile].rfGain_en);
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

    _rfLevel = user_settings[user_Profile].rfGain; // Get last absolute setting as a value 0-100

    _rfLevel += delta * 4; // convert percentage request to a single digit float

    if (_rfLevel > 100) // Limit the value between 0.0 and 1.0 (100%)
        _rfLevel = 100;
    if (_rfLevel < 1)
        _rfLevel = 1; // do not use 0 to prevent divide/0 error

    // Store new value as 0 to 100%
    user_settings[user_Profile].rfGain = _rfLevel; // 0 to 100 range, ,linear

    // LineIn is 0 to 15 with 15 being the most sensitive
    codec1.lineInLevel(user_settings[user_Profile].lineIn_level * user_settings[user_Profile].rfGain / 100);

    // Attennuating a gain stage is fast and helps give more RFgain effectivness
    I_Switch.gain(0, (float)_rfLevel / 100); //  1 is RX, 0 is TX
    Q_Switch.gain(0, (float)_rfLevel / 100); //  1 is RX, 0 is TX

    // DPRINT("CodecLine IN level set to ");
    // DPRINTLN(user_settings[user_Profile].lineIn_level * user_settings[user_Profile].rfGain/100);
    // DPRINT("RF Gain level set to  ");
    // DPRINTLN(_rfLevel);
    displayRFgain();
}

// PAN ON/OFF button activate control
// -1 is clear meter- used by MF knob and S-meter box   leave pan window alone.
//  0 is OFF - turn off button highlight and center pan window
//  1 is ON
//  2 is toggle ON/OFF state
//  3 is center the pan window
COLD void setPAN(int8_t toggle)
{
    // DPRINT("PAN toggle = "); DPRINTLN(toggle);

    if (toggle == 2) // toggle if ordered, else just set to current state such as for startup.
    {
        if (user_settings[user_Profile].pan_state == ON) // toggle the tracking state
            toggle = 0;
        else
            toggle = 1;
    }

    if (toggle == 1) // Set button to on to track as active
    {
        user_settings[user_Profile].pan_state = ON;
        MeterInUse                            = true;
        setMeter(PAN_BTN);
        PAN(0);
    }

    if (toggle == 0 || toggle == -1 || toggle == 3)
    {
        user_settings[user_Profile].pan_state = OFF;
        MeterInUse                            = false;
        if (toggle != -1) clearMeter();
        if (toggle == 3) // Zero the pan window and turn off on long button press
        {
            pan                                   = 0.0f;
            user_settings[user_Profile].pan_level = 50;
        }
    }
    // DPRINT(" PAN state is  ");
    // DPRINTLN(user_settings[user_Profile].pan_state);
    displayPan();
}

// PAN Adjust
//
//  Input:   0 no change
//          -X Reduce by X%
//          +X Increase by X%
//
COLD void PAN(int8_t delta)
{
    int8_t _panLevel;

    _panLevel = (int8_t)user_settings[user_Profile].pan_level; // Get last absolute setting as a value 0-100
    _panLevel += delta;                                        // convert percentage request to a single digit float

    if (_panLevel >= 100) // Limit the value between 0.0 and 1.0 (100%)
        _panLevel = 100;
    if (_panLevel <= 0)
        _panLevel = 0;

    if (user_settings[user_Profile].pan_state == OFF && delta == 0)
    {
        pan                                   = 0.0f;
        _panLevel                             = 50; // 50 is halfway or 0.0f
        user_settings[user_Profile].pan_level = _panLevel;
    }
    else
    {                                                                           // bins that will not fit on the display can be viewed by shifting our left edge index.  Assuming 1 px per bin
        pan                                   = float(_panLevel - 50) / 100.0f; // 0 is 0.0.  50 is +0.5, -50 is -0.5.
        user_settings[user_Profile].pan_level = _panLevel;
    }

    // DPRINT("Control Change: PAN set to  ");
    // DPRINTLN(user_settings[user_Profile].pan_level-50); // convert to -100 to +100 for UI
    displayPan();
}

// XMIT button
COLD void Xmit(uint8_t state) // state ->  TX=1, RX=0; Toggle =2
{
    uint8_t mode_idx;

    mode_idx = bandmem[curr_band].mode_A;

    if ((user_settings[user_Profile].xmit == ON && state == 2) || state == 0) // Transmit OFF
    {
        user_settings[user_Profile].xmit = OFF;
        if (PTT_OUT1 != 255)
            digitalWrite(PTT_OUT1, HIGH);
 
#ifdef USE_RS_HFIQ
        RS_HFIQ.send_fixed_cmd_to_RSHFIQ("*X0"); // RS-HFIQ TX OFF
        delay(5);
        selectFrequency(0);
#endif
        // enable line input to pass to headphone jack on audio card, set audio levels
        TX_RX_Switch(OFF, mode_idx, OFF, OFF, OFF, OFF, 0.5f);
        // int TX,                 // TX == 1, RX == 0
        // uint8_t mode_sel,       // Current VFO mode index
        // float   Mic_On,         // 0.0f(OFF) or 1.0f (ON)
        // float   ToneA,          // 0.0f(OFF) or 1.0f (ON)
        // float   ToneB,          // 0.0f(OFF) or 1.0f (ON)
        // float   TestTone_Vol)   // 0.90 is max, clips if higher. Use 0.45f with 2 tones
        DPRINTLN("XMIT(): TX OFF");
    }
    else if ((user_settings[user_Profile].xmit == OFF && state == 2) || state == 1) // Transmit ON
    {
        user_settings[user_Profile].xmit = ON;
        if (PTT_OUT1 != 255)
            digitalWrite(PTT_OUT1, LOW);

#ifdef USE_RS_HFIQ
        selectFrequency(0);
        delay(5);                                // slight delay needed for reliable changeover
        RS_HFIQ.send_fixed_cmd_to_RSHFIQ("*X1"); // RS-HFIQ TX ON
#endif

        TX_Timeout.reset(); // Reset our Runaway TX timer.  Main loop will watch for this to trip calling back here to flip back to RX.

        // enable mic input to pass to line out on audio card, set audio levels
        if (TwoToneTest)                                         // do test tones
            TX_RX_Switch(ON, mode_idx, OFF, OFF, ON, ON, 0.05f); // TestOne_Vol => 0.90 is max, clips if higher. Use 0.45f with 2 tones
        else if (mode_idx == DATA || mode_idx == DATA_REV)       // Mic on, turn off test tones
            TX_RX_Switch(ON, mode_idx, OFF, ON, OFF, OFF, OFF);  // Turn on USB input, Turn Mic OFF
        else
            TX_RX_Switch(ON, mode_idx, ON, OFF, OFF, OFF, OFF); // Turn Mic input ON, Turn USB IN OFF
        DPRINTLN("XMIT(): TX ON");
    }
    displayXMIT();
    displayFreq();
    // DPRINT("Set XMIT to "); DPRINTLN(user_settings[user_Profile].xmit);
}

// NB ON/OFF button
// -1 is clear meter- used by MF knob and S-meter box
//  0 is OFF - turn off button highlight and center pan window
//  1 is ON
//  2 is toggle ON/OFF state
COLD void setNB(int8_t toggle)
{
    // char string[80];   // print format stuff
    if (toggle == 2) // toggle if ordered, else just set to current state such as for startup.
    {
        if (user_settings[user_Profile].nb_en) // toggle the tracking state
            toggle = 0;
        else
            toggle = 1;
    }
    if (toggle == 1)
    {
        user_settings[user_Profile].nb_en = ON;
        NBLevel(0);
        MeterInUse = true;
        setMeter(NB_BTN);
    }

    if (toggle == 0)
    {
        user_settings[user_Profile].nb_en = OFF;
        NBLevel(0);
    }

    if (toggle == 0 || toggle == -1)
    {
        MeterInUse = false;
        if (toggle != -1)
            clearMeter();
    }

    // DPRINTF("Set NB to "); DPRINTLN(user_settings[user_Profile].nb_en);
    displayNB();
}

// Adjust the NB level. NB() turn on and off only calling this to initiialize the current level with delta = 0
// NB Level Adjust - range is 1 to 6
//  0 is Use Current value in DB.  If NB_en is OFF, then disable NB.
//  + increments
//  - decrements
COLD void NBLevel(int8_t delta)
{
    int8_t _nbLevel;

    // DPRINT(" TEST NB delta "); DPRINTLN(delta);

    _nbLevel = user_settings[user_Profile].nb_level; // Get last known value

    // DPRINT(" TEST NB Level "); DPRINTLN(_nbLevel);

    _nbLevel += delta; // increment up or down

    /*if (_nbLevel > 100)         // Limit the value
        _nbLevel = 100;
    if (_nbLevel < 0)
        _nbLevel = 0;

    DPRINT(" TEST NB New Level "); DPRINTLN(_nbLevel);
    user_settings[user_Profile].nb_level = _nbLevel;  // We have our new table index value

    // Convert from 0 to 100% to number of valid steps
    _nbLevel *= round(NB_SET_NUM-1 +0.5)/100;
    DPRINT(" TEST NB Converted Level "); DPRINTLN(_nbLevel);
*/
    if (_nbLevel > NB_SET_NUM - 1) // Limit the value
        _nbLevel = NB_SET_NUM - 1;
    if (_nbLevel < 1)
        _nbLevel = 1;

    user_settings[user_Profile].nb_level = _nbLevel; // We have our new table index value

    if (user_settings[user_Profile].nb_en == ON) // Adjust the value if on
    {
        NoiseBlanker.showError(1);
        NoiseBlanker.useTwoChannel(true); // true enables a path through the "I"  or left side for I and Q
        NoiseBlanker.enable(true);        // turn on NB for I
        // void setNoiseBlanker(float32_t _threshold, uint16_t _nAnticipation, uint16_t _nDecay)
        NoiseBlanker.setNoiseBlanker(nb[user_settings[user_Profile].nb_level].nb_threshold,
                                     nb[user_settings[user_Profile].nb_level].nb_nAnticipation,
                                     nb[user_settings[user_Profile].nb_level].nb_decay); // for I
        // threshold recommended to be between 1.5 and 20, closer to 3 maybe best.
        // nAnticipation is 1 to 125
        // Decay is 1 to 10.
    }
    else // NB is disabled so bypass
    {
        NoiseBlanker.enable(false); //  NB block is bypassed
        // DPRINTF("NB Disabled");
    }

    // DPRINTF("NB level set to  "); DPRINTLN(_nbLevel);
    displayNB();
}

// NR button
COLD void setNR()
{
    if (user_settings[user_Profile].nr_en > NROFF)
    {
        user_settings[user_Profile].nr_en = NROFF;
        LMS_Notch.enable(false);
    }
    else if (user_settings[user_Profile].nr_en == NROFF)
    {
        user_settings[user_Profile].nr_en = NR1;
        LMS_Notch.enable(true);
        DPRINTLN(LMS_Notch.initializeLMS(1, 32, 4)); // <== Modify to suit  2=Notch 1=Denoise
        LMS_Notch.setParameters(0.05f, 0.999f);      // (float _beta, float _decay);
    }
    displayNR();
    // DPRINTF("Set NR to "); DPRINTLN(user_settings[user_Profile].nr_en);
}

// Enet button
COLD void Enet()
{
    if (user_settings[user_Profile].enet_output == ON)
        user_settings[user_Profile].enet_output = OFF;
    else if (user_settings[user_Profile].enet_output == OFF)
        user_settings[user_Profile].enet_output = ON;

    displayEnet();
    // DPRINTF("Set Ethernet to "); DPRINTLN(user_settings[user_Profile].enet_output);
}

// Spot button
COLD void Spot()
{
    if (user_settings[user_Profile].spot == OFF)
        user_settings[user_Profile].spot = ON;
    else
        user_settings[user_Profile].spot = OFF;
    // displaySpot();
    // DPRINTF("Set Spot to "); DPRINTLN(user_settings[user_Profile].spot);
}

// REF LEVEL button activate control
COLD void setRefLevel(int8_t toggle)
{
    if (toggle == 2) // toggle if ordered, else just set to current state such as for startup.
    {
        if (std_btn[REFLVL_BTN].enabled) // toggle the tracking state
            toggle = 0;
        else
            toggle = 1;
    }

    if (toggle == 1) // Set button to on to track as active
    {
        std_btn[REFLVL_BTN].enabled = ON;
        if (MF_client != REFLVL_BTN)
        {
            MeterInUse = true;
            setMeter(REFLVL_BTN);
        }
        // DPRINTF("Set REFLVL to ON "); DPRINT(std_btn[REFLVL_BTN].enabled);
    }

    if (toggle == 0 || toggle == -1)
    {
        std_btn[REFLVL_BTN].enabled = OFF;
        MeterInUse                  = false;
        if (toggle != -1)
            clearMeter();
        // DPRINTF("Set REFLVL to OFF "); DPRINT(std_btn[REFLVL_BTN].enabled);
    }

    displayRefLevel();
    // DPRINTF(" and Ref Level is "); DPRINTLN(Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor);
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
    // DPRINTF("Set Reference Level to "); DPRINTLN(Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_floor);
}

// Notch button
COLD void Notch()
{
    if (user_settings[user_Profile].notch == ON)
    {
        user_settings[user_Profile].notch = OFF;
        LMS_Notch.enable(false);
    }
    else if (user_settings[user_Profile].notch == OFF)
    {
        user_settings[user_Profile].notch = ON;
        LMS_Notch.enable(true);
        DPRINTLN(LMS_Notch.initializeLMS(2, 32, 4)); // <== Modify to suit  2=Notch 1=Denoise
        LMS_Notch.setParameters(0.05f, 0.999f);      // (float _beta, float _decay);
    }

    displayNotch();
    // DPRINT("Set Notch to "); DPRINTLN(user_settings[user_Profile].notch);
}

// BAND UP button
COLD void BandUp()
{
    changeBands(1);
    displayBandUp();
    // DPRINTF("Set Band UP to "); DPRINTLN(bandmem[curr_band].band_num,DEC);
}

// BAND DOWN button
COLD void BandDn()
{
    // DPRINTLN("BAND DN");
    changeBands(-1);
    displayBandDn();
    // DPRINTF("Set Band DN to "); DPRINTLN(bandmem[curr_band].band_num,DEC);
}

// BAND button
// Bandstack will copy in one of the alternative saved VFOA values and cycle throgh them.  VFO_A_last is always the last active value. Last_1 the previous, and last_2 previous to that.
COLD void Band(uint8_t new_band)
{
    if (std_btn[BAND_BTN].enabled == ON)
    {
        if (popup && new_band != 255)
        {
            if (curr_band == new_band) // already on this band so new request must be for bandstack, cycle through, saving changes each time
            {
                DPRINT("Previous VFO A on Band ");
                DPRINTLN(curr_band);
                uint64_t temp_vfo_last;
                uint8_t temp_mode_last;

                temp_vfo_last                   = bandmem[curr_band].vfo_A_last; // Save  current freq and mode
                temp_mode_last                  = bandmem[curr_band].mode_A;
                bandmem[curr_band].vfo_A_last   = bandmem[curr_band].vfo_A_last_1; // shuffle previous up to curent
                bandmem[curr_band].mode_A       = bandmem[curr_band].mode_A_1;
                bandmem[curr_band].vfo_A_last_1 = bandmem[curr_band].vfo_A_last_2; // shuffle more
                bandmem[curr_band].mode_A_1     = bandmem[curr_band].mode_A_2;
                bandmem[curr_band].vfo_A_last_2 = temp_vfo_last; // let changeBands compute new band based on VFO frequency
                bandmem[curr_band].mode_A_2     = temp_mode_last;
                VFOA                            = bandmem[curr_band].vfo_A_last; // store in the Active VFO register
            }
            else
            {
                DPRINT("Last VFO A on Band ");
                DPRINTLN(new_band);
                VFOA = bandmem[new_band].vfo_A_last; // let changeBands compute new band based on VFO frequency
            }
            changeBands(0);
        }
        std_btn[BAND_BTN].enabled = OFF;
        displayBand_Menu(0); // Exit window
    }
    else
    {
        std_btn[BAND_BTN].enabled = ON;
        displayBand_Menu(1); // Init window
    }
    displayBand();
    displayMode();
    displayFilter();
    // displayRefresh();
    displayFreq(); // show freq on display
    // DPRINTF("Set Band to "); DPRINTLN(bandmem[curr_band].band_num,DEC);
}

// DISPLAY button
COLD void Display()
{
#ifndef BYPASS_SPECTRUM_MODULE
    if (Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_dot_bar_mode)
    {
        display_state                                                          = 0;
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_dot_bar_mode = 0;
    }
    else
    {
        display_state                                                          = 1;
        Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_dot_bar_mode = 1;
    }
    drawSpectrumFrame(user_settings[user_Profile].sp_preset);
#endif
    // popup = 1;
    // pop_win_up(1);
    displayDisplay();
    // DPRINTF("Set Display Button to "); DPRINTLN(display_state);
}

COLD void TouchTune(int16_t touch_Freq)
{

    if (popup == 1) return; // skip if menu window is active

#ifndef BYPASS_SPECTRUM_MODULE
    int64_t pk;

    pk = pan * (fft_size - SCREEN_WIDTH);                                                   // pan offset in fft_bin count
    touch_Freq -= Sp_Parms_Def[user_settings[user_Profile].sp_preset].spect_width / 2 - pk; // adjust coordinate relative to center accounting for pan offset
    int64_t _newfreq = touch_Freq * fft_bin_size * 2;                                       // convert touch X coordinate to a frequency and jump to it.
    // We have our new target frequency from touch
    // DPRINTF("\npan offset (bins from center)     ="); DPRINTLN(pk);
    // DPRINTF("touch_Freq (bins from center)     ="); DPRINTLN(touch_Freq);
    // DPRINTF("Touch target change in Hz         ="); DPRINTLN(_newfreq);
    // DPRINTF("New target touch VFO (Hz)         ="); DPRINTLN(formatVFO(VFOA+_newfreq));
    // Suggested frequency from the Peak Frequency function in the spectrum library - may not be the one we want! It is just the strongest.
    // DPRINTF("\nFreq_Peak (Hz)                  ="); DPRINTLN(formatVFO(Freq_Peak));
    // DPRINTF("Target VFOA - VFO_peak (Hz)       ="); DPRINTLN(VFOA+_newfreq-Freq_Peak);

    DPRINT(F("Touch-Tune frequency is "));
    VFOA += _newfreq;

    // If the Peak happens to be close to the touch target frequency then we can use that to fine tune the new VFO
    if (abs(VFOA - Freq_Peak < 1000))
    {
        if (bandmem[curr_band].mode_A == CW || bandmem[curr_band].mode_A == CW_REV)
            VFOA = Freq_Peak - ModeOffset; // user_settings[user_Profile].pitch;
        else
            VFOA = Freq_Peak; // bin number from spectrum
    }
    DPRINTLN(formatVFO(VFOA));
#endif
    selectFrequency(0);
    displayFreq();
}

COLD void selectStep(uint8_t fndx)
{
    if (fndx <= 1)
    {
        fndx = 0;
    }

    if (fndx >= TS_STEPS - 1)
    {
        fndx = TS_STEPS - 1;
    }
    //  remove the int fndx arg if using a global fndx
    bandmem[curr_band].tune_step = fndx;
    displayRate();
}

COLD void selectAgc(uint8_t andx)
{
    struct AGC* pAGC = &agc_set[andx];

    if (andx >= AGC_SET_NUM)
        andx = AGC_OFF; // Cycle around

    if (andx < AGC_OFF)
        andx = AGC_SET_NUM - 1; // Cycle around

    bandmem[curr_band].agc_mode = andx;

    if (andx == AGC_OFF)
    {
        codec1.autoVolumeControl(
            pAGC->agc_maxGain,
            pAGC->agc_response,
            pAGC->agc_hardlimit,
            pAGC->agc_threshold,
            pAGC->agc_attack,
            pAGC->agc_decay);
        codec1.autoVolumeDisable();
        codec1.audioProcessorDisable();
    }
    else
    {
        codec1.autoVolumeControl(
            pAGC->agc_maxGain,
            pAGC->agc_response,
            pAGC->agc_hardlimit,
            pAGC->agc_threshold,
            pAGC->agc_attack,
            pAGC->agc_decay);
        codec1.audioPostProcessorEnable();
        codec1.autoVolumeEnable();
    }
    // displayAgc();
}

// Turns meter off
void clearMeter(void)
{
    DPRINTLN("Turn OFF meter");
    MeterInUse = false;
    // blank out text line from overruns
    draw_2_state_Button(SMETER_BTN, &std_btn[SMETER_BTN].show);
    set_MF_Service(encoder_list[default_MF_slot].default_MF_client); // will turn off the button, if any, and set the default as new owner.
    MF_default_is_active = true;
}

// Turns meter on and assigns function to the new MF focus
void setMeter(uint8_t id)
{
    if (MF_client != id)
    {
        // DPRINTLN("Turn ON meter");
        set_MF_Service(id); // reset encoder counter and set up for next read if any until another functionm takes ownership
        MF_default_is_active = false;
    }
}

// Toggle the assigned function on an encoder shaft.
void setEncoderMode(uint8_t role)
{
    if (MF_client != role) // Probably don't do this if assigned to a multifunction knob sicne it is temporary
    {
        DPRINT("Encoder Switch ID = ");
        DPRINTLN(role);
        int slot;
        // find enabled aux encoders in order of slot assignment and copy to local var for processing in a loop, skip VFO slot
        for (slot = 1; slot < NUM_AUX_ENCODERS; slot++)
        {
            #ifdef DEBUG
                uint8_t _type, _enabled, _id, _mfenc, _roleA, _a_active, _roleB, _tap, _press;

                _type     = encoder_list[slot].type;
                _id       = encoder_list[slot].id;
                _enabled  = encoder_list[slot].enabled;
                _mfenc    = encoder_list[slot].default_MF_client;
                _roleA    = encoder_list[slot].role_A;
                _a_active = encoder_list[slot].a_active;
                _roleB    = encoder_list[slot].role_B;
                _tap      = encoder_list[slot].tap;
                _press    = encoder_list[slot].press;

                DEBUG_PRINTF("Slot#=%1d type=%1d id=0x%1d enabled=%1d MFENC?=%2d RoleA=%2d, A_active=%1d RoleB=%2d TAP=%2d PRESS=%2d\n",
                            slot, _type, _id, _enabled, _mfenc, _roleA, _a_active, _roleB, _tap, _press);
            #else
                uint8_t _tap;
                _tap = encoder_list[slot].tap;
            #endif

            // when switch is tapped or pressed, toggle which function is assigned to the encoder rotation
            // Uses the KEYWORD TOGGLE in thr encoder_list table
            if (_tap == role)
            {
                switch (_tap)
                {
                    case SW1_BTN:
                    case SW2_BTN:
                    case SW3_BTN:
                    case SW4_BTN:
                    case SW5_BTN:
                    case SW6_BTN:
                        DPRINT("Toggle Active Control ");
                        DPRINTLN(role);
                        encoder_list[slot].a_active ^= 1;
                        update_icon_outline();
                        // if was MF knob, turn on the normal meter asap to return to normal use fast.
                        if (MF_client != MFTUNE && !encoder_list[slot].a_active)
                        {
                            MF_Timeout.reset();
                            clearMeter();
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

// Takes a 0 to 100 input, converts to the appropriate hardware steps such as 0-31dB in 1 dB steps
// Code is for the PE4302 digital step attenuator
// Can clone and modify this for other hardware, call it from teh ATTEN() function
COLD void digital_step_attenuator_PE4302(int16_t _atten)
{
    #ifndef PE4302
        DPRINTLN(F("Error: PE4302 digital step attenuator not configured, exiting"));
        return; // Wrong hardware if not PE4302
    #else

    const uint8_t atten_size_31 = 62;  // 62 steps  31dB in 0.5 dB steps. Use 31 for 1dB steps
  
    char atten_str[8]  = {'\0'};
    char atten_data[8] = {'\0'};
    uint8_t i;
    int16_t atten;

    // scale 0 to 100% to size of attenuator hardware.  Assuming 0 to 31dB here.
    atten = round(((_atten * atten_size_31 / 100) - 0.5)); // round down to get 0-31 range

    if (atten >= atten_size_31) // will crash is exceed 0 to 31!
        atten = atten_size_31;
    if (atten < 0)
        atten = 0;

    //DPRINT("digital step converted = ");DPRINTLN(atten);

    // comment out for 0.5dB steps
    //atten *= 2; // shift the value x2 so the LSB controls the 1dB step.

    /* Convert to 8 bits of  0 and 1 format */
    itoa(atten, atten_str, 2);
    //DPRINTLN(atten_str);   // should be 6 bits of binary in the range of 0 to 64
    
    // Convert to 6 bits of  0 and 1 format 
    // pad with leading 0s as needed.  6 bits for the PE4302 + '\0' at end for 7 bytes
    //snprintf(atten_data, 7, "%06s", atten_str);   // works but get compiler warning for 0 and string together    
    for (i = 0; (i < 6 - strlen(atten_str)); i++)
    {
        atten_data[i] = '0';
    }
    strncat(atten_data, atten_str, strlen(atten_str));
    //DPRINTLN(atten_data);   // should be 6 bits of binary in the range of 0 to 64

    //  LE = 0 to allow writing data into shift register
    digitalWrite(Atten_LE, (uint8_t)OFF);
    digitalWrite(Atten_DATA, (uint8_t)OFF);
    digitalWrite(Atten_CLK, (uint8_t)OFF);
    delayMicroseconds(10);
 
    //  Now loop for 6 bits, set data on Data pin and toggle Clock pin.
    //    Start with the MSB first so start at the left end of the string
    for (i = 0; i < 6; i++)
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
    #endif
}

//
// Changes to the correct band settings for the new target frequency.
// If the new frequency is below or above the band limits it returns a value of 0
//
HOT uint64_t find_new_band(uint64_t new_frequency, uint8_t &_curr_band)
{
    int i;

    for (i=BANDS-1; i>=0; i--)    // start at the top and look for first band that VFOA fits under bandmem[i].edge_upper
    {
        #ifdef DBG
            DPRINTF("find_band(): Edge_Lower Search = "); DPRINTLN(bandmem[i].edge_lower);
        #endif
        if (new_frequency >= bandmem[i].edge_lower && new_frequency <= bandmem[i].edge_upper) // found a band lower than new_frequency so search has ended
        {
            #ifdef DBG
                DPRINTF("find_band(): Edge_Lower = "); DPRINTLN(bandmem[i].edge_lower);
            #endif
            _curr_band = bandmem[i].band_num;
            #ifdef DBG
                DPRINTF("find_band(): New Band = "); DPRINTLN(_curr_band);
            #endif
            //  Calculate frequency difference between the designated xvtr IF band's lower edge and the current VFO band's lower edge (the LO frequency).
            //if (bandmem[*_curr_band].xvtr_IF)
            //    xvtr_offset = bandmem[*_curr_band].edge_lower - bandmem[bandmem[*_curr_band].xvtr_IF].edge_lower; // if band is 144 then PLL will be set to VFOA-xvtr_offset
            //else
            //    xvtr_offset = 0;
            if (bandmem[_curr_band].bandmap_en) // filter out disabled bands
                return new_frequency;
            else
            {
                DPRINTF("find_band(): Disabled band requested "); DPRINTLN(bandmem[_curr_band].band_name);
                return 0;
            }
        }
    }
    //#ifdef DBG
       // DPRINTLNF("MAIN: Invalid Frequency Requested");
    //#endif
    return 0; // 0 means frequency was not found in the table
}