
#ifndef SVN1AFN_BandpassFilters_h
#define SVN1AFN_BandpassFilters_h

#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_MCP23017.h>

// GPIO bits on latch A
#define PA0 0
#define PA1 1
#define PA2 2
#define PA3 3
#define PA4 4
#define PA5 5
#define PA6 6
#define PA7 7

// GPIO bits on latch B
#define PB0 8
#define PB1 9
#define PB2 10
#define PB3 11
#define PB4 12
#define PB5 13
#define PB6 14
#define PB7 15

// GPIO map to Pins on J9 of bandpass filter.
// Feel free to change the mapping to suit your connections.
#define J9_1M8 PA0
#define J9_3M5  PA1
#define J9_7M   PA2
#define J9_10M  PA3
#define J9_14M  PA4
#define J9_18M  PA5
#define J9_21M  PA6
#define J9_24M  PA7
#define J9_28M  PB0
#define J9_BP   PB1
#define J9_ATT2 PB2
#define J9_LNA2 PB3
#define J9_5M   PB4

#define ALLFILTERS 1<<J9_1M8 | 1<<J9_3M5 | 1<<J9_5M | 1<<J9_7M | 1<<J9_10M | 1<<J9_14M | 1<<J9_18M | 1<<J9_21M | 1<<J9_24M | 1<<J9_28M | 1<<J9_BP

enum HFBand { HF160, HF80, HF60, HF40, HF30, HF20, HF17, HF15, HF12, HF10, HFBypass, HFNone };

class SVN1AFN_BandpassFilters
{
  public:
    SVN1AFN_BandpassFilters();
    void begin(uint8_t addr, TwoWire *theWire = &Wire);
    void begin(TwoWire *theWire = &Wire);
    void setBand(enum HFBand);
    void setPreamp(bool on);
    void setAttenuator(bool on);
    
    private:
      Adafruit_MCP23017 *_mcp;
      void setBandBit(uint8_t);
      void setNoFilters();
};
#endif
