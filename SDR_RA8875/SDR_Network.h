//
//  SDR_Network.h
//
//  Contains ethernet code for the SDR.  
//  Goal is to use UDP messages to connect a control head to the SDR base rig.  
//  The FFT output and 2-way control messages are passed over UDP.
//  Audio connection to the control head will for now be analog.
//  The Control head could be another Teensy/arduino with display, or a PC client app.
//
//#define ENET   // defined in RadioConfig..h
#ifdef ENET    

#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

#define BUFFER_SIZE         (4096)
#define LINE_STR_LENGTH     (20u)

// defined in RadioConfig.h
// extern IPAddress ip;                //(192, 168, 1, 237);    // Our static IP address.  Could use DHCP but preferring static address.
// extern unsigned int localPort;       // = 7943;     // local port to LISTEN for the remote display/Desktop app

// function declarations
void toggle_enet_data_out(uint8_t mode);
uint8_t enet_write(uint8_t *tx_buffer, const int count);
uint8_t enet_read(void);
void teensyMAC(uint8_t *mac);
void enet_start(void);

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
char ReplyBuffer[] = "Random Reply";        // a string to send back

// our variables
uint8_t enet_ready = 0;
unsigned long enet_start_fail_time = 0;
uint8_t rx_buffer[BUFFER_SIZE];
uint8_t tx_buffer[BUFFER_SIZE];
uint8_t rx_count = 0;
uint8_t tx_count = 0;
uint8_t enet_data_out = 0;
static uint8_t sdata[BUFFER_SIZE], *pSdata1=sdata, *pSdata2=sdata;
extern uint8_t user_Profile;

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// Toggle UDP output data
void toggle_enet_data_out(uint8_t mode)
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
          Serial.println(">Enabled UDP Data Output");
      }
      else {          
          user_settings[user_Profile].enet_output = OFF;
          Serial.println(">Disabled UDP Data Output");
      }
}

void teensyMAC(uint8_t *mac) 
{
  static char teensyMac[23];
  
  #if defined (HW_OCOTP_MAC1) && defined(HW_OCOTP_MAC0)
    Serial.println("using HW_OCOTP_MAC* - see https://forum.pjrc.com/threads/57595-Serial-amp-MAC-Address-Teensy-4-0");
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
      Serial.println("using FTFL_FSTAT_FTFA - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
      
      FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
      FTFL_FCCOB0 = 0x41;
      FTFL_FCCOB1 = 15;
      FTFL_FSTAT = FTFL_FSTAT_CCIF;
      while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ; // wait
      SN = *(uint32_t *)&FTFL_FCCOB7;

      #define MAC_OK
      
    #elif defined(HAS_KINETIS_FLASH_FTFE)
      Serial.println("using FTFL_FSTAT_FTFE - vis teensyID.h - see https://github.com/sstaub/TeensyID/blob/master/TeensyID.h");
      
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
    Serial.println(teensyMac);
  #else
    Serial.println("ERROR: could not get MAC");
  #endif
}

uint8_t enet_read(void)
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
			Udp.read(rx_buffer, BUFFER_SIZE);
			rx_buffer[count] = '\0';
			rx_count = count;          
			Serial.println(rx_count);
			Serial.println((char *) rx_buffer);
			
			// initially p1 = p2.  parser will move p1 up to p2 and when they are equal, buffer is empty, parser will reset p1 and p2 back to start of sData         
			memcpy(pSdata2, rx_buffer, rx_count+1);   // append the new buffer data to current end marked by pointer 2        
			pSdata2 += rx_count;                      // Update the end pointer position. The function processing chars will update the p1 and p2 pointer             
			rx_count = pSdata2 - pSdata1;             // update count for total unread chars. 
			//Serial.println(rx_count);  
        }
        rx_buffer[0] = '\0';
        return rx_count;
	}
	return 0;
}

uint8_t enet_write(uint8_t *tx_buffer, const int count)   //, uint16_t tx_count)
{   
   if (enet_ready && user_settings[user_Profile].enet_enabled && user_settings[user_Profile].enet_output)  // skip if no enet connection
   {
		//Serial.print("ENET Write: ");
		//Serial.println((char *) tx_buffer);
		Udp.beginPacket(remote_ip, remoteport);
		Udp.write(tx_buffer, count);
		Udp.endPacket();
		return 1;
   }
   return 0;
} 

void enet_start(void)
{
	if (!user_settings[user_Profile].enet_enabled)
		return;

	uint8_t mac[6];
	teensyMAC(mac);   
	delay(1000);
	// start the Ethernet  
	// If using DHCP (leave off the ip arg) works but more difficult to configure the desktop and remote touchscreen clients
	Ethernet.begin(mac, ip);
	// Check for Ethernet hardware present
	enet_ready = 0;
	if (Ethernet.hardwareStatus() == EthernetNoHardware) 
	{
		Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
		//while (true) {
		//  delay(1); // do nothing, no point running without Ethernet hardware
		//}
		enet_ready = 0;  // shut down usage of enet
	}
	else
	{
		delay(1000);
		Serial.print("Ethernet Address = ");
		Serial.println(Ethernet.localIP());
		delay(5000);
		if (Ethernet.linkStatus() == LinkOFF) 
		{
			Serial.println("Ethernet cable is not connected.");
			enet_ready = 0;
		}
		else
		{  
			enet_ready = 1;
			delay(100);
			Serial.println("Ethernet cable connected.");
			// start UDP
			Udp.begin(localPort);
		}
	}
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
        Serial.println("Waiting for Link Status");
        delay(10);
        return;
    }  
 *
 */
#endif
