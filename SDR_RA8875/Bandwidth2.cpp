//
//   Bandwidth2.cpp
//
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"
//#include "Bandwidth2.h"
//#include "Hilbert.h"            // filter coefficients

//extern AudioFilterFIR_F32               RX_Hilbert_Plus_45;
//extern AudioFilterFIR_F32               RX_Hilbert_Minus_45;
//extern AudioFilterBiquad_F32    CW_Filter;
extern struct  Band_Memory              bandmem[];
extern uint8_t                          curr_band;   // global tracks our current band setting. 
extern const struct Filter_Settings     filter[];
extern void                             SetFilter(void);
extern struct User_Settings             user_settings[];
extern uint8_t                          user_Profile;
extern int16_t                          filterCenter;
extern int16_t                          filterBandwidth;
extern AudioEffectGain_F32              Amp1_L;  // Some well placed gain stages
extern AudioEffectGain_F32              Amp1_R;  // Some well placed gain stages



////////////////////////////////////////////////////////////////////////////////////
COLD void selectBandwidth(uint8_t bndx)
{
    Amp1_L.setGain_dB(AUDIOBOOST);    // Adjustable fixed output boost in dB.
    Amp1_R.setGain_dB(AUDIOBOOST);
    // For convolutional filter method, just set a single fixed Hibert filter width. Rest is taken care of after the summer
    
    if(bndx==0)
    {
        //bandwidth="Bw 250 Hz";
        //CW_Filter.end();
        //CW_Filter.setBandpass(0,250.0f,9.0f);   
        //CW_Filter.begin();           
        filterCenter = user_settings[user_Profile].pitch;  // Use pitch since this is a CW filter
        filterBandwidth = 250;
        AudioNoInterrupts();
        Amp1_L.setGain_dB(AUDIOBOOST+8.0f);    // Adjustable fixed output boost in dB.
        Amp1_R.setGain_dB(AUDIOBOOST+8.0f);
        AudioInterrupts();
        SetFilter();               
    }

    if(bndx==1)
    {
        //bandwidth="Bw 500 Hz";
        //CW_Filter.end();
        //CW_Filter.setBandpass(0,500.0f,9.0f);
        //CW_Filter.begin();
        filterCenter = user_settings[user_Profile].pitch;  // Use pitch since this is a CW filter
        filterBandwidth = 500;
        AudioNoInterrupts();
        Amp1_L.setGain_dB(AUDIOBOOST+6.0f);    // Adjustable fixed output boost in dB.
        Amp1_R.setGain_dB(AUDIOBOOST+6.0f);
        AudioInterrupts();
        SetFilter();
    }
    
    if(bndx==2)
    {
        //bandwidth="Bw 700 Hz";
        //CW_Filter.end();
        //CW_Filter.setBandpass(0,700.0f,9.0f);
        //CW_Filter.begin();
        filterCenter = user_settings[user_Profile].pitch;  // Use pitch since this is a CW filter
        filterBandwidth = 700;
        AudioNoInterrupts();
        Amp1_L.setGain_dB(AUDIOBOOST+5.0f);    // Adjustable fixed output boost in dB.
        Amp1_R.setGain_dB(AUDIOBOOST+5.0f);
        AudioInterrupts();
        SetFilter();
    }

    if(bndx==3)
    {
        //bandwidth="Bw 1.0 kHz";
        //CW_Filter.end();
        //CW_Filter.setBandpass(0,1000.0f,9.0f);
        //CW_Filter.begin();
        filterCenter = user_settings[user_Profile].pitch;  // Use pitch since this is a CW filter
        filterBandwidth=1000;
        AudioNoInterrupts();
        Amp1_L.setGain_dB(AUDIOBOOST+3.0f);    // Adjustable fixed output boost in dB.
        Amp1_R.setGain_dB(AUDIOBOOST+3.0f);
        AudioInterrupts();
        SetFilter();
    }
    
    // Above are CW mode filter widths.  Below are wider filters for voice and data modes
    
    if(bndx==4)
    {
        //bandwidth="Bw 1.8 kHz";  
        filterCenter = 1900/2;
        filterBandwidth = 1800;
        SetFilter();
    }

    if(bndx==5)
    {
        //bandwidth="Bw 2.3kHz";
        filterCenter = 2500/2;
        filterBandwidth = 2300;
        SetFilter();
    }

    if(bndx==6)
    {
        //bandwidth="Bw 2.8 kHz";
        filterCenter = 3000/2;
        filterBandwidth = 2800;
        SetFilter();
    }
    
    if(bndx==7)
    {
        //bandwidth="Bw 3.2 kHz";
        filterCenter = 3400/2;
        filterBandwidth = 3200;
        SetFilter();           
    }
  
    if(bndx==8) 
    {
        //bandwidth="4.0 kHz";
        filterCenter = 4200/2;
        filterBandwidth = 4000;
        SetFilter();
    } 

    if(bndx==9) 
    {
        //bandwidth="6.0 kHz";
        filterCenter = 6000/2;
        filterBandwidth = 6000;
        SetFilter();
    } 
    bandmem[curr_band].filter = bndx; // Set new filter into memory
    //DPRINTF("Filter Set to "); DPRINTLN(filter[bandmem[curr_band].filter].Filter_name);
    //displayFilter();
}