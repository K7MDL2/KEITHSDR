//
// Step.h
//

extern struct TuneSteps tstep[];
extern struct Band_Memory bandmem[];

void selectStep(int8_t swiped)
{ 
	
	static int direction = 1;
	int fndx = bandmem[curr_band].tune_step;
Serial.println(fndx);
	if (user_settings[user_Profile].fine == 0)
	{
		// 1. Limit to allowed step range
		// 2. Cycle up and at top, cycle back down, do nto roll over.
		if (fndx <= 1)
		{
			fndx = 1;
			direction = 1;   // cycle upwards
		}

		if (fndx >= TS_STEPS-1)
		{
			fndx = TS_STEPS-1;
			direction = -1;
		}
		
		if (swiped == 0)
			fndx += direction; // Index our step up or down
		else	
			fndx += swiped;  // forces a step higher or lower then current
		
		if (fndx > TS_STEPS-1)   // ensure we are still in range
			fndx = TS_STEPS - 1;  // just in case it over ranges, bad stuff happens when it does
		if (fndx < 1)
			fndx = 1;  // just in case it over ranges, bad stuff happens when it does		
	}
	if (user_settings[user_Profile].fine && swiped == -1)  // 1 Hz steps
		bandmem[curr_band].tune_step = 0;   // set to 1 hz steps
	else if (user_settings[user_Profile].fine && swiped == 1)
		bandmem[curr_band].tune_step = 1;    // normally swiped is +1 or -1
	else if (user_settings[user_Profile].fine && swiped == 0)
	{
Serial.println(fndx);
		if (fndx > 0)
			fndx = 0;			
		else
			fndx = 1;
Serial.println(fndx);
		bandmem[curr_band].tune_step = fndx;
	}
	else 
		bandmem[curr_band].tune_step = fndx;  // Fine tunig mode is off, allow all steps 10hz and higher

	Serial.print("Set Tune Step to ");
	Serial.println(tstep[bandmem[curr_band].tune_step].ts_name);
	displayRate();
}
