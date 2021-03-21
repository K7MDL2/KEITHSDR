//
//	 Mode.h
//

extern AudioMixer4_F32  RX_Summer; 
extern AudioMixer4_F32  FFT_Switch1;
extern AudioMixer4_F32  FFT_Switch2;

void selectMode(uint8_t delta)   // Change Mode of the current active VFO by increment delta.
{
	uint8_t mndx;

	if (bandmem[curr_band].VFO_AB_Active == VFO_A)  // get Active VFO mode
		mndx = bandmem[curr_band].mode_A;			
	else
		mndx = bandmem[curr_band].mode_B;
	
	mndx += delta; // Make the change

  	if (mndx > DATA)		// Validate change anb fix if needed
   		mndx=0;
	if (mndx < CW)
		mndx = CW;

	if(mndx == CW)
	{
		//mode="CW";
		AudioNoInterrupts();
		RX_Summer.gain(0,1);
		RX_Summer.gain(1,-1);
		FFT_Switch1.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
		FFT_Switch1.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
		FFT_Switch2.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
		FFT_Switch2.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
  	}
      
	if(mndx == LSB)
	{
		//mode="LSB";
		AudioNoInterrupts();
		RX_Summer.gain(0,1);
		RX_Summer.gain(1,-1);
		FFT_Switch1.gain(0,1.0f);   // 1  for Filtered FFT,  1 for Unfiltered FFT
		FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT          
		FFT_Switch2.gain(0,1.0f);   // 1  for Filtered FFT,  1 for Unfiltered FFT
		FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT       
		AudioInterrupts(); 
	}

	if(mndx == USB)
	{
		//mode="USB";          
		AudioNoInterrupts();
		RX_Summer.gain(0,1);
		RX_Summer.gain(1,1);
		FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
	}

	if(mndx == DATA)
	{
		//mode="DATA";          
		AudioNoInterrupts();
		RX_Summer.gain(0,1);
		RX_Summer.gain(1,1);
		FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
		FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
		AudioInterrupts();
	}

	if (bandmem[curr_band].VFO_AB_Active == VFO_A)   // Store our mode for the active VFO
		bandmem[curr_band].mode_A = mndx;
	else	
		bandmem[curr_band].mode_B = mndx;
	Serial.print("Lets set the mode to ");
	Serial.println(Mode[mndx]);  	
  	//displayMode();
}
