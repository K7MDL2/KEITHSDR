//
//     SDR_I2C_Encoder.cpp
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"
//#include "SDR_I2C_Encoder.h"

//  In RadioConfig.h use   #define USE_MIDI to enable MIDI 	-  
//  Experimental dev work to use Teensy SDR controls to send out MIDI events over USB
#ifdef USE_MIDI
	#include "MIDIUSB.h"

	#define AUX_PIN     	6
	#define KEY_PIN     	7
	#define MUTE        	57
  	#define VFOA     		50
  	#define VFOB     		51
	#define VELOCITY    	127
	#define CHANNEL     	0
  	#define CHANNEL4     	4
	
	// First parameter is the event type (0x09 = note on, 0x08 = note off).
	// Second parameter is note-on/note-off, combined with the channel.
	// Channel can be anything between 0-15. Typically reported to the user as 1-16.
	// Third parameter is the note number.
	// Fourth parameter is the velocity (64 = normal, 127 = fastest).

	void noteOn(uint8_t channel, byte pitch, byte velocity) 
	{
		midiEventPacket_t noteOn = {0x09, uint8_t (0x90 | channel), pitch, velocity};
		MidiUSB.sendMIDI(noteOn);
		MidiUSB.flush();
	}

	void noteOff(uint8_t channel, byte pitch, byte velocity) 
	{
		midiEventPacket_t noteOff = {0x0B, uint8_t (0xB0 | channel), pitch, velocity};
		MidiUSB.sendMIDI(noteOff);
		MidiUSB.flush();
	}

  	void note(uint8_t channel, byte cmd_byte, byte cmd_val) 
	{
		midiEventPacket_t note = {0x0B, uint8_t (0xB0 | channel), cmd_byte, cmd_val};
		MidiUSB.sendMIDI(note);
		MidiUSB.flush();
	}

#endif

#ifdef I2C_ENCODERS

#include <i2cEncoderLibV2.h>
// These are the per-encoder function declarations
void set_I2CEncoders(void); 
extern void setEncoderMode(uint8_t id);
extern uint8_t MF_client;     // Flag for current owner of MF knob services
extern uint8_t curr_band;     // global tracks our current band setting. 
extern uint8_t user_Profile;  // global tracks our current user profile
extern struct User_Settings user_settings[];
extern struct Band_Memory bandmem[];
extern struct Label labels[];
extern struct EncoderList encoder_list[];
extern bool MeterInUse;  // S-meter flag to block updates while the MF knob has control
extern Metro MF_Timeout;

Metro press_timer1 = Metro(500);
Metro press_timer2 = Metro(500);
Metro press_timer3 = Metro(500);
Metro press_timer4 = Metro(500);
Metro press_timer5 = Metro(500);
Metro press_timer6 = Metro(500);

//Class initialization with the I2C addresses - add more here if needed
//i2cEncoderLibV2 i2c_encoder[2] = { i2cEncoderLibV2(0x62), i2cEncoderLibV2(0x61)};
#ifdef I2C_ENC1_ADDR
	void blink_I2C_ENC1_RGB(void);
	i2cEncoderLibV2 I2C_ENC1(I2C_ENC1_ADDR);  	/* Address 0x61 only - Jumpers A0, A5 and A6 are soldered.*/
#endif
#ifdef I2C_ENC2_ADDR
	void blink_I2C_ENC2_RGB(void);
	i2cEncoderLibV2 I2C_ENC2(I2C_ENC2_ADDR);  	/* Address 0x62 only - Jumpers A1, A5 and A6 are soldered.*/
#endif
#ifdef I2C_ENC3_ADDR
	void blink_I2C_ENC3_RGB(void);
	i2cEncoderLibV2 I2C_ENC3(I2C_ENC3_ADDR);  	/* Address 0x63 only - Jumpers A0, A1, A5 and A6 are soldered.*/
