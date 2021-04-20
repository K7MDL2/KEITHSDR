# KEITHSDR

Teensy4.X with PJRC audio card Arduino based SDR Radio project

## 4/19/2021

    1. Set larger CAT_Serial RX buffer to avoid buffer overwrites when message stack up, mostly at startup.
    2. Added SD card support functions incouding directory and card info.
    3. Writing table data to SD Card file RadioCfg.db for BandMem, user_setttings, and Sp_Parms_Def.
    4. Writing #define data to RadioCfg.h.  This is only useful when transferred to the compiling PC and used in place of Radioconfig.h.  This is work in progress.  It should be done as a PC utility but it is convenient to do it here on the SD card while I sort things out.  In theory you can take this file, when done, read in the full original RadioConfig.h as normal, then read this .h file which will override #defines by testing for each variable it knows about and then does a #undef to unset appropriate #defines.
    5. RadioCfg.cfg is intended for misc individual variables.  These ideally would go into EEPROM.  Nothing is written to this file yet, it is only created as an empty file at this point.
    6. MF knob encoder switch press cycles through tune rates. Added 1KHz back into cycle of rates so now have 10, 100, 1000Hz active. Fine mode enables 1Hz/10hz toggle as before.
    7. MF knob encoder switch long press swaps VFOs. The longer term goal is to enable VFO B tune, useful for split mode without needing to swapping VFOs. This requires adding a new MF_client function "VFO_B_Tune()".

## 4/17-18/2021+

    1. Panadapter Mode additions, some for Kenwood/Elecraft CAT protocol as appropriate:
        a. Update display to match VFO A mode, RIT, XIT, Split.
        b. RIT and XIT are the same on a K3. RIT/XIT is set to match the radio, both + and -.  
        c. VFO A is updated with RIT offset if enabled.
        d. Most parameters updated only if they change.
        e. New Modes added: AM, FM, CW-REV, DATA-REV.  They are the same as upper or lower CW or SSB and does not add demodulation AM or FM though that will occur later. For now tune carefully in SSB mode.
        f. Meter is S-meter in RX, Power or ALC depending on radio meter mode, in TX.
        g. Swipe up or down changes spectrum scale by 3.  Limited to a range of 10 to 80.
        h. AGC, Filter, Mode and ANT all update to match the radio.
        i. The IF center frequency data is used set the PLL Fc offset value to move the display to account for radio mode/filter frequency shifts. The spectrum signals are always in the correct orientation with regard to the center line.
        j. A serial message broker looks for any incoming message and calls the matching functions to decode it.
        k. Presently setting the K3 in AutoInfo2 mode so it sends out a message for any front panel changes. We still poll for bar graph data and certain other settings like the IF center frequency shift and mode. Mode is present in the IF; message but that message is not normally sent out by the radio until a band change. Same for IF shift info.
        l. There is a XXXX_Request() function for every XXX_Decode() function.  If not using AI1 or AI2 (AutoInfo) modes then you must poll for info periodically. Use these xxx_Request() functions.

## 4/16/2021

    1. Changed Reference Level calculation to auto-set near bottom of window. Updated the default band memory database and spectrum databases to use values close to 0 vs. the previous -170ish values. Each update it looks at the 3rd FFT bin (an arbitrary choice) for a starting level and then scans all the bins for anything lower.
    2. When in PANADAPTER config, a radio's VFO is now polled via CAT and updated on the display so follows as you tune the radio. It is set for about 500ms updates.  This is working for a K3 with 8.215MHz IF.
    3. User Profile 2 is being used for PANADAPTER config. It sets BAND0 as eht curernt band. Can use all fields as normal.
    4. Swipes up/dn to change bands are disabled in PANADAPER config. Encoder 2 is set to RefLvl so a drag up/dn will change ref level value.  RefLvl is now limited to +/- 50 in step=1 unit.  0 to 10 is the normal value.
    5. Spectrum scale changes are made in the Spectrum table. 20-60 are probably good numbers to stick to. The spectrum values are not yet linked to the waterfall settings. I tweaked the waterfall gain and color for the 7" display.

## 4/14/2021

    1. Swipes are now limited to the spectrum hotspot area to prevent unintended band changes while operating buttons or labels.
    2. Long press is functional. Touch the MF meter and it will toggle the AFGain and drag will adjust the setting. Long press the MF meter and it will switch from AFGain to RF gain.  Further, these are both toggles so the first touch for each gives drag the focus, the second will release drag back to default, not waiting for the normal timeout. The meter will time out back to S-meter mode as normal.
    3. Long press will enable On/Off buttons that have adjustments to use drag and not change their On/Off state. ATT and NB are 2 good examples. You want to have these on or off but need to adjust the setting.  Long press these 2 and the MF meter changes and make your drag adjustment.

## 4/13/2021

    1. Added drag feature.  The Horizontal drag now follows the MF Knob (Encoder1) assignment in the database.  Vertical follows 2nd encoder assignment.
    2. Assigned vertical drag to change the spectrum reference level
    3. Assigned horizontal drag to MFTUNE which will slide the VFO up and down using the current tune step rate. 
    4. Previous gestures still work. Swipe up/down to change bands, touch a spot to change to that frequency, 3 finger swipe up and down to change audio level (this takes practice), swipe left/right to change VFO by 1 tune step. The later may be obsolete now since dragging to tune works well.  The swipe left/right may be changed to spectrum span later.
    5. Tapping the MF meter changes it to AFGain mode.  You can then drag left/right to change volume quickly, no need to find the panel with the button or look for a knob.
    6. By adding these featues the goal is to prove that we can have a good and efficient touch control experience requiring no knobs or switches.  That means less expense, drilling, panel space, and noise from long wiring.

