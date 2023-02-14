//
//      SDR_RS_HFIQ.h
//
//      Teensy 4 USB host port serial control library for RS-HFIQ SDR 5W transceiver board
//      April 24, 2022 by K7MDL
//
//      Placed in the Public Domain
//
//
#ifndef _SDR_RS_HFIQ_SERIAL_H_
#define _SDR_RS_HFIQ_SERIAL_H_

#ifdef USE_RS_HFIQ

#include <Arduino.h>

class SDR_RS_HFIQ
{
    public:
        SDR_RS_HFIQ()  // -- Place any args here --          
        //  Place functions here if needed    ---
        {}  // Copy arguments to local variables here
        // publish externally available functions below
        uint64_t cmd_console(uint8_t &_swap_vfo, uint64_t &_VFOA, uint64_t &_VFOB, uint8_t &_curr_band, uint8_t &_xmit, uint8_t &_split, uint8_t &_mode, uint8_t &_clip);  // returns new or unchanged active VFO value
                                                                    // returns new or unchanged VFO value and modified band index and other parameters
        void        setup_RSHFIQ(int _blocking, uint64_t VFO);
        void        send_variable_cmd_to_RSHFIQ(const char * str, char * cmd_str);
        char *      convert_freq_to_Str(uint64_t freq);
        void        send_fixed_cmd_to_RSHFIQ(const char * str);
        uint64_t    find_new_band(uint64_t new_frequency, uint8_t &_curr_band);  // Validate frequency is RS-HFIQ comtaptible and retured band and frequency
                                                                                    // If freq is out of RS-HFIQ band then the freq returned is 0;
        void        print_RSHFIQ(int flag);  // reads response from RS-HFIQ and prints to the CAT terminal
        void        print_RSHFIQ_User(int flag);  // reads response from RS-HFIQ and prints to the user terminal
        
    private:  
        char freq_str[22] = "7074000";  // *Fxxxx command to set LO freq, PLL Clock 0
        const char s_initPLL[5]       = "*OF3";   // turns on LO clock0 output and sets drive current.
        const char q_freq[4]          = "*F?";    // returns current LO frequency
        const char s_freq[3]          = "*F";     // set LO frequency template.  3 to 30Mhz range
        const char q_dev_name[3]      = "*?";     // example "RSHFIQ"
        const char q_ver_num[3]       = "*W";     // example "RS-HFIQ FW 2.4a"
        const char s_TX_OFF[4]        = "*X0";    // Transmit OFF 
        const char s_TX_ON[4]         = "*X1";    // Transmit ON - power is controlled via audio input level
        const char q_Temp[3]          = "*T";     // Temp on board in degrees C
        const char q_Analog_Read[3]   = "*L";     // analog read
        const char q_EXT_freq[4]      = "*E?";    // query the setting for PLL Clock 2 frequency presented on EX-RF jack or used for CW
        const char s_EXT_freq[15]     = "*E";     // sets PLL Clock 1.  4KHz to 225Mhz range
        const char q_F_Offset[4]      = "*D?";    // Query Offset added to LO, BIT, or EXT frequency
        const char s_F_Offset[15]     = "*D";     // Sets Offset to add to LO, BIT, or EXT frequency
        const char q_clip_on[3]       = "*C";     // clipping occuring, add external attenuation
        const char q_BIT_freq[4]      = "*B?";    // Built In Test. Uses PLL clock 1        // Internal band validation.  Can be bypassed for any frequency if desired in the code.
        const char s_BIT_freq[4]      = "*B";     // Built In Test. Uses PLL clock 1        // Internal band validation.  Can be bypassed for any frequency if desired in the code.
            
        bool refresh_RSHFIQ(void);
        void init_PLL(void);
        void wait_reply(int blocking); // BLOCKING CALL!  Use with care       
        void update_VFOs(uint64_t newfreq);
        void write_RSHFIQ(int ch);
        int  read_RSHFIQ(int flag);
};

  #endif // USE_RS_HFIQ
#endif   // _SDR_RS_HFIQ_SERIAL_H_
