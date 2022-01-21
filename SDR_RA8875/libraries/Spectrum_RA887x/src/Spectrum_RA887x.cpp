//
//      Spectrum_RA8875.cpp
//
// Spectrum_RA887x Library draws the spectrum display in a resizable window area at any reasonable X,Y coordinate
// Supports RA8875 and RA8876 usign the #define USE_RA8875 (undefined compiles for RA8876)
//
//------------------------------------------------------------  Waterfall and Spectrum  Display ------------------------------------------------------
//  
#include "Spectrum_RA887x.h"
#include <Metro.h>              // GitHub https://github.com/nusolar/Metro
#include <Audio.h>              // Included with Teensy and at GitHub https://github.com/PaulStoffregen/Audio
#include <OpenAudio_ArduinoLibrary.h> // F32 library located on GitHub. https://github.com/chipaudette/OpenAudio_ArduinoLibrary

// From main file where sampling rate and other audio library features are set
extern AudioAnalyzeFFT4096_IQ_F32  myFFT;
//extern AudioAnalyzeFFT2048_IQ_F32  myFFT;
//extern AudioAnalyzeFFT1024_IQ_F32  myFFT;
extern int16_t                  fft_bins;    //Number of FFT bins. 1024 FFT has 512 bins for 50Hz per bin   (sample rate / FFT size)
extern float                    fft_bin_size;     //   Hz per bin
//extern uint32_t                 VFOA;
//extern uint32_t                 VFOB;
//extern struct Band_Memory       bandmem[];
//extern uint8_t                  curr_band;   // global tracks our current band setting.
extern volatile int32_t         Freq_Peak;
#ifdef USE_RA8875
	extern RA8875 tft;
#else 
	extern RA8876_t3 tft;
    int16_t	_activeWindowXL = 0;
    int16_t _activeWindowXR = SCREEN_WIDTH;
    int16_t	_activeWindowYT = 0;
    int16_t _activeWindowYB = SCREEN_HEIGHT;
#endif

int16_t wf_time_line = 15000;
int16_t fftFreq_refresh = 1000;
Metro waterfall_timestamp=Metro(wf_time_line);  // Used to draw a time stamp line across the waterfall window.  Cha
Metro fftFreq_timestamp = Metro(fftFreq_refresh);
Metro spectrum_clear = Metro(1000);
Metro spectrum_waterfall_update = Metro(80); // using default of 80.
//void spectrum_update(int16_t s, int16_t VFOA_YES, int16_t VfoA, int16_t VfoB);
void setActiveWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB);
void setActiveWindow_default(void);
int16_t _colorMap(int16_t val, int16_t color_temp);
int16_t _find_FFT_Max(uint16_t bin_min, uint16_t bin_max);
char* _formatFreq(uint32_t Freq);
//static uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);
//inline uint16_t _Color565(uint8_t r,uint8_t g,uint8_t b);
int16_t _waterfall_color_update(float sample, int16_t waterfall_low);
static int16_t spectrum_scale_maxdB    = 1;       // max value in dB above the spectrum floor we will plot signal values (dB scale max)
static int16_t spectrum_scale_mindB    = 80;       // min value in dB above the spectrum floor we will plot signal values (dB scale max)
//static int16_t fftFrequency            = 0;        // Used to hold the FFT peak signal's frequency offsewt from Fc. Use a RF sig gen to measure its frequency and spot it on the display, useful for calibration
static int16_t fftMaxPower             = 0;        // Used to hold the FFT peak power for the strongest signal
/*
On ordering of the frequencies, this ends up being dependent on the mixer wring. If you switch the ends on a transformer and you switch + and - frequencies.  So, I decided to make the order from the FFT programmable.  There is a new function setXAxis(uint8_t xAxis);  It follows these rules:
   If xAxis=0  f=fs/2 in middle, f=0 on right edge
   If xAxis=1  f=fs/2 in middle, f=0 on left edge
   If xAxis=2  f=fs/2 on left edge, f=0 in middle
   If xAxis=3  f=fs/2 on right edge, f=0 in middle
The default is 3 (requires no call) and I believe it is the same as the I16/F32 converted 256 code that Keith supplied, way back.  But that is not important.  Just change xAxis, call
  myFFT.setXAxis(2);
in your INO setup(), or 0 or 1, and pretty soon it will be what you want, maybe.
*/
uint8_t fft_axis                = FFT_AXIS;
// Function Declarations

//-------------- COLOR CONVERSION -----------------------------------------------------------
	inline uint16_t _Color565(uint8_t r,uint8_t g,uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }
//	inline uint16_t Color24To565(int32_t color_) { return ((((color_ >> 16) & 0xFF) / 8) << 11) | ((((color_ >> 8) & 0xFF) / 4) << 5) | (((color_) &  0xFF) / 8);}
//	inline uint16_t htmlTo565(int32_t color_) { return (uint16_t)(((color_ & 0xF80000) >> 8) | ((color_ & 0x00FC00) >> 5) | ((color_ & 0x0000F8) >> 3));}
//	inline void 	Color565ToRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b){r = (((color & 0xF800) >> 11) * 527 + 23) >> 6; g = (((color & 0x07E0) >> 5) * 259 + 33) >> 6; b = ((color & 0x001F) * 527 + 23) >> 6;}

struct Spectrum_Parms Sp_Parms_Def[PRESETS] = { // define default sets of spectrum window parameters, mostly for easy testing but could be used for future custom preset layout options
        //W        LE  RE  CG                                            x   y   w  h  c sp st clr sc mode scal reflvl wfrate
        #ifdef USE_RA8875
        {798,0, 0,  0,798,398,14,8,157,179,179,408,400,110,111,289,289,  0,153,799,256,50,20,6,240,1.0,0.9,1,20, 8, 70},
        {500,2,49,150,650,400,14,8,133,155,155,478,470, 94,221,249,249,130,129,540,350,30,25,2,550,1.0,0.9,1,30, 8, 70}, // hal
        {796,2, 2,  2,798,400,14,8,143,165,165,408,400, 94,141,259,259,  0,139,800,270,40,20,2,310,1.0,0.9,1,40, 5, 90},
        {500,2,49,150,650,400,14,8,133,155,155,478,470, 94,221,249,249,130,129,540,350,30,25,2,550,1.0,0.9,1,30, 8, 70}, // hal
        #else
        {1020,1,1,  1,1021,510,14,8,143,165,165,528,520,142,213,307,307,  0,139,1022,390,40,20,6,890,1.5,0.9,1,20,10, 80},
        { 508,1,1,  1, 509,254,14,8,214,236,236,528,520,113,171,349,349,  0,210, 510,319,40,20,2,310,1.0,0.9,0,40, 8,100},
        { 508,1,1,513,1021,766,14,8,214,236,236,528,520,113,171,349,349,512,210, 510,319,40,20,2,310,1.0,0.9,1,40, 8,100},
        { 298,1,1,601, 899,749,14,8,304,326,326,499,491, 99, 66,425,425,600,300, 300,200,60,20,2,310,1.0,0.9,0,40, 6,100},
        #endif        
        {512,2,43,143,655,399,14,8,354,376,376,479,471, 57, 38,433,433,100,350,599,130,60,25,2,340,1.7,0.9,0,60, 8, 80},  // Small wide bottom screen area to fit under pop up wndows.
        {498,1, 1,  1,499,249,14,8,143,165,165,408,400, 94,141,259,259,  0,139,500,270,40,20,2,310,1.0,0.9,0,40, 6,100},    //smaller centered
        {198,1, 1,551,749,649,14,8,183,205,205,408,400,136, 59,341,341,550,179,200,230,70,20,2,310,1.0,0.9,1,40, 0,100},  // low wide high gain
        {500,2, 2,150,650,400,14,8,133,155,155,418,410,102,153,257,257,130,129,540,290,40,25,2,320,1.0,0.9,1,30, 8, 75},     //60-100 good
        {512,2,43,143,655,399,14,8,223,245,245,348,340, 57, 38,302,302,100,219,599,130,60,25,2,310,1.7,0.9,0,60, 8,100},
        {396,2, 2,102,498,300,14,8,243,265,265,438,430, 99, 66,364,364,100,239,400,200,60,25,2,310,1.7,0.9,0,40, 8,100},
        {512,2,43,143,655,399,14,8,183,205,205,478,470,106,159,311,311,100,179,599,300,40,25,2,450,0.7,0.9,1,40, 8, 40},
        {796,2, 2,  2,798,400,14,8,183,205,205,478,470,106,159,311,311,  0,179,800,300,40,25,5,440,1.0,0.9,0,40, 8, 30}
    };
    
