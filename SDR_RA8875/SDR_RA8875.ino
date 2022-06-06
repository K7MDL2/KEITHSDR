//  SDR_RA8875.INO
//
//  Main Program File
//
//  Spectrum, Display, full FP32 library conversion completed 3/2021. Uses FFTXXXX_IQ_F32 FFT I and Q version files
//      XXXX can be the 1024, 2048 or 4096 versions.
//  Spectrum uses the raw FFT output and is not calibrated.
//
// _______________________________________ Setup_____________________________________________
//
// Pickup build time options from RadioConfig.h
#include "RadioConfig.h"        // Majority of declarations here to drive the #ifdefs that follow
#include "SDR_RA8875.h"
#include "SDR_Data.h"
#include "Hilbert.h"            // filter coefficients
#include "hilbert19A.h"
#include "hilbert121A.h"
#include "hilbert251A.h"

//#define USB32   // Switch between F32 and I16 versions of USB Audio interface
// So far I16 method has been working better.  

#ifdef USB32
#include "AudioStream_F32.h"   // This is included by USB_Audio_F32.h but is placed here as a reminder to use the 48Khz modified version
#include "USB_Audio_F32.h"
#endif

#ifdef USE_RS_HFIQ
    // init the RS-HFIQ library
    SDR_RS_HFIQ RS_HFIQ;
#endif

#ifdef USE_FFT_LO_MIXER
    #include "hilbert251A.h"        // filter coefficients
#endif

#ifdef SV1AFN_BPF               // This turns on support for the Bandpass Filter board and relays for LNA and Attenuation
    #include <SVN1AFN_BandpassFilters.h> // Modified and redistributed in this build source folder
    SVN1AFN_BandpassFilters bpf;  // The SV1AFN Preselector module supporing all HF bands and a preamp and Attenuator. 
                                // For 60M coverage requires and updated libary file set.
#endif // SV1AFN_BPF

#ifdef I2C_ENCODERS              // This turns on support for DuPPa.net I2C encoder with RGB LED integrated. 
    //  This is a basic example for using the I2C Encoder V2
    //  The counter is set to work between +10 to -10, at every encoder click the counter value is printed on the terminal.
    //  It's also printed when the push button is released.
    //  When the encoder is turned the led turn green
    //  When the encoder reach the max or min the led turn red
    //  When the encoder is pushed, the led turn blue

    //  Connections with Teensy 4.1:
    //  - -> GND
    //  + -> 3.3V
    //  SDA -> 18
    //  SCL -> 19
    //  INT -> 29 - Dependent on particular board pin assignments
    #include "SDR_I2C_Encoder.h"              // See RadioConfig.h for more config including assigning an INT pin.                                          // Hardware verson 2.1, Arduino library version 1.40.                                 // Hardware verson 2.1, Arduino library version 1.40.
    //Class initialization with the I2C addresses
    #ifdef MF_ENC_ADDR
      extern i2cEncoderLibV2 MF_ENC; // Address 0x61 only - Jumpers A0, A5 and A6 are soldered.//
    #endif
    #ifdef ENC2_ADDR
      extern i2cEncoderLibV2 ENC2; // Address 0x62 only - Jumpers A1, A5 and A6 are soldered.//  
    #endif
    #ifdef ENC3_ADDR
      extern i2cEncoderLibV2 ENC3; // Address 0x63 only - Jumpers A1, A5 and A6 are soldered.// 
    #endif
    #ifdef ENC4_ADDR
      extern i2cEncoderLibV2 ENC4;    /* Address 0x64 only - Jumpers A0, A1, A5 and A6 are soldered.*/
    #endif
    #ifdef ENC5_ADDR
      extern i2cEncoderLibV2 ENC5;    /* Address 0x65 only - Jumpers A0, A1, A5 and A6 are soldered.*/
    #endif
    #ifdef ENC6_ADDR
      extern i2cEncoderLibV2 ENC6;    /* Address 0x66 only - Jumpers A0, A1, A5 and A6 are soldered.*/
    #endif    
#else 
    #ifdef MECH_ENCODERS   // if you have both i2c and mechanical encoders, assignment get tricky.  Default is only i2c OR mechanical
        // On the Teensy motherboards, ENC1 is the VFO.  This is ENC2 jack. In the database this is handled as "encoder1_client" and "default_MF_client"
        Encoder Multi(ENC2_PIN_A, ENC2_PIN_B);  // Multi Function Encoder GPIO pins assignments
        // These are single fucntion encoders "encoderX_client" where X is 2 through 6
        //Encoder Encoder2(ENC3_PIN_A, ENC3_PIN_B);   // "encoder2_client" mapped to ENC3 on the board
    #endif
#endif // I2C_ENCODER

// Choose your actual pin assignments for any you may have.
Encoder VFO(ENC1_PIN_A, ENC1_PIN_B); // pins defined in RadioConfig.h - mapped to ENC1 on the PCB

#ifndef USE_RS_HFIQ
    #ifdef OCXO_10MHZ               // This turns on a group of features feature that are hardware required.  Leave this commented out if you do not have this hardware!
        #include <si5351.h>            // Using this etherkits library because it supporst the B and C version PLLs with external ref clock
        Si5351 si5351;
    #else // OCXO_10MHZ
        #include <si5351mcu.h>         // Github https://github.com/pavelmc/Si5351mcu
        Si5351mcu si5351;
    #endif // OCXO_10MHZ
#endif // RS-HFIQ

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
HOT  void Check_PTT(void);
COLD void initDSP(void);
COLD void SetFilter(void);
HOT  void RF_Limiter(float peak_avg);
COLD void TX_RX_Switch(bool TX,uint8_t mode_sel,bool b_Mic_On,bool b_USBIn_On,bool b_ToneA,bool b_ToneB,float TestTone_Vol);
COLD void Change_FFT_Size(uint16_t new_size, float new_sample_rate_Hz);
COLD void resetCodec(void);
COLD void TwinPeaks(void);  // Test auto I2S Alignment 
HOT void Check_Encoders(void);
#ifdef USE_RS_HFIQ
  HOT  void RS_HFIQ_Service(void);  // commands the RS_HFIQ over USB Host serial port
#endif
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
uint8_t     curr_band   = BAND80M;    // global tracks our current band setting.  
uint32_t    VFOA        = 0;        // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
uint32_t    VFOB        = 0;
int32_t     Fc          = 0;        //(sample_rate_Hz/4);  // Center Frequency - Offset from DC to see band up and down from cener of BPF.   Adjust Displayed RX freq and Tx carrier accordingly
int32_t     ModeOffset  = 0;        // Holds offset based on CW mode pitch

//control display and serial interaction
bool        enable_printCPUandMemory = false;   // CPU , memory and temperature
void        togglePrintMemoryAndCPU(void) { enable_printCPUandMemory = !enable_printCPUandMemory; };
uint8_t     popup                   = 0;   // experimental flag for pop up windows
int32_t     multiKnob(uint8_t clear);      // consumer features use this for control input
int32_t     Freq_Peak               = 0;
uint8_t     display_state;   // something to hold the button state for the display pop-up window later.
bool        touchBeep_flag          = false;
bool        MeterInUse;  // S-meter flag to block updates while the MF knob has control
uint8_t     last_PTT_Input          = 1;   // track input pin state changes after any debounce timers
uint8_t     PTT_pin_state           = 1;   // current input pin state
unsigned long PTT_Input_time        = 0;   // Debounce timer
uint8_t     PTT_Input_debounce      = 0;   // Debounce state tracking
float       S_Meter_Peak_Avg;              // For RF AGC Limiter
bool        TwoToneTest             = OFF; // Chooses between Mic ON or Dual test tones in transmit (Xmit() in Control.cpp)
float       pan                     = 0.0f;

#ifdef USE_RA8875
    RA8875 tft    = RA8875(RA8875_CS,RA8875_RESET); //initialize the display object
#else
    RA8876_t3 tft = RA8876_t3(RA8876_CS,RA8876_RESET); //initiate the display object
    FT5206 cts    = FT5206(CTP_INT);    // Be sure to set the motherboard version used to get the correct Touch INT!                                    // Set in 2 places, the spectrum_RA887x library and in RadioConfig.h
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
Metro MF_Timeout        = Metro(2000);  // MultiFunction Knob and Switch 
Metro touchBeep_timer   = Metro(80);    // Feedback beep for button touches
Metro ENC_Read_timer    = Metro(250);   // time allowed to accumulate counts for slow moving detented encoders

uint8_t enc_ppr_response = VFO_PPR;   // for VFO A/B Tuning encoder. This scales the PPR to account for high vs low PPR encoders.  
                            // 600ppr is very fast at 1Hz steps, worse at 10Khz!

// Set this to be the default MF knob function when it does not have settings focus from a button touch.
// Choose any from the MF Knob aware list below.
uint8_t MF_client;  // Flag for current owner of MF knob services
bool    MF_default_is_active = true;

//
//============================================  Start of Spectrum Setup Section =====================================================
//

// Audio Library setup stuff
//float sample_rate_Hz = 11000.0f;  //43Hz /bin  5K spectrum
//float sample_rate_Hz = 22000.0f;  //21Hz /bin 6K wide
//float sample_rate_Hz = 44100.0f;  //43Hz /bin  12.5K spectrum
float sample_rate_Hz = 48000.0f;  //46Hz /bin  24K spectrum for 1024.  
//float sample_rate_Hz = 96000.0f;    // <100Hz/bin at 1024FFT, 50Hz at 2048, 40Khz span at 800 pixels and 2048FFT
//float sample_rate_Hz = 192000.0f; // 190Hz/bin - does

float zoom_in_sample_rate_Hz = sample_rate_Hz;  // used in combo with new fft size for zoom level
//
// ---------------------------- Set some FFT related parameters ------------------------------------

// Pick the one to run through the whole audio chain and FFT on the display
// defined in Spectrum_RA887x.h, included here for FYI.
//#define FFT_SIZE 4096               // 4096 //2048//1024     
uint16_t    fft_size            = FFT_SIZE;       // This value wil lbe passed to the lib init function.
                                    // Ensure the matching FFT resources are enabled in the lib .h file!                            
int16_t     fft_bins            = fft_size;     // Number of FFT bins which is FFT_SIZE/2 for real version or FFT_SIZE for iq version
float       fft_bin_size        = sample_rate_Hz/(fft_size*2);   // Size of FFT bin in HZ.  From sample_rate_Hz/FFT_SIZE for iq
const int   audio_block_samples = 128;          // do not change this!
const int   RxAudioIn = AUDIO_INPUT_LINEIN;
const int   MicAudioIn = AUDIO_INPUT_MIC;
uint16_t    filterCenter;
uint16_t    filterBandwidth;
uint16_t    TX_filterCenter = 1800;
uint16_t    TX_filterBandwidth = 2000;
#ifndef BYPASS_SPECTRUM_MODULE
  extern Metro    spectrum_waterfall_update;          // Timer used for controlling the Spectrum module update rate.
  extern struct   Spectrum_Parms Sp_Parms_Def[];
#endif

#ifdef BETATEST
    DMAMEM  float32_t  fftOutput[4096];  // Array used for FFT Output to the INO program
    DMAMEM  float32_t  window[2048];     // Windows reduce sidelobes with FFT's *Half Size*
    DMAMEM  float32_t  fftBuffer[8192];  // Used by FFT, 4096 real, 4096 imag, interleaved
    DMAMEM  float32_t  sumsq[4096];      // Required ONLY if power averaging is being done
