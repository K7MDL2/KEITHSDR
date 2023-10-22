//***************************************************************************************************
//
//      SDR_RS_HFIQ.cpp 
//
//      June 1, 2022 by K7MDL
//      Dec 17, 2022 Changed CAT side interface to Elecraft K3 protocol to enable programs like WSJT-X 
//                   to set split and follow BAND/VFO changes from the radio side.  The RSHFIQ was only 
//                   controlled by a computer, this is bidirectional now and no external proxy required.
//
//      USB host control for RS-HFIQ 5W transciever board
//      USB Host comms portion from the Teensy4 USBHost_t36 example program
//      Requires Teensy 4.0 or 4.1 with USB Host port connector.
//
//      Accepts expanded set of CAT port commands such as *FA, *FB, *SW0, etc. for Elecraft K3 protocol
//          and passes back these controls to the main program.  
//      A Custom rig file is part of the GitHub project files for the Teensy SDR project under user K7MDL2
//          at https://github.com/K7MDL2/KEITHSDR/tree/main/SDR_RA8875/RS-HFIQ%20Omni-RIg
//
//      Placed in the Public Domain.
//
//***************************************************************************************************
#include <Arduino.h>
#include <USBHost_t36.h>
#include "RadioConfig.h"
#include "SDR_RA8875.h"
#include "SDR_RS_HFIQ.h"

