#include <WioLTEforArduino.h>
#include <stdio.h>

#define APN               "soracom.io"
#define USERNAME          "sora"
#define PASSWORD          "sora"

#define WEBHOOK_EVENTNAME "wiolte_uptime"
#define WEBHOOK_KEY       "_6wh2r_acHW26-rm3XV5D"
#define WEBHOOK_URL       "https://maker.ifttt.com/trigger/"WEBHOOK_EVENTNAME"/with/key/"WEBHOOK_KEY

#define INTERVAL          (60000)

WioLTE Wio;
  
void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  SerialUSB.println("### I/O Initialize.");
  Wio.Init();
  
  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  delay(5000);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Connect the \""APN"\".");
  delay(5000);
  if (!Wio.Activate(APN, USERNAME, PASSWORD)) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
}

void loop() {
  char data[100];
  
  SerialUSB.println("### Post.");
  sprintf(data, "{\"value1\":\"uptime %lu\"}", millis() / 1000);
  SerialUSB.print("Post:");
  SerialUSB.print(data);
  SerialUSB.println("");
  if (!Wio.HttpPost(WEBHOOK_URL, data)) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }
  
err:
  SerialUSB.println("### Wait.");
  delay(INTERVAL);
}

