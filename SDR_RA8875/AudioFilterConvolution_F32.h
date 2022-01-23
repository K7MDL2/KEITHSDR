/**
  ******************************************************************************
  * @file    AudioFilterConvolution_F32.h
  * @author  Giuseppe Callipo - IK8YFW - ik8yfw@libero.it
  * @version V1.0.0
  * @date    02-05-2021
  * @brief   F32 Filter Convolution
  *
  ******************************************************************************
  ******************************************************************************
   This software is based on the  AudioFilterConvolution routine 
   Written by Brian Millier on Mar 2017
   
   https://circuitcellar.com/research-design-hub/fancy-filtering-with-the-teensy-3-6/
   and modified by Giuseppe Callipo - ik8yfw.
   Modifications: 
    1) Class refactoring, change some methods visibility;
    2) Filter coefficients calculation included into class;
    3) Change the class for running in both with F32 
     OpenAudio_ArduinoLibrary for Teensy; 
  4) Added initFilter method for single anf fast initialization and on 
     the fly reinititializzation; 
  5) Optimize it to use as output audio filter on SDR receiver.
  *******************************************************************/

#ifndef AudioFilterConvolution_F32_h_
#define AudioFilterConvolution_F32_h_

#include <AudioStream_F32.h>
#include <arm_math.h>
#include <arm_const_structs.h>

#define MAX_NUMCOEF 513
#define TPI           6.28318530717959f
#define PIH           1.57079632679490f
#define FOURPI        2.0 * TPI
#define SIXPI         3.0 * TPI

class AudioFilterConvolution_F32 : 
 public AudioStream_F32
{

public:

  AudioFilterConvolution_F32(const AudioSettings_F32 &settings) : AudioStream_F32(1, inputQueueArray) {
        // Performs the first initialize
        fs = settings.sample_rate_Hz;
      };
  
  boolean begin(int status);
  virtual void update(void);
  void passThrough(int stat);
  void initFilter (float32_t fc, float32_t Astop, int type, float dfc);
  
private:
  #define BUFFER_SIZE 128
  float32_t fs;
  audio_block_f32_t *inputQueueArray[1];
  float32_t *sp_L;
  volatile uint8_t state;
  int i;
  int k;
  int l;
  int passThru;
  int enabled;
  float32_t FIR_Coef[MAX_NUMCOEF];
  const uint32_t FFT_length = 1024;
  float32_t FIR_coef[2048] __attribute__((aligned(4)));
  float32_t FIR_filter_mask[2048] __attribute__((aligned(4)));
  float32_t buffer[2048] __attribute__((aligned(4)));
  float32_t tbuffer[2048]__attribute__((aligned(4)));
  float32_t FFT_buffer[2048] __attribute__((aligned(4)));
  float32_t iFFT_buffer[2048] __attribute__((aligned(4)));
  float32_t float_buffer_L[512]__attribute__((aligned(4)));
  float32_t last_sample_buffer_L[512];
  void impulse(float32_t *coefs);
  float32_t Izero (float32_t x);
  float     m_sinc(int m, float fc);
  void      calc_FIR_coeffs (float * coeffs, int numCoeffs, float32_t fc, float32_t Astop, int type, float dfc, float Fsamprate);
  
  };

#endif
