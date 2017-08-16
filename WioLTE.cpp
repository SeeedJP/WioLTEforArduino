#include <Arduino.h>
#include <stdio.h>
#include "wiolte-driver.h"

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(str)			SerialUSB.print(str)
#define DEBUG_PRINTLN(str)			SerialUSB.println(str)
#else
#define DEBUG_PRINT(str)
#define DEBUG_PRINTLN(str)
#endif

////////////////////////////////////////////////////////////////////////////////////////
// Helper functions

static void PinModeAndDefault(int pin, WiringPinMode mode)
{
	pinMode(pin, mode);
}

static void PinModeAndDefault(int pin, WiringPinMode mode, int value)
{
	pinMode(pin, mode);
	if (mode == OUTPUT) digitalWrite(pin, value);
}

////////////////////////////////////////////////////////////////////////////////////////
// WioLTE

bool WioLTE::Reset()
{
	digitalWrite(RESET_MODULE_PIN, LOW);
	delay(200);
	digitalWrite(RESET_MODULE_PIN, HIGH);
	delay(300);

	Stopwatch sw;
	sw.Start();
	while (_Module.WaitForResponse("RDY", 100) == NULL) {
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	DEBUG_PRINTLN("");

	return true;
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
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 5000) return false;
		delay(100);
	}
	DEBUG_PRINTLN("");

	sw.Start();
	while (_Module.WaitForResponse("RDY", 100) == NULL) {
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	DEBUG_PRINTLN("");

	return true;
}

WioLTE::WioLTE() : _Module(), _Led(1, RGB_LED_PIN)
{
}

void WioLTE::Init()
{
	// Power supply
	PinModeAndDefault(MODULE_PWR_PIN, OUTPUT, LOW);
	PinModeAndDefault(ANT_PWR_PIN, OUTPUT, LOW);
	PinModeAndDefault(ENABLE_VCCB_PIN, OUTPUT, LOW);

	// Turn on/off Pins
	PinModeAndDefault(PWR_KEY_PIN, OUTPUT, LOW);
	PinModeAndDefault(RESET_MODULE_PIN, OUTPUT, HIGH);

	// Status Indication Pins
	PinModeAndDefault(STATUS_PIN, INPUT);

	// GPIO Pins
	PinModeAndDefault(WAKEUP_IN_PIN, OUTPUT, LOW);
	PinModeAndDefault(WAKEUP_DISABLE_PIN, OUTPUT, HIGH);
	//PinModeAndDefault(AP_READY_PIN, OUTPUT);  // NOT use
  
	_Module.Init();
	_Led.begin();
}

void WioLTE::LedSetRGB(byte red, byte green, byte blue)
{
	_Led.WS2812SetRGB(0, red, green, blue);
	_Led.WS2812Send();
}

void WioLTE::PowerSupplyLTE(bool on)
{
	digitalWrite(MODULE_PWR_PIN, on ? HIGH : LOW);
}

void WioLTE::PowerSupplyGNSS(bool on)
{
	digitalWrite(ANT_PWR_PIN, on ? HIGH : LOW);
}

void WioLTE::PowerSupplyGrove(bool on)
{
	digitalWrite(ENABLE_VCCB_PIN, on ? HIGH : LOW);
}

bool WioLTE::IsBusy() const
{
	return digitalRead(STATUS_PIN) ? true : false;
}

