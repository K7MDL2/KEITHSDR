/*--------------------------------------------------------------------------------------- 
  AudioSDRpreprocessor.h 

  Function: A input pre-proccessor to "condition"  quadrature (IQ) input signals before passing 
            to the AudioSDR software-defined-radio Teensy 3.6 Audio block.

  Author:   Derek Rowell (drowell@mit.edu)
  Date:     April 26, 2019  

  Notes:    Includes the following functions:
            a) Automatically detect and correct the random Teensy single-sample delay
               bug in the I2S input stream,
            b) Manually turn on and off I2S error correction (overides the automatic correction)
            c) Return the current I2S error correction state.
            d) Swap the I and Q input channels.   The convention used in the AudioSDR system
              is that the I channel should be connected to the I2S input 0 (left) and that
              the Q channel should be connect to input 1 (right).   If your hardware does not
              use this convention, you can use this software fix.
  --  
  Copyright (c) 2019 Derek Rowell
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
--------------------------------------------------------------------------------------------- */

#ifndef audio_sdr_preprocessor_h_
#define audio_sdr_preprocessor_h_
#include "core_pins.h"
#include "AudioStream.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "Arduino.h"
//
#define maxSuccessCount        1000
#define maxFailureCount        10
#define minImbalanceRatio      10.0
#define spectralAvgMultiplier  10.0
#define n_block 128

class AudioSDRpreProcessor: public AudioStream {
  public:
    AudioSDRpreProcessor() : AudioStream(2, inputQueueArray) {}
    virtual void update(void);
    // --
    // Public  functions
    void    startAutoI2SerrorDetection(void); 
    void    stopAutoI2SerrorDetection(void); 
    bool    getAutoI2SerrorDetectionStatus(void);
    void    setI2SerrorCompensation(int correction);
    int16_t getI2SerrorCompensation(void);
    void    swapIQ(boolean swap);
    // -- 
  private:
    audio_block_t *inputQueueArray[2];
    float   buffer[256];
    float   image_ratio   = 1.0;
    float   avg;
    int16_t I2Scorrection = 0;
    int16_t savedSample   = 0;
    int16_t failureCount  = 0;
    int16_t successCount  = 0;
     bool   IQswap = false;
    bool    I2SerrorCheckEnabled = true;
    bool    autoDetectFlag  = false;
};
#endif
