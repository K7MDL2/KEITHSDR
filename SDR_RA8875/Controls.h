#ifndef _CONTROLS_H_
#define _CONTROLS_H_
//
//    Controls.h
//
//    Core functions that control things.  The controller may be a touch event from a button or label object
//      or from a mechanical device or another function in a chain of interdepedencies.
//
//    Many controls have multipe possible controllers so have to adopt an Controller->control->display model
//      allowing parallel control requests.  
//      The control object changes (or requests to change) states, the display only scans and reports the state.
//      It becomes importan to pass through this as remote control and monitoring get built.
#include <Arduino.h>

void Set_Spectrum_Scale(int8_t zoom_dir);
void Set_Spectrum_RefLvl(int8_t zoom_dir);
void changeBands(int8_t direction);
void pop_win(uint8_t init);
void Mute();
void Menu();
void Display();
void Band();
void BandDn();
void BandUp();
void Notch();
void Spot();
void Enet();
void NR();
void NB(int8_t toggle);
void Xmit();
void Ant();
void Fine();
void Rate(int8_t direction);
void setMode(int8_t dir);
void AGC();
void Filter(int dir);
void ATU();
void Xvtr();
void Split();
void XIT();
void RIT();
void Preamp(int8_t toggle);
void Atten(int8_t toggle);
void VFO_AB();
void setAtten_dB(int8_t atten);
void setAFgain(int8_t toggle);
void AFgain(int8_t delta);
void setRFgain(int8_t toggle);
void RFgain(int8_t delta);
void setRefLevel(int8_t toggle);
void setNBLevel(int8_t delta);
void RefLevel(int8_t newval);
void TouchTune(int16_t touch_Freq);
void selectStep(uint8_t fndx);
void selectAgc(uint8_t andx);

#endif  // _CONTROLS_H_
