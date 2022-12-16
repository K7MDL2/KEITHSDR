//
//     SDR_I2C_Encoder.cpp
//
#include "SDR_RA8875.h"
#include "RadioConfig.h"
//#include "SDR_I2C_Encoder.h"

//  In RadioCOnfig.h use   #define USE_MIDI to enable MIDI 	-  
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
extern uint8_t MF_client;     // Flag for current owner of MF knob services
extern uint8_t curr_band;     // global tracks our current band setting. 
extern uint8_t user_Profile;  // global tracks our current user profile
extern struct User_Settings user_settings[];
extern struct Band_Memory bandmem[];
extern struct Label labels[];
extern bool MeterInUse;  // S-meter flag to block updates while the MF knob has control
extern Metro MF_Timeout;
Metro press_timer  = Metro(600);
Metro press_timer2 = Metro(600);
Metro press_timer3 = Metro(600);
Metro press_timer4 = Metro(600);
Metro press_timer5 = Metro(600);
Metro press_timer6 = Metro(600);
extern bool ENC1b_active;  // ENCxb is the alternate encoder shaft function. A switch push toggles between the primary and alternate functions.
extern bool ENC2b_active;
extern bool ENC3b_active;
extern bool ENC4b_active;
extern bool ENC5b_active;
extern bool ENC6b_active;

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
#ifdef ENC4_ADDR
	void blink_ENC4_RGB(void);
	i2cEncoderLibV2 ENC4(ENC4_ADDR);  	/* Address 0x64 only - Jumpers A0, A1, A5 and A6 are soldered.*/
#endif
#ifdef ENC5_ADDR
	void blink_ENC5_RGB(void);
	i2cEncoderLibV2 ENC5(ENC5_ADDR);  	/* Address 0x65 only - Jumpers A0, A1, A5 and A6 are soldered.*/
#endif
#ifdef ENC6_ADDR
	void blink_ENC6_RGB(void);
	i2cEncoderLibV2 ENC6(ENC6_ADDR);  	/* Address 0x66 only - Jumpers A0, A1, A5 and A6 are soldered.*/
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
	uint8_t knob_assigned, z_lvl;

	//DPRINT(F("Encoder ID = "));
    //DPRINTLN(obj->id);

	if (obj->id == user_settings[user_Profile].encoder1_client)
		knob_assigned = MF_client; 
	else
		knob_assigned = obj->id;

	if (obj->readStatus(i2cEncoderLibV2::RINC))
		{}//DPRINT(F("Increment: "));
	else
		{}//DPRINT(F("Decrement: "));
	int16_t count = obj->readCounterInt();
	//DPRINTLN(count);

	// reassign knob functionality to alternate mode if alternate active
	if (ENC1b_active && knob_assigned == user_settings[user_Profile].encoder1_client)
		knob_assigned = user_settings[user_Profile].encoder1_clientb;
	if (ENC2b_active && knob_assigned == user_settings[user_Profile].encoder2_client)
		knob_assigned = user_settings[user_Profile].encoder2_clientb;
	if (ENC3b_active && knob_assigned == user_settings[user_Profile].encoder3_client)
		knob_assigned = user_settings[user_Profile].encoder3_clientb;
	if (ENC4b_active && knob_assigned == user_settings[user_Profile].encoder4_client)
		knob_assigned = user_settings[user_Profile].encoder4_clientb;
	if (ENC5b_active && knob_assigned == user_settings[user_Profile].encoder5_client)
		knob_assigned = user_settings[user_Profile].encoder5_clientb;
	if (ENC6b_active && knob_assigned == user_settings[user_Profile].encoder6_client)
		knob_assigned = user_settings[user_Profile].encoder6_clientb;
	
	MF_Service(count, knob_assigned);
	//obj->writeCounter((int32_t) 0); // Reset the counter value if in absolute mode. Not required in relative mode
	// Update the color
	uint32_t tval = 0x00FF00;  // Set the default color to green
	//DPRINT(F("Knob Assigned to "));
	//DPRINTLN(knob_assigned);
	
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
		case FILTER_BTN:    labels[FILTER_BTN].outline_color = CYAN;
							labels[MODE_BTN].outline_color = BLACK;
							displayFilter();
							displayMode();
							if (bandmem[curr_band].filter < 1 || bandmem[curr_band].filter >= FILTER-1)
							tval = 0xFF0000;  // Change to red
							break;
		case RATE_BTN:      if (bandmem[curr_band].tune_step < 1 || bandmem[curr_band].tune_step >= TS_STEPS-1)
							tval = 0xFF0000;  // Change to red
							break;
		case MODE_BTN:      labels[MODE_BTN].outline_color = CYAN;
							labels[FILTER_BTN].outline_color = BLACK;
							displayFilter();
							displayMode();
							if (bandmem[curr_band].mode_A < 1 || bandmem[curr_band].mode_A >= MODES_NUM-1)
							tval = 0xFF0000;  // Change to red
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
								
