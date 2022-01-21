#include <Spectrum-Waterfall.h>

void I2C_Scanner(void);

Spectrum_RA887x spectrum_RA887x;   // initialize the Spectrum Library
#ifdef USE_RA8875
  RA8875 tft = RA8875(RA8875_CS,RA8875_RESET); //initiate the display object
#else
  RA8876_t3 tft = RA8876_t3(RA8876_CS,RA8876_RESET); //initiate the display object
  FT5206 cts = FT5206(CTP_INT); 
#endif

//
//============================================ End of Spectrum Setup Section =====================================================
//
// Audio Library setup stuff
//const float sample_rate_Hz = 11000.0f;  //43Hz /bin  5K spectrum
//const float sample_rate_Hz = 22000.0f;  //21Hz /bin 6K wide
//const float sample_rate_Hz = 44100.0f;  //43Hz /bin  12.5K spectrum
//const float sample_rate_Hz = 48000.0f;  //46Hz /bin  24K spectrum for 1024.  
//const float sample_rate_Hz = 51200.0f;  // 50Hz/bin for 1024, 200Hz/bin for 256 FFT. 20Khz span at 800 pixels 2048 FFT
const float sample_rate_Hz = 102400.0f;   // 100Hz/bin at 1024FFT, 50Hz at 2048, 40Khz span at 800 pixels and 2048FFT
//const float sample_rate_Hz = 192000.0f; // 190Hz/bin - does
//const float sample_rate_Hz = 204800.0f; // 200/bin at 1024 FFT

const int audio_block_samples = 128;          // do not change this!
AudioSettings_F32 audio_settings(sample_rate_Hz, audio_block_samples);

const int myInput = AUDIO_INPUT_LINEIN;
//const int myInput = AUDIO_INPUT_MIC;
                            
AudioInputI2S_F32       Input(audio_settings);
AudioMixer4_F32         FFT_Switch1(audio_settings);
AudioMixer4_F32         FFT_Switch2(audio_settings);
AudioFilterFIR_F32      Hilbert1(audio_settings);
AudioFilterFIR_F32      Hilbert2(audio_settings);
AudioFilterBiquad_F32   CW_Filter(audio_settings);
AudioMixer4_F32         RX_Summer(audio_settings);
AudioAnalyzePeak_F32    S_Peak(audio_settings); 
AudioAnalyzePeak_F32    Q_Peak(audio_settings); 
AudioAnalyzePeak_F32    I_Peak(audio_settings);
AudioAnalyzePeak_F32    CW_Peak(audio_settings);
AudioAnalyzeRMS_F32     CW_RMS(audio_settings);  
AudioAnalyzeFFT4096_IQ_F32  myFFT;  // choose which you like, set FFT_SIZE accordingly.
//AudioAnalyzeFFT2048_IQ_F32  myFFT;
//AudioAnalyzeFFT1024_IQ_F32  myFFT;
//AudioAnalyzeFFT256_IQ_F32   myFFT;
AudioOutputI2S_F32      Output(audio_settings);
radioNoiseBlanker_F32   NoiseBlanker(audio_settings);
AudioEffectCompressor2_F32  compressor1(audio_settings); // Audio Compressor
AudioEffectCompressor2_F32  compressor2(audio_settings); // Audio Compressor
AudioSynthWaveformSine_F32 sinewave1; // for audible alerts like touch beep confirmations

//#define TEST_SINEWAVE_SIG
#ifdef TEST_SINEWAVE_SIG
//AudioSynthSineCosine_F32   sinewave1;
//AudioSynthSineCosine_F32   sinewave2;
//AudioSynthSineCosine_F32   sinewave3;

AudioSynthWaveformSine_F32 sinewave2;
AudioSynthWaveformSine_F32 sinewave3;
AudioConnection_F32     patchCord4w(sinewave2,0,  FFT_Switch1,2);
AudioConnection_F32     patchCord4x(sinewave3,0,  FFT_Switch1,3);
AudioConnection_F32     patchCord4y(sinewave2,0,  FFT_Switch2,2);
AudioConnection_F32     patchCord4z(sinewave3,0,  FFT_Switch2,3);
AudioConnection_F32     patchCord4u(sinewave2,0,     Output,0);
AudioConnection_F32     patchCord4v(sinewave3,0,     Output,1);
#endif

