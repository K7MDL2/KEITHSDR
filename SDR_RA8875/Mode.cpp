//
//		Mode.cpp
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"
//#include "Mode.h"

extern AudioMixer4_F32  RX_Summer; 
//extern AudioMixer4_F32  FFT_Switch1;
//extern AudioMixer4_F32  FFT_Switch2;
extern struct Modes_List modeList[];

COLD void selectMode(uint8_t mndx)   // Change Mode of the current active VFO by increment delta.
{
	if(mndx == CW)
	{
		//mode="CW";
		AudioNoInterrupts();
		RX_Summer.gain(0, 2.0f);
		RX_Summer.gain(1, 2.0f);
		//Serial.println("CW");
		//FFT_Switch1.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
  	}

	if(mndx == CW_REV)
	{
		//mode="CW";
		AudioNoInterrupts();
		RX_Summer.gain(0, 2.0f);
		RX_Summer.gain(1, -2.0f);
		//Serial.println("CW-R");
		//FFT_Switch1.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
  	}
      
	if(mndx == LSB)
	{
		//mode="LSB";
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, -1.0f);
		//Serial.println("LSB");
		//FFT_Switch1.gain(0,1.0f);   // 1  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT          
		//FFT_Switch2.gain(0,1.0f);   // 1  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT       
		AudioInterrupts(); 
	}

	if(mndx == USB)
	{
		//mode="USB";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, 1.0f);
		//Serial.println("USB");
		//FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
	}

	if(mndx == DATA)
	{
		//mode="DATA";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, 1.0f);
		//Serial.println("DATA");
		//FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
	}

	if(mndx == DATA_REV)
	{
		//mode="DATA";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, -1.0f);
		//Serial.println("DATA-R");
		//FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
	}

	if(mndx == FM)
	{
		//mode="DATA";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, 1.0f);
		//FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
	}

	if(mndx == AM)
	{
		//mode="DATA";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, 1.0f);
		//FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		//FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		//FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
	}

	Serial.print("Set mode to ");
	Serial.println(modeList[mndx].mode_label);  	
  	//displayMode();
}
