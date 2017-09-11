#include <WioLTEforArduino.h>

#define MAGNETIC_SWITCH_PIN (WIOLTE_D38)
#define INTERVAL            (100)

void setup()
{
  pinMode(MAGNETIC_SWITCH_PIN, INPUT);
}

void loop()
{
  int switchState = digitalRead(MAGNETIC_SWITCH_PIN);
  SerialUSB.print(switchState ? '*' : '.');
  
  delay(INTERVAL);
}