## 4/10/2021

    1. Reworked the way the buttons activate and display on the MF meter.  They are now more consistent.  All will timeout. A new button pressed will take over from the previous one immediately, shutting off the button color except for ATT and NB which will remain in their last On/Off state. All when active will have the MF knob and the MF meter focus.
    2. Optimized the OCXO/TCXO #defines
    3. Tested combinations of OCXO (sine and square wave), TCXO with A and C version Si5351 boards and also a ADF4351A PLL. Various degrees of spurs occur on multiples of 5MHz. The crystals, TCXOs an OCXOs are either 10Mhz or 25Mhz.  Still investigating.

## 4/9/2021

    1. Added new band 0 as a dummy or IF.  Shifts Band index for other bands up by 1. This also aligns with some band decoder library band definitions useful for PANADAPTER mode.
    2. Decreased the height of the spectrum windows by 10px to make a bit more room for touchable labels just above the spectrum box.
    3. Tagged most functions as COLD which is a macro for FLASHMEM. This locates the little used functions into FLASH memory saving RAM for variables and fast run code like the spectrum update function.  Increased RAM free space from 90K to 125K. Individual strings are wrapped with the F() macro which places those constants into PROGMEM memory area saving more RAM.
    4. Incorporated remoteQTH.com band decoder source, heavily modified to include the minimum needed for serial CAT port. Testing with Elecraft/Kenwood config using Serial port 6.
    5. Any encoder knob controlling a MF knob assignable function, including the MF knob itself, will now take over the S-meter and display its setting in the segments (range 0 to 10) in a different color along with a number.  After a timeout the meter is released to normal S-meter duty.

## 4/8/2021

    1. Fixed RA8875 mode S-meter location.
    2. New Screen rotation define now defaults back to 0.
    3. Working on Panadapter mode features so configuration might not always be set for Radio mode.

## 4/7/2021

    1. The S-meter is now a button type object so its size and location are controlled by the button table, including the outline as well. The ringMeter uses the button's x,y, and h values to write direct to the interior of the button including attempting to resize the meter best it can.
    2. The function displayMeter(val, string) is used to update the Meter object. Since the button properties include color for background and text, any other function could use the meter button to display its own suff. For example, when the MF knob is active on RF gain, the meter can change color and style perhaps, and show the RF gain visually during adjustment. When the MF knob timer expires, the meter reverts back to S-meter default state.  As of today, only the S-meter usage is active.
    3. Incorporated FT-817 library from same author as the Si5351mcu lib. This will enable reading and controlling a FT-8xx series radio from our SDR.  I am targeting the FT817 primarily first to use our SDR as a high quality panadapter and then as a control head.  Today there is a series of prints on the Serial terminal at startup requesting info from the radio if present.

## 4/6/2021

    1. Ported the S-Unit "ringMeter" over to the RA8876 and moved it into a box in the upper right corner of the 7" display. This will make room for moving a few indicators around into the empty space on this larger display. Moved the clock into the far upper right corner.
    2. For the RA8875, the ringmeter code uses the RA8875 library.  For the RA8876 it is run as local functions.  There is very little dependency on specific display capabilities in this port so you can use this on most any display now.  All the code in is Display.cpp and is called in SMeter.cpp.
    3. Centered VFO panel.

## 4/5/2021

    1. PANADAPTER Configuration settings added. There are a few PANADAPTER_xxx #defines added to RadioConfig.h along with related code changes. This configures the SDR to replace the VFO with a with a fixed LO. This means the VFO is not used and the VFO encoder knob will be reassigned for other purposes TBD.  
    2. The displayed FFT data tuning "direction" can be inverted as many radio's IFs are inverted, meaning, if you tune up, the signal goes down at the IF output.  This can be radio and band dependent. The FFT setAxis(x) function is used to flip FFT data order to correct for this.
    3. I am testing this on the 8.215MHz IF output of my K3. I plan to do the same with a modified FT-817 next month when that rig will be in my reach again. A serial CAT port and likely an accessory port connection will round out the feature set to make our SDR a control head with panadapter for the FT-817 and other radios if so desired.
    4. This panadapter implementation far from complete, especially without info from a radio to be informed of band/inversion/mode/filter Fc changes. The actual frequency of the radio or transverter will be displayed in the VFO display.
    5. If you change the SDR into DATA mode the PANADAPTER_MODE_OFFSET value is added to the LO frequency to recenter the SDR display. The K3 in DATA mode will have the IF offset by the filter Fc. Use this value as the OFFSET value.  This can be automated with a serial CAT link. For now you have to manually match modes with the radio.

## 4/4-5/2021

    1. Can now running 2 instances of the spectrum window. On the RA8875 I get loop time of about 110ms for 1 larger and 1 smaller window and 90ms on the RA8876 with 2 510px wide windows. That is faster than the single full size window in prior builds for 2 reasons listed below. It is now essentially video quality since there is no flicker and I can increase the refresh rate to aboput 60-70ms.
        1. Less total pixels drawn
            a. Erasing old lines doubles our time
            b. 2 smaller windows have fewer total lines to draw.
        2. I changed the method to plot the spectrum areas to be similar to the waterfall. 
            a. For each update I fill a black rectangle on a hidden 2nd page or layer.
            b. I then write the spectrum plot lines or bars on the 2nd page
            c. I write the onscreen information (Peak frequency, scales, etc) to Page 2
            d. Use BTE mem copy to move the block to Page 1 in the spectrum window
    These changes cut the total lines/bars drawn by half and we get instant refresh so no flicker. Since the info text inside the window is written after the line draws, they are never overwritten and remain clear and flicker free also.
    2. The RA8876 has issues with smaller size windows breaking the waterfall memory copy and writing outside the window boundaries.  Also one side waterfall will not update. So plenty of work to do on the RA8876. The RA8875 works fine.
    3. Todays version of Spectrum_RA8875.ino has 2 lines added to draw the 2nd window frame then update it at the same time as the main window.  I will restore that to single window in the next build update but is now in the version history and you can look at past versions to see what was done.

