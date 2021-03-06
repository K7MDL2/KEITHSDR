#include <Audio.h> 
//extern AudioFilterBiquad       BandPass; 
extern AudioFilterFIR_F32      Hilbert1;
extern AudioFilterFIR_F32      Hilbert2;
extern AudioFilterBiquad_F32   CW_Filter;
extern int bndx;
extern  String bandwidth;

////////////////////////////////////////////////////////////////////////////////////
void selectBandwidth(int ndx)
{
 
  if(bndx==0)
  {
        Serial.println("Lets set the bandwidth to 250 Hz");
        bandwidth="Bw 250 Hz";
        Hilbert1.begin(Hilbert_Plus45_500,151);
        Hilbert2.begin(Hilbert_Minus45_500,151);
        CW_Filter.setBandpass(0,250.0f,9.0f);                     
  }
  if(bndx==1)
  {
       Serial.println("Lets set the bandwidth to 500 Hz");
       bandwidth="Bw 500 Hz";
       Hilbert1.begin(Hilbert_Plus45_500,151);
       Hilbert2.begin(Hilbert_Minus45_500,151);
       CW_Filter.setBandpass(0,500.0f,9.0f);
  }
  
  if(bndx==2)
  {
      Serial.println("Lets set the bandwidth to 700 Hz");
      bandwidth="Bw 700 Hz";
      Hilbert1.begin(Hilbert_Plus45_700,151);
      Hilbert2.begin(Hilbert_Minus45_700,151);
      CW_Filter.setBandpass(0,700.0f,9.0f);
  }

  if(bndx==3)
  {
      Serial.println("Lets set the bandwidth to 1.0kHz");
      bandwidth="Bw 1.0 kHz";
      Hilbert1.begin(Hilbert_Plus45_1K,151);
      Hilbert2.begin(Hilbert_Minus45_1K,151);
      CW_Filter.setBandpass(0,1000.0f,9.0f);
  }

  if(bndx==4)
  {
      Serial.println("Lets set the bandwidth to 1.8kHz");
      bandwidth="Bw 1.8 kHz";  
      Hilbert1.begin(Hilbert_Plus45_18K,151);
      Hilbert2.begin(Hilbert_Minus45_18K,151);
   }

  if(bndx==5)
  {
      Serial.println("Lets set the bandwidth to 2.3kkHz");
      bandwidth="Bw 2.3kHz";
      Hilbert1.begin(Hilbert_Plus45_23K,151);
      Hilbert2.begin(Hilbert_Minus45_23K,151);
  }

  if(bndx==6)
  {
      Serial.println("Lets set the bandwidth to 2.8 kHz");
      bandwidth="Bw 2.8 kHz";
      Hilbert1.begin(Hilbert_Plus45_28K,151);
      Hilbert2.begin(Hilbert_Minus45_28K,151);
  }
 
  if(bndx==7)
  {
      Serial.println("Lets set the bandwidth to 3.2 kHz");
      bandwidth="Bw 3.2 kHz";
      Hilbert1.begin(Hilbert_Plus45_32K,151);
      Hilbert2.begin(Hilbert_Minus45_32K,151);           
  }
  
  if(bndx==8) 
  {
      Serial.println("Lets set the bandwidth to 4.0 kHz");
      bandwidth="4.0 kHz";
      Hilbert1.begin(Hilbert_Plus45_40K,151);
      Hilbert2.begin(Hilbert_Minus45_40K,151);
  } 

  displayBandwidth();
}
