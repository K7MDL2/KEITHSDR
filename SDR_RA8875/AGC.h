#include <Audio.h> 
extern AudioMixer4_F32              RX_Summer; 
extern int andx;
extern String agc;

void selectAgc()
{
 // String mode;
  
  if(andx==0)
  {
            Serial.println("Lets set the AGC to off");
            agc="AGC-Off";
  }
      
  if(andx==1)
  {
            Serial.println("Lets set the AGC to Slow");
            agc="AGC-S";
  }

  if(andx==2)
  {
            Serial.println("Lets set the AGC to Medium");
            agc="AGC-M";
  }
  
  if(andx==3)
  {
            Serial.println("Lets set the AGC to Fast");
            agc="AGC-F";
  }
  

  if(andx==4)
  {
    andx=0;
  }
  else
  {
    andx= andx+1;
  }
  displayAgc();
}