#endif
#ifdef I2C_ENC4_ADDR
	void blink_I2C_ENC4_RGB(void);
	i2cEncoderLibV2 I2C_ENC4(I2C_ENC4_ADDR);  	/* Address 0x64 only - Jumpers A0, A1, A5 and A6 are soldered.*/
#endif
#ifdef I2C_ENC5_ADDR
	void blink_I2C_ENC5_RGB(void);
	i2cEncoderLibV2 I2C_ENC5(I2C_ENC5_ADDR);  	/* Address 0x65 only - Jumpers A0, A1, A5 and A6 are soldered.*/
#endif
#ifdef I2C_ENC6_ADDR
	void blink_I2C_ENC6_RGB(void);
	i2cEncoderLibV2 I2C_ENC6(I2C_ENC6_ADDR);  	/* Address 0x66 only - Jumpers A0, A1, A5 and A6 are soldered.*/
#endif

// These are generic callback functions - meaning when a hardware event occurs these functions are 
// called with the info associated with that encoder.  We can assign each encoder to things like AF and RF gain.
void encoder_rotated(i2cEncoderLibV2* obj);
void encoder_click(i2cEncoderLibV2* obj);
void encoder_thresholds(i2cEncoderLibV2* obj);

extern void MF_Service(int8_t counts, uint8_t knob);