// Place to hold custom data, enventually in EEPROM
struct Spectrum_Parms Sp_Parms_Custom[1] = {};

Spectrum_RA887x::Spectrum_RA887x(void)
{
    // Initialization for library
}

#ifndef USE_RA8875
// These are from the RA8875 library because they are marked protected in the RA8876 library.  
// Putting copies of them here eliminate the need to change the library but will lose certain features like rotation to portrait. 

/**************************************************************************/
void Spectrum_RA887x::setActiveWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB)
{
	//if (_portrait){ swapvals(XL,YT); swapvals(XR,YB);}

//	if (XR >= SCREEN_WIDTH) XR = SCREEN_WIDTH;
//	if (YB >= SCREEN_HEIGHT) YB = SCREEN_HEIGHT;
	
	_activeWindowXL = XL; _activeWindowXR = XR;
	_activeWindowYT = YT; _activeWindowYB = YB;
	updateActiveWindow(false);
}

/**************************************************************************/
/*!		
		Set the Active Window as FULL SCREEN
*/
/**************************************************************************/
void Spectrum_RA887x::setActiveWindow_default(void)
{
	_activeWindowXL = 0; _activeWindowXR = SCREEN_WIDTH;
	_activeWindowYT = 0; _activeWindowYB = SCREEN_HEIGHT;
	//if (_portrait){swapvals(_activeWindowXL,_activeWindowYT); swapvals(_activeWindowXR,_activeWindowYB);}
	updateActiveWindow(true);
}

/**************************************************************************/
/*!
		this update the RA8875 Active Window registers
		[private]
*/
/**************************************************************************/
void Spectrum_RA887x::updateActiveWindow(bool full)
{ 
	if (full){
		// X
		tft.activeWindowXY(0, 0);
		tft.activeWindowWH(SCREEN_WIDTH, SCREEN_HEIGHT);;
	} else {
		tft.activeWindowXY(_activeWindowXL, _activeWindowYT);
		tft.activeWindowWH(_activeWindowXR-_activeWindowXL, _activeWindowYB-_activeWindowYT);		
	}
}
#endif

