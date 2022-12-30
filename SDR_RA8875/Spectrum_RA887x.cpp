//
// Spectrum_RA887x.cpp
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"
#include "Spectrum_RA887x.h" 

#ifdef USE_RA8875
    extern RA8875 tft;
#else
    extern RA8876_t3 tft;
#endif

//uint16_t fft_sz = 0;
//int16_t fft_binc = 0;
//float fft_bin_sz = 0;

#ifdef BETATEST
    extern float32_t  fftOutput[];  // Array used for FFT Output to the INO program
    extern float32_t  window[];     // Windows reduce sidelobes with FFT's *Half Size*
    extern float32_t  fftBuffer[];  // Used by FFT, 4096 real, 4096 imag, interleaved
    extern float32_t  sumsq[];      // Required ONLY if power averaging is being done
#endif

#ifdef FFT_4096
    #ifndef BETATEST
         extern AudioAnalyzeFFT4096_IQ_F32    myFFT_4096;  // choose which you like, set FFT_SIZE accordingly.
    #else
        extern AudioAnalyzeFFT4096_IQEM_F32  myFFT_4096;  // choose which you like, set FFT_SIZE accordingly.
    #endif
#endif
#ifdef FFT_2048    
    extern AudioAnalyzeFFT2048_IQ_F32  myFFT_2048;
#endif
#ifdef FFT_1024
    extern AudioAnalyzeFFT1024_IQ_F32  myFFT_1024;
#endif

#ifndef USE_RA8875
    int16_t _activeWindowXL = 0;
    int16_t _activeWindowXR = SCREEN_WIDTH;
    int16_t _activeWindowYT = 0;
    int16_t _activeWindowYB = SCREEN_HEIGHT;
#endif

#ifdef PANADAPTER_INVERT
    const uint16_t FFT_AXIS  = 3;  // 2 is typical, 3 for panadapter mode (upside down tuning) 0 for the beta shared memory FFT.

#else
    const uint16_t FFT_AXIS  = 2;  // 2 is typical, 3 for panadapter mode (upside down tuning) 0 for the beta shared memory FFT.
#endif

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
int16_t _find_FFT_Max(uint16_t bin_min, uint16_t bin_max, uint16_t fft_sz);
char* _formatFreq(uint32_t Freq);
//static uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);
//inline uint16_t _Color565(uint8_t r, uint8_t g, uint8_t b);
int16_t _waterfall_color_update(float sample, int16_t waterfall_low);

// Function Declarations
//-------------- COLOR CONVERSION -----------------------------------------------------------
inline uint16_t _Color565(uint8_t r,uint8_t g,uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }
inline uint16_t Color24To565(int32_t color_) { return ((((color_ >> 16) & 0xFF) / 8) << 11) | ((((color_ >> 8) & 0xFF) / 4) << 5) | (((color_) &  0xFF) / 8);}
inline uint16_t htmlTo565(int32_t color_) { return (uint16_t)(((color_ & 0xF80000) >> 8) | ((color_ & 0x00FC00) >> 5) | ((color_ & 0x0000F8) >> 3));}
inline void 	Color565ToRGB(uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b){r = (((color & 0xF800) >> 11) * 527 + 23) >> 6; g = (((color & 0x07E0) >> 5) * 259 + 33) >> 6; b = ((color & 0x001F) * 527 + 23) >> 6;}

int16_t wf_time_line                = 15000;
int16_t fftFreq_refresh             = 1000;
Metro   waterfall_timestamp         = Metro(wf_time_line);  // Used to draw a time stamp line across the waterfall window.  Cha
Metro   fftFreq_timestamp           = Metro(fftFreq_refresh);
Metro   spectrum_clear              = Metro(1000);
Metro   spectrum_waterfall_update   = Metro(80); // using default of 80.
int16_t spectrum_scale_maxdB = 1;    // max value in dB above the spectrum floor we will plot signal values (dB scale max)
int16_t spectrum_scale_mindB = 80;   // min value in dB above the spectrum floor we will plot signal values (dB scale max)
//static int16_t fftFrequency         = 0;  // Used to hold the FFT peak signal's frequency offsewt from Fc. Use a RF sig gen to measure its frequency and spot it on the display, useful for calibration
int16_t fftMaxPower;//          = 0;    // Used to hold the FFT peak power for the strongest signal
//static int16_t fft_avg              = 0;    // Internal FFT averaging feature. Must be >= 1.  Set this > 1 to trigger usage
const int8_t  NAvg                  = 6; //5;
//static uint32_t time_spectrum;

// Place to hold custom data for creating new layouts using the Generator function
struct Spectrum_Parms Sp_Parms_Custom[1]    = {};      // Temp storage for generating new layouts    
struct Spectrum_Parms *ptr                  = &Sp_Parms_Def[0];

//  ToDo: make work again to dump FFT data over ethernet.  Worked before moving to library.
    //#ifdef ENET
      //  extern uint8_t enet_ready;
        //extern unsigned long enet_start_fail_time;
        //extern uint8_t rx_count;
      //  extern uint8_t enet_data_out;
    //#endif

#ifndef USE_RA8875

/**************************************************************************/
void setActiveWindow(int16_t XL,int16_t XR ,int16_t YT ,int16_t YB)
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
void setActiveWindow_default(void)
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
void updateActiveWindow(bool full)
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

