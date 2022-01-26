//
//  SD_CARD.cpp
//
//  SD Card realted support
//
//

#include "SDR_RA8875.h"
#include "RadioConfig.h"
#include "SD_Card.h"

#ifndef DBGSPECT
extern struct   Spectrum_Parms      Sp_Parms_Def[];
extern struct   Band_Memory         bandmem[];
extern struct   User_Settings       user_settings[];

// *******************************   SD Card  ************************************************************
// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.

// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// Teensy audio board: pin 10
// Teensy 3.5 & 3.6 & 4.1 on-board: BUILTIN_SDCARD
// Wiz820+SD board: pin 4
// Teensy 2.0: pin 0
// Teensy++ 2.0: pin 20
const int chipSelect = BUILTIN_SDCARD;

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
File SDR_sd_file;
void printDirectory(File dir, int numTabs);
// make a string for assembling the data to log:
String dataString = "";
//
// *******************************   SD Card  ************************************************************

// SD Card list files
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

bool Open_SD_cfgfile(void)
{
    uint8_t success;

    Serial.print("\nInitializing SD Card...");
  
    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
        Serial.println("SD Card failed, or not present");
        // don't do anything more:
        return false;
    }
    Serial.println("SD Card initialized.");
    
    Serial.println("\nOpen or Create our RADIOCFG.CFG data file.");
    //SD.remove("radiocfg.cfg");
    if (SD.exists("radiocfg.cfg")) 
    {
        Serial.println("radiocfg.cfg exists.");
        success = true;
    }
    else 
    {    
        Serial.println("radiocfg.cfg doesn't exist.");
        // open a new file and immediately close it:
        Serial.println("Creating RADIOCFG.CFG file..");
        SDR_sd_file = SD.open("radiocfg.cfg", FILE_WRITE);
        SDR_sd_file.close();

        // Check to see if the file exists: 
        if (SD.exists("radiocfg.cfg")) 
        {
            Serial.println("radiocfg.cfg exists.");
            success = true;
        }
        else 
        {
            Serial.println("radiocfg.cfg doesn't exist.");  
            success = false;
        }
    }
    Serial.println("\nSD Card Directory...");
    SDR_sd_file = SD.open("/");
    printDirectory(SDR_sd_file, 0);
    Serial.println("");
    SDR_sd_file.close();
    return success;
}

void SD_CardInfo(void)
{
    Serial.print("\nSD Card Information...");
    // we'll use the initialization code from the utility libraries
    // since we're just testing if the card is working!
    if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
    } else {
    Serial.println("Wiring is correct and a card is present.");
    }

    // print the type of card
    Serial.print("\nCard type: ");
    switch(card.type()) {
    case SD_CARD_TYPE_SD1:
        Serial.println("SD1");
        break;
    case SD_CARD_TYPE_SD2:
        Serial.println("SD2");
        break;
    case SD_CARD_TYPE_SDHC:
        Serial.println("SDHC");
        break;
    default:
        Serial.println("Unknown");
    }

    // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
    if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
    }


    // print the type and size of the first FAT-type volume
    uint32_t volumesize;
    Serial.print("\nVolume type is FAT");
    Serial.println(volume.fatType(), DEC);
    Serial.println();

    volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
    volumesize *= volume.clusterCount();       // we'll have a lot of clusters
    if (volumesize < 8388608ul) {
    Serial.print("Volume size (bytes): ");
    Serial.println(volumesize * 512);        // SD card blocks are always 512 bytes
    }
    Serial.print("Volume size (Kbytes): ");
    volumesize /= 2;
    Serial.println(volumesize);
    Serial.print("Volume size (Mbytes): ");
    volumesize /= 1024;
    Serial.println(volumesize);


    Serial.println("\nFiles found on the card (name, date and size in bytes): ");
    //root.openRoot(volume);

    // list all files in the card with date and size
    //root.ls(LS_R | LS_DATE | LS_SIZE);
}

