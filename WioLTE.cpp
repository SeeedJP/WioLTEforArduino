#include <Arduino.h>
#include <stdio.h>
#include <limits.h>
#include "wiolte-driver.h"

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(str)			SerialUSB.print(str)
#define DEBUG_PRINTLN(str)			SerialUSB.println(str)
#else
#define DEBUG_PRINT(str)
#define DEBUG_PRINTLN(str)
#endif

#define CONNECT_ID_NUM				(12)
#define POLLING_INTERVAL			(100)

#define LINEAR_SCALE(val, inMin, inMax, outMin, outMax)	(((val) - (inMin)) / ((inMax) - (inMin)) * ((outMax) - (outMin)) + (outMin))

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
	if (_Module.WriteCommandAndWaitForResponse("AT+QURCCFG=\"urcport\",\"uart1\"", "OK", 500) == NULL) return false;

	sw.Start();
	while (true) {
		_Module.WriteCommand("AT+CPIN?");
		const char* response = _Module.WaitForResponse("OK", 5000, "+CME ERROR: ", ModuleSerial::WFR_START_WITH);
		if (response == NULL) return false;
		if (strcmp(response, "OK") == 0) break;
		if (sw.ElapsedMilliseconds() >= 10000) return false;
		delay(POLLING_INTERVAL);
	}

	return true;
}

int WioLTE::GetReceivedSignalStrength()
{
	const char* parameter;

	_Module.WriteCommand("AT+CSQ");
	if ((parameter = _Module.WaitForResponse(NULL, 500, "+CSQ: ", (ModuleSerial::WaitForResponseFlag)(ModuleSerial::WFR_START_WITH | ModuleSerial::WFR_REMOVE_START_WITH))) == NULL) return INT_MIN;

	ArgumentParser parser;
	parser.Parse(parameter);
	if (parser.Size() != 2) return INT_MIN;
	int rssi = atoi(parser[0]);

	if (_Module.WaitForResponse("OK", 500) == NULL) return INT_MIN;

	if (rssi == 0) return -113;
	else if (rssi == 1) return -111;
	else if (2 <= rssi && rssi <= 30) return LINEAR_SCALE((double)rssi, 2, 30, -109, -53);
	else if (rssi == 31) return -51;
	else if (rssi == 99) return -999;
	else if (rssi == 100) return -116;
	else if (rssi == 101) return -115;
	else if (102 <= rssi && rssi <= 190) return LINEAR_SCALE((double)rssi, 102, 190, -114, -26);
	else if (rssi == 191) return -25;
	else if (rssi == 199) return -999;
	
	return -999;
}

bool WioLTE::GetTime(struct tm* tim)
{
	const char* parameter;

	_Module.WriteCommand("AT+CCLK?");
	if ((parameter = _Module.WaitForResponse(NULL, 500, "+CCLK: ", (ModuleSerial::WaitForResponseFlag)(ModuleSerial::WFR_START_WITH | ModuleSerial::WFR_REMOVE_START_WITH))) == NULL) return false;
	if (_Module.WaitForResponse("OK", 500) == NULL) return false;

	if (strlen(parameter) != 19) return false;
	if (parameter[0] != '"') return false;
	if (parameter[3] != '/') return false;
	if (parameter[6] != '/') return false;
	if (parameter[9] != ',') return false;
	if (parameter[12] != ':') return false;
	if (parameter[15] != ':') return false;
	if (parameter[18] != '"') return false;

	int yearOffset = atoi(&parameter[1]);
	tim->tm_year = (yearOffset >= 80 ? 1900 : 2000) + yearOffset;
	tim->tm_mon = atoi(&parameter[4]) - 1;
	tim->tm_mday = atoi(&parameter[7]);
	tim->tm_hour = atoi(&parameter[10]);
	tim->tm_min = atoi(&parameter[13]);
	tim->tm_sec = atoi(&parameter[16]);
	tim->tm_wday = 0;
	tim->tm_yday = 0;
	tim->tm_isdst = -1;

	return true;
}

