#include <WioLTEforArduino.h>
#include <ADXL345.h>          // https://github.com/Seeed-Studio/Accelerometer_ADXL345

#define INTERVAL    (100)

ADXL345 Accel;

void setup()
{
  Accel.powerOn();
}

void loop()
{
  int x;
  int y;
  int z;
  Accel.readXYZ(&x, &y, &z);
  SerialUSB.print(x);
  SerialUSB.print(' ');
  SerialUSB.print(y);
  SerialUSB.print(' ');
  SerialUSB.println(z);
  
  delay(INTERVAL);
}