void Spectrum_RA887x::spectrum_update(int16_t s, int16_t VFOA_YES, int32_t VfoA, int32_t VfoB)
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
    static int16_t spect_scale_last   = 0;
    //static int16_t spect_ref_last     = 0;
    int16_t fft_pk_bin                = 0;
    static int16_t fftPower_pk_last   = ptr->spect_floor;
    static int16_t pix_min            = ptr->spect_floor;
        
    if (s >= PRESETS) 
        s=PRESETS-1;   // Cycle back to 0
    // See Spectrum_Parm_Generator() below for details on Global values requires and how the woindows variables are used.    
    
    //for testing alignments
    //tft.drawRect(spectrum_x, spectrum_y, spectrum_width, spectrum_height, myBLUE);  // x start, y start, width, height, array of colors w x h
    //tft.drawRect(ptr->spect_x, ptr->spect_y, ptr->spect_width, ptr->spect_height, myBLUE);  // x start, y start, width, height, array of colors w x h
     
    int16_t i;
    float avg = 0.0;
    int16_t pixelnew[SCREEN_WIDTH+2];           //  Stores current pixel fopr spectrum portion only
    static int16_t pixelold[SCREEN_WIDTH+2];    //  Stores copy of current pixel so it can be erased in next update
    //int8_t span_FFT[SCREEN_WIDTH+2];         // Intended to store averaged values representnig a larger FFT set into the smaller screen width set
    float *pout = myFFT.getData();          // Get pointer to data array of powers, float output[512]; 
    int16_t line_buffer[SCREEN_WIDTH+2];      // Will only use the x bytes defined by wf_sp_width var.  Could be 4096 FFT later which is larger than our width in pixels. 
    int16_t L_EDGE = 0; 

    #ifdef ENET
     extern uint8_t enet_write(uint8_t *tx_buffer, const int count);
     extern uint8_t tx_buffer[];
    #endif
    
    if (myFFT.available()) 
    {     
        #ifdef ENET
            if (enet_data_out && enet_ready)
            {
                for (i = 0; i < FFT_SIZE; i++)
                {
                    tx_buffer[i] = (uint8_t) abs(*(pout+i));
                }
                //memcpy(tx_buffer, full_FFT, FFT_SIZE);
                enet_write(tx_buffer, FFT_SIZE);
            }
        #endif

        // Calculate center then if FFT is larger than graph area width, trim ends evently
        if (ptr->spect_span == 50)   //span width in KHz.
        {      
            // pack all bins into the available display width.  
            int wd = ptr->wf_sp_width;
            //int div = ptr->spect_span;
            int binsz = round(FFT_SIZE/wd);  // bins that will be compressed into 1 pixel to fit the screen
            for (i = 0; i < ptr->wf_sp_width; i++)
            {        
                if ( i > SCREEN_WIDTH) // do not overrun our buffer size.  Ideally wf_sp_width would never be > SCREENWIDTH but....
                    i = SCREEN_WIDTH;
                //span_FFT[i] = (int16_t) myFFT.read(binsz*i);
            }
            //pout = span_FFT;
            Serial.print("Zoom Out =");
            Serial.println(binsz,DEC);
        }              
        else if ( FFT_SIZE > ptr->wf_sp_width-2)  // When FFT data is > available graph area
        {
            L_EDGE = (FFT_SIZE - ptr->wf_sp_width)/2;
            pout = pout+L_EDGE;  // adjust the starting point up a bit to keep things centered.
        }
        
        // else   // When FFT data is < available graph area
        // {      // If our display area is less then our data width, fill in the outside areas with low values.
            //L_EDGE = (ptr->wf_sp_width - FFT_SIZE - )/2;
            //pout = pout+L_EDGE;  // adjust the starting point up a bit to keep things centered.
            /*
            for (i=0; i< FFT_SIZE/4; i++)        
                tempfft[i] = -500;
            for (i=(FFT_SIZE/4)*3; i< FFT_SIZE; i++)        
                 tempfft[i] = -500;
            //L_EDGE = FFT_center - GRAPH_center;
            */
        // }

        for (i = 0; i < ptr->wf_sp_width; i++)        // Grab all 512 values.  Need to do at one time since averaging is looking at many values in this array
        { 
            if (isnanf(*(pout+i)) || isinff (*(pout+i)))    // trap float 'NotaNumber NaN" and Infinity values
            {
                Serial.println(F("FFT Invalid Data INF or NaN"));
                //Serial.println(*(pout+i));                
                pixelnew[i] = -200;   // fill in the missing value with somting harmless
                //pixelnew[i] = myFFT.read(i+1);  // hope the next one is better.
            }
            // Now capture Spectrum value for use later
            pixelnew[i] = (int16_t) *(pout+i);

            // Several different ways to process the FFT data for display. Gather up a complete FFT sample to do averaging then go on to update the display with the results
            switch (ptr->spect_wf_style)
            { 
              case 0: if ( i > 1 )  // prevent reading array out fo bound < 1. 
                {
                    avg = *(pout+(i*16/10))*0.5 + *(pout+(i-1)*16/10)*0.18 + *(pout+(i-2)*16/10)*0.07 + *(pout+(i+1)*16/10)*0.18 + *(pout+(i+2)*16/10)*0.07;                
                    //line_buffer[i] = (LPFcoeff * 8 * sqrt (100+(abs(avg)*wf_scale)) + (1 - LPFcoeff) * line_buffer[i]);
                    line_buffer[i] = (ptr->spect_LPFcoeff * 8 * sqrtf(abs(avg)) + (1 - ptr->spect_LPFcoeff) * line_buffer[i]);                      
                }      
                      break;
              case 1: if ( i > 1 )  // prevent reading array out fo bound < 1.
                {
                    avg = *(pout+i)*0.5 + *(pout+i-1)*0.18 + *(pout+i-2)*0.07 + *(pout+i+1)*0.18 + *(pout+i+2)*0.07;                
                    line_buffer[i] = ptr->spect_LPFcoeff * 8 * sqrtf(abs(avg)) + (1 - ptr->spect_LPFcoeff);
                    line_buffer[i] = _colorMap(line_buffer[i], ptr->spect_wf_colortemp);
                    //Serial.println(line_buffer[i]);    
                }
                      break;                  
              case 2: avg = line_buffer[i] = _colorMap(abs(*(pout+i)) * 1.9 *  ptr->spect_wf_scale, ptr->spect_wf_colortemp);
                      break;
              case 3: avg = line_buffer[i] = _colorMap(abs(*(pout+i)) * 0.4 *  ptr->spect_wf_scale, ptr->spect_wf_colortemp);
                      break;
              case 4: avg = line_buffer[i] = _colorMap(16000 - abs(*(pout+i)), ptr->spect_wf_colortemp) * ptr->spect_wf_scale;
                      break;
              case 6: avg = line_buffer[i] = _waterfall_color_update(*(pout+i), pix_min);//  * ptr->spect_sp_scale;  // test new waterfall colorization method
                      break;
              case 5:
             default: avg = line_buffer[i] = _colorMap(abs(*(pout+i)), ptr->spect_wf_colortemp);                          
                      break; 
            };

// Serial.println(tft.gradient( (uint16_t) pix_n16));

            // Fc Blanking
            if (i >= (ptr->wf_sp_width/2)-blanking  && i <= (ptr->wf_sp_width/2)+blanking+1)
            {
                line_buffer[i] = myBLACK;    
                if (i == ((ptr->wf_sp_width)/2) + 1)
                    line_buffer[i] = myLT_GREY;  // draw center Fc line in waterfall
            }
        }   // Done with copying the FFT output array

        for (i = 2; i < (ptr->wf_sp_width-1); i++)
        {
            if (i == 2)
            {
                pix_min = pixelnew[2];  // start off each set with a sample value to compare others with
            }
            else
            {
                if (pixelnew[i] < pix_min)
                    pix_min = pixelnew[i];
            }
        }
        // Takes a snapshot of the current window without the bottom row. Stores it in Layer 2 then brings it back beginning at the 2nd row. 
        //    Then write new row data into the missing top row to get a scroll effect using display hardware, not the CPU.
        //    Documentation for BTE: BTE_move(int16_t SourceX, int16_t SourceY, int16_t Width, int16_t Height, int16_t DestX, int16_t DestY, uint8_t SourceLayer=0, uint8_t DestLayer=0, bool Transparent = false, uint8_t ROP=RA8875_BTEROP_SOURCE, bool Monochrome=false, bool ReverseDir = false);                  
        #ifdef USE_RA8875
            tft.BTE_move(ptr->l_graph_edge+1, ptr->wf_top_line+1, ptr->wf_sp_width, ptr->wf_height-4, ptr->l_graph_edge+1, ptr->wf_top_line+2, 1, 2);  // Layer 1 to Layer 2
            while (tft.readStatus());  // Make sure it is done.  Memory moves can take time.
            
            // Move the block back on Layer 1 but place it 1 row down from the top
            tft.BTE_move(ptr->l_graph_edge+1, ptr->wf_top_line+2, ptr->wf_sp_width, ptr->wf_height-4, ptr->l_graph_edge+1, ptr->wf_top_line+2, 2);  // Move layer 2 up to Layer 1 (1 is assumed).  0 means use current layer.
            while (tft.readStatus());   // Make sure it is done.  Memory moves can take time.        
        #else   // RA8876  
            tft.canvasImageStartAddress(PAGE2_START_ADDR);
            tft.boxPut(PAGE2_START_ADDR, ptr->l_graph_edge+1, ptr->wf_top_line+1, ptr->wf_sp_width, ptr->wf_bottom_line-2, ptr->l_graph_edge+1, ptr->wf_top_line+2);                    
            tft.check2dBusy();     
            
            tft.canvasImageStartAddress(PAGE1_START_ADDR);
            tft.boxGet(PAGE2_START_ADDR, ptr->l_graph_edge+1, ptr->wf_top_line+2, ptr->wf_sp_width, ptr->wf_bottom_line-1, ptr->l_graph_edge+1, ptr->wf_top_line+2);
            tft.check2dBusy();              
        #endif  // USE_RA8875
        
        // draw a periodic time stamp line
        if (waterfall_timestamp.check() == 1)
            tft.drawRect(ptr->l_graph_edge+1, ptr->wf_top_line+2, 20, 1, myLT_GREY);  // x start, y start, width, height, colors w x h           
            //tft.drawFastHLine(ptr->l_graph_edge+1, ptr->wf_top_line+1, ptr->wf_sp_width, myLT_GREY);  // x start, y start, width, height, colors w x h           
        else  // Draw the new line at the top
            tft.writeRect(ptr->l_graph_edge+1, ptr->wf_top_line+1, ptr->wf_sp_width, 1, (uint16_t*) &line_buffer);  // x start, y start, width, height, array of colors w x h

        //
        //--------------------------------  Spectrum Window ------------------------------------------
        //
        // Done with waterfall, now draw the spectrum section
        // start at 2 to prevent reading out of bounds during averaging formula
                    // Draw our image on canvas 2 which is not visible

        #ifdef USE_RA8875
            tft.setActiveWindow(ptr->l_graph_edge+1, ptr->r_graph_edge-1, ptr->sp_top_line+2, ptr->sp_bottom_line-2); 
            tft.writeTo(L2);         //L1, L2, CGRAM, PATTERN, CURSOR     
        #else            
            // NOTE - setActiveWindow() function in the RA8876_t3 library is marked as protected: Can change it to public:
            // Instead we are using own copies for RA8876
            // For RA8876 switch to hidden Page 2, draw our line and all label/info text as normal then at end, 
            // do a BTE mem copy from page 2 to page 1 for a flicker free, clean screen drawn fast.
            tft.canvasImageStartAddress(PAGE2_START_ADDR);
            // Blank the plot area and we will draw a new line, flicker free!
            setActiveWindow(ptr->l_graph_edge+1, ptr->r_graph_edge-1, ptr->sp_top_line+1, ptr->sp_bottom_line-1);
        #endif
        
        tft.fillRect(ptr->l_graph_edge+1,    ptr->sp_top_line+1,    ptr->wf_sp_width,     ptr->sp_height-2,    myBLACK);

        //int pix_min = pixelnew[2];
        //for (i = 2; i < (ptr->wf_sp_width-1); i++)
        //    if (pixelnew[i] < pix_min)
        //        pix_min = pixelnew[i];
        // average a few values to smooth the line a bit
        float avg_pix2 = (pixelnew[i]+pixelnew[i+1])/2;     // avg of 2 bins            
        float avg_pix5 = (pixelnew[i-2]+pixelnew[i-1]+pixelnew[i]+pixelnew[i+1]+pixelnew[i+2])/5; //avg of 5 bins
        
        if (abs(pixelnew[i]) > abs(avg_pix2) * 1.6f)    // compare to a small average to toss out wild spikes
            pixelnew[i] = (int16_t) avg_pix5;                     // average it out over a wider segment to patch the hole   

        //Serial.print("pix min ="); Serial.println(pix_min);

        for (i = 2; i < (ptr->wf_sp_width-1); i++)   // Add SPAN control to spread things out here.  Currently 10KHz per side span with 96K sample rate  
        {       

            if (i >= (ptr->wf_sp_width/2)-blanking-1 && i <= (ptr->wf_sp_width/2)+blanking+1)
                pixelnew[i] = -200; 

            //#define DBG_SPECTRUM_SCALE
            //#define DBG_SPECTRUM_PIXEL
            //#define DBG_SPECTRUM_WINDOWLIMITS
            
            #ifdef DBG_SPECTRUM_PIXEL
            Serial.print(" raw =");
            Serial.print(pixelnew[i],DEC);
            #endif
            
            // limit the upper and lower dB level to between these ranges (set scale) (User Setting)  Can be limited further by window heights   
            //spectrum_scale_maxdB = 10;     //scale most zoomed in.  This is +10dB above the spectrum floor value.   That value is adjustables and is our refence point set to the bottom line.  
                                                //Forms the top range of values that line up with the top of our "window" on the FFT data set value range, typiclly -150 to -0dBm possible.
            //spectrum_scale_mindB = 80;   // scale most zoomed out.  This is +80 dB relative to the spectrum_floor so teh top end of our window.  Typically -150 to -0dBm possible range of signal.             
                    // range limit our settings.    This number is added to teh spectrum floor.  The pixel value will be plotted where ever it lands as along as it is in the window.

            #ifdef DBG_SPECTRUM_SCALE
            Serial.print("   SC_ORG="); Serial.print(ptr->spect_sp_scale);                  
            #endif
        
            ptr->spect_sp_scale = constrain(ptr->spect_sp_scale, spectrum_scale_maxdB, spectrum_scale_mindB);

            #ifdef DBG_SPECTRUM_SCALE
            Serial.print("   SC_LIM="); Serial.print(ptr->spect_sp_scale);                  
            #endif
                
            #ifdef DBG_SPECTRUM_SCALE
            Serial.print("   SC_HT="); Serial.print(ptr->spect_sp_scale);
            Serial.print("   HT="); Serial.print(ptr->sp_height-4);
            Serial.print("   SC_FLR="); Serial.print(ptr->spect_floor);                  
            #endif       
            
            // Invert the sign since the display is also inverted, Increasing value = weaker signal strength, they are now going the same direction.  
            // Small value = bigger signal, closer to 0 on the display coordinates
            pixelnew[i] = (int16_t) abs(pixelnew[i]);   

            
            // We are plotting our pixel in the window if it lands between the bottom line and top lines        
            // set the grass floor to just above the bottom line.  These are the weakest signals. Typically -90 coming out of the FFT right now
            // Offset the pixel position relative to the bottom of the window
            pixelnew[i] +=  ptr->spect_floor;      

            //#if defined (DBG_SPECTRUM_PIXEL) || defined (DBG_SPECTRUM_WINDOWLIMITS)
            //Serial.print("  NF  pix ="); Serial.println(pixelnew[i],DEC);
            //#endif

            pixelnew[i] = map(pixelnew[i], abs(pix_min), abs(ptr->spect_sp_scale), ptr->sp_bottom_line, ptr->sp_top_line); 
            
            #ifdef DBG_SPECTRUM_WINDOWLIMITS 
            //Serial.print("  win-ht:"); Serial.print(ptr->sp_height-4);
            Serial.print("  top line="); Serial.print(ptr->sp_top_line+2);
            #endif
            
            //#if defined (DBG_SPECTRUM_PIXEL) || defined (DBG_SPECTRUM_WINDOWLIMITS)
            //Serial.print("  MAP pix ="); Serial.println(pixelnew[i],DEC);
            //#endif

            #ifdef DBG_SPECTRUM_WINDOWLIMITS 
            Serial.print("  bottom line="); Serial.print(ptr->sp_bottom_line-2);
            #endif

            //#define DBG_SHOW_OVR
            
            if (pixelnew[i] < ptr->sp_top_line+1)        
            {
                #if defined(DBG_SPECTRUM_WINDOWLIMITS) || defined(DBG_SPECTRUM_PIXEL) || defined(DBG_SPECTRUM_SCALE) || defined(DBG_SHOW_OVR) 
                Serial.print(" !!OVR!! = ");    Serial.println(pixelnew[i] - ptr->sp_top_line+2,0);
                #endif
                pixelnew[i] = ptr->sp_top_line+1;

            }

            if (pixelnew[i] > ptr->sp_bottom_line-1)        
            { 
                #if defined(DBG_SPECTRUM_WINDOWLIMITS) || defined(DBG_SPECTRUM_PIXEL) || defined(DBG_SPECTRUM_SCALE) || defined(DBG_SHOW_OVR)
                Serial.print(" !!UNDER!! = ");  Serial.println(pixelnew[i] - ptr->sp_top_line+2,0);
                #endif
                pixelnew[i] = ptr->sp_bottom_line-1;
            }          
            //#endif
            
            pix_n16 = pixelnew[i];  // convert float to uint16_t to match the draw functions type
            pix_o16 = pixelold[i];

            // Limit access to the spectrum box to control misbehaved pixel and bar draws
            //
            //------------------------ Code below is writing only in the active spectrum window ----------------------
            //
            if (i == 2)
            {
                //tft.clearActiveScreen();
                //tft.fillRect(ptr->l_graph_edge+1, ptr->sp_top_line+20, ptr->wf_sp_width-2, ptr->sp_height-22, myBLACK);
                //tft.drawFastVLine(ptr->l_graph_edge+ptr->wf_sp_width/2+1, ptr->sp_top_line+1, ptr->sp_height, myLT_GREY);
            }
            if (i < (ptr->wf_sp_width/2)-5 || i > (ptr->wf_sp_width/2) + 5)   // blank the DC carrier noise at Fc
            {
                if ((i < ptr->wf_sp_width-2) && (pix_n16 > ptr->sp_top_line+2) && (pix_n16 < ptr->sp_bottom_line-2)  ) // will blank out the center spike
                {   
                    if (ptr->spect_dot_bar_mode == 0)   // BAR Mode 
                    {
                        if (pix_o16 != pix_n16) 
                        {                          
                            if (pix_n16 > ptr->sp_top_line && pix_n16 < ptr->sp_bottom_line-1)
                            {
                                //common way: draw bars                                                                        
                                #ifdef USE_RA8875
                                    //tft.drawFastVLine(ptr->l_graph_edge+i, pix_o16, ptr->sp_bottom_line-pix_o16,    myBLACK); // GREEN);
                                    tft.drawFastVLine(ptr->l_graph_edge+i, pix_n16, ptr->sp_bottom_line-pix_n16,    myYELLOW); //BLACK);
                                #else
                                    tft.drawFastVLine(ptr->l_graph_edge+i, pix_n16, ptr->sp_bottom_line-pix_n16,    myYELLOW); //BLACK);
                                #endif
                                
                                pixelold[i] = pixelnew[i];
                            }
                        }
                    }
                    else  // was DOT mode, now LINE mode
                    {   
                        // DOT Mode
                        //if (pix_n16 > ptr->sp_top_line && pix_n16 < ptr->sp_bottom_line-1)
                            // DOT Mode
                            // --->> This is a good working line but wastes time erasing mostly black space from the top down.
                            //tft.drawFastVLine(ptr->l_graph_edge+1+i, ptr->sp_top_line+1, pixelnew[i-1], myBLACK);   // works with some artifacts
                            //tft.drawFastVLine(ptr->l_graph_edge+1+i, ptr->sp_top_line+1, ptr->sp_height-2, myBLACK);// works with some artifacts  

                            // LINE Mode                     
                            #ifdef USE_RA8875
                            // For the RA8875 erase the column we are about to draw in.  The RA8876 blanks the whole box and used BTE
                                //tft.drawRect(ptr->l_graph_edge+1+i, ptr->sp_top_line+1, 2, ptr->sp_height-2, myBLACK);  // works pretty good
                            #endif

                        // This will be drawn on Canvas 2 if this is a RA8876, layer 2 if a RA8875                               
                        //if (pixelnew[i] < ptr->sp_bottom_line && i > 3 )
                        tft.drawLine(ptr->l_graph_edge+i, pixelnew[i-1], ptr->l_graph_edge+i, pixelnew[i],  myYELLOW);

                        pixelold[i] = pixelnew[i]; 
                    }
                }
            }
            
            // Draw Grid Lines
            if (i == (ptr->wf_sp_width/2))
            {
            // draw a grid line for XX dB level             
                tft.setTextColor(myLT_GREY, myBLACK);
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
                tft.drawFastVLine(ptr->l_graph_edge+ptr->wf_sp_width/2+2, ptr->sp_top_line+1, ptr->sp_height, myLT_GREY);
            }
        } // end of spectrum pixel plotting

        // Update graph scale, ref level, power and freq
        tft.setTextColor(myLT_GREY, myBLACK);
        tft.setFont(Arial_12);

        fft_pk_bin = _find_FFT_Max(L_EDGE+2, L_EDGE+ptr->wf_sp_width-2);   // get new frequency and power values for strongest signal 
        
        uint32_t _VFO_;   // Get active VFO frequency
        //if (bandmem[curr_band].VFO_AB_Active == VFO_A)
        if (VFOA_YES)
            _VFO_ = VfoA;
        else    
            _VFO_ = VfoB;

        // Calculate and print the power of the strongest signal if possible
        // Start by getting the highest power within a period of time
        if (fftMaxPower > fftPower_pk_last)
        { 
            fftPower_pk_last = fftMaxPower;            
            //tft.fillRect(ptr->l_graph_edge+30,         ptr->sp_txt_row+30, 70, 13, RA8875_BLACK);  // clear the text space
            //tft.setCursor(ptr->l_graph_edge+30,        ptr->sp_txt_row+30); // Write the legend
            //tft.print("P: "); 
            //tft.setCursor(ptr->l_graph_edge+46,        ptr->sp_txt_row+30);  // write the value
            //tft.print(fftMaxPower-20);  // fudge factor added
            //Serial.print("Ppk="); Serial.println(fftMaxPower);
            //fftFreq_timestamp.reset();  // reset the timer since we have new good data
        }
                            
        if (fftFreq_timestamp.check() == 1)
        {
            fftPower_pk_last = -200;  // reset the timer since we have new good data
            //Serial.println("Reset");
        }
                
        // Calculate and print the frequency of the strongest signal if possible 
        //Serial.print("Freq="); Serial.println(fftFrequency, 3); 
        //tft.fillRect(ptr->l_graph_edge+109,    ptr->sp_txt_row+30, 140, 13, RA8875_BLACK);
        tft.setCursor(ptr->l_graph_edge+110,  ptr->sp_txt_row+30);
        tft.print("F: "); 
        tft.setCursor(ptr->l_graph_edge+126,  ptr->sp_txt_row+30);
        float pk_temp = _VFO_ + (2 * (fft_bin_size * fft_pk_bin));   // relate the peak bin to the center bin
        Freq_Peak = pk_temp;
        tft.print(_formatFreq(pk_temp));
        
        // Write the Scale value 

        tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+50, ptr->sp_txt_row+30);
        tft.print("S:   "); // actual value is updated elsewhere   
        //tft.fillRect( ptr->l_graph_edge+(ptr->wf_sp_width/2)+64, ptr->sp_txt_row+30, 32, 13, RA8875_BLACK);       
        tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+64, ptr->sp_txt_row+30);
        tft.print(ptr->spect_sp_scale);     
        
        if (spect_scale_last != ptr->spect_sp_scale)
        {      
            spect_scale_last = ptr->spect_sp_scale;   // update memory
        }
        
        // Write the Reference Level to top line area
        tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+100, ptr->sp_txt_row+30);
        tft.print("R:   ");  // actual value is updated elsewhere            
        //tft.fillRect( ptr->l_graph_edge+(ptr->wf_sp_width/2)+114, ptr->sp_txt_row+30, 32, 13, RA8875_BLACK);
        tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+114, ptr->sp_txt_row+30);
        tft.print(ptr-> spect_floor);
        //Serial.print("R lvl="); Serial.println(ptr-> spect_floor);

        //if (spect_ref_last != ptr->spect_floor)
        //{
            //spect_ref_last = ptr->spect_floor;   // update memory
        //}    
            
        // Write the dB range of the window 
        tft.setTextColor(myLT_GREY, myBLACK);
        tft.setFont(Arial_10);
        tft.setCursor(ptr->r_graph_edge-50, ptr->sp_top_line+8);
        tft.print("H:   ");  // actual value is updated elsewhere
        tft.setCursor(ptr->r_graph_edge-38, ptr->sp_top_line+8); 
        tft.print(ptr->sp_height);

        // Reset spectrum screen blanking timeout
        spectrum_clear.reset();

        //
        //------------------------ Code above is writing only in the active spectrum window  (RA8875 only)----------------------
        //
        //-----------------------   This part onward is outside the active window (for RA8875 only)------------------------------
        //

        #ifdef USE_RA8875
            // Insert RA8875 BTE_move
            tft.writeTo(L1);         //L1, L2, CGRAM, PATTERN, CURSOR
            tft.BTE_move(ptr->l_graph_edge+1, ptr->sp_top_line+1, ptr->wf_sp_width, ptr->sp_height-2, ptr->l_graph_edge+1, ptr->sp_top_line+1, 2);  // Move layer 2 up to Layer 1 (1 is assumed).  0 means use current layer.
            while (tft.readStatus());   // Make sure it is done.  Memory moves can take time.
            tft.setActiveWindow();
        #else
            // BTE block copy it to page 1 spectrum window area. No flicker this way, no artifacts since we clear the window each time.            
            tft.boxGet(PAGE2_START_ADDR, ptr->l_graph_edge+1, ptr->sp_top_line+1, ptr->wf_sp_width, ptr->sp_bottom_line-1, ptr->l_graph_edge+1, ptr->sp_top_line+1);
            tft.check2dBusy();            
            tft.canvasImageStartAddress(PAGE1_START_ADDR);
            setActiveWindow_default();
        #endif

        // Update the span labels with current VFO frequencies    
        tft.setTextColor(myLT_GREY, myBLACK);
        tft.setFont(Arial_12);

        static uint32_t old_VFO_ = 0;

        if (old_VFO_ != _VFO_)
        {
            tft.fillRect( ptr->l_graph_edge, ptr->sp_txt_row, 110, 13, RA8875_BLACK);
            tft.setCursor(ptr->l_graph_edge, ptr->sp_txt_row);
            tft.print(_formatFreq(_VFO_ - (ptr->wf_sp_width*fft_bin_size)));       // Write left side of graph Freq
            
            tft.fillRect( ptr->c_graph-60, ptr->sp_txt_row, 110, 13, RA8875_BLACK);
            tft.setCursor(ptr->c_graph-60, ptr->sp_txt_row);
            tft.print(_formatFreq(_VFO_));   // Write center of graph Freq   
            
            tft.fillRect( ptr->r_graph_edge - 112, ptr->sp_txt_row, 110, 13, RA8875_BLACK);
            tft.setCursor(ptr->r_graph_edge - 112, ptr->sp_txt_row);
            tft.print(_formatFreq(_VFO_ + (ptr->wf_sp_width*fft_bin_size)));  // Write right side of graph Freq
            old_VFO_ = _VFO_;   // save to minimize updates for no reason.
        }
    }                
    else      // Clear stale data
    {
        if (spectrum_clear.check() == 1)      // Spectrum Screen blanking timer
        {
            Serial.println(F("*** Cleared Screen, no data to Draw! ***"));
            //tft.fillRect(ptr->l_graph_edge+1, ptr->sp_top_line+1, ptr->wf_sp_width-2, ptr->sp_height-2, myBLACK);
            //tft.drawFastVLine(ptr->l_graph_edge+ptr->wf_sp_width/2+1, ptr->sp_top_line+1, ptr->sp_height, myLT_GREY);
        }
    }
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
void Spectrum_RA887x::drawSpectrumFrame(uint8_t s)
{
    // See Spectrum_Parm_Generator() below for details on Global values requires and how the woindows variables are used.

    // s = The PRESET index into Sp_Parms_Def[] structure for windows location and size params.  Specify the default layout option for spectrum window placement and size.
    if (s >= PRESETS) s=PRESETS-1;   // Cycle back to 0
    // Test lines for alignment

    struct Spectrum_Parms *ptr = &Sp_Parms_Def[s];
    
    if (ptr->spect_wf_rate > 40)
        spectrum_waterfall_update.interval(ptr->spect_wf_rate);
    else
        spectrum_waterfall_update.interval(120);   // set to something acceptable in case the stored value does not exist or is too low.

    #ifdef PANADAPTER_INVERT
        fft_axis = 3;
    #else
        fft_axis = FFT_AXIS;   // normally =2
    #endif

    Serial.print("fft_axis=");Serial.println(fft_axis);
    myFFT.setXAxis(fft_axis);    // Set the FFT bin order to our needs
    
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
    //The scroll region is over the same area
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
    tft.drawLine(ptr->l_graph_edge+1+(ptr->wf_sp_width/2)+1, ptr->sp_tick_row,      ptr->l_graph_edge+1+(ptr->wf_sp_width/2)+1, ptr->sp_tick_row-ptr->tick_height, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+((ptr->wf_sp_width/4)*3), ptr->sp_tick_row,      ptr->l_graph_edge+((ptr->wf_sp_width/4)*3), ptr->sp_tick_row-ptr->tick_height, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+ptr->wf_sp_width-1,       ptr->sp_tick_row,      ptr->l_graph_edge+ptr->wf_sp_width-1,       ptr->sp_tick_row-ptr->tick_height, myLT_GREY);
    
    // draw the spectrum box center grid line for tuning help.
    //tft.drawFastVLine(ptr->l_graph_edge+1+ptr->wf_sp_width/2+1, ptr->sp_top_line+1, ptr->sp_height, myLT_GREY);
    
    // Draw the ticks on the bottom of Waterfall window also
    tft.drawLine(ptr->l_graph_edge+1,                         ptr->wf_bottom_line,   ptr->l_graph_edge+1,                        ptr->wf_tick_row, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+(ptr->wf_sp_width/4),      ptr->wf_bottom_line,   ptr->l_graph_edge+(ptr->wf_sp_width/4),     ptr->wf_tick_row, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+1+(ptr->wf_sp_width/2)+1,  ptr->wf_bottom_line,   ptr->l_graph_edge+1+(ptr->wf_sp_width/2)+1, ptr->wf_tick_row, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+(ptr->wf_sp_width/4)*3,    ptr->wf_bottom_line,   ptr->l_graph_edge+(ptr->wf_sp_width/4)*3,   ptr->wf_tick_row, myLT_GREY);
    tft.drawLine(ptr->l_graph_edge+ptr->wf_sp_width-1,        ptr->wf_bottom_line,   ptr->l_graph_edge+ptr->wf_sp_width-1,       ptr->wf_tick_row, myLT_GREY);    
}

