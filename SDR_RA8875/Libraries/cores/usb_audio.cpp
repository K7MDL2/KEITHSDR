/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>
#include "usb_dev.h"
#include "usb_audio.h"
#include "debug/printf.h"

#ifdef AUDIO_INTERFACE

bool AudioInputUSB::update_responsibility;

struct usb_audio_features_struct AudioInputUSB::features = {0,0,FEATURE_MAX_VOLUME/2};

extern volatile uint8_t usb_high_speed;
static void rx_event(transfer_t *t);
static void tx_event(transfer_t *t);

/*static*/ transfer_t rx_transfer __attribute__ ((used, aligned(32)));
/*static*/ transfer_t sync_transfer __attribute__ ((used, aligned(32)));
/*static*/ transfer_t tx_transfer __attribute__ ((used, aligned(32)));
DMAMEM static uint8_t rx_buffer[AUDIO_RX_SIZE] __attribute__ ((aligned(32)));
DMAMEM static uint8_t tx_buffer[AUDIO_RX_SIZE] __attribute__ ((aligned(32)));
DMAMEM uint32_t usb_audio_sync_feedback __attribute__ ((aligned(32)));

uint8_t usb_audio_receive_setting=0;
uint8_t usb_audio_transmit_setting=0;
uint8_t usb_audio_sync_nbytes;
uint8_t usb_audio_sync_rshift;

uint32_t feedback_accumulator;

volatile uint32_t usb_audio_underrun_count = 0;
volatile uint32_t usb_audio_overrun_count = 0;

volatile int16_t buffer_counter = 0;  // used to monitor the current buffer filling

static void rx_event(transfer_t *t)
{
	if (t) {
		int len = AUDIO_RX_SIZE - ((rx_transfer.status >> 16) & 0x7FFF);
		printf("rx %u\n", len);
		usb_audio_receive_callback(len);
	}
	usb_prepare_transfer(&rx_transfer, rx_buffer, AUDIO_RX_SIZE, 0);
	arm_dcache_delete(&rx_buffer, AUDIO_RX_SIZE);
	usb_receive(AUDIO_RX_ENDPOINT, &rx_transfer);
}

static void sync_event(transfer_t *t)
{
	// USB 2.0 Specification, 5.12.4.2 Feedback, pages 73-75
	//printf("sync %x\n", sync_transfer.status); // too slow, can't print this much
	usb_audio_sync_feedback = feedback_accumulator >> usb_audio_sync_rshift;
	usb_prepare_transfer(&sync_transfer, &usb_audio_sync_feedback, usb_audio_sync_nbytes, 0);
	arm_dcache_flush(&usb_audio_sync_feedback, usb_audio_sync_nbytes);
	usb_transmit(AUDIO_SYNC_ENDPOINT, &sync_transfer);
}

static void copy_to_buffers(const uint32_t *src, int16_t *left, int16_t *right, unsigned int len)
{
	uint32_t *target = (uint32_t*) src + len; 
	while ((src < target) && (((uintptr_t) left & 0x02) != 0)) {
		uint32_t n = *src++;
		*left++ = n & 0xFFFF;
		*right++ = n >> 16;
	}

	while ((src < target - 2)) {
		uint32_t n1 = *src++;
		uint32_t n = *src++;
		*(uint32_t *)left = (n1 & 0xFFFF) | ((n & 0xFFFF) << 16);
		left+=2;
		*(uint32_t *)right = (n1 >> 16) | ((n & 0xFFFF0000)) ;
		right+=2;
	}

	while ((src < target)) {
		uint32_t n = *src++;
		*left++ = n & 0xFFFF;
		*right++ = n >> 16;
	}
}

//
// For very small audio block sizes, the original code cannot work
// so we must use the DL1YCF version (since FEEDBACK_SOF) requires
// some adjustments in usb_audio.h)
//
#if (AUDIO_BLOCK_SAMPLES < 64) && !defined(USB_AUDIO_FEEDBACK_SOF) && !defined(USB_AUDIO_FEEDBACK_DL1YCF)
#warning AUDIO_BLOCK_SAMPLES too small, using DL1YCF feedback algorithm
#define USB_AUDIO_FEEDBACK_DL1YCF
#endif



#if defined(USB_AUDIO_FEEDBACK_SOF)

// Precompute values for USB_AUDIO_FEEDBACK_SOF
// Total 0.125us to compute exponential moving average
static const uint32_t USB_AUDIO_FEEDBACK_SOF_MAX = 480000;

// Buffers
static const uint16_t USB_AUDIO_INPUT_BUFFERS=4;

// Limit for near under/over run to start adjusting clock
//static const uint16_t USB_AUDIO_GUARD_RAIL=4;

#ifdef USB_AUDIO_48KHZ	
static const uint32_t USB_AUDIO_FEEDBACK_INIT = 805306368; // 48 * 2^24
static const uint32_t USB_AUDIO_FEEDBACK_MAX  = 805641912; // 48.020 * 2^24
static const uint32_t USB_AUDIO_FEEDBACK_MIN  = 804970824; // 47.980 * 2^24
#else
static const uint32_t USB_AUDIO_FEEDBACK_INIT = 739875226; // 44.1 * 2^24
static const uint32_t USB_AUDIO_FEEDBACK_MAX  = 740210769; // 44.120 * 2^24
static const uint32_t USB_AUDIO_FEEDBACK_MIN  = 739539681; // 44.080 * 2^24
#endif

// Static in this context (outside of a function) means the variable scope is this file only
static audio_block_t *ready_left[USB_AUDIO_INPUT_BUFFERS];
static audio_block_t *ready_right[USB_AUDIO_INPUT_BUFFERS];
static volatile uint16_t write_index = 0;
static volatile uint16_t read_index = 0;
static volatile uint16_t write_count = 0;

//static uint32_t usb_audio_near_overrun_count = 0;
//static uint32_t usb_audio_near_underrun_count = 0;

