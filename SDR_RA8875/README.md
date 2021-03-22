# KEITHSDR

Teensy4.X with PJRC audio card Arduino based SDR Radio project.

3/20-21/2021

    1.Moved step increment/decrement control into Rate() function.  Left old step.h thinned down to set the tune rate directly in the database and call display. 
    2. Moved filter increment/decrement control into Filter() function.  It calls select bandwidth() in bandwidth2.h as before which now has the increment and database logic I inserted some time back removed. It now moves the value upwards then downwards and back upwards like the rate key does.
    3. With 1 minor change, new bandwidth.h files will plug right in. Since the current bandwidth value of bndx is stored in the per-band database a global bndx var is not used but is instead passed as an arg. You add (int bndx) to the function definition like this:
        void selectBandwidth(int bndx)
    You can call it direct or through the Filter(dir) function, same as Rate(dir). dir set to -1  or +1 increments the current filter down or up. dir = 0 cycles the current filter (or tune step for Rate(dir) in the last direction until it hits top or bottom then reverses direction.  The +1 and -1 are needed particularly for swipe commands which are left and right for changing the tune step rate.
    4. Added SV1AFN Bandpass Filter board (aka Preselector board) library to the repository, updated to support the 60M band. It allows I2C control via a MCP23017 I2C Port Expander module.  Preamp, Attenuator and of course 10 bands worth of band pass filters.  The Control.h, Preamp() and Atten() talk to the BPF board.  Working good after a rough start with some I2C bus issues.  Ran I2C scanner and power cycled and things cleared up. Something to monitor.
    5. SelectFrequency() now monitors for band edges and will BYPASS the BPFs when tuned outside the ham bands.
    6. Comment out "#define SV1AFN_BPF" in the SDR_8875.h file to exclude all code related to the Bandpass filter board in case of any side effects if you do not have a board connected.
    7. Staged PAtten_Set() function in Controls.h for the PE4302 digial step attenuator. Need to map it to IO pins TBD.

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
    10. MOved Peak Power, Frequency, Scale, and REf Level below the top line
    11. Broke the horizontal grid line generation
    12. Began work on band up and down using band memory settings
    13. Thickened the dot mode spectrum line by averaging a few adjacxent bins and drawing a 2 pixel line.
