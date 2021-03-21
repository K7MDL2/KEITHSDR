//
// Step.h
//   
//   Provided for some backward compatibility. Use Rate(int swiped) instead for most uses.
//	 This will set the band to the index supplied.  Remove the arg and make it a global if needed.
//	 The tune rate step is stored in a databese of user settings. Rate(arg) will increment or decrement
//   the current tune rate, store it, and call displayRate().
//
//	 This function does the same as changing the database vaue direct and calls display to update all buttons and labels.

extern struct TuneSteps tstep[];
extern struct Band_Memory bandmem[];
extern uint8_t curr_band;   // global tracks our current band setting.

void selectStep(int fndx)
{ 
		if (fndx <= 1)
		{
			fndx = 0;
		}

		if (fndx >= TS_STEPS-1)
		{
			fndx = TS_STEPS-1;
		}
	//  remove the int fndx arg if using a global fndx
	bandmem[curr_band].tune_step = fndx;
	displayRate();
}
