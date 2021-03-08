# KEITHSDR
Teensy4.X with PJRC audio card Arduino based SDR Radio project.

    3/6/2021
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
