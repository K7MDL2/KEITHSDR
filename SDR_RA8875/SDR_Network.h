#ifndef _SDR_NETWORK_H_
#define _SDR_NETWORK_H_
//
//  SDR_Network.h
//
//  Contains ethernet code for the SDR.  
//  Goal is to use UDP messages to connect a control head to the SDR base rig.  
//  The FFT output and 2-way control messages are passed over UDP.
//  Audio connection to the control head will for now be analog.
//  The Control head could be another Teensy/arduino with display, or a PC client app.
//
#include <Arduino.h>
//#include "RadioConfig.h"

//#ifdef ENET    

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
extern const char timeServer[];   // time.nist.gov NTP server
extern time_t prevDisplay;    // When the digital clock was displayed

//#endif  //ENET

#endif //_SDR_NETWORK_
