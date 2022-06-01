//
//		Mode.cpp
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"
//#include "Mode.h"

extern AudioMixer4_F32  	RX_Summer; 
extern AudioSwitch4_OA_F32  RxTx_InputSwitch_L;
extern AudioSwitch4_OA_F32  RxTx_InputSwitch_R;
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
		RX_Summer.gain(0, 1.0f);  // Turn on non-FM
		RX_Summer.gain(1, 1.0f);
		RX_Summer.gain(3, 0.0f);  // Turn off FM
		// Select our sources for the FFT.  mode.h will change this so CW uses the output (for now as an experiment)
        RxTx_InputSwitch_L.setChannel(0); // Select RX path
        RxTx_InputSwitch_R.setChannel(0); // Select RX path
		AudioInterrupts();
		ModeOffset = (int32_t) user_settings[user_Profile].pitch;  // show shaded filter width on right side of center
  	}

	if(mndx == CW_REV)
	{
		//mode="CW_REV;
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, -1.0f);
		RX_Summer.gain(3, 0.0f);  // Turn off FM
		// Select our sources for the FFT.  mode.h will change this so CW uses the output (for now as an experiment)
        RxTx_InputSwitch_L.setChannel(0); // Select RX path
        RxTx_InputSwitch_R.setChannel(0); // Select RX path
		AudioInterrupts();
		ModeOffset = -1 * (int32_t) user_settings[user_Profile].pitch; // show shaded filter width on left side of center
  	}

	if(mndx == USB)
	{
		//mode="USB";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, 1.0f);
		RX_Summer.gain(3, 0.0f);  // Turn off FM
		// Select our sources for the FFT.  mode.h will change this so CW uses the output (for now as an experiment)
        RxTx_InputSwitch_L.setChannel(0); // Select RX path
        RxTx_InputSwitch_R.setChannel(0); // Select RX path
		AudioInterrupts();
		ModeOffset = 1; // show shaded filter width on right side of center
	}

	if(mndx == LSB)
	{
		//mode="LSB";
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, -1.0f);
		RX_Summer.gain(3, 0.0f);  // Turn off FM
		// Select our sources for the FFT.  mode.h will change this so CW uses the output (for now as an experiment)
        RxTx_InputSwitch_L.setChannel(0); // Select RX path
        RxTx_InputSwitch_R.setChannel(0); // Select RX path      
		AudioInterrupts(); 
		ModeOffset = -1; // show shaded filter width on left side of center
	}

	if(mndx == DATA)
	{
		//mode="DATA";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, 1.0f);
		RX_Summer.gain(3, 0.0f);  // Turn off FM
		// Select our sources for the FFT.  mode.h will change this so CW uses the output (for now as an experiment)
        RxTx_InputSwitch_L.setChannel(0); // Select RX path
        RxTx_InputSwitch_R.setChannel(0); // Select RX path
		AudioInterrupts();
		ModeOffset = 1; // show shaded filter width on right side of center
		NBLevel(-100);	// Turn off NB for data modes
	}

	if(mndx == DATA_REV)
	{
		//mode="DATA_REV";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, -1.0f);
		RX_Summer.gain(3, 0.0f);  // Turn off FM
		// Select our sources for the FFT.  mode.h will change this so CW uses the output (for now as an experiment)
        RxTx_InputSwitch_L.setChannel(0); // Select RX path
        RxTx_InputSwitch_R.setChannel(0); // Select RX path
		AudioInterrupts();
		ModeOffset = -1; // show shaded filter width on left side of center
		NBLevel(-100);	// Turn off NB for data modes
	}

	if(mndx == AM)
	{
		//mode="AM";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 1.0f);
		RX_Summer.gain(1, 1.0f);
		RX_Summer.gain(3, 0.0f);  // Turn off FM
		// Select our sources for the FFT.  mode.h will change this so CW uses the output (for now as an experiment)
        RxTx_InputSwitch_L.setChannel(0); // Select RX path
        RxTx_InputSwitch_R.setChannel(0); // Select RX path
		AudioInterrupts();
		ModeOffset = 0; // show shaded filter width on both sides of center
	}

	if(mndx == FM)
	{
		//mode="FM";          
		AudioNoInterrupts();
		RX_Summer.gain(0, 0.0f);
		RX_Summer.gain(1, 0.0f);  // Turn off other modes
		RX_Summer.gain(3, 1.0f);  // Select FM path
		RxTx_InputSwitch_L.setChannel(2); // Select FM path
		RxTx_InputSwitch_R.setChannel(2); // Shut off unused output (in this mode)		
		AudioInterrupts();
		ModeOffset = 0; // show shaded filter width on both sides of center
		NBLevel(-100);	// Turn off NB for FM mode
	}

	//DPRINT("Set ModeOffset "); DPRINTLN(ModeOffset);
	//DPRINT("Set mode to "); DPRINTLN(modeList[mndx].mode_label);  	
  	//displayMode();
}
