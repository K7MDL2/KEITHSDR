
#include <Wire.h>
#include <SVN1AFN_BandpassFilters.h>
// Expanded test program to cycle through all relays, 1 per second.  Supports 60M band with modified library.

SVN1AFN_BandpassFilters _bpf;

void setup() {
  // put your setup code here, to run once:

  // bpf.begin(uint8_t baseAddressOffset, TwoWire &Wire);
  
  // I2C base address for the MCP23017 is 0x20;
  // You can change address jumpers on the I/O board to set it 
  //  to anywhere between 0x20 and 0x28.  You would do this if you
  //  have multiple port expanders on the same I2C bus.
  //  Use respective base address offset.   
  //     Example:  Base address set to 0x23, 
  //         call with bpf.begin(3, &Wire);

  // Wire.h accomodates other I2C buses on the processor.
  // Wire = the first bus, Wire1 = second bus, and so forth.
  // If the bandpass filters are on the second bus, use
  //    bpf.begin(0, &Wire1);
  Serial.begin(9600);
  delay(1000);
  Serial.println("Init Start");
 
   _bpf.begin(0, &Wire);

  // Call with HF160, HF80, HF60, HF40, HF30, HF20, HF17, HF15, HF12, HF10, HFBypass, or HFNone
  _bpf.setBand(HF40);

  // To change bands call setBand again
  _bpf.setBand(HF80);

  _bpf.setPreamp(true);
  _bpf.setAttenuator(false);

  // To deactivate all relays on the filter board.
  _bpf.setBand(HFNone);
  _bpf.setPreamp(false);
  _bpf.setAttenuator(false); 
  
  Serial.println("Init Done");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  _bpf.setBand(HF160);
  Serial.println(HF160);
  delay(1000);
  _bpf.setBand(HF80);
  Serial.println(HF80);
  delay(1000);
  _bpf.setBand(HF60);
  Serial.println(HF60);
  delay(1000);
  _bpf.setBand(HF40);
  Serial.println(HF40);
  delay(1000);
  _bpf.setBand(HF30);
  Serial.println(HF30);
  delay(1000);
  _bpf.setBand(HF20);
  Serial.println(HF20);
  delay(1000);
  _bpf.setBand(HF17);
  Serial.println(HF17);
  delay(1000);
  _bpf.setBand(HF15);
  Serial.println(HF15);
  delay(1000);
  _bpf.setBand(HF12);
  Serial.println(HF12);
  delay(1000);
  _bpf.setBand(HF10);
  Serial.println(HF10);
  delay(1000);
  _bpf.setPreamp(false);
  Serial.println("Preamp Off");
  delay(1000);
  _bpf.setAttenuator(true);
  Serial.println("Preamp On");
  delay(1000);
  _bpf.setAttenuator(false);
  Serial.println("Attenuator Off");
  delay(1000);
  _bpf.setAttenuator(true);
  Serial.println("Attenuator On");
  delay(3000);  
}