static volatile uint64_t usb_audio_samples_consumed = 0;
static volatile uint32_t usb_audio_frames_counted = 0;

void usb_audio_update_sof_count(void)
{
	if (usb_high_speed) usb_audio_frames_counted++;
	else usb_audio_frames_counted += 8; // full speed sof is 1ms or 8*0.125us
}

void usb_audio_configure(void)
{
	feedback_accumulator = USB_AUDIO_FEEDBACK_INIT;

	for (uint16_t i = 0; i<USB_AUDIO_INPUT_BUFFERS; i++) {
		ready_right[i] = NULL;
		ready_left[i] = NULL;
	}

	// Microsoft windows still expects 10.14 when UAC1 even at high speed
	// Linux accepts this too
	//if (usb_high_speed) {
	//	usb_audio_sync_nbytes = 4;
	//	usb_audio_sync_rshift = 8;
	//} else {
		usb_audio_sync_nbytes = 3;
		usb_audio_sync_rshift = 10;
	//}
	memset(&rx_transfer, 0, sizeof(rx_transfer));
	usb_config_rx_iso(AUDIO_RX_ENDPOINT, AUDIO_RX_SIZE, 1, rx_event);
	rx_event(NULL);
	memset(&sync_transfer, 0, sizeof(sync_transfer));
	usb_config_tx_iso(AUDIO_SYNC_ENDPOINT, usb_audio_sync_nbytes, 1, sync_event);
	sync_event(NULL);
	memset(&tx_transfer, 0, sizeof(tx_transfer));
	usb_config_tx_iso(AUDIO_TX_ENDPOINT, AUDIO_TX_SIZE, 1, tx_event);
	tx_event(NULL);
	usb_start_sof_interrupts(AUDIO_INTERFACE);
}

void AudioInputUSB::begin(void)
{
	// update_responsibility = update_setup();
	// TODO: update responsibility is tough, partly because the USB
	// interrupts aren't sychronous to the audio library block size,
	// but also because the PC may stop transmitting data, which
	// means we no longer get receive callbacks from usb.c
	update_responsibility = false;
}


// Called from the USB interrupt when an isochronous packet arrives
// we must completely remove it from the receive buffer before returning
//
void usb_audio_receive_callback(unsigned int len)
{
	static audio_block_t *left = NULL;
	static audio_block_t *right = NULL;
	const uint32_t *data;
	uint16_t avail;

	len >>= 2; // 1 sample = 4 bytes: 2 left, 2 right
	data = (const uint32_t *)rx_buffer;

	if (left == NULL) {
		left = AudioStream::allocate();
		if (left == NULL) return;
	}
	if (right == NULL) {
		right = AudioStream::allocate();
		if (right == NULL) return;
	}

	while (len > 0) {

		avail = AUDIO_BLOCK_SAMPLES - write_count;
		if (len < avail) {
			copy_to_buffers(data, left->data + write_count, right->data + write_count, len);
			buffer_counter += len;
			write_count = write_count + len;

			// Check for near overrun condition
			//if ((avail - len) >= USB_AUDIO_GUARD_RAIL) return;

			//// Within guard rail so check if possible to send now
			//if (ready_left[write_index] || ready_right[write_index]) {
			//	usb_audio_near_overrun_count++;
			//	usb_audio_samples_consumed -= uint64_t(usb_audio_samples_consumed >> 25);
			//}
			return;

		} else if (avail > 0) {
			copy_to_buffers(data, left->data + write_count, right->data + write_count, avail);
			buffer_counter += avail;
			data += avail;
			len -= avail;

			if (ready_left[write_index] || ready_right[write_index]) {
				// buffer overrun, PC sending too fast
				write_count = write_count + avail;
				if (len > 0) {
					usb_audio_overrun_count++;
					//usb_audio_samples_consumed -= uint64_t(usb_audio_samples_consumed >> 25);

				} 
				//else {
					// Exactly enough space so near overrun
				//	usb_audio_near_overrun_count++;
				//	usb_audio_samples_consumed -= uint64_t(usb_audio_samples_consumed >> 25);
				//}
				return;
			}
			
			send:


			ready_left[write_index] = left;
			ready_right[write_index] = right;

			write_count = 0;

			write_index++;
			if (write_index >= USB_AUDIO_INPUT_BUFFERS) write_index = 0;
	
			left = AudioStream::allocate();
			if (left == NULL) {
				return;
			}
			right = AudioStream::allocate();
			if (right == NULL) {
				AudioStream::release(left);
				return;
			}

		} else {

			if (ready_left[write_index] || ready_right[write_index]) {
				// Continued overrun, dropping full length
				usb_audio_overrun_count++;
				return;
			}
			goto send; // recover from buffer overrun
		}
	}
}


