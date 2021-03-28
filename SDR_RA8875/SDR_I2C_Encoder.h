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

#ifdef I2C_ENCODER

// These are the per-encoder function declarations
void blink_MFG_RGB(void);
void set_I2CEncoder();

extern uint8_t curr_band;    // global tracks our current band setting. 
extern uint8_t user_Profile; // global tracks our current user profile

// These are generic callback functions - meaning when a hardware event occurs these functions are 
// called with the info associated with that encoder.  We can assing each encoder to things like AF and RF gain.
void encoder_rotated(i2cEncoderLibV2* obj);
void encoder_click(i2cEncoderLibV2* obj);
void encoder_thresholds(i2cEncoderLibV2* obj);

extern void MF_Service(int8_t counts);

//Callback when the MF Gain encoder is rotated
void encoder_rotated(i2cEncoderLibV2* obj) {
  if (obj->readStatus(i2cEncoderLibV2::RINC))
    Serial.print("Increment: ");
  else
    Serial.print("Decrement: ");
  Serial.println(obj->readCounterInt());
  int16_t count = obj->readCounterInt();
  MF_Service(count);
  MF_ENC.writeCounter((int32_t) 0); // Reset the counter value
  // Update the color
  uint32_t tval = 0x00FF00;  // Set the default color to green
  switch(MF_client)
  {
    case RFGAIN_BTN:    if (user_settings[user_Profile].rfGain >= 97 || user_settings[user_Profile].rfGain <=3)
                            tval = 0xFF0000;  // Change to red
                            break;
    case AFGAIN_BTN:    if (user_settings[user_Profile].afGain >= 97 || user_settings[user_Profile].afGain <=3)
                            tval = 0xFF0000;  // Change to red
                            break;
    case ATTEN_BTN:     if (bandmem[curr_band].attenuator_dB > 30 || bandmem[curr_band].attenuator_dB < 2)
                            tval = 0xFF0000;  // Change to red
                            break;
    case REFLVL_BTN:     if (bandmem[curr_band].sp_ref_lvl > -120 || bandmem[curr_band].sp_ref_lvl < -200)
                            tval = 0xFF0000;  // Change to red
                            break;
    default: obj->writeRGBCode(tval); break;
  }
  obj->writeRGBCode(tval);  // set color
}

//Callback when the encoder is pushed
void encoder_click(i2cEncoderLibV2* obj) {
  Serial.println("Push: ");
  obj->writeRGBCode(0x0000FF);
}

//Callback when the encoder reach the max or min
void encoder_thresholds(i2cEncoderLibV2* obj) {
  if (obj->readStatus(i2cEncoderLibV2::RMAX))
    Serial.println("Max!");
  else
    Serial.println("Min!");

  obj->writeRGBCode(0xFF0000);
}

//Callback when the fading process finish and set the RGB led off
void encoder_fade(i2cEncoderLibV2* obj) {
  obj->writeRGBCode(0x000000);
}

void set_I2CEncoders()
{
    pinMode(IntPin, INPUT);

    // MF KNOB - Multi-Fucntion knob setup.
    MF_ENC.reset();
    MF_ENC.begin(
        i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE
        | i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_ENABLE
        | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
    //  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
    //  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!

    MF_ENC.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a daabase value also*/
    MF_ENC.writeMax((int32_t) 100); /* Set the maximum threshold*/
    MF_ENC.writeMin((int32_t) -100); /* Set the minimum threshold */
    MF_ENC.writeStep((int32_t) 1); /* Set the step to 1*/

    /* Configure the events */
    MF_ENC.onChange = encoder_rotated;
    MF_ENC.onButtonRelease = encoder_click;
    MF_ENC.onMinMax = encoder_thresholds;
    MF_ENC.onFadeProcess = encoder_fade;

    /* Enable the I2C Encoder V2 interrupts according to the previus attached callback */
    MF_ENC.autoconfigInterrupt();
    //AF_ENC.writeInterruptConfig(0xff); /* Enable all the interrupt */
    //AF_ENC.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
    //AF_ENC.writeDoublePushPeriod(50); /*Set a period for the double push of 500ms */
    blink_MFG_RGB();
}

void blink_MFG_RGB(void)
{
    /* blink the RGB LED */
    MF_ENC.writeRGBCode(0xFF0000);
    delay(250);
    MF_ENC.writeRGBCode(0x00FF00);
    delay(250);
    MF_ENC.writeRGBCode(0x0000FF);
    delay(250);
    MF_ENC.writeRGBCode(0x000000);

    MF_ENC.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif // I2C_ENCODER

#endif //  _SDR_I2C_Encoder_H_