//
//--------------------------------------------------  Spectrum init ------------------------------------------------------------------------
//
//   Must be called before any text is written tothe screen. If not that text will be corrupted.
//
void Spectrum_RA887x::initSpectrum(int16_t preset) // preset is the spectrum frame definition index value chosen by external functions.
{
    //tft.begin(RA8875_800x480);   // likely redundant but just in case and allows to be used standalone.
#ifdef USE_RA8875
    // Setup for scrollig attributes
    tft.useLayers(1);       //mainly used to turn of layers!    
    tft.writeTo(L1);         //L1, L2, CGRAM, PATTERN, CURSOR
    tft.setScrollMode(LAYER1ONLY);    // One of these 4 modes {SIMULTANEOUS, LAYER1ONLY, LAYER2ONLY, BUFFERED }
#else
    tft.selectScreen(PAGE1_START_ADDR);   // For the RA8876 this is the equivalent of Layer1Only
#endif

    //spectrum_preset = S;   // <<<==== Set this value.  Range is 0-PRESETS.  Specify the default layout option for spectrum window placement and size.
    
    // These values are only used to generate a new config record.  Cut and paste the results into the array to use.
    spectrum_x              = 0;      // 0 to width of display - window width. Must fit within the button frame edges left and right
                                                // ->Pay attention to the fact that position X starts with 0 so 100 pixels wide makes the right side value of x=99.
    spectrum_y              = 153;      // 0 to vertical height of display - height of your window. Odd Numbers are best if needed to make the height an even number and still fit on the screen
    spectrum_height         = 256;      // Total height of the window. Even numbers are best. (height + Y) cannot exceed height of the display or the window will be off screen.
    spectrum_center         = 50;       // Value 0 to 100.  Smaller value = biggger waterfall. Specifies the relative size (%) between the spectrum and waterfall areas by moving the dividing line up or down as a percentage
                                                // Smaller value makes spectrum smaller, waterfall bigger
    spectrum_width          = 799;      // Total width of window. Even numbers are best. 552 is minimum to fit a full 512 pixel graph plus the min 20 pixel border used on each side. Can be smaller but will reduce graph area
    spectrum_span           = 20;       // Value in KHz.  Ths will be the maximum span shown in the display graphs.  
                                                // The graph code knows how many Hz per bin so will scale down to magnify a smaller range.
                                                // Max value and resolutoin (pixels per bin) is dependent on sample frequency
                                                // 25000 is max for 1024 FFT with 500 bins at 1:1 bins per pixel
                                                // 12500 would result in 2 pixels per bin. Bad numbers here should be corrected to best fit by the function
    spectrum_wf_style       = 2;        // Range 1- 6. Specifies the Waterfall style.
    spectrum_wf_colortemp   = 330;      // Range 1 - 1023. Specifies the waterfall color temperature to tune it to your liking
    spectrum_wf_scale       = 1.0;      // 0.0f to 40.0f. Specifies thew waterfall zoom level - may be redundant when Span is worked out later.
    spectrum_LPFcoeff       = 0.9;      // 1.0f to 0.0f. Data smoothing
    spectrum_dot_bar_mode   = 1;        // 0=bar, 1=Line. Spectrum box
    spectrum_sp_scale       = 40;       // 10 to 80. Spectrum scale factor in dB. This is the height of the scale (if possible by windows sizes). Will plot the spectrum window of values between the floor and the scale value creating a zoom effect.
    spectrum_floor          = -175;      // 0 to -150. The reference point for plotting values.  Anything signal value > than this (less negative) will be plotted until stronger than the window height*scale factor.
    spectrum_wf_rate        = 70;          // window update rate in ms.  25 is fast enough to see dit and dahs well   
    spectrum_waterfall_update = Metro(spectrum_wf_rate); 
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
void Spectrum_RA887x::Spectrum_Parm_Generator(int16_t parm_set, int16_t preset)
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
    ptr->border_space_min = 0;  // Left and right side space. Graph space would be this this value*2 less.
    ptr->border_space = ptr->border_space_min;
    if (spectrum_width > tft.width())
        spectrum_width = tft.width();
    if (spectrum_width > (fft_bins*2) + (ptr->border_space_min*2) - 1)
    {  
        // space is wider than max graph size fft_bins to pad with border space and center graph area
        ptr->border_space = (spectrum_width - (fft_bins*2))/2;   // padding for each side
        ptr->wf_sp_width  = fft_bins*2;
    }
    else  // make smaller than FFT_bins
    {
        ptr->border_space = ptr->border_space_min;
        ptr->wf_sp_width = spectrum_width - (ptr->border_space*2) -1;
    }
    ptr->l_graph_edge     = spectrum_x + ptr->border_space;
    ptr->r_graph_edge     = ptr->l_graph_edge + ptr->wf_sp_width;      
    ptr->c_graph          = ptr->l_graph_edge + ptr->wf_sp_width/2 -1;  // center of graph window areas

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
    ptr->spect_wf_rate       = spectrum_wf_rate;
  
// print out results to the serial terminal for manual copy into the default table.  This is 1 set of data only, for each run.  
// Change the globals and run again for a new set

    Serial.println(F("Start of Spectrum Parameter Generator List."));
    Serial.println(F("This is a complete parameter record for the current window."));
    Serial.println(F("Cut and paste the data in the braces to modify the predefined records."));
    Serial.print(F("{"));
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
    Serial.print(ptr->spect_floor);  Serial.print(",");
    Serial.print(ptr->spect_wf_rate);  Serial.print("}");

    Serial.println(F("\nEnd of Spectrum Parameter Generator List"));
    Serial.print(F("Current Preset="));
    Serial.print(preset);  // display extrnal specified frame drawing definition 
    Serial.print(F("  Selected Preset="));
    Serial.print(parm_set);
    Serial.print(F("  Current Waterfall Style="));
    Serial.print(spectrum_wf_style);
    Serial.print(F("  Current Color Temp="));
    Serial.println(spectrum_wf_colortemp);
}
//
//--------------------------------------------------  find_FFT_Max(min, max) ------------------------------------------------------------------------
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
int16_t Spectrum_RA887x::_find_FFT_Max(uint16_t bin_min, uint16_t bin_max)    // args for min and max bins to look at based on display width.
{
    float specMax = -200.0f;
    uint16_t iiMax = 0;
    int16_t f_peak = 0.0f;
    
    uint16_t bin_center = (bin_max-bin_min)/2 + bin_min;

    //myFFT.setOutputType(FFT_POWER);   // change to power, return it to FFT_DBFS at end
    myFFT.windowFunction(AudioWindowHanning1024);
    // Get pointer to data array of powers, float output[512];
    //  setAxis(fft_axis)  // Called in drawSpectrumFrame() to set the bin order to lowest frequency at 0 bin and highest at bin 1023
    float *pPwr = myFFT.getData();
    // Find biggest bin
    for(int ii=bin_min; ii<bin_max; ii++)  
    {        
        if (ii == bin_center -1)        
            ii += 3;   // advance ii to skip the center few bins around our center frequency.

//Serial.print("ii="); Serial.print(ii); Serial.print("  binval="); Serial.println(*(pPwr + ii));  
        if (*(pPwr + ii) > specMax && *(pPwr + ii) < -1.0)   // filter out periodic blocks of 0 values 
        { // Find highest peak of range
          specMax = *(pPwr + ii);
          iiMax = ii;         
        }
    }
//Serial.print("iiMax="); Serial.print(iiMax); Serial.print(" specMax="); Serial.println(specMax);   
    if (specMax != 0.0)
    {
        float vm =  *(pPwr + iiMax - 1);
        float vc =  *(pPwr + iiMax);
        float vp =  *(pPwr + iiMax + 1);
        float R=0;

        if(vp > vm)  
        {
         // set global fftMaxPower = Power of the strongest signal if possible
           R = vc/vp;
        }
        else  
        {          
            R = vc/vm;
        }
        // return a frequency value adjusted to be relative to the center bin
        fftMaxPower = iiMax + (2-R)/(1+R);
        f_peak = fftMaxPower - bin_center;   //adjust for center
        fftMaxPower = myFFT.read(fftMaxPower)-20; // set global fftMaxPower = Power of the strongest signal if possible
                                                            // -20 is a cal factor experimentally determined
    }
    //Serial.print("iiMax="); Serial.print(iiMax); Serial.print(" fftMaxPower="); Serial.println(fftMaxPower); 
    return f_peak;    // return -200 unless there is a good value to send out
}

