//***************************************************************************************************
//
//      SDR_CAT_Serial.cpp 
//
//      SDR_CAR_Serial Class for CAT side interface using Elecraft K3/kenwood CAT protocol to enable programs 
//          like WSJT-X Hamlib interface to set split and follow BAND/VFO changes from the radio side. 
//          Accepts expanded set of CAT port commands such as *FA, *FB, *SW0, etc. for Elecraft K3 protocol
//          and passes back these controls to the main program.  
//
//      Called to parse CA serial message by RS-HFIQ and non-RSHFIQ configurations
//
//      Feb 3, 2023  by K7MDL
//      
//      Placed in the Public Domain.
//
//***************************************************************************************************
#include <Arduino.h>
#include "RadioConfig.h"
#include "SDR_RA8875.h"
#include "SDR_CAT_Serial.h"

#ifdef USE_CAT_SER
#ifndef USE_RS_HFIQ

//#define DBG
//#define DEBUG_CAT  //set to true for debug output, false for no debug output
/*
#ifdef DEBUG_CAT
    //#define DSERIALBEGIN(...)    Serial.begin(__VA_ARGS__)
    //#define DPRINTLN(...)       Serial.println(__VA_ARGS__)
    //#define DPRINT(...)         Serial.print(__VA_ARGS__)
    //#define DRINTF(...)         Serial.print(F(__VA_ARGS__))
    //#define DPRINTLNF(...)      Serial.println(F(__VA_ARGS__)) //printing text using the F macro
    //#define DELAY(...)          delay(__VA_ARGS__)
    //#define PINMODE(...)        pinMode(__VA_ARGS__)
    //#define TOGGLEd13           PINB = 0x20                    //UNO's pin D13
    #define DEBUG_PRINT(...)    Serial.print(F(#__VA_ARGS__" = ")); Serial.print(__VA_ARGS__); Serial.print(F(" ")) 
    #define DEBUG_PRINTLN(...)  DEBUG_PRINT(__VA_ARGS__); Serial.println()
    #define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
    //#define DSERIALBEGIN(...)
    //#define DPRINTLN(...)
    //#define DPRINT(...)
    //#define DPRINTF(...)      
    //#define DPRINTLNF(...)    
    //#define DELAY(...)        
    //#define PINMODE(...)      
    //#define TOGGLEd13      
    #define DEBUG_PRINT(...)    
    #define DEBUG_PRINTLN(...) 
    #define DEBUG_PRINTF(...) 
#endif
*/

#ifdef USE_RA8875
    extern RA8875 tft;
#else
    extern RA8876_t3 tft;
#endif

// Serial port for external CAT control
#ifndef ALT_CAT_PORT
    #define CAT_port SerialUSB1  // if you have 2 serial ports.  Make this the 2nd.
#else
    #define CAT_port Serial   // if you only have 1 serial port and want CAT, turn off DEBUG and use this.
#endif

uint64_t rs_freq;
static char S_Input[20];

extern struct   User_Settings       user_settings[];
extern          uint8_t             user_Profile;
extern struct   Band_Memory         bandmem[];
extern          uint8_t             curr_band;   // global tracks our current band setting.
extern          uint64_t            find_new_band(uint64_t new_frequency, uint8_t &_curr_band);

// ************************************************* Setup *****************************************
//
// *************************************************************************************************
void SDR_CAT_Serial::setup_CAT_Serial()  // 0 non block, 1 blocking
{   
    CAT_port.begin(38400);
    DPRINTLNF("\nStart of CAT Serial Setup"); 
    tft.setFont(Arial_14);
    tft.setTextColor(BLUE);
    tft.setCursor(60, 320);
    tft.print(F("Waiting for connection to USB port - Is it connected?"));
    delay(1000);
    DPRINTLNF("End of CAT Serial Setup");
}

