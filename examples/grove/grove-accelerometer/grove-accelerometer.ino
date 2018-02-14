#include <WioLTEforArduino.h>
#include <ADXL345.h>          // https://github.com/Seeed-Studio/Accelerometer_ADXL345

#define INTERVAL    (100)

WioLTE Wio;
ADXL345 Accel;

void setup()
{
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  SerialUSB.println("### I/O Initialize.");
  Wio.Init();
  
  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyGrove(true);
  delay(500);
  Accel.powerOn();
  
  SerialUSB.println("### Setup completed.");
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

