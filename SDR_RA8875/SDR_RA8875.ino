//  SDR_RA8875.INO
//
//  Main Program File
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
#include "RadioConfig.h"        // Majority of declarations here to drive the #ifdefs that follow
#include "Hilbert.h"            // filter coefficients
#include "AudioFilterConvolution_F32.h"
#include <AudioSwitch_OA_F32.h>

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
    #include "SDR_I2C_Encoder.h"              // See RadioConfig.h for more config including assigning an INT pin.                                          // Hardware verson 2.1, Arduino library version 1.40.                                 // Hardware verson 2.1, Arduino library version 1.40.
    //Class initialization with the I2C addresses
    extern i2cEncoderLibV2 MF_ENC; // Address 0x61 only - Jumpers A0, A5 and A6 are soldered.//
    extern i2cEncoderLibV2 ENC2; // Address 0x62 only - Jumpers A1, A5 and A6 are soldered.//  
    extern i2cEncoderLibV2 ENC3; // Address 0x62 only - Jumpers A1, A5 and A6 are soldered.// 
#else 
    #ifdef MECH_ENCODERS
        Encoder Multi(MF_ENC_PIN_A, MF_ENC_PIN_B);  // Multi Function Encoder pins assignments usnig GPIO pinss
        Encoder AF(29,28);   // AF gain control - not used yet
        Encoder RF(33,34);   // RF gain control - not used yet 
    #endif
#endif // I2C_ENCODER

#ifdef OCXO_10MHZ               // This turns on a group of features feature that are hardware required.  Leave this commented out if you do not have this hardware!
 #include <si5351.h>            // Using this etherkits library because it supporst the B and C version PLLs with external ref clock
 Si5351 si5351;
#else // OCXO_10MHZ
 #include <si5351mcu.h>         // Github https://github.com/pavelmc/Si5351mcu
 Si5351mcu si5351;
#endif // OCXO_10MHZ

#ifdef I2C_LCD
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(LCD_ADR,LCD_COL, LCD_LINES);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#endif

COLD void I2C_Scanner(void);
COLD void MF_Service(int8_t counts, int8_t knob);
COLD void RampVolume(float vol, int16_t rampType);
COLD void printHelp(void);
COLD void printCPUandMemory(unsigned long curTime_millis, unsigned long updatePeriod_millis);
COLD void respondToByte(char c);
COLD void touchBeep(bool enable);
COLD void digitalClockDisplay(void); 
COLD unsigned long processSyncMessage();
COLD time_t getTeensy3Time();
COLD void printDigits(int digits);
HOT void Check_PTT(void);
COLD void initDSP(void);
COLD void SetFilter(void);

//
// --------------------------------------------User Profile Selection --------------------------------------------------------
//
//#define USE_ENET_PROFILE    // <<--- Uncomment this line if you want to use ethernet without editing any variables. 
//

#ifndef PANADAPTER
 #ifdef USE_ENET_PROFILE
    uint8_t     user_Profile = 0;   // Profile 0 has enet enabled, 1 and 2 do not.
 #else  // USE_ENET_PROFILE
    uint8_t     user_Profile = 1;   // Profile 0 has enet enabled, 1 and 2 do not.
 #endif  // USE_ENET_PROFILE
#else  // PANADAPTER
    uint8_t     user_Profile = 2;   // Profile 2 is optimized for Panadapter usage
#endif  // PANADAPTER

//
//----------------------------------------------------------------------------------------------------------------------------
//
// These should be saved in EEPROM periodically along with several other parameters
uint8_t     curr_band   = BAND2;    // global tracks our current band setting.  
uint32_t    VFOA        = 0;        // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
uint32_t    VFOB        = 0;
int32_t     Fc          = 0;        //(sample_rate_Hz/4);  // Center Frequency - Offset from DC to see band up and down from cener of BPF.   Adjust Displayed RX freq and Tx carrier accordingly

//control display and serial interaction
bool        enable_printCPUandMemory = false;   // CPU , memory and temperature
void        togglePrintMemoryAndCPU(void) { enable_printCPUandMemory = !enable_printCPUandMemory; };
uint8_t     popup = 0;                          // experimental flag for pop up windows
int32_t     multiKnob(uint8_t clear);           // consumer features use this for control input
int32_t     Freq_Peak = 0;
uint8_t     display_state;   // something to hold the button state for the display pop-up window later.
bool        touchBeep_flag = false;
bool        MeterInUse;  // S-meter flag to block updates while the MF knob has control
static int  last_PTT_Input = 1;   // track input pin state changes after any debounce timers
static int  PTT_pin_state = 1;    // current input pin state
static unsigned long PTT_Input_time = 0;  // Debounce timer
static int  PTT_Input_debounce = 0;   // Debounce state tracking

#ifdef USE_RA8875
    RA8875 tft    = RA8875(RA8875_CS,RA8875_RESET); //initiate the display object
#else
    RA8876_t3 tft = RA8876_t3(RA8876_CS,RA8876_RESET); //initiate the display object
    FT5206 cts    = FT5206(CTP_INT); 
#endif

#ifdef ENET
    extern uint8_t enet_ready;
    extern unsigned long enet_start_fail_time;
    extern uint8_t rx_count;
#endif

//#include <Metro.h>
// Most of our timers are here.  Spectrum waterfall is in the spectrum settings section of that file
Metro touch             = Metro(50);    // used to check for touch events
Metro tuner             = Metro(1000);  // used to dump unused encoder counts for high PPR encoders when counts is < enc_ppr_response for X time.
Metro meter             = Metro(400);   // used to update the meters
Metro popup_timer       = Metro(500);   // used to check for popup screen request
Metro NTP_updateTx      = Metro(10000); // NTP Request Time interval
Metro NTP_updateRx      = Metro(65000); // Initial NTP timer reply timeout. Program will shorten this after each request.
Metro MF_Timeout        = Metro(4000);  // MultiFunction Knob and Switch 
Metro touchBeep_timer   = Metro(80); // Feedback beep for button touches

uint8_t enc_ppr_response = VFO_PPR;   // for VFO A/B Tuning encoder. This scales the PPR to account for high vs low PPR encoders.  
                            // 600ppr is very fast at 1Hz steps, worse at 10Khz!