void write_db_tables(void)
{
    // Clear out the old file for testing.  We will rewrite the whole thing for now.
    SD.remove("radiocfg.db");

    SDR_sd_file = SD.open("radiocfg.db", FILE_WRITE);
    
    // if the file is available, write to it:
    if (SDR_sd_file) {
        // Write our data file here
        Serial.println("Copy Database Records from memory to SD Card file radiocfg.db");
        // Start with User Profiles 
        for (int i = 0; i < USER_SETTINGS_NUM; i++)
        {
            byte dataS[sizeof(user_settings[0])];
            memmove(dataS, &user_settings[i], sizeof(user_settings[i]));
            SDR_sd_file.write(dataS, sizeof(dataS));
        }

        // Band Memory table         
        for (int i = 0; i < BANDS; i++)
        {
            byte dataS[sizeof(bandmem[0])];
            memmove(dataS, &bandmem[i], sizeof(bandmem[i]));
            SDR_sd_file.write(dataS, sizeof(dataS));
        }
        
        // Spectrum table         
        for (int i = 0; i < PRESETS; i++)
        {
            byte dataS[sizeof(Sp_Parms_Def[0])];;
            memmove(dataS, &Sp_Parms_Def[i], sizeof(Sp_Parms_Def[i]));
            SDR_sd_file.write(dataS, sizeof(dataS));
        }

        // reads for later
            //SDR_sd_file.read(dataS, sizeof(dataS));  //read it back
            //memmove(&user_settings[0], dataS, sizeof(user_settings[i]));

        Serial.println("\nClose File");
        SDR_sd_file.close();
        
        Serial.println("Print Directory");
        SDR_sd_file = SD.open("/");
        printDirectory(SDR_sd_file, 0);
        
        Serial.println("Close Directory\n");
        SDR_sd_file.close();
    }  
    // if the file isn't open, pop up an error:
    else {
        Serial.println("error opening radiocfg.db");
    }
}

void read_db_tables(void)
{
    SDR_sd_file = SD.open("radiocfg.db", FILE_READ);
    
    // if the file is available, read it:
    if (SDR_sd_file) {
        // Read our data file here
        Serial.println("Copy Database Records from SD Card file radiocfg.db to memory");
        // Start with User Profiles 
        for (int i = 0; i < USER_SETTINGS_NUM; i++)
        {
            byte dataS[sizeof(user_settings[0])];
            SDR_sd_file.read(dataS, sizeof(dataS));  //read it back
            memmove(&user_settings[i], dataS, sizeof(user_settings[i]));
        }

        // Band Memory table         
        for (int i = 0; i < BANDS; i++)
        {
            byte dataS[sizeof(bandmem[0])];
            SDR_sd_file.read(dataS, sizeof(dataS));
            memmove(&bandmem[i], dataS, sizeof(bandmem[i]));
        }
        
        // Spectrum table         
        for (int i = 0; i < PRESETS; i++)
        {
            byte dataS[sizeof(Sp_Parms_Def[0])];;
            SDR_sd_file.read(dataS, sizeof(dataS));
            memmove(&Sp_Parms_Def[i], dataS, sizeof(Sp_Parms_Def[i]));
        }

        Serial.println("\nClose File");
        SDR_sd_file.close();
        
        Serial.println("Print Directory");
        SDR_sd_file = SD.open("/");
        printDirectory(SDR_sd_file, 0);
        
        Serial.println("Close Directory\n");
        SDR_sd_file.close();
    }  
    // if the file isn't open, pop up an error:
    else {
        Serial.println("error opening radiocfg.db");
    }
}