#endif

AudioSettings_F32  audio_settings(sample_rate_Hz, audio_block_samples);    

#ifdef FFT_4096
    #ifndef BETATEST
        DMAMEM  AudioAnalyzeFFT4096_IQ_F32  myFFT_4096;  // choose which you like, set FFT_SIZE accordingly.
    #else
        AudioAnalyzeFFT4096_IQEM_F32 myFFT_4096(fftOutput, window, fftBuffer, sumsq);  // with power averaging array 
    #endif
#endif
#ifdef FFT_2048   
    DMAMEM AudioAnalyzeFFT2048_IQ_F32  myFFT_2048;
#endif
#ifdef FFT_1024
    DMAMEM AudioAnalyzeFFT1024_IQ_F32  myFFT_1024;
#endif

#ifdef W7PUA_I2S_CORRECTION
  AudioAlignLR_F32          TwinPeak(SIGNAL_HARDWARE, PIN_FOR_TP, false, audio_settings);
#endif

#ifdef USB32
AudioInputUSB_F32           USB_In(audio_settings);
AudioOutputUSB_F32          USB_Out(audio_settings);
#else
AudioInputUSB               USB_In;
AudioOutputUSB              USB_Out;
AudioConvert_I16toF32       convertL_In;
AudioConvert_I16toF32       convertR_In;
AudioConvert_F32toI16       convertL_Out;
AudioConvert_F32toI16       convertR_Out;
#endif

AudioInputI2S_F32           Input(audio_settings);  // Input from Line In jack (RX board)
AudioMixer4_F32             I_Switch(audio_settings); // Select between Input from RX board or Mic/TestTone
AudioMixer4_F32             Q_Switch(audio_settings);
AudioMixer4_F32             TX_Source(audio_settings);  // Select Mic, ToneA or ToneB or any combo
AudioSwitch4_OA_F32         RxTx_InputSwitch_L(audio_settings);  // Select among TX or RX sources except FM
AudioSwitch4_OA_F32         RxTx_InputSwitch_R(audio_settings);
AudioSwitch4_OA_F32         FFT_OutSwitch_I(audio_settings); // Select a source to send to the FFT engine
AudioSwitch4_OA_F32         FFT_OutSwitch_Q(audio_settings);
AudioMixer4_F32             OutputSwitch_I(audio_settings); // Processed audio from any mode to boost amp then out
AudioMixer4_F32             OutputSwitch_Q(audio_settings);
DMAMEM AudioFilterFIR_F32   RX_Hilbert_Plus_45(audio_settings);
DMAMEM AudioFilterFIR_F32   RX_Hilbert_Minus_45(audio_settings);
DMAMEM AudioFilterFIR_F32   TX_Hilbert_Plus_45(audio_settings);
DMAMEM AudioFilterFIR_F32   TX_Hilbert_Minus_45(audio_settings);
AudioFilterConvolution_F32  RX_FilterConv(audio_settings);  // DMAMEM on this causes it to not be adjustable. Would save 50K local variable space if it worked.
//AudioFilterConvolution_F32  TX_FilterConv(audio_settings);  // DMAMEM on this causes it to not be adjustable. Would save 50K local variable space if it worked.
AudioMixer4_F32             RX_Summer(audio_settings);
AudioAnalyzePeak_F32        S_Peak(audio_settings); 
AudioOutputI2S_F32          Output(audio_settings);
radioNoiseBlanker_F32       NoiseBlanker(audio_settings);   // DMAMEM on this item breaks stopping RX audio flow.  Would save 10K local variable space
AudioLMSDenoiseNotch_F32    LMS_Notch(audio_settings);
RadioFMDetector_F32         FM_Detector(audio_settings);
AudioSynthWaveformSine_F32  Beep_Tone(audio_settings);      // for audible alerts like touch beep confirmations
AudioSynthSineCosine_F32    TxTestTone_A(audio_settings);   // For TX path test tone
AudioSynthWaveformSine_F32  TxTestTone_B(audio_settings);   // For TX path test tone
AudioEffectGain_F32         Amp1_L(audio_settings);
AudioEffectGain_F32         Amp1_R(audio_settings);         // Some well placed gain stages
AudioMixer4_F32             FFT_Atten_I(audio_settings);
AudioMixer4_F32             FFT_Atten_Q(audio_settings);         // Some well placed gain stages
RadioIQMixer_F32            FM_LO_Mixer(audio_settings);

DMAMEM AudioFilter90Deg_F32        FFT_90deg_Hilbert(audio_settings);
DMAMEM AudioFilterFIR_F32          bpf1(audio_settings);

#ifdef USE_FFT_LO_MIXER
    AudioFilter90Deg_F32    FFT_90deg_Hilbert(audio_settings);
    RadioIQMixer_F32        FFT_LO_Mixer_I(audio_settings);
    RadioIQMixer_F32        FFT_LO_Mixer_Q(audio_settings);
#endif
#ifdef USE_FREQ_SHIFTER
    AudioEffectFreqShiftFD_OA_F32 FFT_SHIFT_I(audio_settings); // the frequency-domain processing block
    AudioEffectFreqShiftFD_OA_F32 FFT_SHIFT_Q(audio_settings); // the frequency-domain processing block
#endif

// Connections for LineInput and FFT - chooses either the input or the output to display in the spectrum plot
// Assuming the mic input is applied to both left and right - need to verify.  Only need the left really
#if defined(W7PUA_I2S_CORRECTION)
    AudioConnection_F32     patchCord_RX_In_L(Input,0,                           TwinPeak,0); // correct i2s phase imbalance
    AudioConnection_F32     patchCord_RX_In_R(Input,1,                           TwinPeak,1);
    AudioConnection_F32     patchCord_RX_Ph_L(TwinPeak,0,                        I_Switch,0);  // route raw input audio to the FFT display
    AudioConnection_F32     patchCord_RX_Ph_R(TwinPeak,1,                        Q_Switch,0);
#else
    AudioConnection_F32     patchCord_RX_Ph_L(Input,0,                           I_Switch,0);  // route raw input audio to the FFT display
    AudioConnection_F32     patchCord_RX_Ph_R(Input,1,                           Q_Switch,0);
#endif

// Test tone sources for single or two tone in place of (or in addition to) real input audio
// Mic and Test Tones need to be converted to I and Q

// switch to select inputs.  In DATA mode USB in should be default, mic in voice modes

// Analog mic input
AudioConnection_F32     patchCord_Mic_In(Input,0,                               TX_Source,0);   // Mic source

#ifdef USB32
AudioConnection_F32     patchcord_Mic_InUL(USB_In,0,                            TX_Source,1);
#else
AudioConnection         patchcord_USB_InU(USB_In,0,                             convertL_In,0); 
AudioConnection_F32     patchCord_USB_In(convertL_In,0,                         TX_Source,1);
#endif

AudioConnection_F32     patchCord_Tx_Tone_A(TxTestTone_A,0,                     TX_Source,2);   // Combine mic, tone B and B into L channel
AudioConnection_F32     patchCord_Tx_Tone_B(TxTestTone_B,0,                     TX_Source,3);

//AudioConnection_F32     patchCord_Audio_Filter(TX_Source,0,                     bpf1,0);  // variable filter for TX    
//AudioConnection_F32     patchCord_Audio_Filter_L(bpf1,0,                        FFT_90deg_Hilbert,0);  // variable filter for TX    
//AudioConnection_F32     patchCord_Audio_Filter_R(bpf1,0,                        FFT_90deg_Hilbert,1);  // variable filter for TX
//AudioConnection_F32     patchCord_Feed_L(FFT_90deg_Hilbert,1,                   I_Switch,1); // Feed into normal chain 
//AudioConnection_F32     patchCord_Feed_R(FFT_90deg_Hilbert,0,                   Q_Switch,1); 

//AudioConnection_F32     patchCord_Audio_Filter(TX_Source,0,                     TX_FilterConv,0);  // variable filter for TX    
//AudioConnection_F32     patchCord_Audio_Filter_L(TX_FilterConv,0,               FFT_90deg_Hilbert,0);  // variable filter for TX    
//AudioConnection_F32     patchCord_Audio_Filter_R(TX_FilterConv,0,               FFT_90deg_Hilbert,1);  // variable filter for TX
//AudioConnection_F32     patchCord_Feed_L(FFT_90deg_Hilbert,1,                   I_Switch,1); // Feed into normal chain 
//AudioConnection_F32     patchCord_Feed_R(FFT_90deg_Hilbert,0,                   Q_Switch,1); 

AudioConnection_F32     patchCord_Audio_Filter(TX_Source,0,                     bpf1,0);  // variable filter for TX    
AudioConnection_F32     patchCord_IQ_Mix_L(bpf1,0,                              TX_Hilbert_Plus_45,0); 
AudioConnection_F32     patchCord_IQ_Mix_R(bpf1,0,                              TX_Hilbert_Minus_45,0); 
AudioConnection_F32     patchCord_Feed_L(TX_Hilbert_Minus_45,0,                 I_Switch,1); // Feed into normal chain 
AudioConnection_F32     patchCord_Feed_R(TX_Hilbert_Plus_45,0,                  Q_Switch,1); 

// I_Switch has our selected audio source(s), share with the FFT distribution switch FFT_OutSwitch.  
#if defined (USE_FFT_LO_MIXER)
    //AudioConnection_F32     patchCord_FFT_OUT_L(I_Switch,0,                     FFT_LO_Mixer_I,0);     // Attenuate signals to FFT while in TX mode
    //AudioConnection_F32     patchCord_FFT_OUT_R(Q_Switch,0,                     FFT_LO_Mixer_I,1);
    //AudioConnection_F32     patchCord_LO_Mix_L(FFT_LO_Mixer_I,0,              FFT_90deg_Hilbert,0); // Filter I and Q
    //AudioConnection_F32     patchCord_LO_Mix_R(FFT_LO_Mixer_I,1,              FFT_90deg_Hilbert,1); 
    //AudioConnection_F32     patchCord_LO_Fil_L(FFT_90deg_Hilbert,0,           FFT_Atten_I,0); // Filter I and Q
    //AudioConnection_F32     patchCord_LO_Fil_R(FFT_90deg_Hilbert,1,           FFT_Atten_Q,0);
    AudioConnection_F32     patchCord_LO_90Fil_L(TX_FilterConv,0,               FFT_90deg_Hilbert,0); // Filter I and Q
    AudioConnection_F32     patchCord_LO_90Fil_R(TX_FilterConv,0,               FFT_90deg_Hilbert,1); 
    AudioConnection_F32     patchCord_FFT_OUT_L(FFT_90deg_Hilbert,1,            I_Switch,1);     // Attenuate signals to FFT while in TX mode
    AudioConnection_F32     patchCord_FFT_OUT_R(FFT_90deg_Hilbert,0,            Q_Switch,1);     // Swap I and Q for correct FFT  
    //AudioConnection_F32     patchCord_LO_Fil_L(FFT_LO_Mixer_I,0,                FFT_Atten_I,0); // Filter I and Q
    //AudioConnection_F32     patchCord_LO_Fil_R(FFT_LO_Mixer_I,1,                FFT_Atten_Q,0);
