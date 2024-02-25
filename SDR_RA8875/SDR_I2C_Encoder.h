#ifndef _SDR_I2C_Encoder_H_
#define _SDR_I2C_Encoder_H_
//#ifdef I2C_ENCODERS
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

// These are the per-encoder function declarations
#ifdef I2C_ENC1_ADDR
void blink_I2C_ENC1_RGB(void);
#endif
#ifdef I2C_ENC2_ADDR
void blink_I2C_ENC2_RGB(void);
#endif
#ifdef I2C_ENC3_ADDR
void blink_I2C_ENC3_RGB(void);
#endif
#ifdef I2C_ENC4_ADDR
void blink_I2C_ENC4_RGB(void);
#endif
#ifdef I2C_ENC5_ADDR
void blink_I2C_ENC5_RGB(void);
#endif
#ifdef I2C_ENC6_ADDR
void blink_I2C_ENC6_RGB(void);
#endif
void set_I2CEncoders(void);

// These are generic callback functions - meaning when a hardware event occurs these functions are 
// called with the info associated with that encoder.  We can assing each encoder to things like AF and RF gain.
void i2c_encoder_rotated(i2cEncoderLibV2* obj);
void i2c_switch_click(i2cEncoderLibV2* obj, uint8_t slot);
void i2c_encoder_thresholds(i2cEncoderLibV2* obj, uint8_t slot);
void gpio_switch_timer_start(uint8_t _id);
void gpio_switch_click(uint8_t _id);
void gpio_encoder_rotated(i2cEncoderLibV2* obj, int32_t count);

//#endif //I2C_ENCODERS
#endif //  _SDR_I2C_Encoder_H_
