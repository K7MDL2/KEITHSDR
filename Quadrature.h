#include <RA8875.h>
#include <Audio.h> 

extern RA8875 tft;
extern AudioAnalyzePeak_F32         Q_Peak;          
extern AudioAnalyzePeak_F32         I_Peak;         

void Quad_Check()
{
 float Q_sample;  // Raw signal strength (max per 1ms) 
 float I_sample;  // Raw signal strength (max per 1ms)
 float I_uv,Q_uv;// microvolts, db-microvolts, s-units

 if (I_Peak.available())
 {
   I_sample =I_Peak.read();
   I_uv= I_sample * 10000;  
   if(I_uv>490)
   {
    I_uv=490;
   }
   tft.fillRect(210,450, 490,15,RA8875_BLACK);
   tft.fillRect(210,450,abs(I_uv/2),15,RA8875_GREEN );
   tft.setFont(Arial_14);
   tft.setTextColor(RA8875_GREEN);
   tft.setCursor(120,450);
   tft.print("I-Channel");
 }
 if (Q_Peak.available())
 { 
   Q_sample =Q_Peak.read();
   Q_uv= Q_sample * 10000;
    if(Q_uv>490)
   {
    Q_uv=490;
   }
   tft.fillRect(210, 420,490,15,RA8875_BLACK);
   tft.fillRect(210, 420,abs(Q_uv/2  ),15,RA8875_GREEN);
   tft.setFont(Arial_14);
   tft.setCursor(113,420);
   tft.setTextColor(RA8875_GREEN);
   tft.print("Q-Channel");
 }

}
