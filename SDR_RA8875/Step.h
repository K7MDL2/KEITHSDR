//
// Step.h
//

extern struct TuneSteps tstep[];
extern struct Band_Memory bandmem[];

void selectStep()
{ 
	
	static int direction = 1;
	int fndx = bandmem[curr_band].tune_step;
	
	// 1. Limit to allowed step range
	// 2. Cycle up and at top, cycle back down, do nto roll over.
  	if (fndx <= 0)
 	{
   		fndx = 0;
		direction = 1;   // cycle upwards
	}

	if (fndx >= TS_STEPS-1)
	{
      	fndx = TS_STEPS-1;
		direction = -1;
	}
	
	fndx += direction; // Index our step up or down

	if (fndx > TS_STEPS-1)   // ensure we are still in range
		fndx = TS_STEPS - 1;  // just in case it over ranges, bad stuff happens when it does
	if (fndx < 0)
		fndx = 0;  // just in case it over ranges, bad stuff happens when it does		


	bandmem[curr_band].tune_step = fndx;

	Serial.print("Set Tune Step to ");
	Serial.println(tstep[bandmem[curr_band].tune_step].ts_name);
	displayRate();
}
