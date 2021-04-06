///////////////////////////spin the encoder win a frequency!!////////////////////////////
//
//		Tuner.cpp
//
//   Input: Counts from encoder to change the current VFO by the current step rate.  
//			If newFreq is 0, then just use VFOx which may have been set elsewhere to a desired frequency
//			use this fucntion rather than setFreq() since this tracks VFOs and updates their memories.
//


#include "SDR_RA8875.h"
#include "RadioConfig.h"
#include "Tuner.h"

extern uint8_t curr_band;   // global tracks our current band setting.
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint32_t VFOB;
extern struct Band_Memory bandmem[];
extern const struct TuneSteps tstep[];
extern void RampVolume(float vol, int16_t rampType);

#ifdef SV1AFN_BPF
  #include <SVN1AFN_BandpassFilters.h>
  extern SVN1AFN_BandpassFilters bpf;
#endif

static const uint32_t topFreq = 54000000;  // sets receiver upper  frequency limit 30 MHz
static const uint32_t bottomFreq = 1000000; // sets the receiver lower frequency limit 1.6 MHz

//
//-------------------------- selectFrequency --------------------------------------
//
void selectFrequency(int32_t newFreq)  // 0 = no change unless an offset is required for mode
{
    uint16_t fstep = tstep[bandmem[curr_band].tune_step].step;
  	uint32_t Freq;
  
  	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
  		  Freq = VFOA;
  	else
  		  Freq = VFOB;	
  
  	Freq = (Freq + newFreq*fstep); 
  	if (Freq >= topFreq)            
  		  Freq = topFreq;        
  	if (Freq <= bottomFreq)            
  		  Freq = bottomFreq;   

  	// If this is configured to be a pandapter then change from VFO to fixed LO.	
  	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
  	{
		#ifdef PANADAPTER
        	Freq = PANADAPTER_LO;
			if (bandmem[curr_band].mode_A == DATA)      
				Freq += PANADAPTER_MODE_OFFSET_DATA;  // offset if in DATA mode
		#else
  			VFOA = Freq;
  			bandmem[curr_band].vfo_A_last = VFOA;
		#endif
	
  	}
  	else
  	{
		#ifdef PANADAPTER
			Freq = PANADAPTER_LO;
			if (bandmem[curr_band].mode_B == DATA)      
            	Freq += PANADAPTER_MODE_OFFSET_DATA;  // offset if in DATA mode    
		#else
			VFOB = Freq;
  			bandmem[curr_band].vfo_B_last = VFOB;
		#endif		  
  	}
  
  	#ifdef SV1AFN_BPF
  	if (Freq < bandmem[curr_band].edge_lower || Freq > bandmem[curr_band].edge_upper)
  	{
  		RampVolume(0.0, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
  		//Serial.print("BPF Set to "); Serial.println("Bypassed");  
        bpf.setBand(HFBand(HFBypass));
  		RampVolume(1.0, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
  	}
  	else
  	{
    	//RampVolume(0.0, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
    	//Serial.print("BPF Set to "); Serial.println(bandmem[curr_band].preselector);  
        bpf.setBand(HFBand(bandmem[curr_band].preselector));
    	//RampVolume(1.0, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
  	}
  	#endif
  
    displayFreq(); // show freq on display
    SetFreq(Freq); // send freq to SI5351
}
