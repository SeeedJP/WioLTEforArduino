#include <Arduino.h>
#include <stdio.h>
#include "wiolte-driver.h"

#define DEBUG

#define MODULE_RESPONSE_MAX_SIZE	(100)

#ifdef DEBUG
#define DEBUG_PRINT(str)			SerialUSB.print(str)
#define DEBUG_PRINTLN(str)			SerialUSB.println(str)
#define DEBUG_PRINTLN_DUMP(data)	DebugPrintlnDump(data)
#else
#define DEBUG_PRINT(str)
#define DEBUG_PRINTLN(str)
#define DEBUG_PRINTLN_DUMP(data)
#endif

#define CHAR_CR (0x0d)
#define CHAR_LF (0x0a)

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

static void DebugPrintlnDump(const char* data)
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
// WioLTE

void WioLTE::DiscardRead()
{
	while (_Serial->available()) _Serial->read();
}

bool WioLTE::WaitForAvailable(WioLTE::Stopwatch* sw, long timeout)
{
	while (!_Serial->available()) {
		if (sw->ElapsedMilliseconds() >= timeout) return false;
	}
	return true;
}

const char* WioLTE::ReadResponse()
{
	_LastResponse.clear();

	while (true) {
		// Wait for available.
		WioLTE::Stopwatch sw;
		sw.Start();
		if (!WaitForAvailable(&sw, 10)) return NULL;

		// Read byte.
		int b = _Serial->read();
		_LastResponse.push_back(b);

		// Is received delimiter?
		int responseSize = _LastResponse.size();
		if (responseSize >= 2 && _LastResponse[responseSize - 2] == CHAR_CR && _LastResponse[responseSize - 1] == CHAR_LF) {
			_LastResponse.pop_back();
			*_LastResponse.rbegin() = '\0';
			return &_LastResponse[0];
		}
	}
}

