#include <RA8875.h>
#include <Audio.h> 
extern AudioAnalyzePeak_F32 S_Peak;  
extern RA8875 tft;
 

////////////////////////// this is the S meter code/////totall uncalibrated use at your own risk
void Peak()
{
   float s_sample;  // Raw signal strength (max per 1ms)
   float uv, dbuv, s;// microvolts, db-microvolts, s-units
   char string[80];   // print format stuff
 
  	if (S_Peak.available())
    {
      	s_sample =S_Peak.read();
     
		uv= s_sample * 1000;
		dbuv = 20.0*log10(uv);
		/*
		tft.fillRect(700, 38, 99,25,RA8875_BLACK);
		tft.setFont(Arial_14);
		tft.setCursor(720, 42);
		tft.setTextColor(RA8875_GREEN);
		tft.print(dbuv);
		tft.fillRect(130, 47, 550,10, RA8875_BLACK);
		tft.fillRect(130, 47,abs(dbuv*4),10,RA8875_GREEN );
        */
		s = (dbuv-3)/6.0;
		
		if (s <0.0) 
			s=0.0;

		if (s>9.0)
			s = 9.0;
		else 
			dbuv = 0;
			
		tft.setTextColor(RA8875_BLUE);
		tft.setFont(Arial_14);

		/*		
		// bar meter
		tft.fillRect(72, 38, 57,25,RA8875_BLACK);
		tft.setCursor(1, 42);		
		if (dbuv == 0)
			sprintf(string,"S-Meter:%1.0f",s);
		else 
			sprintf(string,"S-Meter:9+%02.0f",dbuv);
		tft.print(string);
		*/

		// rounded meter
		if (dbuv == 0) 
			sprintf(string,"S-%1.0f",s);
		else 
			sprintf(string,"      S-9+%02.0f",dbuv);
		
		tft.ringMeter(s, 0, 9, 686, 40, 50, string, 3, 1, 90, 8);		
	}
}