#elif defined(USE_FREQ_SHIFTER)
    AudioConnection_F32     patchCord_FFT_OUT_L(I_Switch,0,                     FFT_SHIFT_I,0);     // Attenuate signals to FFT while in TX mode
    AudioConnection_F32     patchCord_FFT_OUT_R(Q_Switch,0,                     FFT_SHIFT_Q,0);
    AudioConnection_F32     patchCord_FFT_Shift_L(FFT_SHIFT_I,0,                FFT_Atten_I,0); // Filter I and Q
    AudioConnection_F32     patchCord_FFT_Shift_R(FFT_SHIFT_Q,0,                FFT_Atten_Q,0); 
#else
    AudioConnection_F32     patchCord_FFT_OUT_L(I_Switch,0,                     FFT_Atten_I,0);     // Attenuate signals to FFT while in TX mode
    AudioConnection_F32     patchCord_FFT_OUT_R(Q_Switch,0,                     FFT_Atten_Q,0);     // Swap I and Q for correct FFT 
#endif

//AudioConnection_F32     patchCord_LO_Fil_L(I_Switch,0,                      FFT_Atten_I,0); // Filter I and Q
//AudioConnection_F32     patchCord_LO_Fil_R(Q_Switch,0,                      FFT_Atten_Q,0);
AudioConnection_F32     patchCord_FFT_ATT_L(FFT_Atten_I,0,                  FFT_OutSwitch_I,0); // Route selected audio source to the selected FFT - should save CPU time
AudioConnection_F32     patchCord_FFT_ATT_R(FFT_Atten_Q,0,                  FFT_OutSwitch_Q,0);

// One or more of these FFT pipelines can be used, most likely for pan and zoom.  Normally just 1 is used.
#ifdef FFT_4096
    AudioConnection_F32     patchCord_FFT_L_4096(FFT_OutSwitch_I,0,             myFFT_4096,0);   // Route selected audio source to the FFT
    AudioConnection_F32     patchCord_FFT_R_4096(FFT_OutSwitch_Q,0,             myFFT_4096,1);
#endif
#ifdef FFT_2048
    AudioConnection_F32     patchCord_FFT_L_2048(FFT_OutSwitch_I,1,             myFFT_2048,0);        // Route selected audio source to the FFT
    AudioConnection_F32     patchCord_FFT_R_2048(FFT_OutSwitch_Q,1,             myFFT_2048,1);
#endif
#ifdef FFT_1024
    AudioConnection_F32     patchCord_FFT_L_1024(FFT_OutSwitch_I,2,             myFFT_1024,0);        // Route selected audio source to the FFT
    AudioConnection_F32     patchCord_FFT_R_1024(FFT_OutSwitch_Q,2,             myFFT_1024,1);
#endif

// Send selected IQ source(s) to the audio processing chain for demodulation
AudioConnection_F32     patchCord_Input_L(I_Switch,0,                       RxTx_InputSwitch_L,0);  // 0 is RX. Output 1 is Tx chain
AudioConnection_F32     patchCord_Input_R(Q_Switch,0,                       RxTx_InputSwitch_R,0);

// Non-FM path
AudioConnection_F32     patchCord10a(RxTx_InputSwitch_L,0,                  NoiseBlanker,0);
AudioConnection_F32     patchCord10b(RxTx_InputSwitch_R,0,                  NoiseBlanker,1);
AudioConnection_F32     patchCord11a(NoiseBlanker,0,                        RX_Hilbert_Plus_45,0);
AudioConnection_F32     patchCord11b(NoiseBlanker,1,                        RX_Hilbert_Minus_45,0);
// Test dual channel hilbert block 
//AudioConnection_F32     patchCord_IQ_MIX_L(NoiseBlanker,0,                  RX_Hilbert,0);
//AudioConnection_F32     patchCord_IQ_MIX_R(NoiseBlanker,1,                  RX_Hilbert,1);                                            
//AudioConnection_F32     patchCord2c(RX_Hilbert,0,                           RX_Summer,0);  // phase shift +45 deg
//AudioConnection_F32     patchCord2d(RX_Hilbert,1,                           RX_Summer,1);  // phase shift -45 deg
AudioConnection_F32     patchCord2c(RX_Hilbert_Plus_45,0,                   RX_Summer,0);  // phase shift +45 deg
AudioConnection_F32     patchCord2d(RX_Hilbert_Minus_45,0,                  RX_Summer,1);  // phase shift -45 deg
AudioConnection_F32     patchCord2e(Beep_Tone,0,                            RX_Summer,2);  // For button beep if enabled

// Alternate FM Path use non-IQ signal. Only I.  
// Using a test tone for testing
// FM is 10KHz to 20KHz, LO at 15KHz
AudioConnection_F32     patchCord_FM_DET(TxTestTone_B,0,                    FM_LO_Mixer,0);  // Shift tone up to 15KHz
AudioConnection_F32     patchCord_FM_Mix_LO(FM_LO_Mixer,0,                  FM_Detector,0);
AudioConnection_F32     patchCord_FM_Mix_Det(FM_Detector,0,                 OutputSwitch_I,2);
AudioConnection_F32     patchCord_FM_Mix_Out(FM_Detector,0,                 OutputSwitch_Q,2);

// Post mixer processing (now treated as mono audio)
AudioConnection_F32     patchCord_Summer_Peak(RX_Summer,0,                  S_Peak,0);      // S meter source
AudioConnection_F32     patchCord_Summer_Notch(RX_Summer,0,                 LMS_Notch,0);   // NR and Notch
AudioConnection_F32     patchCord_Notch(LMS_Notch,0,                        RX_FilterConv,0);  // variable bandwidth filter
AudioConnection_F32     patchCord_RxOut_L(RX_FilterConv,0,                  OutputSwitch_I,0);  // demod and filtering complete
AudioConnection_F32     patchCord_RxOut_R(RX_FilterConv,0,                  OutputSwitch_Q,0);  

// In TX the mic source is selected in FFT_Mixer and was phase shifted so just passed
AudioConnection_F32     patchCord_Mic_Input_L(RxTx_InputSwitch_R,1,         OutputSwitch_I,1);  // phase shift mono source 90 degrees
AudioConnection_F32     patchCord_Mic_Input_R(RxTx_InputSwitch_L,1,         OutputSwitch_Q,1);  // Using L source twice since mic source is mono

// Selected source goes to output (selected as headphone or lineout in the code) and boosted if needed
AudioConnection_F32     patchCord_Output_L(OutputSwitch_I,0,                Output,0);  // output to headphone jack/Line out Left
AudioConnection_F32     patchCord_Output_R(OutputSwitch_Q,0,                Output,1);  // output to headphone jack/Line out Right
AudioConnection_F32     patchCord_Amp1_L(OutputSwitch_I,0,                  Amp1_L,0);  // output audio to USB, line out
AudioConnection_F32     patchCord_Amp1_R(OutputSwitch_Q,0,                  Amp1_R,0);  // output audio to USB, line out

#ifdef USB32
//AudioConnection_F32     patchcord_Out_USB_L(Amp1_L,0,                       USB_Out,0);  // output to USB Audio Out L
//AudioConnection_F32     patchcord_Out_USB_R(Amp1_R,0,                       USB_Out,1);  // output to USB Audio Out R
AudioConnection_F32     patchcord_Out_USB_L(USB_In,0,                       USB_Out,0);  // output to USB Audio Out L
AudioConnection_F32     patchcord_Out_USB_R(USB_In,1,                       USB_Out,1);  // output to USB Audio Out R
#else
AudioConnection_F32     patchcord_Out_L32(Amp1_L,0,                         convertL_Out,0);  // output to headphone jack Right
AudioConnection_F32     patchcord_Out_R32(Amp1_R,0,                         convertR_Out,0);  // output to headphone jack Right
AudioConnection         patchcord_Out_L16U(convertL_Out,0,                  USB_Out,0);  // output to headphone jack Right
AudioConnection         patchcord_Out_R16U(convertR_Out,0,                  USB_Out,1);  // output to headphone jack Right
//AudioConnection         patchcord_Out_USB_L16U(USB_In,0,                       USB_Out,0);  // output to USB Audio Out L
//AudioConnection         patchcord_Out_USB_R16U(USB_In,1,                       USB_Out,1);  // output to USB Audio Out R

#endif

AudioControlSGTL5000    codec1;
//AudioControlWM8960    codec1;   // Does not work yet, hangs

// -------------------------------------Setup() -------------------------------------------------------------------
// 
tmElements_t tm;
time_t prevDisplay = 0; // When the digital clock was displayed

COLD void setup()
{
    pinMode(PTT_INPUT, INPUT_PULLUP);   // Init PTT in and out lines
    pinMode(PTT_OUT1, OUTPUT);
    digitalWrite(PTT_OUT1, HIGH);
    DSERIALBEGIN(115200);
    delay(500);
    DPRINTLN(F("Initializing SDR_RA887x Program"));
    DPRINT(F("FFT Size is "));
    DPRINTLN(fft_size);
    DPRINTLN(F("**** Running I2C Scanner ****"));
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

    #ifdef MECH_ENCODERS
        pinMode(ENC2_PIN_SW, INPUT_PULLUP);   // Pullups for Enc2 and 3 switches
        pinMode(ENC3_PIN_SW, INPUT_PULLUP);
    #endif

    #ifdef SV1AFN_BPF
        // set address to 0x20 (32 decimal) for V2.X adafruit MCP23017 library. 
        // A value of 0 now kills the I2C bus.
        bpf.begin((int) 32, (TwoWire*) &Wire);
        bpf.setBand(HFNone);
        bpf.setPreamp(false);
        bpf.setAttenuator(false);
    #endif

    #ifdef USE_RA8875
        DPRINTLN(F("Initializing RA8875 Display"));
        tft.begin(RA8875_800x480);
        tft.setRotation(SCREEN_ROTATION); // 0 is normal, 1 is 90, 2 is 180, 3 is 270 degrees
        #ifdef USE_FT5206_TOUCH
            tft.useCapINT(RA8875_INT);
            tft.setTouchLimit(MAXTOUCHLIMIT);
            tft.enableCapISR(true);
            tft.setTextColor(RA8875_WHITE, RA8875_BLACK);
        #else
            #ifdef USE_RA8875
                //tft.print("you should open RA8875UserSettings.h file and uncomment USE_FT5206_TOUCH!");
            #endif  // USE_RA8875
        #endif // USE_FT5206_TOUCH
    #else 
        DPRINTLN(F("Initializing RA8876 Display"));   
        tft.begin(30000000UL);
        cts.begin();
        cts.setTouchLimit(MAXTOUCHLIMIT);
        tft.touchEnable(false);   // Ensure the resitive controller, if any is off
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
        setActiveWindow_default();
#endif        
        tft.graphicMode(true);
        tft.clearActiveScreen();
        tft.selectScreen(0);  // Select screen page 0
        tft.fillScreen(BLACK);
        tft.setBackGroundColor(BLACK);
        tft.setTextColor(WHITE, BLACK);
        tft.backlight(true);
        tft.displayOn(true);
        tft.setRotation(SCREEN_ROTATION); // 0 is normal, 1 is 90, 2 is 180, 3 is 270 degrees.  
                        // RA8876 touch controller is upside down compared to the RA8875 so correcting for it there.
    #endif

    // Display Startup Banner
    tft.setFont(Arial_28_Bold);
    tft.setTextColor(BLUE);
    tft.setCursor(70, 100);
    tft.print(F(BANNER));   // Customize the Startup Banner Text
    tft.setCursor(140, 200);
    tft.setFont(Arial_28_Bold);
    tft.setTextColor(WHITE);
    tft.print(F(CALLSIGN));   // Put your callsign here

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
            MSG_DPRINTLN(F("Ethernet System Startup Failed, setting retry timer (10 minutes)"));
        }
        DPRINTLN(F("Ethernet System Startup Completed"));
        //setSyncProvider(getNtpTime);
    }
    #endif

    // Update time on startup from RTC. If a USB connection is up, get the time from a PC.  
    // Later if enet is up, get time from NTP periodically.
    setSyncProvider(getTeensy3Time);   // the function to get the time from the RTC
    if(timeStatus()!= timeSet) // try this other way
        DPRINTLN(F("Unable to sync with the RTC"));
    else
        DPRINTLN(F("RTC has set the system time")); 
   
    #ifdef DEBUG
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
    DPRINTLN(F("Clock Update"));
    #endif
    

