//
// Spectrum_RA887x.h
//
// Spectrum_RA887x Library draws the spectrum display in a resizable window area at any reasonable X,Y coordinate
// This version is optimized for the RA8875 or RA8876 displays.  
// The #define USE_RA8875 determines which display is used, they use slightly different libraries, function calls, parameters
// This #define is normally specified in your main program (RadioConfig.h in my SDR_8875 program)
// Scrolls the waterfall and draws a spectrum chart or dot style
// Depends on Layer1 mosde and BTE capability of hte RA8875 display controller
// Most of this will work unchanged in other displays provided the waterfall scroll effect can be worked out another way and the 2-3 lines of code yused forf waterfall scrlling here are modified to suit..  
// The RA8875/76 BTE approach means the CPU only draws 1 line of colors 1 pixel high per scheduleed update and issues 2 block mnove commands whcih copy the block to layer 2, then moves it back 1 row lower on layer 1. 
// The controller does all the work this way, no large data transfers required.
//
//
#ifndef _SPECTRUM_RA887x_H
#define _SPECTRUM_RA887x_H
/*  Some of the waterfall averaging code and colorMap() function used in this spectrum.h were adopted from Waterfall example in the Arduino Audio library Analysis examples.
 *   
 *  Waterfall Audio Spectrum Analyzer, adapted from Nathaniel Quillin's
   award winning (most over the top) Hackaday SuperCon 2015 Badge Hack.

   https://hackaday.io/project/8575-audio-spectrum-analyzer-a-supercon-badge
   https://github.com/nqbit/superconbadge

   ILI9341 Color TFT Display is used to display spectral data.
   Two pots on analog A2 and A3 are required to adjust sensitivity.

   Copyright (c) 2015 Nathaniel Quillin

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <Arduino.h>
#include <OpenAudio_ArduinoLibrary.h> // F32 library located on GitHub. https://github.com/chipaudette/OpenAudio_ArduinoLibrary

// USER DEFINED SECTION --------------------------------------------------------------------//

#define USE_RA8875                 // Uncomment for RA8876 AND in your main program
// Vars from main program.  Eventually pass these into function at run time.
extern struct Spectrum_Parms        Sp_Parms_Def[]; // The main program should have at least 1 layout record defined 
extern struct New_Spectrum_Layout   Custom_Layout[1];

// Choose 1024, 2048, or 4096 - usually defined in the main program
////#define FFT_SIZE            2048   

// --->>>> Enable one or more FFT pipelines
// Ideally this would be automatically chosen from the main program when we figure out how to do that.
#define FFT_4096
//#define FFT_2048
//#define FFT_1024
// --->>>><<<<<---- //

#ifdef FFT_4096
    extern AudioAnalyzeFFT4096_IQ_F32  myFFT_4096;  // choose which you like, set FFT_SIZE accordingly.
#endif
#ifdef FFT_2048    
    extern AudioAnalyzeFFT2048_IQ_F32  myFFT_2048;
#endif
#ifdef FFT_1024
    extern AudioAnalyzeFFT1024_IQ_F32  myFFT_1024;
#endif

// END of USER DEFINED SECTION ------------------------------------------------------------//

//  Usually also defined in main program header file such as RadioConfig.h for SDR_887x program
#ifdef USE_RA8875
  #define  SCREEN_WIDTH       800 
  #define  SCREEN_HEIGHT      480
  //#define  RA8875_INT        14   //any pin
  //#define  RA8875_CS         10   //any digital pin
  //#define  RA8875_RESET      9    //any pin or nothing!
  //#define  MAXTOUCHLIMIT     3    //1...5  using 3 for 3 finger swipes, otherwise 2 for pinches or just 1 for touch
  #include <SPI.h>                // included with Arduino
  #include <RA8875.h>           // internal Teensy library with ft5206 cap touch enabled in user_setting.h
  extern RA8875 tft;
#else 
  //
  //--------------------------------- RA8876 LCD TOUCH DISPLAY INIT & PINS --------------------------
  //
  #define USE_RA8876_t3
  //
  #define  SCREEN_WIDTH       1024 
  #define  SCREEN_HEIGHT      600
  #include <RA8876_t3.h>           // Github
  //#include <FT5206.h>
  //#define  CTP_INT           14   // Use an interrupt capable pin such as pin 2 (any pin on a Teensy)
  //#define  RA8876_CS         10   //any digital pin
  //#define  RA8876_RESET      9    //any pin or nothing!
  //#define  MAXTOUCHLIMIT     3    //1...5  using 3 for 3 finger swipes, otherwise 2 for pinches or just 1 for touch
  
  // From RA8875/_settings/RA8875ColorPresets.h
  // Colors preset (RGB565)
  const uint16_t	RA8875_BLACK            = 0x0000;
  const uint16_t 	RA8875_WHITE            = 0xFFFF;
  const uint16_t	RA8875_RED              = 0xF800;
  const uint16_t	RA8875_GREEN            = 0x07E0;
  const uint16_t	RA8875_BLUE             = 0x001F;
  const uint16_t 	RA8875_CYAN             = RA8875_GREEN | RA8875_BLUE; //0x07FF;
  const uint16_t 	RA8875_MAGENTA          = 0xF81F;
  const uint16_t 	RA8875_YELLOW           = RA8875_RED | RA8875_GREEN; //0xFFE0;  
  const uint16_t 	RA8875_LIGHT_GREY 		  = 0xB5B2; // the experimentalist
  const uint16_t 	RA8875_LIGHT_ORANGE 	  = 0xFC80; // the experimentalist
  const uint16_t 	RA8875_DARK_ORANGE 		  = 0xFB60; // the experimentalist
  const uint16_t 	RA8875_PINK 			      = 0xFCFF; // M.Sandercock
  const uint16_t 	RA8875_PURPLE 			    = 0x8017; // M.Sandercock
  const uint16_t 	RA8875_GRAYSCALE 		    = 2113; //grayscale30 = RA8875_GRAYSCALE*30
  
  extern RA8876_t3 tft;
  int16_t	_activeWindowXL = 0;
  int16_t _activeWindowXR = SCREEN_WIDTH;
  int16_t	_activeWindowYT = 0;
  int16_t _activeWindowYB = SCREEN_HEIGHT;
#endif // USE_RA8876_t3

struct Spectrum_Parms {
    int16_t wf_sp_width;        // User specified active graphing area width with no padding. Max is fft_bins, can be smaller.
    int16_t border_space_min;   // Left and right side minimum border space. Total width minimum is graph width*2*border_space_minimum.
    int16_t border_space;       // Self-calculated value. Border padding size used on both sideds fo graphing area 
    int16_t l_graph_edge;       // Self calculated. Left side opf active graph
    int16_t r_graph_edge;       // Self calculated. Right side opf active graph
    int16_t c_graph;            // Self calculated. Center of graph window area
    int16_t sp_txt_row_height;  // Self calculated. Space for span frequency marker labels
    int16_t tick_height;        // Self calculated. Frequency markers top and bottom of display regions. 
    int16_t sp_txt_row;         // Self calculated. Spectrum text line below the top of space we have 
    int16_t sp_tick_row;        // Self calculated. Bottom of tick mark rectangle space that is 1 space below text
    int16_t sp_top_line;        // Self calculated. Spectrum top of graphing active area or window    
    int16_t wf_tick_row;        // Self calculated. Bottom of tick mark rectangle space that is 1 space below text
    int16_t wf_bottom_line;     // Self calculated. Bottom line of waterfall window
    int16_t sp_height;          // Self calculated. Account for span label and tic mark space outside of spectrum and waterfall
    int16_t wf_height;          // Self calculated. Waterfall height is what height is left after all other spaces have been figured
    int16_t sp_bottom_line;     // Self calculated. Bottom of spectrum window
    int16_t wf_top_line;        // Self calculated. Top Line of waterfall window
    int16_t spect_x;            // User specified X coordinate of the NE anchor point of outermost spectrum object box
    int16_t spect_y;            // User specified Y coordinate of the NE anchor point of outermost spectrum object box
    int16_t spect_width;        // User specified overall width and height
    int16_t spect_height;       // User specified overall height.  All other heights are calculated to fit within this box.
    int16_t spect_center;       // User specified center ratio of the line dividing the spectrum and waterfall.  SMaller  - smaller spectrum, bigger waterfall.
    int16_t spect_span;         // User specified span width.  The actual box width may not allow so it becomes best effort
    int16_t spect_wf_style;     // User specified waterfall averaging algorithym to use
    int16_t spect_wf_colortemp; // User specified colorization of the data in the waterfall.  Experimentally decided value.
    float   spect_wf_scale;     // User specified requested waterfall zoom level actual size or sample rate may cause best effort
    float   spect_LPFcoeff;     // User specified data smoothing factor.
    int16_t spect_dot_bar_mode; // User specified spectrum presentation mode. 0=BAR, 1=DOT, 2=LINE - Line is experimental line draw attempt.  Might be acheived via dot averaging
    int16_t spect_sp_scale;     // User specified vertical spectrum scale in dB.  This is to be the top dBm leel in the box.  
                                // Ths is usually between 10 and 80dB. Limited by spectrum_scale_maxdB and spectrum_scale_mindB vars
                                // The diff between this and box bottom results in scaling (zoom). If peaks occur outside the box bounds then they are not drawn.
    int16_t spect_floor;        // Slides the data up and down relative to the specrum bottom box line. The noise floor may be above or below and if outside the box is simply not drawn.
    int16_t spect_wf_rate;      // Used by external timer to control refresh rate for this layout. drawSpectrumFRame() will read this and set the timer
};

// The main program should define at least 1 layout record based on this structure.
// Here is a working example usually placed in your main program header files.
/*
struct Spectrum_Parms Sp_Parms_Def[PRESETS] = { // define default sets of spectrum window parameters, mostly for easy testing but could be used for future custom preset layout options
  //W LE  RE  CG x   y   w  h  c sp st clr sc mode scal reflvl wfrate
  #ifdef USE_RA8875
    {798,0, 0,  0,798,398,14,8,157,179,179,408,400,110,111,289,289,  0,153,799,256,50,20,6,240,1.0,0.9,1,20, 8, 70},
  #else
    {1020,1,1,  1,1021,510,14,8,143,165,165,528,520,142,213,307,307,  0,139,1022,390,40,20,6,890,1.5,0.9,1,20,10, 80},
  #endif
};
*/

