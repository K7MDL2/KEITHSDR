# KEITHSDR
Teensy4.X with PJRC audio card Arduino based SDR Radio project.  

The project has a forum and more wiki pages and info at https://groups.io/g/keithsdr

This is a GitHub Repository for Group Member builds and variations of hardware and feature experiments

The folder SDR_RA8875 is one such SDR project folder and is built on Teensy 4.1 with Audio Card and RA8875 based capacitive touchscreen at 800x480 or a RA8876 capacitive touchscreen at 1024x600.  
  
  Features:
  
  1. 4096 point I&Q FFT using the OpenAudio_Library converted to 32 bit Floating Point by W7PUA.
  2. Resizable Spectrum and Waterfall module can be dropped into most any Arduino project using the RA8875 or with minor mods RA8876 based display controllers for minimal CPU overhead for waterfall graphics.  As of Jan 2022 the Spectrum Display related code is now in an new Arduino Library.
  3. Supports the FT5206 Capacitive Touch screen controller for touch and gestures.
  4. Custom gesture code replaces the poorly working internal FT5206 Gesture detection.  Found in UserInput.h
  5. Table driven UI configuration, per-band settings, and User Profile settings with centralized control functions.
  6. Supports both RA8875 and RA8876 based displays with capacitive touch controllers
  7. Supports USB Host connection to a RS-HFIQ 5W SDR transceiver.  Can use many other hardware choices and build your own RF sections.

See the README file in each project for specifics about that build.

UI as of 3/25/2021.  40M FT-8.  Testing colors and styles of indicators.

Sample rate at 102.4KHz with 4096 point I and Q FFT on a 800x480 4.3" display. 24% max CPU, 4% average. 1:1 bins-per-pixel mapping.

![UI Screenshot 3-25-2021](https://github.com/K7MDL2/KEITHSDR/blob/main/SDR_RA8875/Pictures/3-25-2021%20ScreenShot.jpg)
