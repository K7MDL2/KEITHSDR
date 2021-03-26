#ifndef _AGC_H_
#define _AGC_H_

#include <Audio.h> 
extern AudioMixer4_F32    RX_Summer; 
extern struct Band_Memory bandmem[];

void selectAgc(uint8_t andx)
{

	// TODO   Put in some code for custom AGC. Use the AGC table of settings.
    if (andx >= AGC_SET_NUM)
      	andx = AGC_OFF; 		// Cycle around

	if (andx <  AGC_OFF)
    	andx = AGC_SET_NUM - 1;		// Cycle around
		
  	bandmem[curr_band].agc_mode = andx;
 	//displayAgc();
}

#endif // _AGC_H_
