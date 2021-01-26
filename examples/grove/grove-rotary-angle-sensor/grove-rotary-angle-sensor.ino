#include <WioLTEforArduino.h>

#define ROTARY_ANGLE_PIN  (WIOLTE_A4)
#define INTERVAL          (500)
#define BAR_LENGTH        (40)

WioLTE Wio;

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyGrove(true);
  delay(500);

  SerialUSB.println("### Setup pin mode.");
  pinMode(ROTARY_ANGLE_PIN, INPUT_ANALOG);

#ifdef ARDUINO_ARCH_STM32
  analogReadResolution(12);
#endif // ARDUINO_ARCH_STM32
}

void loop() {
  int rotaryAngle = analogRead(ROTARY_ANGLE_PIN);

  int i;
  for (i = 0; i < BAR_LENGTH * rotaryAngle / 4095; i++) SerialUSB.print("*");
  for (; i < BAR_LENGTH; i++) SerialUSB.print(".");
  SerialUSB.print(" ");
  SerialUSB.println(rotaryAngle);
  
  delay(INTERVAL);
}