void AudioInputUSB::update(void)
{
	audio_block_t *left, *right;
	static uint16_t rate_errors = 0;

	uint16_t next_read_index;

	__disable_irq();

	//uint16_t local_write_index = write_index;
	//uint16_t local_write_count = write_count;
	left = ready_left[read_index];
	ready_left[read_index] = NULL;
	right = ready_right[read_index];
	ready_right[read_index] = NULL;
	if (left && right) buffer_counter -= AUDIO_BLOCK_SAMPLES;
	uint16_t local_buf_cnt = buffer_counter;

	uint32_t frames_counted = usb_audio_frames_counted;
	uint16_t to_remove;
	// This is the frames to average over, exponential moving average
	if (frames_counted > USB_AUDIO_FEEDBACK_SOF_MAX) {
		to_remove = frames_counted - USB_AUDIO_FEEDBACK_SOF_MAX;
		usb_audio_frames_counted -= to_remove;
	} else {
		to_remove = 0;
	}

	__enable_irq();


#if 0
	//
	// This prints out the actual value of the feedback
	// correction (converted to Hz) and the min and max
	// buffer filling few times per second on
	// the console. This can be used to debug the
	// feedback control loop
	//
	static uint16_t debug=0;
	static uint16_t min_buf=9999;
	static uint16_t max_buf=0;
	if (local_buf_cnt > max_buf) max_buf=local_buf_cnt;
	if (local_buf_cnt < min_buf) min_buf=local_buf_cnt;

	if (++debug > 1500) {
		debug=0;
		Serial.print("SOF: Corr= ");
		Serial.print(feedback_accumulator*0.00005960464477539063);
		Serial.print(" MinBuf=");
		Serial.print(min_buf);
		Serial.print(" MaxBuf=");
		Serial.print(max_buf);
		Serial.print(" O");
		Serial.print(usb_audio_overrun_count);
		//Serial.print(" NO");
		//Serial.print(usb_audio_near_overrun_count);
		Serial.print(" U");
		Serial.println(usb_audio_underrun_count);
		//Serial.print(" NU");
		//Serial.println(usb_audio_near_underrun_count);
		min_buf=9999;
		max_buf=0;
	}
#endif

	// Hope the compiler optimizes this to a constant
	usb_audio_samples_consumed += (uint64_t(AUDIO_BLOCK_SAMPLES) << 27);
	feedback_accumulator = usb_audio_samples_consumed / frames_counted;
	usb_audio_samples_consumed -= uint64_t(uint64_t(feedback_accumulator) * uint32_t(to_remove));

	// adjust speed if close to low or high water mark
	if (local_buf_cnt < (1*(USB_AUDIO_INPUT_BUFFERS+1)*AUDIO_BLOCK_SAMPLES)/4) feedback_accumulator += 16772;
	if (local_buf_cnt > (3*(USB_AUDIO_INPUT_BUFFERS+1)*AUDIO_BLOCK_SAMPLES)/4) feedback_accumulator -= 16772;
	// Check for out of range rates
	if ((feedback_accumulator > USB_AUDIO_FEEDBACK_MAX) || (feedback_accumulator < USB_AUDIO_FEEDBACK_MIN)) {
		rate_errors++;
	}

	// Reset if sufficiently out of range
	if (rate_errors > 500) {
		__disable_irq();
		feedback_accumulator = USB_AUDIO_FEEDBACK_INIT;
		usb_audio_frames_counted = 0;
		usb_audio_samples_consumed = 0;
		__enable_irq();
		rate_errors = 0;
	}

	next_read_index = read_index+1;
	if (next_read_index >= USB_AUDIO_INPUT_BUFFERS) {
		next_read_index = 0;
	}

	if (!left || !right) {
		usb_audio_underrun_count++;
		// Don't adjust samples here as when there is no audio for host there will be underruns
	} 
	//else if (next_read_index == local_write_index) {
	//	// Next buffers are being filled, check incoming count
	//	if (local_write_count <= USB_AUDIO_GUARD_RAIL) {
	//		usb_audio_near_underrun_count++;
	//		// Local clock is running faster so speed up relation
	//		usb_audio_samples_consumed += uint64_t(usb_audio_samples_consumed >> 25);
	//	}
	//}

	if (left) {
		read_index = next_read_index;
		transmit(left, 0);
		release(left);
	}
	if (right) {
		transmit(right, 1);
		release(right);
	}
}


#elif defined(USB_AUDIO_FEEDBACK_DL1YCF)

static const uint16_t USB_AUDIO_INPUT_BUFFERS=4;

//
// "Damped oscillator" feedback algorithm
//
// TARGET_BUFFER_FILLING:  try to tie the buffer filling to that value
// DAMPING:                encodes the amount of damping in the damped oscillator.
//                         the two constants 2508 and 2400 amount to zeta=0.5
//


#ifdef USB_AUDIO_48KHZ
static const uint32_t USB_AUDIO_FEEDBACK_INIT = 805306368; // 48 * 2^24
static const uint32_t USB_AUDIO_FEEDBACK_MAX  = 805641912; // 48.020 * 2^24
static const uint32_t USB_AUDIO_FEEDBACK_MIN  = 804970824; // 47.980 * 2^24
static const int16_t  TARGET_BUFFER_FILLING = ((USB_AUDIO_INPUT_BUFFERS+1)*AUDIO_BLOCK_SAMPLES)/2 + 24;
static const int16_t  DAMPING=2508;
#else
static const uint32_t USB_AUDIO_FEEDBACK_INIT = 739875226; // 44.1 * 2^24
static const uint32_t USB_AUDIO_FEEDBACK_MAX  = 740210769; // 44.120 * 2^24
static const uint32_t USB_AUDIO_FEEDBACK_MIN  = 739539681; // 44.080 * 2^24
static const int16_t  TARGET_BUFFER_FILLING=((USB_AUDIO_INPUT_BUFFERS+1)*AUDIO_BLOCK_SAMPLES)/2 + 22;
static const int16_t  DAMPING=2400
#endif

static volatile int16_t   speed_counter = 0; // used to monitor the current speed difference
static volatile uint32_t  time_last_rx = 0;  // last RX event in microsecond resolution
static volatile uint8_t   input_active=0;    // flag to detect "audio interface down"

// First two seconds after power-up, do not do any feedback correction
// We count the RX callbacks, so this count-down begins when the PC starts
// to send audio samples

static uint16_t powerup_counter = 2000;

// Static in this context (outside of a function) means the variable scope is this file only
static audio_block_t *ready_left_queue[USB_AUDIO_INPUT_BUFFERS];
static audio_block_t *ready_right_queue[USB_AUDIO_INPUT_BUFFERS];
static volatile uint16_t write_index = 0;
static volatile uint16_t read_index = 0;
static volatile uint16_t write_count = 0;