#ifndef BYPASS_SPECTRUM_MODULE
    initSpectrum(user_settings[user_Profile].sp_preset); // Call before initDisplay() to put screen into Layer 1 mode before any other text is drawn!
#endif

    #ifdef PE4302
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
    PAN(0);

    //==================================== Frequency Set ==========================================
    #ifdef PANADAPTER
    VFOA = PANADAPTER_LO;
    VFOB = PANADAPTER_LO;
    #else
    VFOA = bandmem[curr_band].vfo_A_last; //I used 7850000  frequency CHU  Time Signal Canada
    VFOB = user_settings[user_Profile].sub_VFO;
    #endif
                               
    #ifdef USE_RS_HFIQ  // if RS-HFIQ is used, then send the active VFO frequency and receive the (possibly) updated VFO
        DPRINTLN(F("Initializing RS-HFIQ Radio via USB Host port"));
        #ifndef NO_RSHFIQ_BLOCKING
            RS_HFIQ.setup_RSHFIQ(1, VFOA);  // 0 is non blocking wait, 1 is blocking wait.  Pass active VFO frequency
        #else
            RS_HFIQ.setup_RSHFIQ(0, VFOA);  // 0 is non blocking wait, 1 is blocking wait.  Pass active VFO frequency
        #endif
        #ifdef USE_RA8875
            tft.clearScreen();
        #else
            tft.clearActiveScreen();
        #endif    
    #endif
        

#ifndef BYPASS_SPECTRUM_MODULE    
    Spectrum_Parm_Generator(0, 0, fft_bins);  // use this to generate new set of params for the current window size values. 
                                                              // 1st arg is new target layout record - usually 0 unless you create more examples
                                                              // 2nd arg is current empty layout record (preset) value - usually 0
                                                              // calling generator before drawSpectrum() will create a new set of values based on the globals
                                                              // Generator only reads the global values, it does not change them or the database, just prints the new params                                                             
    drawSpectrumFrame(user_settings[user_Profile].sp_preset); // Call after initSpectrum() to draw the spectrum object.  Arg is 0 PRESETS to load a preset record
                                                              // DrawSpectrum does not read the globals but does update them to match the current preset.
                                                              // Therefore always call the generator before drawSpectrum() to create a new set of params you can cut anmd paste.
                                                              // Generator never modifies the globals so never affects the layout itself.
                                                              // Print out our starting frequency for testing
    //sp.drawSpectrumFrame(6);   // for 2nd window
#endif  

    DPRINT(F("\nInitial Dial Frequency is "));
    DPRINT(formatVFO(VFOA));
    DPRINTLN(F("MHz"));

    //--------------------------   Setup our Audio System -------------------------------------
    initVfo(); // initialize the si5351 vfo
    delay(10);
    initDSP();
    //RFgain(0);
    changeBands(0);     // Sets the VFOs to last used frequencies, sets preselector, active VFO, other last-used settings per band.
                        // Call changeBands() here after volume to get proper startup volume

    //------------------Finish the setup by printing the help menu to the serial connections--------------------
    #ifdef DEBUG
    printHelp();
    #endif
    InternalTemperature.begin(TEMPERATURE_NO_ADC_SETTING_CHANGES);
   
    #ifdef FT817_CAT
        DPRINTLN(F("Starting the CAT port and reading some radio information if available"));
        init_CAT_comms();  // initialize the CAT port
        print_CAT_status();  // Test Line to read data from FT-817 if attached.
    #endif
    #ifdef ALL_CAT
        CAT_setup();   // Setup the MSG_Serial port for cnfigured Radio comm port
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
    static uint32_t time_n  = 0;
    #ifdef DEBUG
    static uint32_t time_sp;
    #endif

    #ifndef BYPASS_SPECTRUM_MODULE
        // Update spectrum and waterfall based on timer - do not draw in the screen space while the pop up has the screen focus.
        //if (spectrum_waterfall_update.check() == 1 && !popup) // The update rate is set in drawSpectrumFrame() with spect_wf_rate from table
        if (!popup) // The update rate is set in drawSpectrumFrame() with spect_wf_rate from table
        {    
            #ifdef DEBUG  
            time_sp = millis();
            #endif     
        //if (!bandmem[curr_band].XIT_en)  // TEST:  added to test CPU impact
            Freq_Peak = spectrum_update(
                user_settings[user_Profile].sp_preset,
                1,  // No longer used, se tto 1
                VFOA,               // for onscreen freq info
                VFOB,               // Not really needed today
                ModeOffset,         // Move spectrum cursor to center or offset it by pitch value when in CW modes
                filterCenter,       // Center the on screen filter shaded area
                filterBandwidth,    // Display the filter width on screen
                pan,                // Pannng offset from center frequency
                fft_size,           // use this size to display for simple zoom effect
                fft_bin_size,       // pass along the calculated bin size
                fft_bins            // pass along the number of bins.  FOr IQ FFTs, this is fft_size, else fft_size/2
                ); // valid numbers are 0 through PRESETS to index the record of predefined window layouts
            // spectrum_update(6);  // for 2nd window
        }
    #endif

    time_n = millis() - time_old;
    
    if (time_n > delta)
    {
        delta = time_n;
        DPRINT(F("Loop T="));
        DPRINT(delta);
        DPRINT(F("  Spectrum T="));
        DPRINTLN(millis() - time_sp);
    }
    time_old = millis();

    //if (touch.check() == 1)
    //{
        Touch(); // touch points and gestures
    //}

    if (!popup && tuner.check() == 1 && newFreq < enc_ppr_response) // dump counts accumulated over time but < minimum for a step to count.
    {
        VFO.readAndReset();
        //VFO.read();
        newFreq = 0;
    }

    if (!popup)
        newFreq += VFO.read();    // faster to poll for change since last read
                              // accumulate counts until we have enough to act on for scaling factor to work right.
    if (!popup && newFreq != 0 && abs(newFreq) > enc_ppr_response) // newFreq is a positive or negative number of counts since last read.
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
            //DPRINT(F("Switching to Default MF knob assignment, current owner is = "));
            //DPRINTLN(MF_client);
            set_MF_Service(user_settings[user_Profile].default_MF_client);  // will turn off the button, if any, and set the default as new owner.
            MF_default_is_active = true;
        }
    }

    #if defined I2C_ENCODERS || defined MECH_ENCODERS
        Check_Encoders();
    #endif

    if (!popup)
        Check_PTT();

    if (!popup && meter.check() == 1) // update our meters
    {
        //if(!bandmem[curr_band].XIT_en)
            S_Meter_Peak_Avg = Peak();   // return an average for RF AGC limiter if used
        //DPRINT("S-Meter Peak Avg = ");
        //DPRINTLN(S_Meter_Peak_Avg);
    }

    //RF_Limiter(S_Meter_Peak_Avg);  // reduce LineIn gain temprarily until below max level.  Uses the average to restore level
    
    #ifdef PANADAPTER
        #ifdef ALL_CAT
            //if (CAT_update.check() == 1) // update our meters
            //{
                // update Panadapter CAT port data using same time  
                if (!popup)
                    CAT_handler();
            //}
        #endif  // ALL_CAT
    #endif // PANADAPTER

    if (popup_timer.check() == 1 && popup) // stop spectrum updates, clear the screen and post up a keyboard or something
    {
        // timeout the active window
        pop_win_down(BAND_MENU);
        Band(255);
    }
    
    // The timer and flag are set by the rogerBeep() function
    if (touchBeep_flag && touchBeep_timer.check() == 1)   
    {
        touchBeep(false);    
    }

    //respond to MSG_Serial commands
    #ifdef DEBUG
    while (!popup && Serial.available())
    {
        //char ch = (Serial.peek());
        char ch = (Serial.read());
        ch = toupper(ch);
        DPRINTLN(ch);
        switch (ch)
        {
            case 'C':
            case 'H':   //respondToByte((char)MSG_Serial.read()); 
                        respondToByte((char)ch); 
                        break;
            case 'T':   {
                            time_t t = processSyncMessage();
                            if (t != 0) 
                            {
                                DPRINTLN(F("Time Update"));
                                Teensy3Clock.set(t); // set the RTC
                                setTime(t);
                                digitalClockDisplay();
                            }
                        }
                        break;
            default:
                        break;  
        }
    }
    
    //check to see whether to print the CPU and Memory Usage
    if (enable_printCPUandMemory)
        printCPUandMemory(millis(), 3000); //print every 3000 msec
    #endif

#ifdef USE_RS_HFIQ
        RS_HFIQ_Service();
