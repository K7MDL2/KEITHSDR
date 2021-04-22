//
//     SDR_I2C_Encoder.cpp
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"
#include "SDR_I2C_Encoder.h"

#ifdef I2C_ENCODERS

#include <i2cEncoderLibV2.h>
// These are the per-encoder function declarations
void set_I2CEncoders(void);

extern uint8_t MF_client;     // Flag for current owner of MF knob services
extern uint8_t curr_band;     // global tracks our current band setting. 
extern uint8_t user_Profile;  // global tracks our current user profile
extern struct User_Settings user_settings[];
extern struct Band_Memory bandmem[];
extern bool MeterInUse;  // S-meter flag to block updates while the MF knob has control
extern Metro MF_Timeout;
Metro press_timer = Metro(900);

//Class initialization with the I2C addresses - add more here if needed
//i2cEncoderLibV2 i2c_encoder[2] = { i2cEncoderLibV2(0x62), i2cEncoderLibV2(0x61)};
#ifdef MF_ENC_ADDR
void blink_MF_RGB(void);
i2cEncoderLibV2 MF_ENC(MF_ENC_ADDR);  	/* Address 0x61 only - Jumpers A0, A5 and A6 are soldered.*/
#endif
#ifdef ENC2_ADDR
void blink_ENC2_RGB(void);
i2cEncoderLibV2 ENC2(ENC2_ADDR);  	/* Address 0x62 only - Jumpers A1, A5 and A6 are soldered.*/
#endif
#ifdef ENC3_ADDR
void blink_ENC3_RGB(void);
i2cEncoderLibV2 ENC3(ENC3_ADDR);  	/* Address 0x63 only - Jumpers A0, A1, A5 and A6 are soldered.*/
#endif
// These are generic callback functions - meaning when a hardware event occurs these functions are 
// called with the info associated with that encoder.  We can assing each encoder to things like AF and RF gain.
void encoder_rotated(i2cEncoderLibV2* obj);
void encoder_click(i2cEncoderLibV2* obj);
void encoder_thresholds(i2cEncoderLibV2* obj);

extern void MF_Service(int8_t counts, uint8_t knob);

//Callback when the MF Gain encoder is rotated
COLD void encoder_rotated(i2cEncoderLibV2* obj) 
{
	uint8_t knob_assigned;

	Serial.print(F("Encoder ID = "));
    Serial.println(obj->id);
	
	if (obj->id == user_settings[user_Profile].encoder1_client)
		knob_assigned = MF_client; 	// 2nd encoder 
	else
		knob_assigned = obj->id;

	if (obj->readStatus(i2cEncoderLibV2::RINC))
		Serial.print(F("Increment: "));
	else
		Serial.print(F("Decrement: "));
	int16_t count = obj->readCounterInt();
	Serial.println(count);
	MF_Service(count, knob_assigned);
	//obj->writeCounter((int32_t) 0); // Reset the counter value if in absolute mode. Not required in relative mode
	// Update the color
	uint32_t tval = 0x00FF00;  // Set the default color to green
	Serial.print(F("Knob Assigned to "));
    Serial.println(knob_assigned);
	if (0) //press_timer.check() == 1)
	{
		switch(knob_assigned)
		{
			case MFTUNE: 	break;
			default: 		obj->writeRGBCode(tval); break;
		}
	}
	else
	{
		switch(knob_assigned)
		{
			char string[80];   // print format stuff
			case RFGAIN_BTN:    sprintf(string, " RF:%d", user_settings[user_Profile].rfGain);
								MeterInUse = true;
								displayMeter(user_settings[user_Profile].rfGain/10, string, 5);   // val, string label, color scheme							
								if (user_settings[user_Profile].rfGain >= 97 || user_settings[user_Profile].rfGain <=3)								
									tval = 0xFF0000;  // Change to red
									break;
			case AFGAIN_BTN:    sprintf(string, " AF:%d", user_settings[user_Profile].afGain);
								MeterInUse = true;
								displayMeter(user_settings[user_Profile].afGain/10, string, 5);   // val, string label, color scheme
								if (user_settings[user_Profile].afGain >= 97 || user_settings[user_Profile].afGain <=3)
									tval = 0xFF0000;  // Change to red
									break;
			case ATTEN_BTN:     sprintf(string, " ATT:%d", bandmem[curr_band].attenuator_dB);
								MeterInUse = true;
								displayMeter(bandmem[curr_band].attenuator_dB/3, string, 5);   // val, string label, color scheme
								if (bandmem[curr_band].attenuator_dB > 30 || bandmem[curr_band].attenuator_dB < 2)
									tval = 0xFF0000;  // Change to red
									break;
			case REFLVL_BTN:    sprintf(string, "Lvl:%d", bandmem[curr_band].sp_ref_lvl);
								MeterInUse = true; 
								displayMeter((abs(bandmem[curr_band].sp_ref_lvl)-110)/10, string, 5);   // val, string label, color scheme
								if (bandmem[curr_band].sp_ref_lvl > -120 || bandmem[curr_band].sp_ref_lvl < -200)
									tval = 0xFF0000;  // Change to red
									break;
			case NB_BTN:        sprintf(string, "  NB:%d", user_settings[user_Profile].nb_level);
								MeterInUse = true;
								displayMeter(user_settings[user_Profile].nb_level, string, 5);   // val, string label, color scheme
								if (user_settings[user_Profile].nb_level >= 5 || user_settings[user_Profile].nb_level <=1)
									tval = 0xFF0000;  // Change to red
									break;
			default: obj->writeRGBCode(tval); break;
		}
	}
	obj->writeRGBCode(tval);  // set color
}

