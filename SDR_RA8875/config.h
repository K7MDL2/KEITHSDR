////////////////////////////////////////////////////////////////////////////
//
// Example config.h file that uses the CWKeyerShield library and is valid
// for the Hardware developed by Steve
// (see github.com://softerhardware/CWKeyer)
//
////////////////////////////////////////////////////////////////////////////

#define HWSERIAL                    // use standard serial connection (1200 baud) for Winkey protocol

////////////////////////////////////////////////////////////////////////////
//
// Digital input  pins for Paddle and StraightKey.
//
// Note the keyer does not need digital output lines defined if working
// with the CWKeyerShield library
//
////////////////////////////////////////////////////////////////////////////

#define PaddleRight              0   // Digital input for right paddle
#define PaddleLeft               1   // Digital input for left paddle
#define StraightKey              2   // Digital input for straight key

////////////////////////////////////////////////////////////////////////////
//
// Hardware settings for the CWKeyerShield library.
// (if CWKEYERSHIELD is not defined, the other settings in this section
//  have no meaning)
//
// These settings are passed to the CWKeyerShield upon construction
//
////////////////////////////////////////////////////////////////////////////

#define CWKEYERSHIELD                       // use CWKeyerShield library (audio and MIDI)
#define SHIELD_AUDIO_OUTPUT             2   // 0: MQS, 1: I2S(wm8960), 2: I2S(sgtl5000)
#define SHIELD_ANALOG_MASTERVOLUME     A1   // Analog input for master volume pot
#define SHIELD_ANALOG_SIDETONEVOLUME   A2   // Analog input for side tone volume pot
#define SHIELD_ANALOG_SIDETONEFREQ     A3   // Analog input for side tone frequency pot
#define SHIELD_ANALOG_SPEED            A8   // Analog input for speed pot
#define SHIELD_DIGITAL_MICPTT           3   // Digital input for PTT
#define SHIELD_DIGITAL_PTTOUT           13   // Digital output for PTT
#define SHIELD_DIGITAL_CWOUT            5   // Digital output for CW Keydown        

////////////////////////////////////////////////////////////////////////////
//
// run-time configurable settings for the CWKeyerShield library
// (not necessary to define them if we are using the default values)
// Note that settings the side-tone frequency/volume is only meaningful
// if there are *no* pots to adjust them.
//
////////////////////////////////////////////////////////////////////////////

#define MY_MUTE_OPTION                0   // set to 1 then RX audio is muted during CW PTT
#define MY_DEFAULT_FREQ             800   // initial setting of side tone frequency
#define MY_DEFAULT_VOLUME           127   // initial setting of side tone volume (0-127)