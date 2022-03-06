//***************************************************************************************************
//
//    SDR_RSHFIQ.INO
//    TEST program
//    USB host test program for RS-HFIQ transciever board
//    March 5, 2022 by K7MDL
//    Based on the Teensy 3.6/4.x USBHost.ino example and RS-HFIQ.ino code
//    Adds commands to test set and query for the RS-HFIQ transceiver via the Teensy 4.x USB Host serial port.
//
//    NOTE: Configure your terminal to send CR at end of line.  
//
//
//***************************************************************************************************
#include "USBHost_t36.h"
#define USBBAUD 57600   // RS-HFIQ uses 7600 baud
uint32_t baud = USBBAUD;
uint32_t format = USBHOST_SERIAL_8N1;
USBHost RSHFIQ;
USBHub hub1(RSHFIQ);
USBHub hub2(RSHFIQ);
USBHIDParser hid1(RSHFIQ);
USBHIDParser hid2(RSHFIQ);
USBHIDParser hid3(RSHFIQ);

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

bool Proceed  = false; 
int   counter = 0;
uint32_t freq = 5000000;
char freq_str[15] = "7074000";  // *Fxxxx command to set LO freq, PLL Clock 0
char * s_initPLL    = "*OF1";   // turns on LO clock0 output and sets drive current level to 4ma.
char * q_freq       = "*F?";    // returns current LO frequency
char * s_freq       = "*F";     // set LO frequency template.  3 to 30Mhz range
char * q_dev_name   = "*?";     // example "RSHFIQ"
char * q_ver_num    = "*W";     // example "RS-HFIQ FW 2.4a"
char * s_TX_OFF     = "*X0";    // Transmit OFF 
char * s_TX_ON      = "*X1";    // Transmit ON - power is controlled via audio input level
char * q_Temp       = "*T";     // Temp on board in degrees C
char * q_Analog_Read = "*L";    // analog read
char * q_EXT_freq   = "*E?";    // query the setting for PLL Clock 1 frequency presented on EX-RF jack or used for CW
//char * s_EXT_freq = "*EXXXXX";// sets PLL Clock 1.  4KHz to 225Mhz range
char * q_F_Offset   = "*D?";    // Query Offset added to LO, BIT, or EXT frequency
//char * s_F_Offset = "*DXXXXX";// Sets Offset to add to LO, BIT, or EXT frequency
char * q_clip_on    = "*C";     // clipping occuring, add external attenuation
char * q_BIT_freq   = "*B?";    // Built In Test. Uses PLL clock 2


// ************************************************* Setup *****************************************
//
// *************************************************************************************************
void setup()
{
    
    while (!Serial && (millis() < 5000)) ; // wait for Arduino Serial Monitor
    Serial.println("\n\nUSB Host Testing - Serial V0.1");
    RSHFIQ.begin();
    Serial.println("Waiting for RS-HFIQ device to register on USB Host port");
    while (!Proceed)  // observed about 500ms required.
    {
        refresh_RSHFIQ();   // wait until we have a valid USB 
        //Serial.print("Retry (500ms) = "); Serial.println(counter++);
        delay (500);
    }
    delay(1000);  // about 1-2 seconds needed before RS-HFIQ ready to receive commands over USB
    Serial.println("Start of RS-HFIQ Setup");
    send_fixed_cmd_to_RSHFIQ(q_dev_name); // get our device ID name
    Serial.print("Device Name: ");print_RSHFIQ(1);  // waits for serial available (BLOCKING call);
    
    send_fixed_cmd_to_RSHFIQ(q_ver_num);
    Serial.print("Version: ");print_RSHFIQ(1);  // waits for serial available (BLOCKING call);
    
    send_fixed_cmd_to_RSHFIQ(q_F_Offset);
    wait_reply();  // extra wait for serial data step at startup.
    Serial.print("F_Offset (Hz): "); print_RSHFIQ(1);   // Print our query results

    send_variable_cmd_to_RSHFIQ(s_freq, convert_freq_to_Str(freq));  //Inserted here to help reliably set up PLL
    
    send_fixed_cmd_to_RSHFIQ(q_Analog_Read);
    Serial.print("Analog Read: "); print_RSHFIQ(1);   // Print our query results
    
    send_fixed_cmd_to_RSHFIQ(q_Temp);
    Serial.print("Temp: "); print_RSHFIQ(1);   // Print our query results
    
    send_fixed_cmd_to_RSHFIQ(q_BIT_freq);
    Serial.print("Built-in Test Frequency: "); print_RSHFIQ(1);   // Print our query results
    
    send_fixed_cmd_to_RSHFIQ(q_clip_on);
    Serial.print("Clipping (0 No Clipping, 1 Clipping): "); print_RSHFIQ(1);   // Print our query results
    
    send_fixed_cmd_to_RSHFIQ(s_initPLL);  // Turn on the LO clock source  
    Serial.println("Initializing PLL");
    
    send_variable_cmd_to_RSHFIQ(s_freq, convert_freq_to_Str(freq));
    Serial.print("Starting Frequency (Hz): "); Serial.println(freq);

    send_fixed_cmd_to_RSHFIQ(q_freq);  // query the current frequency.
    Serial.print("Reported Frequency (Hz): "); print_RSHFIQ(1);   // Print our query results
    delay(1000);
    
    send_fixed_cmd_to_RSHFIQ(s_initPLL);  // Extra one to deal with occasional init fails
    Serial.println("End of Setup");
    
    counter = 0;
    disp_Menu();
}

