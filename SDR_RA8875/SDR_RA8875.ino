#ifndef _SDR_RA8875_
#define _SDR_RA8875_
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
#include "SDR_Data.h"
// Now pickup build time options from RadioConfig.h
#include "RadioConfig.h"        // Majority of declarations here to drive teh #ifdefs that follow

#ifdef SV1AFN_BPF               // This turns on support for the Bandpass Filter board and relays for LNA and Attenuation
 #include <SVN1AFN_BandpassFilters.h> // Modified and redistributed in this build source folder
 SVN1AFN_BandpassFilters bpf;  // The SV1AFN Preselector module supporing all HF bands and a preamp and Attenuator. 
                                // For 60M coverage requires and updated libary file set.
#endif // SV1AFN_BPF

// Choose your actual pin assignments for any you may have.
Encoder VFO(VFO_ENC_PIN_A, VFO_ENC_PIN_B); //using pins 4 and 5 on teensy 4.0 for VFO A/B tuning encoder 

#ifdef I2C_ENCODERS              // This turns on support for DuPPa.net I2C encoder with RGB LED integrated. 
// This is a basic example for using the I2C Encoder V2
//  The counter is set to work between +10 to -10, at every encoder click the counter value is printed on the terminal.
//  It's also printet when the push button is released.
//  When the encoder is turned the led turn green
//  When the encoder reach the max or min the led turn red
//  When the encoder is pushed, the led turn blue

//  Connections with Teensy 4.1:
//  - -> GND
//  + -> 3.3V
//  SDA -> 18
//  SCL -> 19
//  INT -> 29
  #include "SDR_I2C_Encoder.h"              // See RadioConfig.h for more config including assigning an INT pin.                                          // Hardware verson 2.1, Arduino library version 1.40.
  //Class initialization with the I2C addresses
  extern i2cEncoderLibV2 MF_ENC; // Address 0x61 only - Jumpers A0, A5 and A6 are soldered.//
  extern i2cEncoderLibV2 ENC2; // Address 0x62 only - Jumpers A1, A5 and A6 are soldered.//  
  extern i2cEncoderLibV2 ENC3; // Address 0x62 only - Jumpers A1, A5 and A6 are soldered.// 
#else 
  Encoder Multi(MF_ENC_PIN_A, MF_ENC_PIN_B);  // Multi Function Encoder pins assignments usnig GPIO pinss
  //Encoder AF(29,28);   // AF gain control - not used yet
  //Encoder RF(33,34);   // RF gain control - not used yet 
#endif // I2C_ENCODER

#ifdef OCXO_10MHZ               // This turns on a group of features feature that are hardware required.  Leave this commented out if you do not have this hardware!
 #include <si5351.h>            // Using this etherkits library because it supporst the B and C version PLLs with external ref clock
 Si5351 si5351;
#else // OCXO_10MHZ
 #include <si5351mcu.h>         // Github https://github.com/pavelmc/Si5351mcu
 Si5351mcu si5351;
#endif // OCXO_10MHZ

//#ifdef USE_RA8875
  //#include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
  //#include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
//#endif // USE_RA8875

#ifdef I2C_LCD
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(LCD_ADR,LCD_COL, LCD_LINES);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#endif

void I2C_Scanner(void);
void MF_Service(int8_t counts, int8_t knob);
void RampVolume(float vol, int16_t rampType);
void printHelp(void);
void printCPUandMemory(unsigned long curTime_millis, unsigned long updatePeriod_millis);
void respondToByte(char c);

//
// --------------------------------------------User Profile Selection --------------------------------------------------------
//
//#define USE_ENET_PROFILE    // <<--- Uncomment this line if you want to use ethernet without editing any variables. 
//
#ifdef USE_ENET_PROFILE
    uint8_t     user_Profile = 0;   // Profile 0 has enet enabled, 1 and 2 do not.
#else
    uint8_t     user_Profile = 1;   // Profile 0 has enet enabled, 1 and 2 do not.
