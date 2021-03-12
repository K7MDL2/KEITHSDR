#include <si5351mcu.h> 
extern Si5351mcu si5351;
//extern volatile uint32_t Freq;
extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern uint32_t Fc;

//#define TCXO_24MHZ   // used for user installed TCXO instead of crystal wired into standard si5351a chip.

//////////////////////////Initialize VFO/DDS//////////////////////////////////////////////////////
void initVfo()
{
    delay(100);

    // use the below statement for unmodified library.
    #ifdef TCXO_24MHZ
        si5351.init(24000000);
            /// Si5351mcu library modified by K7MDL to accept load capacitor setting.  Comment this out for standard library
        si5351.load_c(SI5351_CRYSTAL_LOAD_10PF);
         si5351.correction(0);
    #else
        si5351.init(25000000);
        si5351.correction(1300);
    #endif

    si5351.setPower(0, SIOUT_8mA);   // 0 is Clock 0
    si5351.setFreq(0, (VFOA+Fc)*4);  
    si5351.enable(0);   // these enable/disables are optional
    si5351.disable(1);
    si5351.disable(2);
    si5351.reset();   // Only do for very large change and after initial setup.
}

void SetFreq(uint32_t Freq)
{ 
 si5351.setFreq(0, (Freq+Fc)*4);  
}
