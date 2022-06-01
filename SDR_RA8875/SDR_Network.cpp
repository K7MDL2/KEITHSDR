//
//  SDR_Network.cpp
//
//  Contains ethernet code for the SDR.  
//  Goal is to use UDP messages to connect a control head to the SDR base rig.  
//  The FFT output and 2-way control messages are passed over UDP.
//  Audio connection to the control head will for now be analog.
//  The Control head could be another Teensy/arduino with display, or a PC client app.
//
#include "RadioConfig.h"
#include "SDR_RA8875.h"
//#include "SDR_Network.h"

#ifdef ENET    // Skip all of this file if no enet

#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <TimeLib.h>

extern tmElements_t tm;

#define TX_BUFFER_SIZE         4100    
#define RX_BUFFER_SIZE		255
// Choose or create your desired time zone offset or use 0 for UTC.
//const int timeZone = 1;     // Central European Time
const int timeZone = 0;     // UTC
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

// Enter a MAC address and IP address for your controller below. MAC not required for Teensy cause we are using TeensyMAC function.
// The IP address will be dependent on your local network:  don't need this since we can automatically figure ou tthe mac
//byte mac[] = {
//  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEC
//};

// Choose to use DHCP or a static IP address for the SDR - set in RadioConfig.h  
#ifndef USE_DHCP
	// The IP Address is ignored if using DHCP
	IPAddress ip(192, 168, 1, 237);    // Our static IP address.  Could use DHCP but preferring static address.
	IPAddress dns(192, 168, 1, 1);    // Our static IP address.  Could use DHCP but preferring static address.
	IPAddress gateway(192, 168, 1, 1);    // Our static IP address.  Could use DHCP but preferring static address.
#endif // USE_DHCP

unsigned int localPort = MY_LOCAL_PORTNUM;     // local port to LISTEN for the remote display/Desktop app

//#define REMOTE_OPS  - conditionally defined in main header file for now
#ifdef REMOTE_OPS
// This is for later remote operation usage
//Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
IPAddress remote_ip(192, 168, 1, 7);    // Destination IP (desktop app or remote display Arduino)
unsigned int remoteport = MY_REMOTE_PORTNUM;    // The destination port to SENDTO (a remote display or Desktop app)
#endif // REMOTE_OPS

// This is for the NTP service to update the clock when connected to the internet
unsigned int localPort_NTP = 8888;       // Local port to listen for UDP packets
const char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server
extern time_t prevDisplay; // When the digital clock was displayed

// function declarations
void toggle_enet_data_out(uint8_t mode);
uint8_t enet_write(uint8_t *tx_buffer, const int count);
uint8_t enet_read(void);
void teensyMAC(uint8_t *mac);
void enet_start(void);

