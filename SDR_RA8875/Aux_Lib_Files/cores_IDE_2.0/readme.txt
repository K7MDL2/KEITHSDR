Dec 21, 2022 Update
I was able to get Windows to recognize the Teensy USB Audio Sample Rate at 48KHz and get clean audio on both TX and RX by doing the steps below.
The default Teensy usb audio sample rate is 44.1KHz. We want 48KHz.  

Thanks threads in the PJRC forum, and the work of Steve KF7O, DL1YCF, and others creating a Teensy keyer with low latency audio at 48KHz, I am able to get things working. It also works with the OpenAudio_Library F32 functions.  

The CW keyer code used AUDIO_BLOCK_SAMPLES  32 and has lots of code for on the fly feedback correction. 
I am not using either so left that code out to keep things simpler.

Be aware that TeensyDuino supplied files are overwritten with each package update or manual install of TeensyDuino.  
You must make these changes each time after such and update.  The files typically do nto change much but you need to check.  
I am using TeensyDuino 1.57.2  and Arduino IDE 2.0.3 as I write this.

I put the following 6 modified files (5 usb related, 1 AudioStream.h) into the repository under the libraries folder. 

	AudioStream.h
	usb_desc.h
	usb_desc.c 
	usb.c 
	usb_audio.h
	usb_audio.c

In my reading files placed in a library folder under the sketch folder should be processed before all other library locations including the stock TeensyDuino library but I have not verified this yet.

1. You can find the TeensyDuinbo files
	a. For IDE 2.0.x go to C:\Users\Mike\AppData\Local\Arduino15\packages\teensy\hardware\avr\1.57.2\cores\teensy4
	b. For IDE 1.8.x go to C:\Program Files (x86)\Arduino\hardware\teensy\avr\cores\teensy4

2. Edit AudioStream.h - Update, USB_AUDIO_48KHZ only needs to be set in usb_desc.h
	a. NA- About line 40, after the __ASSEMBLER__ #endif section, add the line 
		#define USB_AUDIO_48KHZ 1
    b. Find the lines
			#ifndef AUDIO_SAMPLE_RATE_EXACT
			#define AUDIO_SAMPLE_RATE_EXACT 48000.0f
			#endif
		and replace with
			#ifdef USB_AUDIO_48KHZ
				#	define AUDIO_SAMPLE_RATE_EXACT 48000.0f
				#else
					#define AUDIO_SAMPLE_RATE_EXACT 44100.0f
				#endif
			#endif
    c. AudioStream_F32.h already sets this to 48000 but if you do not enable the #define USB32 then the 16bit version will be active, 
	   needing this #define anyway.

3.  Edit usb_desc.h
	a. Insert the below line at line 113 between the comment section and teh line #if defined(USB_SERIAL)
		#define USB_AUDIO_48KHZ 1
	b. Editing this file first will cause to the new #ifdef USB_AUDIO_48KHZ sections to be added next to light up if using an editor like Visual Studio Code making edits color coded, shaded, and error checking far easier to deal with.

