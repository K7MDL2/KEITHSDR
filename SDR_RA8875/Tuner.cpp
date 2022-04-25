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

#ifdef USE_RS_HFIQ
    // init the RS-HFIQ library
    extern SDR_RS_HFIQ RS_HFIQ;
#endif
extern uint8_t curr_band;   // global tracks our current band setting.
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint32_t VFOB;
extern struct Band_Memory bandmem[];
extern struct User_Settings user_settings[];
extern uint8_t user_Profile;
extern const struct TuneSteps tstep[];
extern void RampVolume(float vol, int16_t rampType);
extern char * convert_freq_to_Str(uint32_t freq);
extern void send_fixed_cmd_to_RSHFIQ(const char * str);
extern void send_variable_cmd_to_RSHFIQ(const char * str, char * cmd_str);

#ifdef SV1AFN_BPF
  #include <SVN1AFN_BandpassFilters.h>
  extern SVN1AFN_BandpassFilters bpf;
#endif

static const uint32_t topFreq = 54000000;  // sets receiver upper  frequency limit 30 MHz
static const uint32_t bottomFreq = 1000000; // sets the receiver lower frequency limit 1.6 MHz

//
//-------------------------- selectFrequency --------------------------------------
//
COLD void selectFrequency(int32_t newFreq)  // 0 = no change unless an offset is required for mode
{
    uint16_t fstep = tstep[bandmem[curr_band].tune_step].step;
  	uint32_t Freq;
  
  	if (bandmem[curr_band].split && user_settings[user_Profile].xmit)
    	Freq = user_settings[user_Profile].sub_VFO;
	else
		Freq = VFOA;
  
  	Freq = (Freq + newFreq*fstep); 
  	if (Freq >= topFreq)            
  		  Freq = topFreq;        
  	if (Freq <= bottomFreq)            
  		  Freq = bottomFreq;   

// If this is configured to be a pandapter then change from VFO to fixed LO.	
	#ifdef PANADAPTER
		Freq = PANADAPTER_LO;
		if (bandmem[curr_band].mode_A == DATA)      
			Freq += PANADAPTER_MODE_OFFSET_DATA;  // offset if in DATA mode
	#else
		if (bandmem[curr_band].split && user_settings[user_Profile].xmit)
		{}
		else
		{
			VFOA = Freq;
			bandmem[curr_band].vfo_A_last = VFOA;
		}
	#endif

	#ifdef PANADAPTER
		#ifdef SV1AFN_BPF
			bpf.setBand(HFBand(HFBypass));
		#endif
	#else
		#ifdef SV1AFN_BPF
		if (Freq < bandmem[curr_band].edge_lower || Freq > bandmem[curr_band].edge_upper)
		{
			//RampVolume(0.0, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
			//Serial.print("BPF Set to "); Serial.println("Bypassed");  
			bpf.setBand(HFBand(HFBypass));
			//RampVolume(xx, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
		}
		else
		{
			//RampVolume(0.0, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
			//Serial.print("BPF Set to "); Serial.println(bandmem[curr_band].preselector);  
			bpf.setBand(HFBand(bandmem[curr_band].preselector));
			//RampVolume(xx, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
		}
		#endif
	#endif
  
    displayFreq(); // show freq on display
	#ifdef USE_RS_HFIQ
		RS_HFIQ.send_variable_cmd_to_RSHFIQ("*F", RS_HFIQ.convert_freq_to_Str(Freq));
	#else
    	SetFreq(Freq); // send freq to SI5351
	#endif
}
