// 
//      SPECTRUM-WATERFALL.cpp
//
//  This is an example file how to use the Spectrum_RA887x library to display a custom
//  sized window containing a sepctrum display and a waterfall display.
//  It takes advantage of the RA887x controller's ability to move blocks of display memory
//  with very few CPU commands enabling rather high resolution scrolling spectrum and waterfall
//  displays possible without undue burden on the host CPU.   
//  This allows CPU cycles to handle other tasks like audio processing and encoders.
//
//  If you have a RX board connected with a PLL, you can add VFO config code and see radio signal on the screen.
//   Without a RX and PLL, you will see a blank Spectrum (upper half) and a scrolling waterfall (lower half) with no signals.
//
//

#include "Spectrum-Waterfall.h"

#ifdef USE_RA8875
  RA8875 tft = RA8875(RA8875_CS,RA8875_RESET); //initiate the display object
#else
  RA8876_t3 tft = RA8876_t3(RA8876_CS,RA8876_RESET); //initiate the display object
  FT5206 cts = FT5206(CTP_INT); 
#endif
//
//============================================ End of Spectrum Setup Section =====================================================
//

void I2C_Scanner(void);
void printHelp(void);
void printCPUandMemory(unsigned long curTime_millis, unsigned long updatePeriod_millis);
void respondToByte(char c);
const char* formatVFO(uint32_t vfo);

// User program would supply this to describe the screen layout
struct Spectrum_Parms Sp_Parms_Def[1] = { // define default sets of spectrum window parameters, mostly for easy testing but could be used for future custom preset layout options
  //W LE  RE  CG x   y   w  h  c sp st clr sc mode scal reflvl wfrate
  #ifdef USE_RA8875
    {798,0, 0,  0,798,398,14,8,157,179,179,408,400,110,111,289,289,  0,153,799,256,50,20,6,240,1.0,0.9,1,20, 8, 70},
  #else
    {1020,1,1,  1,1021,510,14,8,143,165,165,528,520,142,213,307,307,  0,139,1022,390,40,20,6,890,1.5,0.9,1,20,10, 80},
  #endif
};

// User program only needs to edit this to create new layout records you can add to the above table.
// Not used in this example sicnce 1 layout is provided above for 1x 4.3" and 1x 7" screen.
struct New_Spectrum_Layout Custom_Layout[1] = {      // Temp storage for generating new layouts    
    0,        // spectrum_x >  0 to width of display - window width. Must fit within the button frame edges left and right
                    // ->Pay attention to the fact that position X starts with 0 so 100 pixels wide makes the right side value of x=99.
    153,      // spectrum_y0 to vertical height of display - height of your window. Odd Numbers are best if needed to make the height an even number and still fit on the screen
    256,      // spectrum_height Total height of the window. Even numbers are best. (height + Y) cannot exceed height of the display or the window will be off screen.
    50,       // spectrum_center Value 0 to 100.  Smaller value = biggger waterfall. Specifies the relative size (%) between the spectrum and waterfall areas by moving the dividing line up or down as a percentage
                    // Smaller value makes spectrum smaller, waterfall bigger
    799,      // spectrum_width Total width of window. Even numbers are best. 552 is minimum to fit a full 512 pixel graph plus the min 20 pixel border used on each side. Can be smaller but will reduce graph area
    20,       // spectrum_span Value in KHz.  Ths will be the maximum span shown in the display graphs.  
                    // The graph code knows how many Hz per bin so will scale down to magnify a smaller range.
                    // Max value and resolutoin (pixels per bin) is dependent on sample frequency
                    // 25000 is max for 1024 FFT with 500 bins at 1:1 bins per pixel
                    // 12500 would result in 2 pixels per bin. Bad numbers here should be corrected to best fit by the function
    2,        // spectrum_wf_style Range 1- 6. Specifies the Waterfall style.
    330,      // spectrum_wf_colortemp Range 1 - 1023. Specifies the waterfall color temperature to tune it to your liking
    1.0,      // spectrum_wf_scal e0.0f to 40.0f. Specifies thew waterfall zoom level - may be redundant when Span is worked out later.
    0.9,      // spectrum_LPFcoeff 1.0f to 0.0f. Data smoothing
    1,        // spectrum_dot_bar_mode 0=bar, 1=Line. Spectrum box
    40,       // spectrum_sp_scale 10 to 80. Spectrum scale factor in dB. This is the height of the scale (if possible by windows sizes). Will plot the spectrum window of values between the floor and the scale value creating a zoom effect.
    -175,     // spectrum_floor 0 to -150. The reference point for plotting values.  Anything signal value > than this (less negative) will be plotted until stronger than the window height*scale factor.
    70        // spectrum_wf_rate window update rate in ms.  25 is fast enough to see dit and dahs well    
};