void usb_audio_configure(void)
{
		feedback_accumulator = USB_AUDIO_FEEDBACK_INIT;

		for (uint16_t i = 0; i<USB_AUDIO_INPUT_BUFFERS; i++) {
				ready_right_queue[i] = NULL;
				ready_left_queue[i] = NULL;
		}

		if (usb_high_speed) {
				usb_audio_sync_nbytes = 4;
				usb_audio_sync_rshift = 8;
		} else {
				usb_audio_sync_nbytes = 3;
				usb_audio_sync_rshift = 10;
		}
		memset(&rx_transfer, 0, sizeof(rx_transfer));
		usb_config_rx_iso(AUDIO_RX_ENDPOINT, AUDIO_RX_SIZE, 1, rx_event);
		rx_event(NULL);
		memset(&sync_transfer, 0, sizeof(sync_transfer));
		usb_config_tx_iso(AUDIO_SYNC_ENDPOINT, usb_audio_sync_nbytes, 1, sync_event);
		sync_event(NULL);
		memset(&tx_transfer, 0, sizeof(tx_transfer));
		usb_config_tx_iso(AUDIO_TX_ENDPOINT, AUDIO_TX_SIZE, 1, tx_event);
		tx_event(NULL);
		usb_start_sof_interrupts(AUDIO_INTERFACE);
}

void AudioInputUSB::begin(void)
{
		// update_responsibility = update_setup();
		// TODO: update responsibility is tough, partly because the USB
		// interrupts aren't sychronous to the audio library block size,
		// but also because the PC may stop transmitting data, which
		// means we no longer get receive callbacks from usb.c
		update_responsibility = false;
}

// Called from the USB interrupt when an isochronous packet arrives
// we must completely remove it from the receive buffer before returning
//
void usb_audio_receive_callback(unsigned int len)
{
	static audio_block_t *left = NULL;
	static audio_block_t *right = NULL;
	const uint32_t *data;
	uint16_t avail;

	len >>= 2; // 1 sample = 4 bytes: 2 left, 2 right
	data = (const uint32_t *)rx_buffer;

		speed_counter += len;      // DL1YCF: counts *all* incoming samples
		time_last_rx = micros();   //         time of last RX callback
		input_active=3;            //         mark interface "up and running"
		if (powerup_counter > 0) powerup_counter--;


	if (left == NULL) {
		left = AudioStream::allocate();
				write_count=0;
		if (left == NULL) return;
	}
	if (right == NULL) {
		right = AudioStream::allocate();
				write_count=0;
		if (right == NULL) return;
	}

	while (len > 0) {

		avail = AUDIO_BLOCK_SAMPLES - write_count;
		if (len < avail) {
						//
						// less samples to store than left in the "incoming" buffer.
						// so copy them and return
						//
			copy_to_buffers(data, left->data + write_count, right->data + write_count, len);
			buffer_counter += len;
			write_count = write_count + len;
			return;
		} else {
					if (avail > 0) {
						//
						// There is still space in the incoming buffer, so make it full
						// and transfer it to the "ready" queue (if space available)
						//
			copy_to_buffers(data, left->data + write_count, right->data + write_count, avail);
			buffer_counter += avail;
			write_count += avail;
			data += avail;
			len -= avail;
					}
					//
					// At this point, the incoming buffer is full
					//
					if (ready_left_queue[write_index] || ready_right_queue[write_index]) {
						//
						// No space in "ready" queue
						// (PC sending too fast). Return and try to send the buffer
						// in the next RX callback.
						//
			if (len > 0) {
			  usb_audio_overrun_count++;
			}
			return;
					}
					//
					// Transfer the (full) incoming buffer to the ready queue
					// and cycle
					//
			ready_left_queue[write_index] = left;
			ready_right_queue[write_index] = right;
					write_index++;
			if (write_index >= USB_AUDIO_INPUT_BUFFERS) write_index = 0;
					//
					// allocate new "incoming" buffer and set count to zero
					//
			write_count = 0;
			left = AudioStream::allocate();
			if (left == NULL) {
			return;
			}
			right = AudioStream::allocate();
			if (right == NULL) {
			AudioStream::release(left);
			return;
			}
		}
				// continue with "while" until all samples stored or overrun detected
	} // while
}

