#include <WioLTEforArduino.h>

#define INTERVAL  (1000)

WioCellular Wio;

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  SerialUSB.println("### I/O Initialize.");
  Wio.Init();
  
  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyCellular(true);
  delay(500);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
  delay(3000);

  SerialUSB.println("### Get phone number.");
  char str[100];
  if (Wio.GetPhoneNumber(str, sizeof (str)) <= 0) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
  SerialUSB.println(str);

  SerialUSB.println("### Setup completed.");
}

void loop() {
  SerialUSB.print(".");
  while (true) {
    char str[200];
    int strLen = Wio.ReceiveSMS(str, sizeof (str));
    if (strLen < 0) {
      SerialUSB.println("### ERROR! ###");
      goto err;
    }
    if (strLen == 0) break;
    
    SerialUSB.println("");
    SerialUSB.println(str);

    if (!Wio.DeleteReceivedSMS()) {
      SerialUSB.println("### ERROR! ###");
      goto err;
    }
  }
  
err:
  delay(INTERVAL);
}
