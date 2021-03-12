#include <Encoder.h>

extern Encoder Position;
extern uint8_t curr_band;   // global tracks our current band setting.
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint32_t VFOB;
extern struct Band_Memory bandmem[];

static const uint32_t topFreq = 54000000;  // sets receiver upper  frequency limit 30 MHz
static const uint32_t bottomFreq = 1000000; // sets the receiver lower frequency limit 1.6 MHz

///////////////////////////spin the encoder win a frequency!!////////////////////////////
//   Input: Counts from encoder to change the current VFO by the current step rate.  
//			If newFreq is 0, then just use VFOx which may have been set elsewhere to a desired frequency
//			use this fucntion rather than setFreq() since this tracks VFOs and updates their memories.
//
void selectFrequency(uint32_t newFreq)
{
    uint16_t fstep = tstep[bandmem[curr_band].tune_step].step;
	uint32_t Freq;

	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
		Freq = VFOA;
	else
		Freq = VFOB;	

	Freq = (Freq + newFreq*fstep); 
	if (Freq >= topFreq)            
		Freq = topFreq;        
	if (Freq <= bottomFreq)            
		Freq = bottomFreq;   
	Serial.println(Freq);	Serial.println(newFreq);         
	
	if (bandmem[curr_band].VFO_AB_Active == VFO_A)
	{
		VFOA = Freq;
		bandmem[curr_band].vfo_A_last = VFOA;
	}
	else
	{
		VFOB = Freq;
		bandmem[curr_band].vfo_B_last = VFOB;
	}
    displayFreq(); // show freq on display
    SetFreq(Freq); // send freq to SI5351
}