#endif
                            
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
#endif // End of Ethernet related functions here

    // Check if the time has updated (1 second) and update the clock display
    if (timeStatus() != timeNotSet) // && enet_ready) // Only display if ethernet is active and have a valid time source
    {
        if (now() != prevDisplay)
        {
            //update the display only if time has changed
            prevDisplay = now();
//if(!bandmem[curr_band].XIT_en)
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
    //DPRINT("PTT State "); DPRINTLN(PTT_pin_state);
    // Start debounce timer if a new pin change of state detected
    if (PTT_pin_state != last_PTT_Input)   // user_settings[user_Profile].xmit == OFF)
    {   
        //DPRINT("PTT Changed State "); DPRINTLN(PTT_pin_state);
        if (!PTT_Input_debounce)
        {
            //DPRINTLN("Start PTT Settle Timer");
            PTT_Input_debounce = 1;     // Start debounce timer
            PTT_Input_time = millis();  // Start of transition
            last_PTT_Input = PTT_pin_state;
        }
    }
    // Debounce timer in progress?  If so exit.
    if (PTT_Input_debounce && ((PTT_Input_time - millis()) < 30)) 
    {
        //DPRINTLN("Waiting for PTT Settle Time");
        return;
    }
    // If the timer is satisfied, change the TX/RX state
    if (PTT_Input_debounce)
    {
        PTT_Input_debounce= 0;     // Reset timer for next change
        if (!last_PTT_Input)  // if TX
        {
            DPRINTLN("\nPTT TX Detected");
            Xmit(1);  // transmit state
        }
        else // if RX
        {
            DPRINTLN("\nPTT TX Released");
            Xmit(0);  // receive state
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
    //DPRINTLN(rampName[rampType]);

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

#ifdef DEBUG
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
        DPRINT(F("\nCPU Cur/Peak: "));
        DPRINT(audio_settings.processorUsage());
        DPRINT(F("%/"));
        DPRINT(audio_settings.processorUsageMax());
        DPRINTLN(F("%"));
        DPRINT(F("CPU Temperature:"));
        DPRINT(InternalTemperature.readTemperatureF(), 1);
        DPRINT(F("F "));
        DPRINT(InternalTemperature.readTemperatureC(), 1);
        DPRINTLN(F("C"));
        DPRINT(F(" Audio MEM Float32 Cur/Peak: "));
        DPRINT(AudioMemoryUsage_F32());
        DPRINT(F("/"));
        DPRINTLN(AudioMemoryUsageMax_F32());
        DPRINT(F(" Audio MEM 16-Bit  Cur/Peak: "));
        DPRINT(AudioMemoryUsage());
        DPRINT(F("/"));
        DPRINTLN(AudioMemoryUsageMax());
        DPRINTLN(F("*** End of Report ***"));

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
    #ifdef DEBUG
    char s[2];
    #endif
    s[0] = c;
    s[1] = 0;
    if (!isalpha((int)c) && c != '?' && c != '*')
        return;
    switch (c)
    {
    case 'h':
    case 'H':
    case '?':
        printHelp();
        break;
    case 'C':
    case 'c':
        DPRINTLN(F("Toggle printing of memory and CPU usage."));
        togglePrintMemoryAndCPU();
        break;
    default:
        DPRINT(F("You typed "));
        DPRINT(s);
        DPRINTLN(F(".  What command?"));
    }
}
//
// _______________________________________ Print Help Menu ____________________________________
//
COLD void printHelp(void)
{
    DPRINTLN();
    DPRINTLN(F("Help: Available Commands:"));
    DPRINTLN(F("   h: Print this help"));
    DPRINTLN(F("   C: Toggle printing of CPU and Memory usage"));
    DPRINTLN(F("   T+10 digits: Time Update. Enter T and 10 digits for seconds since 1/1/1970"));
    //#ifdef USE_RS_HFIQ
      //DPRINTLN(F("   R to display the RS-HFIQ Menu"));
    //#endif
}
#endif

#ifdef MECH_ENCODERS
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
        Multi.readAndReset(); // toss results, zero the encoder
        mf_count = 0;
    }
    else
    {
        mf_count = Multi.readAndReset(); // read and reset the Multifunction knob.  Apply results to any in-focus function, if any
    }
    return mf_count;
}
#endif  //MECH_ENCODERS


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
        case MFNONE:                            break;  // no current owner, return
        case RFGAIN_BTN:    setRFgain(-1);      break;  // since it was active toggle the output off
        case AFGAIN_BTN:    setAFgain(-1);      break;
        case REFLVL_BTN:    setRefLevel(-1);    break;
        case PAN_BTN:       setPAN(-1);         break;
        case ATTEN_BTN:     setAtten(-1);       break;
        case NB_BTN:        setNB(-1);          break;
        case MFTUNE:
        default     :                           break;  // No button for VFO tune, atten button stays on
                                                        //MF_client = user_settings[user_Profile].default_MF_client;
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
    #ifdef MECH_ENCODERS   // The I2c encoders are using interrupt driven callback functions so no need to call them, they will call us.
        multiKnob(1);       // for non-I2C encoder, clear the counts.
    #endif
    unset_MF_Service(MF_client);    //  turn off current button if on
    MF_client = new_client_name;        // Now assign new owner
    MF_Timeout.reset();  // reset (extend) timeout timer as long as there is activity.  
                         // When it expires it will be switched to default
    //DPRINT("New MF Knob Client ID is ");
    //DPRINTLN(MF_client);
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

    //DPRINT("MF Knob Client ID is "); DPRINTLN(MF_client);

    switch (knob)      // Give this owner control until timeout
    {
        case MFNONE:                                break; // no current owner, return
        case RFGAIN_BTN:    RFgain(counts);         break;
        case AFGAIN_BTN:    AFgain(counts);         break;
        case REFLVL_BTN:    RefLevel(counts*-1);    break;
        case PAN_BTN:       PAN(counts);            break;
        case ATTEN_BTN:     Atten(counts);          break;  // set attenuator level to value in database for this band
        case NB_BTN:        NBLevel(counts);        break;
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
  
  DPRINTLN(F("Scanning..."));

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    //Wire1.beginTransmission(address);
    error = Wire.endTransmission();
    //error1 = Wire1.endTransmission();

    if (error == 0)
    {
      DPRINT(F("I2C device found at address 0x"));
      if (address < 16)
        DPRINT("0");
      DPRINT(address, HEX);
      DPRINT("  (");
      printKnownChips(address);
      DPRINTLN(")");
      //DPRINTLN("  !");
      nDevices++;
    }
    else if (error == 4)
    {
      DPRINT(F("Unknown error at address 0x"));
      if (address < 16)
        DPRINT("0");
      DPRINTLN(address, HEX);
    }
  }
  if (nDevices == 0)
    DPRINTLN(F("No I2C devices found\n"));
  else
    DPRINTLN(F("done\n"));

  //delay(500); // wait 5 seconds for the next I2C scan
}

