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

#include <OpenAudio_ArduinoLibrary.h>
#include <AudioStream_F32.h>
#include <Audio.h>
#include <Metro.h> 
#include <RA8875.h>
#include <ili9488_t3_font_Arial.h>
#include <ili9488_t3_font_ArialBold.h>

int16_t wf_time_line = 5000;
int16_t fftFreq_refresh = 1000;
Metro waterfall_timestamp=Metro(wf_time_line);  // Used to draw a time stamp line across the waterfall window.  Cha
Metro fftFreq_timestamp = Metro(fftFreq_refresh);

#define myLT_GREY               RA8875_LIGHT_GREY 
#define myBLUE                  RA8875_BLUE
#define myBLACK                 RA8875_BLACK
#define myWHITE                 RA8875_WHITE
#define myYELLOW                RA8875_YELLOW
#define myGREEN                 RA8875_GREEN

// From main file where sampling rate and other audio library features are set
extern AudioAnalyzeFFT256_IQ_F32  myFFT;
extern int16_t                  fft_bins;    //Number of FFT bins. 1024 FFT has 512 bins for 50Hz per bin   (sample rate / FFT size)
extern float                    fft_bin_size;       
extern RA8875                   tft;
extern volatile uint32_t        Freq;                     

#define FFT_SIZE                256        // need a constant for array size declarion so manually set this value here   Could try a macro later
int16_t line_buffer[FFT_SIZE*2];            // Will only use the first x bytes defined by wf_sp_width var.  Could be 4096 FFT later which is larger than our width in pixels. 
int16_t spectrum_scale_maxdB    = 80;       // max value in dB above the spectrum floor we will plot signal values (dB scale max)
int16_t spectrum_scale_mindB    = 10;       // min value in dB above the spectrum floor we will plot signal values (dB scale max)
float   fftFrequency            = 0;        // Used to hold the FFT peak signal's frequency. Use a RF sig gen to measure its frequency and spot it on the display, useful for calibration
float   fftMaxPower             = 0;        // Used to hold the FFT peak power for the strongest signal

//function declarations
void Spectrum_Parm_Generator(int16_t parm_set);
void spectrum_update(int16_t s);
void drawSpectrumFrame(uint8_t s);
void initSpectrum_RA8875(void);
int16_t colorMap(int16_t val, int16_t color_temp);
void find_FFT_Max(void);

// Globals.  Generally these are only used to set up a new configuration set, or if a setting UI is built and the user is permitted to move and resize things.  
// These globals are othewise ignored
// See below commented section for ready made block of extern declarations to access these from elsewhere
// There are several predefined sets of window data Change these values and run run the program, open the Serial Monitor and cut and paste 
//    the generated window parameters into one of the records in the struct further below.  
// Preset is the index into array of structs.  The spectrum_update() fuinction takes as an arg the index so this is only for the generators until we get user customization UI later.

///******************************************************************************************************************************
//   *************  Set Preset to select your configuration record to use.  The rest are ignored for operation ******************
///
int16_t spectrum_preset         = 9;   // <<<==== Set this value.  Range is 0-PRESETS.  Specify the default layout option for spectrum window placement and size.
///
//
///******************************************************************************************************************************

// These values are only used to generate a new config record.  Cut and paste the results into the array to use.
int16_t spectrum_x              = 100;      // 0 to width of display - window width. Must fit within the button frame edges left and right
                                            // ->Pay attention to the fact that position X starts with 0 so 100 pixels wide makes the right side value of x=99.
int16_t spectrum_y              = 179;      // 0 to vertical height of display - height of your window. Odd Numbers are best if needed to make the height an even number and still fit on the screen
int16_t spectrum_height         = 300;      // Total height of the window. Even numbers are best. (height + Y) cannot exceed height of the display or the window will be off screen.
int16_t spectrum_center         = 40;       // Value 0 to 100.  Smaller value = biggger waterfall. Specifies the relative size (%) between the spectrum and waterfall areas by moving the dividing line up or down as a percentage
                                            // Smaller value makes spectrum smaller, waterfall bigger
int16_t spectrum_width          = 599;      // Total width of window. Even numbers are best. 552 is minimum to fit a full 512 pixel graph plus the min 20 pixel border used on each side. Can be smaller but will reduce graph area
float   spectrum_span           = 25000;    // UNUSED for now.  Value in Hz.  Ths will be the maximum span shown in the display graphs.  
                                            // The graph code knows how many Hz per bin so will scale down to magnify a smaller range.
                                            // Max value and resolutoin (pixels per bin) is dependent on sample frequency
                                            // 25000 is max for 1024 FFT with 500 bins at 1:1 bins per pixel
                                            // 12500 would result in 2 pixels per bin. Bad numbers here should be corrected to best fit by the function
int16_t spectrum_wf_style       = 2;        // Range 1- 6. Specifies the Waterfall style.
int16_t spectrum_wf_colortemp   = 90;      // Range 1 - 1023. Specifies the waterfall color temperature to tune it to your liking
float   spectrum_wf_scale       = 0.7;      // 0.0f to 40.0f. Specifies thew waterfall zoom level - may be redundant when Span is worked out later.
float   spectrum_LPFcoeff       = 0.9;      // 1.0f to 0.0f. Data smoothing
int16_t spectrum_dot_bar_mode   = 1;        // 0=bar, 1=DOT, 3=Line. Spectrum box . Line mode is experimental
int16_t spectrum_sp_scale       = 40;       // 10 to 80. Spectrum scale factor in dB. This is the height of the scale (if possible by windows sizes). Will plot the spectrum window of values between the floor and the scale value creating a zoom effect.
int16_t spectrum_floor          = 20;      // 0 to -150. The reference point for plotting values.  Anything signal value > than this (less negative) will be plotted until stronger than the window height*scale factor.
/*
 *   Copy some or all of this section to your main file to gain access to any or all of these for user controls   
 *   If just using the database predefined parameters, you can ignore these.  
 *   If you want to generate new wiondows parametes you can edit the above or copy these to another file and edit them in your own functions/Setting UI.  
 *
extern int16_t spectrum_preset;
extern int16_t spectrum_x;
extern int16_t spectrum_y;
extern int16_t spectrum_height;
extern int16_t spectrum_center;
extern int16_t spectrum_width;
extern float spectrum_span;
extern int16_t spectrum_wf_style;
extern int16_t spectrum_wf_colortemp;
extern float spectrum_wf_scale;
extern float spectrum_LPFcoeff;
extern int16_t spectrum_dot_bar_mode;
extern int16_t spectrum_sp_scale;
extern int16_t spectrum_floor;
*/