4.  Edit usb_audio.h
	a. Find the lines below around 390
		unsigned int usb_audio_transmit_callback(void)
		{
			static uint32_t count=5;
			uint32_t avail, num, target, offset, len=0;
			audio_block_t *left, *right;

			if (++count < 10) {   // TODO: dynamic adjust to match USB rate
				target = 44;
			} else {
				count = 0;
				target = 45;
			}
		    ...
	b. Replace with these lines
			unsigned int usb_audio_transmit_callback(void)
			{
				
				uint32_t avail, num, target, offset, len=0;
				audio_block_t *left, *right;

			#ifdef USB_AUDIO_48KHZ
				target = 48;
			#else
				static uint32_t count=5;
				if (++count < 10) {   // TODO: dynamic adjust to match USB rate
					target = 44;
				} else {
					count = 0;
					target = 45;
				}
			#endif
			...

3. Edit usb_desc.c file.  #define PRODUCT_ID		0x048A
    a. Search for "MICROPHONE".  You will find it in 2 places inside #ifdef AUDIO_INTERFACE sections
    b. The 3 lines for wTerminalType will look like below with the Digital Audio likely set. 
       	//0x01, 0x02,				// wTerminalType, 0x0201 = MICROPHONE
	   	//0x03, 0x06,				// wTerminalType, 0x0603 = Line Connector
	   	0x02, 0x06,				// wTerminalType, 0x0602 = Digital Audio 
    c. Change the wTerminalType to something else like Line Connector to get Windows to recognize the change in the cached driver
       	//0x01, 0x02,				// wTerminalType, 0x0201 = MICROPHONE
	   	0x03, 0x06,				// wTerminalType, 0x0603 = Line Connector
	   	//0x02, 0x06,				// wTerminalType, 0x0602 = Digital Audio
	d. Search for "Headphones".  You will find it in 2 places inside #ifdef AUDIO_INTERFACE sections
       	//0x02, 0x03,				// wTerminalType, 0x0302 = Headphones
	   	0x02, 0x06,				// wTerminalType, 0x0602 = Digital Audio
    e. Change the wTerminalType from Digital Audio to Headphones to get Windows to recognize the change in the cached driver
       	0x02, 0x03,				// wTerminalType, 0x0302 = Headphones
	   	//0x02, 0x06,				// wTerminalType, 0x0602 = Digital Audio
	f. change #define PRODUCT_ID		0x048A  to 0x48B.  

4. Edit usb_desc.c file. 
	a. Search for LSB(44100).  You will find it in 4 places
		Replace
			LSB(44100), MSB(44100), 0,      // tSamFreq
		with 
			#ifdef USB_AUDIO_48KHZ
				LSB(48000), MSB(48000), 0,
			#else
				LSB(44100), MSB(44100), 0,      // tSamFreq
			#endif

5. Edit usb_desc.h.  This updates the sample rate length for USB_MIDI_AUDIO_SERIAL and 3 other sectoins containing Audio.
	a. In the #elif defined(USB_XXXXXXX) sections
		replace 
  			#define AUDIO_TX_SIZE         180
  			#define AUDIO_RX_SIZE         180
		with
			#ifdef USB_AUDIO_48KHZ
				#define AUDIO_TX_SIZE         196   // longer buffer
				#define AUDIO_RX_SIZE         196
			#else
			  	#define AUDIO_TX_SIZE         180
  				#define AUDIO_RX_SIZE         180
			#endif
	b. Be careful to not delete or edit the   #define AUDIO_TX_ENDPOINT x and  #define AUDIO_RX_ENDPOINT y lines, they are mixed in.  
	   The endpoint numbers are unique to each section so leave them as they are.
	c. I chose to customize the Product Name to use my call sign.  Each section contain a product name.  
		Since we are only using USB_MIDI_AUDIO_SERIAL for this SDR project I chose to only edit this section replacing the MIDI/Audio name with my own string.
			#define PRODUCT_NAME		{'K','7','M','D','L',' ','S','D','R'}
			#define PRODUCT_NAME_LEN	9

6. Edit usb.c 
	a. Search for 0x81A2 around line 662 and replace
			endpoint0_buffer[0] = 44100 & 255;
			endpoint0_buffer[1] = 44100 >> 8;
		  with
			#ifdef USB_AUDIO_48KHZ
				endpoint0_buffer[0] = 48000 & 255;
				endpoint0_buffer[1] = 48000 >> 8;
			#else
				endpoint0_buffer[0] = 44100 & 255;
				endpoint0_buffer[1] = 44100 >> 8;
			#endif

7. Uninstall the previous Record and Playback Teensy Digital Audio device instances that were likely at 44.1KHz using the Sound Control Panel or Device Manager.

8. Unplug the Teensy USB and reboot your Windows computer to finish the removal.

9. Plug the Teensy USB cable back in. You should now have new Teensy MIDI/Audio devices in both Playback and Record views.

10. Both should have an Advanced tab with 16bit 48000 Hz DVD Quality listed. Turn off any offered enchancements.

11. Rename each device from to something you prefer.  I like "SDR Audio RX Input" and SDR Audio TX Output" to help identify these easier. Change the icon if you like.

12. Enable listen on the SDR Line In record device and play it back to your speakers and see if it sounds proper.

13. In usb_desc.c you can change the Headphone and Line devices wTerminal type back to Digital Audio if desired. 
 	Sometimes this is needed to get your low level edits to register correctly due to the device caching.  Or try changing the USB PRoduct ID (see later steps). May also need to reboot.

14. Adding a Custom USB type for the Teensy to enable 2 Serial ports plus Audio to the Arduino 2.0 IDE.  This enables debug on Serial and the optional CAT interface on USBSerial1.
	a. add this section into usb_desc.h after the USB_AUDIO section.  
	The Product ID must be unique from the other entries in this file or the IDE menu will use the name of the matching ID and likely be wrong.
		#elif defined(USB_SERIAL_SERIAL_AUDIO)
			#define VENDOR_ID		0x16C0
			#define PRODUCT_ID		0x0484   // usually make this unique.  See project wiki notes though
			#define MANUFACTURER_NAME	{'T','e','e','n','s','y','d','u','i','n','o'}
			#define MANUFACTURER_NAME_LEN	11
			#define PRODUCT_NAME		{'K','7','M','D','L',' ','S','D','R'}
			#define PRODUCT_NAME_LEN	    9
			#define EP0_SIZE		          64
			#define NUM_ENDPOINTS         7   // 5 for 2 serial, + 2 for audio
			#define NUM_INTERFACE		      7   // 4 for 2 serial, + 3 for audio
			#define CDC_IAD_DESCRIPTOR	  1
			#define CDC_STATUS_INTERFACE	0
			#define CDC_DATA_INTERFACE	  1	// Serial
			#define CDC_ACM_ENDPOINT	    2
			#define CDC_RX_ENDPOINT       3
			#define CDC_TX_ENDPOINT       3
			#define CDC_ACM_SIZE          16
			#define CDC_RX_SIZE_480       512
			#define CDC_TX_SIZE_480       512
			#define CDC_RX_SIZE_12        64
			#define CDC_TX_SIZE_12        64
			#define CDC2_STATUS_INTERFACE 2       // SerialUSB1
			#define CDC2_DATA_INTERFACE   3
			#define CDC2_ACM_ENDPOINT     4
			#define CDC2_RX_ENDPOINT      5
			#define CDC2_TX_ENDPOINT      5
			#define AUDIO_INTERFACE	4	// Audio (uses 3 consecutive interfaces)
			#define AUDIO_TX_ENDPOINT     6
			#define AUDIO_RX_ENDPOINT     6
			#ifdef USB_AUDIO_48KHZ
				#define AUDIO_TX_SIZE         196   // longer buffer
				#define AUDIO_RX_SIZE         196
			#else
				#define AUDIO_TX_SIZE         180
				#define AUDIO_RX_SIZE         180
			#endif
			#define AUDIO_SYNC_ENDPOINT	7
			#define ENDPOINT2_CONFIG	ENDPOINT_RECEIVE_UNUSED + ENDPOINT_TRANSMIT_INTERRUPT
			#define ENDPOINT3_CONFIG	ENDPOINT_RECEIVE_BULK + ENDPOINT_TRANSMIT_BULK
			#define ENDPOINT4_CONFIG	ENDPOINT_RECEIVE_UNUSED + ENDPOINT_TRANSMIT_INTERRUPT
			#define ENDPOINT5_CONFIG	ENDPOINT_RECEIVE_BULK + ENDPOINT_TRANSMIT_BULK
			#define ENDPOINT6_CONFIG	ENDPOINT_RECEIVE_ISOCHRONOUS + ENDPOINT_TRANSMIT_ISOCHRONOUS
			#define ENDPOINT7_CONFIG	ENDPOINT_RECEIVE_UNUSED + ENDPOINT_TRANSMIT_ISOCHRONOUS
	   Note: as a workaround the file usb_desc.h I supply in libraries folder has the same contents for USB_SERIAL_MIDI_AUDIO. More below.

Updated method Jan 2 2023.
	b. To add custom Serial + Serial + Audio USB type into the IDE  Tools:USB Type menu do the following.
		Create a new file called boards.local.txt with the following 3 lines. It will add to the existing Teensy Menu items.   I have already created one in the Cores subfolder.
			
		teensy41.menu.usb.serialserialaudio=Serial + Serial + Audio
		teensy41.menu.usb.serialserialaudio.build.usbtype=USB_SERIAL_SERIAL_AUDIO
		teensy41.menu.usb.serialserialaudio.upload_port.usbtype=USB_SERIAL_SERIAL_AUDIO

		Place your boards.local.txt into %AppData%\Local\Arduino15\packages\teensy\hardware\avr\xxxxx.  
		xxxxx is the TeensyDuino version you have installed and are using.  In my test case it is 0.58.3 (aka 1.58 beta 3)

		The USB_SERIAL_SERIAL_AUDIO becomes a #define during compile. The file usb-desc.h has all teh usb combo descriptions and uses similar #define to activate your chosen combo.
		I have provided a modified usb_desc.h file that uses this define and supports the 48K mod. 
		Other files are provided to also "patch" the library and are related to the 48KHz modification only.
		
	c. Shutdown the Arduino IDE.   
	
	d. There is a known bug in IDE that does not update its cache (a small dB) when board.txt is changed. 
	    See https://github.com/arduino/arduino-ide/issues/1030
	 	For Windows users, this is typically C:\Users\<user name>\AppData\Roaming\arduino-ide
		Further reading I found a less drastic solution proposed just delete 1 sub folder called leveldb
		In %AppData%\Roaming\arduino-ide\Local Storage folder delete the leveldb folder. 
	
	e. Start the Arduino IDE 2.0.  Ignore the errors in the output window.  Restart the IDE and it will clear the error.
	
	f. Under Tools
		Board choose Teensy 4.1
		Port choose the port displayed under the teensy ports section.  
		The correct name may not show up until you do your first compile, maybe not even then but you will see 2 serial ports.
		CPU Speed choose 816MHz (recommended, but anything you like that works)
		USB Type choose Serial + Serial + Audio.  This is our new custom USB device type.
	
	g. If DEBUG is enabled in top of SDR_RA8875.h, debug output will be on the first serial port. 
	
	h. If you have the RS_HFIQ, the second serial port will be CAT control at 38400 which emulates an Elecraft K3.


Configuration Note:  I have IDE 2.03 and TeensyDuino 0.58.3 installed.   Along the way I removed 1.8.19 as I found the VS Code was still using it and I wanted a clean 2.0 system.

_________________________________________________________________________________________

Previous version notes from the IDE 1.8.x days:

This file is a modified version of the original TeensyDuino file called AudioStream.h

For Arduino IDE < 2.0
	C:\Program Files (x86)\Arduino\hardware\teensy\avr\cores\teensy4

or for Arduino > 2.0

	C:\Users\[username]\AppData\Local\Arduino15\packages\teensy\hardware\avr\1.57.2\cores

The changes are in the #defines at the top to set the default to either 44.1KHz or 48KHz.  

We are using 48KHz.  Kept the sample block count the same at 128.  The CWKeyer project used 32 to keep latency low and has lots of feedback loop code for on the fly corrections.