#endif
//
//----------------------------------------------------------------------------------------------------------------------------
//
// These should be saved in EEPROM periodically along with several other parameters
uint8_t     curr_band   = BAND4;    // global tracks our current band setting.  
uint32_t    VFOA        = 0;        // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
uint32_t    VFOB        = 0;
int32_t     Fc          = 0;        //(sample_rate_Hz/4);  // Center Frequency - Offset from DC to see band up and down from cener of BPF.   Adjust Displayed RX freq and Tx carrier accordingly

//control display and serial interaction
bool                enable_printCPUandMemory = false;   // CPU , memory and temperature
void                togglePrintMemoryAndCPU(void) { enable_printCPUandMemory = !enable_printCPUandMemory; };
uint8_t             popup = 0;                          // experimental flag for pop up windows
int32_t             multiKnob(uint8_t clear);           // consumer features use this for control input
volatile int32_t    Freq_Peak = 0;
uint8_t             display_state;   // something to hold the button state for the display pop-up window later.
float               alert_tone_vol = 0.0;

#ifdef USE_RA8875
  RA8875 tft = RA8875(RA8875_CS,RA8875_RESET); //initiate the display object
#else
  RA8876_t3 tft = RA8876_t3(RA8876_CS,RA8876_RESET); //initiate the display object
  FT5206 cts = FT5206(CTP_INT); 
#endif

#ifdef ENET
    extern uint8_t enet_ready;
    extern unsigned long enet_start_fail_time;
    extern uint8_t rx_count;
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
#endif

// Copnnections for FFT Only - chooses either the input or the output to display in the spectrum plot
AudioConnection_F32     patchCord7a(Input,0,         FFT_Switch1,0);
AudioConnection_F32     patchCord7b(Input,1,         FFT_Switch2,0);
AudioConnection_F32     patchCord6a(Output,0,        FFT_Switch1,1);
AudioConnection_F32     patchCord6b(Output,1,        FFT_Switch2,1);
AudioConnection_F32     patchCord5a(FFT_Switch1,0,   myFFT,0);
AudioConnection_F32     patchCord5b(FFT_Switch2,0,   myFFT,1);

// TEST trying out new NB and AGC features  - use selected lines below as make sense
AudioConnection_F32     patchCord10a(Input,0,         NoiseBlanker,0);
AudioConnection_F32     patchCord10b(Input,0,         NoiseBlanker,1);
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
AudioConnection_F32     patchCord3a(RX_Summer,0,     S_Peak,0);
AudioConnection_F32     patchCord3b(RX_Summer,0,     CW_Filter,0);
AudioConnection_F32     patchCord4a(CW_Filter,0,     CW_Peak,0);
AudioConnection_F32     patchCord4b(CW_Filter,0,    CW_RMS,0);
AudioConnection_F32     patchCord4c(CW_Filter,0,     Output,0);
AudioConnection_F32     patchCord4d(CW_Filter,0,     Output,1);
AudioConnection_F32     patchCord4e(sinewave1,0,     Output,0);

AudioControlSGTL5000    codec1;

#include <Metro.h>
// Most of our timers are here.  Spectrum waterfall is in the spectrum settings section of that file
Metro touch         = Metro(50);    // used to check for touch events
Metro tuner         = Metro(1000);  // used to dump unused encoder counts for high PPR encoders when counts is < enc_ppr_response for X time.
Metro meter         = Metro(400);   // used to update the meters
Metro popup_timer   = Metro(500);   // used to check for popup screen request
Metro NTP_updateTx  = Metro(10000); // NTP Request Time interval
Metro NTP_updateRx  = Metro(65000); // Initial NTP timer reply timeout. Program will shorten this after each request.
Metro MF_Timeout    = Metro(4000);// MultiFunction Knob and Switch 

uint8_t enc_ppr_response = VFO_PPR;   // for VFO A/B Tuning encoder. This scales the PPR to account for high vs low PPR encoders.  
                            // 600ppr is very fast at 1Hz steps, worse at 10Khz!

// Set this to be the default MF knob function when it does not have settings focus from a button touch.
// Choose any from the MF Knob aware list below.
uint8_t MF_client;  // Flag for current owner of MF knob services
//
//============================================  Start of Spectrum Setup Section =====================================================
//

