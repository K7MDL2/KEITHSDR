/*------------------------------------------------------------------------------- 
   AudioSDRpreProcessor_F32.cpp 

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
  ---  
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
------------------------------------------------------------------------------- */

#include "AudioSDRpreProcessor_F32.h"
#include "utility/dspinst.h"

// -----
void AudioSDRpreProcessor_F32::update(void) 
{ 
    audio_block_f32_t *blockI, *blockQ;
    blockI = receiveWritable_f32(0);                        // real (quadrature I) data
    blockQ = receiveWritable_f32(1);                        // imaginary (quadrature Q) data
    if (!blockI &&  blockQ) {release(blockQ); return;}
    if ( blockI && !blockQ) {release(blockI); return;}
    if (!blockI && !blockQ) return;
    //
    //---------------------------------------------------------------------------------------------
    // Teensy I2S single-sample delay IQ lag compensation:
    //   Note: The Teensy I2S bug causes a randomly occuring single-sample delay in I2S input
    //     channel 1 (blockQ in this case) on power-up or program reload.   To correct this,
    //     simply delay the samples in I2S input channel 0 (blockI) by a single sample so that
    //     the channels are synchronized again.
    // ---
    if (I2Scorrection == 1){
        float32_t temp = blockI->data[n_block-1];             // save the most recent sample for the next buffer
        for(int16_t i=n_block-1; i>0; i--) blockI->data[i] = blockI->data[i-1];
        blockI->data[0] = savedSample;
        savedSample = temp;}
    else if (I2Scorrection == -1){
        float32_t temp = blockQ->data[n_block-1];             // save the most recent sample for the next buffer
        for(int16_t i=n_block-1; i>0; i--) blockQ->data[i] = blockQ->data[i-1];
        blockQ->data[0] = savedSample;
        savedSample = temp;}
    //
    //---------------------------------------------------------------------------------------------
    //   I2S single-sample delay detection - look for spectral images in the data FFT.
    //   The method recognizes that errors in the phase (and amplitude) of the I and Q channels
    //   will generate symmetrical image lines in the complex spectrum, reflecting similar amplitudes in lines
    //   j and (n__FTT j).   If there is no I2S error, the magnitude ratio between these lines will be large.
    //   The decision on the existence of a delay error is based on the ratio between the powers of the 
    //   strongest spectral line and its image.
    // ---
    if (autoDetectFlag)
    {
        const   int16_t n_FFT = 128;
        const   int16_t min   = 5; 
        int     maxLine       = 0;
        //                                // At this point the output data block has already been updated
        for (int16_t i=0; i<128;i++) {        // Take 128 point FFT and compute the magnitude squared
            buffer[2*i]   = blockI->data[i];  // data is +1.0f to -1.0f for f32.  
            buffer[2*i+1] = blockQ->data[i];
        }
        // Take 128 point FFT and compute the magnitude squared
        arm_cfft_f32(&arm_cfft_sR_f32_len128, buffer, 0, 1);
        arm_cmplx_mag_squared_f32 (buffer, buffer, 128);       // "power" spectrum in elements 0 to 127
        // Find the strongest spectral line and compute the average line power across the whole spectrum.
        float32_t average_power = 0.0f;
        float32_t maximum_power = 0.0f;
        for (int16_t i=min; i<(n_FFT-min); i++) {                  // Ignore spectral lines around dc (noise)
            average_power  += buffer[i];
            if (buffer[i]>maximum_power) {
            maxLine       = i;
            maximum_power = buffer[i];
            }
        }
        average_power /= (n_FFT-2*min);                              // average power over all spectral lines
        // Find the ratio of the amplitude of the maximum power line to its spectral image
        float32_t imbalance_ratio = maximum_power/buffer[n_FFT-maxLine];  
            //  Make sure the maximum power line is well above the spectral "floor"
        if (maximum_power > spectralAvgMultiplier*average_power) {   // Limit to "strong" spectral lines
            //Serial.print("max="); Serial.print(maximum_power); Serial.print("> avg="); Serial.println(spectralAvgMultiplier*average_power);
            if (imbalance_ratio < minImbalanceRatio) 
            {
                //Serial.print("Fail    ");Serial.println(imbalance_ratio);
                failureCount++;   // Ratio too low, increment failure counter    
            }
            else 
            {
                //Serial.print("Success ");Serial.println(imbalance_ratio);
                failureCount = 0;                                   // Success - start the count over
            }
            if (failureCount > maxFailureCount) {                   // Too many failures (low ratios)in a row...
                I2Scorrection++ ;                                   // Try to correct
                if (I2Scorrection > 1) 
                    I2Scorrection = -1;                             // Try a new correction factor (-1, 0, or 1)...
                failureCount  = 0;                                       // and start over...
                successCount  = 0;
                Serial.print(">>>>>>>Correction Attempted="); Serial.print(I2Scorrection); Serial.print("    ratio="); Serial.println(imbalance_ratio);
            }
            successCount++;
            //Serial.println("**************Success");
        }
        if (successCount > maxSuccessCount) {
            Serial.println("\n AutoCorrect turned OFF\n");
            autoDetectFlag = false;                    // Turn autoCorrection off and accept the current correction
        }
    }
  
    //
    // ----------------------------------------------------------------------
    // Swap I and Q channels to I in channel and Q in channel 0 to correct for
    // incorrect quadrature input connections
    if (IQswap){
        for (int16_t i=0; i<128; i++) {
        float32_t temp = blockI->data[i];
        blockI->data[i] = blockQ->data[i];
        blockQ->data[i] = temp;
        }
    }
    transmit(blockI, 0);
    transmit(blockQ, 1);
    release(blockQ);
    release(blockI);
}
// -------------------------- Public Functions ----------------------
// ---
// --- Enable auto detection and correction of the I2S input error
void  AudioSDRpreProcessor_F32::startAutoI2SerrorDetection(void) {
    autoDetectFlag = true;
    I2Scorrection  = 0;
    failureCount   = 0;
    successCount   = 0;
 // autoDetectFlag = false;
}
// ---
// --- Disable auto detection and correction of the I2S input error
void  AudioSDRpreProcessor_F32::stopAutoI2SerrorDetection(void) {
    autoDetectFlag = false;
    I2Scorrection  = 0;            // Revert to no compensation
}
// --- Return the state of the auto detection
//     true = auto detection is active, false = auto detection is inactve
bool AudioSDRpreProcessor_F32::getAutoI2SerrorDetectionStatus(void) {return autoDetectFlag;}
//
// --- Manually set I2S error correction mode 
void  AudioSDRpreProcessor_F32::setI2SerrorCompensation(int16_t correction) {
    I2Scorrection   = correction;
    autoDetectFlag  = false;                 // Cancel auto correction if active
}
// ---
// ---  Fetch the current state of the I2S error correction (on or off)
int16_t AudioSDRpreProcessor_F32::getI2SerrorCompensation(void) {return I2Scorrection;}
// ---
// --- Swap quadrature inputs from I on channel 0 to I on channel 1
void  AudioSDRpreProcessor_F32::swapIQ(boolean swap) {IQswap = swap;}