void AudioInputUSB::update(void)
{
	audio_block_t *left, *right;

	__disable_irq();

	left = ready_left_queue[read_index];
	ready_left_queue[read_index] = NULL;
	right = ready_right_queue[read_index];
	ready_right_queue[read_index] = NULL;
		if (left) buffer_counter -= AUDIO_BLOCK_SAMPLES;
		uint16_t local_buf_cnt = buffer_counter;

		if (input_active > 0) input_active--;
		//
		// local_speed: "current speed difference", the number of samples received
		// in the receive callback since the last update(),
		// minus the size of samples passed downstream this each update()
		//
		int16_t   local_speed = speed_counter - AUDIO_BLOCK_SAMPLES;
		uint32_t  now   = micros();
		int16_t   yet_unsent;

		if (now > time_last_rx && input_active > 0) {

#ifdef USB_AUDIO_48KHZ
		  yet_unsent = (49*(now-time_last_rx)) >> 10;  // 48k  = 20.833 usec per sample
#else
		  yet_unsent = (45*(now-time_last_rx)) >> 10;  // 44k1 = 22.676 usec per sample
#endif
		  //
		  // yet_unsent is now the (approximate) number of samples in the
		  // "input queue" but not yet sent. This number is close to
		  // zero if the last receive callback took place just few
		  // microseconds ago and approaches the number of samples
		  // transmitted in each RX callback if it took place
		  // immedeately after the preceeding update().
		  //
		  // The number need not be overly accurate and leads to a
		  // "flattening" of local_speed, which now  mostly assumes one of the
		  // values {-1, 0, 1} instead of oscillating wildly.
		  // Note its time average is left unchanged!
		  // time average of d is not changed!
		  //
		  speed_counter   = -yet_unsent;
		} else {
		  //
		  // now < time_last_rx :  32-bit timer overrun, happens every 70 hours
		  // input_active == 0     :  audio interface closed
		  //
		  // so switch off and restart the feedback correction
		  //
		  speed_counter = 0;
		  yet_unsent=0;     // will be out-of-bounds in case of 32-bit overrun
		}

	__enable_irq();

		//
		// This can be done after enabling interrupts
		//
		local_buf_cnt  +=  yet_unsent;
		local_speed    +=  yet_unsent;
#if 0
	//
	// This prints out the actual value of the feedback
	// correction (converted to Hz) and the min and max
	// buffer filling few times per second on
	// the console. This can be used to debug the
	// feedback control loop
	//
	static uint16_t debug=0;

		static uint16_t min_buf=9999;
		static uint16_t max_buf=0;
	 
		if (input_active == 0) {
		  // print "sanitized" values. These have no effect further down 
		  // if input_active is FALSE
		  local_speed = 0;
		  local_buf_cnt = 999;
		}
		if (local_buf_cnt > max_buf) max_buf=local_buf_cnt;
		if (local_buf_cnt < min_buf) min_buf=local_buf_cnt;

		static int16_t  min_speed=9999;
		static int16_t  max_speed=-9999;
		if (local_speed   > max_speed) max_speed=local_speed;
		if (local_speed   < min_speed) min_speed=local_speed;

	if (++debug > 1500) {
		debug=0;
		Serial.print("Damp: Corr= ");
		Serial.print(feedback_accumulator*0.00005960464477539063);
		Serial.print(" MinBuf=");
		Serial.print(min_buf);
		Serial.print(" MaxBuf=");
		Serial.print(max_buf);
		Serial.print(" MinSpd=");
		Serial.print(min_speed);
		Serial.print(" MaxSpd=");
		Serial.print(max_speed);
		Serial.print(" O");
		Serial.print(usb_audio_overrun_count);
		Serial.print(" U");
		Serial.println(usb_audio_underrun_count);
		min_buf=9999;
		max_buf=0;
		min_speed=9999;
		max_speed=-9999;

	}
#endif

		if (input_active > 0 && powerup_counter == 0) {
		  //
		  // DL1YCF: new feedback "damped oscillator" correction
		  //         we keep it within Steve's safety margins but to not reset
		  //
		  feedback_accumulator += TARGET_BUFFER_FILLING - (int) local_buf_cnt - (DAMPING * local_speed);
		  if (feedback_accumulator > USB_AUDIO_FEEDBACK_MAX) feedback_accumulator=USB_AUDIO_FEEDBACK_MAX;
		  if (feedback_accumulator < USB_AUDIO_FEEDBACK_MIN) feedback_accumulator=USB_AUDIO_FEEDBACK_MIN;
		}

	if (!left || !right) {
		usb_audio_underrun_count++;
		} else {
		  read_index +=1;
		  if (read_index >= USB_AUDIO_INPUT_BUFFERS) {
			read_index=0;
		  }
		}

	if (left) {
		transmit(left, 0);
		release(left);
	}
	if (right) {
		transmit(right, 1);
		release(right);
	}
}
#else
//
// Original feedback algorithm
//
audio_block_t * AudioInputUSB::incoming_left;
audio_block_t * AudioInputUSB::incoming_right;
audio_block_t * AudioInputUSB::ready_left;
audio_block_t * AudioInputUSB::ready_right;
uint16_t AudioInputUSB::incoming_count;
uint8_t AudioInputUSB::receive_flag;

void usb_audio_configure(void)
{
	printf("usb_audio_configure\n");
	usb_audio_underrun_count = 0;
	usb_audio_overrun_count = 0;

#ifdef USB_AUDIO_48KHZ
	feedback_accumulator = 805306368; // 48 * 2^24
#else
	feedback_accumulator = 739875226; // 44.1 * 2^24
#endif

	if (usb_high_speed) {
		usb_audio_sync_nbytes = 4;
		usb_audio_sync_rshift = 8;
	} else {
		usb_audio_sync_nbytes = 3;
		usb_audio_sync_rshift = 10;
	}
	memset(&rx_transfer, 0, sizeof(rx_transfer));
	usb_config_rx_iso(AUDIO_RX_ENDPOINT, AUDIO_RX_SIZE, 1, rx_event);
	rx_event(NULL);
	memset(&sync_transfer, 0, sizeof(sync_transfer));
	usb_config_tx_iso(AUDIO_SYNC_ENDPOINT, usb_audio_sync_nbytes, 1, sync_event);
	sync_event(NULL);
	memset(&tx_transfer, 0, sizeof(tx_transfer));
	usb_config_tx_iso(AUDIO_TX_ENDPOINT, AUDIO_TX_SIZE, 1, tx_event);
	tx_event(NULL);
	usb_start_sof_interrupts(AUDIO_INTERFACE);
}

void AudioInputUSB::begin(void)
{
	incoming_count = 0;
	incoming_left = NULL;
	incoming_right = NULL;
	ready_left = NULL;
	ready_right = NULL;
	receive_flag = 0;
	// update_responsibility = update_setup();
	// TODO: update responsibility is tough, partly because the USB
	// interrupts aren't sychronous to the audio library block size,
	// but also because the PC may stop transmitting data, which
	// means we no longer get receive callbacks from usb.c
	update_responsibility = false;
}