// Connections for FFT Only - chooses either the input or the output to display in the spectrum plot
AudioConnection_F32     patchCord7a(Input,0,         FFT_Switch1,0);
AudioConnection_F32     patchCord7b(Input,1,         FFT_Switch2,0);
AudioConnection_F32     patchCord6a(Input,0,        FFT_Switch1,1);
AudioConnection_F32     patchCord6b(Input,1,        FFT_Switch2,1);
AudioConnection_F32     patchCord5a(FFT_Switch1,0,   myFFT,0);
AudioConnection_F32     patchCord5b(FFT_Switch2,0,   myFFT,1);

// TEST trying out new NB and AGC features  - use selected lines below as make sense
AudioConnection_F32     patchCord10a(Input,0,         NoiseBlanker,0);
AudioConnection_F32     patchCord10b(Input,1,         NoiseBlanker,1);
//AudioConnection_F32     patchCord8a(NoiseBlanker1,0, compressor1, 0);
//AudioConnection_F32     patchCord8b(NoiseBlanker2,0, compressor2, 0);
//AudioConnection_F32     patchCord9a(compressor1,0,   Hilbert1,0);
//AudioConnection_F32     patchCord9b(compressor2,0,   Hilbert2,0);
AudioConnection_F32     patchCord11a(NoiseBlanker,0,  Hilbert1,0);
AudioConnection_F32     patchCord11b(NoiseBlanker,1,  Hilbert2,0);

// Normal Audio Chain
//AudioConnection_F32     patchCord1a(Input,0,  Hilbert1,0);
//AudioConnection_F32     patchCord1b(Input,1,  Hilbert2,0);
AudioConnection_F32     patchCord2a(Hilbert1,0,      Q_Peak,0);
AudioConnection_F32     patchCord2b(Hilbert2,0,      I_Peak,0);
AudioConnection_F32     patchCord2c(Hilbert1, 0,     RX_Summer,0);
AudioConnection_F32     patchCord2d(Hilbert2, 0,     RX_Summer,1);
AudioConnection_F32     patchCord2e(sinewave1,0,     RX_Summer,2);
AudioConnection_F32     patchCord3a(RX_Summer,0,     S_Peak,0);
AudioConnection_F32     patchCord3b(RX_Summer,0,     CW_Filter,0);
AudioConnection_F32     patchCord4a(CW_Filter,0,     CW_Peak,0);
AudioConnection_F32     patchCord4b(CW_Filter,0,     CW_RMS,0);
AudioConnection_F32     patchCord4c(CW_Filter,0,     Output,0);
AudioConnection_F32     patchCord4d(CW_Filter,0,     Output,1);
//AudioConnection_F32     patchCord4c(Input,0,     Output,0);
//AudioConnection_F32     patchCord4d(Input,1,     Output,1);

AudioControlSGTL5000    codec1;

#include <Metro.h>
// Most of our timers are here.  Spectrum waterfall is in the spectrum settings section of that file
Metro touch         = Metro(50);    // used to check for touch events
Metro meter         = Metro(400);   // used to update the meters
Metro popup_timer   = Metro(500);   // used to check for popup screen request
Metro touchBeep_timer = Metro(80); // Feedback beep for button touches

// used for spectrum object
//#define FFT_SIZE                  4096            // Need a constant for array size declarion so manually set this value here.  Could try a macro later
int16_t         fft_bins            = FFT_SIZE;     // Number of FFT bins which is FFT_SIZE/2 for real version or FFT_SIZE for iq version
float           fft_bin_size        = sample_rate_Hz/(FFT_SIZE*2);   // Size of FFT bin in HZ.  From sample_rate_Hz/FFT_SIZE for iq
int16_t         spectrum_preset     = 0;                    // Specify the default layout option for spectrum window placement and size.
int16_t         FFT_Source          = 0;            // Used to switch the FFT input source around
extern Metro    spectrum_waterfall_update;          // Timer used for controlling the Spectrum module update rate.
extern struct   Spectrum_Parms Sp_Parms_Def[];

