#include <RA8875.h>
#include <Audio.h> 
extern AudioAnalyzePeak_F32 CW_Peak;  
extern AudioAnalyzeRMS_F32 CW_RMS; 
extern RA8875 tft;
 

////////////////////////// this is the S meter code/////totall uncalibrated use at your own risk
void Code_Peak()
{
   float s_sample;  // Raw signal strength (max per 1ms)
   float uv, dbuv;// microvolts, db-microvolts, s-units
 
if (CW_Peak.available())
    {
      s_sample =CW_Peak.read();
     
       uv= s_sample * 100;
       dbuv = 20.0*log10(uv);
       if(dbuv>475)
       {
        dbuv=475;
       }
       
       tft.fillRect(225,100,475,10, RA8875_BLACK);
       tft.fillRect(225,100,abs(dbuv*10),10,RA8875_GREEN);
       tft.setFont(Arial_14);
       tft.setCursor(125,100);
       tft.setTextColor(RA8875_GREEN);
       tft.print("CW Peak");
     }
}

void Code_RMS()
{
   float s_sample;  // Raw signal strength (max per 1ms)
   float uv, dbuv;// microvolts, db-microvolts, s-units
 
if (CW_RMS.available())
    {
      s_sample =CW_RMS.read();
     
       uv= s_sample * 100;
       dbuv = 20.0*log10(uv);
       tft.fillRect(225,100,450,10, RA8875_BLACK);
       tft.fillRect(225,100,abs(dbuv*10),10,RA8875_GREEN );
       tft.setFont(Arial_14);
       tft.setCursor(125,100);
       tft.setTextColor(RA8875_GREEN);
       tft.print("CW RMS");
     }
}
