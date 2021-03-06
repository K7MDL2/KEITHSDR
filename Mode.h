#include <Audio.h> 
extern AudioMixer4_F32  RX_Summer; 
extern AudioMixer4_F32  FFT_Switch1;
extern AudioMixer4_F32  FFT_Switch2;
extern int mndx;
extern String mode;

void selectMode()
{
 // String mode;
  
  if(mndx==0)
  {
            Serial.println("Lets set the mode to CW");
            mode="CW";
            AudioNoInterrupts();
              RX_Summer.gain(0,1);
              RX_Summer.gain(1,-1);
              FFT_Switch1.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
              FFT_Switch1.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
              FFT_Switch2.gain(0,0.0f);  // 0  for Filtered FFT,  1 for Unfiltered FFT
              FFT_Switch2.gain(1,1.0f);  // 1  for Filtered FFT,  0 for Unfiltered FFT
            AudioInterrupts();
  }
      
  if(mndx==1)
  {
            Serial.println("Lets set the mode to LSB");
            mode="LSB";
            AudioNoInterrupts();
              RX_Summer.gain(0,1);
              RX_Summer.gain(1,-1);
              FFT_Switch1.gain(0,1.0f);   // 1  for Filtered FFT,  1 for Unfiltered FFT
              FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT          
              FFT_Switch2.gain(0,1.0f);   // 1  for Filtered FFT,  1 for Unfiltered FFT
              FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT       
            AudioInterrupts(); 
  }

  if(mndx==2)
  {
            Serial.println("Lets set the mode to USB");
            mode="USB";          
            AudioNoInterrupts();
              RX_Summer.gain(0,1);
              RX_Summer.gain(1,1);
              FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
              FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
              FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
              FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
            AudioInterrupts();
  }

  if(mndx==3)
  {
            Serial.println("Lets set the mode to DATA at 4KHz BW");
            mode="DATA";          
            AudioNoInterrupts();
              RX_Summer.gain(0,1);
              RX_Summer.gain(1,1);
              FFT_Switch1.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
              FFT_Switch1.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
              FFT_Switch2.gain(0,1.0f);   // 0  for Filtered FFT,  1 for Unfiltered FFT
              FFT_Switch2.gain(1,0.0f);   // 1  for Filtered FFT,  0 for Unfiltered FFT
            AudioInterrupts();
  }
  
  displayMode();
   
  if(mndx==3)
  {
    mndx=0;
  }
  else
  {
    mndx= mndx+1;
  }
  
}
