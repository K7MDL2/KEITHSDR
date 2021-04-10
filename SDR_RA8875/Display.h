#ifndef _DISPLAY_H_
#define _DISPLAY_H_
//////////////////////////////////////////////////////////////
//
//   Display.h
//
//////////////////////////////////////////////////////////////
#include <Arduino.h>

//void ringMeter(int val, int minV, int maxV, int16_t x, int16_t y, uint16_t r, const char* units, uint16_t colorScheme,uint16_t backSegColor,int16_t angle,uint8_t inc);
uint16_t grandient(uint8_t val);
void draw_2_state_Button(uint8_t button, uint8_t *function_ptr);
void refreshScreen(void);
const char * formatVFO(uint32_t vfo);
void displayTime(void);
void displayMeter(int val, const char *string);
void drawLabel(uint8_t lbl_num, uint8_t *function_ptr);
void displayRefresh();
// Bottom Panel Anchor button
void displayFn();   // make fn=1 to call displayFn() to prevent calling itself
void displayFreq();    // display frequency
#ifdef ENET
void displayTime();
#endif
// Panel 1 buttons
void displayMode();
void displayFilter();
void displayAttn();
void displayPreamp();
void displayRate();
void displayBand();
//Panel 2 buttons
void displayNB();
void displayNR();
void displaySpot();
void displayNotch();
void displayAgc();
void displayMute();
//Panel 3 buttons
void displayMenu();
void displayANT();
void displayATU();
void displayXMIT();
void displayBandUp();
void displayBandDn();
//Panel 4 buttons
void displayRIT();
void displayXIT();
void displayVFO_AB();
void displayFine();
void displayDisplay();
void displaySplit();
//Panel 5 buttons
void displayRFgain();
void displayAFgain();
void displayEnet();
void displayXVTR();
void displayRefLevel();

#endif //_DISPLAY_H_