// Called from the USB interrupt when an isochronous packet arrives
// we must completely remove it from the receive buffer before returning
//
void usb_audio_receive_callback(unsigned int len)
{
	unsigned int count, avail;
	audio_block_t *left, *right;
	const uint32_t *data;

	AudioInputUSB::receive_flag = 1;
	len >>= 2; // 1 sample = 4 bytes: 2 left, 2 right
	data = (const uint32_t *)rx_buffer;

	count = AudioInputUSB::incoming_count;
	left = AudioInputUSB::incoming_left;
	right = AudioInputUSB::incoming_right;
	if (left == NULL) {
		left = AudioStream::allocate();
		if (left == NULL) return;
		AudioInputUSB::incoming_left = left;
	}
	if (right == NULL) {
		right = AudioStream::allocate();
		if (right == NULL) return;
		AudioInputUSB::incoming_right = right;
	}
	while (len > 0) {
		avail = AUDIO_BLOCK_SAMPLES - count;
		if (len < avail) {
			copy_to_buffers(data, left->data + count, right->data + count, len);
			AudioInputUSB::incoming_count = count + len;
			return;
		} else if (avail > 0) {
			copy_to_buffers(data, left->data + count, right->data + count, avail);
			data += avail;
			len -= avail;
			if (AudioInputUSB::ready_left || AudioInputUSB::ready_right) {
				// buffer overrun, PC sending too fast
				AudioInputUSB::incoming_count = count + avail;
				if (len > 0) {
					usb_audio_overrun_count++;
					//Serial.print("U");
					//Serial.println(len);
					//printf("!");
					//serial_phex(len);
				}
				return;
			}
			send:
			AudioInputUSB::ready_left = left;
			AudioInputUSB::ready_right = right;
			//if (AudioInputUSB::update_responsibility) AudioStream::update_all();
			left = AudioStream::allocate();
			if (left == NULL) {
				AudioInputUSB::incoming_left = NULL;
				AudioInputUSB::incoming_right = NULL;
				AudioInputUSB::incoming_count = 0;
				return;
			}
			right = AudioStream::allocate();
			if (right == NULL) {
				AudioStream::release(left);
				AudioInputUSB::incoming_left = NULL;
				AudioInputUSB::incoming_right = NULL;
				AudioInputUSB::incoming_count = 0;
				return;
			}
			AudioInputUSB::incoming_left = left;
			AudioInputUSB::incoming_right = right;
			count = 0;
		} else {
			if (AudioInputUSB::ready_left || AudioInputUSB::ready_right) return;
			goto send; // recover from buffer overrun
		}
	}
	AudioInputUSB::incoming_count = count;
}

void AudioInputUSB::update(void)
{
	audio_block_t *left, *right;

	__disable_irq();
	left = ready_left;
	ready_left = NULL;
	right = ready_right;
	ready_right = NULL;
	uint16_t c = incoming_count;
	uint8_t f = receive_flag;

	receive_flag = 0;
	__enable_irq();


#if 0
	//
	// This prints out the actual value of the feedback
	// correction (converted to Hz) and the min and max
	// value of incoming_count few times per second on
	// the console. This can be used to debug the
	// feedback control loop
	//
	static uint16_t max_buf=0;
	static uint16_t min_buf=9999;
	static uint16_t debug=0;

	if (c > max_buf) max_buf=c;
	if (c < min_buf) min_buf=c;

	if (++debug > 1500) {
		debug=0;
		Serial.print("ORIG: Corr= ");
		Serial.print(feedback_accumulator*0.00005960464477539063);
		Serial.print(" Min= ");
		Serial.print(min_buf); 
		Serial.print(" Max= ");
		Serial.print(max_buf);
		Serial.print(" O");
		Serial.print(usb_audio_overrun_count);
		Serial.print(" U");
		Serial.print(usb_audio_underrun_count);
		Serial.println(";");
		min_buf=9999;
		max_buf=0;
	}
#endif

	if (f) {
		int diff = AUDIO_BLOCK_SAMPLES/2 - (int)c;
		feedback_accumulator += diff * 1;
		//uint32_t feedback = (feedback_accumulator >> 8) + diff * 100;
		//usb_audio_sync_feedback = feedback;

		//printf(diff >= 0 ? "." : "^");
	}
	//serial_phex(c);
	//serial_print(".");

	if (!left || !right) {
		usb_audio_underrun_count++;
		//Serial.print("O");
		//Serial.println(incoming_count);
		//printf("#"); // buffer underrun - PC sending too slow
		// USB_AUDIO_48KHZ For some reason there are many underruns during the 1 second of powerup
		// It seems as if the PC is either sending too short packets, or too infrequent packets at first
		// The line below causes the feedback_accumulator to be way off
		if (f) feedback_accumulator += 3500;
	}

	if (left) {
		transmit(left, 0);
		release(left);
	}
	if (right) {
		transmit(right, 1);
		release(right);
	}
}



#endif // feedback type








/*DMAMEM*/ uint16_t usb_audio_transmit_buffer[AUDIO_TX_SIZE/2] __attribute__ ((used, aligned(32)));


static void tx_event(transfer_t *t)
{
	int len = usb_audio_transmit_callback();
	usb_audio_sync_feedback = feedback_accumulator >> usb_audio_sync_rshift;
	usb_prepare_transfer(&tx_transfer, usb_audio_transmit_buffer, len, 0);
	arm_dcache_flush_delete(usb_audio_transmit_buffer, len);
	usb_transmit(AUDIO_TX_ENDPOINT, &tx_transfer);
}

static void copy_from_buffers(uint32_t *dst, int16_t *left, int16_t *right, unsigned int len)
{
	// TODO: optimize...
	while (len > 0) {
		*dst++ = (*right++ << 16) | (*left++ & 0xFFFF);
		len--;
	}
}

bool AudioOutputUSB::update_responsibility;


#if defined(USB_AUDIO_FEEDBACK_SOF)

volatile uint32_t usb_audio_out_underrun_count = 0;
volatile uint32_t usb_audio_out_overrun_count = 0;


// Buffers
static const uint16_t USB_AUDIO_OUTPUT_BUFFERS=4;


// Static in this context (outside of a function) means the variable scope is this file only
static audio_block_t *out_ready_left[USB_AUDIO_OUTPUT_BUFFERS];
static audio_block_t *out_ready_right[USB_AUDIO_OUTPUT_BUFFERS];
static volatile uint16_t out_write_index = 0;
static volatile uint16_t out_read_index = 0;
static volatile uint16_t out_read_count = 0;

volatile int16_t out_buffer_counter = 0;  // used to monitor the current buffer filling

void AudioOutputUSB::begin(void)
{
	update_responsibility = false;
}