// used for spectrum object
//#define FFT_SIZE                  4096            // Need a constant for array size declarion so manually set this value here.  Could try a macro later
int16_t         fft_bins            = FFT_SIZE;     // Number of FFT bins which is FFT_SIZE/2 for real version or FFT_SIZE for iq version
float           fft_bin_size        = sample_rate_Hz/(FFT_SIZE*2);   // Size of FFT bin in HZ.  From sample_rate_Hz/FFT_SIZE for iq
extern int16_t  spectrum_preset;                    // Specify the default layout option for spectrum window placement and size.
int16_t         FFT_Source          = 0;            // Used to switch the FFT input source around
extern Metro spectrum_waterfall_update;             // Timer used for controlling the Spectrum module update rate.

//
// -------------------------------------Setup() -------------------------------------------------------------------
//
FLASHMEM 
void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("**** Running I2C Scanner ****");

    // ---------------- Setup our basic display and comms ---------------------------
    Wire.begin();
    Wire.setClock(100000UL); // Keep at 100K I2C bus transfer data rate for I2C Encoders to work right
    I2C_Scanner();
    MF_client = user_settings[user_Profile].default_MF_client;

#ifdef  I2C_ENCODERS  
    set_I2CEncoders();
#endif // I2C_ENCODERS

#ifdef SV1AFN_BPF
    bpf.begin((int) 0, (TwoWire*) &Wire);
    bpf.setBand(HFNone);
    bpf.setPreamp(false);
    bpf.setAttenuator(false);
#endif

#ifdef USE_RA8875
    tft.begin(RA8875_800x480);
#else
    tft.begin(30000000UL);
    cts.begin();
    cts.setTouchLimit(MAXTOUCHLIMIT);
    tft.touchEnable(false);   // Ensure the resitive ocntroller, if any is off
    tft.backlight(true);
    tft.displayOn(true);
    tft.graphicMode(true);
    tft.clearActiveScreen();
    tft.selectScreen(0);  // Select screen page 0
    tft.fillScreen(BLACK);
    tft.setBackGroundColor(BLACK);
    tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
#endif
    tft.setRotation(0); // 0 is normal, 1 is 90, 2 is 180, 3 is 270 degrees.  
                        // RA8876 touch controller is upside down compared to the RA8875 so correcting for it there.

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

#ifdef DIG_STEP_ATT
    // Initialize the I/O for digital step attenuator if used.
    pinMode(Atten_DATA, OUTPUT);
    pinMode(Atten_CLK, OUTPUT);
    pinMode(Atten_LE, OUTPUT);
    digitalWrite(Atten_DATA,  (uint8_t) OFF);
    digitalWrite(Atten_CLK,  (uint8_t) OFF);
    digitalWrite(Atten_LE,  (uint8_t) OFF);
#endif

#ifdef I2C_LCD    // initialize the I2C LCD
    lcd.init(); 
    lcd.backlight();
    lcd.print("Keith's SDR");
#endif

    //--------------------------   Setup our Audio System -------------------------------------

    AudioMemory_F32(100, audio_settings);
    codec1.enable(); // MUST be before inputSelect()
    delay(5);
    codec1.dacVolumeRampDisable(); // Turn off the sound for now
    codec1.inputSelect(myInput);
    RFgain(0);
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
    alert_tone_vol = 0.0;
    sinewave1.amplitude(alert_tone_vol);
    sinewave1.frequency(600.0); //    Alert tones