// The RS-HFIQ has only 1 "VFO" so does not itself care about VFO A or B or split, or which is active
// However this is also the CAT interface and commands will come down for such things.  
// We need to act on the active VFO and pass back the info needed to the calling program.
uint64_t SDR_CAT_Serial::cmd_console(uint8_t &_swap_vfo, uint64_t &_VFOA, uint64_t &_VFOB, uint8_t &_curr_band, uint8_t &_xmit, uint8_t &_split, uint8_t &_mode, uint8_t &_clip)  // returns new or unchanged active VFO value
{
    char c;
    //static unsigned char Ser_Flag = 0, Ser_NDX = 0;
    static uint8_t swap_vfo_last = 0;
    static uint8_t _ai = 0;  // track AI mode

    rs_freq = _VFOA;

    while (CAT_port.available() > 0)    // Process any and all characters in the buffer
    {
        c = CAT_port.readBytesUntil(';',S_Input, 15); 
        
        if (c>0)
        {
            #ifdef DBG 
                DPRINTF("SDR_CAT_Serial: Cmd String = "); DPRINTLN(S_Input);
            #endif
            if (!strncmp(S_Input, "ID", 2))
            {
                CAT_port.print("ID017;");
            } 
            else if (!strncmp(S_Input, "OM", 2))   // && c == 13)
            {
                CAT_port.print("OM ------------;");
            }
            else if (!strncmp(S_Input, "K20", 3)) // && strlen(S_Input) == 2)   // && c == 13)
            {
                //CAT_port.print("K20;");
            }
            else if (!strncmp(S_Input, "K22", 3)) // && strlen(S_Input) == 2)   // && c == 13)
            {
                //CAT_port.print("K22;");
            }
            else if (!strncmp(S_Input, "K2", 2)) // && strlen(S_Input) == 2)   // && c == 13)
            {
                CAT_port.print("K20;");
            }
            else if (!strncmp(S_Input, "K3", 2)) //&& strlen(S_Input) == 2)   // && c == 13)
            {
                CAT_port.print("K30;");
            }
            else if (!strncmp(S_Input, "RVM", 3)&& strlen(S_Input) == 3)
            {
                CAT_port.print("RVM05.67;");
            }
            else if (!strncmp(S_Input, "BW", 2) && strlen(S_Input) == 2)
            {
                CAT_port.print("BW4000;");
            }
            else if (!strncmp(S_Input, "LN", 2) && strlen(S_Input) == 2)
            {
                CAT_port.print("LN0;");
            }
            else if (!strncmp(S_Input, "PS", 2) && strlen(S_Input) == 2)
            {
                CAT_port.print("PS1;");   // radio is turned on
            }
            else if (!strncmp(S_Input, "FR", 2) && strlen(S_Input) == 2)
            {
                CAT_port.print("FR0;");
            }
            else if (!strncmp(S_Input, "FT", 2) && strlen(S_Input) == 2)
            {
                if (_split == 0) CAT_port.print("FT0;");
                else CAT_port.print("FT1;");
            }
            else if (!strncmp(S_Input, "FR0", 3)  && strlen(S_Input) == 3) // Split OFF
            {
                _split = 0;
                #ifdef DBG  
                DPRINTLNF("SDR_CAT_Serial: Split Mode OFF"); 
                #endif
            }
            else if (!strncmp(S_Input, "FT1", 3)  && strlen(S_Input) == 3) // Split ON
            {
                _split = 1;
                #ifdef DBG  
                DPRINTLNF("SDR_CAT_Serial: Split Mode ON VFOB=TX");
                #endif
            }
            else if (!strncmp(S_Input, "FT0", 3)  && strlen(S_Input) == 3) // Split ON
            {
                _split = 0;
                #ifdef DBG  
                DPRINTLNF("SDR_CAT_Serial: Split Mode ON VFOA=TX");
                #endif
            }
            else if (!strncmp(S_Input, "DT", 2) && strlen(S_Input) == 2)
            {
                CAT_port.print("DT0;");  // return 0 = DATA A mode
            }
            else if (!strncmp(S_Input, "FI", 2) && strlen(S_Input) == 2)
            {
                CAT_port.print("FI5000;");  // last 4 digits of the IF cener frequency used for shifting panadapater.
            }
            else if (!strncmp(S_Input, "AI", 2) && strlen(S_Input) == 2)
            {
                CAT_port.printf("AI%1d;", _ai);  // return current AI mode
            }
            else if (!strncmp(S_Input, "AI0", 3) && strlen(S_Input) == 3)
            {
                _ai = 0;
                //CAT_port.print("AI0;");
            }
            else if (!strncmp(S_Input, "AI1", 3) && strlen(S_Input) == 3)
            {
                _ai = 1;
                CAT_port.printf("IF%011llu     -000000 00%d600%d001 ;", rs_freq, user_settings[user_Profile].xmit, _split);
            }
            else if (!strncmp(S_Input, "AI2", 3) && strlen(S_Input) == 3)
            {
                _ai = 2;
                //CAT_port.print("AI2;");
                //CAT_port.printf("IF%011llu     -000000 00%d600%d001 ;", rs_freq, user_settings[user_Profile].xmit, *split);
            }
            else if (!strncmp(S_Input, "IF", 2)) // Transceiver Info
            {
                CAT_port.printf("IF%011llu     -000000 00%d600%d001 ;", rs_freq, user_settings[user_Profile].xmit, _split);
            }
            else if (!strncmp(S_Input, "MD$", 3) && strlen(S_Input) == 3 || !strncmp(S_Input, "MD", 2) && strlen(S_Input) == 2)   // report Radio current Mode per K3 numbering
            {
                uint8_t _mode_ = 0;

                switch (_mode)
                {
                    case LSB:       _mode_ = 1; break;
                    case USB:       _mode_ = 2;; break;
                    case CW:        _mode_ = 3; break;
                    case FM:        _mode_ = 4; break;
                    case AM:        _mode_ = 5; break;
                    case DATA:      _mode_ = 6; break;
                    case CW_REV:    _mode_ = 7; break;
                    case DATA_REV:  _mode_ = 9; break;
                    default: break;
                } 
                CAT_port.printf("MD%d;", _mode_);
            }
            else if (!strncmp(S_Input, "MD", 2) && strlen(S_Input) > 2)  // map incoming mode change request from K3 values to our mode numbering and return the value
            {
                switch (S_Input[2])
                {
                    case '1': _mode = LSB; break;
                    case '2': _mode = USB; break;
                    case '3': _mode = CW; break;
                    case '4': _mode = FM; break;
                    case '5': _mode = AM; break;
                    case '6': _mode = DATA; break;
                    case '7': _mode = CW_REV; break;
                    case '9': _mode = DATA_REV; break;
                    default: break;                    
                } 
            }
            else if (!strncmp(S_Input, "FA", 2) && strlen(S_Input) == 2)
            {
                CAT_port.printf("FA%011llu;", _VFOA);  // Respond back for confirmation FA + 11 freq + ;
            }
            else if (!strncmp(S_Input, "FB", 2) && strlen(S_Input) == 2)
            {
                CAT_port.printf("FB%011llu;", _VFOB);  // Respond back for confirmation FA + 11 freq + ;
            } 
            else if (!strncmp(S_Input, "FA", 2) && strlen(S_Input) > 2)
            {
                _VFOA = rs_freq = atoll(&S_Input[2]);   // Pass thru to main program to deal with and reply back to CAT program
            }
            else if (!strncmp(S_Input, "FB", 2) && strlen(S_Input) > 2)
            {
                _VFOB = rs_freq = atoll(&S_Input[2]);   // Pass thru to main program to deal with and reply back to CAT program
            }  
            else if (!strncmp(S_Input, "RX", 2))
            {
                #ifdef DBG  
                DPRINTLN(F("SDR_CAT_Serial: RX->XMIT OFF"));
                #endif
                _xmit = 0;
            }
            else if (!strncmp(S_Input, "TX", 2))
            {
                #ifdef DBG  
                DPRINTLN(F("SDR_CAT_Serial: TX->XMIT ON"));
                #endif
                _xmit = 1;
            }
            else if (!strncmp(S_Input, "SW0", 3))
            {
                if (swap_vfo_last)
                    swap_vfo_last = 0;
                else 
                    swap_vfo_last = 1;
                _swap_vfo = swap_vfo_last;
                #ifdef DBG  
                DPRINT(F("SDR_CAT_Serial: Swap VFOs: ")); DPRINTLN(_swap_vfo);
                #endif
            }
        }
    }
    S_Input[0] = '\0';
    for (int z = 0; z < 16; z++)  // Fill the buffer with spaces
        S_Input[z] = '\0';
    //Ser_Flag = 0;
    S_Input[0] = '\0';
    c=0;
    CAT_port.flush();
    //DPRINTF("rs_freq = "); DPRINTLN(rs_freq);
    //DPRINTF("_VFOA = "); DPRINTLN(_VFOA);
    return rs_freq;
}

#endif //USE_RS_HFIQ
#endif //USE_CAT_SER