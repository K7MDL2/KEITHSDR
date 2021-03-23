//  SDR_RA8875.h
//
//  Header File for the Main Arduino program file
//
//  Spectrum, Display, full F32 library conversion completed 3/2021. Uses FFTXXXX_IQ_F32 FFT I and Q version files
//      XXXX can be the 256, 1024, 2048 or 4096 versions.
//  Spectrum uses the raw FFT output and is not calibrated.
//
//  Test tones are enabled in spectrum only, not in audio path.
//
#include <SPI.h>                // included with Arduino
#include <Wire.h>               // included with Arduino
#include <WireIMXRT.h>          // gets installed with wire.h
#include <WireKinetis.h>        // included with Arduino
#define  RA8875_INT        14   //any pin
#define  RA8875_CS         10   //any digital pin
#define  RA8875_RESET      9    //any pin or nothing!
#define  MAXTOUCHLIMIT     3    //1...5
#include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
#include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
#include <RA8875.h>             // internal Teensy library with ft5206 cap touch enabled in user_setting.h
//#define OCXO_10MHZ              // uncoment this line to use a different library that supports External CLKIN for si5351C version PLL boards.
//#define DIG_STEP_ATT 
//#define SV1AFN_BPF
#ifdef OCXO_10MHZ 
 #define ENET                   // Turn off or on ethernet features and hardware
 #define USE_ENET_PROFILE       // This is inserted here to conveniently turn on ethernet profile for me using 1 setting.
 #define REMOTE_OPS             // Turn on Remote_Ops ethernet write feature for remote control head dev work.
 #define SV1AFN_BPF             // Enable the SV1AFN 10-band Preselector (Bandpass filter) board.
 #define DIG_STEP_ATT           // Enable the PE4302 Digital Step Attenuator.  0-31.5dB in 0.5dB steps via I2C port expander
 #include <si5351.h>            // Using this liunrary because it support the B and C version PLLs with external ref clock
 Si5351 si5351;
#else
 #include <si5351mcu.h>          // Github https://github.com/pavelmc/Si5351mcu
 Si5351mcu si5351;
#endif
#define  ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>            // Internal Teensy library and at C:\Program Files (x86)\Arduino\hardware\teensy\avr\libraries
#include <Metro.h>              // GitHub https://github.com/nusolar/Metro
#include <Audio.h>              // Included with Teensy and at GitHub https://github.com/PaulStoffregen/Audio
#include <OpenAudio_ArduinoLibrary.h> // F32 library located on GitHub. https://github.com/chipaudette/OpenAudio_ArduinoLibrary
#include <InternalTemperature.h>
#include <TimeLib.h>
#ifdef SV1AFN_BPF
  #include <SVN1AFN_BandpassFilters.h>
#endif
// Below are local project files
#include "RadioConfig.h"        // Majority of declarations here
#include "SDR_Network.h"        // for ethernet UDP remote control and monitoring
#include "Spectrum_RA8875.h"    // spectrum
#include "Hilbert.h"            // filter coefficients
#include "Vfo.h"
#include "Display.h"
#include "Tuner.h"
#include "AGC.h"
#include "Mode.h"
#include "Bandwidth2.h"
#include "Step.h"               // not used by the program, left for backward compat for some time.
#include "Smeter.h"
#include "CW_Tune.h"
#include "Quadrature.h"
#include "Controls.h"
#include "UserInput.h"          // include after Spectrum_RA8875.h and Display.h

#ifdef SV1AFN_BPF
SVN1AFN_BandpassFilters bpf;   // The SV1AFN Preselector module supporing all HF bands and a preamp and Attenuator. 
// For 60M coverage requires and updated libary file set.
#endif

// Audio Library setup stuff
//const float sample_rate_Hz = 11000.0f;  //43Hz /bin  5K spectrum
//const float sample_rate_Hz = 22000.0f;  //21Hz /bin 6K wide
//const float sample_rate_Hz = 44100.0f;  //43Hz /bin  12.5K spectrum
//const float sample_rate_Hz = 48000.0f;  //46Hz /bin  24K spectrum for 1024.  
//const float sample_rate_Hz = 51200.0f;  // 50Hz/bin for 1024, 200Hz/bin for 256 FFT. 20Khz span at 800 pixels 2048 FFT
const float sample_rate_Hz = 102400.0f;   // 100Hz/bin at 1024FFT, 50Hz at 2048, 40Khz span at 800 pixels and 2048FFT
//const float sample_rate_Hz = 192000.0f; // 190Hz/bin - does
//const float sample_rate_Hz = 204800.0f; // 200/bin at 1024 FFT

const int   audio_block_samples = 128;          // do not change this!
AudioSettings_F32 audio_settings(sample_rate_Hz, audio_block_samples);