## 4/3/2021

    1. Fixed Swipes when in RA8876 Configuration. Oddity in the FT5206 library which is used with the RA8876.
    2. Added a feedback beep when a button is touched. Uses the pitch and rogerBeep_Vol level value in the user_settings table.  To turn off the beep set the volume to 0.0 in the table.
    3. Added an attempt to auto-tune a CW signal when you TouchTune in CW mode. It adds the pitch frequency to the peak signal frequency, if there is one. If not then it just jumps to the screen's touch point frequency.
    4. Waterfall timestamp line changed to a short tick line every 15 sceonds on left side.
    5. Fixed display stoppage when the Menu or Display buttons were pressed, remnants of the old pop up window on that button and the band button.  The pop ups will return in a sequel. The Display button now switched between LINE and BAR mode cleanly.
    6. Converted the spectrum display DOT mode to LINE mode for a more traditional spectrum image.  Loop time is around 120ms.  A lot of energy is required to clear the old vertical space before the new line is drawn. Clearing the whole rectangle then draw the 1022 line segments is fast but give a bit of flicker. The current method is smooth but overwrites the on-screen indicators so those will need to be relocated outside of the window.

## 4/2/2021

    1. Enabled support for the RA8876 controller based 7" display with FT5206 compatible touch controller. Use #ifdef USE_RA8875 to enable the RA8875, else it will use the RA8876.  
    2. RA8875 is enabled by default in RadioConfig.h
    3. The touch screen is mounted upside down on the RA8876 7" screen relative to the RA8875 4.3" display. Part of the RA8876 changes involve correcting for that. Other size displays may be different.
    4. I2C Encoders now work. Need to run I2C bus at 100K vs 400K.
    5. In RA8876 mode Swipes in any direction do NOT work yet.
    6. Added Noise Blanker (NB) to input I and Q. There are 6 predefined sets to try out.  Change those values in the NB table in SDR_Data.h.
    7. Added copies of setActiveWindow() functions from RA8875 library to Spectrum_RA8875.cpp for use with RA8876. The RA8876_t3 library has the function marked "protected:"  Rather than require everyone modify the library, the function are now local in RA8876 mode.  It won't work if the display is rotated to portrait mode.
    8. I2C knobs can be added and enabled by #defines in the RadioConfig file to specify the addresses. There is code to support 3 I2C knobs today. The VFO is a mechanical encoder. If more than 3 I2C knopbs are needed, just clone a couple code blocks in SDR_I2C_Encoder.cpp to set them up and add 1 variable for each to the end of the user_settings table structure that is the actual assignment.
    9. The user_settings table currently has 4 variables for encoder knobs, I2C or not. They are Default, Encoder 1, 2 and 3 assigned functions. Encoder 1 is always treated as a MF (MultiFunction) knob. If you assign Encoder 1 and default to be the same then it will act as a dedicated knob until you hit a MF-aware button, and then that new button only has control for a short time.
    10. Any function the MF knob can control today is a candidate for assignment to a dedicated knob. All the encoders share the same code path as the MF knob. The MF knob is a bit special in that it has a timer and buttons can temporarily reassign it's function ID.
    11. Multiple mechanical encoders can be added in a similar way except they do not require the setup code that I2C knobs require. If you have no I2C knobs then by default a mechanical MF knob is defined and if you have one connected, it will just work (using the right configured pins of course). Adding more requires cloning the bit of mechanical MF encoder code, make the database assignments, and have the new code call MF_Service(counts, knob) function with "knob" assigned to the function ID which is simply the button #define number such as ATTEN_BTN or RFGAIN_BTN, the same ID used for all control and display functions as it is the index into the buttons table for all things buttons.
    12. All the I2C buttons light green when in motion. They turn red at the range limits (if any) and when motion stops, the light fades.  They turn blue when pushed. They are capable of double push and press and hold usage which we will take advantage of later.

## 3/31/2021

    1. NB now controlled by the MF knob smae as teh other controls like Atten and RF gain.
    2. Started adding Long Press logic to help with control for those MF enabled buttons.

## 3/30/2021

    1. Fixed ethernet feature, all features working now after the .h/.cpp split.
    2. Added Noise Blanker block into each I and Q chain to start experimenting with it. The NB parameter set is chosen by user_Settings[user_Profile].NB_en database value which has threshold, nAnticipation, and decay values.  The NB button and status label now show which number or nothing for off (NB- or NB-X)
    3. Staged AGC (aka AudiCompressor) into the chain to start finding best adjustments.  Not active just yet.

## 3/29/2021

    1. Added FlexInfo() memory region usage report to the terminal menu.
    2. Added I2C Device Scanner Report to the terminal at startup.
    3. Split the .h files into .h and .cpp file pairs.  As a result there are around 13 or 14 new .cpp files.
    4. Deleted AGC and Step .h files, they were very small and are now part of Controls.cpp.
    5. Reduced memory usage from 79% to 64%. likely because some data was being duplicated.
    6. Started using FLASHMEM decoration to assign certain code/data to FLASH region. Startup() and several utility functions for example.
    7. Do not use the ENET feature yet.  There are some code changes required to make that feature compile.
    8. When all features excluding ENET are on, ITCM memory is at 89%. Report below.
     Memory Usage (FlexInfo)
     FLASH: 217976  2.68% of 7936kB (7908488 Bytes free) FLASHMEM, PROGMEM
          ITCM:  115944 88.46% of  128kB (  15128 Bytes free) (RAM1) FASTRUN
     PSRAM: none
     OCRAM:
        524288 Bytes (512 kB)
         12960 Bytes (12 kB) DMAMEM
         42792 Bytes (41 kB) Heap
        468536 Bytes heap free (457 kB), 55752 Bytes OCRAM in use (54 kB).
     DTCM:
        393216 Bytes (384 kB)
        205504 Bytes (200 kB) global variable
          4392 Bytes (4 kB) max. stack so far
     =========
        183320 Bytes free (179 kB), 209896 Bytes in use (204 kB).
     *** End of Report ***