struct New_Spectrum_Layout {      // Temp storage for generating new layouts    
      int16_t spectrum_x;             // 0 to width of display - window width. Must fit within the button frame edges left and right
                                          // ->Pay attention to the fact that position X starts with 0 so 100 pixels wide makes the right side value of x=99.
      int16_t spectrum_y;             // 0 to vertical height of display - height of your window. Odd Numbers are best if needed to make the height an even number and still fit on the screen
      int16_t spectrum_height;        // Total height of the window. Even numbers are best. (height + Y) cannot exceed height of the display or the window will be off screen.
      int16_t spectrum_center;        // Value 0 to 100.  Smaller value = biggger waterfall. Specifies the relative size (%) between the spectrum and waterfall areas by moving the dividing line up or down as a percentage
                                          // Smaller value makes spectrum smaller, waterfall bigger
      int16_t spectrum_width;         // Total width of window. Even numbers are best. 552 is minimum to fit a full 512 pixel graph plus the min 20 pixel border used on each side. Can be smaller but will reduce graph area
      int16_t spectrum_span;          // Value in KHz.  Ths will be the maximum span shown in the display graphs.  
                                          // The graph code knows how many Hz per bin so will scale down to magnify a smaller range.
                                          // Max value and resolutoin (pixels per bin) is dependent on sample frequency
                                          // 25000 is max for 1024 FFT with 500 bins at 1:1 bins per pixel
                                          // 12500 would result in 2 pixels per bin. Bad numbers here should be corrected to best fit by the function
      int16_t spectrum_wf_style;      // Range 1- 6. Specifies the Waterfall style.
      int16_t spectrum_wf_colortemp;  // Range 1 - 1023. Specifies the waterfall color temperature to tune it to your liking
      float   spectrum_wf_scale;      // 0.0f to 40.0f. Specifies thew waterfall zoom level - may be redundant when Span is worked out later.
      float   spectrum_LPFcoeff;      // 1.0f to 0.0f. Data smoothing
      int16_t spectrum_dot_bar_mode;      // 0=bar, 1=Line. Spectrum box
      int16_t spectrum_sp_scale;      // 10 to 80. Spectrum scale factor in dB. This is the height of the scale (if possible by windows sizes). Will plot the spectrum window of values between the floor and the scale value creating a zoom effect.
      int16_t spectrum_floor;         // 0 to -150. The reference point for plotting values.  Anything signal value > than this (less negative) will be plotted until stronger than the window height*scale factor.
      int16_t spectrum_wf_rate;       // window update rate in ms.  25 is fast enough to see dit and dahs well    
};

