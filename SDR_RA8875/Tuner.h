#include <Encoder.h>

extern Encoder Position;
extern long oldFreq;
extern long newFreq;

extern uint8_t curr_band;   // global tracks our current band setting.
extern volatile uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern volatile uint32_t VFOB;
extern struct Band_Memory bandmem[];

static const long topFreq = 54000000;  // sets receiver upper  frequency limit 30 MHz
static const long bottomFreq = 1000000; // sets the receiver lower frequency limit 1.6 MHz

///////////////////////////spin the encoder win a frequency!!////////////////////////////
void selectFrequency()
{
    uint16_t fstep = tstep[bandmem[curr_band].tune_step].step;
	uint32_t Freq = VFOA;

    if (newFreq>oldFreq || newFreq<oldFreq) 
    {
    	if (newFreq > oldFreq)
      	{
        	Freq = (Freq + fstep);   
          	if (Freq >= topFreq)            
              	Freq = topFreq;        
      	}
   
    	if(newFreq<oldFreq)
      	{
       		Freq = (Freq - fstep);
       		if (Freq <= bottomFreq)            
              	Freq = bottomFreq;            
      	}
    }
	VFOA = Freq;
    displayFreq(); // show freq on display
    SetFreq(); // send freq to SI5351
    oldFreq=newFreq; //update oldFreq 
}
