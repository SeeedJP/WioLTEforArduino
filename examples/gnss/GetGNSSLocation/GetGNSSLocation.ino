#include <WioLTEforArduino.h>

#define INTERVAL  (5000)

WioLTE Wio;

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("### I/O Initialize.");
  Wio.Init();

  SerialUSB.println("### Power supply ON.");
  Wio.PowerSupplyLTE(true);
  SerialUSB.println("### Power supply GNSS.");
  Wio.PowerSupplyGNSS(true);
  delay(500);

  SerialUSB.println("### Turn on or reset.");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### ERROR! ###");
    return;
  }

  SerialUSB.println("### Enable GNSS.");
  if (!Wio.EnableGNSS()) {
    SerialUSB.println("### ERROR! ###");
	return;
  }

  SerialUSB.println("### Setup completed.");
}

void loop() {
  double longitude;
  double latitude;
  double altitude;
  struct tm utc;

  SerialUSB.println("### Get GNSS location.");
  bool getLocation = Wio.GetGNSSLocation(&longitude, &latitude, &altitude, &utc);

  if (getLocation) {
    SerialUSB.print("    long: ");
    SerialUSB.println(longitude, 6);
    SerialUSB.print("    lat: ");
    SerialUSB.println(latitude, 6);
    SerialUSB.print("    altitude: ");
    SerialUSB.println(altitude, 6);
    SerialUSB.print("    utc: ");
    SerialUSB.println(asctime(&utc));
  }
  else if (Wio.GetLastError() == WioLTE::E_GNSS_NOT_FIXED) {
    SerialUSB.println("### NOT FIXED. ###");
  }
  else {
    SerialUSB.println("### ERROR! ###");
  }

  delay(INTERVAL);
}