//Callback when the MF Gain encoder is rotated
COLD void encoder_rotated(i2cEncoderLibV2* obj) 
{
	uint8_t knob_assigned, z_lvl, slot;

	//DPRINT(F("Encoder ID passed in = ")); DPRINTLN(obj->id);
	//DPRINT(F("ID from Record lookup = ")); DPRINTLN(encoder_list[obj->id].id);
	//DPRINT(F("Check MF Client ID = ")); DPRINTLN(MF_client);
	//DPRINT(F("Role = ")); DPRINTLN(encoder_list[obj->id].role_A);
	    
	for (slot = 1; slot< NUM_AUX_ENCODERS; slot++)
	{
		//DPRINT(F("Slot # = ")); DPRINTLN(slot);
		//DPRINT(F("id from slot # = ")); DPRINTLN(encoder_list[slot].id);
		if ((obj->id == encoder_list[slot].id) && encoder_list[slot].enabled)
		{	
			//DPRINTLN(F("Valid Encoder Match"));
			break;
		} 
	}  // got our slot number
	//DPRINT(F("slot is ")); DPRINTLN(slot);
	//DPRINT(F("def is ")); DPRINTLN(encoder_list[slot].default_MF_client);
	
	if ((encoder_list[slot].role_A  == encoder_list[slot].default_MF_client) && encoder_list[slot].role_A != MF_client)		
			knob_assigned = MF_client; 
	else 
		knob_assigned = encoder_list[slot].role_A;
	
	if (obj->readStatus(i2cEncoderLibV2::RINC))
		{}//DPRINT(F("Increment: "));
	else
		{}//DPRINT(F("Decrement: "));
	int16_t count = obj->readCounterInt();
	//DPRINTLN(count);

	// ID arrives as Role_A.  Need to check which role is active. If B active, update the knob, else skip.
	// If a button tap (switch) happens and it is ENCx_BTN, setEncoderMode() is called in control.cpp.  
	// The function assigned to that encoder shaft is set to the alternate function (a vs b) and the unset staus s set to 0. 
	if (knob_assigned == encoder_list[slot].role_A && encoder_list[slot].enabled && !encoder_list[slot].a_active)
	{
		knob_assigned = encoder_list[slot].role_B;  // reassign to role B
		DPRINT(F("Role B Assigned ")); DPRINTLN(encoder_list[slot].role_B);
	}
	
	MF_Service(count, knob_assigned);
	//obj->writeCounter((int32_t) 0); // Reset the counter value if in absolute mode. Not required in relative mode
	// Update the color
	uint32_t tval = 0x00FF00;  // Set the default color to green
	DPRINT(F("Knob Assigned to ")); DPRINTLN(knob_assigned);

	switch(knob_assigned)
	{
		char string[80];   // print format stuff
		case RFGAIN_BTN:    sprintf(string, " RF:%d", user_settings[user_Profile].rfGain);
							MeterInUse = true;
							displayMeter(user_settings[user_Profile].rfGain/10, string, 5);   // val, string label, color scheme
							if (user_settings[user_Profile].rfGain < 2 || user_settings[user_Profile].rfGain >98)								
								tval = 0xFF0000;  // Change to red
							break;
		case AFGAIN_BTN:    sprintf(string, " AF:%d", user_settings[user_Profile].afGain);
							MeterInUse = true;
							displayMeter(user_settings[user_Profile].afGain/10, string, 5);   // val, string label, color scheme
							if (user_settings[user_Profile].afGain < 2 || user_settings[user_Profile].afGain >98)
								tval = 0xFF0000;  // Change to red
							//AFgain(0);
							#ifdef USE_MIDI
								note(CHANNEL, 52, (user_settings[user_Profile].afGain * 1.27));  // scale 100% to 127 for MIDI max of 127.
							#endif  
							break;
		case ATTEN_BTN:     sprintf(string, " ATT:%d", bandmem[curr_band].attenuator_dB);
							MeterInUse = true;
							displayMeter(bandmem[curr_band].attenuator_dB/10, string, 5);   // val, string label, color scheme
							if (bandmem[curr_band].attenuator_dB < 2  || bandmem[curr_band].attenuator_dB > 98)
								tval = 0xFF0000;  // Change to red
							//Atten(0);
							break;
		case REFLVL_BTN:   	sprintf(string, "Lvl:%d", bandmem[curr_band].sp_ref_lvl);
							MeterInUse = true; 
							displayMeter((bandmem[curr_band].sp_ref_lvl+50)/10, string, 5);   // val, string label, color scheme
							if (bandmem[curr_band].sp_ref_lvl < -48 || bandmem[curr_band].sp_ref_lvl > 48)
								tval = 0xFF0000;  // Change to red
							//RefLevel(0);
							break;
		case NB_BTN:   		sprintf(string, "  NB:%1d", user_settings[user_Profile].nb_level);
							MeterInUse = true;
							displayMeter((int) user_settings[user_Profile].nb_level*1.7, string, 5);   // val, string label, color scheme 
							if (user_settings[user_Profile].nb_level >= 6 || user_settings[user_Profile].nb_level <=0)
								tval = 0xFF0000;  // Change to red
							//NBLevel(0);
							break;
		case PAN_BTN:       sprintf(string, "Pan:%3d", user_settings[user_Profile].pan_level-50);
							MeterInUse = true;
							displayMeter(user_settings[user_Profile].pan_level/10, string, 5);   // val, string label, color scheme
							if (user_settings[user_Profile].pan_level < 2 || user_settings[user_Profile].pan_level > 98)
								tval = 0xFF0000;  // Change to red
							break;
		case ZOOM_BTN:      z_lvl = user_settings[user_Profile].zoom_level;
							if (z_lvl == 0)
								z_lvl = 1;
							else
								z_lvl *= 2;
							sprintf(string, "Zoom:%1d",z_lvl);
							MeterInUse = true;
							displayMeter(z_lvl, string, 5);   // val, string label, color scheme
							if (z_lvl < 2 || z_lvl > 3)
							tval = 0xFF0000;  // Change to red
							break;
		case FILTER_BTN:	if (bandmem[curr_band].filter < 1 || bandmem[curr_band].filter >= FILTER-1)
							tval = 0xFF0000;  // Change to red
							break;
		case RATE_BTN:      if (bandmem[curr_band].tune_step < 1 || bandmem[curr_band].tune_step >= TS_STEPS-1)
							tval = 0xFF0000;  // Change to red
							break;
		case MODE_BTN:      if (bandmem[curr_band].mode_A < 1 || bandmem[curr_band].mode_A >= MODES_NUM-1)
							tval = 0xFF0000;  // Change to red
							break;
		case ATU_BTN:       if (bandmem[curr_band].ATU < 1 || bandmem[curr_band].ATU > 0)
							tval = 0xFF0000;  // Change to red
							break;
		case ANT_BTN:       if (bandmem[curr_band].ant_sw < 1 || bandmem[curr_band].ant_sw > 0)
							tval = 0xFF0000;  // Change to red
							break;
		case BANDUP_BTN:    if (bandmem[curr_band].ant_sw < 1 || bandmem[curr_band].ant_sw > 0)
							tval = 0xFF0000;  // Change to red
							break;
		case BANDDN_BTN:    if (bandmem[curr_band].ant_sw < 1 || bandmem[curr_band].ant_sw > 0)
							tval = 0xFF0000;  // Change to red
							break;
		case BAND_BTN:      tval = 0xFF0000;  // Change to red
							break;
		default:  
							#ifdef USE_MIDI
								note(CHANNEL, 50, 64+count);   // MIDI jog wheel uses 64 as center
							#endif
							obj->writeRGBCode(tval); 
							break;
	}
	obj->writeRGBCode(tval);  // set color
}