#ifdef TEST_SINEWAVE_SIG
    // Create a synthetic sine wave, for testing
    // To use this, edit the connections above
    // # sources to test edges and middle of BW
    float sinewave_vol = 0.005;
    sinewave2.amplitude(sinewave_vol);
    sinewave2.frequency(5000.000); //
    sinewave3.amplitude(sinewave_vol);
    sinewave3.frequency(1000.000); //
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

    initSpectrum_RA8875();                                   // Call before initDisplay() to put screen into Layer 1 mode before any other text is drawn!
    curr_band = user_settings[user_Profile].last_band;       // get last band used from user profile.
    user_settings[user_Profile].sp_preset = spectrum_preset; // uncomment this line to update user profile layout choice
    spectrum_preset = user_settings[user_Profile].sp_preset;
    //==================================== Frequency Set ==========================================
    VFOA = bandmem[curr_band].vfo_A_last; //I used 7850000  frequency CHU  Time Signal Canada
    VFOB = bandmem[curr_band].vfo_B_last;
    // Assignments for our encoder knobs, if any
    initVfo(); // initialize the si5351 vfo
    changeBands(0);   // Sets the VFOs to last used frequencies, sets preselector, active VFO, other last-used settings per band.
    displayRefresh(); // calls the whole group of displayxxx();  Needed to refresh after other windows moving.
    Spectrum_Parm_Generator(spectrum_preset);                 // use this to generate new set of params for the current window size values.
                                                              // calling generator before drawSpectrum() will create a new set of values based on the globals
                                                              // Generator only reads the global values, it does not change them or the database, just prints the new params
    drawSpectrumFrame(user_settings[user_Profile].sp_preset); // Call after initSpectrum() to draw the spectrum object.  Arg is 0 PRESETS to load a preset record
                                                              // DrawSpectrum does not read the globals but does update them to match the current preset.
                                                              // Therefore always call the generator before drawSpectrum() to create a new set of params you can cut anmd paste.
                                                              // Generator never modifies the globals so never affects the layout itself.
                                                              // Print out our starting frequency for testing
    Serial.print("\nInitial Dial Frequency is ");
    Serial.print(formatVFO(VFOA));
    Serial.println("MHz");

    // -------------------- Setup Ethernet and NTP Time and Clock button  --------------------------------
#ifdef ENET
    if (user_settings[user_Profile].enet_enabled)
    {
        struct Standard_Button *t_ptr = &std_btn[UTCTIME_BTN];

        tft.fillRect(t_ptr->bx, t_ptr->by, t_ptr->bw, t_ptr->bh, RA8875_BLACK);
        tft.setFont(Arial_14);
        tft.setTextColor(RA8875_BLUE);
        tft.setCursor(t_ptr->bx+10, t_ptr->by+10);
        tft.print("Starting Network");
        enet_start();
        if (!enet_ready)
        {
            enet_start_fail_time = millis(); // set timer for 10 minute self recovery in main loop
            Serial.println(">Ethernet System Startup Failed, setting retry timer (10 minutes)");
        }
        Serial.println(">Ethernet System Startup");
        //setSyncProvider(getNtpTime);
    }