class Spectrum_RA887x
{
    //  ToDo: make work again to dump FFT data over ethernet.  Worked before moving to library.
    //#ifdef ENET
      //  extern uint8_t enet_ready;
        //extern unsigned long enet_start_fail_time;
        //extern uint8_t rx_count;
      //  extern uint8_t enet_data_out;
    //#endif

    public:
      Spectrum_RA887x(             
        uint16_t sp_fft_size,
        int16_t sp_fft_bins,
        float sp_fft_bin_size)  // end of args
        // 
        //   ---   Place functions here if needed    ---
        //
        {
            // Copy arguments to local variables
            // 
            fft_size      = sp_fft_size;   // Array size declaration.  Can override compile #define but will break if different
            fft_bins      = sp_fft_bins;
            fft_bin_size  = sp_fft_bin_size;
        }
        // publish externally avaialble functions
        int32_t spectrum_update(int16_t s, int16_t VFOA_YES, int32_t VfoA, int32_t VfoB, int32_t Offset, uint16_t filterCenter, uint16_t filterBandwidth);
        FLASHMEM void Spectrum_Parm_Generator(int16_t parm_set, int16_t preset);
        FLASHMEM void drawSpectrumFrame(uint8_t s);
        FLASHMEM void initSpectrum(int16_t preset);
        void setActiveWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB);
        void setActiveWindow_default(void);
        void updateActiveWindow(bool full);
    
    private:  
      uint16_t fft_size   = 0;
      int16_t fft_bins    = 0;
      float fft_bin_size  = 0;

      const uint16_t myLT_GREY  = RA8875_LIGHT_GREY;
      const uint16_t myBLUE     = RA8875_BLUE;
      const uint16_t myBLACK    = RA8875_BLACK;
      const uint16_t myWHITE    = RA8875_WHITE;
      const uint16_t myYELLOW   = RA8875_YELLOW;
      const uint16_t myGREEN    = RA8875_GREEN;
      const uint16_t myRED      = RA8875_RED;

      const uint16_t FFT_AXIS  = 2;
      uint8_t fft_axis = FFT_AXIS;
      // Set the FFT bin order to our needs. Called in drawSpectrumframe()
      // used for sp_FFT.setXAxis(FFT_AXIS);   
          // Note from Bob W7PUA
          // On ordering of the frequencies, this ends up being dependent on the mixer wiring. 
          // If you switch the ends on a transformer and you switch + and - frequencies.  
          // So, I decided to make the order from the FFT programmable.  
          // There is a new function setXAxis(uint8_t xAxis);  It follows these rules:
          //  If xAxis=0  f=fs/2 in middle, f=0 on right edge   - fc on left
          //  If xAxis=1  f=fs/2 in middle, f=0 on left edge    - Fc on left
          //  If xAxis=2  f=fs/2 on left edge, f=0 in middle 
          //  If xAxis=3  f=fs/2 on right edge, f=0 in middle
          //  The default is 3 (requires no call)
      // The default is 3 (requires no call) and I believe it is the same as the I16/F32 converted 256 code that Keith supplied, way back.  But that is not important.  Just change xAxis, call
      // in your INO setup(), or 0 or 1, and pretty soon it will be what you want, maybe.

      int16_t _colorMap(int16_t val, int16_t color_temp);
      int16_t _find_FFT_Max(uint16_t bin_min, uint16_t bin_max);
      char* _formatFreq(uint32_t Freq);
      // static uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);
      //inline uint16_t _Color565(uint8_t r, uint8_t g, uint8_t b);
      int16_t _waterfall_color_update(float sample, int16_t waterfall_low);
};

#endif  // _SPECTRUM_RA887xH