void knob_press(i2cEncoderLibV2* obj, uint8_t slot)
{	
	DPRINT(F("Knob Press ")); DPRINTLN(encoder_list[slot].press);
	obj->writeRGBCode(0x00FF00);
	#ifdef USE_MIDI
		noteOn(CHANNEL, 62, 127);
		noteOff(CHANNEL, 62, 0);
	#else
		Button_Action(encoder_list[slot].press);
	#endif
}

void knob_tap(i2cEncoderLibV2* obj, uint8_t slot)
{
	DPRINT(F("Knob Tap ")); DPRINTLN(encoder_list[slot].tap);
	obj->writeRGBCode(0x0000FF);
	#ifdef USE_MIDI
		noteOn(CHANNEL, 63, 127);
		noteOff(CHANNEL, 63, 0);
	#else	
		Button_Action(encoder_list[slot].tap);
	#endif
}

//Callback when the encoder is pushed
COLD void encoder_click(i2cEncoderLibV2* obj) 
{   
	uint8_t slot;
	
	DPRINT(F("Click Event ")); DPRINTLN(obj->id);

    for (slot = 1; slot < NUM_AUX_ENCODERS; slot++)
	{
		if ((obj->id == encoder_list[slot].id) && encoder_list[slot].enabled)
		{
			uint8_t _press = 0;

			DPRINT(F("Slot ")); DPRINTLN(slot);
			switch (slot)
			{
				case 1: if (press_timer1.check() == 1) _press = 1; break;
				case 2: if (press_timer2.check() == 1) _press = 1; break;
				case 3: if (press_timer3.check() == 1) _press = 1; break;
				case 4: if (press_timer4.check() == 1) _press = 1; break;
				case 5: if (press_timer5.check() == 1) _press = 1; break;
				case 6: if (press_timer6.check() == 1) _press = 1; break;
			}
				
			if (_press)	knob_press(obj, slot);  // this is a tap, call the button action 
			else knob_tap(obj, slot);			// this is a tap, call the button action 
		}
	}	
}

//Callback when the encoder is first pushed, will start a timer to see if it was long or short
COLD void encoder_timer_start(i2cEncoderLibV2* obj) 
{
	uint8_t slot;
	DPRINT(F("Start Push Switch Timer ")); DPRINTLN(obj->id);
	obj->writeRGBCode(0x0000FF);
	
	for (slot = 1; slot < NUM_AUX_ENCODERS; slot++)
	{
		if ((obj->id == encoder_list[slot].id) && encoder_list[slot].enabled)
		{
			switch (slot)
			{
				case 1: press_timer1.reset(); DPRINTLN(F("Start Timer 1")); break;
				case 2: press_timer2.reset(); DPRINTLN(F("Start Timer 2")); break;
				case 3: press_timer3.reset(); DPRINTLN(F("Start Timer 3")); break;
				case 4: press_timer4.reset(); DPRINTLN(F("Start Timer 4")); break;
				case 5: press_timer5.reset(); DPRINTLN(F("Start Timer 5")); break;
				case 6: press_timer6.reset(); DPRINTLN(F("Start Timer 6")); break;
			}
		}
	}
}

