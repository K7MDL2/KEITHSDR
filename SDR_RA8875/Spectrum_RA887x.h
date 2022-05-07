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
#ifndef SPECTRUM_RA887x_H_
#define SPECTRUM_RA887x_H_
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
#include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
#include <Metro.h>              // GitHub https://github.com/nusolar/Metro

// Vars from main program.  Eventually pass these into function at run time.
extern struct Spectrum_Parms        Sp_Parms_Def[]; // The main program should have at least 1 layout record defined 
extern struct New_Spectrum_Layout   Custom_Layout[1];

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

int32_t spectrum_update(int16_t s, int16_t VFOA_YES, int32_t VfoA, int32_t VfoB, int32_t Offset, uint16_t filterCenter, uint16_t filterBandwidth, float pan, uint16_t zoom_fft_size, float fft_bin_sz, int16_t fft_binc);
void Spectrum_Parm_Generator(int16_t parm_set, int16_t preset, uint16_t fft_binc);
void drawSpectrumFrame(uint8_t s);
void initSpectrum(int16_t preset);
void setActiveWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB);
void setActiveWindow_default(void);
void updateActiveWindow(bool full);

#endif