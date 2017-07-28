#include <wiolte-driver.h>

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  WioLTE wio;
  
  SerialUSB.println("### I/O Initialize.");
  wio.Init();
  
  SerialUSB.println("### Power supply ON.");
  wio.PowerSupplyLTE(true);
  delay(5000);

  SerialUSB.println("### Turn on or reset.");
  if (!wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
  
  SerialUSB.println("### Connect the \"soracom.io\".");
  delay(5000);
  if (!wio.WriteCommandAndWaitForResponse("AT+CREG?", "OK", 2000)) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+CGREG?", "OK", 2000)) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+CEREG?", "OK", 2000)) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+QIACT?", "OK", 2000)) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+QICSGP=1,1,\"soracom.io\",\"sora\",\"sora\",1", "OK", 2000)) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+QIACT=1", "OK", 2000)) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+QIACT?", "OK", 2000)) SerialUSB.println("### ERROR! ###");

  SerialUSB.println("### Finish.");
}

void loop() {
}

////////////////////////////////////////////////////////////////////////////////////////
