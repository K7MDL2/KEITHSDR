///////////////////////////spin the encoder win a frequency!!////////////////////////////
//
//		Tuner.cpp
//
//   Input: Counts from encoder to change the current VFO by the current step rate.  
//			If newFreq is 0, then just use VFOx which may have been set elsewhere to a desired frequency
//			use this function rather than setFreq() since this tracks VFOs and updates their memories.
//

#include "SDR_RA8875.h"
#include "RadioConfig.h"
#include "Tuner.h"

#ifdef USE_RS_HFIQ
    extern SDR_RS_HFIQ RS_HFIQ;		// init the RS-HFIQ library
#endif
extern uint8_t curr_band;   // global tracks our current band setting.
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern struct Band_Memory bandmem[];
extern struct User_Settings user_settings[];
extern uint8_t user_Profile;
extern const struct TuneSteps tstep[];
extern char * convert_freq_to_Str(uint32_t freq);
extern void send_variable_cmd_to_RSHFIQ(const char * str, char * cmd_str);
extern int16_t rit_offset;  // global rit value in Hz
extern int16_t xit_offset;  // global xit value in Hz

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
		Freq = VFOA - (VFOA % fstep);   // Round down to step size if step > 1Hz
	Freq += newFreq * fstep;
    
    // Keep frequency within limits
  	if (Freq >= topFreq)            
  		Freq = topFreq;        
  	if (Freq <= bottomFreq)            
  		Freq = bottomFreq;   

// If this is configured to be a panadapter then change from VFO to fixed LO.	
	#ifdef PANADAPTER
		Freq = PANADAPTER_LO;
		if (bandmem[curr_band].mode_A == DATA)      
			Freq += PANADAPTER_MODE_OFFSET_DATA;  // offset if in DATA mode
	#else
		if (bandmem[curr_band].split && user_settings[user_Profile].xmit)
			user_settings[user_Profile].sub_VFO = Freq;
		else
		{
			VFOA = Freq;   // Do not store rit_offset into VFOA!
			bandmem[curr_band].vfo_A_last = VFOA;   // save for band stacking
		}
		
		//Now have the correct Freq form VFO A or B used xit or rit
		if (user_settings[user_Profile].xmit)
			Freq += xit_offset;  // Add in any XIT offset.  
		else
			Freq += rit_offset;  // Add in any RIT offset.  

		DPRINTF("TUNER: Freq = "); DPRINT(Freq); DPRINTF("  rit = "); DPRINT(rit_offset); DPRINTF("  xit = "); DPRINTLN(xit_offset);
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
			//DPRINT("BPF Set to ");DPRINTLN("Bypassed");  
			bpf.setBand(HFBand(HFBypass));
			//RampVolume(xx, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
		}
		else
		{
			//RampVolume(0.0, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
			//DPRINT("BPF Set to ");DPRINTLN(bandmem[curr_band].preselector);  
			bpf.setBand(HFBand(bandmem[curr_band].preselector));
			//RampVolume(xx, 1); //     0 ="No Ramp (instant)"  // loud pop due to instant change || 1="Normal Ramp" // graceful transition between volume levels || 2= "Linear Ramp"
		}
		#endif
	#endif
	
	#ifdef USE_RS_HFIQ
		RS_HFIQ.send_variable_cmd_to_RSHFIQ("*F", RS_HFIQ.convert_freq_to_Str(Freq));
	#else
    	SetFreq(Freq); // send freq to SI5351
		displayFreq(); // show freq on display
	#endif
}