#endif

    // ---------------------------- Setup speaker on or off and unmute outputs --------------------------------
    if (user_settings[user_Profile].spkr_en == ON)
    {
        codec1.volume(1.0); // Set to full scale.  RampVolume will then scale it up or down 0-100, scaled down to 0.0 to 1.0
        // 0.7 seemed optimal for K7MDL with QRP_Labs RX board with 15 on line input and 20 on line output
        codec1.unmuteHeadphone();
        codec1.unmuteLineout(); //unmute the audio output
        user_settings[user_Profile].mute = OFF;
        displayMute();
        AFgain(0);   // 0 is no change, set to stored last value.  range -100 to +100 percent change of full scale.
        //RampVolume(user_settings[user_Profile].spkr_Vol_last/100, 1); //0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    }
    else
    {
        codec1.muteHeadphone();
        user_settings[user_Profile].mute = ON;
        displayMute();
    }

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
        if (!popup)                           // do not draw in the screen space while the pop up has the screen focus.
                                              // a popup must call drawSpectrumFrame when it is done and clear this flag.
            if (!user_settings[user_Profile].notch)  // TEST:  added to test CPU impact
                spectrum_update(spectrum_preset); // valid numbers are 0 through PRESETS to index the record of predefined window layouts
    }
    
    uint32_t time_n = millis() - time_old;

    if (time_n > delta)
    {
        delta = time_n;
        Serial.print("Tms=");
        Serial.println(delta);
    }
    time_old = millis();

    if (touch.check() == 1)
    {
        Touch(); // touch points and gestures
    }

    if (tuner.check() == 1 && newFreq < enc_ppr_response) // dump counts accumulated over time but < minimum for a step to count.
    {
        VFO.readAndReset();
        newFreq = 0;
    }

    newFreq += VFO.read();    // faster to poll for change since last read
                              // accumulate counts until we have enough to act on for scaling factor to work right.
    if (newFreq != 0 && abs(newFreq) > enc_ppr_response) // newFreq is a positive or negative number of counts since last read.
    {
        newFreq /= enc_ppr_response;    // adjust for high vs low PPR encoders.  600ppr is too fast!
        selectFrequency(newFreq);
        VFO.readAndReset();             // zero out counter fo rnext read.
        newFreq = 0;
    }

    if (MF_Timeout.check() == 1 && MF_client != user_settings[user_Profile].default_MF_client)
    {
        Serial.print("Switching to Default MF knob assignment, current owner is = ");
        Serial.println(MF_client);
        unset_MF_Service(user_settings[user_Profile].default_MF_client);  // will turn off the button, if any, and set the default as new owner.
    }

    #ifdef I2C_ENCODERS
    uint8_t mfg;

    /* Watch for the INT pin to go low */
    if (digitalRead(I2C_INT_PIN) == LOW) 
    {
        #ifdef MF_ENC_ADDR
        // Check the status of the encoder (if enabled) and call the callback
        if(MF_ENC.updateStatus() && user_settings[user_Profile].encoder1_client)
        {            
            mfg = MF_ENC.readStatus();
            if (mfg) { Serial.print("****Checked MF_Enc status = "); Serial.println(mfg); }
        }
        #endif
        #ifdef ENC2_ADDR
        if(ENC2.updateStatus() && user_settings[user_Profile].encoder2_client)
        {
            mfg = ENC2.readStatus();
            if (mfg) {Serial.print("****Checked Encoder #2 status = "); Serial.println(mfg); }
        }
        #endif
        #ifdef ENC3_ADDR
        if(ENC3.updateStatus() && user_settings[user_Profile].encoder3_client)
        {
            mfg = ENC3.readStatus();
            if (mfg) {Serial.print("****Checked Encoder #3 status = "); Serial.println(mfg); }
        }
        #endif
    }
    
    #else
    // Use a mechanical encoder on the GPIO pisn, if any.
    if (MF_client)  // skip if no one is listening.MF_Service();  // check the Multi-Function encoder and pass results to the current owner, of any.
    {
        static int8_t counts = 0;   
        counts = (int8_t) round(multiKnob(0)/4);
        MF_Service(counts, MF_client);
    }
    #endif // I2C_ENCODERS

    if (meter.check() == 1) // update our meters
    {
        Peak();
        // Code_Peak();
        // Quad_Check();
    }

    if (popup_timer.check() == 1 && popup) // stop spectrum updates, clear the screen and post up a keyboard or something
    {
        // Service popup window
    }

    //respond to Serial commands
    while (Serial.available())
    {
        respondToByte((char)Serial.read());
    }

    //check to see whether to print the CPU and Memory Usage
    if (enable_printCPUandMemory)
        printCPUandMemory(millis(), 3000); //print every 3000 msec

#ifdef ENET  // Don't compile this code if no ethernet usage intended

    if (user_settings[user_Profile].enet_enabled) // only process enet if enabled.
    {
        if (!enet_ready)
            if ((millis() - enet_start_fail_time) > 600000) // check every 10 minutes (600K ms) and attempt a restart.
                enet_start();
        enet_read(); // Check for Control head commands
        if (rx_count != 0)
        {
        } //get_remote_cmd();       // scan buffer for command strings

        if (NTP_updateTx.check() == 1)
        {
            //while (Udp_NTP.parsePacket() > 0)
            //{};  // discard any previously received packets
            sendNTPpacket(timeServer);  // send an NTP packet to a time server
            NTP_updateRx.interval(100); // Start a timer to check RX reply
        }
        if (NTP_updateRx.check() == 1) // Time to check for a reply
        {
            if (getNtpTime());                         // Get our reply
            NTP_updateRx.interval(65000); // set it long until we need it again later
            Ethernet.maintain();          // keep our connection fresh
        }
    }

    // Check if the time has updated (1 second) and update the clock display
    if (timeStatus() != timeNotSet && enet_ready) // Only display if ethernet is active and have a valid time source
    {
        if (now() != prevDisplay)
        {
            //update the display only if time has changed
            prevDisplay = now();
            displayTime();
        }
    }