bool write_radiocfg_h(void) // Standalone function wil create a file if needed and 
{
    char buf[80];
    uint8_t success;

    Serial.print("\nWriting RadioCfg.h to SD Card...");
  
    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
        Serial.println("SD Card failed, or not present");
        // don't do anything more:
        return false;
    }
    Serial.println("SD Card initialized.");
    
    Serial.println("Open or Create our RadioCfg.h data file.");
    
    if (SD.exists("radiocfg.h")) 
    {
        Serial.println("RadioCfg.h exists, removing the file.");
        SD.remove("radiocfg.h");  // remove to rewrite fresh each time
        success = true;
    }

    if (!SD.exists("radiocfg.h")) 
    {    
        Serial.println("RadioCfg.h doesn't exist.");
        // open a new file and immediately close it:
        Serial.println("Creating RadioCfg.h file..");
        SDR_sd_file = SD.open("radiocfg.h", FILE_WRITE);
        SDR_sd_file.close();
        success = true;
    }

    SDR_sd_file = SD.open("radiocfg.h", FILE_WRITE);    

    if (!SD.exists("radiocfg.h")) 
    {
        Serial.println("radiocfg.h does not exists.");
        return false;
    }
    // Write out our individual config parameters,  mostly #defines from RadioCfg.h
    // Since these are pre-compile and not run time variables, this is intended to be read as a .h at compile time.
    // Need to set each up as a #ifdef XXX #undef #define construct to machine generate the .h file used for compile. 
    // The current Radioconfig.h text is the source for this to write out the machine generated version.
    // The include should be this file, not RadioCfg.h. 
    // This way, unless you call this write function, each compile will use the last used .h file regardless of what 
    // new changes are brought down from source control.
    // This need to be done on the PC doing the compiling but will start here. Can transfer the file manually to the PC.
    #ifdef USE_RA8875
        strcpy(buf,"#define USE_RA8875");        
    #else
        strcpy(buf,"#ifdef USE_RA8875\n#undef USE_RA8875\n#endif");        
    #endif
    SDR_sd_file.println(buf);
    #ifdef OCXO_10MHZ
        strcpy(buf,"#define OCXO_10MHZ");        
    #else
        strcpy(buf,"#ifdef OCXO_10MHZ\n#undef OCXO_10MHZ\n#endif");        
    #endif
    #ifdef si5351_TCXO
        strcpy(buf,"#define si5351_TCXO");        
    #else
        strcpy(buf,"#ifdef si5351_TCXO\n#undef si5351_TCXO\n#endif");        
    #endif
    #ifdef si5351_XTAL_25MHZ
        strcpy(buf,"#define si5351_XTAL_25MHZ");        
    #else
        strcpy(buf,"#ifdef si5351_XTAL_25MHZ\n#undef si5351_XTAL_25MHZ\n#endif");        
    #endif
    #ifdef DIG_STEP_ATT
        strcpy(buf,"#define DIG_STEP_ATT");        
    #else
        strcpy(buf,"#ifdef DIG_STEP_ATT\n#undef DIG_STEP_ATT\n#endif");        
    #endif
    #ifdef SV1AFN_BPF
        strcpy(buf,"#define SV1AFN_BPF");        
    #else
        strcpy(buf,"#ifdef SV1AFN_BPF\n#undef SV1AFN_BPF\n#endif");        
    #endif
    #ifdef ENET
        strcpy(buf,"#define ENET");        
    #else
        strcpy(buf,"#ifdef ENET\n#undef ENET\n#endif");        
    #endif
    #ifdef USE_DHCP
        strcpy(buf,"#define ENET");        
    #else
        strcpy(buf,"#ifdef ENET\n#undef ENET\n#endif");        
    #endif
    #ifdef I2C_LCD
        strcpy(buf,"#define I2C_LCD");        
    #else
        strcpy(buf,"#ifdef I2C_LCD\n#undef I2C_LCD\n#endif");        
    #endif
    #ifdef I2C_ENCODERS
        strcpy(buf,"#define I2C_ENCODERS");        
    #else
        strcpy(buf,"#ifdef I2C_ENCODERS\n#undef I2C_ENCODERS\n#endif");        
    #endif
    #ifdef USE_ENET_PROFILE
        strcpy(buf,"#define USE_ENET_PROFILE");        
    #else
        strcpy(buf,"#ifdef USE_ENET_PROFILE\n#undef USE_ENET_PROFILE\n#endif");        
    #endif
    #ifdef REMOTE_OPS
        strcpy(buf,"#define REMOTE_OPS");        
    #else
        strcpy(buf,"#ifdef REMOTE_OPS\n#undef REMOTE_OPS\n#endif");        
    #endif
    #ifdef TEST_SINEWAVE_SIG
        strcpy(buf,"#define TEST_SINEWAVE_SIG");        
    #else
        strcpy(buf,"#ifdef TEST_SINEWAVE_SIG\n#undef TEST_SINEWAVE_SIG\n#endif");        
    #endif
    #ifdef PANADAPTER
        strcpy(buf,"#define PANADAPTER");        
    #else
        strcpy(buf,"#ifdef PANADAPTER\n#undef PANADAPTER\n#endif");        
    #endif
    #ifdef PANADAPTER_INVERT
        strcpy(buf,"#define PANADAPTER_INVERT");        
    #else
        strcpy(buf,"#ifdef PANADAPTER_INVERT\n#undef PANADAPTER_INVERT\n#endif");        
    #endif
    #ifdef ALL_CAT
        strcpy(buf,"#define ALL_CAT");        
    #else
        strcpy(buf,"#ifdef ALL_CAT\n#undef ALL_CAT\n#endif");        
    #endif
    #ifdef TOUCH_ROTATION
        strcpy(buf,"#define TOUCH_ROTATION");        
    #else
        strcpy(buf,"#ifdef TOUCH_ROTATION\n#undef TOUCH_ROTATION\n#endif");        
    #endif

    Serial.println("File Write Completed, Closing the File");
    SDR_sd_file.close();
    
    Serial.println("\nPrint Directory");
    SDR_sd_file = SD.open("/");
    printDirectory(SDR_sd_file, 0);
    
    Serial.println("Close Directory\n");
    SDR_sd_file.close();  // Close out the file whcih also flushes unwritten bytes.
    return success;
}

