# KEITHSDR
Teensy4.X with PJRC audio card Arduino based SDR Radio project.  

The project has a forum and more wiki pages and info at https://groups.io/g/keithsdr

This is a GitHub Repository for Group Member builds and variations of hardware and feature experiments.  There is a simpler version maintained by Keith, and this is my derivative, maintained by me, Mike.

The folder SDR_RA8875 is one such SDR project folder and is built on Teensy 4.1 with Audio Card and RA8875 based capacitive touchscreen at 800x480, or a RA8876 capacitive touchscreen at 1024x600. The UI elements use table driven data so they are resizeable/relocatable. The size screen is less important than the controller used. I am leveraging the BTE (Bit Transfer Engine) feature of these controllers to offload the CPU enabling high resolution spectrum and waterfall graphics with minimal CPU overhead.
  
  ![K7MDL Front Panel RA8875 Compact Teensy SDR](https://github.com/K7MDL2/KEITHSDR/blob/main/SDR_RA8875/Pictures/TeensySDR%20in%20Hammond%201455N1601-Front-1.jpg)
   More pics at https://github.com/K7MDL2/KEITHSDR/tree/main/SDR_RA8875/Pictures
   and at my website https://k7mdl2.wixsite.com/k7mdl/copy-of-teensy-4-sdr
  
  
  Features:
  
  1. 4096, 2048 and 1024 point I&Q FFTs using the OpenAudio_Library converted to 32 bit Floating Point by W7PUA.  Uses all 3 FFT sizes chosen according to the zoom level.  Zoom simply uses FFT size today, no other processing done.
  2. Resizable Spectrum and Waterfall module can be dropped into most any Arduino project using the RA8875 or with minor mods RA8876 based display controllers for minimal CPU overhead for waterfall graphics.  As of Jan 2022 the Spectrum Display related code is now in an new Arduino Library.
  3. Supports the FT5206 Capacitive Touch screen controller for touch and gestures.
  4. Custom gesture code replaces the poorly working internal FT5206 Gesture detection.  Found in UserInput.h
  5. Table driven UI configuration, per-band settings, and User Profile settings with centralized control functions.
  6. Supports both RA8875 and RA8876 based displays with capacitive touch controllers
  7. Supports USB Host connection to a RS-HFIQ 5W SDR transceiver.  Can use many other hardware choices and build your own RF sections.
  8. Deafult sample rate used is 48KHz.
  9. Supports USB Audio connection ((both in ane Out) at 48KHz to a PC for external digital mode programs along with CAT control port using the Kenwood/Elecraft K3 command set.  Works great with WSJT-X.  Instructions provided in the Wiki on how to set up a custom Dual Serial+Audio USB 48Khz configuration.

See the README file in each project for specifics about that build.

UI as of 3/25/2021.  40M FT-8.  Testing colors and styles of indicators.

Sample rate at 102.4KHz with 4096 point I and Q FFT on a 800x480 4.3" display. 24% max CPU, 4% average. 1:1 bins-per-pixel mapping.

![UI Screenshot 3-25-2021](https://github.com/K7MDL2/KEITHSDR/blob/main/SDR_RA8875/Pictures/3-25-2021%20ScreenShot.jpg)

There are many Wiki Pages with hardware and software notes and how to build and configure.  I sugest reviewing them and then this page to start your first build.
https://github.com/K7MDL2/KEITHSDR/wiki/Getting-Started-Building-SDR_887x-SDR-program
