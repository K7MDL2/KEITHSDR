/*--------------------------------------------------------------------------------------- 
  AudioSDRpreProcessor_F32.h 

  Function: A input pre-proccessor to "condition"  quadrature (IQ) input signals before passing 
            to the AudioSDR software-defined-radio Teensy 3.6 Audio block.

  Author:   Derek Rowell (drowell@mit.edu)
  Date:     April 26, 2019  
  Modified  Feb 26, 2022 by K7MDL for F32 library use

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

#include "Arduino.h"
#include "core_pins.h"
#include "AudioStream_F32.h"
#include "arm_math.h"
#include "mathDSP_F32.h"
#include "arm_const_structs.h"

//
#define maxSuccessCount        10  //1000
#define maxFailureCount        10  //10
#define minImbalanceRatio      10.0f //10
#define spectralAvgMultiplier  10.0f //10
#define n_block 128

class AudioSDRpreProcessor_F32: public AudioStream_F32 {
  public:
    AudioSDRpreProcessor_F32() : AudioStream_F32(2, inputQueueArray_f32) {}
    virtual void update(void);
    // --
    // Public  functions
    void    startAutoI2SerrorDetection(void); 
    void    stopAutoI2SerrorDetection(void); 
    bool    getAutoI2SerrorDetectionStatus(void);
    void    setI2SerrorCompensation(int16_t correction);
    int16_t getI2SerrorCompensation(void);
    void    swapIQ(boolean swap);
    // -- 
  private:
    audio_block_f32_t *inputQueueArray_f32[2];
    float32_t   buffer[256];
    float32_t   image_ratio   = 1.0f;
    float32_t   avg;
    int16_t     I2Scorrection = 0;
    float32_t   savedSample   = 0;
    int16_t     failureCount  = 0;
    int16_t     successCount  = 0;
    bool        IQswap = false;
    bool        I2SerrorCheckEnabled = true;
    bool        autoDetectFlag  = false;
};
#endif