// Duplicate of the function in Display.h but included here to make the spectrum module self contained. Minor changes included
char* Spectrum_RA887x::_formatFreq(uint32_t Freq)
{
	static char Freq_str[15];
	
	uint16_t MHz = (Freq/1000000 % 1000000);
	uint16_t Hz  = (Freq % 1000);
	uint16_t KHz = ((Freq % 1000000) - Hz)/1000;
	sprintf(Freq_str, "%5d.%03d.%03d", MHz, KHz, Hz);
	//Serial.print("Freq: ");Serial.println(Freq_str);
	return Freq_str;
}
//
//____________________________________________________Color Mapping _____________________________________
//       
int16_t Spectrum_RA887x::_colorMap(int16_t val, int16_t color_temp) 
{
    float red;
    float green;
    float blue;
    float temp = val / 65536.0 * (color_temp);
  //Serial.print("temp="); Serial.println(temp);
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
    //Serial.print("  CM="); Serial.print(tft.Color565(red * 256, green * 256, blue * 256));Serial.print("  val="); Serial.println(val);Color24To565
    return _Color565(red * 256, green * 256, blue * 256);
}

/*
// Pass 8-bit (each) R,G,B, get back 16-bit packed color
static uint16_t Color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

//color565toRGB		- converts 565 format 16 bit color to RGB
static void color565toRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b) {
    r = (color>>8)&0x00F8;
    g = (color>>3)&0x00FC;
    b = (color<<3)&0x00F8;
}

//color565toRGB14		- converts 16 bit 565 format color to 14 bit RGB (2 bits clear for math and sign)
//returns 00rrrrr000000000,00gggggg00000000,00bbbbb000000000
//thus not overloading sign, and allowing up to double for additions for fixed point delta
static void color565toRGB14(uint16_t color, int16_t &r, int16_t &g, int16_t &b) {
    r = (color>>2)&0x3E00;
    g = (color<<3)&0x3F00;
    b = (color<<9)&0x3E00;
}

//RGB14tocolor565		- converts 14 bit RGB back to 16 bit 565 format color
static uint16_t RGB14tocolor565(int16_t r, int16_t g, int16_t b)
{
    return (((r & 0x3E00) << 2) | ((g & 0x3F00) >>3) | ((b & 0x3E00) >> 9));
}
*/

