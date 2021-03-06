/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Arduino.h>
#include "analyze_fft256iq.h"
#include "utility/sqrt_integer.h"
#include "utility/dspinst.h"

//#include "analyze_fft256iq.h"
//#include "utility/sqrt_integer.h"
//#include "utility/dspinst.h"


// 140312 - PAH - slightly faster copy
static void copy_to_fft_buffer(void *destination, const void *source1, const void *source2)
{
  const uint16_t *src1 = (const uint16_t *)source1;
        const uint16_t *src2 = (const uint16_t *)source2;
  uint32_t *dst = (uint32_t *)destination;

  for (int i=0; i < AUDIO_BLOCK_SAMPLES; i++) {
    
                *dst++ = *src1++ | ((*src2++) <<16); 
  }
}

static void apply_window_to_fft_buffer(void *buffer, const void *window)
{
  int16_t *buf = (int16_t *)buffer;
  const int16_t *win = (int16_t *)window;;

  for (int i=0; i < 256; i++) {

                buf[0] = (buf[0] * *win) >> 15;
                buf[1] = (buf[1] * *win) >> 15;
    buf += 2;
                win++;
  }

}

void AudioAnalyzeFFT256IQ::update(void)
{
  audio_block_t *block_i,*block_q;

  
        block_i=receiveReadOnly(0); 
        block_q=receiveReadOnly(1);     
  if (!block_i || !block_q ) return;
  if (!prevblock_i || !prevblock_q) {
    prevblock_i = block_i;
                prevblock_q = block_q;
    return;
  }
  copy_to_fft_buffer(buffer, prevblock_i->data,prevblock_q->data);
  copy_to_fft_buffer(buffer+256, block_i->data,block_q->data);
  
  if (window) apply_window_to_fft_buffer(buffer, window);
  arm_cfft_radix4_q15(&fft_inst, buffer);

  // G. Heinzel's paper says we're supposed to average the magnitude
  // squared, then do the square root at the end.
  if (count == 0) {
    for (int i=0; i < 256; i++) {
      uint32_t tmp = *((uint32_t *)buffer + i);
      uint32_t magsq = multiply_16tx16t_add_16bx16b(tmp, tmp);
      sum[i] = magsq / naverage;
    }
  } else {
    for (int i=0; i < 256; i++) {
      uint32_t tmp = *((uint32_t *)buffer + i);
      uint32_t magsq = multiply_16tx16t_add_16bx16b(tmp, tmp);
      sum[i] += magsq / naverage;
    }
  }
  if (++count == naverage) {
    count = 0;

    for (int i = 0; i < 256; i++) {
      output[255 - (i ^ 128)] = sqrt_uint32_approx(sum[i]);
    }

    outputflag = true;
  }
  release(prevblock_i);
        release(prevblock_q);
  prevblock_i = block_i;
        prevblock_q = block_q;
}