void AudioOutputUSB::update(void)
{
	audio_block_t *left, *right;
	uint16_t i;

	// TODO: we shouldn't be writing to these......
	//left = receiveReadOnly(0); // input 0 = left channel
	//right = receiveReadOnly(1); // input 1 = right channel
	left = receiveWritable(0); // input 0 = left channel
	right = receiveWritable(1); // input 1 = right channel
	if (usb_audio_transmit_setting == 0) {
		if (left) release(left);
		if (right) release(right);
		//for (i = 0; i<USB_AUDIO_OUTPUT_BUFFERS; i++) {
		//	left = out_ready_left[i];
		//	if (left) { release(left); out_ready_left[i] = NULL; }
		//	right = out_ready_right[i];
		//	if (right) { release(right); out_ready_right[i] = NULL; }
		//}
		//out_buffer_counter = 0;
		return;
	}
	if (left == NULL) {
		left = allocate();
		if (left == NULL) {
			if (right) release(right);
			return;
		}
		memset(left->data, 0, sizeof(left->data));
	}
	if (right == NULL) {
		right = allocate();
		if (right == NULL) {
			release(left);
			return;
		}
		memset(right->data, 0, sizeof(right->data));
	}
	__disable_irq();
	if (out_ready_left[out_write_index] || out_ready_right[out_write_index]) {
		// Buffer overrun - PC is consuming too slowly
		release(left);
		release(right);
		usb_audio_out_overrun_count++;
	} else {
		// Store in ring buffer
		out_ready_left[out_write_index] = left;
		out_ready_right[out_write_index] = right;

		out_buffer_counter += AUDIO_BLOCK_SAMPLES;
		out_write_index++;
		if (out_write_index >= USB_AUDIO_OUTPUT_BUFFERS) out_write_index = 0;
	}
	__enable_irq();
}


// Called from the USB interrupt when ready to transmit another
// isochronous packet.  If we place data into the transmit buffer,
// the return is the number of bytes.  Otherwise, return 0 means
// no data to transmit
unsigned int usb_audio_transmit_callback(void)
{

	uint32_t avail, num, target, offset, len=0;
	audio_block_t *left, *right;

#ifdef USB_AUDIO_48KHZ

	// 1.5X48 samples buffered then send slower to build up samples
	if (out_buffer_counter < 72 ) target = 47;
	// BUFFERSPACE-(0.5*48) then send faster to clear buffer space
	else if (out_buffer_counter > ((AUDIO_BLOCK_SAMPLES*USB_AUDIO_OUTPUT_BUFFERS)-24)) target = 49;
	else target = 48;

#else
	static uint32_t count=5;
	if (++count < 10) {
		// 1.5X44 samples buffered then send slower to build up samples
		if (out_buffer_counter < 66 ) target = 43;
		// BUFFERSPACE-(0.5*44) then send faster to clear buffer space
		else if (out_buffer_counter > ((AUDIO_BLOCK_SAMPLES*USB_AUDIO_OUTPUT_BUFFERS)-22)) target = 45;
		else target = 44;
	} else {
		count = 0;
		target = 45;
	}
#endif


#if 0
	static uint16_t debug=0;
	static uint16_t min_buf=9999;
	static uint16_t max_buf=0;
	if (out_buffer_counter > max_buf) max_buf=out_buffer_counter;
	if (out_buffer_counter < min_buf) min_buf=out_buffer_counter;

	if (++debug > 1500) {
		debug=0;
		Serial.print("SOF MinOutBuf=");
		Serial.print(min_buf);
		Serial.print(" MaxOutBuf=");
		Serial.print(max_buf);
		Serial.print(" O");
		Serial.print(usb_audio_out_overrun_count);
		//Serial.print(" NO");
		//Serial.print(usb_audio_near_overrun_count);
		Serial.print(" U");
		Serial.println(usb_audio_out_underrun_count);
		//Serial.print(" NU");
		//Serial.println(usb_audio_near_underrun_count);
		min_buf=9999;
		max_buf=0;
	}
#endif

	while (len < target) {

		num = target - len;

		left = out_ready_left[out_read_index];
		right = out_ready_right[out_read_index];

		if (left == NULL || right == NULL) {
			// buffer underrun - PC is consuming too quickly
			memset(usb_audio_transmit_buffer + len, 0, num * 4);
			out_read_count = 0;
			usb_audio_out_underrun_count++;
			break;
		}

		avail = AUDIO_BLOCK_SAMPLES - out_read_count;
		if (num > avail) num = avail;

		copy_from_buffers((uint32_t *)usb_audio_transmit_buffer + len,
			left->data + out_read_count, right->data + out_read_count, num);
		len += num;
		out_read_count += num;
		out_buffer_counter -= num;
		if (out_read_count >= AUDIO_BLOCK_SAMPLES) {
			out_ready_left[out_read_index] = NULL;
			out_ready_right[out_read_index] = NULL;
			AudioStream::release(left);
			AudioStream::release(right);
			out_read_count = 0;
			out_read_index++;
			if (out_read_index >= USB_AUDIO_OUTPUT_BUFFERS) out_read_index = 0;
		}
	}

	return target * 4;
}





#else

audio_block_t * AudioOutputUSB::left_1st;
audio_block_t * AudioOutputUSB::left_2nd;
audio_block_t * AudioOutputUSB::right_1st;
audio_block_t * AudioOutputUSB::right_2nd;
uint16_t AudioOutputUSB::offset_1st;

void AudioOutputUSB::begin(void)
{
	update_responsibility = false;
	left_1st = NULL;
	right_1st = NULL;
}