#ifdef USE_RS_HFIQ
/*
//#define DEBUG_RSHFIQ  //set to true for debug output, false for no debug output
#ifdef DEBUG_RSHFIQ
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

// Teensy USB Host port
#define USBBAUD 57600   // RS-HFIQ uses 57600 baud
uint32_t baud = USBBAUD;
uint32_t format = USBHOST_SERIAL_8N1;
USBHost RSHFIQ;
USBHub hub1(RSHFIQ);
USBHub hub2(RSHFIQ);
USBHIDParser hid1(RSHFIQ);
USBHIDParser hid2(RSHFIQ);
USBHIDParser hid3(RSHFIQ);

//#define DBG

uint64_t rs_freq;
int  counter  = 0;
int  blocking = 1;  // 0 means do not wait for serial response from RS-HFIQ - for testing only.  1 is normal
static char S_Input[20];
static char R_Input[20];

#define RS_BANDS    9
struct RS_Band_Memory {
    uint8_t     band_num;        // Assigned bandnum for compat with external program tables
    char        band_name[10];  // Friendly name or label.  Default here but can be changed by user.  Not actually used by code.
    uint64_t    edge_lower;     // band edge limits for TX and for when to change to next band when tuning up or down.
    uint64_t    edge_upper;
};

struct RS_Band_Memory rs_bandmem[RS_BANDS] = {
    {  1, "80M", 3500000, 4000000},
    {  2, "60M", 4990000, 5367000},  // expanded to include coverage to lower side of WWV
    {  3, "40M", 7000000, 7300000},
    {  4, "30M", 9990000,10150000},  // expanded to include coverage to lower side of WWV
    {  5, "20M",14000000,14350000},
    {  6, "17M",18068000,18168000},
    {  7, "15M",21000000,21450000},
    {  8, "12M",24890000,24990000},
    {  9, "10M",28000000,29600000}
};

// There is now two versions of the USBSerial class, that are both derived from a common Base class
// The difference is on how large of transfers that it can handle.  This is controlled by
// the device descriptor, where up to now we handled those up to 64 byte USB transfers.
// But there are now new devices that support larger transfer like 512 bytes.  This for example
// includes the Teensy 4.x boards.  For these we need the big buffer version. 
// uncomment one of the following defines for userial
USBSerial userial(RSHFIQ);  // works only for those Serial devices who transfer <=64 bytes (like T3.x, FTDI...)
//USBSerial_BigBuffer userial(myusb, 1); // Handles anything up to 512 bytes
//USBSerial_BigBuffer userial(myusb); // Handles up to 512 but by default only for those > 64 bytes

USBDriver *drivers[] = {&hub1, &hub2, &hid1, &hid2, &hid3, &userial};
#define CNT_DEVICES (sizeof(drivers)/sizeof(drivers[0]))
const char * driver_names[CNT_DEVICES] = {"Hub1", "Hub2",  "HID1", "HID2", "HID3", "USERIAL1" };
bool driver_active[CNT_DEVICES] = {false, false, false, false};

extern struct   User_Settings       user_settings[];
extern          uint8_t             user_Profile;
extern struct   Band_Memory         bandmem[];
extern          uint8_t             curr_band;   // global tracks our current band setting.
extern          uint64_t            find_new_band(uint64_t new_frequency, uint8_t &_curr_band);

// ************************************************* Setup *****************************************
//
// *************************************************************************************************
void SDR_RS_HFIQ::setup_RSHFIQ(int _blocking, uint64_t VFO)  // 0 non block, 1 blocking
{   
    CAT_port.begin(38400);
    delay(100);
    DPRINTLN("\nStart of RS-HFIQ Setup"); 
    //SerialUSB1.println("\nStart of RS-HFIQ Setup"); 
    rs_freq = VFO;
    blocking = _blocking;

    tft.setFont(Arial_14);
    tft.setTextColor(BLUE);
    tft.setCursor(60, 320);
    tft.print(F("Waiting for connection to RS-HFIQ Radio via USB Host port - Is it connected?"));
    DPRINTLNF("Looking for USB Host Connection to RS-HFIQ");
    delay(5000);
    RSHFIQ.begin();
    //delay(1000);
    DPRINTLNF("Waiting for RS-HFIQ device to register on USB Host port  ");

    int retry_count = 0;
    while (!refresh_RSHFIQ())  // observed about 500ms required.
    {        
        if (!blocking) break; 
        tft.setFont(Arial_14);
        tft.setTextColor(RED);
        tft.setCursor(60, 420);
        tft.printf("Retry Count: %d", retry_count++);
        // wait until we have a valid USB 
        DPRINTF("Retry RS-HFIQ USB connection (~500ms) = "); DPRINTLN(counter++);
    }
    delay(2000);  // about 1-2 seconds needed before RS-HFIQ ready to receive commands over USB
    
    while (userial.available() > 0)  // Clear out RX channel garbage if any
        DPRINTLN(userial.read());

    send_fixed_cmd_to_RSHFIQ(q_dev_name); // get our device ID name
    DPRINTF("Device Name: "); print_RSHFIQ_User(blocking);  // waits for serial available (BLOCKING call);
    
    send_fixed_cmd_to_RSHFIQ(q_ver_num);
    DPRINTF("Version: "); print_RSHFIQ_User(blocking);  // waits for serial available (BLOCKING call);

    send_fixed_cmd_to_RSHFIQ(q_Temp);
    DPRINTF("Temp: "); print_RSHFIQ_User(blocking);   // Print our query results
    
    send_fixed_cmd_to_RSHFIQ(s_initPLL);  // Turn on the LO clock source  
    DPRINTLNF("Initializing PLL");
    //delay(1000);  // about 1-2 seconds needed before RS-HFIQ ready to receive commands over USB
    
    send_fixed_cmd_to_RSHFIQ(q_Analog_Read);
    DPRINTF("Analog Read: "); print_RSHFIQ_User(blocking);   // Print our query results
    
    send_fixed_cmd_to_RSHFIQ(q_BIT_freq);
    DPRINTF("Built-in Test Frequency: "); print_RSHFIQ_User(blocking);   // Print our query results
    
    send_fixed_cmd_to_RSHFIQ(q_clip_on);
    DPRINTF("Clipping (0 No Clipping, 1 Clipping): "); print_RSHFIQ_User(blocking);   // Print our query results

    send_variable_cmd_to_RSHFIQ(s_freq, convert_freq_to_Str(rs_freq));
    DPRINTF("Starting Frequency (Hz): "); DPRINTLN(convert_freq_to_Str(rs_freq));
    
    char d_offset[20];
    sprintf(d_offset,"%d",RSHFIQ_CAL_OFFSET);
    send_variable_cmd_to_RSHFIQ(s_F_Offset, d_offset);
    DPRINTF("Set F_Offset (Hz) to: "); DPRINTLN(RSHFIQ_CAL_OFFSET);  // Print our query result

    send_fixed_cmd_to_RSHFIQ(q_F_Offset);
    DPRINTF("F_Offset (Hz) Now: "); print_RSHFIQ_User(blocking);   // Print our query result

    send_fixed_cmd_to_RSHFIQ(q_freq);  // query the current frequency.
    DPRINTF("Reported Frequency (Hz): "); print_RSHFIQ_User(blocking);   // Print our query results
    
    DPRINTLNF("End of RS-HFIQ Setup");
    counter = 0;
}

// The RS-HFIQ has only 1 "VFO" so does not itself care about VFO A or B or split, or which is active
// However this is also the CAT interface and commands will come down for such things.  
// We need to act on the active VFO and pass back the info needed to the calling program.
uint64_t SDR_RS_HFIQ::cmd_console(uint8_t &_swap_vfo, uint64_t &_VFOA, uint64_t &_VFOB, uint8_t &_curr_band, uint8_t &_xmit, uint8_t &_split, uint8_t &_mode, uint8_t &_clip)  // returns new or unchanged active VFO value
{
    char c;
    //static unsigned char Ser_Flag = 0, Ser_NDX = 0;
    static uint8_t swap_vfo_last = 0;
    static uint8_t _ai = 0;  // track AI mode

    rs_freq = _VFOA;

    if (_xmit != 0)  // if in TX mode, check for clipping and set indicator
    {
        send_fixed_cmd_to_RSHFIQ(q_clip_on);
        //DPRINTF("Clipping (0 No Clipping, 1 Clipping): ");
        print_RSHFIQ_User(blocking);   // Read and if Debug is on print our query results
        _clip = (uint8_t) atoi(&R_Input[0]);
        //DPRINTF("_clip"); DPRINTLN(*_clip);
    }
    else 
        _clip = 0;

//   Test code.  Passes through all chars both directions.
/*
    while (CAT_port.available() > 0)    // Process any and all characters in the buffer
        userial.write(CAT_port.read());
    while (userial.available()) 
        CAT_port.write(userial.read());
    return 0;
*/
    while (CAT_port.available() > 0)    // Process any and all characters in the buffer
    {
        c = CAT_port.readBytesUntil(';',S_Input, 15); 
        
        if (c>0)
        {
            //CAT_port.print("Count=");
            //CAT_port.print(c,DEC);
            //CAT_port.printf("  %s\r\n",S_Input);

            #ifdef DBG 
            //DPRINTF("\nRS-HFIQ: Cmd String: "); DPRINTLN(S_Input);
            #endif
            if (!strncmp(S_Input, "ID", 2))
            {
                CAT_port.print("ID017;");
            } 
            else if (!strncmp(S_Input, "OM", 2))   // && c == 13)
            {
                CAT_port.print("OM ------------;");
            }
            else if (!strncmp(S_Input, "K2", 2) && strlen(S_Input) == 2)   // && c == 13)
            {
                CAT_port.print("K20;");
            }
            else if (!strncmp(S_Input, "K3", 2) && strlen(S_Input) == 2)   // && c == 13)
            {
                CAT_port.print("K30;");
            }
            else if (!strncmp(S_Input, "RVM", 3)&& strlen(S_Input) == 3)
            {
                CAT_port.print("RVM05.67;");
            }
            else if (!strncmp(S_Input, "RVD", 3)&& strlen(S_Input) == 3)
            {
                CAT_port.print("RVD02.88;");
            }
            else if (!strncmp(S_Input, "RVA", 3)&& strlen(S_Input) == 3)
            {
                CAT_port.print("RVA99.99;");
            }
            else if (!strncmp(S_Input, "RVR", 3)&& strlen(S_Input) == 3)
            {
                CAT_port.print("RVR99.99;");
            }
            else if (!strncmp(S_Input, "RVF", 3)&& strlen(S_Input) == 3)
            {
                CAT_port.print("RVF01.26;");
            }
            else if (!strncmp(S_Input, "BW", 2) && strlen(S_Input) == 2)
            {
                CAT_port.print("BW4000;");
            }
            else if (!strncmp(S_Input, "BW$", 3) && strlen(S_Input) == 3)
            {
                CAT_port.print("BW$4000;");
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
                DPRINTLNF("RS-HFIQ: Split Mode OFF"); 
                #endif
            }
            else if (!strncmp(S_Input, "FT1", 3)  && strlen(S_Input) == 3) // Split ON
            {
                _split = 1;
                #ifdef DBG  
                DPRINTLNF("RS-HFIQ: Split Mode ON VFOB=TX");
                #endif
            }
            else if (!strncmp(S_Input, "FT0", 3)  && strlen(S_Input) == 3) // Split ON
            {
                _split = 0;
                #ifdef DBG  
                DPRINTLNF("RS-HFIQ: Split Mode ON VFOA=TX");
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
            else if (!strncmp(S_Input, "KS", 2)) // Keyer Speed.  Fake it at 12wpm
            {
                CAT_port.printf("KS012;");
            }        
            else if ((!strncmp(S_Input, "MD$", 3) && (strlen(S_Input) == 3)) || (!strncmp(S_Input, "MD", 2) && (strlen(S_Input) == 2)))  // report Radio current Mode per K3 numbering
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

                if (!strncmp(S_Input, "MD$", 3))
                    CAT_port.printf("MD$%d;", _mode_);
                else
                    CAT_port.printf("MD%d;", _mode_);
            }
            else if ((!strncmp(S_Input, "MD", 2) && strlen(S_Input) == 3) || (!strncmp(S_Input, "MD$", 3) && (strlen(S_Input) == 4)))  // map incoming mode change request from K3 values to our mode numbering and return the value
            {
                uint8_t _mode_ = 0;

                if (S_Input[2] == '$')
                    _mode_ = S_Input[3];
                else
                 _mode_ = S_Input[2];

                switch (_mode_)
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
            else if (!strncmp(S_Input, "B?", 2))
            {
                // convert string to number  
                rs_freq = atoll(&S_Input[1]);   // skip the first letter 'B' and convert the number
                sprintf(freq_str, "%8llu", rs_freq);
                send_variable_cmd_to_RSHFIQ(s_BIT_freq, convert_freq_to_Str(rs_freq));
                #ifdef DBG
                //DPRINTLN(freq_str);
                DPRINT(F("RS-HFIQ: Set BIT Frequency (Hz): ")); DPRINTLN(convert_freq_to_Str(rs_freq));
                #endif
                print_RSHFIQ(0);
            }
            else if (!strncmp(S_Input, "D?", 2))
            {
                // This is an offset frequency, possibly used for dial calibration or perhaps RIT.
                send_variable_cmd_to_RSHFIQ(s_F_Offset, &S_Input[1]);
                #ifdef DBG
                DPRINT(F("Set Offset Frequency (Hz): ")); DPRINTLN(convert_freq_to_Str(rs_freq));
                DPRINT(F("RS-HFIQ: Set Offset Frequency (Hz): ")); DPRINTLN(&S_Input[1]);
                //DPRINTLN(freq_str);
                #endif
                delay(3);
                print_RSHFIQ(0);
            }
            else if (!strncmp(S_Input, "E?", 2))
            {
                // convert string to number  
                rs_freq = atoll(&S_Input[1]);   // skip the first letter 'E' and convert the number
                sprintf(freq_str, "%8llu", rs_freq);
                send_variable_cmd_to_RSHFIQ(s_EXT_freq, convert_freq_to_Str(rs_freq));
                #ifdef DBG  
                //DPRINTLN(freq_str);
                DPRINT(F("RS-HFIQ: Set External Frequency (Hz): ")); DPRINTLN(convert_freq_to_Str(rs_freq));
                #endif
                print_RSHFIQ(0);
            }
            else if (!strncmp(S_Input, "RX", 2))
            {
                #ifdef DBG  
                DPRINTLN(F("RS-HFIQ: RX->XMIT OFF"));
                #endif
                _xmit = 0;
            }
            else if (!strncmp(S_Input, "TX", 2))
            {
                #ifdef DBG  
                DPRINTLN(F("RS-HFIQ: TX->XMIT ON"));
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
                DPRINT(F("RS-HFIQ: Swap VFOs: ")); DPRINTLN(*_swap_vfo);
                #endif
            }           
            else if (!strncmp(S_Input, "F?", 2)) 
            {
                #ifdef DBG  
                DPRINT(F("RS_HFIQ Freq Query: ")); DPRINTLN(S_Input);
                #endif
                send_fixed_cmd_to_RSHFIQ(S_Input);  // Ask for current freq from radio hardware
                delay(5);
                print_RSHFIQ(1);
            }
            else if (S_Input[1] == '?' || S_Input[0] == '?' || S_Input[0] == 'W')
            {
                #ifdef DBG  
                DPRINT(F("RS_HFIQ Command: ")); DPRINTLN(S_Input);
                #endif
                send_fixed_cmd_to_RSHFIQ(S_Input);
                delay(5);
                print_RSHFIQ(0);  // do not print this for freq changes, causes a hang since there is no response and this is a blocking call 
                                // Use non blocking since user input could be in error and a response may not be returned
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

void SDR_RS_HFIQ::send_fixed_cmd_to_RSHFIQ(const char * str)
{
    userial.printf("*%s\r", str);
    delay(5);
}

void SDR_RS_HFIQ::send_variable_cmd_to_RSHFIQ(const char * str, char * cmd_str)
{
    userial.printf("%s%s\r", str, cmd_str);
    delay(5);
}

void SDR_RS_HFIQ::init_PLL(void)
{
  #ifdef DBG  
  DPRINTLN(F("init_PLL: Begin"));
  #endif
  send_fixed_cmd_to_RSHFIQ(s_initPLL);
  delay(2000);   // delay needed to turn on PLL clock0
  #ifdef DBG  
  DPRINTLN(F("init_PLL: End"));
  #endif
}

char * SDR_RS_HFIQ::convert_freq_to_Str(uint64_t rs_freq)
{
    sprintf(freq_str, "%llu", rs_freq);
    send_fixed_cmd_to_RSHFIQ(freq_str);
    return freq_str;
}

void SDR_RS_HFIQ::write_RSHFIQ(int ch)
{   
    userial.write(ch);
} 

int SDR_RS_HFIQ::read_RSHFIQ(int flag)
{
    char c = 0;
    unsigned char Ser_Flag = 0, Ser_NDX = 0;

    if (flag)  // we are waiting for a reply (BLOCKING)
        while (userial.available() == 0) {} // Wait for delayed reply   ToDo: put a timeout in here

    // Clear buffer for expected receive string
    if (Ser_Flag == 0)                  
    {
        Ser_NDX = 0;                   // character index = 0
        Ser_Flag = 1;                  // State 1 means we are collecting characters for processing
        for (int z = 0; z < 19; z++)  // Fill the buffer with spaces
        {
            R_Input[z] = ' ';
        }
    }

    // wait for and collect chars
    while (Ser_Flag == 1 && isAscii(c) && userial.available() > 0)
    {
        c = userial.read(); 
        c = toupper(c);
        #ifdef DBG  
        DPRINT(c);    
        #endif
        if (c != 13)
            R_Input[Ser_NDX++] = c;  // If we are in state 1 and the character isn't a <CR>, put it in the buffer
        if (Ser_NDX > 19)
        {
            Ser_Flag = 0; 
            Ser_NDX = 19;   // If we've received more than 15 characters without a <CR> just keep overwriting the last character
        }
        if (Ser_Flag == 1 && c == 13)                         // If it is a <CR> ...
        {
            c = userial.read(); 
            R_Input[Ser_NDX] = 0;                                // terminate the input string with a null (0)
            Ser_Flag = 0;                                        // Set state to 3 to indicate a command is ready for processing
            #ifdef DBG  
            DPRINT(F("Reply string = ")); DPRINTLN(R_Input);
            #endif
        }
    }
    return 1;
}

// flag = 0 do not Block
// flag = 1, block while waiting for a serial char
void SDR_RS_HFIQ::print_RSHFIQ(int flag)
{
    read_RSHFIQ(flag);
    CAT_port.println(R_Input);
    return;
}

// flag = 0 do not Block
// flag = 1, block while waiting for a serial char
void SDR_RS_HFIQ::print_RSHFIQ_User(int flag)
{
    read_RSHFIQ(flag);
    //#ifdef DBG
        DPRINTLN(R_Input);
    //#endif
    return;
}

bool SDR_RS_HFIQ::refresh_RSHFIQ(void)
{
    bool Proceed;

    Proceed = false;
    
    RSHFIQ.Task();
    
    // Print out information about different devices.
    for (uint8_t i = 0; i < CNT_DEVICES; i++) 
    {
        if (*drivers[i] != driver_active[i]) 
        {
            if (driver_active[i]) 
            {
                DEBUG_PRINTF("*** Device %s - disconnected ***\n", driver_names[i]);
                driver_active[i] = false;
                Proceed = false;
            } 
            else 
            {
                DEBUG_PRINTF("*** Device %s %x:%x - connected ***\n", driver_names[i], drivers[i]->idVendor(), drivers[i]->idProduct());
                driver_active[i] = true;
                Proceed = true;

                tft.setFont(Arial_14);
                tft.setTextColor(BLUE);
                
                const uint8_t *psz = drivers[i]->manufacturer();
                if (psz && *psz)
                {
                    tft.setTextColor(CYAN);
                    tft.setCursor(60, 360);
                    tft.printf("  Manufacturer: %s\n", psz);
                    DEBUG_PRINTF("  Manufacturer: %s\n", psz);
                }
                psz = drivers[i]->product();
                if (psz && *psz)
                {
                    tft.setTextColor(YELLOW);
                    tft.setCursor(60, 380);
                    tft.printf("  Product: %s\n", psz); 
                    DEBUG_PRINTF("  Product: %s\n", psz);
                }
                psz = drivers[i]->serialNumber();
                if (psz && *psz)
                {
                    tft.setTextColor(GREEN);
                    tft.setCursor(60, 400);
                    tft.printf("  Serial: %s\n", psz);
                    DEBUG_PRINTF("  Serial: %s\n", psz);
                }
                // If this is a new Serial device.
                if (drivers[i] == &userial) 
                {
                    // Lets try first outputting something to our USerial to see if it will go out...
                    userial.begin(baud);
                }
            }
        }
    }
    delay (500);
    return Proceed;
}
/*
// For RS-HFIQ free-form frequency entry validation but can be useful for external program CAT control such as a logger program.
// Changes to the correct band settings for the new target frequency.  
// The active VFO will become the new frequency, the other VFO will come from the database last used frequency for that band.
// If the new frequency is below or above the band limits it returns a value of 0 and skips any updates.
//
uint64_t SDR_RS_HFIQ::find_new_band(uint64_t new_frequency, uint8_t &_curr_band)
{
    int i;

    for (i=RS_BANDS-1; i>=0; i--)    // start at the top and look for first band that VFOA fits under bandmem[i].edge_upper
    {
        #ifdef DBG  
        DPRINT(F("RS-HFIQ: Edge_Lower Search = ")); DPRINTLN(rs_bandmem[i].edge_lower);
        #endif
        if (new_frequency >= rs_bandmem[i].edge_lower && new_frequency <= rs_bandmem[i].edge_upper)  // found a band lower than new_frequency so search has ended
        {
            #ifdef DBG  
            DPRINT(F("RS-HFIQ: Edge_Lower = ")); DPRINTLN(rs_bandmem[i].edge_lower);
            #endif
            *rs_curr_band = rs_bandmem[i].band_num;
            #ifdef DBG  
            DPRINT(F("RS-HFIQ: find_band(): New Band = ")); DPRINTLN(*rs_curr_band);
            #endif
            return new_frequency;
        }        
    }
    #ifdef DBG  
    DPRINTLN(F("RS-HFIQ: Invalid Frequency Requested - Out of RS-HFIQ Band"));
    #endif
    return 0;  // 0 means frequency was not found in the table
}
*/

#endif // USE_RS_HFIQ