// ********************************************Loop ******************************************

void loop()
{  
  cmd_Console();
  
  //Serial.print("loop counter = "); Serial.println(counter++);
  //delay(1000);
}

void cmd_Console(void)
{
    char c;
    unsigned char Ser_Flag = 0, Ser_NDX = 0;
    char S_Input[16];

    while (Serial.available() > 0)    // Process any and all characters in the buffer
    {
        c = Serial.read(); 
        c = toupper(c);
        if (c == '*')                   // No matter where we are in the state machine, a '*' say clear the buffer and start over.
        {
            Ser_NDX = 0;                   // character index = 0
            Ser_Flag = 1;                  // State 1 means the '*' has been received and we are collecting characters for processing
            for (int z = 0; z < 16; z++)  // Fill the buffer with spaces
            {
                S_Input[z] = ' ';
            }
            Serial.println("Start cmd string");
        }
        else 
        {
          if (Ser_Flag == 1 && c != 13) S_Input[Ser_NDX++] = c;  // If we are in state 1 and the character isn't a <CR>, put it in the buffer
          if (Ser_Flag == 1 && c == 13)                         // If it is a <CR> ...
          {
              S_Input[Ser_NDX] = 0;                                // terminate the input string with a null (0)
              Ser_Flag = 3;                                        // Set state to 3 to indicate a command is ready for processing
              Serial.println("Cmd string complete");
          }
          if (Ser_NDX > 15) Ser_NDX = 15;   // If we've received more than 15 characters without a <CR> just keep overwriting the last character
        }
        if (S_Input[0] != '*' and  Ser_Flag == 0 && Ser_NDX < 2)  // process menu pick list
        {
            int32_t fr_adj = 0;     

            Ser_NDX = 0;

            switch (c)
            {
                case '1': Serial.println("TX OFF"); send_fixed_cmd_to_RSHFIQ(s_TX_OFF); break; 
                case '2': Serial.println("TX ON"); send_fixed_cmd_to_RSHFIQ(s_TX_ON); break; 
                case '3': Serial.println("Query Frequency"); send_fixed_cmd_to_RSHFIQ(q_freq); print_RSHFIQ(1); break;
                case '4': Serial.println("Query Frequency Offset"); send_fixed_cmd_to_RSHFIQ(q_F_Offset); print_RSHFIQ(1); break;
                case '5': Serial.println("Query Built-in Test Frequency"); send_fixed_cmd_to_RSHFIQ(q_BIT_freq); print_RSHFIQ(1); break;
                case '6': Serial.println("Query Analog Read"); send_fixed_cmd_to_RSHFIQ(q_Analog_Read); print_RSHFIQ(1); break;
                case '7': Serial.println("Query Ext Frequency or CW"); send_fixed_cmd_to_RSHFIQ(q_EXT_freq); print_RSHFIQ(1); break;
                case '8': Serial.println("Query Temp"); send_fixed_cmd_to_RSHFIQ(q_Temp); print_RSHFIQ(1); break;
                case '9': Serial.println("Initialize PLL Clock0 (LO)"); send_fixed_cmd_to_RSHFIQ(s_initPLL); break;
                case '0': Serial.println("Query Clipping"); send_fixed_cmd_to_RSHFIQ(q_clip_on); print_RSHFIQ(1); break;
                case 'K': Serial.println("Move Down 1000Hz "); fr_adj = -1000; break;
                case 'L': Serial.println("Move Up 1000Hz "); fr_adj = 1000; break;
                case '<': Serial.println("Move Down 100Hz "); fr_adj = -100; break;
                case '>': Serial.println("Move Up 100Hz "); fr_adj = 100; break;  
                case ',': Serial.println("Move Down 10Hz "); fr_adj = -100; break;
                case '.': Serial.println("Move Up 10Hz "); fr_adj = 100; break; 
                case 'h':
                case 'H': disp_Menu(); break;
                default: Serial.write(c); userial.write(c); break;
                break;  
            }
            if (fr_adj != 0)  // Move up or down
            {
                freq += fr_adj;
                Serial.print("Target Freq = "); Serial.println(freq);
                send_variable_cmd_to_RSHFIQ(s_freq, convert_freq_to_Str(freq));
                fr_adj = 0;
            } 
        }
    }

    // If a complete command is received, process it
    if (Ser_Flag == 3) 
    {
        Serial.print("Send Cmd String : *");Serial.println(S_Input);
        if (S_Input[0] == 'F' && S_Input[1] != '?')
        {
            // convert string to number  
            freq = atoi(&S_Input[1]);   // skip the first letter 'F' and convert the number
        }
        send_fixed_cmd_to_RSHFIQ(S_Input);
        Ser_Flag = 0;
        print_RSHFIQ(0);
    }
}