## 3/28/2021

    1. Moved some more config related items to RadioConfig.h
    2. Merged Pull Request adding and I2C connected 2x16 char LCD as an Auxillary display showing AF and RF gains, VFOs.
    3. Added support for I2C connected encoders. Using a #define I2C_ENCODER you can choose between GPIO encoder or I2C encoders. The VFO encoder is not touched, it is GPIO still.  Specifically these are the DuPPa.com V2.1 I2C boards that mount either standard small encoders or RGB LED versions with clear shafts and translucent knob caps or rings
    4. The first I2C encoder usage is the MF knob. Using a RGB LED version encoder it behaves exactly as the GPIO connected encoder except it changes colors to RED at min or max values, and GREEN for values in between. The color fades when motion stops. It lights up when you turn or push thr knob.  
    5. These I2C encoders support push, release and double push events so will be using them for navigation and selections. Also press and hold type settings. Next up are RF and AF gain pots, and Spectrum Reference Level. I am thinking a push of the MF knob rotates through the possible assigned functions (AF, RF, Tune, Ref Level, Atten today) without requiring a menu button be found and pressed.

## 3/25/2021

    1. Refactored the UserInput.h file to reduce 1,000 lines of repetitive code into 150. This makes it easier to read and understand and makes adding or removing a new button or label into a panel very fast. There is a new "panel" field in the button table to specify which buttons show up in which panels, if any.  0 means no panel is associated.
    2. This build has 2 sinewave generators that can be turned on with a #define. The output is visible only to the FFT a as frequency marker on the spectrum. The frequency where it appears is not right, investigating.
    3. Tweaked RFGain values and calculation method for smooth 100% coverage.  Was only effective in lower 45% before. LineIn table setting is now a max input level and RFGain is scaled against that value for 0-100%.
    4. Some AFGain tweaks along with more comments and small fixes for improved initialization accuracy relative to the table settings to be applied.
    5. Reorganized the collections of user and non-user configuration and build settings into the top of RadioConfig.h. The items most likely to be changed are at the very top. Effort was made to make clear of any dependencies for #define type build options.

## 3/24/2021

    1. New Feature: There is now a MF (Multi-Function) knob if you have an encoder connected.  Not using the push switch function yet.  Look in "SDR_8875.h" at the bottom and change "Encoder Multi(40,39);" to match your meahanical encoder pin connections. 
    I now have I2C RGB LED encoders here so I will soon be adding support for those.  Right now the AF and RF Gain are assumed to be on the MF knob, but I will devise a config switch to have them assigned to dedicated knobs.
    The MF knob will now control RF Gain (audio card line in level), AF Gain (volume, using the RampVolume function), spectrum Reference Level, and Attenuator (range 0-31dB) if you have a digital step attenuator such as the PE4302 module, connected via a SV1AFN bandpass filter board. It also tunes the VFO at a fixed rate when it is not actively changing settings.
    To use this feature you simply push the key of interest, for example RF Gain. The key cap will turn blue indicating it is active and the MF knob has "focus" on that settng. The current value is displayed in the key cap label.  A 3 second timer starts. Turn the MF knob to raise and lower the setting. The timer is extended each time you move the knob.  When the knob is no longer moving after 3 seconds, the key cap turns black and the MF knob returns to an assignable "Default" function. I have it set as a sub-VFO with coarse tune rate. If you press another MF enabled key, it will become active and turn off the previous key, shifting focus to the new setting until the timer expires.  Attenuator is an exception, it stays blue when the relays are engaged but will still lose focus like the other keys. need to create a press and hold gesture to improve that experience. Need a drag too....
    To enable the attenuator feature you must uncomment the //#define DIG_STEP_ATT in the main header file.  Also note that even if you do not have an attenuator board, the Atten key still shows a value and the key will not turn blue. The value is the current value stored in the database and is the initial value if you were to turn it on.

## 3/23/2021

    1. Added code for the PE4302 Digital Step Atenuator. For now you can tap the ANT key to cycle through 31dB in 1dB steps. Watch the Serial Monitor for the actual value printed out.  ATT key turns it On and Off. You can have a fixed attenuator instead. The code writes blind so it won't matter if you have one of these connected or not. I used pins 30-32 on the Teensy4.1 but I think a more efficient plan is to use the 3 left over pins on the MCP23017 I2C port expander that you likely used if you have the SV1AFN bandpass filter board.  I had the code written to use the expander pins. I forgot I was going to use them and wired it to the Teensy direct. 
    2. Enabled a small bit of FFT averaging.  Helps smooth out the spectrum displays. Too much and things slow to a crawl.
    3. Moved more code around to land more of it in the high level control functions and less in the low level device functions. The idea is the control functions take a simple argument like up or down or update to current state, or toggle.  They do this and take care of all the necessary dependencies to make everything play together. The low level functions do the bare minimum to change usually only 1 specific thing like operate a relay.  It has no idea if it is a good idea or not or how to update a display.
    4. Lots of tweaks to mostly the ATT and Preamp control functions to make them work in all test cases including startup and band changes with the last remembered band and modes.
    5. Fixed an issue where time was 10 seconds off.  Need to integrate this into the onboard RTC.
    6. Added a new field to the BandMemory structure, "attenuator_dB" which holds the variable attenuator value for each band independent of the attenautor relay on/off state.  You can use a BPF or PE4302, both, or neither. Set the define in the main header file if you want to compile without the code for those.
    7. New Panel of buttons, Row #5. Has Enet, RFGain, AFGain, and for convenience a repeat of Ref Level and Display buttons.  
        a. Display toggles the spectrum DOT and BAR modes.  
        b. Enet is ethernet data output on or off to a remote host. 
        c. For testing, tapping the RFGain decrements the LineIN codec setting
        d. Tapping the AFGain button increments the RampVolume() function scale factor bwtween 0 and 1.0. It does not alter the actual speaker volume initial setting, it returns volume level to the codec volume setting value.  This way the codec.Volume is a MAX volume and RampVolume scales it between 0.0 and 1.0 which we represent, and store, as 0 to 100.  
        e. 3 finger swipe UP increments RF Gain
        f. 3 finger swipe down decrements AF gain
    Ultimately we need to create up and down buttons. The control functions takes -100 to 0 to +100 as an increment value. It scales the underlying actual values needed for the codec lineIn() and RampVolume().  Settings are saved per user profile.  An encoder just has to call the RFGain(x) or AFGain(x) function with its positive or negative counts, scaled to what ever sensitivity we want. Maybe do a label near the sMeter, popup window with a slider.  Many UI solutions possible. 
    8. Note: I am changing the LineIn() level for RFGain. It seems to work quite well. Don't know if that is before or after the ADC. If it is after, then overload is still an issue and a digital potentiometer would be a solution.
    9. Added ramped volume dips during relay operations to prevent loud speaker thumping. This means PRE, ATT and band changes, including preselector bypass when moving in and out of the defined ham bands.  Related to this I have moved speaker related codec volume initialization code to near the end of setup() after all the initial relay activity is done. The speaker is then unmuted and set to the last known speaker volume (per user Profile), as is RFGain (per user Profile).
    10. Automatically adjusts the spectrum reference level when the attenuator and/or premap is on or off.
    11. New Feature Added: TouchTune with AutoTune.  You can now touch a signal on the spectrum and if it is within +/- 800Hz of the Peak Frequency detector it will jump to that peak frequency. I can tune WWV off to the edge of the screen, touch it and will be exactly on 5.000.000 or 10.000.000. Look at the F: XXX indicator on the screen for the frequency it is detecting as strongest. If it is near what you want, it should work. An obvious improvement to this would be to search for the peak within X-distance of your touch point and tune most any signal, not just the strongest in view.