bool WioLTE::SendSMS(const char* dialNumber, const char* message)
{
	if (_Module.WriteCommandAndWaitForResponse("AT+CMGF=1", "OK", 500) == NULL) return false;

	char* str = (char*)alloca(9 + strlen(dialNumber) + 1 + 1);
	sprintf(str, "AT+CMGS=\"%s\"", dialNumber);
	_Module.WriteCommand(str);
	if (_Module.WaitForResponse(NULL, 500, "> ", ModuleSerial::WFR_WITHOUT_DELIM) == NULL) return false;
	_Module.Write(message);
	_Module.Write("\x1a");
	if (_Module.WaitForResponse("OK", 120000) == NULL) return false;

	return true;
}

bool WioLTE::Activate(const char* accessPointName, const char* userName, const char* password)
{
	const char* parameter;

	_Module.WriteCommand("AT+CREG?");
	if ((parameter = _Module.WaitForResponse(NULL, 500, "+CREG: ", (ModuleSerial::WaitForResponseFlag)(ModuleSerial::WFR_START_WITH | ModuleSerial::WFR_REMOVE_START_WITH))) == NULL) return false;
	if (strcmp(parameter, "0,1") != 0 && strcmp(parameter, "0,3") != 0) return false;
	if (_Module.WaitForResponse("OK", 500) == NULL) return false;

	_Module.WriteCommand("AT+CGREG?");
	if ((parameter = _Module.WaitForResponse(NULL, 500, "+CGREG: ", (ModuleSerial::WaitForResponseFlag)(ModuleSerial::WFR_START_WITH | ModuleSerial::WFR_REMOVE_START_WITH))) == NULL) return false;
	if (strcmp(parameter, "0,1") != 0) return false;
	if (_Module.WaitForResponse("OK", 500) == NULL) return false;

	_Module.WriteCommand("AT+CEREG?");
	if ((parameter = _Module.WaitForResponse(NULL, 500, "+CEREG: ", (ModuleSerial::WaitForResponseFlag)(ModuleSerial::WFR_START_WITH | ModuleSerial::WFR_REMOVE_START_WITH))) == NULL) return false;
	if (strcmp(parameter, "0,1") != 0) return false;
	if (_Module.WaitForResponse("OK", 500) == NULL) return false;

	char* str = (char*)alloca(15 + strlen(accessPointName) + 3 + strlen(userName) + 3 + strlen(password) + 3 + 1);
	sprintf(str, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1", accessPointName, userName, password);
	if (_Module.WriteCommandAndWaitForResponse(str, "OK", 500) == NULL) return false;

	if (_Module.WriteCommandAndWaitForResponse("AT+QIACT=1", "OK", 150000) == NULL) return false;

	if (_Module.WriteCommandAndWaitForResponse("AT+QIACT?", "OK", 150000) == NULL) return false;

	return true;
}

bool WioLTE::SyncTime(const char* host)
{
	const char* parameter;

	char* str = (char*)alloca(11 + strlen(host) + 1 + 1);
	sprintf(str, "AT+QNTP=1,\"%s\"", host);
	if (_Module.WriteCommandAndWaitForResponse(str, "OK", 500) == NULL) return false;
	if ((parameter = _Module.WaitForResponse(NULL, 125000, "+QNTP: ", (ModuleSerial::WaitForResponseFlag)(ModuleSerial::WFR_START_WITH | ModuleSerial::WFR_REMOVE_START_WITH))) == NULL) return false;

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

	bool connectIdUsed[CONNECT_ID_NUM];
	for (int i = 0; i < CONNECT_ID_NUM; i++) connectIdUsed[i] = false;

	_Module.WriteCommand("AT+QISTATE?");
	const char* response;
	ArgumentParser parser;
	do {
		if ((response = _Module.WaitForResponse("OK", 10000, "+QISTATE: ", ModuleSerial::WFR_START_WITH)) == NULL) return -1;
		if (strncmp(response, "+QISTATE: ", 10) == 0) {
			parser.Parse(&response[10]);
			if (parser.Size() >= 1) {
				int connectId = atoi(parser[0]);
				if (connectId < 0 || CONNECT_ID_NUM <= connectId) return -1;
				connectIdUsed[connectId] = true;
			}
		}

	} while (strcmp(response, "OK") != 0);

	int connectId;
	for (connectId = 0; connectId < CONNECT_ID_NUM; connectId++) {
		if (!connectIdUsed[connectId]) break;
	}
	if (connectId >= CONNECT_ID_NUM) return -1;

	char* str = (char*)alloca(12 + 2 + 2 + 3 + 3 + strlen(host) + 2 + 5 + 1);
	sprintf(str, "AT+QIOPEN=1,%d,\"%s\",\"%s\",%d", connectId, typeStr, host, port);
	if (_Module.WriteCommandAndWaitForResponse(str, "OK", 150000) == NULL) return -1;
	char* str2 = (char*)alloca(9 + 2 + 2 + 1);
	sprintf(str2, "+QIOPEN: %d,0", connectId);
	if (_Module.WaitForResponse(str2, 150000) == NULL) return -1;

	return connectId;
}

bool WioLTE::SocketSend(int connectId, const byte* data, int dataSize)
{
	if (connectId >= CONNECT_ID_NUM) return false;
	if (dataSize > 1460) return false;

	char* str = (char*)alloca(10 + 2 + 1 + 4 + 1);
	sprintf(str, "AT+QISEND=%d,%d", connectId, dataSize);
	_Module.WriteCommand(str);
	if (_Module.WaitForResponse(NULL, 500, "> ", ModuleSerial::WFR_WITHOUT_DELIM) == NULL) return false;
	_Module.Write(data, dataSize);
	if (_Module.WaitForResponse("SEND OK", 5000) == NULL) return false;

	return true;
}

bool WioLTE::SocketSend(int connectId, const char* data)
{
	return SocketSend(connectId, (const byte*)data, strlen(data));
}

int WioLTE::SocketReceive(int connectId, byte* data, int dataSize)
{
	if (connectId >= CONNECT_ID_NUM) return -1;

	char* str2 = (char*)alloca(8 + 2 + 1);
	sprintf(str2, "AT+QIRD=%d", connectId);
	_Module.WriteCommand(str2);
	const char* parameter;
	if ((parameter = _Module.WaitForResponse(NULL, 500, "+QIRD: ", ModuleSerial::WFR_START_WITH)) == NULL) return -1;
	int dataLength = atoi(&parameter[7]);
	if (dataLength >= 1) {
		if (dataLength > dataSize) return -1;
		if (_Module.Read(data, dataLength, 500) != dataLength) return -1;
	}
	if (_Module.WaitForResponse("OK", 500) == NULL) return -1;

	return dataLength;
}

int WioLTE::SocketReceive(int connectId, char* data, int dataSize)
{
	int dataLength = SocketReceive(connectId, (byte*)data, dataSize - 1);
	if (dataLength >= 0) data[dataLength] = '\0';

	return dataLength;
}

int WioLTE::SocketReceive(int connectId, byte* data, int dataSize, long timeout)
{
	Stopwatch sw;
	sw.Start();
	int dataLength;
	while ((dataLength = SocketReceive(connectId, data, dataSize)) == 0) {
		if (sw.ElapsedMilliseconds() >= timeout) return 0;
		delay(POLLING_INTERVAL);
	}
	return dataLength;
}

int WioLTE::SocketReceive(int connectId, char* data, int dataSize, long timeout)
{
	Stopwatch sw;
	sw.Start();
	int dataLength;
	while ((dataLength = SocketReceive(connectId, data, dataSize)) == 0) {
		if (sw.ElapsedMilliseconds() >= timeout) return 0;
		delay(POLLING_INTERVAL);
	}
	return dataLength;
}

bool WioLTE::SocketClose(int connectId)
{
	if (connectId >= CONNECT_ID_NUM) return false;

	char* str = (char*)alloca(11 + 2 + 1);
	sprintf(str, "AT+QICLOSE=%d", connectId);
	if (_Module.WriteCommandAndWaitForResponse(str, "OK", 10000) == NULL) return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
