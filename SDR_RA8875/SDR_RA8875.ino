//  SDR_RA8875.INO
//
//  Main PRogram File
//
//  Spectrum, Display, full F32 library conversion completed 3/2021. Uses FFTXXXX_IQ_F32 FFT I and Q version files
//      XXXX can be the 256, 1024, 2048 or 4096 versions.
//  Spectrum uses the raw FFT output and is not calibrated.
//
//  Test tones are enabled in spectrum only, not in audio path.
//
// _______________________________________ Setup_____________________________________________
//

#include "SDR_RA8875.h"

void setup() 
{
    Wire.setClock(400000);  // Increase i2C bus transfer data rate from default of 100KHz 
    tft.begin(RA8875_800x480);
    tft.setRotation(0);
    #if defined(USE_FT5206_TOUCH)
    tft.useCapINT(RA8875_INT);
    tft.setTouchLimit(MAXTOUCHLIMIT);
    tft.enableCapISR(true);
    tft.setTextColor(RA8875_WHITE,RA8875_BLACK);
    #else
    tft.print("you should open RA8875UserSettings.h file and uncomment USE_FT5206_TOUCH!");
    #endif    
    initSpectrum_RA8875();    // Call before initDisplay() to put screen into Layer 1 mode before any other text is drawn!
    curr_band = user_settings[user_Profile].last_band;  // get last band used from user profile. 
    user_settings[user_Profile].sp_preset = spectrum_preset;  // uncomment this line to update user profile layout choice
    spectrum_preset = user_settings[user_Profile].sp_preset;
    //
    //================================================ Frequency Set =============================================================
    VFOA = bandmem[curr_band].vfo_A_last;    //I used 7850000  frequency CHU  Time Signal Canada
	VFOB = bandmem[curr_band].vfo_B_last;
    //================================================ Frequency Set =============================================================
    //
    initVfo();        // initialize the si5351 vfo
	selectFrequency(0);
    selectMode(0);
    selectAgc(bandmem[curr_band].agc_mode);
    displayAgc();
    displayRefresh();  // calls the whole group of displayxxx();  Needed to refresh after other windows moving.

    //AudioMemory(16);   // moved to 32 bit so no longer needed hopefully
    AudioMemory_F32(80, audio_settings);

    //TODO: Many of these need to be called in other places also such as when changing bands or AGC to mute and unmute, during TX for another example  
    codec1.enable();  // MUST be before inputSelect()
    delay(5);
    codec1.dacVolumeRampDisable();    // Turn off the sound for now
    codec1.inputSelect(myInput);    
    codec1.lineInLevel(user_settings[user_Profile].lineIn_Vol_last);     // range 0 to 15.  0 => 3.12Vp-p, 15 => 0.24Vp-p sensitivity
    codec1.lineOutLevel(user_settings[user_Profile].lineOut_Vol_last);    // range 13 to 31.  13 => 3.16Vp-p, 31=> 1.16Vp-p
    codec1.autoVolumeControl(2,0,0,-36.0,12,6); // add a compressor limiter
    //codec1.autoVolumeControl( 0-2, 0-3, 0-1, 0-96, 3, 3);
    //autoVolumeControl(maxGain, response, hardLimit, threshold, attack, decay);
    codec1.autoVolumeEnable();// let the volume control itself..... poor mans agc
    //codec1.autoVolumeDisable();// Or don't let the volume control itself
    codec1.unmuteLineout(); //unmute the audio output
    codec1.adcHighPassFilterDisable();
    codec1.dacVolume(0);    // set the "dac" volume (extra control)
    // Now turn on the sound    
	if (user_settings[user_Profile].spkr_en == ON)
    {   
		codec1.volume(user_settings[user_Profile].spkr_Vol_last);   // 0.7 seemed optimal for K7MDL with QRP_Labs RX board with 15 on line input and 20 on line output
    	codec1.unmuteHeadphone();
		user_settings[user_Profile].mute = OFF;
      	displayMute();
		RampVolume(1.0, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp" 
    }
    else 
    {
        codec1.muteHeadphone();
		user_settings[user_Profile].mute = ON;
		displayMute();
    }
    
    // Select our sources for the FFT.  mode.h will change this so CW uses the output (for now as an experiment)
    AudioNoInterrupts();
    FFT_Switch1.gain(0,1.0f);   //  1 is Input source before filtering, 0 is off,
    FFT_Switch1.gain(1,0.0f);   //  1  is CW Filtered (output), 0 is off
    FFT_Switch1.gain(2,0.0f);    //  1  Sinewave2 to FFT for test cal, 0 is off
    FFT_Switch1.gain(3,0.0f);    //  1  Sinewave3 to FFT for test cal, 0 is off
    FFT_Switch2.gain(0,1.0f);   //  1 is Input source before filtering, 0 is off,
    FFT_Switch2.gain(1,0.0f);   //  1  is CW Filtered (output), 0 is off
    FFT_Switch2.gain(2,0.0f);    //  1  Sinewave2 to FFT for test cal, 0 is off
    FFT_Switch2.gain(3,0.0f);    //  1  Sinewave3 to FFT for test cal, 0 is off
    AudioInterrupts();
    /*   Shows how to use the switch object.  Not using right now but have several ideas for later so saving it here.
    // The switch is single pole 4 position, numbered (0, 3)  0=FFT before filters, 1 = FFT after filters
    if(mndx = 1 || mndx==2)
    { 
      FFT_Switch1.setChanne1(1); Serial.println("Unfiltered FFT"); }
      FFT_Switch2.setChanne1(0); Serial.println("Unfiltered FFT"); }
    )  
    else if(mndx==0) // Input is on Switch 1, CW is on Switch 2
    { 
      FFT_Switch1.setChannel(0); Serial.println("CW Filtered FFT"); 
      FFT_Switch2.setChannel(1); Serial.println("CW Filtered FFT"); 
    }
    */
    #ifdef TEST_SINEWAVE_SIG
    // Create a synthetic sine wave, for testing
    // To use this, edit the connections above
    // # sources to test edges and middle of BW
    float sinewave_vol = 0.03;
    sinewave1.amplitude(sinewave_vol);
    sinewave1.frequency(4000.000);   //     
    sinewave2.amplitude(sinewave_vol);
    sinewave2.frequency(600.000);   //     
    sinewave3.amplitude(sinewave_vol);
    sinewave3.frequency(12800.000);   //
    #endif
   
    // TODO: Move this to set mode and/or bandwidth section when ready.  messes up initial USB/or LSB/CW alignments until one hits the mode button.
    RX_Summer.gain(0,-3.0);   // Leave at 1.0
    RX_Summer.gain(1,3.0);  // -1 for LSB out
 
    selectBandwidth(bandmem[curr_band].filter);
    selectMode(0);  // set mode of thge Active VFO using last recorded value (0 = no change) 

    // Choose our output type.  Can do dB, RMS or power
    myFFT.setOutputType(FFT_DBFS);  // FFT_RMS or FFT_POWER or FFT_DBFS
    
    // Uncomment one these to try other window functions
    //  myFFT.windowFunction(NULL);
    //  myFFT.windowFunction(AudioWindowBartlett1024);
    //  myFFT.windowFunction(AudioWindowFlattop1024);
    myFFT.windowFunction(AudioWindowHanning1024);

    Spectrum_Parm_Generator(spectrum_preset); // use this to generate new set of params for the current window size values.
            // calling generator before drawSpectrum() will create a new set of values based on the globals
            // Generator only reads the global values, it does not change them or the database, just prints the new params             
    drawSpectrumFrame(user_settings[user_Profile].sp_preset);  // Call after initSpectrum() to draw the spectrum object.  Arg is 0 PRESETS to load a preset record
            // DrawSpectrum does not read the globals but does update them to match the current preset.
            // Therefore always call the generator before drawSpectrum() to create a new set of params you can cut anmd paste.
            // Generator never modifies the globals so never affects the layout itself.
     // Print out our starting frequency for testing
    Serial.print("\nInitial Dial Frequency is "); 
    Serial.print(VFOA); 
    Serial.println("MHz");

    //finish the setup by printing the help menu to the serial connections
    printHelp();
    InternalTemperature.begin(TEMPERATURE_NO_ADC_SETTING_CHANGES);
    
    #ifdef ENET
    if (user_settings[user_Profile].enet_enabled) 
    { 
        enet_start(); 
        if (!enet_ready)
        {
            enet_start_fail_time = millis();      // set timer for 10 minute self recovery in main loop
            Serial.println(">Ethernet System Startup Failed, setting retry timer (10 minutes)");
        }
        Serial.println(">Ethernet System Startup");
    }
    #endif
}

static uint32_t delta = 0;
//
// __________________________________________ Main Program Loop  _____________________________________
//
void loop() 
{
    static int32_t newFreq = 0;
    static uint32_t time_old = 0;;

    // Update spectrum and waterfall based on timer

    if (spectrum.check() == 1)
    {   
        if (!popup)   // do not draw in the screen space while the pop up has the screen focus.
                      // a popup must call drawSpectrumFrame when it is done and clear this flag.
            spectrum_update(spectrum_preset);   // valid numbers are 0 through PRESETS to index the record of predefined window layouts 
    }
    uint32_t time_n = millis()-time_old;
    
    if (time_n > delta) {
       delta = time_n;
       Serial.print("Tms="); Serial.println(delta);
    }
    time_old = millis();

    if (touch.check()==1)
    {
        Touch(); // touch points and gestures        
    }
 
    if (tuner.check() == 1 && newFreq < enc_ppr_response)  // dump counts accumulated over time but < minimum for a step to count.
    {
        Position.readAndReset();
        newFreq = 0;
    }

    newFreq += Position.read();   // faster to poll for change since last read
    // accumulate conts until we have enough to act on for scaling factor to work right.
    if(newFreq != 0 && abs(newFreq) > enc_ppr_response)  // newFreq is a positive or negative number of counts since last read.
    {
        newFreq /= enc_ppr_response;   // adjust for high vs low PPR encoders.  600ppr is too fast!
        selectFrequency(newFreq);
        Position.readAndReset();   // zero out counter fo rnext read.
        newFreq = 0;
    }

    if (meter.check()==1)  // update our meters
    {
        Peak();
        // Code_Peak();
        // Quad_Check();
    }

    if ( popup_timer.check() == 1 && popup) // stop spectrum updates, clear the screen and post up a keyboard or something
    {
          // Service popup window
    }

    //respond to Serial commands
    while (Serial.available())
    {
        respondToByte((char)Serial.read());
    }
    
    //check to see whether to print the CPU and Memory Usage
    if (enable_printCPUandMemory) printCPUandMemory(millis(), 3000); //print every 3000 msec

    #ifdef ENET     // remove this code if no ethernet usage intended
    if (user_settings[user_Profile].enet_enabled)  // only process enet if enabled.
    {
        if (!enet_ready)
            if ((millis() - enet_start_fail_time) >  600000)  // check every 10 minutes (600K ms) and attempt a restart.
                enet_start();
        enet_read();        // Check for Control head commands
        if (rx_count!=0)
          {}//get_remote_cmd();       // scan buffer for command strings
        
        if (NTP_updateTx.check() == 1) 
        {
            sendNTPpacket(timeServer);      // send an NTP packet to a time server
            NTP_updateRx.interval(1000);    // Start a timer to check RX reply         
        }
        if (NTP_updateRx.check() == 1)          // Time to check for a reply
        {
            RX_NTP_time();                  // Get our reply
            NTP_updateRx.interval(65000);   // set it long until we need it again later
            Ethernet.maintain();            // keep our connection fresh
        }
    }
    #endif
}

//
// _______________________________________ Volume Ramp __________________
//
// Ramps the volume down to specified level 0 to 1.0 range using 1 of 3 types.  It remembers the original volume level so 
// you are reducing it by a factor then raisinmg back up a factor toward the orignal volume setting.
// Range is 1.0 for full original and 0 for off.
void RampVolume(float vol, int16_t rampType)
{
    const char *rampName[] = {
    	"No Ramp (instant)",  // loud pop due to instant change
    	"Normal Ramp",        // graceful transition between volume levels
      	"Linear Ramp"         // slight click/chirp
    };
    
    Serial.println(rampName[rampType]);

    // configure which type of volume transition to use
    if (rampType == 0) {
      	codec1.dacVolumeRampDisable();
    } else if (rampType == 1) {
      	codec1.dacVolumeRamp();
    } else {
      	codec1.dacVolumeRampLinear();
    }

    // set the dacVolume.  The actual change may take place over time, if ramping
    // this is a logarithmic volume,
    // that is, the range 0.0 to 1.0 gets converted to -90dB to 0dB in 0.5dB steps
    codec1.dacVolume(vol);
}
//
// _______________________________________ Print CPU Stats, Adjsut Dial Freq ____________________________
//
//This routine prints the current and maximum CPU usage and the current usage of the AudioMemory that has been allocated
void printCPUandMemory(unsigned long curTime_millis, unsigned long updatePeriod_millis) 
{
    //static unsigned long updatePeriod_millis = 3000; //how many milliseconds between updating gain reading?
    static unsigned long lastUpdate_millis = 0;
    
    //has enough time passed to update everything?
    if (curTime_millis < lastUpdate_millis) lastUpdate_millis = 0; //handle wrap-around of the clock
    if ((curTime_millis - lastUpdate_millis) > updatePeriod_millis) 
    { //is it time to update the user interface?
        Serial.print("CPU Cur/Peak: ");
        Serial.print(audio_settings.processorUsage());
        Serial.print("%/");
        Serial.print(audio_settings.processorUsageMax());
        Serial.println("%");
        Serial.print("CPU Temperature:");
        Serial.print(InternalTemperature.readTemperatureF(), 1);
        Serial.print("F ");
        Serial.print(InternalTemperature.readTemperatureC(), 1);
        Serial.println("C");
        Serial.print(" Audio MEM Float32 Cur/Peak: ");
        Serial.print(AudioMemoryUsage_F32());
        Serial.print("/");
        Serial.println(AudioMemoryUsageMax_F32());
        
        lastUpdate_millis = curTime_millis; //we will use this value the next time around.
        delta = 0;
    }
}
//
// _______________________________________ Console Parser ____________________________________
//
//switch yard to determine the desired action
void respondToByte(char c) 
{
    char s[2];
    s[0] = c;  s[1] = 0;
    if( !isalpha((int)c) && c!='?') return;
    switch (c) 
    {
        case 'h': case '?':
          printHelp();
          break;
        case 'C': case 'c':
          Serial.println("Toggle printing of memory and CPU usage.");
          togglePrintMemoryAndCPU();
          break;     
        default:
          Serial.print("You typed "); Serial.print(s);
          Serial.println(".  What command?");
    }
}
//
// _______________________________________ Print Help Menu ____________________________________
//
void printHelp(void) 
{
    Serial.println();
    Serial.println("Help: Available Commands:");
    Serial.println("   h: Print this help");
    Serial.println("   C: Toggle printing of CPU and Memory usage");
}
//
// ---------------------------------  multiKnob() -----------------------------------------------
//
//  Handles a detented incremental encoder for any calling function returning the count, if any, 
//          positive or negative until a reset is requested to 0.
//
//  Input:  uint8_t clear.  If clear == 1 then reset the encoder to ready it for a next read.
//
//  Usage:  A consumer function will call this with clear flag set at the start of use. 
//          It will poll this function for counts acting on any in any way it needs to.
//          It will celar the count to look for next action.
//          If a clear is not performed then the consumer function must deal with figuring out how
//          the value has changed and what to do with it.
//          The value returned will be a positive or negative value with some count (usally each step si 4 but not always)
//
int32_t multiKnob(uint8_t clear)
{
    static uint32_t mf_count = 0;
    
    if (clear)
    {
        Multi.readAndReset();   // toss results, zero the encoder
        mf_count = 0;
    }
    else
        mf_count = Multi.read();  // read and reset the Multifunction knob.  Apply results to any in-focus function, if any.
    return mf_count;
}