// use the generator finction to create 1 set of data to define preset values for window size and placement.  
// Just copy and paste from the serial terminal into each record row.
#define PRESETS 10  // number of parameter records with our preset spectrum window values

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
    float   spect_span;         // User specified span width.  The actual box width may not allow so it becomes best effort
    int16_t spect_wf_style;     // User specified waterfall averaging algorithym to use
    int16_t spect_wf_colortemp; // User specified colorization of the data in the waterfall.  Experimentally decided value.
    float   spect_wf_scale;     // User specified requested waterfall zoom level actual size or sample rate may cause best effort
    float   spect_LPFcoeff;     // User specified data smoothing factor.
    int16_t spect_dot_bar_mode; // User specified spectrum presentation mode. 0=BAR, 1=DOT, 2=LINE - Line is experimental line draw attempt.  Might be acheived via dot averaging
    int16_t spect_sp_scale;     // User specified vertical spectrum scale in dB.  This is to be the top dBm leel in the box.  
                                // Ths is usually between 10 and 80dB. Limited by spectrum_scale_maxdB and spectrum_scale_mindB vars
                                // The diff between this and box bottom results in scaling (zoom). If peaks occur outside the box bounds then they are not drawn.
    int16_t spect_floor;        // Slides the data up and down relative to the specrum bottom box line. The noise floor may be above or below and if outside the box is simply not drawn.
} Sp_Parms_Def[PRESETS] = { // define default sets of spectrum window parameters, mostly for easy testing but could be used for future custom preset layout options
    //W        LE  RE  CG                                            x   y   w  h   x  span   st clr  sc     mode scal reflvl
    {512,2,43,143,655,399,14,8, 74, 96, 96,479,471,225,150,321,321,100, 70,599,410,60,25000.0,3,2160,1.7,0.9,0,40,-210},   // Main full size window
    {500,2,49,150,650,400,14,8,133,155,155,478,470, 94,221,249,249,130,129,540,350,30,25000.0,2,550,1.0,0.9,1,30,-80}, // hal
    {512,2,43,143,655,399,14,8,354,376,376,479,471, 57, 38,433,433,100,350,599,130,60,25000.0,2,340,1.7,0.9,0,60,-250},  // Small wide bottom screen area to fit under pop up wndows.
    {396,2, 2,202,598,400,14,8,243,265,265,438,430, 99, 66,364,364,200,239,400,200,60,25000.0,2,310,1.7,0.9,0,60,-220},    //smaller centered
    {500,0,49,149,650,400,14,8,243,265,265,438,430, 82, 83,347,347,100,239,599,200,25,25000.0,3,950,2.0,0.7,1,40,-245},  // low wide high gain
    {500,2, 2,150,650,400,14,8,133,155,155,418,410,102,153,257,257,130,129,540,290,40,25000.0,2,320,1.0,0.9,1,30,-100},     //60-100 good
    {512,2,43,143,655,399,14,8,183,205,205,478,470,106,159,311,311,100,179,599,300,40,25000.0,0, 90,0.7,0.9,1,40,  40},     //60-100 good
    {512,2,43,143,655,399,14,8,223,245,245,348,340, 57, 38,302,302,100,219,599,130,60,25000.0,2,310,1.7,0.9,0,60,-200},
    {396,2, 2,102,498,300,14,8,243,265,265,438,430, 99, 66,364,364,100,239,400,200,60,25000.0,2,310,1.7,0.9,0,40,-220},
    {512,2,43,143,655,399,14,8,183,205,205,478,470,106,159,311,311,100,179,599,300,40,25000.0,2,450,0.7,0.9,1,40,30}
    }; 

struct Spectrum_Parms  Sp_Parms_Custom[PRESETS];