//Callback when the encoder is pushed
COLD void encoder_click(i2cEncoderLibV2* obj) 
{
	if (obj->id == user_settings[user_Profile].encoder1_client && press_timer.check() == 1)
	{	
		DPRINTLN(F("Long MF Knob Push"));
		obj->writeRGBCode(0x00FF00);
		#ifdef USE_MIDI
			noteOn(CHANNEL, 62, 127);
			noteOff(CHANNEL, 62, 0);
		#else
			Button_Action(user_settings[user_Profile].encoder1_client_swl);
		#endif
	}
	else if (obj->id == user_settings[user_Profile].encoder1_client)
	{
		DPRINTLN(F("MF Knob Push"));
		obj->writeRGBCode(0xFF0000);
		#ifdef USE_MIDI
			noteOn(CHANNEL, 63, 127);
			noteOff(CHANNEL, 63, 0);
		#else	
			Button_Action(user_settings[user_Profile].encoder1_client_sw);
		#endif
	}
	else if (obj->id == user_settings[user_Profile].encoder2_client && press_timer2.check() == 1)
	{
		DPRINTLN(F("Knob #2 Long Push"));
		obj->writeRGBCode(0x00FF00);
		#ifdef USE_MIDI
			noteOn(CHANNEL, 64, 127);
			noteOff(CHANNEL, 64, 0);
		#else
			Button_Action(user_settings[user_Profile].encoder2_client_swl);
		#endif
	}
	else if (obj->id == user_settings[user_Profile].encoder2_client)
	{
		DPRINTLN(F("Knob #2 Push"));
		obj->writeRGBCode(0x0000FF);
		#ifdef USE_MIDI
			noteOn(CHANcNEL, 65, 127);
			noteOff(CHANNEL, 65, 0);
		#else
			Button_Action(user_settings[user_Profile].encoder2_client_sw);
		#endif
	}
	else if (obj->id == user_settings[user_Profile].encoder3_client && press_timer3.check() == 1)
	{
		DPRINTLN(F("Knob #3 Long Push"));
		obj->writeRGBCode(0x00FF00);
		#ifdef USE_MIDI
			noteOn(CHANNEL, 64, 127);
			noteOff(CHANNEL, 64, 0);
		#else
			Button_Action(user_settings[user_Profile].encoder3_client_swl);
		#endif
	}
	else if (obj->id == user_settings[user_Profile].encoder3_client)
	{
		DPRINTLN(F("Knob #3 Push"));
		obj->writeRGBCode(0x0000FF);
		#ifdef USE_MIDI
			noteOn(CHANcNEL, 65, 127);
			noteOff(CHANNEL, 65, 0);
		#else
			Button_Action(user_settings[user_Profile].encoder3_client_sw);
		#endif
	}
	else if (obj->id == user_settings[user_Profile].encoder4_client && press_timer4.check() == 1)
	{
		DPRINTLN(F("Knob #4 Long Push"));
		obj->writeRGBCode(0x00FF00);
		#ifdef USE_MIDI
			noteOn(CHANNEL, 64, 127);
			noteOff(CHANNEL, 64, 0);
		#else
			Button_Action(user_settings[user_Profile].encoder4_client_swl);
		#endif
	}
	else if (obj->id == user_settings[user_Profile].encoder4_client)
	{
		DPRINTLN(F("Knob #4 Push"));
		obj->writeRGBCode(0x0000FF);
		#ifdef USE_MIDI
			noteOn(CHANcNEL, 65, 127);
			noteOff(CHANNEL, 65, 0);
		#else
			Button_Action(user_settings[user_Profile].encoder4_client_sw);
		#endif
	}
	else if (obj->id == user_settings[user_Profile].encoder5_client && press_timer5.check() == 1)
	{
		DPRINTLN(F("Knob #5 Long Push"));
		obj->writeRGBCode(0x00FF00);
		#ifdef USE_MIDI
			noteOn(CHANNEL, 64, 127);
			noteOff(CHANNEL, 64, 0);
		#else
			Button_Action(user_settings[user_Profile].encoder5_client_swl);
		#endif
	}
	else if (obj->id == user_settings[user_Profile].encoder5_client)
	{
		DPRINTLN(F("Knob #5 Push"));
		obj->writeRGBCode(0x0000FF);
		#ifdef USE_MIDI
			noteOn(CHANcNEL, 65, 127);
			noteOff(CHANNEL, 65, 0);
		#else
			Button_Action(user_settings[user_Profile].encoder5_client_sw);
		#endif
	}
	else if (obj->id == user_settings[user_Profile].encoder6_client && press_timer6.check() == 1)
	{
		DPRINTLN(F("Knob #6 Long Push"));
		obj->writeRGBCode(0x00FF00);
		#ifdef USE_MIDI
			noteOn(CHANNEL, 64, 127);
			noteOff(CHANNEL, 64, 0);
		#else
			Button_Action(user_settings[user_Profile].encoder6_client_swl);
		#endif
	}
	else if (obj->id == user_settings[user_Profile].encoder6_client)
	{
		DPRINTLN(F("Knob #6 Push"));
		obj->writeRGBCode(0x0000FF);
		#ifdef USE_MIDI
			noteOn(CHANcNEL, 65, 127);
			noteOff(CHANNEL, 65, 0);
		#else
			Button_Action(user_settings[user_Profile].encoder6_client_sw);
		#endif
	}
}

