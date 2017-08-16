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
// ModuleSerial

WioLTE::ModuleSerial::ModuleSerial()
{
}

void WioLTE::ModuleSerial::DiscardRead()
{
	while (SerialAvailable()) SerialRead();
}

bool WioLTE::ModuleSerial::WaitForAvailable(WioLTE::Stopwatch* sw, long timeout) const
{
	while (!SerialAvailable()) {
		if (sw->ElapsedMilliseconds() >= timeout) return false;
	}
	return true;
}

const char* WioLTE::ModuleSerial::ReadResponse()
{
	_LastResponse.clear();

	while (true) {
		// Wait for available.
		WioLTE::Stopwatch sw;
		sw.Start();
		if (!WaitForAvailable(&sw, 10)) return NULL;

		// Read byte.
		byte b = SerialRead();
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

bool WioLTE::ModuleSerial::ReadLine(char* data, int dataSize, long timeout)
{
	int dataIndex = 0;

	Stopwatch sw;
	while (dataIndex < dataSize - 1) {
		sw.Start();
		while (!SerialAvailable()) {
			if (sw.ElapsedMilliseconds() > timeout) {
				data[dataIndex] = '\0';
				return false; // Timeout.
			}
		}
		int c = SerialRead();
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

void WioLTE::ModuleSerial::Init()
{
	SerialInit();
}

void WioLTE::ModuleSerial::Write(const char* str)
{
	DEBUG_PRINT("<- ");
	DEBUG_PRINTLN(str);

	SerialWrite((const byte*)str, strlen(str));
}

void WioLTE::ModuleSerial::WriteCommand(const char* command)
{
	DEBUG_PRINT("<- ");
	DEBUG_PRINTLN(command);

	SerialWrite((const byte*)command, strlen(command));
	SerialWrite(CHAR_CR);
}

const char* WioLTE::ModuleSerial::WaitForResponse(const char* waitResponse, long timeout)
{
	WioLTE::Stopwatch sw;
	sw.Start();

	while (true) {
		if (!WaitForAvailable(&sw, timeout)) return NULL;

		const char* response = ReadResponse();

		if (response[0] == '\0') continue;

		if (strcmp(response, waitResponse) == 0) {
			DEBUG_PRINT("-> ");
			DEBUG_PRINTLN(response);
			return response;
		}

		DEBUG_PRINT("-> (");
		DEBUG_PRINT(response);
		DEBUG_PRINTLN(")");
	}
}

const char* WioLTE::ModuleSerial::WriteCommandAndWaitForResponse(const char* command, const char* response, long timeout)
{
	WriteCommand(command);
	return WaitForResponse(response, timeout);
}

bool WioLTE::ModuleSerial::WaitForResponse(const char* response, char* parameter, int parameterSize, long timeout)
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

////////////////////////////////////////////////////////////////////////////////////////