// -------------------------------------------------------------------------------------------
// Audio Library setup stuff to provide FFT data with optional Test tone
// -------------------------------------------------------------------------------------------
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

// used for spectrum object
#define FFT_SIZE                  4096            // Need a constant for array size declarion so manually set this value here.  Could try a macro later
int16_t         fft_bins            = FFT_SIZE;     // Number of FFT bins which is FFT_SIZE/2 for real version or FFT_SIZE for iq version
float           fft_bin_size        = sample_rate_Hz/(FFT_SIZE*2);   // Size of FFT bin in HZ.  From sample_rate_Hz/FFT_SIZE for iq
int16_t         spectrum_preset     = 0;                    // Specify the default layout option for spectrum window placement and size.
extern Metro    spectrum_waterfall_update;          // Timer used for controlling the Spectrum module update rate.
extern struct   Spectrum_Parms Sp_Parms_Def[];

const int myInput = AUDIO_INPUT_LINEIN;
//const int myInput = AUDIO_INPUT_MIC;
                            
AudioInputI2S_F32           Input(audio_settings);
AudioAnalyzeFFT4096_IQ_F32  myFFT;  // choose which you like, set FFT_SIZE accordingly.
AudioMixer4_F32             FFT_Switch1(audio_settings);
AudioMixer4_F32             FFT_Switch2(audio_settings);
AudioOutputI2S_F32          Output(audio_settings);

#ifdef TEST_SINE
// Connections for FFT Only - chooses either the input or the output to display in the spectrum plot
  AudioSynthWaveformSine_F32  sinewave1; // for audible alerts like touch beep confirmations
  AudioSynthWaveformSine_F32  sinewave2; // for audible alerts like touch beep confirmations
  AudioConnection_F32     patchCord4w(sinewave1,0,  FFT_Switch1,2);
  AudioConnection_F32     patchCord4x(sinewave2,0,  FFT_Switch1,3);
  AudioConnection_F32     patchCord4y(sinewave1,0,  FFT_Switch2,2);
  AudioConnection_F32     patchCord4z(sinewave2,0,  FFT_Switch2,3);
  // patch through the sinewave to the headphones/lineout
  AudioConnection_F32     patchCord4u(sinewave1,0,     Output,0);
  AudioConnection_F32     patchCord4v(sinewave2,0,     Output,1);
#else
  // patch through the audio input to the headphones/lineout
  AudioConnection_F32     patchCord4c(Input,0,     Output,0);
  AudioConnection_F32     patchCord4d(Input,1,     Output,1);
#endif
// patch through the input to a audio switch to select combinations of audio in and test tone
AudioConnection_F32         patchCord7a(Input,0,         FFT_Switch1,0);
AudioConnection_F32         patchCord7b(Input,1,         FFT_Switch2,0);
AudioConnection_F32         patchCord6a(Input,0,         FFT_Switch1,1);
AudioConnection_F32         patchCord6b(Input,1,         FFT_Switch2,1);
// Pass audio to the FFT to create data for our spectrum
AudioConnection_F32         patchCord5a(FFT_Switch1,0,   myFFT,0);
AudioConnection_F32         patchCord5b(FFT_Switch2,0,   myFFT,1);

