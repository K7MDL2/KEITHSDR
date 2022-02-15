//
//		Mode.cpp
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"
//#include "Mode.h"

extern AudioMixer4_F32  	RX_Summer; 
//extern AudioMixer4_F32  	FFT_Switch1;
//extern AudioMixer4_F32  	FFT_Switch2;
extern struct Modes_List 	modeList[];
extern int32_t 				ModeOffset;
extern struct User_Settings user_settings[];
extern uint8_t              user_Profile;

COLD void selectMode(uint8_t mndx)   // Change Mode of the current active VFO by increment delta.
{
	ModeOffset = 0;  // Holds displayed VFO offset based on CW mode pitch.  0 default for non-CW modes
	if(mndx == CW)
	{
		//mode="CW";
		AudioNoInterrupts();
		RX_Summer.gain(0, 3.0f);
		RX_Summer.gain(1, 3.0f);
		//Serial.println("CW");
		//FFT_Switch1.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
		ModeOffset = (int32_t) user_settings[user_Profile].pitch;  // show shaded filter width on right side of center
  	}

	if(mndx == CW_REV)
	{
		//mode="CW_REV;
		AudioNoInterrupts();
		RX_Summer.gain(0, 3.0f);
		RX_Summer.gain(1, -3.0f);
		//Serial.println("CW-R");
		//FFT_Switch1.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
		ModeOffset = -1 * (int32_t) user_settings[user_Profile].pitch; // show shaded filter width on left side of center
  	}

	if(mndx == USB)
	{
		//mode="USB";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 0.8f);
		RX_Summer.gain(1, 0.8f);
		//Serial.println("USB");
		//FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
		ModeOffset = 1; // show shaded filter width on right side of center
	}

	if(mndx == LSB)
	{
		//mode="LSB";
		AudioNoInterrupts();
		RX_Summer.gain(0, 0.8f);
		RX_Summer.gain(1, -0.8f);
		//Serial.println("LSB");
		//FFT_Switch1.gain(0,1.0f);   // 1  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT          
		//FFT_Switch2.gain(0,1.0f);   // 1  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT       
		AudioInterrupts(); 
		ModeOffset = -1; // show shaded filter width on left side of center
	}

	if(mndx == DATA)
	{
		//mode="DATA";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 0.8f);
		RX_Summer.gain(1, 0.8f);
		//Serial.println("DATA");
		//FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
		ModeOffset = 1; // show shaded filter width on right side of center
	}

	if(mndx == DATA_REV)
	{
		//mode="DATA_REV";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 0.8f);
		RX_Summer.gain(1,-0.8f);
		//Serial.println("DATA-R");
		//FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
		ModeOffset = -1; // show shaded filter width on left side of center
	}

	if(mndx == AM)
	{
		//mode="AM";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 0.8f);
		RX_Summer.gain(1, 0.8f);
		//FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
		ModeOffset = 0; // show shaded filter width on both sides of center
	}

	if(mndx == FM)
	{
		//mode="FM";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 0.8f);
		RX_Summer.gain(1, 0.8f);
		//FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
		ModeOffset = 0; // show shaded filter width on both sides of center
	}
	Serial.print("Set ModeOffset ");
	Serial.println(ModeOffset);
	Serial.print("Set mode to ");
	Serial.println(modeList[mndx].mode_label);  	
  	//displayMode();
}