#endif // End of Ethenet related functions here
}

//
// _______________________________________ Volume Ramp __________________
//
// Ramps the volume down to specified level 0 to 1.0 range using 1 of 3 types.  It remembers the original volume level so
// you are reducing it by a factor then raisinmg back up a factor toward the orignal volume setting.
// Range is 1.0 for full original and 0 for off.
FLASHMEM 
void RampVolume(float vol, int16_t rampType)
{
    const char *rampName[] = {
        "No Ramp (instant)", // loud pop due to instant change
        "Normal Ramp",       // graceful transition between volume levels
        "Linear Ramp"        // slight click/chirp
    };

    Serial.println(rampName[rampType]);

    // configure which type of volume transition to use
    if (rampType == 0)
    {
        codec1.dacVolumeRampDisable();
    }
    else if (rampType == 1)
    {
        codec1.dacVolumeRamp();
    }
    else
    {
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
    if (curTime_millis < lastUpdate_millis)
        lastUpdate_millis = 0; //handle wrap-around of the clock
    if ((curTime_millis - lastUpdate_millis) > updatePeriod_millis)
    { //is it time to update the user interface?
        Serial.print("\nCPU Cur/Peak: ");
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
        Serial.println("*** End of Report ***");

        lastUpdate_millis = curTime_millis; //we will use this value the next time around.
        delta = 0;
        #ifdef I2C_ENCODERS
          //blink_MF_RGB();
          //blink_AF_RGB();
        #endif // I2C_ENCODERS
    }
}
//
// _______________________________________ Console Parser ____________________________________
//
//switch yard to determine the desired action
FLASHMEM
void respondToByte(char c)
{
    char s[2];
    s[0] = c;
    s[1] = 0;
    if (!isalpha((int)c) && c != '?')
        return;
    switch (c)
    {
    case 'h':
    case '?':
        printHelp();
        break;
    case 'C':
    case 'c':
        Serial.println("Toggle printing of memory and CPU usage.");
        togglePrintMemoryAndCPU();
        break;
    case 'M':
    case 'm':
        Serial.println("\nMemory Usage (FlexInfo)");
        flexRamInfo();
        Serial.println("*** End of Report ***");
        break;
    default:
        Serial.print("You typed ");
        Serial.print(s);
        Serial.println(".  What command?");
    }
}
//
// _______________________________________ Print Help Menu ____________________________________
//
FLASHMEM 
void printHelp(void)
{
    Serial.println();
    Serial.println("Help: Available Commands:");
    Serial.println("   h: Print this help");
    Serial.println("   C: Toggle printing of CPU and Memory usage");
    Serial.println("   M: Print Detailed Memory Region Usage Report");
}
#ifndef I2C_ENCODERS
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
//          It will clear the count to look for next action.
//          If a clear is not performed then the consumer function must deal with figuring out how
//          the value has changed and what to do with it.
//          The value returned will be a positive or negative value with some count (usally each step si 4 but not always)
//
int32_t multiKnob(uint8_t clear)
{
    static int32_t mf_count = 0;

    if (clear)
    {
        //Multi.readAndReset(); // toss results, zero the encoder
        mf_count = 0;
    }
    else
    {}    //mf_count = Multi.readAndReset(); // read and reset the Multifunction knob.  Apply results to any in-focus function, if any
    return mf_count;
}
#endif  // I2C_ENCODERS

// Deregister the MF_client
FLASHMEM
void unset_MF_Service(uint8_t client_name)
{
    if (client_name == MF_client)  // nothing needed if this is called from the button itself to deregister
    {
        MF_client = user_settings[user_Profile].default_MF_client;    // assign new owner to default for now.
        return;   
    }
    // This must be from a timeout or a new button before timeout
    // Turn off button of the previous owner, if any, using the MF knob at change of owner or timeout
    // Some button can be left on such as Atten or other non-button MF users.  Just leave them off this list.
    switch (MF_client)
    {
        case MFNONE: {
            // no current owner, return
        } break;
        case RFGAIN_BTN: {           
            setRFgain();   //since it was active toggle the output off
        } break;
        case AFGAIN_BTN: {         
            setAFgain();
        } break;
        case  REFLVL_BTN: {
            setRefLevel();
        } break;
        case NB_BTN:
        case MFTUNE:
        case ATTEN_BTN:
        default     : {          
        // No button for VFO tune, atten button stays on
            MF_client = user_settings[user_Profile].default_MF_client;
        } break;
    
    } 
}
// ---------------------------------- set_MF_Service -----------------------------------
// Register the owner for the MF services.  Called by a client like RF Gain button.  Last caller wins.
// Clears the MF knob count and sets the flag for the new owner
// On any knob event the owner will get called with the MF counter value or switch action

// Potential owners can query the MF_client variable to see who owns the MF knob.  
// Can take ownership by calling this fucntion and passing the enum ID for it's service function
FLASHMEM 
void set_MF_Service(uint8_t client_name)
{
    
    #ifndef I2C_ENCODERS   // The I2c encoders are using interrupt driven callback functions so no need to call them, they will call us.
    multiKnob(1);       // for non-I2C encoder, clear the counts.
    #endif //  I2C_ENCODERS
    unset_MF_Service(user_settings[user_Profile].default_MF_client);
    MF_client = client_name;    
    MF_Timeout.reset();  // reset (extend) timeout timer as long as there is activity.  
                         // When it expires it will be switched to default
    //Serial.print("New MF Knob Client ID is ");
    //Serial.println(MF_client);
}

// ------------------------------------ MF_Service --------------------------------------
//
//  Called in the main loop to look for an encoder event and if found, call the registered function
//
static uint16_t old_ts;
FLASHMEM 
void MF_Service(int8_t counts, int8_t knob)
{  
    if (counts == 0)  // no knob movement, nothing to do.
        return;
    
    if (knob == MF_client)
        MF_Timeout.reset();  // if it isthe MF_Client then reset (extend) timeout timer as long as there is activity.  
                            // When it experies it will be switched to default

    //Serial.print("MF Knob Client ID is ");
    //Serial.println(MF_client);

    switch (knob)      // Give this owner control until timeout
    {
        case MFNONE: {
            // no current owner, return
        } break;
        case RFGAIN_BTN: {           
            RFgain(counts);
        } break;
        case AFGAIN_BTN: {   
            AFgain(counts);
        } break;
        case  REFLVL_BTN: {
            RefLevel(counts*-1);
        } break;
        case  ATTEN_BTN: {
            if (counts> 31)
                counts = 31;
            if (counts <= 0)
                counts = -1;
            int8_t att_tmp = bandmem[curr_band].attenuator_dB + counts;
            if (att_tmp <=0)
                att_tmp = 0;
            #ifdef DIG_STEP_ATT
              setAtten_dB(att_tmp);  // set attenuator level to value in database for this band           
            #endif
        } break;
        case  NB_BTN: {
            setNBLevel(counts);
        } break;
        case MFTUNE :
        default     : {   
            old_ts = bandmem[curr_band].tune_step;
            bandmem[curr_band].tune_step =3;
            selectFrequency(counts);
            bandmem[curr_band].tune_step = old_ts;
        } break;        
    };
}
//
//  Scans for any I2C connected devices and reports them to the serial terminal.  Usually done early in startup.
//
FLASHMEM 
void I2C_Scanner(void)
{
  byte error, address; //variable for error and I2C address
  int nDevices;

  Serial.println("Scanning...");

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
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  //delay(500); // wait 5 seconds for the next I2C scan
}

#endif  // _SDR_RA8875_
