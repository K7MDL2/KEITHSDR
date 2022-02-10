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
extern int                              filterCenter;
extern int                              filterBandwidth;

////////////////////////////////////////////////////////////////////////////////////
COLD void selectBandwidth(uint8_t bndx)
{
    // For convolutional filter method, just set a single fixed Hibert filter width. Rest is taken care of after the summer
    //RX_Hilbert_Plus_45.end();
    //RX_Hilbert_Minus_45.end();

    //RX_Hilbert_Plus_45.begin(Hilbert_Plus45_40K,151);
    //RX_Hilbert_Minus_45.begin(Hilbert_Minus45_40K,151);
    
    if(bndx==0)
    {
        //bandwidth="Bw 250 Hz";
    //    RX_Hilbert_Plus_45.begin(Hilbert_Plus45_500,151);
    //    RX_Hilbert_Minus_45.begin(Hilbert_Minus45_500,151);
        //CW_Filter.end();
        //CW_Filter.setBandpass(0,250.0f,9.0f);   
        //CW_Filter.begin();           
        filterCenter = (float) user_settings[user_Profile].pitch;  // Use pitch since this is a CW filter
        filterBandwidth = 250;
        SetFilter();               
    }

    if(bndx==1)
    {
        //bandwidth="Bw 500 Hz";
    //    RX_Hilbert_Plus_45.begin(Hilbert_Plus45_500,151);
    //    RX_Hilbert_Minus_45.begin(Hilbert_Minus45_500,151);
        //CW_Filter.end();
        //CW_Filter.setBandpass(0,500.0f,9.0f);
        //CW_Filter.begin();
        filterCenter = (float) user_settings[user_Profile].pitch;  // Use pitch since this is a CW filter
        filterBandwidth = 500;
        SetFilter();
    }
    
    if(bndx==2)
    {
        //bandwidth="Bw 700 Hz";
    //    RX_Hilbert_Plus_45.begin(Hilbert_Plus45_700,151);
    //    RX_Hilbert_Minus_45.begin(Hilbert_Minus45_700,151);
        //CW_Filter.end();
        //CW_Filter.setBandpass(0,700.0f,9.0f);
        //CW_Filter.begin();
        filterCenter = (float) user_settings[user_Profile].pitch;  // Use pitch since this is a CW filter
        filterBandwidth = 700;
        SetFilter();
    }

    if(bndx==3)
    {
        //bandwidth="Bw 1.0 kHz";
    //    RX_Hilbert_Plus_45.begin(Hilbert_Plus45_1K,151);
    //    RX_Hilbert_Minus_45.begin(Hilbert_Minus45_1K,151);
        //CW_Filter.end();
        //CW_Filter.setBandpass(0,1000.0f,9.0f);
        //CW_Filter.begin();
        filterCenter = (float) user_settings[user_Profile].pitch;  // Use pitch since this is a CW filter
        filterBandwidth=1000;
        SetFilter();
    }

    if(bndx==4)
    {
        //bandwidth="Bw 1.8 kHz";  
    //    RX_Hilbert_Plus_45.begin(Hilbert_Plus45_18K,151);
    //    RX_Hilbert_Minus_45.begin(Hilbert_Minus45_18K,151);
        filterCenter = 1850/2;
        filterBandwidth = 1800;
        SetFilter();
    }

    if(bndx==5)
    {
        //bandwidth="Bw 2.3kHz";
    //    RX_Hilbert_Plus_45.begin(Hilbert_Plus45_23K,151);
    //    RX_Hilbert_Minus_45.begin(Hilbert_Minus45_23K,151);
        filterCenter = 2350/2;
        filterBandwidth = 2300;
        SetFilter();
    }

    if(bndx==6)
    {
        //bandwidth="Bw 2.8 kHz";
    //    RX_Hilbert_Plus_45.begin(Hilbert_Plus45_28K,151);
    //    RX_Hilbert_Minus_45.begin(Hilbert_Minus45_28K,151);
        filterCenter = 2850/2;
        filterBandwidth = 2800;
        SetFilter();
    }
    
    if(bndx==7)
    {
        //bandwidth="Bw 3.2 kHz";
    //    RX_Hilbert_Plus_45.begin(Hilbert_Plus45_32K,151);
    //    RX_Hilbert_Minus_45.begin(Hilbert_Minus45_32K,151);
        filterCenter = 3250/2;
        filterBandwidth = 3200;
        SetFilter();           
    }
  
    if(bndx==8) 
    {
        //bandwidth="4.0 kHz";
    //    RX_Hilbert_Plus_45.begin(Hilbert_Plus45_40K,151);
    //    RX_Hilbert_Minus_45.begin(Hilbert_Minus45_40K,151);
        filterCenter = 4050/2;
        filterBandwidth = 4000;
        SetFilter();
    } 

    bandmem[curr_band].filter = bndx;
    Serial.print("Filter Set to ");
    Serial.println(filter[bandmem[curr_band].filter].Filter_name);
    
    //displayFilter();
}