// Set this to be the default MF knob function when it does not have settings focus from a button touch.
// Choose any from the MF Knob aware list below.
uint8_t MF_client;  // Flag for current owner of MF knob services
bool    MF_default_is_active = true;
//
//============================================  Start of Spectrum Setup Section =====================================================
//
// Pick the one to run through the whole audio chain and FFT on the display
#define FFT_SIZE 4096               // 4096 //2048//1024     
uint16_t fft_size = FFT_SIZE;       // This value wil lbe passed to the lib init function.
                                    // Ensure the matching FFT resources are enabled in the lib .h file!

// These are normally defined in the spectrum_RA887z.h file
// enable any combo for multiple FFT resolutions for pan and zoom - each takes CPU time and more memory
//#define FFT_4096
//#define FFT_2048
//#define FFT_1024

#ifdef FFT_4096
    DMAMEM AudioAnalyzeFFT4096_IQ_F32  myFFT_4096;  // choose which you like, set FFT_SIZE accordingly.
#endif
#ifdef FFT_2048   
    DMAMEM AudioAnalyzeFFT2048_IQ_F32  myFFT_2048;
#endif
#ifdef FFT_1024
    DMAMEM AudioAnalyzeFFT1024_IQ_F32  myFFT_1024;
#endif

#ifndef BYPASS_SPECTRUM_MODULE
  Spectrum_RA887x spectrum_RA887x(0, fft_size);   //, &myFFT);   // initialize the Spectrum Library
  extern Metro    spectrum_waterfall_update;          // Timer used for controlling the Spectrum module update rate.
  extern struct   Spectrum_Parms Sp_Parms_Def[];
#endif

// Audio Library setup stuff
//const float sample_rate_Hz = 11000.0f;  //43Hz /bin  5K spectrum
//const float sample_rate_Hz = 22000.0f;  //21Hz /bin 6K wide
//const float sample_rate_Hz = 44100.0f;  //43Hz /bin  12.5K spectrum
//const float sample_rate_Hz = 48000.0f;  //46Hz /bin  24K spectrum for 1024.  
//const float sample_rate_Hz = 51200.0f;  // 50Hz/bin for 1024, 200Hz/bin for 256 FFT. 20Khz span at 800 pixels 2048 FFT
//const float sample_rate_Hz = 51200.0f;  // 50Hz/bin for 1024, 200Hz/bin for 256 FFT. 20Khz span at 800 pixels 2048 FFT
//const float sample_rate_Hz = 96000.0f;    // <100Hz/bin at 1024FFT, 50Hz at 2048, 40Khz span at 800 pixels and 2048FFT
const float sample_rate_Hz = 102400.0f; // 100Hz/bin at 1024FFT, 50Hz at 2048, 40Khz span at 800 pixels and 2048FFT
//const float sample_rate_Hz = 192000.0f; // 190Hz/bin - does
//const float sample_rate_Hz = 204800.0f; // 200/bin at 1024 FFT
//
// ---------------------------- Set some FFT related parameters ------------------------------------
int16_t     fft_bins         = fft_size;     // Number of FFT bins which is FFT_SIZE/2 for real version or FFT_SIZE for iq version
float       fft_bin_size     = sample_rate_Hz/(fft_size*2);   // Size of FFT bin in HZ.  From sample_rate_Hz/FFT_SIZE for iq
int16_t     FFT_Source       = 0;            // Used to switch the FFT input source around

int filterCenter;
int filterBandwidth;

const int audio_block_samples = 128;          // do not change this!
const int RxAudioIn = AUDIO_INPUT_LINEIN;
const int MicAudioIn = AUDIO_INPUT_MIC;
 
AudioSettings_F32           audio_settings(sample_rate_Hz, audio_block_samples);                           
AudioInputI2S_F32           Input(audio_settings);
AudioMixer4_F32             FFT_Switch_L(audio_settings);
AudioMixer4_F32             FFT_Switch_R(audio_settings);
AudioSwitch4_OA_F32         RxTx_InputSwitch_L(audio_settings);
AudioSwitch4_OA_F32         RxTx_InputSwitch_R(audio_settings);
AudioSwitch4_OA_F32         FFT_OutSwitch_L(audio_settings);
AudioSwitch4_OA_F32         FFT_OutSwitch_R(audio_settings);
AudioMixer4_F32             OutputSwitch_L(audio_settings);
AudioMixer4_F32             OutputSwitch_R(audio_settings);
DMAMEM AudioFilterFIR_F32   RX_Hilbert_Plus_45(audio_settings);
DMAMEM AudioFilterFIR_F32   RX_Hilbert_Minus_45(audio_settings);
DMAMEM AudioFilterFIR_F32   TX_Hilbert_Plus_45(audio_settings);
DMAMEM AudioFilterFIR_F32   TX_Hilbert_Minus_45(audio_settings);
AudioFilterConvolution_F32  FilterConv(audio_settings);  // DMAMEM on this causes it to not be adjustable. Would save 50K local variable space if it worked.
AudioMixer4_F32             RX_Summer(audio_settings);
AudioAnalyzePeak_F32        S_Peak(audio_settings); 
AudioAnalyzePeak_F32        Q_Peak(audio_settings); 
AudioAnalyzePeak_F32        I_Peak(audio_settings);
AudioOutputI2S_F32          Output(audio_settings);
radioNoiseBlanker_F32       NoiseBlanker(audio_settings);  // DMAMEM on this item breaks stopping RX audio flow.  Would save 10K local variable space
AudioSynthWaveformSine_F32  Beep_Tone; // for audible alerts like touch beep confirmations
AudioSynthSineCosine_F32    TxTestTone_A;  // For TX path test tone
AudioSynthWaveformSine_F32  TxTestTone_B;  // For TX path test tone

