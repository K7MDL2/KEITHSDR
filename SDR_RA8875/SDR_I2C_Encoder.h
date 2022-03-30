#ifndef _SDR_I2C_Encoder_H_
#define _SDR_I2C_Encoder_H_
//
//   SDR_I2C_Encoder.h
//
//   A collection of I2C encoder functions.
//   Setup for each encoder
//   Interrupt driven callback back function that do something when tehre is a pus or turn event.
//   Also has a range of RGB LED light effects
//
#include <Arduino.h>
#include <i2cEncoderLibV2.h>
//#include "RadioConfig.h"

#ifdef I2C_ENCODERS
// These are the per-encoder function declarations
#ifdef MF_ENC_ADDR
void blink_MF_RGB(void);
#endif
#ifdef ENC2_ADDR
void blink_ENC2_RGB(void);
#endif
#ifdef ENC3_ADDR
void blink_ENC3_RGB(void);
#endif
#ifdef ENC4_ADDR
void blink_ENC4_RGB(void);
#endif
#ifdef ENC5_ADDR
void blink_ENC5_RGB(void);
#endif
#ifdef ENC6_ADDR
void blink_ENC6_RGB(void);
#endif
void set_I2CEncoders(void);

// These are generic callback functions - meaning when a hardware event occurs these functions are 
// called with the info associated with that encoder.  We can assing each encoder to things like AF and RF gain.
void encoder_rotated(i2cEncoderLibV2* obj);
void encoder_click(i2cEncoderLibV2* obj);
void encoder_thresholds(i2cEncoderLibV2* obj);
#endif //  I2C_ENCODERS

#endif //  _SDR_I2C_Encoder_H_
