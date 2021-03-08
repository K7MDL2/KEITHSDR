//  Updated 3/8/2021
//
// Spectrum, Display, full F32 library conversion completed 3/2021.  Using FFT1024_IQ_F32.   
// Spectrum not calibrated, need t be scaled to full width, bar disp not working, work in progress.
// Spectrum code moved to spectrum.h.  Band up and Band down button configure in lower right corner.  
// Spectrum DOT plot is working.  Bars are not yet.  Scales are not accurate, no zoom/span yet.  Resizing is working.
//  Test tones are enabled in spectrum only, not in audio path.
//
#include <SPI.h>              // included with Arduino
#include <Wire.h>             // included with Arduino
#include <WireIMXRT.h>        // gets installed with wire.h
#include <WireKinetis.h>      // included with Arduino
#define RA8875_INT        14  //any pin
#define RA8875_CS         10  //any digital pin
#define RA8875_RESET      9   //any pin or nothing!
#define MAXTOUCHLIMIT     2   //1...5
#include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
#include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
#include <RA8875.h>           // internal Teensy library with ft5206 cap touch enabled in user_setting.h
#include <si5351mcu.h>        // Github https://github.com/pavelmc/Si5351mcu
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>          // Internal Teensy library and at C:\Program Files (x86)\Arduino\hardware\teensy\avr\libraries
#include <Metro.h>            // GitHub https://github.com/nusolar/Metro
#include <Audio.h>            // Included with Teensy and at GitHub https://github.com/PaulStoffregen/Audio
#include <OpenAudio_ArduinoLibrary.h> // F32 library located on GitHub. https://github.com/chipaudette/OpenAudio_ArduinoLibrary

#include "hilbert.h"          // This and below are local project files
#include "Vfo.h"
#include "Display.h"
#include "Tuner.h"
#include "AGC.h"
#include "Mode.h"
#include "BandWidth2.h"
#include "Step.h"
#include "Smeter.h"
#include "CW_Tune.h"
#include "Quadrature.h"
#include "Spectrum_RA8875.h"
#include "RadioConfig.h"
#include "UserInput.h"   // include after Spectrun_RA8875.h abd Display.h

RA8875 tft = RA8875(RA8875_CS,RA8875_RESET); //initiate the display object
Encoder Position(4,5); //using pins 4 and 5 on teensy 4.0 for A/B tuning encoder 
Si5351mcu si5351;
const int myInput = AUDIO_INPUT_LINEIN;
//const int myInput = AUDIO_INPUT_MIC;

// Audio Library setup stuff
//float sample_rate_Hz = 11000.0f;  //43Hz /bin  5K spectrum
//float sample_rate_Hz = 22000.0f;  //21Hz /bin 6K wide
//float sample_rate_Hz = 44100.0f;  //43Hz /bin  12.5K spectrum
float sample_rate_Hz = 48000.0f;  //46Hz /bin  24K spectrum for 1024, 187.5/bin for 256 FFTiq
//float sample_rate_Hz = 51200.0f;    // 50Hz/bin for 1024, 200Hz/bin for 256 FFT
//float sample_rate_Hz = 102400.0f;  // 100Hz/bin
//float sample_rate_Hz = 192000.0f;  // 190Hz/bin
//float sample_rate_Hz = 204800.0f;  // 200/bin
const int   audio_block_samples = 128;
AudioSettings_F32 audio_settings(sample_rate_Hz, audio_block_samples);

//
//============================================  Start of Spectrum Setup Section =====================================================
//
// used for spectrum object
//#define FFT_SIZE            1024           // need a constant for array size declarion so manually set this value here   Could try a macro later
int16_t fft_bins            = FFT_SIZE;     // Number of FFT bins which is FFT_SIZE/2 or FFT_SIZE for iq version
float fft_bin_size = sample_rate_Hz/(FFT_SIZE*2);   // Size of FFT bin in HZ.  From sample_rate_Hz/FFT_SIZE for iq

extern int16_t spectrum_preset;   // Specify the default layout option for spectrum window placement and size.
int16_t waterfall_speed     = 60;    // window update rate in ms.  25 is fast enough to see dit and dahs well
Metro spectrum = Metro(waterfall_speed);
int16_t FFT_Source          = 0;
//
//============================================ End of Spectrum Setup Section =====================================================
//
                               
