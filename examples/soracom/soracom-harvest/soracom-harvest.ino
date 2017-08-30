#include <WioLTEforArduino.h>
#include <stdio.h>

#define INTERVAL  (60000)

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

  SerialUSB.println("### Connect the \"soracom.io\".");
  delay(5000);
  if (!Wio.Activate("soracom.io", "sora", "sora")) {
    SerialUSB.println("### ERROR! ###");
    return;
  }
}

void loop() {
  char data[100];
  
  SerialUSB.println("### Open.");
  int connectId = Wio.SocketOpen("harvest.soracom.io", 8514, WIOLTE_UDP);
  if (connectId < 0) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }

  SerialUSB.println("### Send.");
  sprintf(data, "{\"uptime\":%lu}", millis() / 1000);
  SerialUSB.print("Send:");
  SerialUSB.print(data);
  SerialUSB.println("");
  if (!Wio.SocketSend(connectId, data)) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }
  
  SerialUSB.println("### Receive.");
  int length;
  do {
    length = Wio.SocketReceive(connectId, data, sizeof (data));
    if (length < 0) {
      SerialUSB.println("### ERROR! ###");
      goto err;
    }
  } while (length == 0);
  SerialUSB.print("Receive:");
  SerialUSB.print(data);
  SerialUSB.println("");

  SerialUSB.println("### Close.");
  if (!Wio.SocketClose(connectId)) {
    SerialUSB.println("### ERROR! ###");
    goto err;
  }
  
err:
  delay(INTERVAL);
}