// Connections for FINput and FFT - chooses either the input or the output to display in the spectrum plot
AudioConnection_F32     patchCord_FFT_In_L(Input,1,                         FFT_Switch_L,0);  // route raw input audio to the FFT display
AudioConnection_F32     patchCord_FFT_In_R(Input,0,                         FFT_Switch_R,0);
AudioConnection_F32     patchCord_FFT_Tone_L(OutputSwitch_L,0,              FFT_Switch_L,1);  // route final audio out to the FFT display
AudioConnection_F32     patchCord_FFT_Tone_R(OutputSwitch_R,0,              FFT_Switch_R,1);
AudioConnection_F32     patchCord_Tx_Tone_L(TxTestTone_A,0,                 FFT_Switch_L,2);  // Test Tones
AudioConnection_F32     patchCord_Tx_Tone_R(TxTestTone_B,0,                 FFT_Switch_R,2);

AudioConnection_F32     patchCord_FFT_OUT_L(FFT_Switch_L,0,                FFT_OutSwitch_L,0);        // Route selected audio source to the selected FFT - should save CPU time
AudioConnection_F32     patchCord_FFT_OUT_R(FFT_Switch_R,0,                FFT_OutSwitch_R,0);

#ifdef FFT_4096
AudioConnection_F32     patchCord_FFT_L_4096(FFT_OutSwitch_L,0,             myFFT_4096,0);        // Route selected audio source to the FFT
AudioConnection_F32     patchCord_FFT_R_4096(FFT_OutSwitch_R,0,             myFFT_4096,1);
#endif
#ifdef FFT_2048
AudioConnection_F32     patchCord_FFT_L_2048(FFT_OutSwitch_L,1,             myFFT_2048,0);        // Route selected audio source to the FFT
AudioConnection_F32     patchCord_FFT_R_2048(FFT_OutSwitch_R,1,             myFFT_2048,1);
#endif
#ifdef FFT_1024
AudioConnection_F32     patchCord_FFT_L_1024(FFT_OutSwitch_L,2,             myFFT_1024,0);        // Route selected audio source to the FFT
AudioConnection_F32     patchCord_FFT_R_1024(FFT_OutSwitch_R,2,             myFFT_1024,1);
#endif
// Input (Mic or Line) go to switch.   Use source selected for FFT.
//AudioConnection_F32     patchCord_Input_L(Input,0,                        RxTx_InputSwitch_L,0);  // Test bypass
//AudioConnection_F32     patchCord_Input_R(Input,1,                        RxTx_InputSwitch_R,0);
AudioConnection_F32     patchCord_Input_L(FFT_Switch_L,0,                   RxTx_InputSwitch_L,0);  // 0 is RX. Output 1 is Tx chain
AudioConnection_F32     patchCord_Input_R(FFT_Switch_R,0,                   RxTx_InputSwitch_R,0);

// Noise Blanker
AudioConnection_F32     patchCord10a(RxTx_InputSwitch_L,0,                  NoiseBlanker,0);
AudioConnection_F32     patchCord10b(RxTx_InputSwitch_R,0,                  NoiseBlanker,1);
AudioConnection_F32     patchCord11a(NoiseBlanker,0,                        RX_Hilbert_Plus_45,0);
AudioConnection_F32     patchCord11b(NoiseBlanker,1,                        RX_Hilbert_Minus_45,0);

// Normal Audio Chain - RX audio on Line In to headphone jack
AudioConnection_F32     patchCord2a(RX_Hilbert_Plus_45,0,                   I_Peak,0);
AudioConnection_F32     patchCord2b(RX_Hilbert_Minus_45,0,                  Q_Peak,0);
AudioConnection_F32     patchCord2c(RX_Hilbert_Plus_45,0,                   RX_Summer,0);  // phase shift +45 deg
AudioConnection_F32     patchCord2d(RX_Hilbert_Minus_45,0,                  RX_Summer,1);  // phase shift -45 deg
AudioConnection_F32     patchCord2e(Beep_Tone,0,                            RX_Summer,2);  // For button beep if enabled
AudioConnection_F32     patchCord3a(RX_Summer,0,                            S_Peak,0);  // S meter source
AudioConnection_F32     patchCord3L(RX_Summer,0,                            FilterConv,0);  // variable Bandwidth filtering
AudioConnection_F32     patchCord_RxOut_L(FilterConv,0,                     OutputSwitch_L,0);  // route raw input audio to output for TX
AudioConnection_F32     patchCord_RxOut_R(FilterConv,0,                     OutputSwitch_R,0);  // route raw input audio to output for TX

// In TX mic is selected as input and is switched to the TX audio path (straight to output for now)
AudioConnection_F32     patchCord_Mic_Input_L(RxTx_InputSwitch_L,1,         TX_Hilbert_Plus_45,0);
AudioConnection_F32     patchCord_Mic_Input_R(RxTx_InputSwitch_R,1,         TX_Hilbert_Minus_45,0);
AudioConnection_F32     patchCord_Tx_Filt_L(TX_Hilbert_Plus_45,0,           OutputSwitch_L,1);  // phase shift +45 deg
AudioConnection_F32     patchCord_Tx_Filt_R(TX_Hilbert_Minus_45,0,          OutputSwitch_R,1);  // phase shift -45 deg

// Selected source go to output (selected as headphone or lineout in the code)
AudioConnection_F32     patchCord4L(OutputSwitch_L,0,                       Output,0);  // output to headphone jack Left
AudioConnection_F32     patchCord4R(OutputSwitch_R,0,                       Output,1);  // output to headphone jack Right

AudioControlSGTL5000    codec1;

// -------------------------------------Setup() -------------------------------------------------------------------
// 
tmElements_t tm;
time_t prevDisplay = 0; // When the digital clock was displayed

