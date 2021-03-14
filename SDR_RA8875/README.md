# KEITHSDR

Teensy4.X with PJRC audio card Arduino based SDR Radio project.

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
    10. MOved Peak Power, Frequency, Scale, and REf Level below the top line
    11. Broke the horizontal grid line generation
    12. Began work on band up and down using band memory settings
    13. Thickened the dot mode spectrum line by averaging a few adjacxent bins and drawing a 2 pixel line.