//
//------------------------------------------------------------  Waterfall and Spectrum  Display ------------------------------------------------------
//         
void spectrum_update(int16_t s)
{
//    s = The PRESET index into Sp_Parms_Def[] structure for windows location and size params  
//    Specify the default layout option for spectrum window placement and size.
//    
//    This function only uses values from the Sp_Parms_Def[] struct (later Sp_Parms_Custom[]).  To update the structure
//    records set the global variables the call the Spectrum_Generator() function and copy and paste the output displayed 
//    on the Serial Terminal into the default array init table.

    struct Spectrum_Parms *ptr = &Sp_Parms_Def[s];
    
    int16_t blanking = 3;  // used to remove the DC line from the graphs at Fc
    int16_t pix_o16;
    int16_t pix_n16;
    static int16_t spect_scale_last = 0;
    static int16_t spect_ref_last   = 0;
    static float fftFreq_max        = 0;
    static float fftPower_pk        = ptr->spect_floor;
    static float fftPower_pk_last   = ptr->spect_floor;
    static float sp_floor_avg       = ptr->spect_floor;   // stat out here.
        
    if (s >= PRESETS) s=PRESETS-1;   // Cycle back to 0
    // See Spectrum_Parm_Generator() below for details on Global values requires and how the woindows variables are used.    
    
    //for testing alignments
    //tft.drawRect(spectrum_x, spectrum_y, spectrum_width, spectrum_height, myBLUE);  // x start, y start, width, height, array of colors w x h
    //tft.drawRect(ptr->spect_x, ptr->spect_y, ptr->spect_width, ptr->spect_height, myBLUE);  // x start, y start, width, height, array of colors w x h
     
    int16_t i;
    float avg = 0.0;
    float pixelnew[FFT_SIZE*2];           //  Stores current pixel fopr spectrum portion only
    static float pixelold[FFT_SIZE*2];    //  Stores copy of current pixel so it can be erased in next update

    float tempfft[FFT_SIZE*2];    

    if (myFFT.available()) 
    {         
        float *pout = myFFT.getData();  // Get pointer to data array of powers, float output[512];

        for (i=0; i< FFT_SIZE/2; i++)        
            tempfft[i] = -500;
        for (i=0; i< FFT_SIZE/2; i++)        
            tempfft[i+FFT_SIZE] = *(pout+i);
        for (i=FFT_SIZE/2; i< FFT_SIZE; i++)
            tempfft[i] = *(pout+i);
        for (i=(FFT_SIZE/2)*3; i< FFT_SIZE*2; i++)        
            tempfft[i] = -500;
        pout = tempfft;

        //for (i = 2; i < (ptr->wf_sp_width-2)/2; i++)        // Grab all 512 values.  Need to do at one time since averaging is looking at many values in this array
        for (i = 2; i < (ptr->wf_sp_width-2); i++)        // Grab all 512 values.  Need to do at one time since averaging is looking at many values in this array
        { 
            if (isnanf(*(pout+i)) || isinff (*(pout+i)))    // trap float 'NotaNumber NaN" and Infinity values
            {
                //tft.print(" FFT Out of Bounds   ");
                Serial.println("FFT Invalid Data INF or NaN");
                //Serial.println(*(pout+i));                
                pixelnew[i] = -200;   // fill in the missing value with somting harmless
                //pixelnew[i] = myFFT.read(i+1);  // hope the next one is better.
            }
            // Now capture Spectrum value for use later
            pixelnew[i] = *(pout+i);

            // Several different ways to process the FFT data for display. Gather up a complete FFT sample to do averaging then go on to update the display with the results
            switch (ptr->spect_wf_style)
            { 
              case 0: avg = *(pout+((i*16/10)))*0.5 + *(pout+((i-1)*16/10))*0.18 + *(pout+((i-2)*16/10))*0.07 + *(pout+((i+1)*16/10))*0.18 + *(pout+((i+2)*16/10))*0.07;                
                      //line_buffer[i] = (LPFcoeff * 8 * sqrt (100+(abs(avg)*wf_scale)) + (1 - LPFcoeff) * line_buffer[i]);
                      line_buffer[i] = (ptr->spect_LPFcoeff * 8 * sqrt (100+(abs(*(pout+i)*ptr->spect_wf_scale))) + (1 - ptr->spect_LPFcoeff) * line_buffer[i]);                      
                      break;
              case 1: avg = *(pout+((i*16/10)))*0.5 + *(pout+((i-1)*16/10))*0.18 + *(pout+((i-2)*16/10))*0.07 + *(pout+((i+1)*16/10))*0.18 + *(pout+((i+2)*16/10))*0.07;                
                      line_buffer[i] = (ptr->spect_LPFcoeff * 8 * sqrt (abs(avg)*ptr->spect_wf_scale) + (1 - ptr->spect_LPFcoeff) * line_buffer[i]);
                      line_buffer[i] = colorMap(line_buffer[i]/1000, ptr->spect_wf_colortemp);
                      Serial.println(line_buffer[i]);    
                      break;                  
              case 2: avg = line_buffer[i] = colorMap(abs(*(pout+i)) * 1.8 *  ptr->spect_wf_scale, ptr->spect_wf_colortemp);                      
                      break;
              case 3: avg = line_buffer[i] = colorMap(abs(*(pout+i)) * 0.4 *  ptr->spect_wf_scale, ptr->spect_wf_colortemp);
                      break;
              case 4: avg = line_buffer[i] = colorMap(16000 - abs(*(pout+i)), ptr->spect_wf_colortemp) * ptr->spect_wf_scale;
                      break;
              case 5:
             default: avg = line_buffer[i] = colorMap(abs(*(pout+i)), ptr->spect_wf_colortemp) * ptr->spect_wf_scale * 0.1;                          
                      break; 
            };

            // Fc Blanking
            if (i >= (ptr->wf_sp_width/2)-blanking  && i <= (ptr->wf_sp_width/2)+blanking+1)
            {
                line_buffer[i] = myBLACK;    
                if (i == (ptr->wf_sp_width)/2)
                    line_buffer[i] = myLT_GREY;  // draw center Fc line in waterfall
            }
        }   // Done with copying the FFT output array


        // Takes a snapshot of the current window without the bottom row. Stores it in Layer 2 then brings it back beginning at the 2nd row. 
        //    Then write new row data into the missing top row to get a scroll effect using display hardware, not the CPU.
        //    Documentation for BTE: BTE_move(int16_t SourceX, int16_t SourceY, int16_t Width, int16_t Height, int16_t DestX, int16_t DestY, uint8_t SourceLayer=0, uint8_t DestLayer=0, bool Transparent = false, uint8_t ROP=RA8875_BTEROP_SOURCE, bool Monochrome=false, bool ReverseDir = false);                  
        tft.BTE_move(ptr->l_graph_edge+1, ptr->wf_top_line+1, ptr->wf_sp_width, ptr->wf_height-4, ptr->l_graph_edge+1, ptr->wf_top_line+2, 1, 2);  // Layer 1 to Layer 2
        while (tft.readStatus());  // Make sure it is done.  Memory moves can take time.
        
        // Move the block back on Layer 1 but place it 1 row down from the top
        tft.BTE_move(ptr->l_graph_edge+1, ptr->wf_top_line+2, ptr->wf_sp_width, ptr->wf_height-4, ptr->l_graph_edge+1, ptr->wf_top_line+2, 2);  // Move layer 2 up to Layer 1 (1 is assumed).  0 means use current layer.
        while (tft.readStatus());   // Make sure it is done.  Memory moves can take time.        
    
        // draw a periodic time stamp line
        if (waterfall_timestamp.check() == 1)
            //tft.drawRect(ptr->l_graph_edge+1, ptr->wf_top_line+1, ptr->wf_sp_width, 1, RA8875_GREEN);  // x start, y start, width, height, colors w x h           
            tft.drawFastHLine(ptr->l_graph_edge+1, ptr->wf_top_line+2, ptr->wf_sp_width, myLT_GREY);  // x start, y start, width, height, colors w x h           
        else  // Draw the new line at the top
            tft.writeRect(ptr->l_graph_edge+1, ptr->wf_top_line+1, ptr->wf_sp_width, 1, (uint16_t*) &line_buffer);  // x start, y start, width, height, array of colors w x h

        //
        //--------------------------------  Spectrum Window ------------------------------------------
        //
        // Done with waterfall, now draw the spectrum section
        
        for (i = 2; i < (ptr->wf_sp_width-2); i++)   // Add SPAN control to spread things out here.  Currently 10KHz per side span with 96K sample rate  
        //for  (i=2; i < (FFT_SIZE)-2; i++)   // Add SPAN control to spread things out here.  Currently 10KHz per side span with 96K sample rate  
        {       
            // average a few values to smooth the line a bit
            float avg_pix2 = (pixelnew[i]+pixelnew[i+1])/2;     // avg of 2 bins            
            float avg_pix5 = (pixelnew[i-2]+pixelnew[i-1]+pixelnew[i]+pixelnew[i+1]+pixelnew[i+2])/5; //avg of 5 bins
            
            if (abs(pixelnew[i]) > abs(avg_pix2) * 1.6f)    // compare to a small average to toss out wild spikes
                pixelnew[i] = avg_pix5;                     // average it out over a wider segment to patch the hole   

            if (i >= (ptr->wf_sp_width/2)-blanking-1 && i <= (ptr->wf_sp_width/2)+blanking+1)
                pixelnew[i] = -200; 

            //#define DBG_SPECTRUM_SCALE
            //#define DBG_SPECTRUM_PIXEL
            //#define DBG_SPECTRUM_WINDOWLIMITS
            
            #ifdef DBG_SPECTRUM_PIXEL
            Serial.print(" raw =");
            Serial.print(pixelnew[i],0);
            #endif
            
            // limit the upper and lower dB level to between these ranges (set scale) (User Setting)  Can be limited further by window heights   
            spectrum_scale_maxdB = 10;     //scale most zoomed in.  This is +10dB above the spectrum floor value.   That value is adjustables and is our refence point set to the bottom line.  
                                                //Forms the top range of values that line up with the top of our "window" on the FFT data set value range, typiclly -150 to -0dBm possible.
            spectrum_scale_mindB = 80;   // scale most zoomed out.  This is +80 dB relative to the spectrum_floor so teh top end of our window.  Typically -150 to -0dBm possible range of signal.             
                    // range limit our settings.    This number is added to teh spectrum floor.  The pixel value will be plotted where ever it lands as along as it is in the window.

            #ifdef DBG_SPECTRUM_SCALE
            Serial.print("   SC_ORG="); Serial.print(ptr->spect_sp_scale);                  
            #endif
        
            ptr->spect_sp_scale = constrain(ptr->spect_sp_scale, spectrum_scale_maxdB, spectrum_scale_mindB);

            #ifdef DBG_SPECTRUM_SCALE
            Serial.print("   SC_LIM="); Serial.print(ptr->spect_sp_scale);                  
            #endif
            
            // if the window height changes and becomes less than our possible window pixel height reduce the scale. 
            //if (abs(spect_floor + ptr->spect_sp_scale) > ptr->sp_height-4)
            //    ptr->spect_sp_scale = ptr->sp_height -4 - abs(ptr->spect_floor);
                
            #ifdef DBG_SPECTRUM_SCALE
            Serial.print("   SC_HT="); Serial.print(ptr->spect_sp_scale);
            Serial.print("   HT="); Serial.print(ptr->sp_height-4);
            Serial.print("   SC_FLR="); Serial.print(ptr->spect_floor);                  
            #endif       
            
            // Plot out pixel in the windows if it lands between the bottom line and top lines        
            // set the grass floor to just above the botom line.  These are the weakest signals. Typically -90 coming out of the FFT right now
            
            // find the strongest signal level in dB while we are here.  
            if (pixelnew[i] < fftPower_pk)
            {
                fftPower_pk = pixelnew[i];
                //Serial.println(pixelnew[i]);
            }

            // Invert the sign since the display is also inverted, Increasing value = weaker signal strength, they are now going the same direction.  
            // Small value = bigger signal, closer to 0 on the display coordinates
            pixelnew[i] = abs(pixelnew[i]);
            
            // Offset the pixel position relative to the bottom of the window
            pixelnew[i] += ptr->sp_bottom_line-2 + ptr->spect_floor;            

            // Now scale it            
            //pixelnew[i] = map(pixelnew[i], ptr->spect_floor, ptr->spect_floor+ptr->spect_sp_scale, ptr->sp_bottom_line-2, ptr->sp_top_line+1);    
            //pixelnew[i] = pixelnew[i] * (ptr->spect_sp_scale/10); // - ptr->spect_floor;
            pixelnew[i] *= ptr->spect_wf_scale;
            //Serial.println(pixelnew[i]);
            // Various averaging  effects.  Ultimately need an averaging feature as well as peak hold but needs calibration to be most useful                            
            //avg = myFFT.output[(i)*16/10]*0.5 + myFFT.output[(i-1)*16/10]*0.18 + myFFT.output[(i-2)*16/10]*0.07 + myFFT.output[(i+1)*16/10]*0.18 + myFFT.output[(i+2)*16/10]*0.07;
            //pixelnew[i] = (20*(abs(LPFcoeff * 0.1 * sqrt (abs(avg)*wf_scale) + (1 - LPFcoeff) * pixelold[i])));
            //pixelnew[i] = ptr->spect_LPFcoeff * 1 * sqrt (abs(avg)*ptr->spect_wf_scale) + (1 - ptr->spect_LPFcoeff) * pixelold[i];
            //pixelnew[i] = ptr->spect_LPFcoeff * 1 * line_buffer[i] * pixelold[i];
            //pixelnew[i] = avg;
            
            #ifdef DBG_SPECTRUM_WINDOWLIMITS
            //Serial.print("  win-ht:"); Serial.print(ptr->sp_height-4);
            Serial.print("  top line="); Serial.print(ptr->sp_top_line+2);
            #endif
            
            #if defined (DBG_SPECTRUM_PIXEL) || defined (DBG_SPECTRUM_WINDOWLIMITS)
            Serial.print("  pix ="); Serial.println(pixelnew[i],0);
            #endif

            #ifdef DBG_SPECTRUM_WINDOWLIMITS 
            Serial.print("  bottom line="); Serial.print(ptr->sp_bottom_line-2);
            #endif

            //#define DBG_SHOW_OVR
            #if defined(DBG_SPECTRUM_WINDOWLIMITS) || defined(DBG_SPECTRUM_PIXEL) || defined(DBG_SPECTRUM_SCALE) || defined(DBG_SHOW_OVR)
            if (pixelnew[i] < ptr->sp_top_line+2)        
            { Serial.print(" !!OVR!! = ");    Serial.println(pixelnew[i] - ptr->sp_top_line+2,0);}
            if (pixelnew[i] > ptr->sp_bottom_line-2)        
            { Serial.print(" !!UNDER!! = ");  Serial.println(pixelnew[i] - ptr->sp_top_line+2,0);}          
            #endif
            
            pix_n16 = (int16_t) round(pixelnew[i]);  // convert float to uint16_t to match the draw functions type
            pix_o16 = (int16_t) round(pixelold[i]);

            // Linit access to the spectrum box to control misbehaved pixel and bar draws
            tft.setActiveWindow(ptr->l_graph_edge+1, ptr->r_graph_edge-1, ptr->sp_top_line+2, ptr->sp_bottom_line-2);
            
            //
            //------------------------ Code below is writing only in the active spectrum window ----------------------
            //

            if (i < (ptr->wf_sp_width/2)-5 || i > (ptr->wf_sp_width/2) + 5)   // blank the DC carrier noise at Fc
            {    
                if ((i < ptr->wf_sp_width-2) && (pix_n16 > ptr->sp_top_line+2) && (pix_n16 < ptr->sp_bottom_line-2)  ) // will blank out the center spike
                {   
                    if (pixelnew[i] <  ptr->sp_bottom_line + 40) // arbitrary cutoff to look for low level avg to act as AGC for noise floor adjustment.
                    {
                        sp_floor_avg += pixelnew[i];             // accumulate the next low sig level value
                        sp_floor_avg /= (ptr->wf_sp_width-2);     // first run through wil be off but rest will be OK after
                        sp_floor_avg -= (ptr-> sp_bottom_line-2 - sp_floor_avg)/2;                                                
                        Serial.println(sp_floor_avg);
                    }
                    if (ptr->spect_dot_bar_mode == 0)   // BAR Mode 
                    {
                        if (pix_o16 != pix_n16) 
                        {                               
                            if (pix_n16 > ptr->sp_top_line+2 && pix_n16 < ptr->sp_bottom_line-2)
                            {
                                //common way: draw bars                                                                        
                                tft.drawFastVLine(ptr->l_graph_edge+i, pix_o16, ptr->sp_bottom_line-pix_o16,    myBLACK); // GREEN);
                                //tft.drawFastVLine(ptr->l_graph_edge+i, ptr->sp_top_line, pix_o16,    myBLACK); // BLACK);

                                tft.drawFastVLine(ptr->l_graph_edge+i, pix_n16, ptr->sp_bottom_line-pix_n16,    myYELLOW); //BLACK);                         
                                //tft.drawFastVLine(ptr->l_graph_edge+i, ptr->sp_top_line, pix_n16,    myBLACK); //BLACK);
                                pixelold[i] = pixelnew[i];
                            }
                        }
                    }
                    else if (ptr->spect_dot_bar_mode == 1)  // DOT mode
                    {   // DOT Mode
                        // alternate way: draw pixels
                        // only plot pixel, if at a new position
                        if (pix_o16 != pix_n16) 
                        {
                            if (pix_n16 > ptr->sp_top_line+2 && pix_n16 < ptr->sp_bottom_line-2)
                            {
                                /*
                                tft.drawPixel(ptr->l_graph_edge+1+i, pix_o16, myBLACK); // delete old pixel
                                tft.drawPixel(ptr->l_graph_edge+1+i, pix_n16, myYELLOW); // write new pixel    
                                                           
                                // Double up the dots to make the spectrum more dense and visible                                
                                tft.drawPixel(ptr->l_graph_edge+1+i, pix_o16-1, myBLACK); // delete old pixel
                                tft.drawPixel(ptr->l_graph_edge+1+i, pix_n16-1, myYELLOW); // write new pixel  
                                
                                // Triple up the dots to make the spectrum more dense and visible                                
                                tft.drawPixel(ptr->l_graph_edge+1+i, pix_o16-2, myBLACK); // delete old pixel
                                tft.drawPixel(ptr->l_graph_edge+1+i, pix_n16-2, myYELLOW); // write new pixel  
                                */
                                //Serial.println(tft.grandient( (uint16_t) pix_n16));
                                // Experimetnal  - draw a 2 pixel wide segment fopr more visibility
                                tft.drawFastHLine(ptr->l_graph_edge+1+i, pix_o16, 2,   myBLACK); //BLACK);                         
                                tft.drawFastHLine(ptr->l_graph_edge+1+i, pix_n16, 2,   myYELLOW); //BLACK);
                                //tft.drawFastVLine(ptr->l_graph_edge+1+i, pix_o16-1, 2,   myBLACK); //BLACK);                         
                                //tft.drawFastVLine(ptr->l_graph_edge+1+i, pix_n16-1, 2,   myYELLOW); //BLACK);                                

                                pixelold[i] = pixelnew[i]; 

                            }
                        }
                    }
                    // Under Dev - attempting to draw connecting line between dots  Lines are drawn all over for some reason TBD.
                    else if (pix_o16 != pix_n16) 
                    { 
                        //if ((pix_n16 > ptr->sp_top_line+10 && pix_n16 < ptr->sp_bottom_line-10) && 
                         //       (pix_o16 > ptr->sp_top_line+10 && pix_o16 < ptr->sp_bottom_line-10))
                        //{
                            //Serial.println(pixelnew[i]);

                            tft.drawLine(ptr->l_graph_edge+1+i,    (int16_t) pixelold[i-1],    ptr->l_graph_edge+1+i,   (int16_t) pixelold[i], myBLACK); // BLACK);                    
                            tft.drawLine(ptr->l_graph_edge+1+i,    (int16_t) pixelnew[i-1],    ptr->l_graph_edge+1+i,   (int16_t) pixelnew[i], myYELLOW); // GREEN);                    
                            
                            //tft.drawLine(ptr->l_graph_edge+1+i,  (int16_t) pixelold[i-1], myBLACK); // delete old pixel
                            //tft.drawLine(ptr->l_graph_edge+1+i,  (int16_t) pixelnew[i-1], myYELLOW); // write new pixel
                            //tft.drawLine(ptr->l_graph_edge+1+i,  pix_o16,     myBLACK); // delete old pixel                             
                            //tft.drawLine(ptr->l_graph_edge+1+i,  pix_n16,     myYELLOW); // write new pixel
                            pixelold[i] = pixelnew[i]; 
                        //}
                    }
                }
            }
            
            // Draw Grid Lines
            if (i == (ptr->wf_sp_width/2))
            {
            // draw a grid line for XX dB level             
                tft.setTextColor(myLT_GREY);
                tft.setFont(Arial_10);
                
                int grid_step = ptr->spect_sp_scale;
                for (int16_t j = grid_step; j < ptr->sp_height-10; j+=grid_step)
                {        
                    if (pix_n16 > ptr->sp_top_line+j+2 && pix_n16 < ptr->sp_bottom_line-2)
                    {    
                        // draw bottom most grid line
                        tft.drawFastHLine(ptr->l_graph_edge+24, ptr->sp_bottom_line-j,   ptr->wf_sp_width-24,    myLT_GREY); // GREEN);
                        // write the scale value for the grid line
                        tft.setCursor(ptr->l_graph_edge+5, ptr->sp_bottom_line-j-5);
                        tft.print(j); 
                    }
                }
                // redraw the center line
                //tft.drawFastVLine(ptr->c_graph, ptr->sp_top_line, ptr->sp_height-2 , myLT_GREY); //myLT_GREY);  
            }
        }
        tft.setActiveWindow();  // restore access to whole screen
        //
        //------------------------ Code above is writing only in the active spectrum window ----------------------
        //
        //-----------------------   This part onward is outside the active window ------------------------------
        //
        // Update graph scale, ref level, power and freq
        tft.setTextColor(myLT_GREY);
        tft.setFont(Arial_12);

        find_FFT_Max();   // get new frequency and power values for streongest signal 
        
        // Calculate and print the power of the strongest signal if possible
        //if (!isnanf(fftMaxPower) && !isinff(fftMaxPower) && 
        if (fftPower_pk != fftPower_pk_last)  // filter out invalid data and DC noise
        {           
            //Serial.print("Ppk="); Serial.println((fftPower_pk+fftPower_pk_last)/2-ptr->spect_sp_scale);       
            tft.fillRect(ptr->l_graph_edge+39,         ptr->sp_txt_row, 50, 13, RA8875_BLACK);  // clear the text space
            tft.setCursor(ptr->l_graph_edge+40,        ptr->sp_txt_row+30); // Write the legend
            tft.print("P: "); 
            tft.setCursor(ptr->l_graph_edge+54,        ptr->sp_txt_row+30);  // write the value
            tft.print((fftPower_pk+fftPower_pk_last)/2-ptr->spect_sp_scale,0);
            fftPower_pk_last = fftPower_pk;        
            fftFreq_timestamp.reset();  // reset the timer since we have new good data
        }
                
        // Calculate and print the frequency of the strongest signal if possible 
        if (!isnanf(fftFrequency) && !isinff(fftFrequency) && fftFrequency > 80.0f && fftFreq_max != fftFrequency)  // filter out invalid data and DC noise
        {
            //Serial.print("Freq="); Serial.println(fftFrequency, 3); 
            tft.fillRect(ptr->l_graph_edge+99,    ptr->sp_txt_row+30, 90, 13, RA8875_BLACK);
            tft.setCursor(ptr->l_graph_edge+100,  ptr->sp_txt_row+30);
            tft.print("F: "); 
            tft.setCursor(ptr->l_graph_edge+114,  ptr->sp_txt_row+30);
            //tft.print(Freq/1000 + fftFrequency/100,2);
            tft.print((Freq + fftFrequency)/1000,1);
            fftFreq_max = fftFrequency;
            fftFreq_timestamp.reset();  // reset the timer sicne we have new good data
        }
        
        if (fftFreq_timestamp.check() == 1)   // clear after no recent data
        {
            fftPower_pk = ptr->spect_floor;
            fftFreq_max = 0.0f;
        }    
        
        // Write the Scale value 
        if (spect_scale_last != ptr->spect_sp_scale)
        {         
            tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+50, ptr->sp_txt_row+30);
            tft.print("S:   "); // actual value is updated elsewhere   
            tft.fillRect( ptr->l_graph_edge+(ptr->wf_sp_width/2)+64, ptr->sp_txt_row+30, 32, 13, RA8875_BLACK);       
            tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+64, ptr->sp_txt_row+30);
            tft.print(ptr->spect_sp_scale);        
            spect_scale_last = ptr->spect_sp_scale;   // update memory
        }
        
        // Write the Reference Level to top line area
        if (spect_ref_last != ptr->spect_floor)
        {
            tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+100, ptr->sp_txt_row+30);
            tft.print("R:   ");  // actual value is updated elsewhere            
            tft.fillRect( ptr->l_graph_edge+(ptr->wf_sp_width/2)+114, ptr->sp_txt_row+30, 32, 13, RA8875_BLACK);
            tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+114, ptr->sp_txt_row+30);
            //tft.print(ptr->spect_floor);
            tft.print(sp_floor_avg,0);
            //spect_ref_last = ptr->spect_floor;   // update memory
            spect_ref_last = sp_floor_avg;   // update memory
        }    
            
        tft.setTextColor(myLT_GREY);
        tft.setFont(Arial_12);

        tft.fillRect( ptr->l_graph_edge, ptr->sp_txt_row, 80, 13, RA8875_BLACK);
        tft.setCursor(ptr->l_graph_edge, ptr->sp_txt_row);
        tft.print( (float) (Freq/1000) - (ptr->wf_sp_width*fft_bin_size/1000),1);       // Write left side of graph Freq
        
        tft.fillRect( ptr->c_graph-27, ptr->sp_txt_row, 80, 13, RA8875_BLACK);
        tft.setCursor(ptr->c_graph-27, ptr->sp_txt_row);
        tft.print( (float) (Freq/1000),1);   // Write center of graph Freq   
        
        tft.fillRect( ptr->r_graph_edge - 60, ptr->sp_txt_row, 80, 13, RA8875_BLACK);
        tft.setCursor(ptr->r_graph_edge - 60, ptr->sp_txt_row);
        tft.print( (float) (Freq/1000) + (ptr->wf_sp_width*fft_bin_size/1000),1);  // Write right side of graph Freq
        
        // Write the dB range of the window 
        tft.setTextColor(myLT_GREY);
        tft.setFont(Arial_10);
        tft.setCursor(ptr->r_graph_edge-40, ptr->sp_top_line+8);
        tft.print("H:   ");  // actual value is updated elsewhere
        tft.setCursor(ptr->r_graph_edge-28, ptr->sp_top_line+8);
        tft.print(ptr->sp_height);
    }  
}
//
//____________________________________________________Color Mapping _____________________________________
//       

