As of May 31, 2022
____________________________

Place these 3 files in the TeensyDuino Audio Library folder:

C:\Program Files (x86)\Arduino\hardware\teensy\avr\libraries\Audio

control_wm8960.h		// established the class for the WM8960 chip.

control_wm8960.cpp   	// Extended version of softerhardware CWkeyer project.  
				// Added identical function names to match the SGTL5000
				// In some cases just calling existing WM8960 functions
				// In other cases created new function and took a quick swag at an equivalent chip register write.
				// It is not a perfect remapping but close enough to start with, some further fine tuning would be good.
				// Not all SGTL5000 functions are reproduced such as ALC, just the ones I am using in the SDR project today.

** Note ** : This WM8960 file is using the alternate i2c bus on the Teensy 4.0/4.1 on pins 17 and 18 (SDA1/SCL1).  
		 This is simply referenced in the code at Wire1 instead of Wire.
		 This was necessary due to unfavorable interaction noted between the RA8876 touch controller and the WM8960 when on the same Wire bus.  


Audio.h   // Added the new control_wm8960.h file to the inventory

The module used for testing was the Waveshare WM8960 Audio module, not the Raspberry Pi Hat version.  
It is a smaller module with speaker connector and mic/headphone jacks on one end, and the CPU interface connectors on the other end.
Wire/trace length is important, must be kept short!  I used < 1.5" wires right off the Teensy CPU pins as I used extended length (double sided) header pins.
The headphone jack is a 4ckt jack with mic mono input and stereo output.  It can be grounded so can connect to an audio amp safely.
The speakers are driven by an internal audio amp up to 1W depending on the voltage.  This module uses 3.3V so the output is not loud.  5V would be better.
Ideally this module should be powered off a separate 3.3V supply as the audio amp will max out the Teensy 250ma 3.3V supply.  The headphones do not seem to be a problem.
The chip has 4 pairs of inputs but this module uses:
	LInput L3 for the Jack Detect to switch speakers and headphones (JD2)
 	Input R1 for mic jack audio
	Input L1 for the onboard MEMEs mic
By default the MEMS mic and mic jack are both enabled, this will probably be changes to switch over in due time.

This module has no connections for the Line2 L & R inputs.  These are the ones needed for the I and Q from the RX/TX board.
Some very fine soldering is required to bring out wires from the tiny chip pins for these 2 inputs.  I have not done that yet and probably won't.
This audio module was used mostly to explore what it performs like compared to the SGTL5000, how the amp works, and to create the library code.
I expect to create a version the Teensy SDR PCBs we have made with the WM8960 or other Cirrus Logic chips soldered to the board in place of the 
   SGTL5000 plug-in module used today.