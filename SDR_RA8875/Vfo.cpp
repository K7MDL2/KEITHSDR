//
//  Vfo.cpp
//
//
#include "SDR_RA8875.h"
#include "Vfo.h"
#include "RadioConfig.h"

#ifndef USE_RS_HFIQ
    #ifdef OCXO_10MHZ
    #include <si5351.h> 
    extern Si5351 si5351;
    #else
    #include <si5351mcu.h> 
    extern Si5351mcu si5351;
    #endif
#endif

extern uint32_t VFOA;  // 0 value should never be used more than 1st boot before EEPROM since init should read last used from table.
extern int32_t Fc;

//#define VFO_MULT    2   // 4x for QRP-Labs RX, 2x for NT7V QSE/QSD board, defined in RadioCfg.h

//////////////////////////Initialize VFO/DDS//////////////////////////////////////////////////////
COLD void initVfo(void)
{

    #ifdef USE_RS_HFIQ
        return;
    #else
        
        delay(100);
        
        //#define OCXO_10MHZ in main header file - Uncomment it there for external reference on the si5351C board version
        #ifdef OCXO_10MHZ
            /*
            * init(uint8_t xtal_load_c, uint32_t ref_osc_freq, int32_t corr)
            *
            * Setup communications to the Si5351 and set the crystal
            * load capacitance.
            *
            * xtal_load_c - Crystal load capacitance. Use the SI5351_CRYSTAL_LOAD_*PF
            * defines in the header file
            * xo_freq - Crystal/reference oscillator frequency in 1 Hz increments.
            * Defaults to 25000000 if a 0 is used here.
            * 27000000 is also common for some boards so be sure to enter thqt number if yours is 27Mhz.
            * corr - Frequency correction constant in parts-per-billion
            *
            * Returns a boolean that indicates whether a device was found on the desired
            * I2C address.
            */

            // --------------- Internal Crystal Clock Section ---------------------------------
            // Initialize the Si5351 to use a 27 MHz clock on the XO input
            #ifndef K7MDL_OCXO
                //si5351.init(SI5351_CRYSTAL_LOAD_8PF, 27000000, 0);      //set up our PLL for internal crystal or TCXO
                si5351.init(SI5351_CRYSTAL_LOAD_0PF,  24999899, 0);
                //si5351.init(SI5351_CRYSTAL_LOAD_0PF,  10000000, 0);
                si5351.set_clock_source(SI5351_CLK0, SI5351_CLK_SRC_XTAL);
                // --------------- End Internal Clock section -------------------------------------  
            #endif      
            // ----  OR  ------//
            
            #ifdef K7MDL_OCXO
                // --------------- External Reference CLock Section -------------------------------
                //Initialize the Si5351 to use an external clock like a 10Mhz OCXO
                si5351.init(SI5351_CRYSTAL_LOAD_0PF, 0, 0);               
                //This section is for external ref clock
                si5351.set_clock_source(SI5351_CLK0, SI5351_CLK_SRC_CLKIN);    // Use the OCXO for Clock 0 output                       
                // Set the CLKIN reference frequency to 10 MHz
                si5351.set_ref_freq(10000000UL, SI5351_PLL_INPUT_CLKIN);
                // Apply a correction factor to CLKIN
                si5351.set_correction(0, SI5351_PLL_INPUT_CLKIN);
                // Set PLLA and PLLB to use the signal on CLKIN instead of the XTAL
                si5351.set_pll_input(SI5351_PLLA, SI5351_PLL_INPUT_CLKIN);
                si5351.set_pll_input(SI5351_PLLB, SI5351_PLL_INPUT_CLKIN);
                // ------------   ---End Ext Clock section ----------------------------------------
            #endif // K7MDL_OCXO

            // Below is common to both Ext Clk and Internal Xtal
            si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
            si5351.output_enable(SI5351_CLK0, 1);   // ON by default but just in case.
            si5351.output_enable(SI5351_CLK1, 0);   // OFF by default but just in case.
            //si5351.output_enable(SI5351_CLK2, 0);   // OFF by default but just in case.  
            //si5351.output_enable(SI5351_CLK3, 0);   // outputs 3 - 8 only apply to the C version PLL     
            //si5351.output_enable(SI5351_CLK4, 0);     
            //si5351.output_enable(SI5351_CLK5, 0);     
            //si5351.output_enable(SI5351_CLK6, 0);        
            //si5351.output_enable(SI5351_CLK7, 0);     
            // Set CLK0 to output Dial Frequency
            si5351.set_freq((VFOA+Fc) * VFO_MULT + 100ULL , SI5351_CLK0);                         // set the output freq on CLK 0 top 5Mhz to start out.
            si5351.reset();   // Must do this for external clock!        
        #else

            // Choose one of these 3 lines. The first is the default crystal which is varied among boards
            //si5351.init();  // Set this to 25MHz or 27MHz depending on what your PLL uses.
            #ifdef si5351_XTAL_25MHZ
            //si5351.init(25000000);
            si5351.init(24999899);       // value for TCXO board   
            #else
            si5351.init(27000000);
            #endif // si5351_XTAL_IS_25MHZ

            /// Si5351mcu library modified by K7MDL to accept load capacitor setting.  Comment this out for standard library        
            #ifdef si5351_TCXO
                si5351.load_c(SI5351_CRYSTAL_LOAD_0PF);  // this is for replacing a crystal with a TCXO
            #endif //si5351_TCXO
            
            // The lines below are stanard to any crystal
            #ifdef si5351_CORRECTION
                si5351.correction(si5351_CORRECTION);   // Set this for your own PLL's crystal error. 100 seems like about 25Hz
            #endif
            si5351.setPower(0, SIOUT_8mA);   // 0 is Clock 0
            si5351.setFreq(0, (VFOA+Fc) * VFO_MULT);  // Multiply x4 for RX board
            si5351.enable(0);   // these enable/disables are optional
            si5351.disable(1);
            si5351.disable(2);
            // si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
            si5351.reset();   // Only do for very large change and after initial setup.
        #endif
    #endif
}

COLD void SetFreq(uint32_t Freq)
{ 
    #ifdef USE_RS_HFIQ
        return;
    #else
        #ifdef OCXO_10MHZ
            si5351.set_freq((Freq+Fc) * VFO_MULT * 100ULL, SI5351_CLK0); // generating 4 x frequency ... set 400ULL to 100ULL for 1x frequency
        #else
            si5351.setFreq(0, (Freq+Fc) * VFO_MULT); // use 4x for QRP-Labs RX vboard and some others. Use 1x if using 2 outputs shifted by 90 degrees 
        #endif
    #endif
}