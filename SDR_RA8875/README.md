# KEITHSDR

Teensy4.X with PJRC audio card Arduino based SDR Radio project.

## 3/92021 to 3/10/2021

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
    10. MOved Peak Power, Frequency, Scale, and REf Level below the top line
    11. Broke the horizontal grid line generation
    12. Began work on band up and down using band memory settings
    13. Thickened the dot mode spectrum line by averaging a few adjacxent bins and drawing a 2 pixel line.
