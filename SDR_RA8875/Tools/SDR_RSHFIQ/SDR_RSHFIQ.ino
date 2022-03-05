//***************************************************************************************************
//
//    SDR_RSHFIQ.INO
//    TEST program
//    USB host test program for RS-HFIQ transciever board
//    March 5, 2022 by K7MDL
//    Based on the Teensy 3.6/4.x USBHost.ino example, adds commands to
//    test set and query for the RS-HFIQ transceiver via the USB Host serial port.
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
char freq[12] = "7074000";  // *Fxxxx command to set LO freq
char * s_initPLL    = "*OF2\r"; // turns on LO clock0 output and sets drive current level to 4ma.
char * q_freq       = "*F?\r";  // returns current LO frequency
char * q_dev_name   = "*?\r";   // example "RSHFIQ"
char * q_ver_num    = "*W\r";   // example "RS-HFIQ FW 2.4a"
char * s_TX_OFF     = "*X0\r";
char * s_TX_ON      = "*X1\r";
char * q_Temp       = "*T\r";
char * q_Analog_Read = "*L\r";
char * q_EXT_freq   = "*E?\r";
//char * s_EXT_freq = "*EXXXXX\r";  // sets PLL Clock1
char * q_F_Offset   = "*D?\r";   // Offset to add to LO freq
char * q_clip_on    = "*C\r";  // clipping occuring, add external attenuation
char * q_BIT_freq   = "*B?\r";  // Built In Test.  Uses PLL clock 2

// ************************************************* Setup *****************************************
void setup()
{
  while (!Serial && (millis() < 5000)) ; // wait for Arduino Serial Monitor
  Serial.println("\n\nUSB Host Testing - Serial");
  RSHFIQ.begin();
  Serial.println("Waiting for RS-HFIQ device to register on USB Host port");
  while (!Proceed)  // observed about 500ms required.
  {
    refresh_RSHFIQ();   // wait until we have a valid USB 
    //Serial.print("Retry (500ms) = "); Serial.println(counter++);
    delay (500);
  }
  delay(1000);  // about 1-2 seconds needed before RS-HFIQ ready to receive commands over USB
  userial.print(q_dev_name); // get our device ID name
  print_RSHFIQ();
  userial.print(q_ver_num);
  print_RSHFIQ();
  init_PLL();  // has a long delay
  counter = 0;
  new_freq("5000000");   // Set a frequency for test.
  delay(500);
  userial.print(q_freq);  // query the current frequency.  First time after startup it will report 00.
  print_RSHFIQ();
  delay(2000);  // wait for device to get ready and respond
}

// ********************************************Loop ******************************************

void loop()
{  
  new_freq(freq);   // Set a frequency for test.
  userial.print(q_freq);  // query the current frequency
  print_RSHFIQ();   // Print our query results
  userial.print(q_F_Offset);
  print_RSHFIQ();   // Print our query results
  userial.print(q_Analog_Read);
  print_RSHFIQ();   // Print our query results
  userial.print(q_Temp);
  print_RSHFIQ();   // Print our query results
  userial.print(q_BIT_freq);
  print_RSHFIQ();   // Print our query results
  userial.print(q_clip_on);
  print_RSHFIQ();   // Print our query results
  //Serial.print("loop counter = "); Serial.println(counter++);
  delay(1);
}

void init_PLL(void)
{
  userial.print(s_initPLL);
  delay(3000);   // delay needed to turn on PLL clock0
}

void new_freq(char * freq)
{
  userial.printf("*F%s\r", freq);
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
}

void print_RSHFIQ()
{
 while ( int c = read_RSHFIQ())
  {
    Serial.write(c);
  }
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
