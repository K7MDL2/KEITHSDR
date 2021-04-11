//
//  Smeter.cpp
//

#include "SDR_RA8875.h"
#include "RadioConfig.h"
#include "Smeter.h"

//#include <Audio.h> 
extern AudioAnalyzePeak_F32 S_Peak;  
#ifdef USE_RA8875
	extern RA8875 tft;
#else 
	extern RA8876_t3 tft;
	extern void ringMeter(int val, int minV, int maxV, int16_t x, int16_t y, uint16_t r, const char* units, uint16_t colorScheme,uint16_t backSegColor,int16_t angle,uint8_t inc);
#endif
extern 			uint8_t 		user_Profile;
extern struct 	User_Settings 	user_settings[];
extern        	uint8_t       	MF_client; // Flag for current owner of MF knob services
extern 			bool 			MeterInUse;  // S-meter flag to block updates while the MF knob has control

////////////////////////// this is the S meter code/////totall uncalibrated use at your own risk
COLD void Peak()
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
			sprintf(string,"   S-%1.0f",s);
		else 
			sprintf(string,"S-9+%02.0f",dbuv);
		
		if (!MeterInUse)  // don't write while the MF knob is busy with a temporary focus
			displayMeter((int) s, string, 3);  // Call the button object display function. 
	}
}