COLD void setup()
{
    pinMode(PTT_INPUT, INPUT_PULLUP);   // Init PTT in and out lines
    pinMode(PTT_OUT1, OUTPUT);
    digitalWrite(PTT_OUT1, HIGH);
    Serial.begin(115200);
    //delay(500);
    Serial.println(F("Initializing SDR_RA887x Program"));
    Serial.print(F("FFT Size is "));
    Serial.println(fft_size);
    Serial.println(F("**** Running I2C Scanner ****"));

    // ---------------- Setup our basic display and comms ---------------------------
    Wire.begin();
    Wire.setClock(100000UL); // Keep at 100K I2C bus transfer data rate for I2C Encoders to work right
    I2C_Scanner();
    MF_client = user_settings[user_Profile].default_MF_client;
    MF_default_is_active = true;
    MeterInUse = false;    
     
    #ifdef  I2C_ENCODERS  
        set_I2CEncoders();
    #endif // I2C_ENCODERS

    #ifdef SV1AFN_BPF
        // set address to 0x20 (32 decimal) for V2.X adafruit MCP23017 library. 
        // A value of 0 now kills the I2C bus.
        bpf.begin((int) 32, (TwoWire*) &Wire);
        bpf.setBand(HFNone);
        bpf.setPreamp(false);
        bpf.setAttenuator(false);
    #endif

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

#ifndef BYPASS_SPECTRUM_MODULE        
        spectrum_RA887x.setActiveWindow_default();
#endif        
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
 
    // -------------------- Setup Ethernet and NTP Time and Clock button  --------------------------------
    #ifdef ENET
    if (user_settings[user_Profile].enet_enabled)
    {
        struct Standard_Button *t_ptr = &std_btn[UTCTIME_BTN];

        tft.fillRect(t_ptr->bx, t_ptr->by, t_ptr->bw, t_ptr->bh, RA8875_BLACK);
        tft.setFont(Arial_14);
        tft.setTextColor(RA8875_BLUE);
        tft.setCursor(t_ptr->bx+10, t_ptr->by+10);
        tft.print(F("Starting Network"));
        enet_start();
        if (!enet_ready)
        {
            enet_start_fail_time = millis(); // set timer for 10 minute self recovery in main loop
            Serial.println(F("Ethernet System Startup Failed, setting retry timer (10 minutes)"));
        }
        Serial.println(F("Ethernet System Startup Completed"));
        //setSyncProvider(getNtpTime);
    }
    #endif

    //--------------------------   Setup our Audio System -------------------------------------
    initDSP();
    RFgain(0);
    
    // Update time on startup from RTC. If a USB connection is up, get the time from a PC.  
    // Later if enet is up, get time from NTP periodically.
    setSyncProvider(getTeensy3Time);   // the function to get the time from the RTC
    if(timeStatus()!= timeSet) // try this other way
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time"); 
    if (Serial.available()) 
    {
        time_t t = processSyncMessage();
        if (t != 0) 
        {
            //Teensy3Clock.set(t); // set the RTC
            setTime(t);
        }
    }
    digitalClockDisplay(); // print time to terminal
    Serial.println("Clock Update");

#ifndef BYPASS_SPECTRUM_MODULE
    spectrum_RA887x.initSpectrum(user_settings[user_Profile].sp_preset); // Call before initDisplay() to put screen into Layer 1 mode before any other text is drawn!
#endif

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
        lcd.print(F("MyCall SDR"));  // Edit this to what you like to see on your display
    #endif

/*  // Read SD Card data
    // To use the audio card SD card Reader instead of the Teensy 4.1 onboard Card Reader
    // UNCOMMENT THESE TWO LINES FOR TEENSY AUDIO BOARD:
    //SPI.setMOSI(7);  // Audio shield has MOSI on pin 7
    //SPI.setSCK(14);  // Audio shield has SCK on pin 14
    // see if the card is present and can be initialized:
    SD_CardInfo();
    // open or create our config file.  Filenames follow DOS 8.3 format rules
    Open_SD_cfgfile();
    // test our file
    // make a string for assembling the data to log:
    write_db_tables();
    read_db_tables();
    write_radiocfg_h();         // write out the #define to a file on the SD card.  
                                // This could be used by the PC during compile to override the RadioConfig.h
*/
    // -------------------- Setup our radio settings and UI layout --------------------------------

    curr_band = user_settings[user_Profile].last_band;       // get last band used from user profile.

    //==================================== Frequency Set ==========================================
    #ifdef PANADAPTER
    VFOA = PANADAPTER_LO;
    VFOB = PANADAPTER_LO;
    #else
    VFOA = bandmem[curr_band].vfo_A_last; //I used 7850000  frequency CHU  Time Signal Canada
    VFOB = bandmem[curr_band].vfo_B_last;
    #endif

    initVfo(); // initialize the si5351 vfo

#ifndef BYPASS_SPECTRUM_MODULE    
    spectrum_RA887x.Spectrum_Parm_Generator(0, 0); // use this to generate new set of params for the current window size values. 
                                                              // 1st arg is new target layout record - usually 0 unless you create more examples
                                                              // 2nd arg is current empty layout record (preset) value - usually 0
                                                              // calling generator before drawSpectrum() will create a new set of values based on the globals
                                                              // Generator only reads the global values, it does not change them or the database, just prints the new params                                                             
    spectrum_RA887x.drawSpectrumFrame(user_settings[user_Profile].sp_preset); // Call after initSpectrum() to draw the spectrum object.  Arg is 0 PRESETS to load a preset record
                                                              // DrawSpectrum does not read the globals but does update them to match the current preset.
                                                              // Therefore always call the generator before drawSpectrum() to create a new set of params you can cut anmd paste.
                                                              // Generator never modifies the globals so never affects the layout itself.
                                                              // Print out our starting frequency for testing
    //sp.drawSpectrumFrame(6);   // for 2nd window
#endif  

    Serial.print(F("\nInitial Dial Frequency is "));
    Serial.print(formatVFO(VFOA));
    Serial.println(F("MHz"));

    // ---------------------------- Setup speaker on or off and unmute outputs --------------------------------
    if (user_settings[user_Profile].spkr_en == ON)
    {
        codec1.volume(1.0f); // Set to full scale.  RampVolume will then scale it up or down 0-100, scaled down to 0.0 to 1.0
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
    changeBands(0);     // Sets the VFOs to last used frequencies, sets preselector, active VFO, other last-used settings per band.
                        // Call changeBands() here after volume to get proper startup volume

    //------------------Finish the setup by printing the help menu to the serial connections--------------------
    printHelp();
    InternalTemperature.begin(TEMPERATURE_NO_ADC_SETTING_CHANGES);
   
    #ifdef FT817_CAT
        Serial.println("Starting the CAT port and reading some radio information if available");
        init_CAT_comms();  // initialize the CAT port
        print_CAT_status();  // Test Line to read daa forfm FT817 if attached.
    #endif
    #ifdef ALL_CAT
        CAT_setup();   // Setup teh Serial port for cnfigured Radio comm port
    #endif
}

static uint32_t delta = 0;
//
// __________________________________________ Main Program Loop  _____________________________________
//
HOT void loop()
{
    static int32_t newFreq = 0;
    static uint32_t time_old = 0;

#ifndef BYPASS_SPECTRUM_MODULE
    // Update spectrum and waterfall based on timer
    if (spectrum_waterfall_update.check() == 1) // The update rate is set in drawSpectrumFrame() with spect_wf_rate from table
    {
        if (!popup)                           // do not draw in the screen space while the pop up has the screen focus.
                                              // a popup must call drawSpectrumFrame when it is done and clear this flag.
            if (!user_settings[user_Profile].notch)  // TEST:  added to test CPU impact
                spectrum_RA887x.spectrum_update(user_settings[user_Profile].sp_preset, (bandmem[curr_band].VFO_AB_Active == VFO_A), VFOA, VFOB); // valid numbers are 0 through PRESETS to index the record of predefined window layouts
                // spectrum_update(6);  // for 2nd window
    }
#endif
    uint32_t time_n = millis() - time_old;

    if (time_n > delta)
    {
        delta = time_n;
        Serial.print(F("Tms="));
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
        //VFO.read();
        newFreq = 0;
    }

    newFreq += VFO.read();    // faster to poll for change since last read
                              // accumulate counts until we have enough to act on for scaling factor to work right.
    if (newFreq != 0 && abs(newFreq) > enc_ppr_response) // newFreq is a positive or negative number of counts since last read.
    {
        newFreq /= enc_ppr_response;    // adjust for high vs low PPR encoders.  600ppr is too fast!
        selectFrequency(newFreq);
        VFO.readAndReset();
        //VFO.read();             // zero out counter for next read.
        newFreq = 0;
    }

    if (MF_Timeout.check() == 1)
    {
        MeterInUse = false;  
        //if (MF_client != user_settings[user_Profile].default_MF_client)
        if (!MF_default_is_active)
        {
            Serial.print(F("Switching to Default MF knob assignment, current owner is = "));
            Serial.println(MF_client);
            set_MF_Service(user_settings[user_Profile].default_MF_client);  // will turn off the button, if any, and set the default as new owner.
            MF_default_is_active = true;
        }
    }

    #ifdef I2C_ENCODERS
    uint8_t mfg;

    // Watch for the I2C Encoder INT pin to go low  (these I2C encoders are typically daisy-chained)
    if (digitalRead(I2C_INT_PIN) == LOW) 
    {
        #ifdef MF_ENC_ADDR
        // Check the status of the encoder (if enabled) and call the callback
        if(MF_ENC.updateStatus() && user_settings[user_Profile].encoder1_client)
        {            
            mfg = MF_ENC.readStatus();
            if (mfg) {}
                //{ Serial.print(F("****Checked MF_Enc status = ")); Serial.println(mfg); }
        }
        #endif
        #ifdef ENC2_ADDR
        if(ENC2.updateStatus() && user_settings[user_Profile].encoder2_client)
        {
            mfg = ENC2.readStatus();
            if (mfg) {}
                //{Serial.print(F("****Checked Encoder #2 status = ")); Serial.println(mfg); }
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
        #ifdef MECH_ENCODERS
            // Use a mechanical encoder on the GPIO pisn, if any.
            if (MF_client)  // skip if no one is listening.MF_Service();  // check the Multi-Function encoder and pass results to the current owner, of any.
            {
                static int8_t counts = 0;   
                counts = (int8_t) round(multiKnob(0)/4);
                MF_Service(counts, MF_client);
            }
        #endif
    #endif // I2C_ENCODERS

    Check_PTT();

    if (meter.check() == 1) // update our meters
    {
        Peak();
        // Code_Peak();
        // Quad_Check();
    }

    #ifdef PANADAPTER
        #ifdef ALL_CAT
            //if (CAT_update.check() == 1) // update our meters
            //{
                // update Panadapter CAT port data using same time  
                CAT_handler();
            //}
        #endif  // ALL_CAT
    #endif // PANADAPTER

    if (popup_timer.check() == 1 && popup) // stop spectrum updates, clear the screen and post up a keyboard or something
    {
        // Service popup window
    }
    
    // The timer and flag are set by the rogerBeep() function
    if (touchBeep_flag && touchBeep_timer.check() == 1)   
    {
        touchBeep(false);    
    }

    //respond to Serial commands
    while (Serial.available())
    {
        if (Serial.peek() == 'T')
        {
            time_t t = processSyncMessage();
            if (t != 0) 
            {
                Serial.println(F("Time Update"));
                Teensy3Clock.set(t); // set the RTC
                setTime(t);
                digitalClockDisplay();
            }
        }
        else
            respondToByte((char)Serial.read());
    }

    //check to see whether to print the CPU and Memory Usage
    if (enable_printCPUandMemory)
        printCPUandMemory(millis(), 3000); //print every 3000 msec

#ifdef ENET // Don't compile this code if no ethernet usage intended

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
#endif // End of Ethenet related functions here

    // Check if the time has updated (1 second) and update the clock display
    if (timeStatus() != timeNotSet) // && enet_ready) // Only display if ethernet is active and have a valid time source
    {
        if (now() != prevDisplay)
        {
            //update the display only if time has changed
            prevDisplay = now();
            displayTime();
        }
    }
}
//
//-------------------------------------  Check_PTT() ------------------------------------------------------
// Check on GPIO pins  - PTT, others
//
HOT void Check_PTT(void)
{
    PTT_pin_state = digitalRead(PTT_INPUT);
    // Start debpunce timer if a new pin change of state detected
    if (PTT_pin_state != last_PTT_Input)   // user_settings[user_Profile].xmit == OFF)
    {   
        if (!PTT_Input_debounce)
        {
            //Serial.println("Start PTT Settle Timer");
            PTT_Input_debounce = 1;     // Start debounce timer
            PTT_Input_time = millis();  // Start of transition
            last_PTT_Input = PTT_pin_state;
        }
    }
    // Debounce timer in progress?  If so exit.
    if (PTT_Input_debounce && ((PTT_Input_time - millis()) < 20)) 
    {
        //Serial.println("Waiting for PTT Settle Time");
        return;
    }
    // If the timer is satisfied, change the TX/RX state
    if (PTT_Input_debounce)
    {
        PTT_Input_debounce= 0;     // Reset timer for next change
        if (!last_PTT_Input)  // if TX
        {
            //Serial.println("PTT TX Detected");
            Xmit();  // toggle transmit state
            last_PTT_Input = 0;
        }
        else // if RX
        {
            //Serial.println("PTT TX Released");
            Xmit();  // toggle transmit state
            last_PTT_Input = 1;
        }
    }
}
//
// _______________________________________ Volume Ramp __________________
//
// Ramps the volume down to specified level 0 to 1.0 range using 1 of 3 types.  It remembers the original volume level so
// you are reducing it by a factor then raisinmg back up a factor toward the orignal volume setting.
// Range is 1.0 for full original and 0 for off.
COLD void RampVolume(float vol, int16_t rampType)
{
    #ifdef PANADAPTER
        vol = 1.0; // No relays changes so not needed in this mode.
        codec1.dacVolume(vol);
        return;
    #endif

    //const char *rampName[] = {
    //    "No Ramp (instant)", // loud pop due to instant change
    //    "Normal Ramp",       // graceful transition between volume levels
    //    "Linear Ramp"        // slight click/chirp
    //};
    //Serial.println(rampName[rampType]);

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
COLD void printCPUandMemory(unsigned long curTime_millis, unsigned long updatePeriod_millis)
{
    //static unsigned long updatePeriod_millis = 3000; //how many milliseconds between updating gain reading?
    static unsigned long lastUpdate_millis = 0;

    //has enough time passed to update everything?
    if (curTime_millis < lastUpdate_millis)
        lastUpdate_millis = 0; //handle wrap-around of the clock
    if ((curTime_millis - lastUpdate_millis) > updatePeriod_millis)
    { //is it time to update the user interface?
        Serial.print(F("\nCPU Cur/Peak: "));
        Serial.print(audio_settings.processorUsage());
        Serial.print(F("%/"));
        Serial.print(audio_settings.processorUsageMax());
        Serial.println(F("%"));
        Serial.print(F("CPU Temperature:"));
        Serial.print(InternalTemperature.readTemperatureF(), 1);
        Serial.print(F("F "));
        Serial.print(InternalTemperature.readTemperatureC(), 1);
        Serial.println(F("C"));
        Serial.print(F(" Audio MEM Float32 Cur/Peak: "));
        Serial.print(AudioMemoryUsage_F32());
        Serial.print(F("/"));
        Serial.println(AudioMemoryUsageMax_F32());
        Serial.println(F("*** End of Report ***"));

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
COLD void respondToByte(char c)
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
        Serial.println(F("Toggle printing of memory and CPU usage."));
        togglePrintMemoryAndCPU();
        break;
    case 'M':
    case 'm':
        Serial.println(F("\nMemory Usage (FlexInfo)"));
        //flexRamInfo();
        Serial.println(F("*** End of Report ***"));
        break;
    default:
        Serial.print(F("You typed "));
        Serial.print(s);
        Serial.println(F(".  What command?"));
    }
}
//
// _______________________________________ Print Help Menu ____________________________________
//
COLD void printHelp(void)
{
    Serial.println();
    Serial.println(F("Help: Available Commands:"));
    Serial.println(F("   h: Print this help"));
    Serial.println(F("   C: Toggle printing of CPU and Memory usage"));
    Serial.println(F("   M: Print Detailed Memory Region Usage Report"));
    Serial.println(F("   T+10 digits: Time Update. Enter T and 10 digits for seconds since 1/1/1970"));
}
#ifndef I2C_ENCODERS
    //#ifdef MECH_ENCODERS
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
    //#endif  //MECH_ENCODERS
#endif  // I2C_ENCODERS

// Deregister the MF_client
COLD void unset_MF_Service(uint8_t old_client_name)  // clear the old owner button status
{ 
    if (old_client_name == MF_client)  // nothing needed if this is called from the button itself to deregister
    {
        //MF_client = user_settings[user_Profile].default_MF_client;    // assign new owner to default for now.
        //return;   
    }
    
    // This must be from a timeout or a new button before timeout
    // Turn off button of the previous owner, if any, using the MF knob at change of owner or timeout
    // Some buttons can be left on such as Atten or other non-button MF users.  Just leave them off this list.
    switch (old_client_name)
    {
        case MFNONE: {
            // no current owner, return
        } break;
        case RFGAIN_BTN: {         
            setRFgain(-1);   //since it was active toggle the output off
        } break;
        case AFGAIN_BTN: {         
            setAFgain(-1);
        } break;
        case  REFLVL_BTN: {
            setRefLevel(-1);
        } break;
        case ATTEN_BTN:
        case NB_BTN:
        case MFTUNE:
        default     : {          
        // No button for VFO tune, atten button stays on
            //MF_client = user_settings[user_Profile].default_MF_client;
        } break;         
    } 
}
// ---------------------------------- set_MF_Service -----------------------------------
// Register the owner for the MF services.  Called by a client like RF Gain button.  Last caller wins.
// Clears the MF knob count and sets the flag for the new owner
// On any knob event the owner will get called with the MF counter value or switch action

// Potential owners can query the MF_client variable to see who owns the MF knob.  
// Can take ownership by calling this fucntion and passing the enum ID for it's service function
COLD void set_MF_Service(uint8_t new_client_name)  // this will be the new owner after we clear the old one
{
    
    #ifndef I2C_ENCODERS   // The I2c encoders are using interrupt driven callback functions so no need to call them, they will call us.
        multiKnob(1);       // for non-I2C encoder, clear the counts.
    #endif //  I2C_ENCODERS
    unset_MF_Service(MF_client);    //  turn off current button if on
    MF_client = new_client_name;        // Now assign new owner
    //if (MF_client == user_settings[user_Profile].default_MF_client)
    //    MF_default_is_active = true;
    //else 
    //    MF_default_is_active = false;
    MF_Timeout.reset();  // reset (extend) timeout timer as long as there is activity.  
                         // When it expires it will be switched to default
    //Serial.print("New MF Knob Client ID is ");
    //Serial.println(MF_client);
}

// ------------------------------------ MF_Service --------------------------------------
//
//  Called in the main loop to look for an encoder event and if found, call the registered function
//
//static uint16_t old_ts;
COLD void MF_Service(int8_t counts, uint8_t knob)
{  
    if (counts == 0)  // no knob movement, nothing to do.
        return;
    
    if (knob == MF_client)
        MF_Timeout.reset();  // if it is the MF_Client then reset (extend) timeout timer as long as there is activity.  
                            // When it expires it will be switched to default

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
              setAtten_dB(att_tmp);  // set attenuator level to value in database for this band
        } break;
        case  NB_BTN: {
            setNBLevel(counts);
        } break;
        case MFTUNE :
        default     : {   
            //old_ts = bandmem[curr_band].tune_step;
            //bandmem[curr_band].tune_step =0;
            selectFrequency(counts);
            //bandmem[curr_band].tune_step = old_ts;
        } break;        
    };
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
COLD void printKnownChips(byte address)
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
        Beep_Tone.amplitude(user_settings[user_Profile].rogerBeep_Vol);
        Beep_Tone.frequency((float) user_settings[user_Profile].pitch); //    Alert tones
    }
    else 
    {
        //if (rogerBeep_timer.check() == 1)   // make sure another event does not cut it off early
        //{ 
            touchBeep_flag = false;
            Beep_Tone.amplitude(0.0);
        //}
    }
}

COLD void printDigits(int digits)
{
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

COLD time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

COLD unsigned long processSyncMessage() 
{
    unsigned long pctime = 0L;
    const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 

    if (Serial.find(TIME_HEADER)) // Search for the 'T' char in incoming serial stream of chars
    {
        pctime = Serial.parseInt();  // following the 'T' get the digits and convert to an int
        //return pctime;
        //Serial.println(pctime);
        if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
            pctime = 0L; // return 0 to indicate that the time is not valid
        }
    }
    return pctime;  // return will be seconds since jan 1 1970.
}

COLD void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

COLD void SetFilter(void)
{
    FilterConv.initFilter((float32_t)filterCenter, 90, 2, filterBandwidth);
}

COLD void initDSP(void)
{
    AudioMemory_F32(100, audio_settings);   // 4096IQ FFT needs about 75 or 80 at 96KHz sample rate
    RX_Hilbert_Plus_45.begin(Hilbert_Plus45_40K,151);   // Left channel Rx
    RX_Hilbert_Minus_45.begin(Hilbert_Minus45_40K,151); // Right channel Rx
    TX_Hilbert_Plus_45.begin(Hilbert_Plus45_28K,151);   // Left channel TX
    TX_Hilbert_Minus_45.begin(Hilbert_Minus45_28K,151); // Right channel TX
    codec1.enable(); // MUST be before inputSelect()
    delay(5);
    codec1.dacVolumeRampDisable(); // Turn off the sound for now
    codec1.inputSelect(RxAudioIn);
    codec1.autoVolumeControl(2, 0, 0, -36.0, 12, 6);                   // add a compressor limiter
        //   codec1.autoVolumeControl( 0-2, 0-3, 0-1, 0-96, 3, 3);
        //   autoVolumeControl(maxGain, response, hardLimit, threshold, attack, decay);
    codec1.autoVolumeEnable(); // let the volume control itself..... poor mans agc
    //codec1.autoVolumeDisable();// Or don't let the volume control itself
    codec1.muteLineout(); //mute TX audio 
    codec1.muteHeadphone();
    codec1.adcHighPassFilterDisable();
    codec1.dacVolume(0); // set the "dac" volume (extra control)

    // Select our sources for the FFT.  mode.h will change this so CW uses the output (for now as an experiment)
    AudioNoInterrupts();    
    RxTx_InputSwitch_L.setChannel(0); // Select RX path
    RxTx_InputSwitch_R.setChannel(0); // Select RX path

    // Typically Choose one pair - RX input, DSP chain output (RX or TX), or Test Tone (input before processing)
    FFT_Switch_L.gain(0, 1.0f); //  1 is RX, 0 is TX
    FFT_Switch_R.gain(0, 1.0f); //  1 is RX, 0 is TX
    FFT_Switch_L.gain(1, 0.0f); //  1 is TX, 0 is RX
    FFT_Switch_R.gain(1, 0.0f); //  1 is TX, 0 is RX
    FFT_Switch_L.gain(2, 0.0f); //  1 is TestTone A on, 0 OFF
    FFT_Switch_R.gain(2, 0.0f); //  1 is TestTone B on, 0 OFF

    // Route selected FFT source to one of the possible many FFT processors - should save CPU time for unused FFTs
    if (fft_size == 4096)
    {
        FFT_OutSwitch_L.setChannel(0); //  1 is 4096, 0 is Off
        FFT_OutSwitch_R.setChannel(0); //  1 is 4096, 0 is Off
    }
    else if (fft_size == 2048)
    {
      FFT_OutSwitch_L.setChannel(1); //  1 is 2048, 0 is Off
      FFT_OutSwitch_R.setChannel(1); //  1 is 2048, 0 is Off
    }
    else if(fft_size == 1024)
    {
        FFT_OutSwitch_L.setChannel(2); //  1 is 1024, 0 is Off
        FFT_OutSwitch_R.setChannel(2); //  1 is 1024, 0 is Off
    }

    // Send the audio out to the world
    OutputSwitch_L.gain(0, 1.0f); // Ch 0 is Left 
    OutputSwitch_R.gain(0, 1.0f); // Ch 1 is Right    
    OutputSwitch_L.gain(1, 0.0f); // Turn TX off
    OutputSwitch_R.gain(1, 0.0f); // Turn TX off          

    NoiseBlanker.useTwoChannel(true);
    AudioInterrupts();

    AFgain(0);  // Set RX audio level back to last position on RX
    // Set up TX Audio audio out level
    codec1.lineOutLevel(user_settings[user_Profile].lineOut_Vol_last); // range 13 to 31.  13 => 3.16Vp-p, 31=> 1.16Vp-p
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
  
    // Choose our output type.  Can do dB, RMS or power
    #ifdef FFT_4096
        myFFT_4096.setOutputType(FFT_DBFS); // FFT_RMS or FFT_POWER or FFT_DBFS
        // Uncomment one these to try other window functions
        //  myFFT.windowFunction(NULL);
        //  myFFT.windowFunction(AudioWindowBartlett1024);
        //  myFFT.windowFunction(AudioWindowFlattop1024);
        myFFT_4096.windowFunction(AudioWindowHanning1024);
        myFFT_4096.setNAverage(3); // experiment with this value.  Too much causes a large time penalty
    #endif
    #ifdef FFT_2048
        myFFT_2048.setOutputType(FFT_DBFS); // FFT_RMS or FFT_POWER or FFT_DBFS
        // Uncomment one these to try other window functions
        //  myFFT.windowFunction(NULL);
        //  myFFT.windowFunction(AudioWindowBartlett1024);
        //  myFFT.windowFunction(AudioWindowFlattop1024);
        myFFT_2048.windowFunction(AudioWindowHanning1024);
        myFFT_2048.setNAverage(3); // experiment with this value.  Too much causes a large time penalty
    #endif
    #ifdef FFT_1024
        myFFT_1024.setOutputType(FFT_DBFS); // FFT_RMS or FFT_POWER or FFT_DBFS
        // Uncomment one these to try other window functions
        //  myFFT.windowFunction(NULL);
        //  myFFT.windowFunction(AudioWindowBartlett1024);
        //  myFFT.windowFunction(AudioWindowFlattop1024);
        myFFT_1024.windowFunction(AudioWindowHanning1024);
        myFFT_1024.setNAverage(3); // experiment with this value.  Too much causes a large time penalty
    #endif
}

void TXAudio(int TX)
{

    float TxTestTone_Vol = 0.5f;

    // initDSP() and startup in RX mode already enables most of our resources.  
    // This function switches input sources between line in and mic in and Test Tones (A and B),
    //   then set levels and retores them on RX.
    // Previously Lineout was the same as the headphone and used for RX audio to speaker.
    // To add TX support, we will send RX audio to the headphone only, and TX audio will go to Line out. 
    
    if (TX)  // Switch to Mic input on TX
    {
        TxTestTone_Vol = 0.8f;
        TxTestTone_A.amplitude(TxTestTone_Vol);
        TxTestTone_A.frequency(1000.0f); 
        TxTestTone_B.amplitude(TxTestTone_Vol); //TxTestTone_Vol);
        TxTestTone_B.frequency(2000.0f); 
        
        AudioNoInterrupts();
        
        RxTx_InputSwitch_L.setChannel(1); // Route audio to TX path (1)
        RxTx_InputSwitch_R.setChannel(1); // Route audio to TX path (1)

        // Typically choose one pair, Ch 0, 1 or 2.
        FFT_Switch_L.gain(0, 0.0f);     //  Turn Rx line-In source Off (ch 0)
        FFT_Switch_R.gain(0, 0.0f);     //  Turn Rx line-In source Off (ch0)
        FFT_Switch_L.gain(1, 0.0f);     //  Select mic source for TX (ch 1)
        FFT_Switch_R.gain(1, 0.0f);     //  Select mic source for TX (ch 1)
        FFT_Switch_L.gain(2, 1.0f);     //  Ch 2 is test tone A source (ch 2) 
        FFT_Switch_R.gain(2, 1.0f);     //  Ch 2 is test tone B source (ch 2)

        OutputSwitch_L.gain(0, 0.0f);   // Turn RX OFF (ch 0 to 0.0)
        OutputSwitch_R.gain(0, 0.0f);   // Turn RX OFF  
        OutputSwitch_L.gain(1, 1.0f);   // Turn TX ON (ch 1 to 1.0f)
        OutputSwitch_R.gain(1, -1.0f);   // Turn TX ON          
        
        codec1.micGain(30);  // 0 to 63dB

        Serial.println("Switching to Tx"); 
        codec1.muteHeadphone(); 
        codec1.inputSelect(MicAudioIn);   // Mic is microphone, Line-In is from Receiver audio
        codec1.unmuteLineout();
        
        AudioInterrupts();
    }
    else // back to RX
    {
        AudioNoInterrupts();

        TxTestTone_Vol = 0.0f;
        TxTestTone_A.amplitude(TxTestTone_Vol);
        TxTestTone_B.amplitude(TxTestTone_Vol);        

        RxTx_InputSwitch_L.setChannel(0); // Select RX path
        RxTx_InputSwitch_R.setChannel(0); // Select RX path

        // Typically choose one pair, Ch 0, 1 or 2.
        FFT_Switch_L.gain(0, 1.0f); //  1 is RX, 0 is TX
        FFT_Switch_R.gain(0, 1.0f); //  1 is RX, 0 is TX
        FFT_Switch_L.gain(1, 0.0f); //  1 is TX, 0 is RX
        FFT_Switch_R.gain(1, 0.0f); //  1 is TX, 0 is RX
        FFT_Switch_L.gain(2, 0.0f); // Ch 2 is test tone, Turn off
        FFT_Switch_R.gain(2, 0.0f); // Ch 2 is test tone, Turn off
        
        OutputSwitch_L.gain(0, 1.0f); // Ch 0 is RX, Turn on
        OutputSwitch_R.gain(0, 1.0f); // Ch 0 is RX, Turn on
        OutputSwitch_L.gain(1, 0.0f); // Turn TX off
        OutputSwitch_R.gain(1, 0.0f); // Turn TX off   
    
        Serial.println("Switching to Rx"); 
        codec1.muteLineout(); //mute the TX audio output to transmitter input 
        codec1.inputSelect(RxAudioIn);  // switch back to RX audio input
        codec1.unmuteHeadphone();
        
        AudioInterrupts();
    }
}