// prints a list of known i2C devices that match a discovered address
COLD void printKnownChips(byte address)
{
  // Is this list missing part numbers for chips you use?
  // Please suggest additions here:
  // https://github.com/PaulStoffregen/Wire/issues/new
  switch (address) {
    case 0x00: DPRINT(F("AS3935")); break;
    case 0x01: DPRINT(F("AS3935")); break;
    case 0x02: DPRINT(F("AS3935")); break;
    case 0x03: DPRINT(F("AS3935")); break;
    case 0x0A: DPRINT(F("SGTL5000")); break; // MCLK required
    case 0x0B: DPRINT(F("SMBusBattery?")); break;
    case 0x0C: DPRINT(F("AK8963")); break;
    case 0x10: DPRINT(F("CS4272")); break;
    case 0x11: DPRINT(F("Si4713")); break;
    case 0x13: DPRINT(F("VCNL4000,AK4558")); break;
    case 0x18: DPRINT(F("LIS331DLH")); break;
    case 0x19: DPRINT(F("LSM303,LIS331DLH")); break;
    case 0x1A: DPRINT(F("WM8731, WM8960")); break;
    case 0x1C: DPRINT(F("LIS3MDL")); break;
    case 0x1D: DPRINT(F("LSM303D,LSM9DS0,ADXL345,MMA7455L,LSM9DS1,LIS3DSH")); break;
    case 0x1E: DPRINT(F("LSM303D,HMC5883L,FXOS8700,LIS3DSH")); break;
    case 0x20: DPRINT(F("MCP23017,MCP23008,PCF8574,FXAS21002,SoilMoisture")); break;
    case 0x21: DPRINT(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x22: DPRINT(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x23: DPRINT(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x24: DPRINT(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x25: DPRINT(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x26: DPRINT(F("MCP23017,MCP23008,PCF8574")); break;
    case 0x27: DPRINT(F("MCP23017,MCP23008,PCF8574,LCD16x2,DigoleDisplay")); break;
    case 0x28: DPRINT(F("BNO055,EM7180,CAP1188")); break;
    case 0x29: DPRINT(F("TSL2561,VL6180,TSL2561,TSL2591,BNO055,CAP1188")); break;
    case 0x2A: DPRINT(F("SGTL5000,CAP1188")); break;
    case 0x2B: DPRINT(F("CAP1188")); break;
    case 0x2C: DPRINT(F("MCP44XX ePot")); break;
    case 0x2D: DPRINT(F("MCP44XX ePot")); break;
    case 0x2E: DPRINT(F("MCP44XX ePot")); break;
    case 0x2F: DPRINT(F("MCP44XX ePot")); break;
    case 0x33: DPRINT(F("MAX11614,MAX11615")); break;
    case 0x34: DPRINT(F("MAX11612,MAX11613")); break;
    case 0x35: DPRINT(F("MAX11616,MAX11617")); break;
    case 0x38: DPRINT(F("RA8875,FT6206")); break;
    case 0x39: DPRINT(F("TSL2561, APDS9960")); break;
    case 0x3C: DPRINT(F("SSD1306,DigisparkOLED")); break;
    case 0x3D: DPRINT(F("SSD1306")); break;
    case 0x40: DPRINT(F("PCA9685,Si7021")); break;
    case 0x41: DPRINT(F("STMPE610,PCA9685")); break;
    case 0x42: DPRINT(F("PCA9685")); break;
    case 0x43: DPRINT(F("PCA9685")); break;
    case 0x44: DPRINT(F("PCA9685, SHT3X")); break;
    case 0x45: DPRINT(F("PCA9685, SHT3X")); break;
    case 0x46: DPRINT(F("PCA9685")); break;
    case 0x47: DPRINT(F("PCA9685")); break;
    case 0x48: DPRINT(F("ADS1115,PN532,TMP102,PCF8591")); break;
    case 0x49: DPRINT(F("ADS1115,TSL2561,PCF8591")); break;
    case 0x4A: DPRINT(F("ADS1115")); break;
    case 0x4B: DPRINT(F("ADS1115,TMP102")); break;
    case 0x50: DPRINT(F("EEPROM")); break;
    case 0x51: DPRINT(F("EEPROM")); break;
    case 0x52: DPRINT(F("Nunchuk,EEPROM")); break;
    case 0x53: DPRINT(F("ADXL345,EEPROM")); break;
    case 0x54: DPRINT(F("EEPROM")); break;
    case 0x55: DPRINT(F("EEPROM")); break;
    case 0x56: DPRINT(F("EEPROM")); break;
    case 0x57: DPRINT(F("EEPROM")); break;
    case 0x58: DPRINT(F("TPA2016,MAX21100")); break;
    case 0x5A: DPRINT(F("MPR121")); break;
    case 0x60: DPRINT(F("MPL3115,MCP4725,MCP4728,TEA5767,Si5351")); break;
    case 0x61: DPRINT(F("MCP4725,AtlasEzoDO,DuPPaEncoder")); break;
    case 0x62: DPRINT(F("LidarLite,MCP4725,AtlasEzoORP,DuPPaEncoder")); break;
    case 0x63: DPRINT(F("MCP4725,AtlasEzoPH,DuPPaEncoder")); break;
    case 0x64: DPRINT(F("AtlasEzoEC,DuPPaEncoder")); break;
    case 0x66: DPRINT(F("AtlasEzoRTD,DuPPaEncoder")); break;
    case 0x67: DPRINT(F("DuPPaEncoder")); break;
    case 0x68: DPRINT(F("DS1307,DS3231,MPU6050,MPU9050,MPU9250,ITG3200,ITG3701,LSM9DS0,L3G4200D,DuPPaEncoder")); break;
    case 0x69: DPRINT(F("MPU6050,MPU9050,MPU9250,ITG3701,L3G4200D")); break;
    case 0x6A: DPRINT(F("LSM9DS1")); break;
    case 0x6B: DPRINT(F("LSM9DS0")); break;
    case 0x70: DPRINT(F("HT16K33")); break;
    case 0x71: DPRINT(F("SFE7SEG,HT16K33")); break;
    case 0x72: DPRINT(F("HT16K33")); break;
    case 0x73: DPRINT(F("HT16K33")); break;
    case 0x76: DPRINT(F("MS5607,MS5611,MS5637,BMP280")); break;
    case 0x77: DPRINT(F("BMP085,BMA180,BMP280,MS5611")); break;
    default: DPRINT(F("unknown chip"));
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
  DPRINT(":");
  if(digits < 10)
    DPRINT('0');
  DPRINT(digits);
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
        //DPRINTLN(pctime);
        if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
            pctime = 0L; // return 0 to indicate that the time is not valid
        }
    }
    return pctime;  // return will be seconds since jan 1 1970.
}

COLD void digitalClockDisplay() {
  // digital clock display of the time
  DPRINT(hour());
  printDigits(minute());
  printDigits(second());
  DPRINT(" ");
  DPRINT(day());
  DPRINT(" ");
  DPRINT(month());
  DPRINT(" ");
  DPRINT(year()); 
  DPRINTLN(); 
}

COLD void SetFilter(void)
{
    RX_FilterConv.initFilter((float32_t)filterCenter, 90, 2, filterBandwidth);
}

COLD void initDSP(void)
{
    AudioMemory(10);  // Does not look like we need this anymore when using all F32 functions?
    AudioMemory_F32(150, audio_settings);   // 4096IQ FFT needs about 75 or 80 at 96KHz sample rate
    resetCodec();
    delay(50);  // Sometimes a delay avoids a Twin Peaks problem.
}

// initDSP() and startup in RX mode enables our resources.  
// This function switches input sources between line in and mic in and Test Tones (A and B),
//   then set levels and retores them on RX.
// RX sends audio to the headphone only (LineOut muted), and TX audio will go to LineOut (headphone muted). 
// 100% output results in 3Vp-p at LineOut.  Power control varies 0 to 100% of 3V (LineOutLevel => 31)  
// Also controls test tone state.  TX often uses two tones at high level, RX single tone at low level.  
COLD void TX_RX_Switch(
        bool    TX,           // TX = ON, RX = OFF
        uint8_t mode_sel,       // Current VFO mode index
        bool    b_Mic_On,       // Turn Mic source on or off
        bool    b_USBIn_On,     // Turn on USB input source (typically for Data modes)
        bool    b_ToneA,        // Test Tone A
        bool    b_ToneB,        // Test Tone B
        float   TestTone_Vol)   // 0.90 is max, clips if higher. Use 0.45f with 2 tones
{
    float   Mic_On;         // 0.0f(OFF) or 1.0f (ON)
    float   USBIn_On;       // 0.0f(OFF) or 1.0f (ON)
    float   ToneA;          // 0.0f(OFF) or 1.0f (ON)
    float   ToneB;          // 0.0f(OFF) or 1.0f (ON)
    float   ch_on  = 1.0f;    
    float   ch_off = 0.0f;

    // Covert bool to floats
    if (b_Mic_On)   Mic_On   = 2.0f; else Mic_On   = ch_off;
    if (b_USBIn_On) USBIn_On = 1.0f; else USBIn_On = ch_off;
    if (b_ToneA)    ToneA    = ch_on; else  ToneA   = ch_off;
    if (b_ToneB)    ToneB    = ch_on; else  ToneB   = ch_off;

    TxTestTone_A.amplitude(TestTone_Vol);
    TxTestTone_A.frequency(700.0f); // for some reason this is doubled but Tone B is not.   Also getting mirror image.
    TxTestTone_B.amplitude(TestTone_Vol); //
    TxTestTone_B.frequency(1900.0f); 

    // Select Mic (0), USBIn(1), Tone A(2), and/or Tone B (3) in any combo.
    TX_Source.gain(0, Mic_On); //  Mono Mic audio
    TX_Source.gain(1, USBIn_On); //  Test Tone B
    TX_Source.gain(2, ToneA); //  Test Tone A   - Use 0.5f with 2 tones, or 1.0f with 1 tone
    TX_Source.gain(3, ToneB); //  Test Tone B

    // use mode to control sideband switching
    float invert;
    switch (mode_sel)
    {
        case CW:
        case DATA:
        case AM:
        case FM: FM_Detector.setSquelchThreshold(0.7f);
        case USB:
            invert = 1.0f; 
            break;
        default: // all other modes flip sideband
            invert = -1.0f;
            break;
    }
    
    if (TX)  // Switch to Mic input on TX
    {
        DPRINTLN(F("Switching to Tx")); 

        AudioNoInterrupts();

        codec1.inputSelect(MicAudioIn);   // Mic is microphone, Line-In is from Receiver audio        
        codec1.muteHeadphone(); 
        codec1.audioProcessorDisable();   // Default 

        // TX so Turn ON
        // Select converted IQ source (mic/test tones) or IQ Stereo LineIn (RX)
        I_Switch.gain(0, ch_off);       // Ch 0 is LineIn I  - RX so shut off
        Q_Switch.gain(0, ch_off);       // Ch 0 is LineIn Q
        I_Switch.gain(1, ch_on);        // Ch 1 is test tone and Mic I - 
        Q_Switch.gain(1, invert);       // Ch 1 is test tone and Mic Q  apply -1 here for sideband invert

        FFT_Atten_I.gain(0, 0.000000001f);    // Attenuate signals to FFT while in TX
        FFT_Atten_Q.gain(0, 0.000000001f);  

        // On switch back to RX the setMode() function on RX will restore this to RX path.
        RxTx_InputSwitch_L.setChannel(1); // Route audio to TX path (1)/
        RxTx_InputSwitch_R.setChannel(1); // Route audio to TX path (1)

//  ----------------------------------------------------------------
//
//  In this space any extra modulation specific blocks will be added
//
//  ----------------------------------------------------------------

        // Connect modulated TX to Lineout Amplifier
        OutputSwitch_I.gain(0, ch_off);     // Turn RX OFF (ch 0 to 0.0)
        OutputSwitch_Q.gain(0, ch_off);     // Turn RX OFF  
        OutputSwitch_I.gain(1, ch_on);      // Turn TX ON (ch 1 to 1.0f)
        OutputSwitch_Q.gain(1, ch_on);      // Turn TX ON   
        OutputSwitch_I.gain(2, ch_off);     // Turn ON for FM   ToDO: automate this based on mode
        OutputSwitch_Q.gain(2, ch_off);     // Turn ON for FM       

        Amp1_L.setGain(0.0f);    // Mute output to USB during TX
        Amp1_R.setGain(0.0f);   
        codec1.lineInLevel(0);      // 0 in LineIn avoids an interaction observed with o'scope on Lineout.
        //TX_FilterConv.initFilter((float32_t)TX_filterCenter, 90, 2, TX_filterBandwidth);

        AudioInterrupts();

        RampVolume(ch_on, 0);        // Instant off.  0 to 1.0f for full scale.
        codec1.unmuteLineout();           // Audio out to Line-Out and TX board        
        AFgain(0);  // sets up the Lineout level for TX testing
    }
    else 
    //  *******************************************************************************************
    //   Back to RX
    //  *******************************************************************************************
    {
        DPRINTLN(F("Switching to Rx"));

        AudioNoInterrupts();

        codec1.muteLineout(); //mute the TX audio output to transmitter input 
        codec1.inputSelect(RxAudioIn);  // switch back to RX audio input

// FM RX test only
        if (mode_sel == FM)
        {
            TxTestTone_B.amplitude(0.2f); //
            TxTestTone_B.frequency(15500.0f);
        }

        // Typically choose one pair, Ch 0, 1 or 2.
        // Use RFGain info to help give more range to adjustment then just LineIn.
        //I_Switch.gain(0, (float) user_settings[user_Profile].rfGain/100); //  1 is RX, 0 is TX
        //Q_Switch.gain(0, (float) user_settings[user_Profile].rfGain/100); //  1 is RX, 0 is TX

        // Select converted IQ source (mic/test tones) or IQ Stereo LineIn (RX)
        I_Switch.gain(0, ch_on); //  1 is LineIn
        Q_Switch.gain(0, ch_on); //  1 is LineIn
        I_Switch.gain(1, ch_off); // Ch 2 is test tone and Mic
        Q_Switch.gain(1, ch_off); // Ch 2 is test tone and Mic

        FFT_Atten_I.gain(0, ch_on);    // Turn off TX Attenuation to FFT while in RX (pass through)
        FFT_Atten_Q.gain(0, ch_on);  

//-----------------------------------------------------------------------------------------------        
// Demodulation specific blocks will be done at end of this by setMode, and other control functions
// This RX/TX is about generic switching and should avoid mode details where possible 
//-----------------------------------------------------------------------------------------------

        // Connect demodulated RX to Lineout Amplifier   Non FM choose Ch 0.  Td is Ch 2
        if (mode_sel != FM)
        {
            OutputSwitch_I.gain(0, ch_on);  // Ch 0 is RX, Turn on
            OutputSwitch_Q.gain(0, ch_on);  // Ch 0 is RX, Turn on
            OutputSwitch_I.gain(1, ch_off); // Turn TX off
            OutputSwitch_Q.gain(1, ch_off); // Turn TX off 
            OutputSwitch_I.gain(2, ch_off); // Turn ON for FM   ToDO: automate this based on mode
            OutputSwitch_Q.gain(2, ch_off); // Turn ON for FM
        }
        else  // FM choose output ch 2
        {

            OutputSwitch_I.gain(0, ch_off);  // Ch 0 is RX, Turn on
            OutputSwitch_Q.gain(0, ch_off);  // Ch 0 is RX, Turn on
            OutputSwitch_I.gain(1, ch_off); // Turn TX off
            OutputSwitch_Q.gain(1, ch_off); // Turn TX off 
            OutputSwitch_I.gain(2, ch_on); // Turn ON for FM   ToDo: automate this based on mode
            OutputSwitch_Q.gain(2, ch_on); // Turn ON for FM
        }   

        Amp1_L.setGain_dB(1.0f);    // Adjustable fixed output boost in dB. Turn on USB Out during RX
        Amp1_R.setGain_dB(1.0f);  

        // Set up AGC - Must Turn ON Pre and/or Post Processor to enable auto-volume control
        codec1.audioPreProcessorEnable();   // AVC on Line-In level
        codec1.audioPostProcessorEnable();  // AVC on Line-Out level
        //codec1.audioProcessorDisable();   // Default 

        // The following enables error checking inside of the "update()"
        // Output goes to the MSG_Serial (USB) Monitor.  Normally, this is quiet. 
        if (mode_sel == FM)
            FM_Detector.showError(1);

       AudioInterrupts();
        
        // Restore RX audio in and out levels, squelch large Pop in unmute.
        delay(25);  // let audio chain settle (empty) from transient
        
        // setup rest of mode-specific path using control functions to do the heavy lifting
        selectMode(mode_sel);// takes care of filter and mode-specific audio path switching         
        float vol_temp =  map( (float) user_settings[user_Profile].lineOut_RX, 31.0f, 13.0f, 0.0f, 1.0f);
        codec1.volume(vol_temp); //set max full scale volume

        if (user_settings[user_Profile].spkr_en == ON)
        {
            codec1.unmuteHeadphone();
            //codec1.unmuteLineout(); //unmute the audio output
            user_settings[user_Profile].mute = OFF;
            displayMute();
        }
        else
        {
            codec1.muteHeadphone();
            user_settings[user_Profile].mute = ON;
            displayMute();
        }
        
        RFgain(0);  // sets up the LineIn level
        AFgain(0);  // sets up the Lineout level
    }
}

//  Change FFT data source for zoom effects 
COLD void Change_FFT_Size(uint16_t new_size, float new_sample_rate_Hz)
{
    fft_size        = new_size;   //  change global size to use for audio and display
    sample_rate_Hz  = new_sample_rate_Hz;
    fft_bin_size    = sample_rate_Hz/(fft_size*2);

    AudioNoInterrupts();
    // Route selected FFT source to one of the possible many FFT processors - should save CPU time for unused FFTs
    if (fft_size == 4096)
    {
        FFT_OutSwitch_I.setChannel(0); //  1 is 4096, 0 is Off
        FFT_OutSwitch_Q.setChannel(0); //  1 is 4096, 0 is Off
    }
    else if (fft_size == 2048)
    {
        FFT_OutSwitch_I.setChannel(1); //  1 is 2048, 0 is Off
        FFT_OutSwitch_Q.setChannel(1); //  1 is 2048, 0 is Off
    }
    else if(fft_size == 1024)
    {
        FFT_OutSwitch_I.setChannel(2); //  1 is 1024, 0 is Off
        FFT_OutSwitch_Q.setChannel(2); //  1 is 1024, 0 is Off
    }
    AudioInterrupts();
}

// If peak power exceeds 100% FS, reduce LineIn level, and/or if there is an attenuator, turn it on
// This is temporary.  It will not change RF or AG Gain levels
// LineIn level will be restored if RFGain is adjusted manually or during band changes
// Set an averaging timer to raise (in small steps) LineIn back up to data table setting when there is no longer overload 
HOT void RF_Limiter(float peak_avg)
{
    static float rf_agc_limit;
    static float rf_agc_limit_last;
    float s;
    uint8_t temp = 0;
    
    s = S_Peak.read() * 100;   // If > 100 there is LineIn overload
    //s = peak_avg * 100;  // use average instead of instant values

    if (s > 100.0)  // Anyting over rf gain setting is excess, reduce RFGain
    {
        rf_agc_limit = s - 100.0f;
        //DPRINT("Peak Power = ");
        //DPRINT(s);    
        //DPRINT("  RF_AGC_Limit = ");
        //DPRINT(rf_agc_limit);
        
        if(rf_agc_limit > 20.0f)
            rf_agc_limit *= 4; // for large changes speed up gain reduction
        else if(rf_agc_limit > 10.0f)
            rf_agc_limit *= 3; // for large changes speed up gain reduction
        else 
            rf_agc_limit *= 2; // for large changes speed up gain reduction
        
        if (rf_agc_limit >= 100.0f)  // max percent we can adjust anything
            rf_agc_limit = 99.0f; // limit to 100% change
        //DPRINT(" 2=");
        //DPRINT(rf_agc_limit); 

        rf_agc_limit = 100.0f - rf_agc_limit; // invert for % delta        
        //DPRINT(" 3=%");
        //DPRINT(rf_agc_limit); 

        rf_agc_limit = user_settings[user_Profile].lineIn_level * rf_agc_limit/100;        
        DPRINT(F("*** RF AGC Limit (0-15) = "));
        DPRINTLN(rf_agc_limit); 
        
        if (rf_agc_limit !=0)
            codec1.lineInLevel(rf_agc_limit);
        
        rf_agc_limit_last = rf_agc_limit; 
        
        //RFgain(rf_agc_limit); 
    }
    else // Restore LineIn level to where it started (user setting)
    {
        temp = user_settings[user_Profile].lineIn_level * user_settings[user_Profile].rfGain/100;

        // Time interval is set by the S meter timer multiplied by the number of samples in avg function                                
        if (peak_avg < 0.10 && rf_agc_limit_last < temp)   // Value os peal_avg is experimentally determined
        {
            //DPRINT("RF AGC Limit Last = ");
            //DPRINT(rf_agc_limit_last); 
            rf_agc_limit_last = temp;
            codec1.lineInLevel(temp);   // retore to normal level
            DPRINT(F("*** Restore LineIn Level = "));
            DPRINTLN(temp);             
        }
    }
}

// Attempt to resolve the Twin peaks issue by restarting the codec and related post config actions.
// Can be used as a major part of initDSP()
COLD void resetCodec(void)
{
    DPRINTLN(F("Start Codec Initialization"));
    setZoom(2);  // 2 = no change requested, set to user settting user profile setting
    //Change_FFT_Size(fft_size, sample_rate_Hz);
    
    codec1.enable(); // MUST be before inputSelect()
    //codec1.inputSelect(MicAudioIn);   // Mic is microphone, Line-In is from Receiver audio
    codec1.lineInLevel(15); // Set to minimum, maybe prevent false Auto-iI2S detection
    codec1.muteHeadphone(); //mute the TX audio output to transmitter input 
    codec1.adcHighPassFilterEnable();
    //codec1.adcHighPassFilterDisable();
    delay(50);

    #ifdef W7PUA_I2S_CORRECTION 
        DPRINTLN(F("Start W7PUA AutoI2S Error Correction"));
        TwinPeaks(); // W7PUA auto detect and correct. Requires 100K resistors on the LineIn pins to a common Teensy GPIO pin       
    #endif

    AudioNoInterrupts();
    
    // The FM detector has error checking during object construction
    // when DPRINT is not available.  See RadioFMDetector_F32.h:
    DPRINT(F("FM Initialization errors: "));
    DPRINTLN(FM_Detector.returnInitializeFMError() );
    //FM_LO_Mixer.setSampleRate_Hz(sample_rate_Hz);
    //FM_LO_Mixer.iqmPhaseS_C(0);  // 0 to cancel the default -90 phase delay  0-512 is 360deg.) We just want the LO shift feature
    FM_LO_Mixer.frequency(15000);
    FM_LO_Mixer.useTwoChannel(false);  //when using only 1 channel for the FM shift this is the case.
    FM_LO_Mixer.useSimple(true);   

    #ifdef USE_FREQ_SHIFTER // Experimental to shift the FFT spectrum  up away from DC
        // Configure the FFT parameters algorithm
        int overlap_factor = 4;  //set to 2, 4 or 8...which yields 50%, 75%, or 87.5% overlap (8x)
        int N_FFT = audio_block_samples * overlap_factor;  
        DPRINT(F("    : N_FFT = ")); DPRINTLN(N_FFT);
        FFT_SHIFT_I.setup(audio_settings, N_FFT); //do after AudioMemory_F32();
        FFT_SHIFT_Q.setup(audio_settings, N_FFT); //do after AudioMemory_F32();

        //configure the frequency shifting
        float shiftFreq_Hz = 1200.0; //shift audio upward a bit
        float Hz_per_bin = audio_settings.sample_rate_Hz / ((float)N_FFT);
        int shift_bins = (int)(shiftFreq_Hz / Hz_per_bin + 0.5);  //round to nearest bin

        //shiftFreq_Hz = shift_bins * Hz_per_bin;
        DPRINT(F("Setting shift to ")); DPRINT(shiftFreq_Hz);
        DPRINT(F(" Hz, which is ")); DPRINT(shift_bins); 
        DPRINTLN(F(" bins"));
        FFT_SHIFT_I.setShift_bins(shift_bins); //0 is no ffreq shifting.
        FFT_SHIFT_Q.setShift_bins(shift_bins); //0 is no ffreq shifting.
    #endif
    
    #ifdef USE_FFT_LO_MIXER   // Experimental to shift the FFT spectrum  up away from DC
        // Mixer using LO to shift signal up the FFT spectrum  Experimental, not working yet
        FFT_LO_Mixer_I.setSampleRate_Hz(sample_rate_Hz);
        //FFT_LO_Mixer_I.iqmPhaseS_C(user_settings[user_Profile].rfGain*5);  // 0 to cancel the default -90 phase delay  0-512 is 360deg.) We just want the LO shift feature
        //FFT_LO_Mixer_I.frequency(user_settings[user_Profile].afGain*100);
        FFT_LO_Mixer_I.iqmPhaseS(-128);  // 0 to cancel the default -90 phase delay  0-512 is 360deg.) We just want the LO shift feature
        FFT_LO_Mixer_I.frequency(0);
        FFT_LO_Mixer_I.useTwoChannel(true);  //true when using both channels.
        FFT_LO_Mixer_I.useSimple(false); // false to use correction factors

        FFT_90deg_Hilbert.begin(hilbert251A, 251);  // Set the Hilbert transform FIR filter

        //FFT_LO_Mixer_Q.setSampleRate_Hz(sample_rate_Hz);
        //FFT_LO_Mixer_Q.iqmPhaseS_C(user_settings[user_Profile].rfGain*5);  // 0 to cancel the default -90 phase delay  0-512 is 360deg.) We just want the LO shift feature
        //FFT_LO_Mixer_Q.iqmPhaseS_C(10);  // 0 to cancel the default -90 phase delay  0-512 is 360deg.) We just want the LO shift feature
        //FFT_LO_Mixer_Q.frequency(user_settings[user_Profile].afGain*100);
        //FFT_LO_Mixer_Q.frequency(0);
        //FFT_LO_Mixer_Q.useTwoChannel(false);  
        //FFT_LO_Mixer_Q.useSimple(true); 
    #endif
    
    // Initialize our filters for RX and TX.  Using RX and TX filters since the filters specs are different later
    RX_Hilbert_Plus_45.begin(Hilbert_Plus45_40K,151);   // Left channel Rx
    RX_Hilbert_Minus_45.begin(Hilbert_Minus45_40K,151); // Right channel Rx
    
    TX_Hilbert_Plus_45.begin(Hilbert_Plus45_28K,151);   // Right channel TX
    TX_Hilbert_Minus_45.begin(Hilbert_Minus45_28K,151); // Left channel TX
    bpf1.begin(fir1, 197, 128);
    
    // Pick one of the three.
    ///FFT_90deg_Hilbert.begin(hilbert19A, 19);
    ///FFT_90deg_Hilbert.begin(hilbert121A, 121);
    //FFT_90deg_Hilbert.begin(hilbert251A, 251);
    //FFT_90deg_Hilbert.showError(1);

    // experiment with numbers  ToDo: enable/disable this via the Notch button
    DPRINT(F("Initializing Notch/NR Feature = "));
    DPRINTLN(LMS_Notch.initializeLMS(2, 32, 4));  // <== Modify to suit  2=Notch 1=Denoise
    LMS_Notch.setParameters(0.05f, 0.999f);      // (float _beta, float _decay);
    LMS_Notch.enable(false);
    NoiseBlanker.useTwoChannel(true);
    
    AudioInterrupts();

    // Will be done in mode function also except for Beep Tone (2)
    //RX_Summer.gain(0, 1.0f);  // Left Channel into mixer
	//RX_Summer.gain(1, 1.0f);  // Right Channel, intoi Miver
    RX_Summer.gain(2, 0.7f);  // Set Beep Tone ON or Off and Volume
    //RX_Summer.gain(3, 0.0f);  // FM Detection Path.  Only turn on for FM Mode
    DPRINTLN(F(" Reset Codec Almost Completed"));
    Xmit(0);  // Finish RX audio chain setup

    DPRINTLN(F(" Reset Codec Completed"));
}

#ifdef TEST1
    /*---------------------------------------------------------------*
                        --- FREQUENCY SHIFTER ---
        Performs a complex frequency shift by complex multiplication
        of a time domain signal by a complex exonential.
        From Fourier theory a spectral shift:
            F(j(w + w0)) = Fourier[f(t) * e^(j(w_0*t))]
                            = Fourier[f(t) * cos(w_0]*t) * jsin(w_0*t)]
            where f(t) is complex, and j is the sqrt(-1),
                                                                        
            Note:  In the discrete-time case this results in a
            spectral ROTATION, ie spectral components at either end
            of the complex spectrum (near the Nyquist frequency)
            will appear at the other end!
    ---------------------------------------------------------------*/
    float32_t freq_shifter(float32_t * _Idata, float32_t * _Qdata, float32_t freq_shift,  float32_t initial_phase) 
    {
        const float32_t twoPI = 2.0*PI;
        float32_t phase_inc   = freq_shift*(twoPI/AUDIO_SAMPLE_RATE_EXACT);
        float32_t phase       = initial_phase;
        // Frequency shifting is a complex multiplication in the time domain:
        for (int i = 0; i < n_block; i++) 
        {
            float32_t cosine = cos_f32(phase);
            float32_t sine   = sin_f32(phase);
            float32_t tempI  = _Idata[i];
            float32_t tempQ  = _Qdata[i];
            _Idata[i]     = tempI*cosine - tempQ*sine;
            _Qdata[i]     = tempQ*cosine + tempI*sine;
            phase        += phase_inc;
            if (phase > twoPI)    phase -= twoPI;
            else if (phase < 0.0) phase += twoPI;
        }
        // Return the updated phase for the next iteration
        return phase;
    }
#endif

#ifdef W7PUA_I2S_CORRECTION

    void TwinPeaks(void)
    {    
        TPinfo* pData;
        uint32_t timeSquareWave = 0;   // Switch every twoPeriods microseconds
        uint32_t twoPeriods;
        uint32_t tMillis = millis();
        #if SIGNAL_HARDWARE==TP_SIGNAL_CODEC
            DPRINTLN(F("Using SGTL5000 Codec output for cross-correlation test signal."));
        #endif
        #if SIGNAL_HARDWARE==TP_SIGNAL_IO_PIN
            pinMode (PIN_FOR_TP, OUTPUT);    // Digital output pin
            DPRINTLN(F("Using I/O pin for cross-correlation test signal."));
        #endif
        
        //TwinPeak.setLRfilter(true);
        //TwinPeak.setThreshold(TP_THRESHOLD);   Not used
        TwinPeak.stateAlignLR(TP_MEASURE);  // Comes up TP_IDLE

        #if SIGNAL_HARDWARE==TP_SIGNAL_IO_PIN
            twoPeriods = (uint32_t)(0.5f + (2000000.0f / sample_rate_Hz));
            // Note that for this hardware, the INO is 100% in charge of the PIN_FOR_TP
            pData = TwinPeak.read();       // Base data to check error
            while (pData->TPerror < 0  &&  millis()-tMillis < 2000)  // with timeout
            {
                if(micros()-timeSquareWave >= twoPeriods && pData->TPstate==TP_MEASURE)
                {
                    static uint16_t squareWave = 0;
                    timeSquareWave = micros();
                    squareWave = squareWave^1;
                    digitalWrite(PIN_FOR_TP, squareWave);
                }
                pData = TwinPeak.read();
            }
            // The update has moved from Measure to Run. Ground the PIN_FOR_TP
            TwinPeak.stateAlignLR(TP_RUN);  // TP is done, not TP_MEASURE
            digitalWrite(PIN_FOR_TP, 0);    // Set pin to zero

            DPRINTLN("");
            DPRINTLN(F("Update  ------------ Outputs  ------------"));
            DPRINTLN(F("Number  xNorm     -1        0         1   Shift Error State"));// Column headings
            DPRINT(pData->nMeas); DPRINT(",  ");
            DPRINT(pData->xNorm, 6); DPRINT(", ");
            DPRINT(pData->xcVal[3], 6); DPRINT(", ");
            DPRINT(pData->xcVal[0], 6); DPRINT(", ");
            DPRINT(pData->xcVal[1], 6); DPRINT(", ");
            DPRINT(pData->neededShift); DPRINT(",   ");
            DPRINT(pData->TPerror); DPRINT(",    ");
            DPRINTLN(pData->TPstate);
            DPRINTLN("");

            // You can see the injected signal level by the printed variable, pData->xNorm
            // It is the sum of the 4 cross-correlation numbers and if it is below 0.0001 the
            // measurement is getting noisy and un-reliable.  Raise the injected signal level
            // to solve the problem.
        #endif        
    }
#endif

#ifdef USE_RS_HFIQ   //  Check the serial ports for manual inputs.
void RS_HFIQ_Service(void)
{
    uint32_t temp_freq;
    static uint32_t last_VFOA = VFOA;
    static uint32_t last_VFOB = VFOB;
    static int8_t last_curr_band = curr_band;
    static uint8_t CAT_xmit_last = 0;
    static uint8_t CAT_xmit = 0;
    static uint8_t split_last = bandmem[curr_band].split;
    static uint8_t swap_VFO = 0;
    static uint8_t swap_VFO_last = 0;

    //if ((temp_freq = RS_HFIQ.cmd_console(&swap_VFO, &VFOA, &VFOB, &curr_band, &(user_settings[user_Profile].xmit), &(bandmem[curr_band].split))) != 0)
    if ((temp_freq = RS_HFIQ.cmd_console(&swap_VFO, &VFOA, &VFOB, &curr_band, &CAT_xmit, &(bandmem[curr_band].split))) != 0)
    {
        if (popup) return;  // ignore external changes while a window is active.  Changes wil be applied when the window is close (popup OFF)

        if (bandmem[curr_band].split != split_last)
        {
            DPRINTLN("Split VFOs");
            split_last = bandmem[curr_band].split;
            Split(bandmem[curr_band].split);
            return;
        }

        if (swap_VFO != swap_VFO_last)
        {
            DPRINTLN("Swap VFOs: "); 
            swap_VFO_last = swap_VFO;
            VFO_AB();
        }

        //DPRINT(F("Main New Band: ")); DPRINTLN(curr_band);
        //DPRINT(F("Main VFOA: ")); DPRINTLN(VFOA);
        //DPRINT(F("Main VFOB: ")); DPRINTLN(VFOB);

        // Now we possibly have a new band so load up the per-band settings, update both VFOs for that band
        if (last_VFOA != VFOA)
        {                     
            bandmem[curr_band].vfo_A_last = VFOA; 
            //DPRINT(F("Main1A VFOA: ")); DPRINTLN(VFOA);
        }
        if (last_VFOB != VFOB)
        {     
            bandmem[curr_band].vfo_B_last = VFOB; 
            user_settings[user_Profile].sub_VFO = VFOB;
            //DPRINT(F("Main1B VFOB: ")); DPRINTLN(VFOB);
        }

        if (last_curr_band != curr_band)
        {
            changeBands(0);
            last_curr_band = curr_band;
            //DPRINT(F("New Band Number: ")); DPRINTLN(curr_band);  
        }
    }

    if (CAT_xmit_last != CAT_xmit) // Pass on any change in Xmit state from CAT port
    {
        //DPRINT(F("CAT Port: TX = ")); DPRINTLN(CAT_xmit);
        CAT_xmit_last = CAT_xmit;
        Xmit(CAT_xmit);
    }

    if (temp_freq != 0)
    {
        if (last_VFOA != VFOA || last_VFOB != VFOB)  // only act on frequency changes, skip other things like queries
        {                         
            selectFrequency(0);
            displayFreq();
            last_VFOA = VFOA;
            last_VFOB = VFOB;
        }
    }
}
#endif  // USE_RS_HFIQ

#if defined I2C_ENCODERS || defined MECH_ENCODERS
void Check_Encoders(void)
{   
    #if defined I2C_ENCODERS
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
                //if (mfg) { DPRINT(F("****Checked MF_Enc status = ")); DPRINTLN(mfg); }
            }
            #endif
            #ifdef ENC2_ADDR
            if(ENC2.updateStatus() && user_settings[user_Profile].encoder2_client)
            {
                mfg = ENC2.readStatus();
                if (mfg) {}
                //if (mfg) {DPRINT(F("****Checked Encoder #2 status = ")); DPRINTLN(mfg); }
            }
            #endif
            #ifdef ENC3_ADDR
            if(ENC3.updateStatus() && user_settings[user_Profile].encoder3_client)
            {
                mfg = ENC3.readStatus();
                if (mfg) {}
                //if (mfg) {DPRINT(F("****Checked Encoder #3 status = ")); DPRINTLN(mfg); }
            }
            #endif
            #ifdef ENC4_ADDR
            if(ENC4.updateStatus() && user_settings[user_Profile].encoder4_client)
            {
                mfg = ENC4.readStatus();
                if (mfg) {}
                //if (mfg) {DPRINT(F("****Checked Encoder #4 status = ")); DPRINTLN(mfg); }
            }
            #endif
            #ifdef ENC5_ADDR
            if(ENC5.updateStatus() && user_settings[user_Profile].encoder5_client)
            {
                mfg = ENC5.readStatus();
                if (mfg) {}
                //if (mfg) {DPRINT(F("****Checked Encoder #5 status = ")); DPRINTLN(mfg); }
            }
            #endif
            #ifdef ENC6_ADDR
            if(ENC6.updateStatus() && user_settings[user_Profile].encoder6_client)
            {
                mfg = ENC6.readStatus();
                if (mfg) {}
                //if (mfg) {DPRINT(F("****Checked Encoder #6 status = ")); DPRINTLN(mfg); }
            }
            #endif
        }
    #else
        #ifdef MECH_ENCODERS
            // Use a mechanical encoder on the GPIO pins, if any.
            if (MF_client)  // skip if no one is listening.MF_Service();  // check the Multi-Function encoder and pass results to the current owner, of any.
            {
                static int8_t counts = 0;
                static int8_t counts_last = 0;
                
                counts += (int8_t) multiKnob(0);
                
                if (!counts_last && counts)  // detect and capture new counts, start timer
                {
                    ENC_Read_timer.reset();
                    counts_last = counts; 
                    return;
                }

                if (counts_last && abs(counts) && ENC_Read_timer.check() == 1)
                { 
                    //DPRINTLN(counts/4);
                    MF_Service(counts/4, MF_client);
                    multiKnob(1); // clear buffer
                    counts = 0;
                    counts_last = 0;
                } 
            }
        #endif
    #endif // I2C_ENCODERS

    #ifdef MECH_ENCODERS
        
        static uint8_t ENC2_sw_pushed = 0;
        static uint8_t ENC3_sw_pushed = 0;

        // ToDo: Check timer for long and short press, add debounce
        //if (!ENC2_PIN_SW) Button_Action(user_settings[user_Profile].encoder1_client_swl);
        
        if (!digitalRead(ENC2_PIN_SW) && !ENC2_sw_pushed)
        {
            Button_Action(user_settings[user_Profile].encoder1_client_sw);
            ENC2_sw_pushed = 1;
        }
        else if (digitalRead(ENC2_PIN_SW) && ENC2_sw_pushed)
            ENC2_sw_pushed = 0;

        if (!digitalRead(ENC3_PIN_SW) && !ENC3_sw_pushed)
        {
            Button_Action(user_settings[user_Profile].encoder2_client_sw);
            ENC3_sw_pushed = 1;
        }
        else  if (digitalRead(ENC3_PIN_SW) && ENC3_sw_pushed)
            ENC3_sw_pushed = 0;
    #endif
}
#endif 
