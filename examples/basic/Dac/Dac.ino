#include <config.h>
#include <WioLTEforArduino.h>

// DAC output sample

void setup() {
  WioLTEDac::Init(WioLTEDac::DAC1);
}

void loop() {
  for (uint16_t i = 0; i < 4096; i++)
  {
    WioLTEDac::Write(WioLTEDac::DAC1, i);
  }
}
