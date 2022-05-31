/* Teensy support for wm8960 audio codec
 * Copyright (c) 2021, Steve Haynal, steve@softerhardware.com
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
#include "control_wm8960.h"
#include <utility/imxrt_hw.h>

// See https://blog.titanwolf.in/a?ID=00500-80a77412-7973-49b3-b2e9-bc1beb847257


#define WM8960_I2C_ADDR 0x1A

// mask set bits are the bits in val which should be written
bool AudioControlWM8960::write(uint16_t reg, uint16_t val, uint16_t mask, bool force=false)
{

    uint16_t newval;

    newval = (regmap[reg] & ~mask) | (val & mask);

    if ((newval != regmap[reg]) || force) {
        regmap[reg] = newval;

        Wire1.beginTransmission(WM8960_I2C_ADDR);

        Wire1.write((reg << 1) | ((newval >> 8) & 1));
        Wire1.write(newval & 0xFF);

        if (Wire1.endTransmission() == 0) return true;
        return false;
    }
    return true;
}



bool AudioControlWM8960::disable(void)
{
    // Reset
    return write(0x0f, 0, 0b01, true);
}


bool AudioControlWM8960::enable(void)
{
    //Wire.begin();
    delay(750);
    // Reset
    if (!write(0x0f, 0, 0b01, true)) {
        Serial.println("WM8960 Reset Failed");
        //return false; // no WM8960 chip responding
    }
    else
        Serial.println("WM8960 Reset Completed");

    delay(5);

    // Enable VMID and VREF
    write(0x19, 0b111000000, 0b111000000);

    // Enable DAC, Headphones and Speakers
    write(0x1a, 0b111111000, 0b111111000);
    
    // Enable mixer
    write(0x2f, 0b000001100, 0b000001100);

    //#define USE_MCLK
    //#define PLL_CLOCKING
    #define PLL_CLOCKING1   
    // Setup clocking

    #ifdef PLL_CLOCKING1
    //#elif defined PLL_CLOCKING1  
    int32_t freq = 48000;
    write(0x04, 0b000000001, 0b111111111); 
    write(0x1a, 0b000000001, 0b000000001);  // Power up PLL
      // PLL between 27*24 = 648MHz und 54*24=1296MHz
    int n1 = 4; //SAI prescaler 4 => (n1*n2) = multiple of 4
    int n2 = 1 + (24000000 * 27) / (freq * 256 * n1);
    double C = ((double)freq * 256 * n1 * n2) / 24000000;
    int c0 = C;
    int c2 = 10000;
    int c1 = C * c2 - (c0 * c2);
    set_audioClock(c0, c1, c2, true);
    CCM_CS1CDR = (CCM_CS1CDR & ~(CCM_CS1CDR_SAI1_CLK_PRED_MASK | CCM_CS1CDR_SAI1_CLK_PODF_MASK))
        | CCM_CS1CDR_SAI1_CLK_PRED(n1-1) // &0x07
        | CCM_CS1CDR_SAI1_CLK_PODF(n2-1); // &0x3f 
    Serial.printf("SetI2SFreq(%d)\n",freq);
    #endif

    #ifdef PLL_CLOCKING
    // Configure PLL clock
    // MCLK->PLL->SYSCLK->DAC/ADC sample Freq = 24MHz/544.217 = 44.100kHz
    // See calculation pages 60 and table 44 in manual
    // f2 = 4 x 2 x 11.2896Hz = 90.3168MHz
    // R = 90.316 / 12 = 7.5264
    // PLLN = int R = 7
    // k = int ( 2^24 x (7.5264 – 7)) = 8831526
    // N = 7
    // Fractional 24bit value = 86C226h

    // FOR 48KHz:
    // MCLK->PLL->SYSCLK->DAC/ADC sample Freq = 24MHz/500 = 48.0kHz
    // See calculation pages 60 and table 44 in manual
    // f2 = 4 x 2 x 12.288MHz = 98.304MHz
    // R = 98.304 / 12 = 8.192
    // PLLN = int R = 8
    // k = int ( 2^24 x (8.192 – 8)) = 
    // N = 8
    // Fractional 24bit value = 3126E8h
    int delay1 = 1000;
    int delay2 = 500;
    uint8_t res;

    write(0x1a, 0b000000001, 0b000000001);  // Power up PLL
    delayMicroseconds(delay2);

    res = write(0x04, 0x0001, 0b111111111); // Select PLL
    if (res == 0) Serial.println("WM8960 Configure clock"); 
    else
    {
        Serial.println("WM8960 Configure clock failed");                
        return res;
    }
    delayMicroseconds(delay1);

    // Power on PLL
    res = write(0x34, 0x0028, 0b111111111); // Select PLL 1

    // Configure PLL 1  0011 0111 = 37h
    //                 00010 0111   27h
    res = write(0x34, 0x0028, 0b000101111); // Select PLL 1
    delayMicroseconds(delay1);

    // Configure PLL 2  bit 8 reserved 7-0 data
    //res = write(0x35, 0x0086, 0b010000110); // Select PLL 2
    res = write(0x35, 0x0031, 0b000111111); // Select PLL 2
    delayMicroseconds(delay1);

    // Configure PLL 3   
    //res = write(0x36, 0x00C2, 0b011000010); // Select PLL 3
    res = write(0x36, 0x0026, 0b111111111); // Select PLL 3
    delayMicroseconds(delay1);

    // Configure PLL 4   
    //res = write(0x37, 0x0026, 0b000100110); // Select PLL 4
    res = write(0x37, 0x00E8, 0b111111111); // Select PLL 4
    delayMicroseconds(delay1);
    #endif
    
    #ifdef USE_MCLK
    write(0x04, 0b000000000, 0b111111111); 
    write(0x1a, 0b000000000, 0b000000001);
    #endif
    
    // Unmute DAC
    write(0x05, 0b000000000, 0b000001000);

    // 16-bit data and i2s interface
    write(0x07, 0b000000010, 0b000001111); // I2S, 16 bit, MCLK slave

    // Mute headphone and speakers, but enable zero crossing changes only
    write(0x02, 0b010000000, 0b011111111);
    write(0x03, 0b110000000, 0b111111111);
    // Speakers
    write(0x28, 0b010000000, 0b011111111);
    write(0x29, 0b110000000, 0b111111111);

    // Set DAC Volume to max 0dB, full range of DAC
    write(0x0a, 0b011111111, 0b011111111);
    write(0x0b, 0b111111111, 0b111111111);

    // Connect Left DAC to left output mixer
    write(0x22, 0b100000000, 0b100000000);

    // Connect Right DAC to right output mixer
    write(0x25, 0b100000000, 0b100000000);


    // Enable headphone detect to disable speaker
    // Enable HPSWEN and set HPSWPOL
    write(0x18, 0b001000000, 0b001100000);
    // Use JD2 as jack detect input
    write(0x30, 0b000001000, 0b000001100);
    // Enable slow clock for jack detect
    write(0x17, 0b000000001, 0b000000011);


    // Enable speaker outputs
    write(0x31, 0b001000000, 0b011000000);
    // Speaker amp DC boost
    write(0x33, 0b000000000, 0b000111000);
    // Speaker amp AC boost
    write(0x33, 0b000000000, 0b000000111);

    //// Configure input
    //// Enable AINL/AINR, and ADCL/ADCR
    write(0x19, 0b000111100, 0b000111100);

    // Unmute and set zero crossing input level change
    write(0x00, 0b101000000, 0b111000000);
    write(0x01, 0b101000000, 0b111000000);

    delay(100); // how long to power up?

    return true;
}

bool AudioControlWM8960::volume(float n) {
    headphoneVolume(n,n);
    return speakerVolume(n,n);
}

bool AudioControlWM8960::volume(float l, float r) {
    headphoneVolume(l,r);
    return speakerVolume(l,r);
}

bool AudioControlWM8960::headphoneVolume(float l, float r)  // 0 to 1.0 to produce 47 to 127
{
    uint16_t i;
    i = 47 + (uint16_t) (80.0*l+0.5);
    // Left headphone
    write(0x02, (i & 0b01111111), 0b001111111, true);

    i = 47 + (uint16_t) (80.0*r+0.5);
    // Right headphone
    return write(0x03, 0b100000000 | (i & 0b01111111), 0b101111111, true);
}

bool AudioControlWM8960::headphonePower(uint8_t p)
{
    uint16_t mask;
    uint16_t value;

    value = (p & 0b11) << 5;
    mask = 0b011 << 5;

    // LOUT1 Enable
    return write(0x1a, value, mask);
}

bool AudioControlWM8960::speakerVolume(float l, float r)  // 0 to 1.0 to produce 47 to 127
{
    uint16_t i;
    i = 47 + (uint16_t) (80.0*l+0.5);
    // Left speaker
    write(0x28, (i & 0b01111111), 0b001111111, true);

    i = 47 + (uint16_t) (80.0*r+0.5);
    // Right speaker
    return write(0x29, 0b100000000 | (i & 0b01111111), 0b101111111, true);
}

bool AudioControlWM8960::speakerPower(uint8_t p)
{
    uint16_t mask;
    uint16_t value;

    value = (p & 0b11) << 3;
    mask = 0b011 << 3;

    // SPK_PN Output Enable
    write(0x1a, value, mask);

    // SPK_OP_EN
    // Bits are swapped
    value = (p & 0b01) ? 0b010000000 : 0b0;
    value = (p & 0b10) ? (value | 0b001000000) : value;
    mask = 0b011 << 6;
    return write(0x31, value, mask);
}


// Write 1 to disable, 0 to enable, default is enabled
bool AudioControlWM8960::disableADCHPF(uint8_t v)
{
    return write(0x05, v, 0b01);
}


// Write 1 to enable
bool AudioControlWM8960::enableMicBias(uint8_t v)
{
    return write(0x19,v << 1,0b000000010);
}

// Write 1 to enable
bool AudioControlWM8960::enableALC(uint16_t v)
{
    // FIXME set LINVOL and RINVOL to same value if both are enabled
    return write(0x11, v << 7, 0b110000000);
}


bool AudioControlWM8960::micPower(uint8_t p)
{
    uint16_t mask;
    uint16_t value;


    // Select microphone inputs with +29dB boost
    if (p & 0b010) {
        // enable left
        write(0x20,0b100111000,0b111111000);
    } else {
        // disable left
        write(0x20,0b000000000,0b111111000);
    }

    if (p & 0b001) {
        // enable right (MEMS)
        write(0x21,0b100111000,0b111111000);
    } else {
        // disable right
        write(0x21,0b000000000,0b111111000);
    }

    // enable microphone inputs
    value = p  << 4;
    mask = 0b011 << 4;

    return write(0x2f, value, mask);
}

bool AudioControlWM8960::lineinPower(uint8_t p)
{
    uint16_t mask;
    uint16_t value;


    // Select line2
    if (p & 0b010) {
        // enable left
        write(0x20,0b001000000,0b111111000);
    } else {
        // disable left
        write(0x20,0b000000000,0b111111000);
    }

    if (p & 0b001) {
        // enable right
        write(0x21,0b001000000,0b111111000);
    } else {
        // disable left
        write(0x21,0b000000000,0b111111000);
    }

    // disable microphone inputs
    value = p << 4;
    mask = 0b011 << 4;
    return write(0x2f, value, mask);
}



bool AudioControlWM8960::inputLevel(float n)
{
    return inputLevel(n,n);

}

bool AudioControlWM8960::inputLevel(float l, float r)
{
    uint16_t i;
    i = (uint16_t) (63.0*l+0.5);
    // Left input
    write(0x0, i, 0b000111111, true);

    i = (uint16_t) (63.0*r+0.5);
    // Right input (MEMS)
    return write(0x1, 0b100000000 | i, 0b100111111, true);
}

bool AudioControlWM8960::inputSelect(int n)
{
    if (n) {
        return lineinPower(0b011);
    } else {
        return micPower(0b011);
    }
}

void AudioControlWM8960::dacVolumeRampDisable(void)
{
     write(0x06, 0b000000000, 0b000001000);
}

void AudioControlWM8960::dacVolumeRamp(void)
{
    write(0x06, 0b000001100, 0b000001100);
}

void AudioControlWM8960::dacVolumeRampLinear(void)
{
    write(0x06, 0b000001000, 0b000001100);    
}

void AudioControlWM8960::dacVolume(float vol)
{
     volume(vol, vol);
}

void AudioControlWM8960::muteHeadphone(void)
{
    volume(0.0f, 0.0f);
}

void AudioControlWM8960::unmuteHeadphone(void)
{
    volume(1.0f, 1.0f);
}

void AudioControlWM8960::lineInLevel(float x)
{
    inputLevel(x);
}

void AudioControlWM8960::unmuteLineout(void)
{
    volume(1.0f, 1.0f);
}

void AudioControlWM8960::muteLineout(void)
{
    volume(0.0f, 0.0f);
}

void AudioControlWM8960::adcHighPassFilterEnable(void)
{
    write(0x05, 0b000000000, 0b000000001);
}

void AudioControlWM8960::adcHighPassFilterDisable(void)
{
        // Unmute DAC
    write(0x05, 0b000000001, 0b000000001);
}

void AudioControlWM8960::audioProcessorDisable(void)
{
    write(0x05, 0b000000000, 0b000000110);  // De-emphasis control 2:1 = 1 = 48Khz sample rate
    write(0x11, 0b000000000, 0b110000000);  // ALC = OFF
}

void AudioControlWM8960::audioPreProcessorEnable(void)   // AVC on Line-In level
{
    write(0x05, 0b000000110, 0b000000110);  // De-emphasis control 2:1 = 1 = 48Khz sample rate
    write(0x11, 0b110000000, 0b110000000);  // ALC = Stereo mode
}

void AudioControlWM8960::audioPostProcessorEnable(void)
{
    write(0x05, 0b000000110, 0b000000110);  // De-emphasis control 2:1 = 1 = 48Khz sample rate
}