AudioControlSGTL5000    codec1;

Spectrum_RA887x spectrum_RA887x(0, FFT_SIZE);     // initialize the Spectrum Librar

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println(F("Initializing SDR_RA887x Program\n"));
    Serial.println(F("**** Running I2C Scanner ****"));

    // ---------------- Setup our basic display and comms ---------------------------
    Wire.begin();
    Wire.setClock(100000UL); // Keep at 100K I2C bus transfer data rate for I2C Encoders to work right
    I2C_Scanner();

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
            tft.print("you should open RA8875UserSettings.h file and uncomment USE_FT5206_TOUCH!");
        #endif  // USE_RA8875
    #endif // USE_FT5206_TOUCH
    
    spectrum_RA887x.initSpectrum(spectrum_preset); // Call before initDisplay() to put screen into Layer 1 mode before any other text is drawn!

    initDSP();
    
    // -------------------- Setup our radio settings and UI layout --------------------------------

    spectrum_preset = 0;    

    spectrum_RA887x.Spectrum_Parm_Generator(0, 0); // use this to generate new set of params for the current window size values. 
                                                              // 1st arg is new target layout record - usually 0 unless you create more examples
                                                              // 2nd arg is current empty layout record (preset) value - usually 0
                                                              // calling generator before drawSpectrum() will create a new set of values based on the globals
                                                              // Generator only reads the global values, it does not change them or the database, just prints the new params                                                             
    spectrum_RA887x.drawSpectrumFrame(spectrum_preset); // Call after initSpectrum() to draw the spectrum object.  Arg is 0 PRESETS to load a preset record
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
    }
    else
    {
        codec1.muteHeadphone();
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
    static uint32_t time_old = 0;
    
    // Update spectrum and waterfall based on timer
    if (spectrum_waterfall_update.check() == 1) // The update rate is set in drawSpectrumFrame() with spect_wf_rate from table
    {
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
    
    //respond to Serial commands
    while (Serial.available())
    {
        if (Serial.peek())
            respondToByte((char)Serial.read());
    }

    //check to see whether to print the CPU and Memory Usage
    if (enable_printCPUandMemory)
        printCPUandMemory(millis(), 3000); //print every 3000 msec

    //check to see whether to print the CPU and Memory Usage
    if (enable_printCPUandMemory)
        printCPUandMemory(millis(), 3000); //print every 3000 msec
}

//
// ----------------------- SUPPORT FUNCTIONS -------------------------------------------------------
//
//  Added in some useful tools for measuring main program loop time, memory usage, and an I2C scanner.
//  These are also used in my SDR program.
//
// -------------------------------------------------------------------------------------------------
//  Scans for any I2C connected devices and reports them to the serial terminal.  Usually done early in startup.
//
void I2C_Scanner(void)
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

const char* formatVFO(uint32_t vfo)
{
  static char vfo_str[25];
  
  uint16_t MHz = (vfo/1000000 % 1000000);
  uint16_t Hz  = (vfo % 1000);
  uint16_t KHz = ((vfo % 1000000) - Hz)/1000;
  sprintf(vfo_str, "%6d.%03d.%03d", MHz, KHz, Hz);
  //sprintf(vfo_str, "%13s", "45.123.123");
  //Serial.print("New VFO: ");Serial.println(vfo_str);
  return vfo_str;
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
void printHelp(void)
{
    Serial.println();
    Serial.println(F("Help: Available Commands:"));
    Serial.println(F("   h: Print this help"));
    Serial.println(F("   C: Toggle printing of CPU and Memory usage"));
    Serial.println(F("   M: Print Detailed Memory Region Usage Report"));
    Serial.println(F("   T+10 digits: Time Update. Enter T and 10 digits for seconds since 1/1/1970"));
}

//--------------------------   Setup our Audio System -------------------------------------
void initDSP()
{
    AudioMemory_F32(100, audio_settings);
    codec1.enable(); // MUST be before inputSelect()
    delay(5);
    codec1.dacVolumeRampDisable(); // Turn off the sound for now
    codec1.inputSelect(myInput);
    codec1.lineOutLevel(13); // range 13 to 31.  13 => 3.16Vp-p, 31=> 1.16Vp-p
    codec1.autoVolumeControl(2, 0, 0, -36.0, 12, 6);                   // add a compressor limiter
    //codec1.autoVolumeControl( 0-2, 0-3, 0-1, 0-96, 3, 3);
    //autoVolumeControl(maxGain, response, hardLimit, threshold, attack, decay);
    codec1.autoVolumeEnable(); // let the volume control itself..... poor mans agc
    //codec1.autoVolumeDisable();// Or don't let the volume control itself
    codec1.muteLineout(); //mute the audio output until we finish thumping relays 
    codec1.adcHighPassFilterDisable();
    codec1.dacVolume(0); // set the "dac" volume (extra control)

    #ifdef TEST_SINE
    // Insert a test tone to see something on the display.
    float sinewave_vol = 0.001;
    sinewave1.amplitude(sinewave_vol);  // tone volume (0 to 1.0)
    sinewave1.frequency(1000.000); // Tone Frequency in Hz
    sinewave2.amplitude(sinewave_vol);  // tone volume (0 to 1.0)
    sinewave2.frequency(4000.000); // Tone Frequency in Hz
    #endif
    
    // Select our sources for the FFT.  mode.h will change this so CW uses the output (for now as an experiment)
    AudioNoInterrupts();
    FFT_Switch1.gain(0, 1.0f); //  1.0f is Input source before filtering, 0.0f is off,
    FFT_Switch1.gain(1, 0.0f); //  1.0f is CW Filtered (output), 0.0f is off
    FFT_Switch2.gain(0, 1.0f); //  1.0f is Input source before filtering, 0.0f is off,
    FFT_Switch2.gain(1, 0.0f); //  1.0f is CW Filtered (output), 0.0f is off
    #ifdef TEST_SINE
      FFT_Switch1.gain(2, 1.0f); //  1.0f  Sinewave1 to FFT for test cal, 0.0f is off
      FFT_Switch1.gain(3, 1.0f); //  1.0f  Sinewave2 to FFT for test cal, 0.0f is off
      FFT_Switch2.gain(2, 1.0f); //  1.0f  Sinewave1 to FFT for test cal, 0.0f is off
      FFT_Switch2.gain(3, 1.0f); //  1.0f  Sinewave2 to FFT for test cal, 0.0f is off
    #else
      FFT_Switch1.gain(2, 0.0f); //  1.0f  Sinewave1 to FFT for test cal, 0.0f is off
      FFT_Switch1.gain(3, 0.0f); //  1.0f  Sinewave2 to FFT for test cal, 0.0f is off
      FFT_Switch2.gain(2, 0.0f); //  1.0f  Sinewave1 to FFT for test cal, 0.0f is off
      FFT_Switch2.gain(3, 0.0f); //  1.0f  Sinewave2 to FFT for test cal, 0.0f is off
    #endif
    AudioInterrupts();
    
    // Choose our output type.  Can do dB, RMS or power
    myFFT.setOutputType(FFT_DBFS); // FFT_RMS or FFT_POWER or FFT_DBFS
    // Uncomment one these to try other window functions
    //  myFFT.windowFunction(NULL);
    //  myFFT.windowFunction(AudioWindowBartlett1024);
    //  myFFT.windowFunction(AudioWindowFlattop1024);
    myFFT.windowFunction(AudioWindowHanning1024);
    myFFT.setNAverage(3); // experiment with this value.  Too much causes a large time penalty
  }