int16_t colorMap(int16_t val, int16_t color_temp) 
{
    float red;
    float green;
    float blue;
    float temp = val / 65536.0 * color_temp;
  
    if (temp < 0.5) 
    {
        red = 0.0;
        green = temp * 2;
        blue = 2 * (0.5 - color_temp);
    } 
    else 
    {
        red = temp;
        green = (1.0 - color_temp);
        blue = 0.0;
    }
    //Serial.print("  CM="); Serial.print(tft.Color565(red * 256, green * 256, blue * 256));Serial.print("  val="); Serial.println(val);
    //return tft.color565(red * 256, green * 256, blue * 256);
    return tft.Color565(red * 256, green * 256, blue * 256);
}
//
//--------------------------------------------------------------------------------------------------------------------------------------------
//                                                              drawSpectrumFrame 
//--------------------------------------------------------------------------------------------------------------------------------------------
//
//   This uses an x,y anchor point (ne corner) and has a width based today on the FFT size.  
//   A 1024 FFT produces 512 bins and we are using 500 of them mapped 1:1 to a pixel.  At 51200Hz sample rate, that is 50Hz per pixel resolution. 
//   Can be used for initial setup and also for refresh after a user changes the location or size, or a pop-up page window is removed requiring a redraw
//
//
void drawSpectrumFrame(uint8_t s)
{
    // See Spectrum_Parm_Generator() below for details on Global values requires and how the woindows variables are used.
    
    // s = The PRESET index into Sp_Parms_Def[] structure for windows location and size params.  Specify the default layout option for spectrum window placement and size.
    if (s >= PRESETS) s=PRESETS-1;   // Cycle back to 0
    // Test lines for alignment

    struct Spectrum_Parms *ptr = &Sp_Parms_Def[s];
    
    tft.fillRect(ptr->spect_x, ptr->spect_y, ptr->spect_width, ptr->spect_height, myBLACK);  // x start, y start, width, height, array of colors w x h
    //tft.drawRect(ptr->spect_x, ptr->spect_y, ptr->spect_width, ptr->spect_height, myBLUE);  // x start, y start, width, height, array of colors w x h
    
    // This section updates the globals from the chosen preset
    // The drawing coordinates and sizes use the record values only, not from a global directly.
    // To change a window record, 
        // 1. Set the globals up
        // 2. Call the generator
        // 3. Cut anmd pastes the results into the default struct array
        // 4. Reboot.  DrawSpectrum does not read the globals but will update them to what it is using.
        // Therefore always call the generator before DraSpectrum to create a new set of params
    
    // Set up the Waterfall scroll box area 
    //These 2 lines are used to test alignments, normally leave them commented out.  The scroll region is over the same area
    tft.drawRect(ptr->l_graph_edge,    ptr->wf_top_line,    ptr->wf_sp_width+2,   ptr->wf_height,    myLT_GREY);  // x start, y start, width, height, array of colors w x h
    tft.fillRect(ptr->l_graph_edge+1,  ptr->wf_top_line+1,  ptr->wf_sp_width,     ptr->wf_height-2,  myBLACK);
    // Set the scroll region for the watefall.  We only need to write 1 new top line and block shift the rest down 1.
    //tft.setScrollWindow(l_graph_edge, r_graph_edge, wf_top_line+1, wf_bottom_line-1);  //Specifies scrolling activity area   XL, XR, Ytop, Ybottom
  
    // Set up the Spectrum box area
    tft.drawRect(ptr->l_graph_edge,      ptr->sp_top_line,      ptr->wf_sp_width+2,   ptr->sp_height,      myLT_GREY);  // x start, y start, width, height, array of colors w x h    
    tft.fillRect(ptr->l_graph_edge+1,    ptr->sp_top_line+1,    ptr->wf_sp_width,     ptr->sp_height-2,    myBLACK);
    
    // Draw Tick marks and Span Labels
    tft.drawLine(ptr->l_graph_edge+1,                        ptr->sp_tick_row,      ptr->l_graph_edge+1,                        ptr->sp_tick_row-ptr->tick_height, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+(ptr->wf_sp_width/4),     ptr->sp_tick_row,      ptr->l_graph_edge+(ptr->wf_sp_width/4),     ptr->sp_tick_row-ptr->tick_height, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+1+(ptr->wf_sp_width/2),   ptr->sp_tick_row,      ptr->l_graph_edge+1+(ptr->wf_sp_width/2),   ptr->sp_tick_row-ptr->tick_height, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+((ptr->wf_sp_width/4)*3), ptr->sp_tick_row,      ptr->l_graph_edge+((ptr->wf_sp_width/4)*3), ptr->sp_tick_row-ptr->tick_height, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+ptr->wf_sp_width-1,       ptr->sp_tick_row,      ptr->l_graph_edge+ptr->wf_sp_width-1,       ptr->sp_tick_row-ptr->tick_height, myLT_GREY);
    
    // draw the spectrum box center grid line for tuning help.
    tft.drawFastVLine(ptr->l_graph_edge+ptr->wf_sp_width/2+1, ptr->sp_top_line+1, ptr->sp_height, myLT_GREY); //BLACK);
    
    // Draw the ticks on the bottom of Waterfall window also
    tft.drawLine(ptr->l_graph_edge+1,                         ptr->wf_bottom_line,   ptr->l_graph_edge+1,                        ptr->wf_tick_row, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+(ptr->wf_sp_width/4),      ptr->wf_bottom_line,   ptr->l_graph_edge+(ptr->wf_sp_width/4),     ptr->wf_tick_row, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+1+(ptr->wf_sp_width/2),    ptr->wf_bottom_line,   ptr->l_graph_edge+1+(ptr->wf_sp_width/2),   ptr->wf_tick_row, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+(ptr->wf_sp_width/4)*3,    ptr->wf_bottom_line,   ptr->l_graph_edge+(ptr->wf_sp_width/4)*3,   ptr->wf_tick_row, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+ptr->wf_sp_width-1,        ptr->wf_bottom_line,   ptr->l_graph_edge+ptr->wf_sp_width-1,       ptr->wf_tick_row, myLT_GREY);    
}

