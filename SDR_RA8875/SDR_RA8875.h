#ifndef _SDR_RA8875_
#define _SDR_RA8875_

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
#define  ENCODER_OPTIMIZE_INTERRUPTS  // leave this one here.  Not normally user changed
#include <Encoder.h>            // Internal Teensy library and at C:\Program Files (x86)\Arduino\hardware\teensy\avr\libraries
#include <Metro.h>              // GitHub https://github.com/nusolar/Metro
#include <Audio.h>              // Included with Teensy and at GitHub https://github.com/PaulStoffregen/Audio
#include <OpenAudio_ArduinoLibrary.h> // F32 library located on GitHub. https://github.com/chipaudette/OpenAudio_ArduinoLibrary
#include <InternalTemperature.h>// TODO  - list where to find this
#include <TimeLib.h>            // TODO  - list where to find this

// Now pickup build time options from RadioConfig.h
#include "RadioConfig.h"        // Majority of declarations here to drive teh #ifdefs that follow

#ifdef I2C_ENCODER              // This turns on support for DuPPa.net I2C encoder with RGB LED integrated. 
#include <i2cEncoderLibV2.h>    // GitHub https://github.com/Fattoresaimon/ArduinoDuPPaLib
#endif  // I2C_ENCODER

#ifdef SV1AFN_BPF               // This turns on support for the Bandpass Filter board and relays for LNA and Attenuation
  #include <SVN1AFN_BandpassFilters.h> // Modified and redistributed in this build source folder
  SVN1AFN_BandpassFilters bpf;   // The SV1AFN Preselector module supporing all HF bands and a preamp and Attenuator. 
  // For 60M coverage requires and updated libary file set.
#endif // SV1AFN_BPF

#ifdef OCXO_10MHZ               // This turns on a group of features feature that are hardware required.  Leave this commented out if you do not have this hardware!
 #include <si5351.h>            // Using this liunrary because it support the B and C version PLLs with external ref clock
 Si5351 si5351;
#else // OCXO_10MHZ
 #include <si5351mcu.h>         // Github https://github.com/pavelmc/Si5351mcu
 Si5351mcu si5351;
#endif // OCXO_10MHZ

// Below are local project files
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

RA8875 tft = RA8875(RA8875_CS,RA8875_RESET); //initiate the display object

#ifdef I2C_LCD
  #include <LiquidCrystal_I2C.h>
  LiquidCrystal_I2C lcd(LCD_ADR,LCD_COL, LCD_LINES);  // set the LCD address to 0x27 for a 16 chars and 2 line display
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

#ifdef TEST_SINEWAVE_SIG
//AudioSynthSineCosine_F32   sinewave1;
//AudioSynthSineCosine_F32   sinewave2;
//AudioSynthSineCosine_F32   sinewave3;
AudioSynthWaveformSine_F32 sinewave1;
AudioSynthWaveformSine_F32 sinewave2;
AudioSynthWaveformSine_F32 sinewave3;
AudioConnection_F32     patchCord4w(sinewave2,0,  FFT_Switch1,2);
AudioConnection_F32     patchCord4x(sinewave3,0,  FFT_Switch1,3);
AudioConnection_F32     patchCord4y(sinewave2,0,  FFT_Switch2,2);
AudioConnection_F32     patchCord4z(sinewave3,0,  FFT_Switch2,3);
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
volatile int32_t  Freq_Peak = 0;

#endif //_SDR_RA8875_
