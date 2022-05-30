#include "SVN1AFN_BandpassFilters.h"
#include "Wire.h"

SVN1AFN_BandpassFilters::SVN1AFN_BandpassFilters()
{
  _mcp = new Adafruit_MCP23017();
}

void SVN1AFN_BandpassFilters::begin(uint8_t address, TwoWire *theWire)
{
  // call versoin of begin for I2C bus (new since V2 adafruit mcp23017 library)
  _mcp->begin_I2C(address, theWire);
  _mcp->pinMode(J9_1M8,  OUTPUT);
  _mcp->pinMode(J9_3M5,  OUTPUT);
  _mcp->pinMode(J9_5M,   OUTPUT);
  _mcp->pinMode(J9_7M,   OUTPUT);
  _mcp->pinMode(J9_10M,  OUTPUT);
  _mcp->pinMode(J9_14M,  OUTPUT);
  _mcp->pinMode(J9_18M,  OUTPUT);
  _mcp->pinMode(J9_21M,  OUTPUT);
  _mcp->pinMode(J9_24M,  OUTPUT);
  _mcp->pinMode(J9_28M,  OUTPUT);
  _mcp->pinMode(J9_BP,   OUTPUT);
  _mcp->pinMode(J9_ATT2, OUTPUT);
  _mcp->pinMode(J9_LNA2, OUTPUT);
}

void SVN1AFN_BandpassFilters::setBand(HFBand band)
{
  switch(band)
  {
    case HF160:
      setBandBit(J9_1M8);
      break;
    case HF80:
      setBandBit(J9_3M5);
      break;
    case HF60:
      setBandBit(J9_5M);
      break;
    case HF40:
      setBandBit(J9_7M);
      break;
    case HF30:
      setBandBit(J9_10M);
      break;
    case HF20:
      setBandBit(J9_14M);
      break;
    case HF17:
      setBandBit(J9_18M);
     break;
    case HF15:
      setBandBit(J9_21M);
      break;
    case HF12:
      setBandBit(J9_24M);
      break;
    case HF10:
      setBandBit(J9_28M);
      break;
    case HFBypass:
      setBandBit(J9_BP);
      break;
    case HFNone:
	setNoFilters();
	break;
  }
}

void SVN1AFN_BandpassFilters::setBandBit(uint8_t bandBit)
{
    // Read the current state of all 16 bits.
    uint16_t currentBits = _mcp->readGPIOAB();
    
    uint16_t allFilters = ALLFILTERS;

    allFilters = ~allFilters;
    
    // Set all band bits to 0
    currentBits &= allFilters;

    uint16_t shiftedBandBit = 1 << bandBit;
    
    // Set the desired band bit
    currentBits = currentBits | shiftedBandBit;
    _mcp->writeGPIOAB(currentBits);
}

void SVN1AFN_BandpassFilters::setNoFilters()
{
    // Read the current state of all 16 bits.
    uint16_t currentBits = _mcp->readGPIOAB();
    
    uint16_t allFilters = ALLFILTERS;

    allFilters = ~allFilters;
    
    // Set all band bits to 0
    currentBits &= allFilters;

    _mcp->writeGPIOAB(currentBits);
}

void SVN1AFN_BandpassFilters::setAttenuator(bool on)
{
    _mcp->digitalWrite(J9_ATT2, (uint8_t)on);
}

void SVN1AFN_BandpassFilters::setPreamp(bool on)
{
      _mcp->digitalWrite(J9_LNA2, (uint8_t)on);
}