//
//--------------------------------------------------  Spectrum init ------------------------------------------------------------------------
//
//   Must be called before any text is written tothe screen. If not that text will be corrupted.
//
void initSpectrum_RA8875(void)
{
    //tft.begin(RA8875_800x480);   // likely redundant but just in case and allows to be used standalone.

    // Setup for scrollig attributes
    tft.useLayers(1);       //mainly used to turn of layers!    
    tft.writeTo(L1);         //L1, L2, CGRAM, PATTERN, CURSOR
    tft.setScrollMode(LAYER1ONLY);    // One of these 4 modes {SIMULTANEOUS, LAYER1ONLY, LAYER2ONLY, BUFFERED }
}
//
//--------------------------------------------------  Spectrum parameter generator tool ------------------------------------------------------------------------
//
//   Call this tool to generate a set of parameters to setup a spectrum window and store then in the specified record set 
//
//   Input: int parm_set: range is 0-PRESETS. It is the index to teh array of structures storing the window's data set
//   
//  Output: struct Spectrum_Parms  Sp_Parms_Custom[PRESETS];   
//          Sp_Parms_Custom is an in-memory stucture with several records, each holding a set of outputs from this generator that
//          define all the window's coordinates and settings.
//     
//   Usage: 1.  Set your global variables to locate and size the spectrum window. 
//          2.  Call this tool.  The numbers will be printed on the serial terminal so you can you can copy and paste them
//              into the default records in the struct definition at the top of this file.
//   Notes: If a user customization is made during run time, you set the globals with their choices, run this generator and 
//          copy the results into the in-memory record (layout) of choice. 
//          To use the results on the fly,
//          1. Update spectrum_preset var, 
//          2. Call drawSpectrumFrame(spectrum_preset) and it will draw the new frame.
//          3. Call Spectrum_Update(spectrum_preset) to pick up the change immediately.
//          Changes will stored in EEPROM when the user or programmer uses the save Config action otherwise those values
//          will be lost on power off.
//
//          This will eventually be populated from EEPROM at startup.  
//          If no valid EEPROM is found then the Default table records are loaded.
//          
//  TODO:  Store data structure in EEPROM
//
void Spectrum_Parm_Generator(int16_t parm_set)
{
//    Globals Variables used:
//  int16_t spectrum_x              // 0 to width of display - window's NE corner. Must fit within the button frame edges left and right
//                                  // ->Pay attention to the fact that position X starts with 0 so 100 pixels wide makes the right side value of x=99.
//  int16_t spectrum_y              // 0 to vertical height of display - height of your window. Odd Numbers are best if needed to make the height an even number and still fit on the screen
//  int16_t spectrum_height         // Total height of the window. Even numbers are best. (height + Y) cannot exceed height of the display or the window will be off screen.
//  int16_t spectrum_center         // Value 0 to 100.  Smaller value = biggger waterfall. Specifies the relative size (%) between the spectrum and waterfall areas by moving the dividing line up or down as a percentage
//                                  // Smaller value makes spectrum smaller, waterfall bigger
//  int16_t spectrum_width          // Total width of window. Even numbers are best. 552 is minimum to fit a full 512 pixel graph plus the min 20 pixel border used on each side. Can be smaller but will reduce graph area
//  float   spectrum_span           // UNUSED for now.  Value in Hz.  Ths will be the maximum span shown in the display graphs.  
                                    // The graph code knows how many Hz per bin so will scale down to magnify a smaller range.
                                    // Max value and resolutoin (pixels per bin) is dependent on sample frequency
                                    // 25000 is max for 1024 FFT with 500 bins at 1:1 bins per pixel
                                    // 12500 would result in 2 pixels per bin. Bad numbers here should be corrected to best fit by the function
//  int16_t spectrum_sp_scale       // 10 to 80. Spectrum scale factor in dB. This is the height of the scale (if possible by windows sizes). Will plot the spectrum window of values between the floor and the scale value creating a zoom effect.  int16_t spectrum_floor          = -240;      // 0 to -150. The reference point for plotting values.  Anything signal value > than this (less negative) will be plotted until stronger than the window height*scale factor.
//  float   spectrum_wf_scale       // 0.0f to 40.0f. Specifies thew waterfall zoom level - may be redundant when Span is worked out later.
//  float   spectrum_LPFcoeff       // 1.0f to 0.0f. Data smoothing
//  int16_t spectrum_wf_colortemp   // Range 1 - 1023. Specifies the waterfall color temperature to tune it to your liking
//  int16_t spectrum_wf_style       // Range 1- 6. Specifies the Waterfall style.
//  int16_t spectrum_dot_bar_mode   // 0=bar, 1=DOT, 3=Line. Spectrum box . Line mode is experimental
//
//        Oddball span numbers will be adjusted to the nearest usable value, typically in powers of 2, 5 or 10 to avoid complicated interpolation issues 
//        For our starting 1024 FFT work, we will assume 512 bins and 1:1 pixel mapping with 50Hz per bin for 25K span (at 51200Hz sample rate)
//        A zoom will have 50Hz per pixel but will use the bin for one or more pixels thus magnifying, no improvement in resolution but easier to see.  
//        Example: zoom = x2 will have 1/2 the span @ 1 bin/2 pixels. The 25KHz x1 span will become 12.5K wide.

 // Work our way left and right for graph area windows, centered in the given width with border space on each side   
    if (parm_set >= PRESETS) 
        parm_set = PRESETS-1;   // Cycle back to 0
    
    struct Spectrum_Parms *ptr = &Sp_Parms_Custom[parm_set];

    //int wf_sp_width;  // This is the actual graph space width to be used.  Max is fft_bins, can be smaller.
    ptr->border_space_min = 2;  // Left and right side space. Graph space would be this this value*2 less.
    ptr->border_space = ptr->border_space_min;
    if (spectrum_width > tft.width())
        spectrum_width = tft.width();
    if (spectrum_width > (fft_bins*2) + (ptr->border_space_min*2))
    {  
        // space is wider than max graph size fft_bins to pad with border space and center graph area
        ptr->border_space = (spectrum_width - (fft_bins*2))/2;   // padding for each side
        ptr->wf_sp_width  = fft_bins*2;
    }
    else  // make smaller than FFT_bins
    {
        ptr->border_space = ptr->border_space_min;
        ptr->wf_sp_width = spectrum_width - (ptr->border_space*2);
    }
    ptr->l_graph_edge     = spectrum_x + ptr->border_space;
    ptr->r_graph_edge     = ptr->l_graph_edge + ptr->wf_sp_width;      
    ptr->c_graph          = ptr->l_graph_edge + ptr->wf_sp_width/2;  // center of graph window areas

    // Work our way down vertically.  Add padding where needed
    ptr->sp_txt_row_height   = 14;   // space for span freequency marker labels
    ptr->tick_height         = 8;    // the frequency markers top and bottom of display regions.  So total space is value*2    
    ptr->sp_txt_row          = spectrum_y +4;     // spectrum text line below the top of space we have 
    ptr->sp_tick_row         = ptr->sp_txt_row + ptr->sp_txt_row_height + ptr->tick_height ;           // bottom of tick mark rectangle space that is 1 space below text
    ptr->sp_top_line         = ptr->sp_tick_row;                       // spectrum top of graphing active area or window    
    ptr->wf_tick_row         = spectrum_y + spectrum_height -1;           // bottom of tick mark rectangle space that is 1 space below text
    ptr->wf_bottom_line      = ptr->wf_tick_row - ptr->tick_height;
    ptr->sp_height           = (ptr->wf_bottom_line - ptr->sp_top_line) * spectrum_center/100; //Account for span label and tic mark space outside of spectrum and waterfall
    ptr->wf_height           = (ptr->wf_bottom_line - ptr->sp_top_line) - ptr->sp_height;     // use what is left        
    ptr->sp_bottom_line      = ptr->sp_top_line + ptr->sp_height;                // bottom of graphing area window
    ptr->wf_top_line         = ptr->sp_bottom_line; 
    // record other master location parms to use for Preset recall othwerwise things get confused.  When reading these must be read first.
    ptr->spect_x             = spectrum_x;
    ptr->spect_y             = spectrum_y;
    ptr->spect_width         = spectrum_width;
    ptr->spect_height        = spectrum_height;
    ptr->spect_center        = spectrum_center;
    ptr->spect_span          = spectrum_span;
    ptr->spect_wf_style      = spectrum_wf_style;
    ptr->spect_wf_colortemp  = spectrum_wf_colortemp;
    ptr->spect_wf_scale      = spectrum_wf_scale;       
    ptr->spect_LPFcoeff      = spectrum_LPFcoeff;
    ptr->spect_dot_bar_mode  = spectrum_dot_bar_mode;
    ptr->spect_sp_scale      = spectrum_sp_scale;
    ptr->spect_floor         = spectrum_floor;
  
// print out results to the serial terminal for manual copy into the default table.  This is 1 set of data only, for each run.  
// Change the globals and run again for a new set

    Serial.println("Start of Spectrum Parameter Generator List.");
    Serial.println("This is a complete parameter record for the current window.");
    Serial.println("Cut and paste the data in the braces to modify the predefined records.");
    Serial.print("{");
    Serial.print(ptr->wf_sp_width); Serial.print(",");
    Serial.print(ptr->border_space_min); Serial.print(",");
    Serial.print(ptr->border_space); Serial.print(",");
    Serial.print(ptr->l_graph_edge); Serial.print(",");
    Serial.print(ptr->r_graph_edge); Serial.print(",");
    Serial.print(ptr->c_graph); Serial.print(",");
    Serial.print(ptr->sp_txt_row_height); Serial.print(",");
    Serial.print(ptr->tick_height); Serial.print(",");
    Serial.print(ptr->sp_txt_row); Serial.print(",");
    Serial.print(ptr->sp_tick_row); Serial.print(",");       
    Serial.print(ptr->sp_top_line); Serial.print(",");
    Serial.print(ptr->wf_tick_row); Serial.print(",");
    Serial.print(ptr->wf_bottom_line); Serial.print(",");
    Serial.print(ptr->sp_height); Serial.print(",");
    Serial.print(ptr->wf_height); Serial.print(",");
    Serial.print(ptr->sp_bottom_line); Serial.print(",");
    Serial.print(ptr->wf_top_line); Serial.print(",");
    Serial.print(ptr->spect_x); Serial.print(",");
    Serial.print(ptr->spect_y); Serial.print(",");
    Serial.print(ptr->spect_width); Serial.print(",");
    Serial.print(ptr->spect_height); Serial.print(",");
    Serial.print(ptr->spect_center); Serial.print(",");
    Serial.print(ptr->spect_span,1);  Serial.print(",");
    Serial.print(ptr->spect_wf_style);  Serial.print(",");
    Serial.print(ptr->spect_wf_colortemp);  Serial.print(",");
    Serial.print(ptr->spect_wf_scale,1);  Serial.print(",");
    Serial.print(ptr->spect_LPFcoeff,1);  Serial.print(",");
    Serial.print(ptr->spect_dot_bar_mode);  Serial.print(",");
    Serial.print(ptr->spect_sp_scale);  Serial.print(",");
    Serial.print(ptr->spect_floor);  Serial.print("}");
    
    Serial.println("\nEnd of Spectrum Parameter Generator List");
    Serial.print("Current Preset=");
    Serial.print(spectrum_preset);
    Serial.print("  Selected Preset=");
    Serial.print(parm_set);
    Serial.print("  Current Waterfall Style=");
    Serial.print(spectrum_wf_style);
    Serial.print("  Current Color Temp=");
    Serial.println(spectrum_wf_colortemp);
}
//
//--------------------------------------------------  find_FFT_Max() ------------------------------------------------------------------------
//
// Return the estimated frequency and max power level, based on powers in FFT bins. -------------------------------------
// Adapted from OpenAudio_ArduinoLibrary example FFTFrequencyMeter.ino by Bob Larkin, 2021
  // Following from DerekR
  // https://forum.pjrc.com/threads/36358-A-New-Accurate-FFT-Interpolator-for-Frequency-Estimation
  // " 1) A record of length 1024 samples is windowed with a Hanning window
  //   2) The magnitude spectrum is computed from the FFT, and the two (adjacent)
  //      largest amplitude lines are found.  Let the largest be line L, and the
  //      other be either L+1, of L-1.
  //   3) Compute the ratio R of the amplitude of the two largest lines.
  //   4) If the amplitude of L+1 is greater than L-1 then
  //              f = (L + (2-R)/(1+R))*f_sample/1024
  //        otherwise
  //              f = (L - (2-R)/(1+R))*f_sample/1024  "
