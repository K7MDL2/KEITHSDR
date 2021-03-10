//
//    Bandwidth2.h
//

//#include <Audio.h> 

extern AudioFilterFIR_F32      Hilbert1;
extern AudioFilterFIR_F32      Hilbert2;
extern AudioFilterBiquad_F32   CW_Filter;
extern struct Bandwidth_Settings bw[];

////////////////////////////////////////////////////////////////////////////////////
void selectBandwidth(int bndx)
{
    if (bandmem[curr_band].mode == CW)  // CW modes
    {
        if (bndx > BW4_0)    // go to bottom band   
            bndx = BW0_25;    // 0 is not used
        if (bndx < BW0_25)    // go to top most band  -  
            bndx = BW4_0;    // 0 is not used so do not have to adjsut with a -1 here
    }
    else  // Non-CW modes
    {
        if (bndx > BW4_0)    // go to bottom band   
            bndx = BW1_8;    // 0 is not used
        if (bndx < BW1_8)    // go to top most band  -  
            bndx = BW4_0;    // 0 is not used so do not have to adjsut with a -1 here
    }

    if(bndx==0)
    {
        //bandwidth="Bw 250 Hz";
        Hilbert1.begin(Hilbert_Plus45_500,151);
        Hilbert2.begin(Hilbert_Minus45_500,151);
        CW_Filter.end();
        CW_Filter.setBandpass(0,250.0f,9.0f);   
        CW_Filter.begin();                  
    }

    if(bndx==1)
    {
        //bandwidth="Bw 500 Hz";
        Hilbert1.begin(Hilbert_Plus45_500,151);
        Hilbert2.begin(Hilbert_Minus45_500,151);
        CW_Filter.end();
        CW_Filter.setBandpass(0,500.0f,9.0f);
        CW_Filter.begin();
    }
    
    if(bndx==2)
    {
        //bandwidth="Bw 700 Hz";
        Hilbert1.begin(Hilbert_Plus45_700,151);
        Hilbert2.begin(Hilbert_Minus45_700,151);
        CW_Filter.end();
        CW_Filter.setBandpass(0,700.0f,9.0f);
        CW_Filter.begin();
    }

    if(bndx==3)
    {
        //bandwidth="Bw 1.0 kHz";
        Hilbert1.begin(Hilbert_Plus45_1K,151);
        Hilbert2.begin(Hilbert_Minus45_1K,151);
        CW_Filter.end();
        CW_Filter.setBandpass(0,1000.0f,9.0f);
        CW_Filter.begin();
    }

    if(bndx==4)
    {
        //bandwidth="Bw 1.8 kHz";  
        Hilbert1.begin(Hilbert_Plus45_18K,151);
        Hilbert2.begin(Hilbert_Minus45_18K,151);
    }

    if(bndx==5)
    {
        //bandwidth="Bw 2.3kHz";
        Hilbert1.begin(Hilbert_Plus45_23K,151);
        Hilbert2.begin(Hilbert_Minus45_23K,151);
    }

    if(bndx==6)
    {
        //bandwidth="Bw 2.8 kHz";
        Hilbert1.begin(Hilbert_Plus45_28K,151);
        Hilbert2.begin(Hilbert_Minus45_28K,151);
    }
    
    if(bndx==7)
    {
        //bandwidth="Bw 3.2 kHz";
        Hilbert1.begin(Hilbert_Plus45_32K,151);
        Hilbert2.begin(Hilbert_Minus45_32K,151);           
    }
  
    if(bndx==8) 
    {
        //bandwidth="4.0 kHz";
        Hilbert1.begin(Hilbert_Plus45_40K,151);
        Hilbert2.begin(Hilbert_Minus45_40K,151);
    } 

    bandmem[curr_band].bandwidth = bndx;
    Serial.print("BW Set to ");
    Serial.println(bw[bandmem[curr_band].bandwidth].bw_name);
    
    displayBandwidth();
}