bool WioLTE::TurnOnOrReset()
{
	if (!IsBusy()) {
		if (!Reset()) return false;
	}
	else {
		if (!TurnOn()) return false;
	}

	Stopwatch sw;
	sw.Start();
	while (_Module.WriteCommandAndWaitForResponse("AT", "OK", 500) == NULL) {
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	DEBUG_PRINTLN("");

	if (_Module.WriteCommandAndWaitForResponse("ATE0", "OK", 500) == NULL) return false;
	if (_Module.WriteCommandAndWaitForResponse("AT+QURCCFG=\"urcport\",\"usbat\"", "OK", 500) == NULL) return false;

	sw.Start();
	while (_Module.WriteCommandAndWaitForResponse("AT+CPIN?", "OK", 5000) == NULL) {	// TODO
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	DEBUG_PRINTLN("");

	return true;
}

bool WioLTE::SendSMS(const char* dialNumber, const char* message)
{
	if (_Module.WriteCommandAndWaitForResponse("AT+CMGF=1", "OK", 500) == NULL) return false;

	char* str = (char*)alloca(9 + strlen(dialNumber) + 1 + 1);
	sprintf(str, "AT+CMGS=\"%s\"", dialNumber);
	if (_Module.WriteCommandAndWaitForResponse(str, "", 500) == NULL) return false;
	_Module.Write(message);
	_Module.Write("\x1a");
	if (_Module.WaitForResponse("OK", 120000) == NULL) return false;

	return true;
}

bool WioLTE::Activate(const char* accessPointName, const char* userName, const char* password)
{
	const char* parameter;

	_Module.WriteCommand("AT+CREG?");
	if ((parameter = _Module.WaitForResponse("+CREG: ", 500, ModuleSerial::WFR_START_WITH)) == NULL) return false;
	if (strcmp(parameter, "+CREG: 0,1") != 0) return false;	// TODO
	if (_Module.WaitForResponse("OK", 500) == NULL) return false;

	_Module.WriteCommand("AT+CGREG?");
	if ((parameter = _Module.WaitForResponse("+CGREG: ", 500, ModuleSerial::WFR_START_WITH)) == NULL) return false;
	if (strcmp(parameter, "+CGREG: 0,1") != 0) return false;	// TODO
	if (_Module.WaitForResponse("OK", 500) == NULL) return false;

	_Module.WriteCommand("AT+CEREG?");
	if ((parameter = _Module.WaitForResponse("+CEREG: ", 500, ModuleSerial::WFR_START_WITH)) == NULL) return false;
	if (strcmp(parameter, "+CEREG: 0,1") != 0) return false;	// TODO
	if (_Module.WaitForResponse("OK", 500) == NULL) return false;

	char* str = (char*)alloca(15 + strlen(accessPointName) + 3 + strlen(userName) + 3 + strlen(password) + 3 + 1);
	sprintf(str, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1", accessPointName, userName, password);
	if (_Module.WriteCommandAndWaitForResponse(str, "OK", 500) == NULL) return false;

	if (_Module.WriteCommandAndWaitForResponse("AT+QIACT=1", "OK", 150000) == NULL) return false;

	if (_Module.WriteCommandAndWaitForResponse("AT+QIACT?", "OK", 150000) == NULL) return false;

	return true;
}

int WioLTE::SocketOpen(const char* host, int port, SocketType type)
{
	if (host == NULL || host[0] == '\0') return -1;
	if (port < 0 || 65535 < port) return -1;

	const char* typeStr;
	switch (type) {
	case SOCKET_TCP:
		typeStr = "TCP";
		break;
	case SOCKET_UDP:
		typeStr = "UDP";
		break;
	default:
		return -1;
	}

	char* str = (char*)alloca(15 + 3 + 3 + strlen(host) + 2 + 5 + 1);
	sprintf(str, "AT+QIOPEN=1,0,\"%s\",\"%s\",%d", typeStr, host, port);	// TODO
	if (_Module.WriteCommandAndWaitForResponse(str, "OK", 150000) == NULL) return -1;
	if (_Module.WaitForResponse("+QIOPEN: 0,0", 150000) == NULL) return -1;		// TODO

	return 0;
}

bool WioLTE::SocketSend(int connectId, const char* data)
{
	if (connectId > 11) return false;
	int dataLength = strlen(data);
	if (dataLength > 1460) return false;

	char* str = (char*)alloca(10 + 2 + 1 + 4 + 1);
	sprintf(str, "AT+QISEND=%d,%d", connectId, dataLength);
	_Module.WriteCommand(str);
	if (_Module.WaitForResponse("> ", 2000, ModuleSerial::WFR_WITHOUT_DELIM) == NULL) return false;	// TODO
	_Module.Write(data);
	if (_Module.WaitForResponse("SEND OK", 5000) == NULL) return false;	// TODO

	return true;
}

int WioLTE::SocketReceive(int connectId, byte* data, int dataSize)
{
	if (connectId > 11) return false;

	char* str = (char*)alloca(15 + 2 + 1);
	sprintf(str, "+QIURC: \"recv\",%d", connectId);
	if (_Module.WaitForResponse(str, 30000) == NULL) return -1;						// TODO

	char* str2 = (char*)alloca(8 + 2 + 1);
	sprintf(str2, "AT+QIRD=%d", connectId);
	_Module.WriteCommand(str2);
	const char* parameter;
	if ((parameter = _Module.WaitForResponse("+QIRD: ", 500, ModuleSerial::WFR_START_WITH)) == NULL) return false;
	int dataLength = atoi(&parameter[7]);
	if (dataLength > dataSize) return -1;
	if (_Module.Read(data, dataLength, 500) != dataLength) return -1;
	if (_Module.WaitForResponse("OK", 500) == NULL) return -1;							// TODO

	return dataLength;
}

int WioLTE::SocketReceive(int connectId, char* data, int dataSize)
{
	int dataLength = SocketReceive(connectId, (byte*)data, dataSize - 1);
	if (dataLength >= 0) data[dataLength] = '\0';

	return dataLength;
}

bool WioLTE::SocketClose(int connectId)
{
	if (connectId > 11) return false;

	char* str = (char*)alloca(11 + 2 + 1);
	sprintf(str, "AT+QICLOSE=%d", connectId);
	if (_Module.WriteCommandAndWaitForResponse(str, "OK", 30000) == NULL) return false;		// TODO

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