void find_FFT_Max(void)  
{
    float specMax = 0.0f;
    uint16_t iiMax = 0;

    myFFT.setOutputType(FFT_POWER);   // change to power, return it to FFT_DBFS at end
    myFFT.windowFunction(AudioWindowHanning1024);

    // Get pointer to data array of powers, float output[512];
    float* pPwr = myFFT.getData();
    // Find biggest bin
    for(int ii=2; ii<FFT_SIZE; ii++)  
    {
        if (*(pPwr + ii) > specMax) 
        { // Find highest peak of 512
          specMax = *(pPwr + ii);
          iiMax = ii;
        }
    }
    float vm = sqrtf( *(pPwr + iiMax - 1) );
    float vc = sqrtf( *(pPwr + iiMax) );
    float vp = sqrtf( *(pPwr + iiMax + 1) );
    if(vp > vm)  
    {
        myFFT.setOutputType(FFT_DBFS);   // change to power, return it to FFT_DBFS at end
        fftMaxPower = vc/vp;  // set global fftMaxPower = Power of the strongest signal if possible
        float R = vc/vp;
        fftFrequency = ( (float32_t)iiMax + (2-R)/(1+R) )*fft_bin_size;   // *44100.0f/1024.0f;
    }
    else  
    {
        myFFT.setOutputType(FFT_DBFS);   // return it to FFT_DBFS
        fftMaxPower = vc/vm;  // set global fftMaxPower = Power of the strongest signal if possible
        float R = vc/vm;
        fftFrequency = ( (float32_t)iiMax - (2-R)/(1+R) )*fft_bin_size;  // fftFrequency is global to this module
    }
}