bool CompareStrings(const char *sz1, const char *sz2) {
  while (*sz2 != 0) {
    if (toupper(*sz1) != toupper(*sz2)) 
      return false;
    sz1++;
    sz2++;
  }
  return true; // end of string so show as match
}

void send_fixed_cmd_to_RSHFIQ(const char * str)
{
    userial.printf("*%s\r", str);
}

void send_variable_cmd_to_RSHFIQ(const char * str, char * cmd_str)
{
    userial.printf("%s%s\r", str, cmd_str);
}

void init_PLL(void)
{
  Serial.println("init_PLL: Begin");
  send_fixed_cmd_to_RSHFIQ(s_initPLL);
  delay(2000);   // delay needed to turn on PLL clock0
  Serial.println("init_PLL: End");
}

char * convert_freq_to_Str(uint32_t freq)
{
  sprintf(freq_str, "%lu", freq);
  send_fixed_cmd_to_RSHFIQ(freq_str);
  return freq_str;
}

void write_RSHFIQ(int ch)
{   
    userial.write(ch);
} 

int read_RSHFIQ(void)
{
    while (userial.available()) 
    {
        //Serial.println("USerial Available");
        return userial.read();
    }
    return 0;
}

void wait_reply(void)
{
    while (!userial.available());
}

// flag = 0 do not Block
// flag = 1, block while waiting for a serial char
void print_RSHFIQ(int flag)
{
  if (flag) wait_reply();  // we are waiting for a reply (BLOCKING)
  //Serial.print("print_RSHFIQ: ");
  while ( int c = read_RSHFIQ())
  {
    Serial.write(c);
  }
}

void disp_Menu(void)
{
    Serial.println("\n\n ***** Control Menu *****\n");
    Serial.println(" 1 - TX OFF");
    Serial.println(" 2 - TX ON");
    Serial.println(" 3 - Query Frequency");
    Serial.println(" 4 - Query Frequency Offset");
    Serial.println(" 5 - Query Built-in Test Frequency");
    Serial.println(" 6 - Query Analog Read");
    Serial.println(" 7 - Query External Frequency or CW");
    Serial.println(" 8 - Query Temp");
    Serial.println(" 9 - Initialize PLL Clock0 (LO)");
    Serial.println(" 0 - Query Clipping");
    
    Serial.println("\n Can use upper or lower case letters");
    Serial.println(" Can use multiple frequency shift actions such as ");
    Serial.println("   <<<< for 4*100Hz  to move down 400Hz");
    Serial.println(" K - Move Down 1000Hz ");
    Serial.println(" L - Move Up 1000Hz ");
    Serial.println(" < - Move Down 100Hz ");
    Serial.println(" > - Move Up 100Hz ");
    Serial.println(" , - Move Down 10Hz ");
    Serial.println(" . - Move Up 10Hz ");

    Serial.println("\n Enter any native command string");
    Serial.println("    Example: *F5000000 will change frequency to WWV at 5MHz");
    Serial.println("    Minimum is 3.0MHz or *F3000000, max is 30.0MHz or *F30000000");
    Serial.println("    Band filters and attenuation are automatically set");
    Serial.println("\n ***** End of Menu *****");
}

void refresh_RSHFIQ(void)
{
    RSHFIQ.Task();
    // Print out information about different devices.
    for (uint8_t i = 0; i < CNT_DEVICES; i++) 
    {
        if (*drivers[i] != driver_active[i]) 
        {
            if (driver_active[i]) 
            {
                Serial.printf("*** Device %s - disconnected ***\n", driver_names[i]);
                driver_active[i] = false;
                Proceed = false;
            } 
            else 
            {
                Serial.printf("*** Device %s %x:%x - connected ***\n", driver_names[i], drivers[i]->idVendor(), drivers[i]->idProduct());
                driver_active[i] = true;
                Proceed = true;

                const uint8_t *psz = drivers[i]->manufacturer();
                if (psz && *psz) Serial.printf("  manufacturer: %s\n", psz);
                psz = drivers[i]->product();
                if (psz && *psz) Serial.printf("  product: %s\n", psz);
                psz = drivers[i]->serialNumber();
                if (psz && *psz) Serial.printf("  Serial: %s\n", psz);

                // If this is a new Serial device.
                if (drivers[i] == &userial) 
                {
                    // Lets try first outputting something to our USerial to see if it will go out...
                    userial.begin(baud);
                }
            }
        }
    }
}
