#include <si5351mcu.h> 
extern Si5351mcu si5351;
extern volatile uint32_t Freq;
extern volatile uint32_t Fc;
//////////////////////////Initialize VFO/DDS//////////////////////////////////////////////////////
void initVfo()
{
delay(100);
// use the below statement for unmodified library.
si5351.init(25000000);
 
/// Si5351mcu library modified by K7MDL to accept load capacitor setting.  Comment this out for standard library
//si5351.load_c(SI5351_CRYSTAL_LOAD_8PF);
///

si5351.correction(2450);
si5351.setPower(0, SIOUT_8mA);   // 0 is Clock 0
si5351.setFreq(0, (Freq+Fc)*4);  
si5351.enable(0);   // these enable/disables are optional
si5351.disable(1);
si5351.disable(2);
si5351.reset();   // Only do for very large change and after initial setup.
}

void SetFreq()
{ 
 si5351.setFreq(0, (Freq+Fc)*4);  
}
