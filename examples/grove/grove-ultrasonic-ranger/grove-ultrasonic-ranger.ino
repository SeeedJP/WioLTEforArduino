#include <WioLTEforArduino.h>
#include <Ultrasonic.h>			// https://github.com/Seeed-Studio/Grove_Ultrasonic_Ranger

#define ULTRASONIC_PIN  (WIOLTE_D38)
#define INTERVAL        (100)

Ultrasonic UltrasonicRanger(ULTRASONIC_PIN);

void setup()
{
}

void loop()
{
  long distance;
  distance = UltrasonicRanger.MeasureInCentimeters();
  SerialUSB.print(distance);
  SerialUSB.println("[cm]");
  
  delay(INTERVAL);
}

