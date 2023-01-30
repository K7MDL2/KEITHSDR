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

#ifndef control_wm8960_h_
#define control_wm8960_h_

#include "AudioControl.h"
#include "Wire.h"

class AudioControlWM8960 : public AudioControl
{
public:
    AudioControlWM8960(void) { Wire1.begin(); delay(5); }
    // Reset and configure ww8960
    bool enable(void);
    // Reset, most settings are in a powered off state when reset
    bool disable(void);

    // Input level, either stereo or combined
    bool inputLevel(float n);
    bool inputLevel(float l, float r);

    // Select input, 0 is microphones, 1 is linein
    bool inputSelect(int n);

    // Master volume for both headphones and speaker, either stereo or combined
    bool volume(float n);
    bool volume(float l, float r);

    // Stereo volume for headphones only
    bool headphoneVolume(float l, float r);
    // Two bits, set L and R to 1 in 0b0LR to turn on channels
    bool headphonePower(uint8_t p);

    // Stereo volume for headphones only
    bool speakerVolume(float l, float r);
    // Two bits, set L and R to 1 in 0b0LR to turn on channels
    bool speakerPower(uint8_t p);

    // Write 1 to disable, 0 to enable, default is enabled
    bool disableADCHPF(uint8_t v);

    // Write 1 to enable, 0 to disable, default is disabled
    bool enableMicBias(uint8_t v);

    // Two bits, set L and R to 1 in 0b0LR to turn on channels
    bool enableALC(uint16_t v);

    // Two bits, set L and R to 1 in 0b0LR to turn on channels
    bool micPower(uint8_t p);

    // Two bits, set L and R to 1 in 0b0LR to turn on channels
    bool lineinPower(uint8_t p);

    // Write directly to wm8960
    bool write(uint16_t reg, uint16_t val, uint16_t mask, bool force);
    
    // return to default fast ramp, max delay 10.7ms
    void dacVolumeRampDisable(void);

    // set slow ramp, max delay 171ms
    void dacVolumeRamp(void);

    // same as RampDisable, set to default fast ramp
    void dacVolumeRampLinear(void);

    void dacVolume(float vol);

    void muteHeadphone(void); 
    void unmuteHeadphone(void);

    void audioProcessorDisable(void);   // Default 
    void audioPreProcessorEnable(void);   // AVC on Line-In level
    void audioPostProcessorEnable(void);

    void lineInLevel(float x);

    void unmuteLineout(void); 

    void muteLineout(void);

    void adcHighPassFilterEnable(void);
    void adcHighPassFilterDisable(void);


protected:

    // wm8960 state is write only, this keeps track of state and is initialized with defaults
    uint16_t regmap[56] = {
        0x097, 0x097, 0x000, 0x000, 0x000, 0x008, 0x000, 0x00a, // Registers 0x00-0x07
        0x1c0, 0x000, 0x0ff, 0x0ff, 0x000, 0x000, 0x000, 0x000, // Registers 0x08-0x0f
        0x000, 0x00b, 0x100, 0x032, 0x000, 0x0c3, 0x0c3, 0x1c0, // Registers 0x10-0x17
        0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, // Registers 0x18-0x1f
        0x100, 0x100, 0x050, 0x050, 0x050, 0x050, 0x000, 0x000, // Registers 0x20-0x27
        0x100, 0x100, 0x040, 0x000, 0x000, 0x050, 0x050, 0x000, // Registers 0x28-0x2f
        0x002, 0x037, 0x04d, 0x080, 0x008, 0x031, 0x026, 0x0e9  // Registers 0x30-0x37
    };

};

#endif