//Callback when the encoder is first pushed, will start a timer to see if it was long or short
COLD void encoder_timer_start(i2cEncoderLibV2* obj) {
	//DPRINTLN(F("Push Timer Start: "));
	obj->writeRGBCode(0x0000FF);
	if (obj->id == user_settings[user_Profile].encoder1_client)
	  	press_timer.reset();
  	if (obj->id == user_settings[user_Profile].encoder2_client) 
    	press_timer2.reset();
	if (obj->id == user_settings[user_Profile].encoder3_client) 
    	press_timer3.reset();
	if (obj->id == user_settings[user_Profile].encoder4_client) 
    	press_timer4.reset();
	if (obj->id == user_settings[user_Profile].encoder5_client) 
    	press_timer5.reset();
	if (obj->id == user_settings[user_Profile].encoder6_client) 
    	press_timer6.reset();
}

//Callback when the encoder reaches the max or min
COLD void encoder_thresholds(i2cEncoderLibV2* obj) 
{
	if (obj->readStatus(i2cEncoderLibV2::RMAX))
		DPRINTLN(F("Max!"));
	else
		DPRINTLN(F("Min!"));
	obj->writeRGBCode(0xFF0000);
}

//Callback when the fading process finishes and set the RGB led off
COLD void encoder_fade(i2cEncoderLibV2* obj) 
{
	//uint8_t mfg; 
	MF_ENC.updateStatus();
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
	pinMode(I2C_INT_PIN, INPUT_PULLUP);
    DPRINTLN(F("Setup ENC"));

	#ifdef MF_ENC_ADDR
    // MF KNOB - Multi-Function knob setup.
	if(user_settings[user_Profile].encoder1_client)  // 0 is no encoder assigned so skip this
	{
		DPRINTLN(F("MF Encoder Setup"));
		MF_ENC.reset();
		delay(20);
		MF_ENC.begin(
			i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
			| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_ENABLE // Pullup is on the Teensy IO pin
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
		DPRINTLN(F("Encoder #2 Setup"));
		ENC2.reset();
		delay(20);
		ENC2.begin(
			i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
			| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_ENABLE  // Pullup is on the Teensy IO pin
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
    	ENC2.onButtonPush = encoder_timer_start;
		ENC2.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
		ENC2.autoconfigInterrupt();
		blink_ENC2_RGB();
	}
	#endif

	#ifdef ENC3_ADDR
	// Encoder 3 setup
	if(user_settings[user_Profile].encoder3_client)  // 0 if no encoder assigned so skip this
    {
		DPRINTLN(F("Encoder #3 Setup"));
		ENC3.reset();
		delay(20);
		ENC3.begin(
			i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
			| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_ENABLE  // Pullup is on the Teensy IO pin
			| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_ENABLE  | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
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
		ENC3.onButtonPush = encoder_timer_start;
		ENC3.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
		ENC3.autoconfigInterrupt();
		blink_ENC3_RGB();
	}
	#endif

	#ifdef ENC4_ADDR
	// Encoder 4 setup
	if(user_settings[user_Profile].encoder4_client)  // 0 if no encoder assigned so skip this
    {
		DPRINTLN(F("Encoder #4 Setup"));
		ENC4.reset();
		delay(20);
		ENC4.begin(
			i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
			| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_ENABLE  // Pullup is on the Teensy IO pin
			| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
		ENC4.id = user_settings[user_Profile].encoder4_client;   
		ENC4.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
		ENC4.writeMax((int32_t) 100); /* Set the maximum threshold*/
		ENC4.writeMin((int32_t) -100); /* Set the minimum threshold */
		ENC4.writeStep((int32_t) 1); /* Set the step to 1*/
		/* Configure the events */
		ENC4.onChange = encoder_rotated;
		ENC4.onButtonRelease = encoder_click;
		ENC4.onMinMax = encoder_thresholds;
		ENC4.onFadeProcess = encoder_fade;
		ENC4.onButtonPush = encoder_timer_start;
		ENC4.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
		ENC4.autoconfigInterrupt();
		blink_ENC4_RGB();
	}
	#endif

	#ifdef ENC5_ADDR
	// Encoder 5 setup
	if(user_settings[user_Profile].encoder5_client)  // 0 if no encoder assigned so skip this
    {
		DPRINTLN(F("Encoder #5 Setup"));
		ENC5.reset();
		delay(20);
		ENC5.begin(
			i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
			| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_DISABLE  // Pullup is on the Teensy IO pin
			| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
		ENC5.id = user_settings[user_Profile].encoder5_client;   
		ENC5.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
		ENC5.writeMax((int32_t) 100); /* Set the maximum threshold*/
		ENC5.writeMin((int32_t) -100); /* Set the minimum threshold */
		ENC5.writeStep((int32_t) 1); /* Set the step to 1*/
		/* Configure the events */
		ENC5.onChange = encoder_rotated;
		ENC5.onButtonRelease = encoder_click;
		ENC5.onMinMax = encoder_thresholds;
		ENC5.onFadeProcess = encoder_fade;
		ENC5.onButtonPush = encoder_timer_start;
		ENC5.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
		ENC5.autoconfigInterrupt();
		blink_ENC5_RGB();
	}
	#endif

	#ifdef ENC6_ADDR
	// Encoder 6 setup
	if(user_settings[user_Profile].encoder6_client)  // 0 if no encoder assigned so skip this
    {
		DPRINTLN(F("Encoder #6 Setup"));
		ENC6.reset();
		delay(20);
		ENC6.begin(
			i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::REL_MODE_ENABLE
			| i2cEncoderLibV2::DIRE_RIGHT | i2cEncoderLibV2::IPUP_DISABLE  // Pullup is on the Teensy IO pin
			| i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA | i2cEncoderLibV2::WRAP_DISABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::STD_ENCODER); // try also this!
		//  Encoder.begin(i2cEncoderLibV2::INT_DATA |i2cEncoderLibV2::WRAP_ENABLE | i2cEncoderLibV2::DIRE_LEFT | i2cEncoderLibV2::IPUP_ENABLE | i2cEncoderLibV2::RMOD_X1 | i2cEncoderLibV2::RGB_ENCODER);  // try also this!
		ENC6.id = user_settings[user_Profile].encoder6_client;   
		ENC6.writeCounter((int32_t) 0); /* Reset the counter value to 0, can be a database value also*/
		ENC6.writeMax((int32_t) 100); /* Set the maximum threshold*/
		ENC6.writeMin((int32_t) -100); /* Set the minimum threshold */
		ENC6.writeStep((int32_t) 1); /* Set the step to 1*/
		/* Configure the events */
		ENC6.onChange = encoder_rotated;
		ENC6.onButtonRelease = encoder_click;
		ENC6.onMinMax = encoder_thresholds;
		ENC6.onFadeProcess = encoder_fade;
		ENC6.onButtonPush = encoder_timer_start;
		ENC6.writeAntibouncingPeriod(20); /* Set an anti-bouncing of 200ms */
		ENC6.autoconfigInterrupt();
		blink_ENC6_RGB();
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
	DPRINTLN(F("Blink MF RGB"));
    MF_ENC.writeFadeRGB(2); //Fade enabled with 2ms step
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
	DPRINTLN(F("Blink ENC2 RGB"));
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
	DPRINTLN(F("Blink ENC3 RGB"));
    ENC3.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#ifdef ENC4_ADDR
COLD void blink_ENC4_RGB(void)
{	
    /* blink the RGB LED */
    ENC4.writeRGBCode(0xFF0000);
    delay(250);
    ENC4.writeRGBCode(0x00FF00);
    delay(250);
    ENC4.writeRGBCode(0x0000FF);
    delay(250);
    ENC4.writeRGBCode(0x000000);
	DPRINTLN(F("Blink ENC4 RGB"));
    ENC4.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#ifdef ENC5_ADDR
COLD void blink_ENC5_RGB(void)
{	
    /* blink the RGB LED */
    ENC5.writeRGBCode(0xFF0000);
    delay(250);
    ENC5.writeRGBCode(0x00FF00);
    delay(250);
    ENC5.writeRGBCode(0x0000FF);
    delay(250);
    ENC5.writeRGBCode(0x000000);
	DPRINTLN(F("Blink ENC5 RGB"));
    ENC5.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#ifdef ENC6_ADDR
COLD void blink_ENC6_RGB(void)
{	
    /* blink the RGB LED */
    ENC6.writeRGBCode(0xFF0000);
    delay(250);
    ENC6.writeRGBCode(0x00FF00);
    delay(250);
    ENC6.writeRGBCode(0x0000FF);
    delay(250);
    ENC6.writeRGBCode(0x000000);
	DPRINTLN(F("Blink ENC6 RGB"));
    ENC6.writeFadeRGB(3); //Fade enabled with 3ms step
}
#endif

#endif // I2C_ENCODER