//Callback when the encoder is pushed
COLD void encoder_click(i2cEncoderLibV2* obj) 
{
	if (obj->id == user_settings[user_Profile].encoder1_client && press_timer.check() == 1)
	{
		//VFO_AB();
		//Serial.println(F("Long MF Knob Push- Swap VFOs "));
		//obj->writeRGBCode(0x00FF00);
	}
	else if (obj->id == user_settings[user_Profile].encoder1_client)
	{
		Rate(0);
		Serial.println(F("MF Knob Push to change Tune Rate "));
		obj->writeRGBCode(0xFF0000);
	}
	else
	{

		Serial.println(F("Push: "));
		obj->writeRGBCode(0x0000FF);
	}
	
}

//Callback when the encoder is first pushed, will start a timer to see if it was long or short
COLD void encoder_timer_start(i2cEncoderLibV2* obj) {
	Serial.println(F("Push Timer Start: "));
	obj->writeRGBCode(0x0000FF);
	press_timer.reset();
}

//Callback when the encoder reach the max or min
COLD void encoder_thresholds(i2cEncoderLibV2* obj) 
{
	if (obj->readStatus(i2cEncoderLibV2::RMAX))
		Serial.println(F("Max!"));
	else
		Serial.println(F("Min!"));
	obj->writeRGBCode(0xFF0000);
}

//Callback when the fading process finish and set the RGB led off
COLD void encoder_fade(i2cEncoderLibV2* obj) 
{
	if (obj->id == user_settings[user_Profile].encoder1_client && press_timer.check() == 1)
	{
		VFO_AB();
		Serial.println(F("Long MF Knob Push- Swap VFOs "));
		//obj->writeRGBCode(0x00FF00);
	}
  	obj->writeRGBCode(0x000000);
}

