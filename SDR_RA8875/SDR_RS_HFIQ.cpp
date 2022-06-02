//***************************************************************************************************
//
//      SDR_RS_HFIQ.cpp 
//
//      June 1, 2022 by K7MDL
//
//      USB host control for RS-HFIQ 5W transciever board
//      USB Host comms portion from the Teensy4 USBHost_t36 example program
//      Requires Teensy 4.0 or 4.1 with USB Host port connector.
//
//      Accepts expanded set of CAT port commands such as *FA, *FB, *SW0, etc. fo OmniRig control
//          and passes back these controls to the main program.  
//      A Custom rig file is part of the GitHub project files for the Teensy SDR project under user K7MDL2
//          at https://github.com/K7MDL2/KEITHSDR/tree/main/SDR_RA8875/RS-HFIQ%20Omni-RIg
//
//      Placed in the Public Domain
//
//***************************************************************************************************

#include <Arduino.h>
#include <USBHost_t36.h>
#include <SDR_RS_HFIQ.h>

//#define DEBUG_RSHFIQ  //set to true for debug output, false for no debug output
#ifdef DEBUG_RSHFIQ
#define DSERIALBEGIN(...)    Serial.begin(__VA_ARGS__)
#define DPRINTLN(...)       Serial.println(__VA_ARGS__)
#define DPRINT(...)         Serial.print(__VA_ARGS__)
#define DRINTF(...)         Serial.print(F(__VA_ARGS__))
#define DPRINTLNF(...)      Serial.println(F(__VA_ARGS__)) //printing text using the F macro
#define DELAY(...)          delay(__VA_ARGS__)
#define PINMODE(...)        pinMode(__VA_ARGS__)
#define TOGGLEd13           PINB = 0x20                    //UNO's pin D13
#define DEBUG_PRINT(...)    Serial.print(F(#__VA_ARGS__" = ")); Serial.print(__VA_ARGS__); Serial.print(F(" ")) 
#define DEBUG_PRINTLN(...)  DEBUG_PRINT(__VA_ARGS__); Serial.println()
#define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
#define DSERIALBEGIN(...)
#define DPRINTLN(...)
#define DPRINT(...)
#define DPRINTF(...)      
#define DPRINTLNF(...)    
#define DELAY(...)        
#define PINMODE(...)      
#define TOGGLEd13      
#define DEBUG_PRINT(...)    
#define DEBUG_PRINTLN(...) 
#define DEBUG_PRINTF(...) 
#endif

// Serial port for external CAT control
//#define CAT_RS_Serial SerialUSB1
#define CAT_RS_Serial Serial

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

static uint32_t rs_freq;
int  counter  = 0;
int  blocking = 0;  // 0 means do not wait for serial response from RS-HFIQ - for testing only.  1 is normal
static char S_Input[16];
static char R_Input[20];