//Callback when the encoder reaches the max or min
COLD void encoder_thresholds(i2cEncoderLibV2* obj) 
{
	if (obj->readStatus(i2cEncoderLibV2::RMAX))
		;//DPRINTLN(F("Max!"));
	else
		;//DPRINTLN(F("Min!"));
	obj->writeRGBCode(0xFF0000);
}

//Callback when the fading process finishes and set the RGB led off
COLD void encoder_fade(i2cEncoderLibV2* obj) 
{
	//uint8_t mfg; 
	#ifdef I2C_ENC1_ADDR
		I2C_ENC1.updateStatus();
	#endif
	//mfg = MF_ENC.readStatus();
	//DPRINT(F("****Checked MF_Enc (in FADE) status = ")); DPRINTLN(mfg);
	//#ifdef MF_ENC_ADDR
	// Check the status of the encoder (if enabled) and call the callback
	//if(mfg == 0 && press_timer.check() == 1 && obj->id == user_settings[user_Profile].encoder1_client && user_settings[user_Profile].encoder1_client == MFTUNE)
	//{     
		//VFO_AB();
		//DPRINTLN(F("Long MF Knob Push- Swap VFOs "));
		//obj->writeRGBCode(0x00FF00);
	//}
	//#endif
  	obj->writeRGBCode(0x000000);
}