COLD void set_I2CEncoders()
{
    pinMode(I2C_INT_PIN, INPUT_PULLUP);
    Serial.println(F("Setup ENC"));

	#ifdef MF_ENC_ADDR
    // MF KNOB - Multi-Function knob setup.
	if(user_settings[user_Profile].encoder1_client)  // 0 is no encoder assigned so skip this
	{
		Serial.println(F("MF Encoder Setup"));
		MF_ENC.reset();
		delay(20);
		MF_ENC.begin(
			i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
			| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_DISABLE // Pullup is on the Teensy IO pin
			| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
		MF_ENC.id = user_settings[user_Profile].encoder1_client;
		MF_ENC.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
		MF_ENC.writeMax((int32_t) 100); /* Set the maximum threshold*/
		MF_ENC.writeMin((int32_t) -100); /* Set the minimum threshold */
		MF_ENC.writeStep((int32_t) 1); /* Set the step to 1*/
		/* Configure the events */
		MF_ENC.onChange = encoder_rotated;
		MF_ENC.onButtonRelease = encoder_click;
		MF_ENC.onMinMax = encoder_thresholds;
		MF_ENC.onFadeProcess = encoder_fade;
		MF_ENC.onButtonPush = encoder_timer_start;
		MF_ENC.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
		//MF_ENC.writeInterruptConfig(0xff); /* Enable all the interrupt */
		//MF_ENC.writeDoublePushPeriod(50); /*Set a period for the double push of 500ms */
		/* Enable the I2C Encoder V2 interrupts according to the previous attached callback */
		MF_ENC.autoconfigInterrupt();
		blink_MF_RGB();
	}
	#endif

	// Setup for other encoders. Uses the button number from the user settings database
	#ifdef ENC2_ADDR
	// Encoder 2 setup
	if(user_settings[user_Profile].encoder2_client)  // 0 if no encoder assigned so skip this
    {
		Serial.println(F("Encoder #2 Setup"));
		ENC2.reset();
		delay(20);
		ENC2.begin(
			i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
			| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_DISABLE  // Pullup is on the Teensy IO pin
			| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
		ENC2.id = user_settings[user_Profile].encoder2_client;   
		ENC2.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
		ENC2.writeMax((int32_t) 100); /* Set the maximum threshold*/
		ENC2.writeMin((int32_t) -100); /* Set the minimum threshold */
		ENC2.writeStep((int32_t) 1); /* Set the step to 1*/
		/* Configure the events */
		ENC2.onChange = encoder_rotated;
		ENC2.onButtonRelease = encoder_click;
		ENC2.onMinMax = encoder_thresholds;
		ENC2.onFadeProcess = encoder_fade;
		ENC2.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
		ENC2.autoconfigInterrupt();
		blink_ENC2_RGB();
	}
	#endif

	#ifdef ENC3_ADDR
	// Encoder 3 setup
	if(user_settings[user_Profile].encoder3_client)  // 0 if no encoder assigned so skip this
    {
		Serial.println(F("Encoder #3 Setup"));
		ENC3.reset();
		delay(20);
		ENC3.begin(
			i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
			| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_DISABLE  // Pullup is on the Teensy IO pin
			| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
		ENC3.id = user_settings[user_Profile].encoder3_client;   
		ENC3.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
		ENC3.writeMax((int32_t) 100); /* Set the maximum threshold*/
		ENC3.writeMin((int32_t) -100); /* Set the minimum threshold */
		ENC3.writeStep((int32_t) 1); /* Set the step to 1*/
		/* Configure the events */
		ENC3.onChange = encoder_rotated;
		ENC3.onButtonRelease = encoder_click;
		ENC3.onMinMax = encoder_thresholds;
		ENC3.onFadeProcess = encoder_fade;
		ENC3.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
		ENC3.autoconfigInterrupt();
		blink_ENC3_RGB();
	}
	#endif
}

#ifdef MF_ENC_ADDR
COLD void blink_MF_RGB(void)
{
    /* blink the RGB LED */
    MF_ENC.writeRGBCode(0xFF0000);
    delay(250);
    MF_ENC.writeRGBCode(0x00FF00);
    delay(250);
    MF_ENC.writeRGBCode(0x0000FF);
    delay(250);
    MF_ENC.writeRGBCode(0x000000);
	Serial.println(F("Blink MF RGB"));
    MF_ENC.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#ifdef ENC2_ADDR
COLD void blink_ENC2_RGB(void)
{
    /* blink the RGB LED */
    ENC2.writeRGBCode(0xFF0000);
    delay(250);
    ENC2.writeRGBCode(0x00FF00);
    delay(250);
    ENC2.writeRGBCode(0x0000FF);
    delay(250);
    ENC2.writeRGBCode(0x000000);
	Serial.println(F("Blink ENC2 RGB"));
    ENC2.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#ifdef ENC3_ADDR
COLD void blink_ENC3_RGB(void)
{	
    /* blink the RGB LED */
    ENC3.writeRGBCode(0xFF0000);
    delay(250);
    ENC3.writeRGBCode(0x00FF00);
    delay(250);
    ENC3.writeRGBCode(0x0000FF);
    delay(250);
    ENC3.writeRGBCode(0x000000);
	Serial.println(F("Blink ENC3 RGB"));
    ENC3.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#endif // I2C_ENCODER
