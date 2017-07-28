#include <wiolte-driver.h>

////////////////////////////////////////////////////////////////////////////////////////
// Helper functions

void PinModeAndDefault(int pin, WiringPinMode mode)
{
  pinMode(pin, mode);
}

void PinModeAndDefault(int pin, WiringPinMode mode, int value)
{
  pinMode(pin, mode);
  if (mode == OUTPUT) digitalWrite(pin, value);
}

////////////////////////////////////////////////////////////////////////////////////////
// setup and loop

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  WioLTE wio;
  WioLTE::Stopwatch sw;
  
  SerialUSB.println("### I/O Initialize.");
  PinModeAndDefault(WioLTE::MODULE_PWR_PIN , OUTPUT, LOW);
  PinModeAndDefault(WioLTE::ANT_PWR_PIN    , OUTPUT, LOW);
  PinModeAndDefault(WioLTE::ENABLE_VCCB_PIN, OUTPUT, LOW);
  wio.Init();
  
  SerialUSB.println("### Power supply ON.");
  digitalWrite(WioLTE::MODULE_PWR_PIN , HIGH);    
  digitalWrite(WioLTE::ANT_PWR_PIN    , LOW );  // Supply for GNSS antenna
  digitalWrite(WioLTE::ENABLE_VCCB_PIN, HIGH);  // Supply for Grove
  delay(5000);

  if (!wio.IsBusy()) {
    SerialUSB.println("### Reset.");
    wio.Reset();
  }
  else {
    SerialUSB.println("### Turn on.");
    if (!wio.TurnOn()) {
      SerialUSB.println("### ERROR! ###");
      return;
    }
  }

  SerialUSB.println("### Wait for ready.");
  sw.Start();
  while (!wio.WaitForResponse("RDY")) {
    if (sw.ElapsedMilliseconds() >= 10000) {
      SerialUSB.println("### ERROR! ###");
      return;
    }
    SerialUSB.print(".");
  }
  SerialUSB.println("");
  
  SerialUSB.println("### Check AT command communication.");
  sw.Start();
  while (!wio.WriteCommandAndWaitForResponse("AT", "OK")) {
    if (sw.ElapsedMilliseconds() >= 10000) {
      SerialUSB.println("### ERROR! ###");
      return;
    }
    SerialUSB.print(".");
  }
  SerialUSB.println("");

  SerialUSB.println("### Echo off.");
  if (!wio.WriteCommandAndWaitForResponse("ATE0", "OK")) SerialUSB.println("### ERROR! ###");

  SerialUSB.println("### Wait for PIN.");
  sw.Start();
  while (!wio.WriteCommandAndWaitForResponse("AT+CPIN?", "OK")) {
    if (sw.ElapsedMilliseconds() >= 10000) {
      SerialUSB.println("### ERROR! ###");
      return;
    }
    SerialUSB.print(".");
  }
  SerialUSB.println("");
  
  SerialUSB.println("### Connect the \"soracom.io\".");
  delay(5000);
  if (!wio.WriteCommandAndWaitForResponse("AT+CREG?", "OK")) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+CGREG?", "OK")) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+CEREG?", "OK")) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+QIACT?", "OK")) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+QICSGP=1,1,\"soracom.io\",\"sora\",\"sora\",1", "OK")) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+QIACT=1", "OK")) SerialUSB.println("### ERROR! ###");
  if (!wio.WriteCommandAndWaitForResponse("AT+QIACT?", "OK")) SerialUSB.println("### ERROR! ###");

  SerialUSB.println("### Finish.");
}

void loop() {
}

////////////////////////////////////////////////////////////////////////////////////////
