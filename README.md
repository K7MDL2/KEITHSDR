# KEITHSDR
Teensy4.X with PJRC audio card Arduino based SDR Radio project.  

The project has a forum and more wiki pages and info at https://groups.io/g/keithsdr

This is a GitHub Repository for Group Member builds and variations of hardware and feature experiments

The folder SDR_RA8875 is one such SDR project folder and is built on Teensy 4.1 with Audio Card and RA8875 based touchscreen at 800x480.  
  
  Features:
  
  1. 2048point I&Q FFT using the OpenAudio_Library converted to 32 bit Floating Point by W7PUA.
  2. Resizable Spectrum and Waterfall module can be dropped into most any Arduino project using the RA8875 or with minor mods RA8876 based display controllers for minimal CPU overhead for waterfall graphics.  
  3. Supports the FT5206 Capacitive Touch screen controller for touch and gestures.
  4. Custom gesture code replaces the poorly working internal FT5206 Gesture detection.  Found in UserInput.h

See the README file in each project for specifics about that build.

UI as of 3/10/2021.  80M FT-8 with CW on teh sides.  Testing style of new buttons and wide graph with 2048FFTIQ. 
  Sample rate at 102.4KHz on a 800x480 4.3" display. 24% max CPU, 4% average.  

![UI Screenshot 3-10-2021](https://github.com/K7MDL2/KEITHSDR/blob/main/SDR_RA8875/Pictures/3-10-2021%20ScreenShot.jpg)

