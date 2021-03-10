//
//	 Mode.h
//

extern AudioMixer4_F32  RX_Summer; 
extern AudioMixer4_F32  FFT_Switch1;
extern AudioMixer4_F32  FFT_Switch2;

void selectMode(uint8_t mndx)
{
  	if (mndx > DATA)
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

	bandmem[curr_band].mode = mndx;
	Serial.print("Lets set the mode to ");
	Serial.println(bw[bandmem[curr_band].mode].bw_name);  	
  	displayMode();
}