void AudioOutputUSB::update(void)
{
	audio_block_t *left, *right;

	// TODO: we shouldn't be writing to these......
	//left = receiveReadOnly(0); // input 0 = left channel
	//right = receiveReadOnly(1); // input 1 = right channel
	left = receiveWritable(0); // input 0 = left channel
	right = receiveWritable(1); // input 1 = right channel
	if (usb_audio_transmit_setting == 0) {
		if (left) release(left);
		if (right) release(right);
		if (left_1st) { release(left_1st); left_1st = NULL; }
		if (left_2nd) { release(left_2nd); left_2nd = NULL; }
		if (right_1st) { release(right_1st); right_1st = NULL; }
		if (right_2nd) { release(right_2nd); right_2nd = NULL; }
		offset_1st = 0;
		return;
	}
	if (left == NULL) {
		left = allocate();
		if (left == NULL) {
			if (right) release(right);
			return;
		}
		memset(left->data, 0, sizeof(left->data));
	}
	if (right == NULL) {
		right = allocate();
		if (right == NULL) {
			release(left);
			return;
		}
		memset(right->data, 0, sizeof(right->data));
	}
	__disable_irq();
	if (left_1st == NULL) {
		left_1st = left;
		right_1st = right;
		offset_1st = 0;
	} else if (left_2nd == NULL) {
		left_2nd = left;
		right_2nd = right;
	} else {
		// buffer overrun - PC is consuming too slowly
		audio_block_t *discard1 = left_1st;
		left_1st = left_2nd;
		left_2nd = left;
		audio_block_t *discard2 = right_1st;
		right_1st = right_2nd;
		right_2nd = right;
		offset_1st = 0; // TODO: discard part of this data?
		//serial_print("*");
		release(discard1);
		release(discard2);
	}
	__enable_irq();
}


// Called from the USB interrupt when ready to transmit another
// isochronous packet.  If we place data into the transmit buffer,
// the return is the number of bytes.  Otherwise, return 0 means
// no data to transmit
unsigned int usb_audio_transmit_callback(void)
{

	uint32_t avail, num, target, offset, len=0;
	audio_block_t *left, *right;

#ifdef USB_AUDIO_48KHZ
	target = 48;
#else
	static uint32_t count=5;
	if (++count < 10) {   // TODO: dynamic adjust to match USB rate
		target = 44;
	} else {
		count = 0;
		target = 45;
	}
#endif

	while (len < target) {
		num = target - len;
		left = AudioOutputUSB::left_1st;
		if (left == NULL) {
			// buffer underrun - PC is consuming too quickly
			memset(usb_audio_transmit_buffer + len, 0, num * 4);
			//serial_print("%");
			break;
		}
		right = AudioOutputUSB::right_1st;
		offset = AudioOutputUSB::offset_1st;

		avail = AUDIO_BLOCK_SAMPLES - offset;
		if (num > avail) num = avail;

		copy_from_buffers((uint32_t *)usb_audio_transmit_buffer + len,
			left->data + offset, right->data + offset, num);
		len += num;
		offset += num;
		if (offset >= AUDIO_BLOCK_SAMPLES) {
			AudioStream::release(left);
			AudioStream::release(right);
			AudioOutputUSB::left_1st = AudioOutputUSB::left_2nd;
			AudioOutputUSB::left_2nd = NULL;
			AudioOutputUSB::right_1st = AudioOutputUSB::right_2nd;
			AudioOutputUSB::right_2nd = NULL;
			AudioOutputUSB::offset_1st = 0;
		} else {
			AudioOutputUSB::offset_1st = offset;
		}
	}
	return target * 4;
}

#endif




struct setup_struct {
  union {
	struct {
	uint8_t bmRequestType;
	uint8_t bRequest;
	union {
		struct {
			uint8_t bChannel;  // 0=main, 1=left, 2=right
			uint8_t bCS;       // Control Selector
		};
		uint16_t wValue;
	};
	union {
		struct {
			uint8_t bIfEp;     // type of entity
			uint8_t bEntityId; // UnitID, TerminalID, etc.
		};
		uint16_t wIndex;
	};
	uint16_t wLength;
	};
  };
};

int usb_audio_get_feature(void *stp, uint8_t *data, uint32_t *datalen)
{
	struct setup_struct setup = *((struct setup_struct *)stp);
	if (setup.bmRequestType==0xA1) { // should check bRequest, bChannel, and UnitID
			if (setup.bCS==0x01) { // mute
				data[0] = AudioInputUSB::features.mute;  // 1=mute, 0=unmute
				*datalen = 1;
				return 1;
			}
			else if (setup.bCS==0x02) { // volume
				if (setup.bRequest==0x81) { // GET_CURR
					data[0] = AudioInputUSB::features.volume & 0xFF;
					data[1] = (AudioInputUSB::features.volume>>8) & 0xFF;
				}
				else if (setup.bRequest==0x82) { // GET_MIN
					//serial_print("vol get_min\n");
					data[0] = 0;     // min level is 0
					data[1] = 0;
				}
				else if (setup.bRequest==0x83) { // GET_MAX
					data[0] = FEATURE_MAX_VOLUME;  // max level, for range of 0 to MAX
					data[1] = 0;
				}
				else if (setup.bRequest==0x84) { // GET_RES
					data[0] = 1; // increment vol by by 1
					data[1] = 0;
				}
				else { // pass over SET_MEM, etc.
					return 0;
				}
				*datalen = 2;
				return 1;
			}
	}
	return 0;
}

int usb_audio_set_feature(void *stp, uint8_t *buf) 
{
	struct setup_struct setup = *((struct setup_struct *)stp);
	if (setup.bmRequestType==0x21) { // should check bRequest, bChannel and UnitID
			if (setup.bCS==0x01) { // mute
				if (setup.bRequest==0x01) { // SET_CUR
					AudioInputUSB::features.mute = buf[0]; // 1=mute,0=unmute
					AudioInputUSB::features.change = 1;
					return 1;
				}
			}
			else if (setup.bCS==0x02) { // volume
				if (setup.bRequest==0x01) { // SET_CUR
					AudioInputUSB::features.volume = buf[0];
					AudioInputUSB::features.change = 1;
					return 1;
				}
			}
	}
	return 0;
}



#endif // AUDIO_INTERFACE