COLD void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println(F("Initializing SDR_RA887x Program"));
    Serial.println(F("**** Running I2C Scanner ****"));

    // ---------------- Setup our basic display and comms ---------------------------
    Wire.begin();
    Wire.setClock(100000UL); // Keep at 100K I2C bus transfer data rate for I2C Encoders to work right
    I2C_Scanner();
    //MF_client = user_settings[user_Profile].default_MF_client;
    //MF_default_is_active = true;
    //MeterInUse = false;    

    #ifdef USE_RA8875
        Serial.println(F("Initializing RA8875 Display"));
        tft.begin(RA8875_800x480);
        tft.setRotation(SCREEN_ROTATION); // 0 is normal, 1 is 90, 2 is 180, 3 is 270 degrees
    #else 
        Serial.println(F("Initializing RA8876 Display"));   
        tft.begin(30000000UL);
        cts.begin();
        cts.setTouchLimit(MAXTOUCHLIMIT);
        tft.touchEnable(false);   // Ensure the resitive ocntroller, if any is off
        tft.displayImageStartAddress(PAGE1_START_ADDR); 
        tft.displayImageWidth(SCREEN_WIDTH);
        tft.displayWindowStartXY(0,0);
        // specify the page 2 for the current canvas
        tft.canvasImageStartAddress(PAGE2_START_ADDR);
        // specify the page 1 for the current canvas
        tft.canvasImageStartAddress(PAGE1_START_ADDR);
        tft.canvasImageWidth(SCREEN_WIDTH);
        //tft.activeWindowXY(0,0);
        //tft.activeWindowWH(SCREEN_WIDTH,SCREEN_HEIGHT);
        spectrum_RA887x.setActiveWindow_default();
        tft.graphicMode(true);
        tft.clearActiveScreen();
        tft.selectScreen(0);  // Select screen page 0
        tft.fillScreen(BLACK);
        tft.setBackGroundColor(BLACK);
        tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
        tft.backlight(true);
        tft.displayOn(true);
        tft.setRotation(SCREEN_ROTATION); // 0 is normal, 1 is 90, 2 is 180, 3 is 270 degrees.  
                        // RA8876 touch controller is upside down compared to the RA8875 so correcting for it there.
    #endif
    
    #if defined(USE_FT5206_TOUCH)
        tft.useCapINT(RA8875_INT);
        tft.setTouchLimit(MAXTOUCHLIMIT);
        tft.enableCapISR(true);
        tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
    #else
        #ifdef USE_RA8875
            //tft.print("you should open RA8875UserSettings.h file and uncomment USE_FT5206_TOUCH!");
        #endif  // USE_RA8875
    #endif // USE_FT5206_TOUCH
    
    spectrum_RA887x.initSpectrum(spectrum_preset); // Call before initDisplay() to put screen into Layer 1 mode before any other text is drawn!

    //--------------------------   Setup our Audio System -------------------------------------

    AudioMemory_F32(100, audio_settings);
    codec1.enable(); // MUST be before inputSelect()
    delay(5);
    codec1.dacVolumeRampDisable(); // Turn off the sound for now
    codec1.inputSelect(myInput);
    //RFgain(0);
    codec1.lineOutLevel(user_settings[user_Profile].lineOut_Vol_last); // range 13 to 31.  13 => 3.16Vp-p, 31=> 1.16Vp-p
    codec1.autoVolumeControl(2, 0, 0, -36.0, 12, 6);                   // add a compressor limiter
    //codec1.autoVolumeControl( 0-2, 0-3, 0-1, 0-96, 3, 3);
    //autoVolumeControl(maxGain, response, hardLimit, threshold, attack, decay);
    codec1.autoVolumeEnable(); // let the volume control itself..... poor mans agc
    //codec1.autoVolumeDisable();// Or don't let the volume control itself
    codec1.muteLineout(); //mute the audio output until we finish thumping relays 
    codec1.adcHighPassFilterDisable();
    codec1.dacVolume(0); // set the "dac" volume (extra control)

    // Select our sources for the FFT.  mode.h will change this so CW uses the output (for now as an experiment)
    AudioNoInterrupts();
    FFT_Switch1.gain(0, 1.0f); //  1 is Input source before filtering, 0 is off,
    FFT_Switch1.gain(1, 0.0f); //  1  is CW Filtered (output), 0 is off
    FFT_Switch2.gain(0, 1.0f); //  1 is Input source before filtering, 0 is off,
    FFT_Switch2.gain(1, 0.0f); //  1  is CW Filtered (output), 0 is off
    #ifdef TEST_SINEWAVE_SIG
    FFT_Switch1.gain(2, 1.0f); //  1  Sinewave2 to FFT for test cal, 0 is off
    FFT_Switch1.gain(3, 1.0f); //  1  Sinewave3 to FFT for test cal, 0 is off
    FFT_Switch2.gain(2, 1.0f); //  1  Sinewave2 to FFT for test cal, 0 is off
    FFT_Switch2.gain(3, 1.0f); //  1  Sinewave3 to FFT for test cal, 0 is off
    #else
    FFT_Switch1.gain(2, 0.0f); //  1  Sinewave2 to FFT for test cal, 0 is off
    FFT_Switch1.gain(3, 0.0f); //  1  Sinewave3 to FFT for test cal, 0 is off
    FFT_Switch2.gain(2, 0.0f); //  1  Sinewave2 to FFT for test cal, 0 is off
    FFT_Switch2.gain(3, 0.0f); //  1  Sinewave3 to FFT for test cal, 0 is off
    #endif
    AudioInterrupts();

    /*
    //Shows how to use the switch object.  Not using right now but have several ideas for later so saving it here.
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
    // Set up an alert tone for feedback   Can also blink KED knobs if used.

#ifdef TEST_SINEWAVE_SIG
    // Create a synthetic sine wave, for testing
    // To use this, edit the connections above
    // # sources to test edges and middle of BW
    float sinewave_vol = 0.005;
    sinewave2.amplitude(sinewave_vol);
    sinewave2.frequency(100.000); //
    sinewave3.amplitude(sinewave_vol);
    sinewave3.frequency(400.000); //
#endif

    // TODO: Move this to set mode and/or bandwidth section when ready.  messes up initial USB/or LSB/CW alignments until one hits the mode button.
    RX_Summer.gain(0, -3.0); // Leave at 1.0
    RX_Summer.gain(1, 3.0);  // -1 for LSB out
    // Choose our output type.  Can do dB, RMS or power
    myFFT.setOutputType(FFT_DBFS); // FFT_RMS or FFT_POWER or FFT_DBFS
    // Uncomment one these to try other window functions
    //  myFFT.windowFunction(NULL);
    //  myFFT.windowFunction(AudioWindowBartlett1024);
    //  myFFT.windowFunction(AudioWindowFlattop1024);
    myFFT.windowFunction(AudioWindowHanning1024);
    myFFT.setNAverage(3); // experiment with this value.  Too much causes a large time penalty
    // -------------------- Setup our radio settings and UI layout --------------------------------

    //curr_band = user_settings[user_Profile].last_band;       // get last band used from user profile.
    //user_settings[user_Profile].sp_preset = spectrum_preset; // uncomment this line to update user profile layout choice
    //spectrum_preset = user_settings[user_Profile].sp_preset;
    spectrum_preset = 0;
    //==================================== Frequency Set ==========================================
    #ifdef PANADAPTER
    VFOA = PANADAPTER_LO;
    VFOB = PANADAPTER_LO;
    #else
    VFOA = 7074000;
    VFOB = 14074000;
    #endif
    // Assignments for our encoder knobs, if any
    //initVfo(); // initialize the si5351 vfo
    //changeBands(0);   // Sets the VFOs to last used frequencies, sets preselector, active VFO, other last-used settings per band.
    //displayRefresh(); // calls the whole group of displayxxx();  Needed to refresh after other windows moving.
    spectrum_RA887x.Spectrum_Parm_Generator(spectrum_preset, spectrum_preset); // use this to generate new set of params for the current window size values. 
                                                              // 1st arg is target, 2nd arg is current value
                                                              // calling generator before drawSpectrum() will create a new set of values based on the globals
                                                              // Generator only reads the global values, it does not change them or the database, just prints the new params
    spectrum_RA887x.drawSpectrumFrame(user_settings[user_Profile].sp_preset); // Call after initSpectrum() to draw the spectrum object.  Arg is 0 PRESETS to load a preset record
                                                              // DrawSpectrum does not read the globals but does update them to match the current preset.
                                                              // Therefore always call the generator before drawSpectrum() to create a new set of params you can cut anmd paste.
                                                              // Generator never modifies the globals so never affects the layout itself.
                                                              // Print out our starting frequency for testing
    //sp.drawSpectrumFrame(6);   // for 2nd window
    Serial.print(F("\nInitial Dial Frequency is "));
    Serial.print(formatVFO(VFOA));
    Serial.println(F("MHz"));
   
    // ---------------------------- Setup speaker on or off and unmute outputs --------------------------------
    if (1)
    {
        codec1.volume(1.0); // Set to full scale.  RampVolume will then scale it up or down 0-100, scaled down to 0.0 to 1.0
        // 0.7 seemed optimal for K7MDL with QRP_Labs RX board with 15 on line input and 20 on line output
        codec1.unmuteHeadphone();
        codec1.unmuteLineout(); //unmute the audio output
        //user_settings[user_Profile].mute = OFF;
        //displayMute();
        //AFgain(0);   // 0 is no change, set to stored last value.  range -100 to +100 percent change of full scale.
        //RampVolume(user_settings[user_Profile].spkr_Vol_last/100, 1); //0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    }
    else
    {
        codec1.muteHeadphone();
        //user_settings[user_Profile].mute = ON;
        //displayMute();
    }

    //changeBands(0);     // Sets the VFOs to last used frequencies, sets preselector, active VFO, other last-used settings per band.
                        // Call changeBands() here after volume to get proper startup volume

    //------------------Finish the setup by printing the help menu to the serial connections--------------------
    printHelp();
    InternalTemperature.begin(TEMPERATURE_NO_ADC_SETTING_CHANGES);
}

static uint32_t delta = 0;
//
// __________________________________________ Main Program Loop  _____________________________________
//
void loop()
{
    static int32_t newFreq = 0;
    static uint32_t time_old = 0;
 
    // Update spectrum and waterfall based on timer
    if (spectrum_waterfall_update.check() == 1) // The update rate is set in drawSpectrumFrame() with spect_wf_rate from table
    {
        if (!user_settings[user_Profile].notch)  // TEST:  added to test CPU impact
            spectrum_RA887x.spectrum_update(spectrum_preset, 1, VFOA, VFOB); // valid numbers are 0 through PRESETS to index the record of predefined window layouts
            // spectrum_RA887x.spectrum_update(6, 1, VFOA, VFOB);  // for 2nd window
    }
    
    // Time stamp our program loop time for performance measurement
    uint32_t time_n = millis() - time_old;

    if (time_n > delta)
    {
        delta = time_n;
        Serial.print(F("Tms="));
        Serial.println(delta);
    }
    time_old = millis();

    // Check for touch actions
    if (touch.check() == 1)
    {
        Touch(); // touch points and gestures
    }
    
    // The timer and flag are set by the rogerBeep() function
    if (touchBeep_flag && touchBeep_timer.check() == 1)   
    {
        touchBeep(false);    
    }

    //check to see whether to print the CPU and Memory Usage
    if (enable_printCPUandMemory)
        printCPUandMemory(millis(), 3000); //print every 3000 msec
}

//
//  Scans for any I2C connected devices and reports them to the serial terminal.  Usually done early in startup.
//
COLD void I2C_Scanner(void)
{
  byte error, address; //variable for error and I2C address
  int nDevices;

  // uncomment these to use alternate pins
  //WIRE.setSCL(37);
  //WIRE.setSDA(36);
  //WIRE.begin();
  
  Serial.println(F("Scanning..."));

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print(F("I2C device found at address 0x"));
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.print("  (");
      printKnownChips(address);
      Serial.println(")");
      //Serial.println("  !");
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print(F("Unknown error at address 0x"));
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println(F("No I2C devices found\n"));
  else
    Serial.println(F("done\n"));

  //delay(500); // wait 5 seconds for the next I2C scan
}

// prints a list of known i2C devices that match a discovered address
void printKnownChips(byte address)
{
  // Is this list missing part numbers for chips you use?
  // Please suggest additions here:
  // https://github.com/PaulStoffregen/Wire/issues/new
  switch (address) {
    case 0x00: Serial.print(F("AS3935")); break;
    case 0x01: Serial.print(F("AS3935")); break;
    case 0x02: Serial.print(F("AS3935")); break;
    case 0x03: Serial.print(F("AS3935")); break;
    case 0x0A: Serial.print(F("SGTL5000")); break; // MCLK required
    case 0x0B: Serial.print(F("SMBusBattery?")); break;
    case 0x0C: Serial.print(F("AK8963")); break;
    case 0x10: Serial.print(F("CS4272")); break;
    case 0x11: Serial.print(F("Si4713")); break;
    case 0x13: Serial.print(F("VCNL4000,AK4558")); break;
    case 0x18: Serial.print(F("LIS331DLH")); break;
    case 0x19: Serial.print(F("LSM303,LIS331DLH")); break;
    case 0x1A: Serial.print(F("WM8731")); break;
    case 0x1C: Serial.print(F("LIS3MDL")); break;
    case 0x1D: Serial.print(F("LSM303D,LSM9DS0,ADXL345,MMA7455L,LSM9DS1,LIS3DSH")); break;
    case 0x1E: Serial.print(F("LSM303D,HMC5883L,FXOS8700,LIS3DSH")); break;
    case 0x20: Serial.print(F("MCP23017,MCP23008,PCF8574,FXAS21002,SoilMoisture")); break;
    case 0x21: Serial.print(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x22: Serial.print(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x23: Serial.print(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x24: Serial.print(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x25: Serial.print(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x26: Serial.print(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x27: Serial.print(F("MCP23017,MCP23008,PCF8574,LCD16x2,DigoleDisplay")); break;
    case 0x28: Serial.print(F("BNO055,EM7180,CAP1188")); break;
    case 0x29: Serial.print(F("TSL2561,VL6180,TSL2561,TSL2591,BNO055,CAP1188")); break;
    case 0x2A: Serial.print(F("SGTL5000,CAP1188")); break;
    case 0x2B: Serial.print(F("CAP1188")); break;
    case 0x2C: Serial.print(F("MCP44XX ePot")); break;
    case 0x2D: Serial.print(F("MCP44XX ePot")); break;
    case 0x2E: Serial.print(F("MCP44XX ePot")); break;
    case 0x2F: Serial.print(F("MCP44XX ePot")); break;
    case 0x33: Serial.print(F("MAX11614,MAX11615")); break;
    case 0x34: Serial.print(F("MAX11612,MAX11613")); break;
    case 0x35: Serial.print(F("MAX11616,MAX11617")); break;
    case 0x38: Serial.print(F("RA8875,FT6206")); break;
    case 0x39: Serial.print(F("TSL2561, APDS9960")); break;
    case 0x3C: Serial.print(F("SSD1306,DigisparkOLED")); break;
    case 0x3D: Serial.print(F("SSD1306")); break;
    case 0x40: Serial.print(F("PCA9685,Si7021")); break;
    case 0x41: Serial.print(F("STMPE610,PCA9685")); break;
    case 0x42: Serial.print(F("PCA9685")); break;
    case 0x43: Serial.print(F("PCA9685")); break;
    case 0x44: Serial.print(F("PCA9685, SHT3X")); break;
    case 0x45: Serial.print(F("PCA9685, SHT3X")); break;
    case 0x46: Serial.print(F("PCA9685")); break;
    case 0x47: Serial.print(F("PCA9685")); break;
    case 0x48: Serial.print(F("ADS1115,PN532,TMP102,PCF8591")); break;
    case 0x49: Serial.print(F("ADS1115,TSL2561,PCF8591")); break;
    case 0x4A: Serial.print(F("ADS1115")); break;
    case 0x4B: Serial.print(F("ADS1115,TMP102")); break;
    case 0x50: Serial.print(F("EEPROM")); break;
    case 0x51: Serial.print(F("EEPROM")); break;
    case 0x52: Serial.print(F("Nunchuk,EEPROM")); break;
    case 0x53: Serial.print(F("ADXL345,EEPROM")); break;
    case 0x54: Serial.print(F("EEPROM")); break;
    case 0x55: Serial.print(F("EEPROM")); break;
    case 0x56: Serial.print(F("EEPROM")); break;
    case 0x57: Serial.print(F("EEPROM")); break;
    case 0x58: Serial.print(F("TPA2016,MAX21100")); break;
    case 0x5A: Serial.print(F("MPR121")); break;
    case 0x60: Serial.print(F("MPL3115,MCP4725,MCP4728,TEA5767,Si5351")); break;
    case 0x61: Serial.print(F("MCP4725,AtlasEzoDO,DuPPaEncoder")); break;
    case 0x62: Serial.print(F("LidarLite,MCP4725,AtlasEzoORP,DuPPaEncoder")); break;
    case 0x63: Serial.print(F("MCP4725,AtlasEzoPH,DuPPaEncoder")); break;
    case 0x64: Serial.print(F("AtlasEzoEC,DuPPaEncoder")); break;
    case 0x66: Serial.print(F("AtlasEzoRTD,DuPPaEncoder")); break;
    case 0x67: Serial.print(F("DuPPaEncoder")); break;
    case 0x68: Serial.print(F("DS1307,DS3231,MPU6050,MPU9050,MPU9250,ITG3200,ITG3701,LSM9DS0,L3G4200D,DuPPaEncoder")); break;
    case 0x69: Serial.print(F("MPU6050,MPU9050,MPU9250,ITG3701,L3G4200D")); break;
    case 0x6A: Serial.print(F("LSM9DS1")); break;
    case 0x6B: Serial.print(F("LSM9DS0")); break;
    case 0x70: Serial.print(F("HT16K33")); break;
    case 0x71: Serial.print(F("SFE7SEG,HT16K33")); break;
    case 0x72: Serial.print(F("HT16K33")); break;
    case 0x73: Serial.print(F("HT16K33")); break;
    case 0x76: Serial.print(F("MS5607,MS5611,MS5637,BMP280")); break;
    case 0x77: Serial.print(F("BMP085,BMA180,BMP280,MS5611")); break;
    default: Serial.print(F("unknown chip"));
  }
}

// Turns on or off a tone injected in the RX_summer block.  
// Function that calls for a rogerBeep sets a global flag.
// The main loop starts a timer for a short beep an calls this to turn the tone on or off.
COLD void touchBeep(bool enable)
{
    if (enable)
    {
        touchBeep_flag = true;
        touchBeep_timer.reset();
        sinewave1.amplitude(user_settings[user_Profile].rogerBeep_Vol);
        sinewave1.frequency((float) user_settings[user_Profile].pitch); //    Alert tones
    }
    else 
    {
        //if (rogerBeep_timer.check() == 1)   // make sure another event does not cut it off early
        //{ 
            touchBeep_flag = false;
            sinewave1.amplitude(0.0);
        //}
    }
}
