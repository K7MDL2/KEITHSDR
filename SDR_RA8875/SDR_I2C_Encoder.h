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
//Callback when the AF Gain encoder is rotated
void encoder_rotated(i2cEncoderLibV2* obj) {
  if (obj->readStatus(i2cEncoderLibV2::RINC))
    Serial.print("Increment: ");
  else
    Serial.print("Decrement: ");
  Serial.println(obj->readCounterInt());
  obj->writeRGBCode(0x00FF00);
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

void I2C_Scanner(void)
{
  byte error, address; //variable for error and I2C address
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(500); // wait 5 seconds for the next I2C scan
}

void set_AF_I2CEncoder()
{
    pinMode(IntPin, INPUT);
    AF_ENC.reset();

    AF_ENC.begin(
        i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE
        | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE
        | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
    //  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
    //  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!

    AF_ENC.writeCounter((int32_t) 0); /* Reset the counter value */
    AF_ENC.writeMax((int32_t) 10); /* Set the maximum threshold*/
    AF_ENC.writeMin((int32_t) - 10); /* Set the minimum threshold */
    AF_ENC.writeStep((int32_t) 1); /* Set the step to 1*/

    /* Configure the events */
    AF_ENC.onChange = encoder_rotated;
    AF_ENC.onButtonRelease = encoder_click;
    AF_ENC.onMinMax = encoder_thresholds;
    AF_ENC.onFadeProcess = encoder_fade;

    /* Enable the I2C Encoder V2 interrupts according to the previus attached callback */
    AF_ENC.autoconfigInterrupt();
    //AF_ENC.writeInterruptConfig(0xff); /* Enable all the interrupt */
    //AF_ENC.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
    //AF_ENC.writeDoublePushPeriod(50); /*Set a period for the double push of 500ms */
    blink_AFG_RGB();
}

void blink_AFG_RGB(void)
{
    /* blink the RGB LED */
    AF_ENC.writeRGBCode(0xFF0000);
    delay(250);
    AF_ENC.writeRGBCode(0x00FF00);
    delay(250);
    AF_ENC.writeRGBCode(0x0000FF);
    delay(250);
    AF_ENC.writeRGBCode(0x000000);

    AF_ENC.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif // I2C_ENCODER

#endif //  _SDR_I2C_Encoder_H_