const int myInput = AUDIO_INPUT_LINEIN;
//const int myInput = AUDIO_INPUT_MIC;
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
//
//============================================  Start of Spectrum Setup Section =====================================================
//
// used for spectrum object
//#define FFT_SIZE                  4096            // need a constant for array size declarion so manually set this value here   Could try a macro later
int16_t         fft_bins            = FFT_SIZE;     // Number of FFT bins which is FFT_SIZE/2 for real version or FFT_SIZE for iq version
float           fft_bin_size        = sample_rate_Hz/(FFT_SIZE*2);   // Size of FFT bin in HZ.  From sample_rate_Hz/FFT_SIZE for iq
extern int16_t  spectrum_preset;                    // Specify the default layout option for spectrum window placement and size.
int16_t         FFT_Source          = 0;            // used to switch teh FFT input source around
extern Metro spectrum_waterfall_update;             // Timer used for controlling the Spectrum module update rate.
//
//============================================ End of Spectrum Setup Section =====================================================
//
                               
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
//AudioAnalyzeFFT256_IQ_F32 myFFT;
AudioOutputI2S_F32      Output(audio_settings);

//#define TEST_SINEWAVE_SIG
#ifdef TEST_SINEWAVE_SIG
//AudioSynthSineCosine_F32   sinewave1;
//AudioSynthSineCosine_F32   sinewave2;
//AudioSynthSineCosine_F32   sinewave3;
AudioSynthWaveformSine_F32 sinewave1;
AudioSynthWaveformSine_F32 sinewave2;
AudioSynthWaveformSine_F32 sinewave3;
AudioConnection_F32     patchCord4c(sinewave2,0,  FFT_Switch1,2);
AudioConnection_F32     patchCord4d(sinewave3,0,  FFT_Switch1,3);
//AudioConnection_F32     patchCord4e(sinewave3,0,  FFT_Switch1,4);
#endif

AudioConnection_F32     patchCord4a(Input,0,      FFT_Switch1,0);
AudioConnection_F32     patchCord4b(Input,1,      FFT_Switch2,0);
AudioConnection_F32     patchCord4c(Output,0,     FFT_Switch1,1);
AudioConnection_F32     patchCord4d(Output,1,     FFT_Switch2,1);
AudioConnection_F32     patchCord1a(Input,0,      Hilbert1,0);
AudioConnection_F32     patchCord1b(Input,1,      Hilbert2,0);
AudioConnection_F32     patchCord1c(Hilbert1,0,   Q_Peak,0);
AudioConnection_F32     patchCord1d(Hilbert2,0,   I_Peak,0);
AudioConnection_F32     patchCord2e(Hilbert1, 0,  RX_Summer,0);
AudioConnection_F32     patchCord2f(Hilbert2, 0,  RX_Summer,1);
AudioConnection_F32     patchCord2g(RX_Summer,0,  S_Peak,0);
AudioConnection_F32     patchCord2h(RX_Summer,0,  CW_Filter,0);
AudioConnection_F32     patchCord2i(CW_Filter,0,  CW_Peak,0);
AudioConnection_F32     patchCord2i1(CW_Filter,0, CW_RMS,0);
AudioConnection_F32     patchCord2j(CW_Filter,0,  Output,0);
AudioConnection_F32     patchCord2k(CW_Filter,0,  Output,1);
AudioConnection_F32     patchCord4f(FFT_Switch1,0, myFFT,0);
AudioConnection_F32     patchCord4g(FFT_Switch2,0, myFFT,1);

AudioControlSGTL5000    codec1;

///////////////////////Set up global variables for Frequency, mode, bandwidth, step

// These should be saved in EEPROM periodically along with several other parameters
uint8_t     curr_band   = BAND4;    // global tracks our current band setting.  
uint32_t    VFOA        = 0;        // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
uint32_t    VFOB        = 0;
int32_t     Fc          = 0;        //(sample_rate_Hz/4);  // Center Frequency - Offset from DC to see band up and down from cener of BPF.   Adjust Displayed RX freq and Tx carrier accordingly

//extern struct User_Settings user_settings[];
//extern struct Band_Memory bandmem[];

//control display and serial interaction
bool        enable_printCPUandMemory = false;   // CPU , memory and temperature
void        togglePrintMemoryAndCPU(void) { enable_printCPUandMemory = !enable_printCPUandMemory; };
uint8_t     popup = 0;                          // experimental flag for pop up windows
int32_t     multiKnob(uint8_t clear);           // consumer features use this for control input

// Most of our timers are here.  Spectrum waerfall is in the spectrum settings section
Metro touch         = Metro(100);   // used to check for touch events
Metro tuner         = Metro(1000);  // used to dump unused encoder counts for high PPR encoders when counts is < enc_ppr_response for X time.
Metro meter         = Metro(400);   // used to update the meters
Metro popup_timer   = Metro(500);   // used to check for popup screen request
Metro NTP_updateTx  = Metro(10000);
Metro NTP_updateRx  = Metro(65000);

RA8875 tft = RA8875(RA8875_CS,RA8875_RESET); //initiate the display object
Encoder Position(4,5); //using pins 4 and 5 on teensy 4.0 for A/B tuning encoder 
Encoder Multi(40,39);
uint8_t     enc_ppr_response = 60;  // this scales the PPR to account for high vs low PPR encoders.  600ppr is very fast at 1Hz steps, worse at 10Khz!
// I find a value of 60 works good for 600ppr. 30 should be good for 300ppr, 1 or 2 for typical 24-36 ppr encoders. Best to use even numbers above 1. 