// -------------------------------------------------------------------------------------
//
//      Spectrum Update()
//
//      Updates the spectrum/waterfall windows with data from chosen FFT
//
//      JH add cyclecounter to track sequential partial updates
//
// -------------------------------------------------------------------------------------
//
int32_t spectrum_update(int16_t s, int16_t VFOA_YES, uint32_t VfoA, uint32_t VfoB, int32_t Offset, uint16_t filterCenter, uint16_t filterBandwidth, float _pan, uint16_t fft_sz, float fft_bin_sz, int16_t fft_binc)
{
//    s = The PRESET index into Sp_Parms_Def[] structure for windows location and size params  
//    Specify the default layout option for spectrum window placement and size.
//    
//    This function only uses values from the Sp_Parms_Def[] struct (later Sp_Parms_Custom[]).  To update the structure
//    records set the global variables the call the Spectrum_Generator() function and copy and paste the output displayed 
//    on the Serial Terminal into the default array init table.

	//if (s >= PRESETS)
    //    s=PRESETS-1;   // Cycle back to 0
    // See Spectrum_Parm_Generator() below for details on Global values requires and how the woindows variables are used.    
    //struct Spectrum_Parms *ptr = &Sp_Parms_Def[s];
    *ptr = Sp_Parms_Def[s];
 
    static uint8_t cycleCounter         = 0; //JH
	//JH: put this here and make it static
    static int16_t line_buffer[SCREEN_WIDTH+2];      // Will only use the x bytes defined by wf_sp_width var.  Could be 4096 FFT later which is larger than our width in pixels. 
    //JH moved to top of function
    static uint32_t old_VFO_            = 0;
    //JH moved to top of function, made static
    static uint32_t _VFO_;   // Get active VFO frequency
	//int16_t blanking = 3; //3;  // used to remove the DC line from the graphs at Fc
    int16_t pix_o16;
    int16_t pix_n16;
    //static int16_t spect_scale_last   = 0;
    //static int16_t spect_ref_last     = 0;
    static int16_t fft_pk_bin           = 0;
    static int16_t fftPower_pk_last     = ptr->spect_floor;
    static int16_t pix_min              = ptr->spect_floor;
    static int32_t freq_peak            = 0;
    static float old_fft_sz             = 0;        // used to update the spectrum scale frequency labels when the FFT size changes and VFO does not
    static int16_t L_EDGE               = 0; 
    static int32_t L_EDGE_no_pan        = 0;        // internediate calculation used to pan
    static float old_pan                = 0.0f;        // update screen freq data when pan setting changes
    static int16_t pan                  = 0;

    //for testing alignments
    //tft.drawRect(spectrum_x, spectrum_y, spectrum_width, spectrum_height, myBLUE);  // x start, y start, width, height, array of colors w x h
    //tft.drawRect(ptr->spect_x, ptr->spect_y, ptr->spect_width, ptr->spect_height, myBLUE);  // x start, y start, width, height, array of colors w x h
     
    int16_t         i;
    static float    avg = 0.0f;
	//JH make pixelnew static
    static int16_t  pixelnew[SCREEN_WIDTH+2];           //  Stores current pixel for spectrum portion only
    static int16_t  pixelold[SCREEN_WIDTH+2];    //  Stores copy of current pixel so it can be erased in next update
    //int8_t span_FFT[SCREEN_WIDTH+2];         // Intended to store averaged values representnig a larger FFT set into the smaller screen width set
    float           *pout=NULL;

    // JH first in sequence
    if (cycleCounter==0) 
    {
        // While more than 1 FFT may be enabled, only 1 (fft_size) is chosen for display
        uint8_t process_FFT;

        process_FFT = 0;   // assume no data
        
        #ifdef FFT_4096
            if (fft_sz == 4096 && myFFT_4096.available())
            {
                #ifndef BETATEST
                    pout = myFFT_4096.getData();
                #else
                    pout = fftOutput;
                #endif
                process_FFT = 1;  // have valid fft data to show
            }
        #endif
        #ifdef FFT_2048
            if (fft_sz == 2048 && myFFT_2048.available())
            {
                pout = myFFT_2048.getData();
                process_FFT = 1;  // have valid fft data to show
            }
        #endif
        #ifdef FFT_1024
            if (fft_sz == 1024 && myFFT_1024.available())
            {
                pout = myFFT_1024.getData();
                process_FFT = 1;  // have valid fft data to show
            }
        #endif

        if (process_FFT == 1)  // have valid fft data to show
        {
            cycleCounter=1;     
            //float *pout = myFFT.getData();          // Get pointer to data array of powers, float output[512]; 
            // Only 1 of the FFT outputs can be displayed

            //JH change to static at top of function
            // int16_t line_buffer[SCREEN_WIDTH+2];      // Will only use the x bytes defined by wf_sp_width var.  Could be 4096 FFT later which is larger than our width in pixels. 

            #ifdef ENET
                extern uint8_t enet_write(uint8_t *tx_buffer, const int count);
                extern uint8_t tx_buffer[];

                if (enet_data_out && enet_ready)
                {
                    for (i = 0; i < fft_sz; i++)
                    {
                        tx_buffer[i] = (uint8_t) fabsf(*(pout+i));
                    }
                    //memcpy(tx_buffer, full_FFT, fft_sz);
                    enet_write(tx_buffer, fft_sz);
                }
            #endif
            // ToDO: Check if this is needed anymore
            // Calculate center. If FFT is larger than graph area width, trim ends evently
            if (ptr->spect_span == 50)   //span width in KHz.   50 is just used for dev test.
            {
                // pack all bins into the available display width.
                //int wd = ptr->wf_sp_width;
                //int div = ptr->spect_span;
                //int binsz = round(fft_sz/wd);  // bins that will be compressed into 1 pixel to fit the screen
                for (i = 0; i < ptr->wf_sp_width; i++)
                {
                    if ( i > SCREEN_WIDTH) // do not overrun our buffer size.  Ideally wf_sp_width would never be > SCREENWIDTH but....
                        i = SCREEN_WIDTH;
                    //span_FFT[i] = (int16_t) sp_FFT.read(binsz*i);
                }
                //pout = span_FFT;
            // DPRINT("Zoom Out =");
            //DPRINTLN(binsz,DEC);
            }
            else if ( fft_sz > ptr->wf_sp_width-2)  // When FFT data is > available graph area
            {
                pan = (int16_t) (_pan * (fft_sz - SCREEN_WIDTH));  // pan comes in as is -0.50f to +0.50f ==> calc # of bins to shift
                L_EDGE_no_pan = (int16_t) ((fft_sz - ptr->wf_sp_width)/2); // left edge calc from reference center
                L_EDGE = L_EDGE_no_pan + pan;  // shift the spectrum up to the max that the screen size can handle
                pout = pout+L_EDGE;  // adjust the starting point up a bit to keep things centered.
            }
            // ToDo: Figure out if this is needed someday.
            // else   // When FFT data is < available graph area
            // {      // If our display area is less then our data width, fill in the outside areas with low values.
                //L_EDGE = (ptr->wf_sp_width - fft_sz - )/2;
                //pout = pout+L_EDGE;  // adjust the starting point up a bit to keep things centered.
                /*
                for (i=0; i< fft_sz/4; i++)
                    tempfft[i] = -500;
                for (i=(fft_sz/4)*3; i< fft_size; i++)
                    tempfft[i] = -500;
                //L_EDGE = FFT_center - GRAPH_center;
                */
            // }

            for (i = 0; i < ptr->wf_sp_width; i++)        // Grab all FFT values.  Need to do at one time since averaging is looking at many values in this array
            {
                if (isnanf(*(pout+i)) || isinff (*(pout+i)))    // trap float 'NotaNumber NaN" and Infinity values
                {
                DPRINTLN(F("FFT Invalid Data INF or NaN"));
                    //Serial.println(*(pout+i));
                    pixelnew[i] = -200;   // fill in the missing value with somting harmless
                    //pixelnew[i] = sp_FFT.read(i+1);  // hope the next one is better.
                }
                // Now capture Spectrum value for use later
                pixelnew[i] = (int16_t) *(pout+i);

                // Several different ways to process the FFT data for display. Gather up a complete FFT sample to do averaging then go on to update the display with the results
                switch (ptr->spect_wf_style)
                {
                    case 0: if ( i > 1 )  // prevent reading array out of bounds < 1. 
                        {
                            avg = *(pout+(i*16/10))*0.5 + *(pout+(i-1)*16/10)*0.18 + *(pout+(i-2)*16/10)*0.07 + *(pout+(i+1)*16/10)*0.18 + *(pout+(i+2)*16/10)*0.07;                
                            //line_buffer[i] = (LPFcoeff * 8 * sqrt (100+(abs(avg)*wf_scale)) + (1 - LPFcoeff) * line_buffer[i]);
                            line_buffer[i] = (ptr->spect_LPFcoeff * 8 * sqrtf(fabsf(avg)) + (1 - ptr->spect_LPFcoeff) * line_buffer[i]);                      
                        }
                            break;
                    case 1: if ( i > 1 )  // prevent reading array out of bounds < 1.
                        {
                            avg = *(pout+i)*0.5 + *(pout+i-1)*0.18 + *(pout+i-2)*0.07 + *(pout+i+1)*0.18 + *(pout+i+2)*0.07;
                            line_buffer[i] = ptr->spect_LPFcoeff * 8 * sqrtf(fabsf(avg)) + (1 - ptr->spect_LPFcoeff);
                            line_buffer[i] = _colorMap(line_buffer[i], ptr->spect_wf_colortemp);
                            //Serial.println(line_buffer[i]);
                        }
                            break;
                    case 2: avg = line_buffer[i] = _colorMap(fabsf(*(pout+i)) * 1.9 *  ptr->spect_wf_scale, ptr->spect_wf_colortemp);
                            break;
                    case 3: avg = line_buffer[i] = _colorMap(fabsf(*(pout+i)) * 0.4 *  ptr->spect_wf_scale, ptr->spect_wf_colortemp);
                            break;
                    case 4: avg = line_buffer[i] = _colorMap(16000 - fabsf(*(pout+i)), ptr->spect_wf_colortemp) * ptr->spect_wf_scale;
                            break;
                    case 6: avg = line_buffer[i] = _waterfall_color_update(*(pout+i), pix_min);//  * ptr->spect_sp_scale;  // test new waterfall colorization method
                            break;
                    case 5:
                    default: avg = line_buffer[i] = _colorMap(fabsf(*(pout+i)), ptr->spect_wf_colortemp);
                        break;
                };

                //DPRINTLN(tft.gradient( (uint16_t) pix_n16));
                /* Used for VFO always on center of screen - commented out while trying to shift the VFO up screen to remove DC gap
                // Does not seem to be needed when SetNAverage is 3 or more + maybe AudioHighPassFilterEnable() on?
                // Fc Blanking
                if (i >= (ptr->wf_sp_width/2)-blanking  && i <= (ptr->wf_sp_width/2)+blanking+1)
                {
                    line_buffer[i] = myBLACK;
                    if (i == ((ptr->wf_sp_width)/2) + 1)
                        line_buffer[i] = myLT_GREY;  // draw center Fc line in waterfall
                }
                */
            }   // Done with copying the FFT output array
        } 
    }
	// start of second sequence:waterfall
	
	else if (cycleCounter==1)
    {
        for (i = 2; i < (ptr->wf_sp_width-1); i++)
        {
            if (i == 2)   // start with 2 because the end values contain special purpose or used for averaging
            {
                pix_min = pixelnew[2];  // start off each set with a sample value to compare others with
            }
            else
            {
                if (pixelnew[i] < pix_min)
                    pix_min = pixelnew[i];
            }
        }

        // ***************************************************************************************************
        //
        //      UPDATE WATERFALL 
        //      Takes a snapshot of the current window without the bottom row. Stores it in Layer 2 then brings it back beginning at the 2nd row. 
        //      Then write new row data into the missing top row to get a scroll effect using display hardware, not the CPU.
        //      Documentation for BTE: BTE_move(int16_t SourceX, int16_t SourceY, int16_t Width, int16_t Height, int16_t DestX, int16_t DestY, uint8_t SourceLayer=0, uint8_t DestLayer=0, bool Transparent = false, uint8_t ROP=RA8875_BTEROP_SOURCE, bool Monochrome=false, bool ReverseDir = false);
        //
        // ***************************************************************************************************
        #ifdef USE_RA8875
            tft.BTE_move(ptr->l_graph_edge+1, ptr->wf_top_line+1, ptr->wf_sp_width, ptr->wf_height-4, ptr->l_graph_edge+1, ptr->wf_top_line+2, 1, 2);  // Layer 1 to Layer 2
            while (tft.readStatus());  // Make sure it is done.  Memory moves can take time.

// 7-8ms to get to here
            // Move the block back on Layer 1 but place it 1 row down from the top
            tft.BTE_move(ptr->l_graph_edge+1, ptr->wf_top_line+2, ptr->wf_sp_width, ptr->wf_height-4, ptr->l_graph_edge+1, ptr->wf_top_line+2, 2);  // Move layer 2 up to Layer 1 (1 is assumed).  0 means use current layer.
            while (tft.readStatus());   // Make sure it is done.  Memory moves can take time.        
      
// 15-16ms to get to here    The while() make no delays
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
            tft.drawRect(ptr->l_graph_edge+1, ptr->wf_top_line+2, 20, 1, LIGHTGREY);  // x start, y start, width, height, colors w x h
            //tft.drawFastHLine(ptr->l_graph_edge+1, ptr->wf_top_line+1, ptr->wf_sp_width, myLT_GREY);  // x start, y start, width, height, colors w x h           
        else  // Draw the new line at the top
            tft.writeRect(ptr->l_graph_edge+1, ptr->wf_top_line+1, ptr->wf_sp_width, 1, (uint16_t*) &line_buffer);  // x start, y start, width, height, array of colors w x h

//  16ms to get to here
        cycleCounter=2;
    }
//--------------------------------  Spectrum Window ------------------------------------------
//
//      UPDATE SPECTRUM
//      Done with waterfall, now UPDATE SPECTRUM section
//      start at 2 to prevent reading out of bounds during averaging formula
//      Draw our image on canvas 2 which is not visible
//    JH - process this part in 8 sequential steps
//
// -------------------------------------------------------------------------------------------
    else if(cycleCounter>1 && cycleCounter < 3) 
    {  
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
        
		if (cycleCounter==2)
        {  // do only on first spectrum window sequence
			// Erase old spectrum window
			tft.fillRect(ptr->l_graph_edge+1, ptr->sp_top_line+1, ptr->wf_sp_width, ptr->sp_height-2, BLACK);
			
			// Draw in filter bandwidth "shaded" area
			int8_t filt_side = 0;
			if (Offset == 1 || Offset == 0 || Offset == -1)
			{
				filt_side = Offset;  //Figure out mode to shade correct side
			}
			else
			{
				if (Offset > 1) filt_side = 1;
				else filt_side = -1;
			}

			// Draw the filter width shaded box.  Translucent would be better.  Correct for pan offset
			tft.fillRect(ptr->l_graph_edge+ptr->wf_sp_width/2+2+((filterCenter/fft_bin_sz/2)*filt_side)-(filterBandwidth/fft_bin_sz/2/2)-pan, ptr->sp_top_line+1, filterBandwidth/fft_bin_sz/2, ptr->sp_height-2, myVERY_DARK_GREEN);
        }
        //---------------------------------------------------------------------------------------------------
        // Now draw the spectrum lines
        // --------------------------------------------------------------------------------------------------
                
        // Average a few values to smooth the line a bit
        // Can likely replace this by trying different FFT.setNAverage values
        //float avg_pix2 = (pixelnew[i]+pixelnew[i+1])/2;     // avg of 2 bins            
        //float avg_pix5 = (pixelnew[i-2]+pixelnew[i-1]+pixelnew[i]+pixelnew[i+1]+pixelnew[i+2])/5;   //avg of 5 bins
        //if (fabsf(pixelnew[i]) > fabsf(avg_pix2) * 1.6f)    // compare to a small average to toss out wild spikes
        //    pixelnew[i] = (int16_t) avg_pix5;               // average it out over a wider segment to patch the hole   

        //Serial.print("pix min =");DPRINTLN(pix_min);
   
        // 19 - 20ms to get to here.
        //JH calculate the appropriate index range for each sequential update
        uint16_t indexStart= (cycleCounter-2)*(ptr->wf_sp_width);  //  /2 update in 2 parts //  /8 for 8 parts
        uint16_t indexEnd= indexStart+ptr->wf_sp_width;
        if (indexStart>2) indexStart=2;
        if (indexEnd > ptr->wf_sp_width-1) indexEnd=ptr->wf_sp_width-1;
        // was:  for (i = 2; i < (ptr->wf_sp_width-1); i++) 
        for (i = indexStart; i < indexEnd; i++)
        {       
            // Temp commented out for fixed offset coding - May not be needed anymore
            //            if (i >= (ptr->wf_sp_width/2)-blanking-1 && i <= (ptr->wf_sp_width/2)+blanking+1)
            //                pixelnew[i] = -200;

            //#define DBG_SPECTRUM_SCALE
            //#define DBG_SPECTRUM_PIXEL
            //#define DBG_SPECTRUM_WINDOWLIMITS
            
            #ifdef DBG_SPECTRUM_PIXEL
                DPRINT(" raw =");
                DPRINT(pixelnew[i],DEC);
            #endif
            
            // limit the upper and lower dB level to between these ranges (set scale) (User Setting)  Can be limited further by window heights   
            //spectrum_scale_maxdB = 1;    // Scale most zoomed in. This is +10dB above the spectrum floor value.   That value is adjustables and is our refence point set to the bottom line.  
                                            // Forms the top range of values that line up with the top of our "window" on the FFT data set value range, typiclly -150 to -0dBm possible.
            //spectrum_scale_mindB = 80;    // Scale most zoomed out. This is +80 dB relative to the spectrum_floor so teh top end of our window.  Typically -150 to -0dBm possible range of signal.             
            // range limit our settings. This number is added to the spectrum floor.  The pixel value will be plotted where ever it lands as along as it is in the window.

            #ifdef DBG_SPECTRUM_SCALE
                DPRINT("   SC_ORG=");DPRINT(ptr->spect_sp_scale);
            #endif
        
            ptr->spect_sp_scale = constrain(ptr->spect_sp_scale, spectrum_scale_maxdB, spectrum_scale_mindB);

            #ifdef DBG_SPECTRUM_SCALE
                DPRINT("   SC_LIM=");DPRINT(ptr->spect_sp_scale);
            #endif
                
            #ifdef DBG_SPECTRUM_SCALE
                DPRINT("   SC_HT=");DPRINT(ptr->spect_sp_scale);
                DPRINT("   HT=");DPRINT(ptr->sp_height-4);
                DPRINT("   SC_FLR=");DPRINT(ptr->spect_floor);
            #endif       
            
            // Invert the sign since the display is also inverted, Increasing value = weaker signal strength, they are now going the same direction.  
            // Small value = bigger signal, closer to 0 on the display coordinates
            pixelnew[i] = (int16_t) fabsf(pixelnew[i]);   
            
            // We are plotting our pixel in the window if it lands between the bottom line and top lines        
            // set the grass floor to just above the bottom line.  These are the weakest signals. Typically -90 coming out of the FFT right now
            // Offset the pixel position relative to the bottom of the window
            pixelnew[i] +=  ptr->spect_floor;      

            //#if defined (DBG_SPECTRUM_PIXEL) || defined (DBG_SPECTRUM_WINDOWLIMITS)
            //Serial.print("  NF  pix =");DPRINTLN(pixelnew[i],DEC);
            //#endif

            pixelnew[i] = map(pixelnew[i], fabsf(pix_min), fabsf(ptr->spect_sp_scale), ptr->sp_bottom_line, ptr->sp_top_line); 
            
            #ifdef DBG_SPECTRUM_WINDOWLIMITS 
                //DPRINT("  win-ht:");DPRINT(ptr->sp_height-4);
                DPRINT("  top line=");DPRINT(ptr->sp_top_line+2);
            #endif
            
            //#if defined (DBG_SPECTRUM_PIXEL) || defined (DBG_SPECTRUM_WINDOWLIMITS)
            //DPRINT("  MAP pix =");DPRINTLN(pixelnew[i],DEC);
            //#endif

            #ifdef DBG_SPECTRUM_WINDOWLIMITS 
                DPRINT("  bottom line=");DPRINT(ptr->sp_bottom_line-2);
            #endif

            //#define DBG_SHOW_OVR
            
            if (pixelnew[i] < ptr->sp_top_line+1)        
            {
                #if defined(DBG_SPECTRUM_WINDOWLIMITS) || defined(DBG_SPECTRUM_PIXEL) || defined(DBG_SPECTRUM_SCALE) || defined(DBG_SHOW_OVR) 
                    DPRINT(" !!OVR!! = ");   DPRINTLN(pixelnew[i] - ptr->sp_top_line+2,0);
                #endif
                pixelnew[i] = ptr->sp_top_line+1;

            }

            if (pixelnew[i] > ptr->sp_bottom_line-1)        
            { 
                #if defined(DBG_SPECTRUM_WINDOWLIMITS) || defined(DBG_SPECTRUM_PIXEL) || defined(DBG_SPECTRUM_SCALE) || defined(DBG_SHOW_OVR)
                    DPRINT(" !!UNDER!! = "); DPRINTLN(pixelnew[i] - ptr->sp_top_line+2,0);
                #endif
                pixelnew[i] = ptr->sp_bottom_line-1;
            }          
            
            pix_n16 = pixelnew[i];  // convert float to uint16_t to match the draw functions type
            pix_o16 = pixelold[i];
   
//
//------------------------ Code below is writing only in the active spectrum window ----------------------
//                Limit access to the spectrum box to control misbehaved pixel and bar draws
//

// TEMP commented out for fixed offset coding tests
//            if (i < (ptr->wf_sp_width/2)-5 || i > (ptr->wf_sp_width/2) + 5)   // blank the DC carrier noise at Fc
//            {
                if ((i < ptr->wf_sp_width-2) && (pix_n16 > ptr->sp_top_line+2) && (pix_n16 < ptr->sp_bottom_line-2)  ) // will blank out the center spike
                {   
                    if (ptr->spect_dot_bar_mode == 0)   // BAR Mode 
                    {
                        if (pix_o16 != pix_n16) 
                        {                          
                            if (pix_n16 > ptr->sp_top_line && pix_n16 < ptr->sp_bottom_line-1)
                            {
                                //common way: draw bars                                                                        
                                tft.drawFastVLine(ptr->l_graph_edge+i, pix_n16, ptr->sp_bottom_line-pix_n16, YELLOW); //BLACK);                             
                                pixelold[i] = pixelnew[i];
                            }
                        }
                    }
                    else  // was DOT mode, now LINE mode
                    {   
                        // This will be drawn on Canvas 2 if this is a RA8876, layer 2 if a RA8875                               
                        tft.drawLine(ptr->l_graph_edge+i, pixelnew[i-1], ptr->l_graph_edge+i, pixelnew[i], YELLOW);
                        pixelold[i] = pixelnew[i]; 
                    }
                }
//            }
        } // end of spectrum pixel plotting
		    
        #ifdef USE_RA8875
            // Use BTE_Move to copy our fresh drawn spectrum form layer 2 to Layer 1
            tft.writeTo(L1);         //L1, L2, CGRAM, PATTERN, CURSOR
            // tft.BTE_move(ptr->l_graph_edge+1, ptr->sp_top_line+1, ptr->wf_sp_width, ptr->sp_height-2, ptr->l_graph_edge+1, ptr->sp_top_line+1, 2);  // Move layer 2 up to Layer 1 (1 is assumed).  0 means use current layer.            
            //while (tft.readStatus());   // Make sure it is done.  Memory moves can take time.
            tft.setActiveWindow();
        #else
            // BTE block copy it to page 1 spectrum window area. No flicker this way, no artifacts since we clear the window each time.            
            //tft.boxGet(PAGE2_START_ADDR, ptr->l_graph_edge+1, ptr->sp_top_line+1, ptr->wf_sp_width, ptr->sp_bottom_line-1, ptr->l_graph_edge+1, ptr->sp_top_line+1);
            //tft.check2dBusy();            
            tft.canvasImageStartAddress(PAGE1_START_ADDR);
            setActiveWindow_default();
        #endif
		
		cycleCounter++;
	}

// 36-44ms to get to here

        // Draw Grid Lines 
    else if (cycleCounter==3)
    {

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

        //if (i == (ptr->wf_sp_width/2))  // Just draw once per update cycle
        //{
        // draw a grid line for XX dB level             
            tft.setTextColor(LIGHTGREY, BLACK);
            tft.setFont(Arial_10);
            
            int grid_step = ptr->spect_sp_scale;
            for (int16_t j = grid_step; j < ptr->sp_height-10; j+=grid_step)
            {        
                //if (pix_n16 > ptr->sp_top_line+j+2 && pix_n16 < ptr->sp_bottom_line-2)
                //{    
                    // draw bottom most grid line
                    tft.drawFastHLine(ptr->l_graph_edge+24, ptr->sp_bottom_line-j,   ptr->wf_sp_width-24,    LIGHTGREY); // GREEN);
                    // write the scale value for the grid line
                    tft.setCursor(ptr->l_graph_edge+5, ptr->sp_bottom_line-j-5);
                    tft.print(j); 
                //}
            }
           
            // redraw the pitch line if in CW modes (Offset not 0).  Offset is in HZ so corect for current fft bin size
            if (Offset < -1 || Offset > 1)  // only draw for CW modes
            {
                tft.drawFastVLine(ptr->l_graph_edge+ptr->wf_sp_width/2+2+(Offset/fft_bin_sz/2)-pan, ptr->sp_top_line+1, ptr->sp_height, RED);
                tft.drawFastVLine(ptr->l_graph_edge+ptr->wf_sp_width/2+3+(Offset/fft_bin_sz/2)-pan, ptr->sp_top_line+1, ptr->sp_height, RED);
            }
            else // redraw the center line
            {   
                tft.drawFastVLine(ptr->l_graph_edge+ptr->wf_sp_width/2+2-pan, ptr->sp_top_line+1, ptr->sp_height, RED);
                tft.drawFastVLine(ptr->l_graph_edge+ptr->wf_sp_width/2+3-pan, ptr->sp_top_line+1, ptr->sp_height, RED);
            }
        //}

//--------------------------------------------------------------------------------------------------------------------
//
//      Drawing work is done, now update the information text
//
//--------------------------------------------------------------------------------------------------------------------
        // Update graph scale, ref level, power and freq
        tft.setTextColor(LIGHTGREY, BLACK);
        tft.setFont(Arial_12);

// 39-54ms to get to here.  45-51 more typical

        fft_pk_bin = _find_FFT_Max(L_EDGE+2, L_EDGE+ptr->wf_sp_width-2, fft_sz);   // get new frequency and power values for strongest signal 

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
        }
                            
        if (fftFreq_timestamp.check() == 1)
        {
            fftPower_pk_last = -200;  // reset the timer since we have new good data
            //Serial.println("Reset");
        }

        //  The next 4 screen updates take 19-20ms.  Not likely worth it so leaving these commented out.  
        //   Total spectrum time reduces to 60ms from 80ms
        //time_spectrum = millis();
        
        // Calculate and print the frequency of the strongest signal if possible 
        //DPRINT("Freq=");DPRINTLN(fftFrequency, 3); 
        //tft.fillRect(ptr->l_graph_edge+109,    ptr->sp_txt_row+30, 140, 13, BLACK);
        tft.setCursor(ptr->l_graph_edge+110,  ptr->sp_txt_row+30);
        tft.print("F: "); 
        tft.setCursor(ptr->l_graph_edge+126,  ptr->sp_txt_row+30);
        float pk_temp = (2 * (fft_bin_sz * (fft_pk_bin + pan)));   // relate the peak bin to the center bin
        freq_peak = _VFO_ + pk_temp;
        tft.print(_formatFreq(freq_peak));
        /*
        // Write the Scale value 
        tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+50, ptr->sp_txt_row+30);
        tft.print("S:   "); // actual value is updated elsewhere   
        //tft.fillRect( ptr->l_graph_edge+(ptr->wf_sp_width/2)+64, ptr->sp_txt_row+30, 32, 13, BLACK);       
        tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+64, ptr->sp_txt_row+30);
        tft.print(ptr->spect_sp_scale);          
        if (spect_scale_last != ptr->spect_sp_scale)
        {      
            spect_scale_last = ptr->spect_sp_scale;   // update memory
        }
        
        // Write the Reference Level to top line area
        tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+100, ptr->sp_txt_row+30);
        tft.print("R:   ");  // actual value is updated elsewhere            
        //tft.fillRect( ptr->l_graph_edge+(ptr->wf_sp_width/2)+114, ptr->sp_txt_row+30, 32, 13, BLACK);
        tft.setCursor(ptr->l_graph_edge+(ptr->wf_sp_width/2)+114, ptr->sp_txt_row+30);
        tft.print(ptr-> spect_floor);
        //DPRINT("R lvl=");DPRINTLN(ptr-> spect_floor);
        //if (spect_ref_last != ptr->spect_floor)
        //{
            //spect_ref_last = ptr->spect_floor;   // update memory
        //}    
            
        // Write the dB range of the window 
        tft.setTextColor(LIGHTGREY, BLACK);
        tft.setFont(Arial_10);
        tft.setCursor(ptr->r_graph_edge-50, ptr->sp_top_line+8);
        tft.print("H:   ");  // actual value is updated elsewhere
        tft.setCursor(ptr->r_graph_edge-38, ptr->sp_top_line+8); 
        tft.print(ptr->sp_height);
        */
        // Reset spectrum screen blanking timeout
        spectrum_clear.reset();
    
        //Serial.println(millis()-time_spectrum);
        // 19-21ms from fft_pk_bin to here.

        #ifdef USE_RA8875
            // Use BTE_Move to copy our fresh drawn spectrum form layer 2 to Layer 1
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
        cycleCounter=4;
	}
    //
    //------------------------ Code above is writing only in the active spectrum window ----------------------
    //
    // 27-28ms from fft_pk_bin to here.

    //-----------------------   This part onward is outside the active spectrum window and al ------------------------------
    //
    else if (cycleCounter==4)
    {
        // Update the span labels with current VFO frequencies    
        tft.setTextColor(LIGHTGREY, BLACK);
        tft.setFont(Arial_12);

        if (old_VFO_ != _VFO_ || old_fft_sz != fft_sz || old_pan != pan)
        {
            int32_t pan_freq = pan*fft_bin_sz*2;

            tft.fillRect( ptr->l_graph_edge, ptr->sp_txt_row, 110, 13, BLACK);
            tft.setCursor(ptr->l_graph_edge, ptr->sp_txt_row);
            tft.print(_formatFreq(_VFO_ + (uint32_t) (pan_freq - (int32_t)(ptr->wf_sp_width*fft_bin_sz))));       // Write left side of graph Freq
            
            tft.fillRect( ptr->c_graph-60, ptr->sp_txt_row, 110, 13, BLACK);
            tft.setCursor(ptr->c_graph-60, ptr->sp_txt_row);
            tft.print(_formatFreq(_VFO_ + (uint32_t) pan_freq));   // Write center of graph Freq   
            
            tft.fillRect( ptr->r_graph_edge - 112, ptr->sp_txt_row, 110, 13, BLACK);
            tft.setCursor(ptr->r_graph_edge - 112, ptr->sp_txt_row);
            tft.print(_formatFreq(_VFO_ + (uint32_t) (pan_freq + (int32_t)(ptr->wf_sp_width*fft_bin_sz))));  // Write right side of graph Freq
            
            // Update our change detector vars
            old_VFO_ = _VFO_;       // save to minimize updates for no reason.
            old_fft_sz = fft_sz;    // used to update the spectrum scale frequency labels when the FFT size changes and VFO does not
            old_pan = pan;          // update when the pan control changes
        }
        cycleCounter=5;
    }                
    else if(cycleCounter==5)
    {     // Clear stale data
        if (spectrum_clear.check() == 1)      // Spectrum Screen blanking timer
        {
           DPRINTLN(F("*** Cleared Screen, no data to Draw! ***"));
            //tft.fillRect(ptr->l_graph_edge+1, ptr->sp_top_line+1, ptr->wf_sp_width-2, ptr->sp_height-2, myBLACK);
            //tft.drawFastVLine(ptr->l_graph_edge+ptr->wf_sp_width/2+1, ptr->sp_top_line+1, ptr->sp_height, myLT_GREY);
        }
      
        cycleCounter=0;  //we're done! start a new spectrum
    }