bool WioLTE::ReadLine(char* data, int dataSize, long timeout)
{
	int dataIndex = 0;

	Stopwatch sw;
	while (dataIndex < dataSize - 1) {
		sw.Start();
		while (!_Serial->available()) {
			if (sw.ElapsedMilliseconds() > timeout) {
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

void WioLTE::Write(const char* str)
{
	DEBUG_PRINT("<- ");
	DEBUG_PRINTLN_DUMP(str);

	_Serial->write(str);
}

void WioLTE::WriteCommand(const char* command)
{
	DEBUG_PRINT("<- ");
	DEBUG_PRINTLN_DUMP(command);

	_Serial->write(command);
	_Serial->write('\r');
}

const char* WioLTE::WaitForResponse(const char* waitResponse, long timeout)
{
	WioLTE::Stopwatch sw;
	sw.Start();

	while (true) {
		if (!WaitForAvailable(&sw, timeout)) return NULL;

		const char* response = ReadResponse();
		DEBUG_PRINT("-> ");
		DEBUG_PRINTLN_DUMP(response);

		if (strcmp(response, waitResponse) == 0) return response;
	}
}

const char* WioLTE::WriteCommandAndWaitForResponse(const char* command, const char* response, long timeout)
{
	WriteCommand(command);
	return WaitForResponse(response, timeout);
}

bool WioLTE::WaitForResponse(const char* response, char* parameter, int parameterSize, long timeout)
{
	char data[MODULE_RESPONSE_MAX_SIZE];
	int responseLength = strlen(response);
	do {
		if (!ReadLine(data, sizeof(data), timeout)) return false;
		DEBUG_PRINT("-> ");
		DEBUG_PRINTLN_DUMP(data);
	} while (strncmp(response, data, responseLength) != 0);

	if (strlen(data) - responseLength + 1 > parameterSize) return false;
	strcpy(parameter, &data[responseLength]);

	return true;
}

bool WioLTE::Reset()
{
	digitalWrite(RESET_MODULE_PIN, LOW);
	delay(200);
	digitalWrite(RESET_MODULE_PIN, HIGH);
	delay(300);

	Stopwatch sw;
	sw.Start();
	while (WaitForResponse("RDY", 100) == NULL) {
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
	while (WaitForResponse("RDY", 100) == NULL) {
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	DEBUG_PRINTLN("");

	return true;
}

WioLTE::WioLTE() : _Serial(&Serial1), _Led(1, RGB_LED_PIN)
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
  
	_Serial->begin(115200);
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
	while (WriteCommandAndWaitForResponse("AT", "OK", 500) == NULL) {
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	DEBUG_PRINTLN("");

	if (WriteCommandAndWaitForResponse("ATE0", "OK", 500) == NULL) return false;
	if (WriteCommandAndWaitForResponse("AT+QURCCFG=\"urcport\",\"usbat\"", "OK", 500) == NULL) return false;

	sw.Start();
	while (WriteCommandAndWaitForResponse("AT+CPIN?", "OK", 5000) == NULL) {	// TODO
		DEBUG_PRINT(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	DEBUG_PRINTLN("");

	return true;
}

bool WioLTE::SendSMS(const char* dialNumber, const char* message)
{
	if (WriteCommandAndWaitForResponse("AT+CMGF=1", "OK", 500) == NULL) return false;

	char* str = (char*)alloca(9 + strlen(dialNumber) + 1 + 1);
	sprintf(str, "AT+CMGS=\"%s\"", dialNumber);
	if (WriteCommandAndWaitForResponse(str, "", 500) == NULL) return false;
	Write(message);
	Write("\x1a");
	if (WaitForResponse("OK", 120000) == NULL) return false;

	return true;
}

bool WioLTE::Activate(const char* accessPointName, const char* userName, const char* password)
{
	char parameter[MODULE_RESPONSE_MAX_SIZE];

	WriteCommand("AT+CREG?");
	if (!WaitForResponse("+CREG: ", parameter, sizeof(parameter), 500)) return false;
	if (strcmp(parameter, "0,1") != 0) return false;	// TODO
	if (WaitForResponse("OK", 500) == NULL) return false;

	WriteCommand("AT+CGREG?");
	if (!WaitForResponse("+CGREG: ", parameter, sizeof(parameter), 500)) return false;
	if (strcmp(parameter, "0,1") != 0) return false;	// TODO
	if (WaitForResponse("OK", 500) == NULL) return false;

	WriteCommand("AT+CEREG?");
	if (!WaitForResponse("+CEREG: ", parameter, sizeof(parameter), 500)) return false;
	if (strcmp(parameter, "0,1") != 0) return false;	// TODO
	if (WaitForResponse("OK", 500) == NULL) return false;

	char* str = (char*)alloca(15 + strlen(accessPointName) + 3 + strlen(userName) + 3 + strlen(password) + 3 + 1);
	sprintf(str, "AT+QICSGP=1,1,\"%s\",\"%s\",\"%s\",1", accessPointName, userName, password);
	if (WriteCommandAndWaitForResponse(str, "OK", 500) == NULL) return false;

	if (WriteCommandAndWaitForResponse("AT+QIACT=1", "OK", 150000) == NULL) return false;

	if (WriteCommandAndWaitForResponse("AT+QIACT?", "OK", 150000) == NULL) return false;

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
	if (WriteCommandAndWaitForResponse(str, "OK", 150000) == NULL) return -1;
	if (WaitForResponse("+QIOPEN: 0,0", 150000) == NULL) return -1;		// TODO

	return 0;
}

bool WioLTE::SocketSend(int connectId, const char* data)
{
	if (connectId > 11) return false;
	int dataLength = strlen(data);
	if (dataLength > 1460) return false;

	char* str = (char*)alloca(10 + 2 + 1 + 4 + 1);
	sprintf(str, "AT+QISEND=%d,%d", connectId, dataLength);
	WriteCommand(str);
	char recv[MODULE_RESPONSE_MAX_SIZE];
	if (!ReadLine(recv, sizeof(recv), 2000)) return false;						// TODO
	if (strcmp(recv, "") != 0) return false;
	while (!_Serial->available());
	if (_Serial->read() != '>') return false;
	while (!_Serial->available());
	if (_Serial->read() != ' ') return false;

	if (WriteCommandAndWaitForResponse(data, "SEND OK", 5000) == NULL) return false;	// TODO

	return true;
}

int WioLTE::SocketReceive(int connectId, byte* data, int dataSize)
{
	if (connectId > 11) return false;

	char* str = (char*)alloca(15 + 2 + 1);
	sprintf(str, "+QIURC: \"recv\",%d", connectId);
	if (WaitForResponse(str, 30000) == NULL) return -1;						// TODO

	char* str2 = (char*)alloca(8 + 2 + 1);
	sprintf(str2, "AT+QIRD=%d", connectId);
	WriteCommand(str2);
	char parameter[MODULE_RESPONSE_MAX_SIZE];
	if (!WaitForResponse("+QIRD: ", parameter, sizeof(parameter), 500)) return false;
	int dataLength = atoi(parameter);
	if (dataLength > dataSize) return -1;
	if (_Serial->readBytes(data, dataLength) != dataLength) return -1;
	if (WaitForResponse("OK", 500) == NULL) return -1;							// TODO

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
	if (WriteCommandAndWaitForResponse(str, "OK", 30000) == NULL) return false;		// TODO

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