AudioInputI2S_F32       Input(audio_settings);
AudioMixer4_F32         FFT_Switch1;
AudioMixer4_F32         FFT_Switch2;
AudioFilterFIR_F32      Hilbert1;
AudioFilterFIR_F32      Hilbert2;
AudioFilterBiquad_F32   CW_Filter;
AudioMixer4_F32         RX_Summer;
AudioAnalyzePeak_F32    S_Peak; 
AudioAnalyzePeak_F32    Q_Peak; 
AudioAnalyzePeak_F32    I_Peak;
AudioAnalyzePeak_F32    CW_Peak;
AudioAnalyzeRMS_F32     CW_RMS;  
AudioAnalyzeFFT1024_IQ_F32  myFFT;
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
int mndx=3; // sets the index to cycle through the modes ..
int bndx=8; // sets the bandwidth initial index
int fndx=4; // sets tuning step increment
int andx=0; // AGC setting
String mode="";
String bandwidth="";
String increment="";
String agc="";
//
//================================================ Frequency Set =============================================================
volatile uint32_t Freq = 3573000;        //I used 7850000  frequency CHU  Time Signal Canada
//================================================ Frequency Set =============================================================
//
volatile uint32_t Fc = 0; //9;   //(sample_rate_Hz/4);  // Center Frequency - Offset from DC to see band up and down from cener of BPF.   Adjust Displayed RX freq and Tx carrier accordingly
volatile uint32_t fstep = 10   ; // sets the tuning increment to 10Hz
//control display and serial interaction
bool enable_printCPUandMemory = false;
void togglePrintMemoryAndCPU(void) { enable_printCPUandMemory = !enable_printCPUandMemory; };
long newFreq=0;
long oldFreq=0;
int attenuator=1;
int preamp=1;
Metro touch=Metro(350);
Metro meter=Metro(200);
Metro tune=Metro(125);
//
// _______________________________________ Setup_____________________________________________
//
void setup() 
{
	//Wire.setClock(400000);  // Increase i2C bus transfer data rate from default of 100KHz 
	//Serial.begin(115200);
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
	//initDisplay();    // Draw main screen  Call after initSpectrum_RA8875()
	initVfo();        // initialize the si5351 vfo
	SetFreq();        // just run this for something to do
	displayFreq(); // display frequency
	displayAttn();
	displayStep();
	selectStep(fndx);    
	displayAgc();

    // TODO: Move this to set mode and/or bandwidth section when ready.  messes up initial USB/or LSB/CW alignments until one hits the mode button.
    RX_Summer.gain(0,-3.0);   // Leave at 1.0
    RX_Summer.gain(1,3.0);  // -1 for LSB out
    bndx = 8;
    selectBandwidth(bndx);
    selectAgc();
    selectMode(); 
    
    //AudioMemory(16);   // moved to 32 bit so no longer needed hopefully
    AudioMemory_F32(50, audio_settings);

    //TODO: Many of these need to be called in other places also such as when changing bands or AGC to mute and unmute, during TX for another example  
    codec1.enable();  // MUST be before inputSelect()
    delay(500);
    codec1.dacVolumeRampDisable();    // Turn off the sound for now
    codec1.inputSelect(myInput);    
    codec1.lineInLevel(8);     // range 0 to 15.  0 => 3.12Vp-p, 15 => 0.24Vp-p sensitivity
    codec1.lineOutLevel(13);    // range 13 to 31.  13 => 3.16Vp-p, 31=> 1.16Vp-p
    codec1.volume(1.0);   // 0.7 seemed optimal for K7MDL with QRP_Labs RX board with 15 on line input and 20 on line output
    codec1.autoVolumeControl(2,0,0,-36.0,12,6); // add a compressor limiter
    //codec1.autoVolumeControl( 0-2, 0-3, 0-1, 0-96, 3, 3);
    //autoVolumeControl(maxGain, response, hardLimit, threshold, attack, decay);
    codec1.autoVolumeEnable();// let the volume control itself..... poor mans agc
    //codec1.autoVolumeDisable();// Or don't let the volume control itself
    codec1.unmuteHeadphone();
    codec1.unmuteLineout(); //unmute the audio output
    codec1.adcHighPassFilterDisable();
    codec1.dacVolume(0);    // set the "dac" volume (extra control)
    // Now turn on the sound    
    RampVolume(1.0, 1);  //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp" 
	mndx=3;
	bndx = 8;
    selectMode();
	selectBandwidth(bndx);
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
    drawSpectrumFrame(spectrum_preset);  // Call after initSpectrum() to draw the spectrum object.  Arg is 0 PRESETS to load a preset record
            // DrawSpectrum does not read the globals but does update them to match the current preset.
            // Therefore always call the generator before drawSpectrum() to create a new set of params you can cut anmd paste.
            // Generator never modifies the globals so never affects the layout itself.
     // Print out our starting frequency for testing
    Serial.print("\nInitial Dial Frequency is "); Serial.print(Freq); Serial.println("MHz");
    
    //finish the setup by printing the help menu to the serial connections
    printHelp();
}
//
// __________________________________________ Main Program Loop  _____________________________________
//
void loop() 
{
    // Update spectrum and waterfall based on timer
    
    if (spectrum.check() == 1)
    {   
        spectrum_update(spectrum_preset);   // valid numbers are 0 through PRESETS to index the record of predefined window layouts 
    }

    if(touch.check()==1) ////// touch interrupt runs wayyy tooo fast .. so scheduled it up
    {
        Touch(); // need to get the touch working //
    }
 
    if(tune.check()==1)
    {
        newFreq = Position.read();
        if(newFreq!=oldFreq)
        {
            selectFrequency(); // need to get the tuner working //
        }
    }
    
    if(meter.check()==1)
    {
        Peak();
        // Code_Peak();
        // Quad_Check();
    }

    //respond to Serial commands
    while(Serial.available())
    {
        respondToByte((char)Serial.read());
    }
    
    //check to see whether to print the CPU and Memory Usage
    if (enable_printCPUandMemory) printCPUandMemory(millis(), 3000); //print every 3000 msec
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
        Serial.print(" Audio MEM Float32 Cur/Peak: ");
        Serial.print(AudioMemoryUsage_F32());
        Serial.print("/");
        Serial.println(AudioMemoryUsageMax_F32());
        Serial.print(" Audio MEM Int16 Cur/Peak: ");
        Serial.print(AudioMemoryUsage());
        Serial.print("/");
        Serial.println(AudioMemoryUsageMax());
        
        lastUpdate_millis = curTime_millis; //we will use this value the next time around.
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