// 0ms to get to here from the  end of spectrum BTW block move

    return freq_peak;  // freq_peak;  // for use by the main program for more accurate touch tuning
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
FLASHMEM void drawSpectrumFrame(uint8_t s)
{
    // See Spectrum_Parm_Generator() below for details on Global values requires and how the woindows variables are used.

    // s = The PRESET index into Sp_Parms_Def[] structure for windows location and size params.  Specify the default layout option for spectrum window placement and size.
    //if (s >= PRESETS) s=PRESETS-1;   // Cycle back to 0
    
    //if (ptr->spect_wf_rate > 40)
        spectrum_waterfall_update.interval(ptr->spect_wf_rate);
    //else
    //    spectrum_waterfall_update.interval(2);   // set to something acceptable in case the stored value does not exist or is too low.

    //Serial.print("fft_axis=");Serial.println(fft_axis);

    // Choose our output type.  Can do dB, RMS or power
    #ifdef FFT_4096
        myFFT_4096.setXAxis(fft_axis);    // Set the FFT bin order to our needs
        myFFT_4096.setOutputType(FFT_DBFS); // FFT_RMS or FFT_POWER or FFT_DBFS
        // Uncomment one these to try other window functions
        //  myFFT.windowFunction(NULL);
        //  myFFT.windowFunction(AudioWindowBartlett4096);
        //  myFFT.windowFunction(AudioWindowFlattop4096);
        myFFT_4096.windowFunction(AudioWindowHanning4096);
        // myFFT_4096.windowFunction(AudioWindowBlackmanHarris4096);
        myFFT_4096.setNAverage(NAvg); // experiment with this value.  Too much causes a large time penalty
    #endif
    #ifdef FFT_2048
        myFFT_2048.setXAxis(fft_axis);    // Set the FFT bin order to our needs
        myFFT_2048.setOutputType(FFT_DBFS); // FFT_RMS or FFT_POWER or FFT_DBFS
        // Uncomment one these to try other window functions
        //  myFFT.windowFunction(NULL);
        //  myFFT.windowFunction(AudioWindowBartlett2048);
        //  myFFT.windowFunction(AudioWindowFlattop2048);
        myFFT_2048.windowFunction(AudioWindowHanning2048);
        //  myFFT_4096.windowFunction(AudioWindowBlackmanHarris2048);
        myFFT_2048.setNAverage(NAvg); // experiment with this value.  Too much causes a large time penalty
    #endif
    #ifdef FFT_1024
        myFFT_1024.setXAxis(fft_axis);    // Set the FFT bin order to our needs
        myFFT_1024.setOutputType(FFT_DBFS); // FFT_RMS or FFT_POWER or FFT_DBFS
        // Uncomment one these to try other window functions
        //  myFFT.windowFunction(NULL);
        //  myFFT.windowFunction(AudioWindowBartlett1024);
        //  myFFT.windowFunction(AudioWindowFlattop1024);
        myFFT_1024.windowFunction(AudioWindowHanning1024);
        //  myFFT_4096.windowFunction(AudioWindowBlackmanHarris1024);
        myFFT_1024.setNAverage(NAvg); // experiment with this value.  Too much causes a large time penalty
    #endif

    tft.fillRect(ptr->spect_x, ptr->spect_y, ptr->spect_width, ptr->spect_height, BLACK);  // x start, y start, width, height, array of colors w x h
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
    tft.drawRect(ptr->l_graph_edge,    ptr->wf_top_line,    ptr->wf_sp_width+2,   ptr->wf_height,    LIGHTGREY);  // x start, y start, width, height, array of colors w x h
    tft.fillRect(ptr->l_graph_edge+1,  ptr->wf_top_line+1,  ptr->wf_sp_width,     ptr->wf_height-2,  BLACK);
    // Set the scroll region for the watefall.  We only need to write 1 new top line and block shift the rest down 1.
    //tft.setScrollWindow(l_graph_edge, r_graph_edge, wf_top_line+1, wf_bottom_line-1);  //Specifies scrolling activity area   XL, XR, Ytop, Ybottom
  
    // Set up the Spectrum box area
    tft.drawRect(ptr->l_graph_edge,      ptr->sp_top_line,      ptr->wf_sp_width+2,   ptr->sp_height,      LIGHTGREY);  // x start, y start, width, height, array of colors w x h    
    tft.fillRect(ptr->l_graph_edge+1,    ptr->sp_top_line+1,    ptr->wf_sp_width,     ptr->sp_height-2,    BLACK);
    
    // Draw Tick marks and Span Labels
    tft.drawLine(ptr->l_graph_edge+1,                        ptr->sp_tick_row,      ptr->l_graph_edge+1,                        ptr->sp_tick_row-ptr->tick_height, LIGHTGREY);
    tft.drawLine(ptr->l_graph_edge+(ptr->wf_sp_width/4),     ptr->sp_tick_row,      ptr->l_graph_edge+(ptr->wf_sp_width/4),     ptr->sp_tick_row-ptr->tick_height, LIGHTGREY);
    tft.drawLine(ptr->l_graph_edge+1+(ptr->wf_sp_width/2)+1, ptr->sp_tick_row,      ptr->l_graph_edge+1+(ptr->wf_sp_width/2)+1, ptr->sp_tick_row-ptr->tick_height, LIGHTGREY);
    tft.drawLine(ptr->l_graph_edge+((ptr->wf_sp_width/4)*3), ptr->sp_tick_row,      ptr->l_graph_edge+((ptr->wf_sp_width/4)*3), ptr->sp_tick_row-ptr->tick_height, LIGHTGREY);
    tft.drawLine(ptr->l_graph_edge+ptr->wf_sp_width-1,       ptr->sp_tick_row,      ptr->l_graph_edge+ptr->wf_sp_width-1,       ptr->sp_tick_row-ptr->tick_height, LIGHTGREY);
    
    // draw the spectrum box center grid line for tuning help.
    //tft.drawFastVLine(ptr->l_graph_edge+1+ptr->wf_sp_width/2+1, ptr->sp_top_line+1, ptr->sp_height, myLT_GREY);
    
    // Draw the ticks on the bottom of Waterfall window also
    tft.drawLine(ptr->l_graph_edge+1,                         ptr->wf_bottom_line,   ptr->l_graph_edge+1,                        ptr->wf_tick_row, LIGHTGREY);
    tft.drawLine(ptr->l_graph_edge+(ptr->wf_sp_width/4),      ptr->wf_bottom_line,   ptr->l_graph_edge+(ptr->wf_sp_width/4),     ptr->wf_tick_row, LIGHTGREY);
    tft.drawLine(ptr->l_graph_edge+1+(ptr->wf_sp_width/2)+1,  ptr->wf_bottom_line,   ptr->l_graph_edge+1+(ptr->wf_sp_width/2)+1, ptr->wf_tick_row, LIGHTGREY);
    tft.drawLine(ptr->l_graph_edge+(ptr->wf_sp_width/4)*3,    ptr->wf_bottom_line,   ptr->l_graph_edge+(ptr->wf_sp_width/4)*3,   ptr->wf_tick_row, LIGHTGREY);
    tft.drawLine(ptr->l_graph_edge+ptr->wf_sp_width-1,        ptr->wf_bottom_line,   ptr->l_graph_edge+ptr->wf_sp_width-1,       ptr->wf_tick_row, LIGHTGREY);    
}

