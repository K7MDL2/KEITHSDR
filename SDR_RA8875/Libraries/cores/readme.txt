This file is a modified version of the original TeensyDuino file called AudioStream.h

C:\Program Files (x86)\Arduino\hardware\teensy\avr\cores\teensy4

The changes are in the #defines at the top to set the default to either 44.1KHz or 48KHz.  

We are using 48KHz.  Kept the sample block count the same at 128.  The CWKeyer project used 32 to keep latency low.

