#include <stdio.h>

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

void DebugPrint(const char* data)
{
  char message[10];
  int length = strlen(data);
  
  SerialUSB.print(length);
  
  SerialUSB.print(":");
  
  for (int i = 0; i < length; i++) {
    if (data[i] >= 0x20)
      SerialUSB.print((char)data[i]);
    else
      SerialUSB.print('.');
  }
  
  SerialUSB.print(":");
  
  for (int i = 0; i < length; i++) {
    sprintf(message, "%02x ", data[i]);
    SerialUSB.print(message);
  }
  
  SerialUSB.println("");
}

////////////////////////////////////////////////////////////////////////////////////////
// Stopwatch

class Stopwatch
{
private:
  unsigned long _BeginTime;
  unsigned long _EndTime;

public:
  Stopwatch() : _BeginTime(0), _EndTime(0)
  {
  }

  void Start()
  {
    _BeginTime = millis();
    _EndTime = 0;
  }

  void Stop()
  {
    _EndTime = millis();
  }

  unsigned long ElapsedMilliseconds()
  {
    if (_EndTime == 0) {
      return millis() - _BeginTime;
    }
    else {
      return _EndTime - _BeginTime;
    }
  }
};

////////////////////////////////////////////////////////////////////////////////////////
// WioLTE

#define MODULE_RESPONSE_MAX_SIZE  (100)

class WioLTE
{
public:
  static const int MODULE_PWR_PIN     = 18; // PB5
  static const int ANT_PWR_PIN        = 28; // PB12
  static const int ENABLE_VCCB_PIN    = 26; // PB10    

private:
  static const int PWR_KEY_PIN        = 36; // PC4 
  static const int WAKEUP_IN_PIN      = 32; // PC0
  static const int AP_READY_PIN       = 33; // PC1
  static const int WAKEUP_DISABLE_PIN = 34; // PC2
  static const int RESET_MODULE_PIN   = 35; // PC3
  static const int STATUS_PIN         = 31; // PB15
  static const int RGB_LED_PIN        = 17; // PB1

private:
  HardwareSerial* _Serial;
  long _Timeout;

private:
  void DiscardRead();
  void SetTimeout(long timeout);
  bool ReadLine(char* data, int dataSize);

public:
  void WriteCommand(const char* command);
  bool WaitForResponse(const char* response);
  bool WriteCommandAndWaitForResponse(const char* command, const char* response);

public:
  WioLTE();
  void Init();
  void Reset();
  bool IsBusy() const;
  bool TurnOn();
  
};

void WioLTE::DiscardRead()
{
  while (_Serial->available()) _Serial->read();
}

void WioLTE::SetTimeout(long timeout)
{
  _Timeout = timeout;
  _Serial->setTimeout(_Timeout);
}

bool WioLTE::ReadLine(char* data, int dataSize)
{
  int dataIndex = 0;

  Stopwatch sw;
  while (dataIndex < dataSize - 1) {
    sw.Start();
    while (!_Serial->available()) {
      if (sw.ElapsedMilliseconds() > _Timeout) {
        data[dataIndex] = '\0';
        return false; // Timeout.
      }
    }
    int c = _Serial->read();
    if (c < 0) {
      data[dataIndex] = '\0';
      return false; // No data.
    }
    if (c == '\r') continue;
    
    if (c == '\n') {
      data[dataIndex] = '\0';
      return true;
    }

    data[dataIndex++] = c;
  }

  if (dataIndex < dataSize) {
    data[dataIndex] = '\0';
  }
  return false; // Overflow.
}

void WioLTE::WriteCommand(const char* command)
{
  SerialUSB.print("<- ");
  DebugPrint(command);
  
  _Serial->write(command);
  _Serial->write('\r');
}

bool WioLTE::WaitForResponse(const char* response)
{
  char data[MODULE_RESPONSE_MAX_SIZE];
  do {
    if (!ReadLine(data, sizeof (data))) return false;
    SerialUSB.print("-> ");
    DebugPrint(data);
  }
  while (strcmp(data, response) != 0);

  return true;
}

bool WioLTE::WriteCommandAndWaitForResponse(const char* command, const char* response)
{
  WriteCommand(command);
  return WaitForResponse(response);
}

WioLTE::WioLTE() : _Serial(&Serial1), _Timeout(2000)
{
}

void WioLTE::Init()
{
  // Turn on/off Pins
  PinModeAndDefault(PWR_KEY_PIN, OUTPUT, LOW);
  PinModeAndDefault(RESET_MODULE_PIN, OUTPUT, HIGH);
  // Status Indication Pins
  PinModeAndDefault(STATUS_PIN, INPUT);
  // GPIO Pins
  PinModeAndDefault(WAKEUP_IN_PIN, OUTPUT, LOW);
  PinModeAndDefault(WAKEUP_DISABLE_PIN, OUTPUT, HIGH);
  //PinModeAndDefault(AP_READY_PIN, OUTPUT);  // NOT use
  
  _Serial->begin(115200);
}

void WioLTE::Reset() 
{
  digitalWrite(RESET_MODULE_PIN, LOW);
  delay(200);
  digitalWrite(RESET_MODULE_PIN, HIGH);
  delay(300);
}

bool WioLTE::IsBusy() const
{
  return digitalRead(STATUS_PIN) ? true : false;
}

bool WioLTE::TurnOn()
{
  delay(100);
  digitalWrite(PWR_KEY_PIN, HIGH);
  delay(200);
  digitalWrite(PWR_KEY_PIN, LOW);

  Stopwatch sw;
  sw.Start();
  while (IsBusy()) {
    SerialUSB.print(".");
    if (sw.ElapsedMilliseconds() >= 5000) return false;
    delay(100);
  }
  SerialUSB.println("");

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////
//

void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");
  
  WioLTE wio;
  Stopwatch sw;
  
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

