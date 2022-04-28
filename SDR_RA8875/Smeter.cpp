//
//  Smeter.cpp
//

#include "SDR_RA8875.h"
#include "RadioConfig.h"
#include "Smeter.h"

//#include <Audio.h> 
uint8_t	smeter_avg = 0;  // Smeter mode.  1 = averaging, 0  = peak

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
extern 			int16_t 		barGraph;  // used for remote meter in Panadapter mode

#define WINDOW_SIZE 30		// number of s meter readings to average.  About 1ms per sample.

float Peak_avg(float val);  // calculate average raw smeter readings

////////////////////////// this is the S meter code/////totall uncalibrated use at your own risk
COLD float Peak(void)
{
   float s_sample;  // Raw signal strength (max per 1ms)
   float uv, dbuv, s;// microvolts, db-microvolts, s-units
   char string[80];   // print format stuff
   float pk_avg = 0; // used for RF AGC limiting
 
  	if (S_Peak.available())
    {
      	s_sample =S_Peak.read();
	    pk_avg = Peak_avg(s_sample);  // while here build up an average for the RF_AGC Limiter
		if (smeter_avg)   // use average or skip to use peak
			s_sample = pk_avg;
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

		#ifdef PANADAPTER
			if (user_settings[user_Profile].xmit)			
				sprintf(string,"   P-%1.0d", barGraph);
			else 
				sprintf(string,"   S-%1.0d", barGraph);
		#else
		// rounded meter
		if (dbuv == 0) 
			sprintf(string,"   S-%1.0f",s);
		else 
			sprintf(string,"S-9+%02.0f",dbuv);
		#endif

		
		if (!MeterInUse)  // don't write while the MF knob is busy with a temporary focus
			displayMeter((int) s, string, 3);  // Call the button object display function. 
	}
	return pk_avg;
}

// Use the S-meter results to build an average
HOT float Peak_avg(float val) 
{
  	static int16_t idx = 0;
	static float sum = 0;
	static float Readings[WINDOW_SIZE] = {};
	static float P_avg = 0;

	sum = sum - Readings[idx];       // Remove the oldest entry from the sum
	Readings[idx] = val;           // Add the newest reading to the window
	sum = sum + val;                 // Add the newest reading to the sum
	idx = (idx+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size

	P_avg = sum / WINDOW_SIZE;      // Divide the sum of the window by the window size for the result

	//MSG_Serial.print("S meter avg = ");
	//MSG_Serial.println(P_avg);

	return P_avg;
}