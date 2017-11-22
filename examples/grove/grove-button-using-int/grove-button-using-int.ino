#include <WioLTEforArduino.h>

#define BUTTON_PIN  (WIOLTE_D38) // You can use WIOLTE_(D20|A4|A6) with `Wio.PowerSupplyGrove(true);`

WioLTE Wio;

volatile boolean state = false;

void change_state() {
  SerialUSB.print(state ? '*' : '.');
  state = !state;
}

void setup()
{
  Wio.Init();
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(BUTTON_PIN, change_state, RISING);
}

void loop()
{
  if (state) { 
    Wio.LedSetRGB(127, 127, 127);
  } else {
    Wio.LedSetRGB(0, 0, 0);
  }
}