#define RS_BANDS    9
struct RS_Band_Memory {
    uint8_t     band_num;        // Assigned bandnum for compat with external program tables
    char        band_name[10];  // Friendly name or label.  Default here but can be changed by user.  Not actually used by code.
    uint32_t    edge_lower;     // band edge limits for TX and for when to change to next band when tuning up or down.
    uint32_t    edge_upper;
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

// ************************************************* Setup *****************************************
//
// *************************************************************************************************
void SDR_RS_HFIQ::setup_RSHFIQ(int _blocking, uint32_t VFO)  // 0 non block, 1 blocking
{   
    CAT_RS_Serial.begin(115200);
    delay(100);
    DPRINTLN("\nStart of RS-HFIQ Setup"); 
    rs_freq = VFO;
    blocking = _blocking;
    //DPRINTLN(F("Looking for USB Host Connection to RS-HFIQ"));
    RSHFIQ.begin();
    delay(2000);
    DPRINTLN(F("Waiting for RS-HFIQ device to register on USB Host port"));
    while (!refresh_RSHFIQ())  // observed about 500ms required.
    {        
        if (!blocking) break;
        // wait until we have a valid USB 
        DPRINT(F("Retry RS-HFIQ USB connection (~500ms) = ")); DPRINTLN(counter++);
    }
    delay(2000);  // about 1-2 seconds needed before RS-HFIQ ready to receive commands over USB
    
    while (userial.available() > 0)  // Clear out RX channel garbage if any
    {
        DPRINTLN(userial.read());
    }

    send_fixed_cmd_to_RSHFIQ(q_dev_name); // get our device ID name
    DPRINT(F("Device Name: ")); print_RSHFIQ_User(blocking);  // waits for serial available (BLOCKING call);
    
    send_fixed_cmd_to_RSHFIQ(q_ver_num);
    DPRINT(F("Version: ")); print_RSHFIQ_User(blocking);  // waits for serial available (BLOCKING call);

    send_fixed_cmd_to_RSHFIQ(q_Temp);
    DPRINT(F("Temp: ")); print_RSHFIQ_User(blocking);   // Print our query results
    
    send_fixed_cmd_to_RSHFIQ(s_initPLL);  // Turn on the LO clock source  
    DPRINTLN(F("Initializing PLL"));
    //delay(1000);  // about 1-2 seconds needed before RS-HFIQ ready to receive commands over USB
    
    send_fixed_cmd_to_RSHFIQ(q_Analog_Read);
    DPRINT(F("Analog Read: ")); print_RSHFIQ_User(blocking);   // Print our query results
    
    send_fixed_cmd_to_RSHFIQ(q_BIT_freq);
    DPRINT(F("Built-in Test Frequency: ")); print_RSHFIQ_User(blocking);   // Print our query results
    
    send_fixed_cmd_to_RSHFIQ(q_clip_on);
    DPRINT(F("Clipping (0 No Clipping, 1 Clipping): ")); print_RSHFIQ_User(blocking);   // Print our query results

    send_variable_cmd_to_RSHFIQ(s_freq, convert_freq_to_Str(rs_freq));
    DPRINT(F("Starting Frequency (Hz): ")); DPRINTLN(convert_freq_to_Str(rs_freq));

    send_fixed_cmd_to_RSHFIQ(q_F_Offset);
    DPRINT(F("F_Offset (Hz): ")); print_RSHFIQ_User(blocking);   // Print our query result

    send_fixed_cmd_to_RSHFIQ(q_freq);  // query the current frequency.
    DPRINT(F("Reported Frequency (Hz): ")); print_RSHFIQ_User(blocking);   // Print our query results
    
    DPRINTLN(F("End of RS-HFIQ Setup"));
    counter = 0;
    disp_Menu();
}

// The RS-HFIQ has only 1 "VFO" so does not itself care about VFO A or B or split, or which is active
// However this is also the CAT interface and commands will come down for such things.  
// We need to act on the active VFO and pass back the info needed to the calling program.
uint32_t SDR_RS_HFIQ::cmd_console(uint8_t * swap_vfo, uint32_t * VFOA, uint32_t * VFOB, uint8_t * rs_curr_band, uint8_t * xmit, uint8_t * split)  // returns new or unchanged active VFO value
{
    char c;
    static unsigned char Ser_Flag = 0, Ser_NDX = 0;
    static uint8_t swap_vfo_last = 0;

    //if (active_vfo)
        rs_freq = *VFOA;
    //else
    //    rs_freq = *VFOB;

//   Test code.  Passes through all chars both directions.
/*
    while (CAT_RS_Serial.available() > 0)    // Process any and all characters in the buffer
        userial.write(CAT_RS_Serial.read());
    while (userial.available()) 
        CAT_RS_Serial.write(userial.read());
    return 0;
*/
    while (CAT_RS_Serial.available() > 0)    // Process any and all characters in the buffer
    {
        c = CAT_RS_Serial.read(); 
        c = toupper(c);
        //CAT_RS_Serial.print(c);
    
        if (c == '*')                   // No matter where we are in the state machine, a '*' say clear the buffer and start over.
        {
            Ser_NDX = 0;                   // character index = 0
            Ser_Flag = 1;                  // State 1 means the '*' has been received and we are collecting characters for processing
            for (int z = 0; z < 16; z++)  // Fill the buffer with spaces
            {
                S_Input[z] = ' ';
            }
            //CAT_RS_Serial.println(F("Start Cmd string "));
        } 
        else if (Ser_Flag == 1 && c != 13 && c!= 10) 
            S_Input[Ser_NDX++] = c;  // If we are in state 1 and the character isn't a <CR>, put it in the buffer
        else if (Ser_Flag == 1 && (c == 13 || c==10))                         // If it is a <CR> ...
        {
            S_Input[Ser_NDX] = 0;                                // terminate the input string with a null (0)
            Ser_Flag = 3;                                        // Set state to 3 to indicate a command is ready for processing
            //CAT_RS_Serial.println(S_Input);
        }
        
        if (Ser_NDX > 15) 
            Ser_NDX = 15;   // If we've received more than 15 characters without a <CR> just keep overwriting the last character
/*        
        //if (S_Input[0] != '*' and  Ser_Flag == 0 && Ser_NDX < 2)  // process menu pick list
        if (S_Input[0] == '*' and Ser_Flag == 3 && Ser_NDX < 2)  // process menu pick list
        {
            int32_t fr_adj = 0;     

            Ser_NDX = 0;

            switch (S_Input[1])
            {
                case '1': DPRINTLN(F("TX OFF")); send_fixed_cmd_to_RSHFIQ(s_TX_OFF); break; 
                case '2': DPRINTLN(F("TX ON")); send_fixed_cmd_to_RSHFIQ(s_TX_ON); break; 
                case '3': DPRINT(F("Query Frequency: ")); send_fixed_cmd_to_RSHFIQ(q_freq); print_RSHFIQ(blocking); break;
                case '4': DPRINT(F("Query Frequency Offset: ")); send_fixed_cmd_to_RSHFIQ(q_F_Offset); print_RSHFIQ(blocking); break;
                case '5': DPRINT(F("Query Built-in Test Frequency: ")); send_fixed_cmd_to_RSHFIQ(q_BIT_freq); print_RSHFIQ(blocking); break;
                case '6': DPRINT(F("Query Analog Read: ")); send_fixed_cmd_to_RSHFIQ(q_Analog_Read); print_RSHFIQ(blocking); break;
                case '7': DPRINT(F("Query Ext Frequency or CW: ")); send_fixed_cmd_to_RSHFIQ(q_EXT_freq); print_RSHFIQ(blocking); break;
                case '8': DPRINT(F("Query Temp: ")); send_fixed_cmd_to_RSHFIQ(q_Temp); print_RSHFIQ(blocking); break;
                case '9': DPRINTLN(F("Initialize PLL Clock0 (LO)")); send_fixed_cmd_to_RSHFIQ(s_initPLL); break;
                case '0': DPRINT(F("Query Clipping: ")); send_fixed_cmd_to_RSHFIQ(q_clip_on); print_RSHFIQ(blocking); break;
                case '?': DPRINT(F("Query Device Name: ")); send_fixed_cmd_to_RSHFIQ(q_dev_name); print_RSHFIQ(blocking); break;
                case 'K': DPRINTLN(F("Move Down 1000Hz ")); fr_adj = -1000; break;
                case 'L': DPRINTLN(F("Move Up 1000Hz ")); fr_adj = 1000; break;
                case '<': DPRINTLN(F("Move Down 100Hz ")); fr_adj = -100; break;
                case '>': DPRINTLN(F("Move Up 100Hz ")); fr_adj = 100; break;  
                case ',': DPRINTLN(F("Move Down 10Hz ")); fr_adj = -10; break;
                case '.': DPRINTLN(F("Move Up 10Hz ")); fr_adj = 10; break; 
                case 'R': disp_Menu(); break;
                default: DBG_Serial.write(c); userial.write(c); break;
                break;  
            }
            if (fr_adj != 0)  // Move up or down
            {
                rs_freq += fr_adj;
                DPRINT(F("RS-HFIQ: Target Freq = ")); DPRINTLN(rs_freq);
                send_variable_cmd_to_RSHFIQ(s_freq, convert_freq_to_Str(rs_freq));
                fr_adj = 0;
                return rs_freq;
            } 
        }
*/
    }

    // If a complete command is received, process it
    if (Ser_Flag == 3) 
    {
        #ifdef DBG 
        DPRINT(F("RS-HFIQ: Cmd String : *")); DPRINTLN(S_Input);
        #endif
        if (S_Input[0] == 'F' && (S_Input[1] != '?' && S_Input[2] != '?' && S_Input[1] != 'R'))
        {
            if (S_Input[1] == 'A')
            {
                rs_freq = atoi(&S_Input[2]);   // skip the first letter 'F' and convert the number   
                #ifdef DBG  
                DPRINT("RS-HFIQ: VFOA# = "); DPRINTLN(rs_freq);
                #endif
                sprintf(freq_str, "%8s", &S_Input[2]);
                rs_freq = find_new_band(rs_freq, rs_curr_band);  // set the correct index and changeBands() to match for possible band change
                if (rs_freq)
                {
                    *VFOA = rs_freq;
                    //send_variable_cmd_to_RSHFIQ(s_freq, freq_str);   
                    #ifdef DBG  
                    DPRINT("RS-HFIQ: VFOA = "); DPRINTLN(freq_str);
                    #endif
                    Ser_Flag = 0;
                    S_Input[0] = '\0';
                    return rs_freq; 
                }
            }
            else if (S_Input[1] == 'B')
            {
                rs_freq = atoi(&S_Input[2]);   // skip the first letter 'F' and convert the number   
                sprintf(freq_str, "%8s", &S_Input[2]);
                rs_freq = find_new_band(rs_freq, rs_curr_band);  // set the correct index and changeBands() to match for possible band change
                if (rs_freq)
                {
                    *VFOB = rs_freq;
                    //send_variable_cmd_to_RSHFIQ(s_freq, freq_str);   
                    #ifdef DBG  
                    DPRINT("RS-HFIQ: VFOB = "); DPRINTLN(freq_str);
                    #endif
                    Ser_Flag = 0;
                    S_Input[0] = '\0';
                    return rs_freq; 
                }
            }
            else 
            { // convert string to number and update the rs_freq variable
                rs_freq = atoi(&S_Input[1]);   // skip the first letter 'F' and convert the number   
                sprintf(freq_str, "%8s", &S_Input[1]);
                rs_freq = find_new_band(rs_freq, rs_curr_band);  // set the correct index and changeBands() to match for possible band change
                if (rs_freq)
                {
                    *VFOA = rs_freq;
                    //send_variable_cmd_to_RSHFIQ(s_freq, freq_str);   
                    #ifdef DBG  
                    DPRINT("RS-HFIQ: Active VFO = "); DPRINTLN(freq_str);
                    #endif
                    Ser_Flag = 0;
                    S_Input[0] = '\0';
                    return rs_freq; 
                }
            }
            //send_variable_cmd_to_RSHFIQ(s_freq, freq_str);   
            //DPRINT(F("RS_HFIQ Frequency Change before Lookup: ")); DPRINTLN(freq_str);
            //rs_freq = find_new_band(rs_freq, rs_curr_band);  // set the correct index and changeBands() to match for possible band change
            #ifdef DBG  
            DPRINT(F("RS_HFIQ Frequency Change Freq: ")); DPRINTLN(freq_str);
            DPRINT(F("RS_HFIQ Frequency Change Band: ")); DPRINTLN(*rs_curr_band);
            #endif
            if (rs_freq == 0)
            {
                #ifdef DBG  
                DPRINT(F("RS-HFIQ: Invalid Frequency = ")); DPRINTLN(S_Input);
                #endif
                Ser_Flag = 0;
                return rs_freq;
            }
            Ser_Flag = 0;
            S_Input[0] = '\0';
            return rs_freq;        
        }    
        if (S_Input[0] == 'B' && S_Input[1] != '?')
        {
            // convert string to number  
            rs_freq = atoi(&S_Input[1]);   // skip the first letter 'B' and convert the number
            sprintf(freq_str, "%8lu", rs_freq);
            send_variable_cmd_to_RSHFIQ(s_BIT_freq, convert_freq_to_Str(rs_freq));
            #ifdef DBG
            //DPRINTLN(freq_str);
            DPRINT(F("RS-HFIQ: Set BIT Frequency (Hz): ")); DPRINTLN(convert_freq_to_Str(rs_freq));
            #endif
            print_RSHFIQ(0);
        }
        if (S_Input[0] == 'D' && S_Input[1] != '?')
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
        if (S_Input[0] == 'E' && S_Input[1] != '?')
        {
            // convert string to number  
            rs_freq = atoi(&S_Input[1]);   // skip the first letter 'E' and convert the number
            sprintf(freq_str, "%8lu", rs_freq);
            send_variable_cmd_to_RSHFIQ(s_EXT_freq, convert_freq_to_Str(rs_freq));
            #ifdef DBG  
            //DPRINTLN(freq_str);
            DPRINT(F("RS-HFIQ: Set External Frequency (Hz): ")); DPRINTLN(convert_freq_to_Str(rs_freq));
            #endif
            print_RSHFIQ(0);
        }
        if (!strcmp(S_Input, "X0"))
        {
            #ifdef DBG  
            DPRINTLN(F("RS-HFIQ: XMIT OFF"));
            #endif
            *xmit = 0;
        }
        if (!strcmp(S_Input, "X1"))
        {
            #ifdef DBG  
            DPRINTLN(F("RS-HFIQ: XMIT ON"));
            #endif
            *xmit = 1;
        }
        if (!strcmp(S_Input, "SW0"))
        {
            if (swap_vfo_last)
                swap_vfo_last = 0;
            else 
                swap_vfo_last = 1;
            *swap_vfo = swap_vfo_last;
            #ifdef DBG  
            DPRINT(F("RS-HFIQ: Swap VFOs: ")); DPRINTLN(*swap_vfo);
            #endif
        }
        if (!strcmp(S_Input, "FR0")) // Split OFF
        {
            *split = 0;
            #ifdef DBG  
            DPRINTLN(F("RS-HFIQ: Split Mode OFF")); 
            #endif
        }
        if (!strcmp(S_Input, "FR1")) // Split ON
        {
            *split = 1;
            #ifdef DBG  
            DPRINTLN(F("RS-HFIQ: Split Mode ON"));
            #endif
        }
        // must be a query
        if (!strcmp(S_Input, "FA?")) // 
        {
            #ifdef DBG  
            DPRINT(F("RS-HFIQ: VFO A Query - Reply: ")); DPRINTLN(*VFOA);
            #endif
            sprintf(freq_str, "*FA%09lu", *VFOA);
            #ifdef DBG  
            DPRINTLN(freq_str);
            #endif
            CAT_RS_Serial.print(freq_str);
        }
        if (!strcmp(S_Input, "FB?")) // 
        {
            #ifdef DBG  
            DPRINT(F("RS-HFIQ: VFO B Query - Reply: ")); DPRINTLN(*VFOB);
            #endif
            sprintf(freq_str, "*FB%09lu", *VFOB);
            #ifdef DBG  
            DPRINTLN(freq_str);
            #endif
            CAT_RS_Serial.print(freq_str);
        }
        if (!strcmp(S_Input, "F?")) 
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
        else if (Ser_NDX == 0)
        {
            userial.printf("*\r");
            delay(5);
            read_RSHFIQ();
            #ifdef DBG  
            DPRINT(F("RS_HFIQ * Query Answer: ")); DPRINTLN(R_Input);
            #endif
            CAT_RS_Serial.print(R_Input);
        }
        S_Input[0] = '\0';
        for (int z = 0; z < 16; z++)  // Fill the buffer with spaces
        {
            S_Input[z] = ' ';
        }
        Ser_Flag = 0;
    }
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

char * SDR_RS_HFIQ::convert_freq_to_Str(uint32_t rs_freq)
{
    sprintf(freq_str, "%lu", rs_freq);
    send_fixed_cmd_to_RSHFIQ(freq_str);
    return freq_str;
}

void SDR_RS_HFIQ::write_RSHFIQ(int ch)
{   
    userial.write(ch);
} 

int SDR_RS_HFIQ::read_RSHFIQ(void)
{
    char c = 0;
    unsigned char Ser_Flag = 0, Ser_NDX = 0;

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
    if (flag)  // we are waiting for a reply (BLOCKING)
        while (userial.available() == 0) {} // Wait for delayed reply   ToDo: put a timeout in here
    read_RSHFIQ();
    CAT_RS_Serial.println(R_Input);
    return;
}

// flag = 0 do not Block
// flag = 1, block while waiting for a serial char
void SDR_RS_HFIQ::print_RSHFIQ_User(int flag)
{
    if (flag)  // we are waiting for a reply (BLOCKING)
        while (userial.available() == 0) {} // Wait for delayed reply   ToDo: put a timeout in here
    read_RSHFIQ();
    DPRINTLN(R_Input);
    return;
}

void SDR_RS_HFIQ::disp_Menu(void)
{
    DPRINTLN(F("\n\n ***** Control Menu *****\n"));
    DPRINTLN(F(" H - Main Menu"));
    DPRINTLN(F(" 1 - TX OFF"));
    DPRINTLN(F(" 2 - TX ON"));
    DPRINTLN(F(" 3 - Query Frequency"));
    DPRINTLN(F(" 4 - Query Frequency Offset"));
    DPRINTLN(F(" 5 - Query Built-in Test Frequency"));
    DPRINTLN(F(" 6 - Query Analog Read"));
    DPRINTLN(F(" 7 - Query External Frequency or CW"));
    DPRINTLN(F(" 8 - Query Temp"));
    DPRINTLN(F(" 9 - Initialize PLL Clock0 (LO)"));
    DPRINTLN(F(" 0 - Query Clipping"));
    DPRINTLN(F(" ? - Query Device Name"));
    
    DPRINTLN(F("\n Can use upper or lower case letters"));
    DPRINTLN(F(" Can use multiple frequency shift actions such as "));
    DPRINTLN(F("   <<<< for 4*100Hz  to move down 400Hz"));
    DPRINTLN(F(" K - Move Down 1000Hz "));
    DPRINTLN(F(" L - Move Up 1000Hz "));
    DPRINTLN(F(" < - Move Down 100Hz "));
    DPRINTLN(F(" > - Move Up 100Hz "));
    DPRINTLN(F(" , - Move Down 10Hz "));
    DPRINTLN(F(" . - Move Up 10Hz "));

    DPRINTLN(F("\n Enter any native command string"));
    DPRINTLN(F("    Example: *F5000000 will change frequency to WWV at 5MHz"));
    DPRINTLN(F("    Minimum is 3.0MHz or *F3000000, max is 30.0MHz or *F30000000"));
    DPRINTLN(F("    Band filters and attenuation are automatically set"));
    DPRINTLN(F("\n ***** End of Menu *****"));
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

                const uint8_t *psz = drivers[i]->manufacturer();
                if (psz && *psz) DEBUG_PRINTF("  manufacturer: %s\n", psz);
                psz = drivers[i]->product();
                if (psz && *psz) DEBUG_PRINTF("  product: %s\n", psz);
                psz = drivers[i]->serialNumber();
                if (psz && *psz) DEBUG_PRINTF("  Serial: %s\n", psz);

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

// For RS-HFIQ free-form frequency entry validation but can be useful for external program CAT control such as a logger program.
// Changes to the correct band settings for the new target frequency.  
// The active VFO will become the new frequency, the other VFO will come from the database last used frequency for that band.
// If the new frequency is below or above the band limits it returns a value of 0 and skips any updates.
//
uint32_t SDR_RS_HFIQ::find_new_band(uint32_t new_frequency, uint8_t * rs_curr_band)
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

