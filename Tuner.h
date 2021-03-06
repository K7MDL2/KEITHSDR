#include <Encoder.h>

extern Encoder Position;
extern long oldFreq;
extern long newFreq;

extern volatile uint32_t Freq;
extern volatile uint32_t fstep;

 
static const long topFreq = 51000000;  // sets receiver upper  frequency limit 30 MHz
static const long bottomFreq = 1000000; // sets the receiver lower frequency limit 1.6 MHz

///////////////////////////spin the encoder win a frequency!!////////////////////////////
void selectFrequency()
{
  
    if(newFreq>oldFreq||newFreq<oldFreq) 
    {
    if(newFreq>oldFreq)
      {
        Freq=(Freq+fstep);
         
          if(Freq>=topFreq)
            {
              Freq=topFreq;
            }
      }
   
    if(newFreq<oldFreq)
      {
       Freq=(Freq-fstep);
       if(Freq<=bottomFreq)
            {
              Freq=bottomFreq;
            }
      }
    }
    displayFreq(); // show freq on display
    SetFreq(); // send freq to SI5351
    oldFreq=newFreq; //update oldFreq
 
}
