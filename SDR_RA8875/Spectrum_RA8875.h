#ifndef _SPECTRUM_RA8875_H_
#define _SPECTRUM_RA8875_H_
//
// Spectrum_RA8875.h
//
// Draws the spectrum display in a resizable window area at any reasonable X,Y coordinate. 
// Scrolls the waterfall and draws a spectrum chart or dot style
// Depends on Layer1 mosde and BTE capability of hte RA8875 display controller
// Most of this will work unchanged in other displays provided the waterfall scroll effect can be worked out another way and the 2-3 lines of code yused forf waterfall scrlling here are modified to suit..  
// The RA8875 BTE approach means the CPU only draws 1 line of colors 1 pixel high per scheduleed update and issues 2 block mnove commands whcih copy the block to layer 2, then moves it back 1 row lower on layer 1. 
// The controller does all the work this way, no large data transfers required.
//
//
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
#include "SDR_RA8875.h"
#include "RadioConfig.h"

#ifdef ENET
    extern uint8_t enet_ready;
    //extern unsigned long enet_start_fail_time;
    //extern uint8_t rx_count;
    extern uint8_t enet_data_out;
#endif

#define myLT_GREY               RA8875_LIGHT_GREY 
#define myBLUE                  RA8875_BLUE
#define myBLACK                 RA8875_BLACK
#define myWHITE                 RA8875_WHITE
#define myYELLOW                RA8875_YELLOW
#define myGREEN                 RA8875_GREEN

#define FFT_AXIS                2
    // Set the FFT bin order to our needs. Called in drawSpectrumframe()
    // used for myFFT.setXAxis(FFT_AXIS);   //Note from Bob W7PUA
    // On ordering of the frequencies, this ends up being dependent on the mixer wiring. 
    // If you switch the ends on a transformer and you switch + and - frequencies.  
    // So, I decided to make the order from the FFT programmable.  
    // There is a new function setXAxis(uint8_t xAxis);  It follows these rules:
    //  If xAxis=0  f=fs/2 in middle, f=0 on right edge   - fc on left
    //  If xAxis=1  f=fs/2 in middle, f=0 on left edge    - Fc on left
    //  If xAxis=2  f=fs/2 on left edge, f=0 in middle 
    //  If xAxis=3  f=fs/2 on right edge, f=0 in middle
    //  The default is 3 (requires no call)

#define FFT_SIZE                4096 //2048//1024        // need a constant for array size declarion so manually set this value here   Could try a macro later

//function declarations
void Spectrum_Parm_Generator(int16_t parm_set);
void spectrum_update(int16_t s);
void drawSpectrumFrame(uint8_t s);
void initSpectrum(void);
int16_t colorMap(int16_t val, int16_t color_temp);
int16_t find_FFT_Max(uint16_t bin_min, uint16_t bin_max);
const char* formatFreq(uint32_t Freq);
void setActiveWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB);
void setActiveWindow();

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
    int16_t spect_wf_rate;    // Used by external timer to control refresh rate for this layout. drawSpectrumFRame() will read this and set the timer
};

// use the generator function to create 1 set of data to define preset values for window size and placement.  
// Just copy and paste from the serial terminal into each record row.
#define PRESETS 12  // number of parameter records with our preset spectrum window values

#endif  // _SPECTRUM_RA8875_H_