/*
    if (si5351_CORRECTION == 0);
    if (SPECTRUM_PRESET == 0);
    if (PANADAPTER_LO ==  8215000);
    if (PANADAPTER_MODE_OFFSET_DATA == 0 );
    if (SCREEN_ROTATION  ==  0);
    if (VFO_ENC_PIN_A == 4);
    if (VFO_ENC_PIN_B == 5);
    if (VFO_PPR == 6);
*/

/*
#ifdef I2C_ENCODERS
  #define I2C_INT_PIN   29
  #define MF_ENC_ADDR  (0x61)  	    // Address 0x61 only - Jumpers A0, A5 and A6 are soldered.
  #define ENC2_ADDR    (0x62)  	    // Address 0x62 only - Jumpers A1, A5 and A6 are soldered.
  //#define ENC3_ADDR    (0x63)  	// Address 0x63 only - Jumpers A0, A1, A5 and A6 are soldered.
#else
  #define MF_ENC_PIN_A 40   // list pins for any non I2C aux encoders.
  #define MF_ENC_PIN_B 39
#endif // I2C_ENCODERS

  
#ifdef DIG_STEP_ATT
  #define Atten_CLK       31
  #define Atten_DATA      32
  #define Atten_LE        30
#endif  // DIG_STEP_ATT
*/
/*
#ifdef USE_RA8875
  #define  SCREEN_WIDTH      800 
  #define  SCREEN_HEIGHT     480
  #define  RA8875_INT        14   //any pin
  #define  RA8875_CS         10   //any digital pin
  #define  RA8875_RESET      9    //any pin or nothing!
  #define  MAXTOUCHLIMIT     3    //1...5  using 3 for 3 finger swipes, otherwise 2 for pinches or just 1 for touch
  #include <SPI.h>                // included with Arduino
  //#include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
  //#include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
  #include <RA8875.h>           // internal Teensy library with ft5206 cap touch enabled in user_setting.h
#else // If RA8876 is not used then assume the RA8876_t3 1024x600 is.
#define USE_RA8876_t3
//
#define  SCREEN_WIDTH      1024 
#define  SCREEN_HEIGHT     600
//#include <ili9488_t3_font_Arial.h>      // https://github.com/PaulStoffregen/ILI9341_t3
//#include <ili9488_t3_font_ArialBold.h>  // https://github.com/PaulStoffregen/ILI9341_t3
#include <RA8876_t3.h>           // Github
#include <FT5206.h>
#define  CTP_INT           14   // Use an interrupt capable pin such as pin 2 (any pin on a Teensy)
#define  RA8876_CS         10   //any digital pin
#define  RA8876_RESET      9    //any pin or nothing!
#define  MAXTOUCHLIMIT     3    //1...5  using 3 for 3 finger swipes, otherwise 2 for pinches or just 1 for touch

// From RA8875/_settings/RA8875ColorPresets.h
// Colors preset (RGB565)
const uint16_t	RA8875_BLACK            = 0x0000;
const uint16_t 	RA8875_WHITE            = 0xFFFF;
const uint16_t	RA8875_RED              = 0xF800;
const uint16_t	RA8875_GREEN            = 0x07E0;
const uint16_t	RA8875_BLUE             = 0x001F;
const uint16_t 	RA8875_CYAN             = RA8875_GREEN | RA8875_BLUE;//0x07FF;
const uint16_t 	RA8875_MAGENTA          = 0xF81F;
const uint16_t 	RA8875_YELLOW           = RA8875_RED | RA8875_GREEN;//0xFFE0;  
const uint16_t 	RA8875_LIGHT_GREY 		  = 0xB5B2; // the experimentalist
const uint16_t 	RA8875_LIGHT_ORANGE 	  = 0xFC80; // the experimentalist
const uint16_t 	RA8875_DARK_ORANGE 		  = 0xFB60; // the experimentalist
const uint16_t 	RA8875_PINK 			      = 0xFCFF; // M.Sandercock
const uint16_t 	RA8875_PURPLE 			    = 0x8017; // M.Sandercock
const uint16_t 	RA8875_GRAYSCALE 		    = 2113;//grayscale30 = RA8875_GRAYSCALE*30
#endif // USE_RA8876_t3

*/

/*
#ifdef ENET
    #include <NativeEthernet.h>
    #include <NativeEthernetUdp.h>
    
    // Choose or create your desired time zone offset or use 0 for UTC.
    #define MYTZ 0

    
    // If NOT using DHCP then assign a static IP address for the SDR   
    #ifndef USE_DHCP
    // The IP Address is ignored if using DHCP
    // IP address is defined in SDR_Network.cpp 
    #endif // USE_DHCP
    
    #define MY_LOCAL_PORTNUM 7943;     // local port the SDR will LISTEN on for any remote display/Desktop app
*/
#endif // debug