## 3/21-22/2021

    1.Moved step increment/decrement control into Rate() function.  Left old step.h thinned down to set the tune rate directly in the database and call display. 
    2. Moved filter increment/decrement control into Filter() function.  It calls select bandwidth() in bandwidth2.h as before which now has the increment and database logic I inserted some time back removed. It now moves the value upwards then downwards and back upwards like the rate key does.
    3. With 1 minor change, new bandwidth.h files will plug right in. Since the current bandwidth value of bndx is stored in the per-band database a global bndx var is not used but is instead passed as an arg. You add (int bndx) to the function definition like this:
        void selectBandwidth(int bndx)
    You can call it direct or through the Filter(dir) function, same as Rate(dir). dir set to -1  or +1 increments the current filter down or up. dir = 0 cycles the current filter (or tune step for Rate(dir) in the last direction until it hits top or bottom then reverses direction.  The +1 and -1 are needed particularly for swipe commands which are left and right for changing the tune step rate.
    4. Added SV1AFN Bandpass Filter board (aka Preselector board) library to the repository, updated to support the 60M band. It allows I2C control via a MCP23017 I2C Port Expander module.  Preamp, Attenuator and of course 10 bands worth of band pass filters.  The Control.h, Preamp() and Atten() talk to the BPF board.  Working good after a rough start with some I2C bus issues.  Ran I2C scanner and power cycled and things cleared up. Something to monitor.
    5. SelectFrequency() now monitors for band edges and will BYPASS the BPFs when tuned outside the ham bands.
    6. Comment out "#define SV1AFN_BPF" in the SDR_8875.h file to exclude all code related to the Bandpass filter board in case of any side effects if you do not have a board connected.
    7. Staged setAtten() function in Controls.h for the PE4302 digital step attenuator. Need to map it to IO pins TBD.
    8. For Testing I assigned the NOTCH key to stop updating the spectrum to compare autdio impacts from the SPI bus activity.  Using a si5351C PLL board's crystal (with a 10MHz OCXO connected but not selected) I noted the SPI clock LED modulating with the audio, worst right on 5 and 10MHz listening to WWV. Move a few Hz off frequency and problem gone.  It was internmittant though.  Maybe some bleed through interacting. Switched to the OCXO later and no problem yet.

## 3/20-21/2021

    1.Improved Spectrum performance a bit in a few ways. 
        a. Now assign out-of-view bound pixel plots to a min or max value to ensure they get plotted. This helps prevent stale data. This sped up the draw speed a bit also for some reason.  Particularly helps when the ref level is set very low.
        b. Added a timer to clear the spectrum plot area if there is no FFT data available for any reason over X time.
        c. Tweaked plot averaging rules to prevent out of array bounds reads of FFT data.
    2. Added a "Starting Network" message in noticeable Blue text in the space the clock will occupy once it gets started up. The network startup takes several seconds to initialize so the spectrum is blank during this wait.
    3.  Moved the waterfall update rate timer into Spectrum_RA8875,h file and the setting into the spectrum layout record so each layout can have its optimum speed. Smaller size plots take less draw time permitting faster update rates.
    4. Removed several spectrum layout records. Will resize the remaining ones. These may be use for spot zooms in a small window.
    5. More button tweaks. Moved some labels around, changed to dark grey when inactive.
    6. S-Meter will now move up a bit to fill in the void space if there is no clock displayed
    7. Remodeled the UserInput.h to separate touch event and functions to permit hardware or other events to trigger activity on a feature or device. 
    8. Created new Controls.h to move out control code from the touch and gesture code. Touch, Gesture, other functions and mechanical devices likes encoders and switches can now call the functons in Control.h to change a setting and the buttons (if showing) and labels are updated automatically.
    9. New label table structure to remove hard coded params from display buttons/label functions. Also enables Labels to be scanned for touch events
    10. Fixed 1/10Hz issue Rate key.
    11. The lower line of status labels are self-updating type, always on and are touch enabled in case their button (key) is not showing in the current panel.  They have enough space aroud them to be "touchable" though they are stil small. I have them set to a variety of colors to see what looks good. Soem change color.  Try all the buttons and see what happens.  Preamp is a background example.   Cyan (used on Filter) is like a strong flashlight in your eyes.

## 3/20/2021

    1. Major UI layout work. There are now on-screen labels for most everything a normal radio would have indicators for.
    2. Rearranged many buttons to match the order of the labels where possible.
    3. Formatted VFO and spectrum frequency labels with MHz.Khz.Hz format. Sized things to display up to 99GHz (11 digits and 2 dots total).
    4. The VFO labels will highlight Red background during Transmit.  You can test this by pressing the XMIT key. 
    5. When Split is on, the split indicator turns green with an arrow pointing to the standby VFO. The Active VFO is on top row, the standby VFO is on bottom in dark grey. VFO A and B can swap between Active and Standby. When yo upress the XMIT key, the standby VFO will now light red.
    6. NOTE: Do not uncomment the "#define OCXO_10MHZ" if you do not have a C version PLL board. 
    7. If you want to ruin etehrnet there are multiple ways to configure it.   Probably the easiest is to uncomment the #define USE_ETHERNET_PROFILE I placed in the SDR_8875.h file today around line 77ish.  It looks like this snippet.
            // --------------------------------------------User Profile Selection --------------------------------------------------------
            //
            //#define USE_ENET_PROFILE    // <<--- Uncomment this line if you want to use ethernet without editing any variables. 
            //
            #ifdef USE_ENET_PROFILE
                uint8_t     user_Profile = 0;   // Profile 0 has enet enabled, 1 and 2 do not.
            #else
                uint8_t     user_Profile = 1;   // Profile 0 has enet enabled, 1 and 2 do not.
            #endif
            //
            //----------------------------------------------------------------------------------
    Other ways are to edit the user settings table or to comment out in the above text what you need to get the User profile 0 to be active. Many ways to skin a cat.
    8. Improved Spectrum performance a bit in a few ways. 
        a. Now assign out-of-view bound pixel plots to a min or max value to ensure they get plotted. This helps prevent stale data. This sped up the draw speed a bit also for some reason.  Particularly helps when the ref level is set very low.
        b. Added a timer to clear the spectrum plot area if there is no FFT data available for any reason over X time.
    9. Added a "Starting Network" message in noticeable Blue text in the space the clock will occupy once it gets started up. The network startup takes several seconds to initialize so the spectrum is blank during this wait.
    10.  Moved the waterfall update rate timer into Spectrum_RA8875,h file and the setting into the spectrum layout record so each layout can have its optimum speed. Smaller size plots take less draw time permitting faster update rates.
    11. Removed severa spectrum layout records.  Will resize the remaining ones. These may be use for spot zooms in a small window.
    12. More button tweaks.

## 3/19/2021

    1. Updated VFO.h and other files to use "#define OCXO_10MHZ" to switch between si5351 libraries and configure the PLL board for a Version C si5351C PLL board with external 10Mhz reference clock.   As a matter of convenience, I am also switching to "user_Profile=0" which has ethernet enabled. Otherwise "user_Profile=1" is used and ethernet is disabled in that profile setting.
    2. If ethernet is not enabled the on-screen time clock is not displayed since there is (currently) no means to set or save it. A battery for the onboard RTC should fix that problem. On the TODO list.
    3. If the Rx signal is below the bottom window level, the data is not plotted saving CPU time. That is a problem because the opportunity to erase teh previous sweep plotted mark is removed leaving a stagnant display until new stronger signals are received. Need to process the old pixel. On the TODO list.
    4. Modified the ethernet startup in teensyMAC() to use DHCP rather than static IP. The IP address setting is ignored.
    5. #ifdef REMOTE_OPS used to bypass the ethernet write function and remote IPO address in RadiConfig.h. This is only needed when we have the ability to remote monitor in the future.

## 3/18/2021

    1. Now have NTP client running if you have ethernet connected. The UTC time is displaying inside a button in the upper right corner over the S-meter with no border color turned on.  This supports easy relocation like all other buttons and can add tap to configure time settings (like format and Time Zone) later. User Profile 0 already has ethernet configured, profiles 1 and 2 do not. To change, either flip the bits in the user settings table or just change the user_Profile var in the main program file.
    2. The internal time keeping is now synced with the NTP time source.   The clock updates ever second.  Not seeing any measurable penalty, I am manually controlling the timing of sends and receives.
    3. Removed the key cap label updates for Mode, Filter and Rate as they have on screen status labels. AGC and some others will retain theirs. Code left in place to restore if desired later.

## 3/17/2021

    1. Display key now toggles spectrum DOT and BAR mode until we do something with the empty popup window.
    2. New #define SCREEN_WIDTH to reduce the size of several FFT storage arrays to only what can be used. 
    ***  NOTE: This is not fully tested yet and some instability has been observed.  ***
    3. VFOA/B display now formatted with dot notation (new format function) and larger font size.
    4. There are now 4 "panels" of keys laid out as below. This will change as feedback arrives. Some have self updating key cap labels.  At minimum they will turn blue when active/pressed. At least half actually do something now. It is now easy to start binding hardware controls to keys. Remember many settings are per-band so the on/off states will change accordingly. The rest are per user profile for now (global). 
    5. The Fn key is always in the first left position and cycles through the panels. The current panel number is updated in the Fn key cap. The Filter, Mode, AGC, Rate, and Atten keys are all self-updating to show their current setting. Others like XIT, RIT, NB level, NR level will eventually do the same, space permitting. Menu and Band keys raise pop up windows where many choices can be offered.
    
                   Key 1    Key 2    Key 3    Key 4    Key 5    Key 6     Key 7
    1st Panel      Fn-1     M:USB    F:3.20   Att:20   Pre      Rate      Band 
    2nd Panel      Fn-2     NB       NR       SPOT     Notch    AGC       Mute
    3rd panel      Fn-3     Menu     ANT      ATU      XMIT     Band -    Band +
    4th Panel      Fn-4     RIT      XIT      A/B      Fine     Display   Split
    
    5. The original touch labels for MODE, FILTER, AGC and TUNE STEP have changed and are now informational only. They are no longer touch enabled though they could be. They are now lined up on the left side. You use keys for all cases or, in the case of the "Rate" setting (formerly Tune Steps), the key or right and left swipe now cycles through 4 steps, 2500Hz 1000Hz, 100Hz, and 10Hz.  
    6. The "Fine" key puts RATE into 1Hz and 10Hz steps. You can swipe left/right to change between them.  It is intended the MF knob can be assigned any rate independent of this setting. I am using a detented knob so steps are (or will) be very easy with no overshoot, normally assigned 1, 2.5 or 5K per detent. The MF knob will have other assignments and will have focus on the settings a key has been touched for allowing 2 ways to change the setting in parallel.  I also slow down the VFO increment tune speed a bit when Fine is ON.
    7. The A/B key will be changed out to A->B (or A<-B on a 2nd tap).
    8. Keys can have extra functionality once tap-and-hold is implemented later. That will reduce the need to cycle through menus some.
    9. Default sample rate set to 102.4KHz and using 4096IQ FFT size. user_Profile 1 is active, not ethernet.  800 pixels give us a 20Khz span with high detail.
    10. Created a new SDR_8875.h file and moved most all of the header type stuff from the main file into it. Performed some formatting and grouping of like settings, added comments, highlighted user settings of interest.
    11. Added More screen object formatting control.
    12. Temporarily took over the SPOT key to make it a Ref Level adjust.  Each tap will change the Spectrum Ref Level by +5 and will stay between -130 and -220.  The Main loop timer is visible in the terminal window and you can see the impact of high grass levels.

## 3/15/2021

    1. Added ethernet init, read and write functions, dumping FFT output to a desktop UDP receiver app successfully. Use #define ENET to enable to disable this feature for compile or use the newly added User Settings table controls for enet_enable (hardware on/off) and/or enet_output. 
    2. The Ethernet UDP feature is intended for future remote control head operation but we may find other uses for it short term.
    3. I set User Profile 0 to have the enet hardware turned on. The other 2 it is off. I set User Profile 1 to be the default on the Github upload. 
    4. All profiles have the enet data output off by default.
    5. There is a new Enet Output key on Panel 3 that replaced the A/B key.
    6. Tapping the VFO area now swaps VFOs. The A/B button no longer has to be visible or even exist. It is now parked in the spare buttons parking lot.
    7. A main program Loop timer was added. It prints out max time lapsed (in ms) every 1 second.
    8. Added temperature readout in F and C units to the CPU usage report (for you overclockers who shall be unamed)
    9. Added a 5x span zoom OUT compressing 4096 data points into the 800px screen width. It worked but needs more work to center it in the display properly so is turned off for now.
    10. Did some performance testing. 22ms for the waterfall draw.  40ms to 120ms for the spectrum depending on how much grass is drawn. CPU usage is low in all cases. Near 0ms for the rest of the program loop.
    11. We noted today that with 4096IQ FFT, the global memory is largely consumed leaving not much left for local variables. 4096IQ is probably the limit unless Bob has more tricks.

## 3/14-15/2021

    1. Added buttons (keys) for 15 functions.  These are individual buttons now grouped in 3 "Panels". They can be enabled or diabled. They will get a Hide/Show property next. Using enable/disable to hide a button was not a good idea.
    2. The Menu, Mute, and Fn X keys are always displayed on the bottom row. The remaining space to the right holds about 4 keys on a 4.3" display with 100px wide keys. Any smaller and touch accuracy gets iffy.
    3. Fn X key: This cycles through available panels (groups) of keys. There are presently 4 keys per panel. These will certainly change over time, and for larger and wider displays, but this is a starting point.
    4. These keys are all 2 state keys today. Coming up are multi-state keys for controls like Tune Rate, AGC, Mode, and Filter selection. 
    5. The VFO A/B "window" now has properties like the keys do with the added feature of font style (i.e. Arial_32). Will add that to the keys properties later now that I have figured out how to do this.
    6. Band + and Band: These are the same as swipe up and down today. 
    7. Band: The Band key pops up a window meant to later show all bands and then either use touch or the MF knob can be used to select one. 
    8. Menu:  Like the Band key, it pops up a window also. Windows are blank at this point.
    9. When a button is not visible, it is disabled today. Therefore when the A/B button is hidden, it is disabled. Tapping on the VFO window to swap the active VFOs won't work until the key is visible again. Need to add a Hide/Show property.
    10. Keys that you turn on while on a given band the first time will turn to off when you change bands. This is normal because many settings are per-band. They all start out as OFF and only change when you are on that band. Those are not yet saved in EEPROM so after a reset you loose your changes today.

## 3/14/2021

    1. Tuned up the VFO encoder behavior more.
    2. Added a multifunction encoder for UI functions to use
    3. Updated to run 4096 FFT I and Q.
    4. Fixed the Peak signal detection frequency and power indication.
    5. Evenly spaced the bottom buttons and made them more rounded.  Added padx and pady to help align text within the button width and height.  Changed the button struct name to describe it better as draw_2_state_button vs draw_std_button.

## 3/12/2021

    1. Rearranged the screen layout to make room for buttons across the bottom and 2nd VFO. Looking for comments. The spectrum wide screen layout is the default and is immediately above the buttons. Eventually the buttons presented will rotate out to context sensitive or lesser used button, if they do not appear in a pop up window. They should work with optional multifunction knobs soon.
    2. Added VFO A and VFO B. Each VFO has last mode used associated with it. VFOs are same band only today.
    3. Mute button is working. The AGC button at bottom will show AGC on or off but controls nothing. Use the upper area AGC label as before.
    4. All the buttons except Atten, Preamp, Mute, and A/B are non functional placeholders for testing or future use. Need to decide on what buttons are needed overall and where and when they wll appear.
    5. The Atten and Preamp buttons work but have no hardware IO pins assigned yet.
    6. A/B button switches active VFO and the Mode associated with it last.  VFOs remember their last frequency used. 
    7. Split will use VFO B fro transmit when implemented later.
    8. The new buttons are "Standard Button" objects with properties like X, Y and text. They can be easily moved around and the touch system uses those same properties. These are 2 state buttons. Other complex buttons will be created to handle multiple states and lists of info (like tune step).
    9. Display.h is now very thin and will get thinner as conversion to button properties continues.
    10. User settings will eventually record the X,Y positions of each object like the buttons. It already stores band independent data like the spectrum layout, last band used, speaker enabled/volume, and mic gain and more.
    11. The spectrum region gain was increased so strong signals will nearly touch the top of the window. Bar chart is working again.  
    12. Slight tweaks continue to improve the waterfall color temperature intensity presentations.  
    13. Preselector will be automatically changed with band, there is no UI for that hardware. The Preamp and Attenutor are switching in and out with the preselector relays. Support will be provided for the PE4302 variable solid state attenutor. You can use a fixed attenuator (or none) if desired.
    14. Can tap the VFO display area to invoke the A/B toggle function, works same as the button does.
    15. Reworked VFO encoder handling to be very responsive.  Use the "enc_ppr_response" variable to scale down or up the number of counts per Hz.  600ppr is very fast and needs to be slowed down.  I find a value of 60 works good for 600ppr.   30 should be good for 300ppr, 1 or 2 for typical 24-36 ppr encoders.  Best to use even numbers above 1. 
    16. Fixed left/right swipe 100KHz change up/down to work again.
    17. Tuned touch some more, 3 finger swipes work better now for volume control.
    18. Removed timer for VFO encoder, not required. Dump counts accumulated but less than enc_ppr_response values after a longish timeout to prevent VFO drift.
    19. Fixed spectrum frequency label to track active VFO.

## 3/10/2021

    1. Fixed CW filter lockup issue.
    2. Press lower left corner hot spot for a suprise feature experiment.
    3. Added Mute button, works when speaker is ENABLED. If Speaker is OFF, then MUTE is always ON.

## 3/9/2021 to 3/10/2021

    1. Converted to 2048_IQ FFT.  Display will center the carrier frequency and trim off unused FFT data to fit the available screen area your layout is using.
    2. Created a 800 pixel wide layout to show 800 pixels of the 2048 bins available with 1:1 bin /pixel mapping.
    3. Removed all the old panel buttons.  Now jsut touch the labels to change them.  Preamp and Attenuator are now standalone rounded-corner buttons shown in 2 different color schemes. also relocated.
    4. Ring S -meter has been resized, moved and relabeled (with RA8875 minor library mod). The  bar type S meter is commented out. Let us know which you like better.
    5. Tuning step now cycles up, then back down, then back up. More natural to use. The BW cycles in a circle still, but in non-CW modes it skips the filters < 1.8KHz wide.
    6. Removed several delays and changed some timers. The touch response is now considerably improved, enough so to make it a usable interface. Swipe up and down to change bands now jumps to ham bands and last freqwuency used on each. Swipe right and left for 100Khz change, and now you can tap rapidly the tune steps, mode, or BW settings.
    7. Rewrote most of the header files to use the new data structures for storing per band settings, user profile settings, and to easily extend several feature areas such as settings for AGC, Tune Step, Filter BW, Modes.   Most files have shrunk in size.
    8. Per-Band setting stores thing like band edges, last used frequency for VFOA and VFOB, last used AGC, tune step, BW, Mode, Speaker Volume, Line In level, Line Out level, Mic gain, preselector band, preamp, attenuator, spectrum layout.  Also several have enable/ignore flags as some might be global depending on user preferences.
    9. 2 Hot spots exist for testing: Lower left corner is color temp adjust for graph. Lower right corner is Spectrum layout.  Will cycle through 10 predefined layouts.
    10. In CW mode you will see a signal image on both sides of center.  If you see an image on the non-CW modes, try resetting the CPU, this is believed to be due to the "Twin Peaks" startup audio channel sync issue under investigation.
    11.  Added 3 finger Swipe UP/DOWN for speaker volume control.  This is global.  Faster flick type moves seem to work best.

    NOTE: Changed settings are not stored until we move these structures into EEPROM.  You can modify the default ("factory Reset") settings in the structure tables in RadioConfig.h until then.

## 3/8/2021

    1. Removed redundant folder level.
    2. Converted from FFT256_IQ_F32 to FFT1024_IQ_F32 and added preset record 10 to go full width hiding the button panels.
    3. makefile (for VS Code users) modified to use a .vsteensy build folder 2 levels up to keep project folder clean for source control and file more portable.
    4. With the display going full width, buttons needed to relocate. Likely make context sensitive pop-up button panels and/or fixed panel on bottom row area under spectrum.
    5. Moved touch locations to the labels themselves in most cases.  Mode, BW, Ts, AGC, now just touch them to cycle through options
    6. Attn and Preamp not handled yet.
    7. Swipe left and right to increment Frequency by 100K steps.
    8. Swipe up and down to change band (for now 1MHz)
    9. Pinch changes scale for now.
    10. Created some wiki pages, particularly a Libraries page with links to the external libraries we use. 

## 3/6/2021

    Created initial upload for SDR with RA8875 display based on latest K7MDL build.
    1. Changed from FFT1024 to FFT256iq in preparaton for FFT1024.  
    2. Spectrum display now cenered with up and down band view
    3. DC line blanked, replaced in part with vertical grid line
    4. Added first attempt at an auto-adjust for the Spectrum Reference Level.  Will see Ref level number change
    5. Renamed Touch.h to UserInput.h sicne it covers buttons and launches tasks
    6. Created new file RadioConfig.h.  Holds the many data structures that wil be needed such as user layout prefences, per band setings, AGC settings, and more.
    7. Replaced the fixed span labels with live frequency labels at each end and center.
    8. Changed the Main filename to to SDR_RA8875.INO
    9. Peak Frequency Indicator now reads actual frequency vs offset.
    10. Moved Peak Power, Frequency, Scale, and REf Level below the top line
    11. Broke the horizontal grid line generation
    12. Began work on band up and down using band memory settings
    13. Thickened the dot mode spectrum line by averaging a few adjacxent bins and drawing a 2 pixel line.