//
//--------------------------------------------------  Spectrum init ------------------------------------------------------------------------
//
//   Must be called before any text is written tothe screen. If not that text will be corrupted.
//
FLASHMEM void initSpectrum(int16_t preset) // preset is the spectrum frame definition index value chosen by external functions.
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
}

//
//--------------------------------------------------  Spectrum parameter generator tool ------------------------------------------------------------------------
//
//   Call this tool to generate a set of parameters to setup a spectrum window and store then in the specified record set 
//
//   Input: int parm_set: range is 0-PRESETS. It is the index to teh array of structures storing the window's data set
//   
//  Output: struct Spectrum_Parms Sp_Parms_Custom[PRESETS];   
//          Sp_Parms_Custom is an in-memory stucture with several records, each holding a set of outputs from this generator that
//          define all the window's coordinates and settings.
//     
//   Usage: 1.  Set your new layout variables to locate and size the spectrum window. 
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
void Spectrum_Parm_Generator(int16_t parm_set, int16_t preset, int16_t fft_binc)
{
//  User Variables used:
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
    //if (parm_set >= PRESETS) 
    //    parm_set = PRESETS-1;   // Cycle back to 0
    
 // The user provided Custom layout data and newly calculated values are filled into a full new layout record
    struct New_Spectrum_Layout *c_ptr = &Custom_Layout[parm_set]; // user provided data
    struct Spectrum_Parms *ptr = &Sp_Parms_Custom[preset];  // empty struct to store user imported values along with calculated values
 
    //int wf_sp_width;  // This is the actual graph space width to be used.  Max is fft_bins, can be smaller.
    ptr->border_space_min = 0;  // Left and right side space. Graph space would be this this value*2 less.
    ptr->border_space = ptr->border_space_min;
    if (c_ptr->spectrum_width > tft.width())
        c_ptr->spectrum_width = tft.width();
    if (c_ptr->spectrum_width > (fft_binc*2) + (ptr->border_space_min*2) - 1)
    {  
        // space is wider than max graph size fft_bins to pad with border space and center graph area
        ptr->border_space = (c_ptr->spectrum_width - (fft_binc*2))/2;   // padding for each side
        ptr->wf_sp_width  = fft_binc*2;
    }
    else  // make smaller than FFT_bins
    {
        ptr->border_space = ptr->border_space_min;
        ptr->wf_sp_width = c_ptr->spectrum_width - (ptr->border_space*2) -1;
    }
    ptr->l_graph_edge     = c_ptr->spectrum_x + ptr->border_space;
    ptr->r_graph_edge     = ptr->l_graph_edge + ptr->wf_sp_width;      
    ptr->c_graph          = ptr->l_graph_edge + ptr->wf_sp_width/2 -1;  // center of graph window areas

    // Work our way down vertically.  Add padding where needed
    ptr->sp_txt_row_height   = 14;   // space for span freequency marker labels
    ptr->tick_height         = 8;    // the frequency markers top and bottom of display regions.  So total space is value*2    
    ptr->sp_txt_row          = c_ptr->spectrum_y +4;     // spectrum text line below the top of space we have 
    ptr->sp_tick_row         = ptr->sp_txt_row + ptr->sp_txt_row_height + ptr->tick_height ;           // bottom of tick mark rectangle space that is 1 space below text
    ptr->sp_top_line         = ptr->sp_tick_row;                       // spectrum top of graphing active area or window    
    ptr->wf_tick_row         = c_ptr->spectrum_y + c_ptr->spectrum_height -1;           // bottom of tick mark rectangle space that is 1 space below text
    ptr->wf_bottom_line      = ptr->wf_tick_row - ptr->tick_height;
    ptr->sp_height           = (ptr->wf_bottom_line - ptr->sp_top_line) * c_ptr->spectrum_center/100; //Account for span label and tic mark space outside of spectrum and waterfall
    ptr->wf_height           = (ptr->wf_bottom_line - ptr->sp_top_line) - ptr->sp_height;     // use what is left        
    ptr->sp_bottom_line      = ptr->sp_top_line + ptr->sp_height;                // bottom of graphing area window
    ptr->wf_top_line         = ptr->sp_bottom_line; 
    // record other master location parms to use for Preset recall othwerwise things get confused.  When reading these must be read first.
    ptr->spect_x             = c_ptr->spectrum_x;
    ptr->spect_y             = c_ptr->spectrum_y;
    ptr->spect_width         = c_ptr->spectrum_width;
    ptr->spect_height        = c_ptr->spectrum_height;
    ptr->spect_center        = c_ptr->spectrum_center;
    ptr->spect_span          = c_ptr->spectrum_span;
    ptr->spect_wf_style      = c_ptr->spectrum_wf_style;
    ptr->spect_wf_colortemp  = c_ptr->spectrum_wf_colortemp;
    ptr->spect_wf_scale      = c_ptr->spectrum_wf_scale;       
    ptr->spect_LPFcoeff      = c_ptr->spectrum_LPFcoeff;
    ptr->spect_dot_bar_mode  = c_ptr->spectrum_dot_bar_mode;
    ptr->spect_sp_scale      = c_ptr->spectrum_sp_scale;
    ptr->spect_floor         = c_ptr->spectrum_floor;
    ptr->spect_wf_rate       = c_ptr->spectrum_wf_rate;
  
// print out results to the serial terminal for manual copy into the default table.  This is 1 set of data only, for each run.  
// Change the globals and run again for a new set

   DPRINTLN(F("Start of Spectrum Parameter Generator List."));
   DPRINTLN(F("This is a complete parameter record for the current window."));
   DPRINTLN(F("Cut and paste the data in the braces to modify the predefined records."));
   DPRINT(F("{"));
   DPRINT(ptr->wf_sp_width);DPRINT(",");
   DPRINT(ptr->border_space_min);DPRINT(",");
   DPRINT(ptr->border_space);DPRINT(",");
   DPRINT(ptr->l_graph_edge);DPRINT(",");
   DPRINT(ptr->r_graph_edge);DPRINT(",");
   DPRINT(ptr->c_graph);DPRINT(",");
   DPRINT(ptr->sp_txt_row_height);DPRINT(",");
   DPRINT(ptr->tick_height);DPRINT(",");
   DPRINT(ptr->sp_txt_row);DPRINT(",");
   DPRINT(ptr->sp_tick_row);DPRINT(",");       
   DPRINT(ptr->sp_top_line);DPRINT(",");
   DPRINT(ptr->wf_tick_row);DPRINT(",");
   DPRINT(ptr->wf_bottom_line);DPRINT(",");
   DPRINT(ptr->sp_height);DPRINT(",");
   DPRINT(ptr->wf_height);DPRINT(",");
   DPRINT(ptr->sp_bottom_line);DPRINT(",");
   DPRINT(ptr->wf_top_line);DPRINT(",");
   DPRINT(ptr->spect_x);DPRINT(",");
   DPRINT(ptr->spect_y);DPRINT(",");
   DPRINT(ptr->spect_width);DPRINT(",");
   DPRINT(ptr->spect_height);DPRINT(",");
   DPRINT(ptr->spect_center);DPRINT(",");
   DPRINT(ptr->spect_span,1); DPRINT(",");
   DPRINT(ptr->spect_wf_style); DPRINT(",");
   DPRINT(ptr->spect_wf_colortemp); DPRINT(",");
   DPRINT(ptr->spect_wf_scale,1); DPRINT(",");
   DPRINT(ptr->spect_LPFcoeff,1); DPRINT(",");
   DPRINT(ptr->spect_dot_bar_mode); DPRINT(",");
   DPRINT(ptr->spect_sp_scale); DPRINT(",");
   DPRINT(ptr->spect_floor); DPRINT(",");
   DPRINT(ptr->spect_wf_rate); DPRINT("}");

   DPRINTLN(F("\nEnd of Spectrum Parameter Generator List"));
   DPRINT(F("Current Preset="));
   DPRINT(preset);  // display extrnal specified frame drawing definition 
   DPRINT(F("  Selected Preset="));
   DPRINT(parm_set);
   DPRINT(F("  Current Waterfall Style="));
   DPRINT(c_ptr->spectrum_wf_style);
   DPRINT(F("  Current Color Temp="));
   DPRINTLN(c_ptr->spectrum_wf_colortemp);
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
  //
int16_t _find_FFT_Max(uint16_t bin_min, uint16_t bin_max, uint16_t fft_sz)    // args for min and max bins to look at based on display width.
{
    float specMax = -200.0f;
    uint16_t iiMax = 0;
    int16_t f_peak = 0.0f;
    uint16_t bin_center = (bin_max-bin_min)/2 + bin_min;
//Serial.println("FFT_MAX CALL");
    //sp_FFT.setOutputType(FFT_POWER);   // change to power, return it to FFT_DBFS at end

    // Get pointer to data array of powers, float output[xxx];
    // setAxis(fft_axis)  // Called in drawSpectrumFrame() to set the bin order to lowest frequency at 0 bin and highest at bin 1023
    float *pPwr=NULL;
    
    // only 1 of these (fft_sz derived from global fft_size) is chosen to be used to display
    #ifdef FFT_4096
        #ifndef BETATEST
            if (fft_sz == 4096) pPwr = myFFT_4096.getData();
        #else
            pPwr = fftOutput;
        #endif
    #endif
    #ifdef FFT_2048
        if (fft_sz == 2048) pPwr = myFFT_2048.getData();
    #endif
    #ifdef FFT_1024
        if (fft_sz == 1024) pPwr = myFFT_1024.getData();   
    #endif
    // Find biggest bin
    for(int ii=bin_min; ii<bin_max; ii++)  
    {        
        if (ii == bin_center -1)        
            ii += 3;   // advance ii to skip the center few bins around our center frequency.

//Serial.print("ii=");DPRINT(ii);DPRINT("  binval=");DPRINTLN(*(pPwr + ii));  
        if (*(pPwr + ii) > specMax && *(pPwr + ii) < -1.0)   // filter out periodic blocks of 0 values 
        { // Find highest peak of range
          specMax = *(pPwr + ii);
          iiMax = ii;         
        }
    }
//Serial.print("iiMax=");DPRINT(iiMax);DPRINT(" specMax=");DPRINTLN(specMax);   
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
        //fftMaxPower = myFFT.read(fftMaxPower)-20; // set global fftMaxPower = Power of the strongest signal if possible
                                                            // -20 is a cal factor experimentally determined
        #ifdef FFT_4096
            if (fft_sz == 4096) fftMaxPower = myFFT_4096.read(fftMaxPower)-20; 
        #endif
        #ifdef FFT_2048
            if (fft_sz == 2048) fftMaxPower = myFFT_2048.read(fftMaxPower)-20; 
        #endif
        #ifdef FFT_1024 
            if (fft_sz == 1024) fftMaxPower = myFFT_1024.read(fftMaxPower)-20;    
        #endif
    }
//DPRINT("iiMax=");DPRINT(iiMax);DPRINT(" fftMaxPower=");DPRINT(fftMaxPower);DPRINT(" fpeak=");DPRINTLN(f_peak);
    return f_peak;    // return -200 unless there is a good value to send out
}

// Duplicate of the function in Display.h but included here to make the spectrum module self contained. Minor changes included
//char* Spectrum_RA887x::_formatFreq(uint32_t Freq)
char* _formatFreq(uint32_t Freq)
{
	static char Freq_str[16];

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
int16_t _colorMap(int16_t val, int16_t color_temp) 
{
    float red;
    float green;
    float blue;
    float temp = val / 65536.0 * (color_temp);
  //DPRINT("temp=");DPRINTLN(temp);
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
    //DPRINT("  CM=");DPRINT(tft.Color565(red * 256, green * 256, blue * 256));Serial.print("  val=");DPRINTLN(val);Color24To565
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
int16_t _waterfall_color_update(float sample, int16_t waterfall_low) 
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
    //DPRINT("Final color = ");DPRINTLN(pval,HEX);
    return pval;
}