COLD void set_I2CEncoders()
{
	uint8_t slot = 0;

	pinMode(I2C_INT_PIN, INPUT_PULLUP);
    //DPRINTLN(F("Setup ENC"));

	#ifdef I2C_ENC1_ADDR
    	// find the slot assigned to I2C_ENC1_ADDR and if enabled, set it up
		for (slot = 1; slot < NUM_AUX_ENCODERS; slot++)
		{
			if (encoder_list[slot].enabled == I2C_ENC1_ENABLE  && encoder_list[slot].type == I2C_ENC)
			{
				DPRINT(F("I2C_ENC1 Encoder Setup Slot "));DPRINTLN(slot);
				I2C_ENC1.reset();
				delay(20);
				I2C_ENC1.begin(
					i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
					| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_DISABLE // Pullup is on the Teensy IO pin
					| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
				I2C_ENC1.id = encoder_list[slot].id;
				I2C_ENC1.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
				I2C_ENC1.writeMax((int32_t) 100); /* Set the maximum threshold*/
				I2C_ENC1.writeMin((int32_t) -100); /* Set the minimum threshold */
				I2C_ENC1.writeStep((int32_t) 1); /* Set the step to 1*/
				/* Configure the events */
				I2C_ENC1.onChange = encoder_rotated;
				I2C_ENC1.onButtonRelease = encoder_click;
				I2C_ENC1.onMinMax = encoder_thresholds;
				I2C_ENC1.onFadeProcess = encoder_fade;
				I2C_ENC1.onButtonPush = encoder_timer_start;
				I2C_ENC1.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
				//MF_ENC.writeInterruptConfig(0xff); /* Enable all the interrupt */
				//MF_ENC.writeDoublePushPeriod(50); /*Set a period for the double push of 500ms */
				/* Enable the I2C Encoder V2 interrupts according to the previous attached callback */
				I2C_ENC1.autoconfigInterrupt();
				blink_I2C_ENC1_RGB();
				//DPRINTLN(F("End Encoder #1 Setup"));
				break;  // now have the record for this encoder
			}
		}
	#endif

	// Setup for other encoders. Uses the button number from the user settings database
	#ifdef I2C_ENC2_ADDR
		// Encoder 2 setup
		// find the slot assigned to I2C_ENC1_ADDR and if enabled, set it up
		for (slot = 1; slot < NUM_AUX_ENCODERS; slot++)
		{
			if (encoder_list[slot].enabled == I2C_ENC2_ENABLE && encoder_list[slot].type == I2C_ENC)
			{
				DPRINT(F("I2C_ENC2 Encoder Setup Slot "));DPRINTLN(slot);
				I2C_ENC2.reset();
				delay(20);
				I2C_ENC2.begin(
					i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
					| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_DISABLE  // Pullup is on the Teensy IO pin
					| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
				I2C_ENC2.id = encoder_list[slot].id;   
				I2C_ENC2.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
				I2C_ENC2.writeMax((int32_t) 100); /* Set the maximum threshold*/
				I2C_ENC2.writeMin((int32_t) -100); /* Set the minimum threshold */
				I2C_ENC2.writeStep((int32_t) 1); /* Set the step to 1*/
				/* Configure the events */
				I2C_ENC2.onChange = encoder_rotated;
				I2C_ENC2.onButtonRelease = encoder_click;
				I2C_ENC2.onMinMax = encoder_thresholds;
				I2C_ENC2.onFadeProcess = encoder_fade;
				I2C_ENC2.onButtonPush = encoder_timer_start;
				I2C_ENC2.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
				I2C_ENC2.autoconfigInterrupt();
				blink_I2C_ENC2_RGB();
				//DPRINTLN(F("End Encoder #2 Setup"));
				break;  // now have the record for this encoder
			}
		}
	#endif

	#ifdef I2C_ENC3_ADDR
		// Encoder 3 setup
		// find the slot assigned to I2C_ENC1_ADDR and if enabled, set it up
		for (slot = 1; slot < NUM_AUX_ENCODERS; slot++)
		{
			if (encoder_list[slot].enabled == I2C_ENC3_ENABLE && encoder_list[slot].type == I2C_ENC)
			{	
				DPRINT(F("I2C_ENC3 Encoder Setup Slot "));DPRINTLN(slot);
				I2C_ENC3.reset();
				delay(20);
				I2C_ENC3.begin(
					i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
					| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_ENABLE  // Pullup is on the Teensy IO pin
					| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_ENABLE  | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
				I2C_ENC3.id = encoder_list[slot].id;   
				I2C_ENC3.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
				I2C_ENC3.writeMax((int32_t) 100); /* Set the maximum threshold*/
				I2C_ENC3.writeMin((int32_t) -100); /* Set the minimum threshold */
				I2C_ENC3.writeStep((int32_t) 1); /* Set the step to 1*/
				/* Configure the events */
				I2C_ENC3.onChange = encoder_rotated;
				I2C_ENC3.onButtonRelease = encoder_click;
				I2C_ENC3.onMinMax = encoder_thresholds;
				I2C_ENC3.onFadeProcess = encoder_fade;
				I2C_ENC3.onButtonPush = encoder_timer_start;
				I2C_ENC3.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
				I2C_ENC3.autoconfigInterrupt();
				blink_I2C_ENC3_RGB();
				//DPRINTLN(F("End Encoder #3 Setup"));
				break;  // now have the record for this encoder
			}
		}
	#endif

	#ifdef I2C_ENC4_ADDR
		// Encoder 4 setup
    	// find the slot assigned to I2C_ENC1_ADDR and if enabled, set it up
		for (slot = 1; slot < NUM_AUX_ENCODERS; slot++)
		{
			if (encoder_list[slot].enabled == I2C_ENC4_ENABLE && encoder_list[slot].type == I2C_ENC)
			{
				DPRINT(F("I2C_ENC4 Encoder Setup Slot "));DPRINTLN(slot);
				I2C_ENC4.reset();
				delay(20);
				I2C_ENC4.begin(
					i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
					| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_ENABLE  // Pullup is on the Teensy IO pin
					| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
				I2C_ENC4.id = encoder_list[slot].id;   
				I2C_ENC4.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
				I2C_ENC4.writeMax((int32_t) 100); /* Set the maximum threshold*/
				I2C_ENC4.writeMin((int32_t) -100); /* Set the minimum threshold */
				I2C_ENC4.writeStep((int32_t) 1); /* Set the step to 1*/
				/* Configure the events */
				I2C_ENC4.onChange = encoder_rotated;
				I2C_ENC4.onButtonRelease = encoder_click;
				I2C_ENC4.onMinMax = encoder_thresholds;
				I2C_ENC4.onFadeProcess = encoder_fade;
				I2C_ENC4.onButtonPush = encoder_timer_start;
				I2C_ENC4.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
				I2C_ENC4.autoconfigInterrupt();
				blink_I2C_ENC4_RGB();
				//DPRINTLN(F("End Encoder #4 Setup"));
				break;
			}
		}
	#endif

	#ifdef I2C_ENC5_ADDR
		// Encoder 5 setup
		// find the slot assigned to I2C_ENC1_ADDR and if enabled, set it up
		for (slot = 1; slot < NUM_AUX_ENCODERS; slot++)
		{
			if (encoder_list[slot].enabled == I2C_ENC5_ENABLE && encoder_list[slot].type == I2C_ENC)
			{
				DPRINT(F("I2C_ENC5 Encoder Setup Slot "));DPRINTLN(slot);
				I2C_ENC5.reset();
				delay(20);
				I2C_ENC5.begin(
					i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
					| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_DISABLE  // Pullup is on the Teensy IO pin
					| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
				I2C_ENC5.id = encoder_list[slot].id;   
				I2C_ENC5.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
				I2C_ENC5.writeMax((int32_t) 100); /* Set the maximum threshold*/
				I2C_ENC5.writeMin((int32_t) -100); /* Set the minimum threshold */
				I2C_ENC5.writeStep((int32_t) 1); /* Set the step to 1*/
				/* Configure the events */
				I2C_ENC5.onChange = encoder_rotated;
				I2C_ENC5.onButtonRelease = encoder_click;
				I2C_ENC5.onMinMax = encoder_thresholds;
				I2C_ENC5.onFadeProcess = encoder_fade;
				I2C_ENC5.onButtonPush = encoder_timer_start;
				I2C_ENC5.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
				I2C_ENC5.autoconfigInterrupt();
				blink_I2C_ENC5_RGB();
				//DPRINTLN(F("End Encoder #5 Setup"));
				break;
			}
		}
	#endif

	#ifdef I2C_ENC6_ADDR
		// Encoder 6 setup
    	// find the slot assigned to I2C_ENC1_ADDR and if enabled, set it up
		for (slot = 1; slot < NUM_AUX_ENCODERS; slot++)
		{
			if (encoder_list[slot].enabled == I2C_ENC6_ENABLE && encoder_list[slot].type == I2C_ENC)
			{
				DPRINT(F("I2C_ENC6 Encoder Setup Slot "));DPRINTLN(slot);
				I2C_ENC6.reset();
				delay(20);
				I2C_ENC6.begin(
					i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
					| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_DISABLE  // Pullup is on the Teensy IO pin
					| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
				//  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
				I2C_ENC6.id = encoder_list[slot].id;   
				I2C_ENC6.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
				I2C_ENC6.writeMax((int32_t) 100); /* Set the maximum threshold*/
				I2C_ENC6.writeMin((int32_t) -100); /* Set the minimum threshold */
				I2C_ENC6.writeStep((int32_t) 1); /* Set the step to 1*/
				/* Configure the events */
				I2C_ENC6.onChange = encoder_rotated;
				I2C_ENC6.onButtonRelease = encoder_click;
				I2C_ENC6.onMinMax = encoder_thresholds;
				I2C_ENC6.onFadeProcess = encoder_fade;
				I2C_ENC6.onButtonPush = encoder_timer_start;
				I2C_ENC6.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
				I2C_ENC6.autoconfigInterrupt();
				blink_I2C_ENC6_RGB();
				//DPRINTLN(F("End Encoder #6 Setup"));
				break;
			}
		}
	#endif
}

#ifdef I2C_ENC1_ADDR
COLD void blink_I2C_ENC1_RGB(void)
{
    /* blink the RGB LED */
    I2C_ENC1.writeRGBCode(0xFF0000);
    delay(250);
    I2C_ENC1.writeRGBCode(0x00FF00);
    delay(250);
    I2C_ENC1.writeRGBCode(0x0000FF);
    delay(250);
    I2C_ENC1.writeRGBCode(0x000000);
	DPRINTLN(F("Blink I2C_ENC1 RGB"));
    I2C_ENC1.writeFadeRGB(2); //Fade enabled with 2ms step
}
#endif

#ifdef I2C_ENC2_ADDR
COLD void blink_I2C_ENC2_RGB(void)
{
    /* blink the RGB LED */
    I2C_ENC2.writeRGBCode(0xFF0000);
    delay(250);
    I2C_ENC2.writeRGBCode(0x00FF00);
    delay(250);
    I2C_ENC2.writeRGBCode(0x0000FF);
    delay(250);
    I2C_ENC2.writeRGBCode(0x000000);
	DPRINTLN(F("Blink I2C_ENC2 RGB"));
    I2C_ENC2.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#ifdef I2C_ENC3_ADDR
COLD void blink_I2C_ENC3_RGB(void)
{	
    /* blink the RGB LED */
    I2C_ENC3.writeRGBCode(0xFF0000);
    delay(250);
    I2C_ENC3.writeRGBCode(0x00FF00);
    delay(250);
    I2C_ENC3.writeRGBCode(0x0000FF);
    delay(250);
    I2C_ENC3.writeRGBCode(0x000000);
	DPRINTLN(F("Blink I2C_ENC3 RGB"));
    I2C_ENC3.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#ifdef I2C_ENC4_ADDR
COLD void blink_I2C_ENC4_RGB(void)
{	
    /* blink the RGB LED */
    I2C_ENC4.writeRGBCode(0xFF0000);
    delay(250);
    I2C_ENC4.writeRGBCode(0x00FF00);
    delay(250);
    I2C_ENC4.writeRGBCode(0x0000FF);
    delay(250);
    I2C_ENC4.writeRGBCode(0x000000);
	DPRINTLN(F("Blink I2C_ENC4 RGB"));
    I2C_ENC4.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#ifdef I2C_ENC5_ADDR
COLD void blink_I2C_ENC5_RGB(void)
{	
    /* blink the RGB LED */
    I2C_ENC5.writeRGBCode(0xFF0000);
    delay(250);
    I2C_ENC5.writeRGBCode(0x00FF00);
    delay(250);
    I2C_ENC5.writeRGBCode(0x0000FF);
    delay(250);
    I2C_ENC5.writeRGBCode(0x000000);
	DPRINTLN(F("Blink I2C_ENC5 RGB"));
    I2C_ENC5.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#ifdef I2C_ENC6_ADDR
COLD void blink_I2C_ENC6_RGB(void)
{	
    /* blink the RGB LED */
    I2C_ENC6.writeRGBCode(0xFF0000);
    delay(250);
    I2C_ENC6.writeRGBCode(0x00FF00);
    delay(250);
    I2C_ENC6.writeRGBCode(0x0000FF);
    delay(250);
    I2C_ENC6.writeRGBCode(0x000000);
	DPRINTLN(F("Blink I2C_ENC6 RGB"));
    I2C_ENC6.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#endif // I2C_ENCODER