/* Copyright (C)
* 2015 - John Melton, G0ORX/N6LYT
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*-------------------------------------------------------------------------
* April 2021 - extracted the waterfall gradient part of waterfall.c and adapted it to provide 
* another method of waterfall coloring so the below is only a part of the original file
* for a different project PiHDSDR on GitHub.
* https://github.com/g0orx/pihpsdr/blob/master/waterfall.c
*
* --------------------------------------------------------------------------
* 
*/

// sample is the provide the FFT bin value.  waterfall low is the min threshold
int16_t Spectrum_RA887x::_waterfall_color_update(float sample, int16_t waterfall_low) 
{
    //int i;
    struct Spectrum_Parms *Gptr = Sp_Parms_Def;
    //int average=0;
    unsigned char rgb[3];
    unsigned char *p;

    static int colorLowR=0; // black
    static int colorLowG=0;
    static int colorLowB=0;

    //static int colorMidR=255; // red
    //static int colorMidG=0;
    //static int colorMidB=0;

    static int colorHighR=255; // yellow
    static int colorHighG=255;
    static int colorHighB=0;
    //int pan = 0;

    // introduce scale factor here possibly - might do something with average later also
    // but the pix_min seems to work well enough
    //if(have_rx_gain) {
    //  sample=samples[i+pan]+(float)(rx_gain_calibration-adc_attenuation[rx->adc]);
    //} else {
    //  sample=samples[i+pan]+(float)adc_attenuation[rx->adc];
    //}

    //waterfall_low += (Gptr->spect_sp_scale/-30);  // slight adjustment to lower the color temp a bit.
    waterfall_low += (Gptr->spect_floor/2) + (Gptr->spect_sp_scale/-30) + 8;  // slight adjustment to lower the color temp a bit.
    int16_t waterfall_high = -40;

    //Serial.print("FFT = " );Serial.println(sample);
    //Serial.print("WtrF Low = " );Serial.println(waterfall_low);

    //average+=sample;        
    //Serial.println(average);
    
    p = rgb;
    if(sample<(float)waterfall_low) {
        *p++=colorLowR;
        *p++=colorLowG;
        *p++=colorLowB;
    } else if(sample>(float)waterfall_high) {
        *p++=colorHighR;
        *p++=colorHighG;
        *p++=colorHighB;
    } else {
        float range=(float)waterfall_high-(float)waterfall_low;
        float offset=sample-(float)waterfall_low;
        float percent=offset/range;
        if(percent<(2.0f/9.0f)) {
            float local_percent = percent / (2.0f/9.0f);
            *p++ = (int)((1.0f-local_percent)*colorLowR);
            *p++ = (int)((1.0f-local_percent)*colorLowG);
            *p++ = (int)(colorLowB + local_percent*(255-colorLowB));
        } else if(percent<(3.0f/9.0f)) {
            float local_percent = (percent - 2.0f/9.0f) / (1.0f/9.0f);
            *p++ = 0;
            *p++ = (int)(local_percent*255);
            *p++ = 255;
        } else if(percent<(4.0f/9.0f)) {
                float local_percent = (percent - 3.0f/9.0f) / (1.0f/9.0f);
                *p++ = 0;
                *p++ = 255;
                *p++ = (int)((1.0f-local_percent)*255);
        } else if(percent<(5.0f/9.0f)) {
                float local_percent = (percent - 4.0f/9.0f) / (1.0f/9.0f);
                *p++ = (int)(local_percent*255);
                *p++ = 255;
                *p++ = 0;
        } else if(percent<(7.0f/9.0f)) {
                float local_percent = (percent - 5.0f/9.0f) / (2.0f/9.0f);
                *p++ = 255;
                *p++ = (int)((1.0f-local_percent)*255);
                *p++ = 0;
        } else if(percent<(8.0f/9.0f)) {
                float local_percent = (percent - 7.0f/9.0f) / (1.0f/9.0f);
                *p++ = 255;
                *p++ = 0;
                *p++ = (int)(local_percent*255);
        } else {
                float local_percent = (percent - 8.0f/9.0f) / (1.0f/9.0f);
                *p++ = (int)((0.75f + 0.25f*(1.0f-local_percent))*255.0f);
                *p++ = (int)(local_percent*255.0f*0.5f);
                *p++ = 255;
        }
    }

    //if(rx->waterfall_automatic) {
    //  waterfall_low=average/display_width;
    //  waterfall_high=waterfall_low+50;
    //}
    
    int16_t pval = _Color565(rgb[0], rgb[1], rgb[2]);
    //Serial.print("Final color = "); Serial.println(pval,HEX);
    return pval;
}
