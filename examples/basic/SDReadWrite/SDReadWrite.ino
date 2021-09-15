#include <WioLTEforArduino.h>
#if defined ARDUINO_ARCH_STM32F4
#include <SD.h>                    // https://github.com/Seeed-Studio/SD
#elif defined ARDUINO_ARCH_STM32
#include <SDforWioLTE.h>           // https://github.com/SeeedJP/SDforWioLTE
#endif

#define FILE_NAME "test.txt"

WioCellular Wio;

void setup() {
  delay(200);

  SerialUSB.begin(115200);
  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplySD(true);
  delay(500);

  SerialUSB.println("### Initialize SD card.");
  if (!SD.begin()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  File myFile;
  
  SerialUSB.println("### Writing to "FILE_NAME".");
  myFile = SD.open(FILE_NAME, FILE_WRITE);
  if (!myFile) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
  myFile.println("testing 1, 2, 3.");
  myFile.close();

  SerialUSB.println("### Reading from "FILE_NAME".");
  myFile = SD.open(FILE_NAME);
  if (!myFile) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
  SerialUSB.println(FILE_NAME":");
  while (myFile.available()) {
    SerialUSB.write(myFile.read());
  }
  myFile.close();

  SerialUSB.println("### Setup completed.");
}

void loop() {
}
