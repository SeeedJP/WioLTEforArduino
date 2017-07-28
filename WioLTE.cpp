#include <Arduino.h>
#include <stdio.h>
#include "wiolte-driver.h"

#define MODULE_RESPONSE_MAX_SIZE  (100)

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

static void DebugPrint(const char* data)
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
	SerialUSB.print("<- ");
	DebugPrint(str);

	_Serial->write(str);
}

void WioLTE::WriteCommand(const char* command)
{
	SerialUSB.print("<- ");
	DebugPrint(command);

	_Serial->write(command);
	_Serial->write('\r');
}

bool WioLTE::WaitForResponse(const char* response, long timeout)
{
	char data[MODULE_RESPONSE_MAX_SIZE];
	do {
		if (!ReadLine(data, sizeof(data), timeout)) return false;
		SerialUSB.print("-> ");
		DebugPrint(data);
	} while (strcmp(response, data) != 0);

	return true;
}

bool WioLTE::WaitForResponse(const char* response, char* parameter, int parameterSize, long timeout)
{
	char data[MODULE_RESPONSE_MAX_SIZE];
	int responseLength = strlen(response);
	do {
		if (!ReadLine(data, sizeof(data), timeout)) return false;
		SerialUSB.print("-> ");
		DebugPrint(data);
	} while (strncmp(response, data, responseLength) != 0);

	if (strlen(data) - responseLength + 1 > parameterSize) return false;
	strcpy(parameter, &data[responseLength]);

	return true;
}

bool WioLTE::WriteCommandAndWaitForResponse(const char* command, const char* response, long timeout)
{
	WriteCommand(command);
	return WaitForResponse(response, timeout);
}

bool WioLTE::Reset()
{
	digitalWrite(RESET_MODULE_PIN, LOW);
	delay(200);
	digitalWrite(RESET_MODULE_PIN, HIGH);
	delay(300);

	Stopwatch sw;
	sw.Start();
	while (!WaitForResponse("RDY", 100)) {
		SerialUSB.print(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	SerialUSB.println("");

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
		SerialUSB.print(".");
		if (sw.ElapsedMilliseconds() >= 5000) return false;
		delay(100);
	}
	SerialUSB.println("");

	sw.Start();
	while (!WaitForResponse("RDY", 100)) {
		SerialUSB.print(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	SerialUSB.println("");

	return true;
}

WioLTE::WioLTE() : _Serial(&Serial1)
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
	while (!WriteCommandAndWaitForResponse("AT", "OK", 500)) {
		SerialUSB.print(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	SerialUSB.println("");

	if (!WriteCommandAndWaitForResponse("ATE0", "OK", 500)) return false;
	if (!WriteCommandAndWaitForResponse("AT+QURCCFG=\"urcport\",\"usbat\"", "OK", 500)) return false;

	sw.Start();
	while (!WriteCommandAndWaitForResponse("AT+CPIN?", "OK", 5000)) {
		SerialUSB.print(".");
		if (sw.ElapsedMilliseconds() >= 10000) return false;
	}
	SerialUSB.println("");

	return true;
}

bool WioLTE::SendSMS(const char* dialNumber, const char* message)
{
	if (!WriteCommandAndWaitForResponse("AT+CMGF=1", "OK", 500)) return false;

	char* str = (char*)alloca(10 + strlen(dialNumber) + 1);
	sprintf(str, "AT+CMGS=\"%s\"", dialNumber);
	if (!WriteCommandAndWaitForResponse(str, "", 500)) return false;
	Write(message);
	Write("\x1a");
	if (!WaitForResponse("OK", 120000)) return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