// NTP client time setup
time_t getNtpTime();
void sendNTPpacket(const char * address);
//void RX_NTP_time(void);

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer_NTP[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp_NTP;

// SDR network setup
// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
char ReplyBuffer[] = "Random Reply";        // a string to send back

// our variables
uint8_t enet_ready = 0;
unsigned long enet_start_fail_time = 0;
uint8_t rx_buffer[RX_BUFFER_SIZE];
uint8_t tx_buffer[TX_BUFFER_SIZE];
uint8_t rx_count = 0;
uint8_t tx_count = 0;
uint8_t enet_data_out = 0;
uint8_t sdata[RX_BUFFER_SIZE], *pSdata1=sdata, *pSdata2=sdata;
extern uint8_t user_Profile;
extern uint8_t NTP_hour;  //NTP time 
extern uint8_t NTP_min;
extern uint8_t NTP_sec;
extern void displayTime(void);
extern const int timeZone;
extern uint8_t user_Profile;  // global tracks our current user profile
extern struct User_Settings user_settings[];

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// Toggle UDP output data
COLD void toggle_enet_data_out(uint8_t mode)
{
	if (mode == 1)
	enet_data_out = 1;
	if (mode ==0)
	enet_data_out = 0;
	if (mode ==2){
		if (enet_data_out == 0)
			enet_data_out = 1;
		else         
			enet_data_out = 0; 
	}
	if (enet_data_out == 1){          
		user_settings[user_Profile].enet_output = ON;    
		DPRINTLN(">Enabled UDP Data Output");
	}
	else {          
		user_settings[user_Profile].enet_output = OFF;
		DPRINTLN(">Disabled UDP Data Output");
	}
}

COLD void teensyMAC(uint8_t *mac) 
{
  	static char teensyMac[23];
  
  	#if defined (HW_OCOTP_MAC1) && defined(HW_OCOTP_MAC0)
      DPRINTLN("using HW_OCOTP_MAC* - see https://forum.pjrc.com/threads/57595-Serial-amp-MAC-Address-Teensy-4-0");
      for(uint8_t by=0; by<2; by++) mac[by]=(HW_OCOTP_MAC1 >> ((1-by)*8)) & 0xFF;
      for(uint8_t by=0; by<4; by++) mac[by+2]=(HW_OCOTP_MAC0 >> ((3-by)*8)) & 0xFF;
  
      #define MAC_OK

  	#else
    
      mac[0] = 0x04;
      mac[1] = 0xE9;
      mac[2] = 0xE5;
  
      uint32_t SN=0;
      __disable_irq();
      
      #if defined(HAS_KINETIS_FLASH_FTFA) || defined(HAS_KINETIS_FLASH_FTFL)
    	  DPRINTLN("using FTFL_FSTAT_FTFA - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
    	
      	FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
      	FTFL_FCCOB0 = 0x41;
      	FTFL_FCCOB1 = 15;
      	FTFL_FSTAT = FTFL_FSTAT_CCIF;
      	while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ; // wait
      	SN = *(uint32_t *)&FTFL_FCCOB7;
    
    	  #define MAC_OK
        
      #elif defined(HAS_KINETIS_FLASH_FTFE)
    	  DPRINTLN("using FTFL_FSTAT_FTFE - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
    	
      	kinetis_hsrun_disable();
      	FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
      	*(uint32_t *)&FTFL_FCCOB3 = 0x41070000;
      	FTFL_FSTAT = FTFL_FSTAT_CCIF;
      	while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ; // wait
      	SN = *(uint32_t *)&FTFL_FCCOBB;
      	kinetis_hsrun_enable();
    
    	  #define MAC_OK
        
      #endif
      
      __enable_irq();
  
      for(uint8_t by=0; by<3; by++) mac[by+3]=(SN >> ((2-by)*8)) & 0xFF;

  	#endif

  	#ifdef MAC_OK
      sprintf(teensyMac, "MAC: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      DPRINTLN(teensyMac);
    	#else
      DPRINTLN("ERROR: could not get MAC");
  	#endif
}

HOT uint8_t enet_read(void)
{
    if (enet_ready && user_settings[user_Profile].enet_enabled)
    {
        //  experiment with this -->   udp.listen( true );           // and wait for incoming messag
        
        rx_count = 0;
        int count = 0; 

        // if there's data available, read a packet
        count = Udp.parsePacket();      
        rx_buffer[0] = _NULL;
        if (count > 0)
        {
            Udp.read(rx_buffer, RX_BUFFER_SIZE);
            rx_buffer[count] = '\0';
            rx_count = count;          
            DPRINTLN(rx_count);
            DPRINTLN((char *) rx_buffer);
            
            // initially p1 = p2.  parser will move p1 up to p2 and when they are equal, buffer is empty, parser will reset p1 and p2 back to start of sData         
            memcpy(pSdata2, rx_buffer, rx_count+1);   // append the new buffer data to current end marked by pointer 2        
            pSdata2 += rx_count;                      // Update the end pointer position. The function processing chars will update the p1 and p2 pointer             
            rx_count = pSdata2 - pSdata1;             // update count for total unread chars. 
            //DPRINTLN(rx_count);  
        }
        rx_buffer[0] = '\0';
        return rx_count;
    }
    return 0;
}

HOT uint8_t enet_write(uint8_t *tx_buffer, const int count)   //, uint16_t tx_count)
{   
	#ifdef REMOTE_OPS
   	if (enet_ready && user_settings[user_Profile].enet_enabled && user_settings[user_Profile].enet_output)  // skip if no enet connection
   	{
		//DPRINT("ENET Write: ");
		//DPRINTLN((char *) tx_buffer);
		Udp.beginPacket(remote_ip, remoteport);
		Udp.write(tx_buffer, count);
		Udp.endPacket();
		return 1;
    }
	#endif
   	return 0;
} 

COLD void enet_start(void)
{
	if (!user_settings[user_Profile].enet_enabled)
		return;

	uint8_t mac[6];
	teensyMAC(mac);   
	//	byte mac[] = {
	//  		0x04, 0xE9, 0xE5, 0x0D, 0x63, 0x2C
	//	};
	
	// start the Ethernet 
	#ifdef USE_DHCP 
		// If using DHCP (leave off the ip arg) works but more difficult to configure the desktop and remote touchscreen clients
		Ethernet.begin(mac);   // DHCP option
	#else 
		Ethernet.begin(mac, ip);  // Static IP option
	#endif

	// Check for Ethernet hardware present
	enet_ready = 0;
	if (Ethernet.hardwareStatus() == EthernetNoHardware) 
	{
		DPRINTLN(F("Ethernet shield was not found.  Sorry, can't run the network without hardware. :("));
		enet_ready = 0;  // shut down usage of enet
	}
	else
	{
		//delay(1000);
		DPRINT(F("Ethernet Address = "));
		DPRINTLN(Ethernet.localIP());
		//delay(4000);
		if (Ethernet.linkStatus() == LinkOFF) 
		{
			DPRINTLN(F("Ethernet cable is not connected."));
			enet_ready = 0;
		}
		else
		{  
			enet_ready = 1;
			//delay(100);
			DPRINTLN(F("Ethernet cable connected."));
			// start UDP
			Udp.begin(localPort); // Startup our SDR comms
      		Udp_NTP.begin(localPort_NTP);  // startup NTP Client comms
		}
	}
}
//
/*--------------------------------------- NTP code -----------------------------------*/
//
COLD time_t getNtpTime()
{
    int size = Udp_NTP.parsePacket();
    if (size >= NTP_PACKET_SIZE) 
	  {
    		//DPRINTLN("Receive NTP Response");
    		Udp_NTP.read(packetBuffer_NTP, NTP_PACKET_SIZE);  // read packet into the buffer
    		unsigned long secsSince1900;
    		// convert four bytes starting at location 40 to a long integer
    		secsSince1900 =  (unsigned long)packetBuffer_NTP[40] << 24;
    		secsSince1900 |= (unsigned long)packetBuffer_NTP[41] << 16;
    		secsSince1900 |= (unsigned long)packetBuffer_NTP[42] << 8;
    		secsSince1900 |= (unsigned long)packetBuffer_NTP[43];
    		//DPRINTLN(secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR);
    		time_t t;
    		t = secsSince1900 - 2208988790UL + timeZone * SECS_PER_HOUR;
    		Teensy3Clock.set(t); // set the RTC
    		setTime(t);			 // set the time structure
    		return t;
    }
  	DPRINTLN(F("No NTP Response :-("));
  	return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
COLD void sendNTPpacket(const char * address) 
{
    // set all bytes in the buffer to 0
    memset(packetBuffer_NTP, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer_NTP[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer_NTP[1] = 0;     // Stratum, or type of clock
    packetBuffer_NTP[2] = 6;     // Polling Interval
    packetBuffer_NTP[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer_NTP[12]  = 49;
    packetBuffer_NTP[13]  = 0x4E;
    packetBuffer_NTP[14]  = 49;
    packetBuffer_NTP[15]  = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp_NTP.beginPacket(address, 123); // NTP requests are to port 123
    Udp_NTP.write(packetBuffer_NTP, NTP_PACKET_SIZE);
    Udp_NTP.endPacket();
}

/*  Mod required for NativeEthernet.cpp file in Ethernet.begin class.  
 *   At end of the function is a statement that hangs if no ethernet cable is connected.  
 *   
 *   while(!link_status){
 *      return;
 *   }
 *   
 *   You can never progress to use the link status function to query if a cable is connected and the program is halted.
 *    
 *   Add the below to let it escape.  Use the enet_ready flag to signal if enet started OK or not.  
 *   
    uint16_t escape_counter = 0;
    while(!link_status && escape_counter < 200){
        escape_counter++;
        DPRINTLN("Waiting for Link Status");
        delay(10);
        return;
    }  
 *
 */
#endif  //ENET
