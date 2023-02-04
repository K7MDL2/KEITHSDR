//
//      SDR_CAT_Serial.h
//
//      Generic Hamlib compatible K3/Kenwood CAT cpontrol message parser
//
//      CAT Serial control library
//      Feb 3, 2023 by K7MDL
//
//      Placed in the Public Domain
//
//

#ifndef _SDR_CAT_SERIAL_H_
#define _SDR_CAT_SERIAL_H_

#include <Arduino.h>

class SDR_CAT_Serial
{
    public:
        SDR_CAT_Serial()  // -- Place any args here --          
        //  Place functions here if needed    ---
        {}  // Copy arguments to local variables
        // publish externally available functions
        uint64_t cmd_console(uint8_t &_swap_vfo, uint64_t &_VFOA, uint64_t &_VFOB, uint8_t &_curr_band, uint8_t &_xmit, uint8_t &_split, uint8_t &_mode, uint8_t &_clip);  // returns new or unchanged active VFO value
        // returns new or unchanged VFO value and modified band index and other parameters
        void     setup_CAT_Serial(void);
        //void   send_variable_cmd_to_RSHFIQ(const char * str, char * cmd_str);
        char *   convert_freq_to_Str(uint64_t freq);
        //void   send_fixed_cmd_to_RSHFIQ(const char * str);
        uint64_t find_new_band(uint64_t new_frequency, uint8_t &_curr_band);  // Validate frequency is RS-HFIQ comtaptible and retured band and frequency
                                                                                    // If freq is out of RS-HFIQ band then the freq returned is 0;
        //void   print_RSHFIQ(int flag);  // reads response from RS-HFIQ and prints to the CAT terminal
        //void   print_RSHFIQ_User(int flag);  // reads response from RS-HFIQ and prints to the user terminal
        
    private:  
        char freq_str[22] = "7074000";  // *Fxxxx command to set LO freq, PLL Clock 0
            
        //bool refresh_RSHFIQ(void);
        //void init_PLL(void);
        //void wait_reply(int blocking); // BLOCKING CALL!  Use with care       
        //void update_VFOs(uint64_t newfreq);
        //void write_RSHFIQ(int ch);
        //int  read_RSHFIQ(int flag);
};

#endif   // _SDR_CAT_SERIAL_H_
