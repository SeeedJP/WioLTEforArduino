#include <Arduino.h>
#include <stdio.h>
#include "WioLTEforArduino.h"

//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(str)			DebugPrint(str)
#define DEBUG_PRINTLN(str)			DebugPrintln(str)
static void DebugPrint(const char* str)
{
	for (int i = 0; i < strlen(str); i++) SerialUSB.print(str[i]);
}
static void DebugPrintln(const char* str)
{
	DebugPrint(str);
	DebugPrint(" (");
	char str2[100];
	sprintf(str2, "%lu", (unsigned long)millis());
	DebugPrint(str2);
	DebugPrint(")");
	DebugPrint("\r\n");
}
#else
#define DEBUG_PRINT(str)
#define DEBUG_PRINTLN(str)
#endif

#define CHAR_CR (0x0d)
#define CHAR_LF (0x0a)

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

int WioLTE::ModuleSerial::Read(byte* data, int dataSize)
{
	for (int i = 0; i < dataSize; i++) {
		// Wait for available.
		WioLTE::Stopwatch sw;
		sw.Start();
		if (!WaitForAvailable(&sw, 10)) return i;

		// Read byte.
		data[i] = SerialRead();
	}

	return dataSize;
}

const char* WioLTE::ModuleSerial::ReadResponse(const char* match)
{
	int matchSize;
	if (match != NULL) matchSize = strlen(match);

	_LastResponse.clear();

	while (true) {
		// Wait for available.
		WioLTE::Stopwatch sw;
		sw.Start();
		if (!WaitForAvailable(&sw, 10)) return NULL;

		// Read byte.
		byte b = SerialRead();
		_LastResponse.push_back(b);
		int responseSize = _LastResponse.size();

		// Is received delimiter?
		if (responseSize >= 2 && _LastResponse[responseSize - 2] == CHAR_CR && _LastResponse[responseSize - 1] == CHAR_LF) {
			_LastResponse.pop_back();
			*_LastResponse.rbegin() = '\0';
			return &_LastResponse[0];
		}

		// Is match string?
		if (match != NULL && responseSize == matchSize && memcmp(&_LastResponse[0], match, matchSize) == 0) {
			_LastResponse.push_back('\0');
			return &_LastResponse[0];
		}
	}
}

void WioLTE::ModuleSerial::Init()
{
	SerialInit();
}

void WioLTE::ModuleSerial::Write(const byte* data, int dataSize)
{
	DEBUG_PRINTLN("<- (binary)");

	SerialWrite(data, dataSize);
}

void WioLTE::ModuleSerial::Write(const char* str)
{
	DEBUG_PRINT("<- ");
	DEBUG_PRINTLN(str);

	SerialWrite((const byte*)str, strlen(str));
}

int WioLTE::ModuleSerial::Read(byte* data, int dataSize, long timeout)
{
	WioLTE::Stopwatch sw;
	sw.Start();

	if (!WaitForAvailable(&sw, timeout)) return 0;

	return Read(data, dataSize);
}

void WioLTE::ModuleSerial::WriteCommand(const char* command)
{
	DEBUG_PRINT("<- ");
	DEBUG_PRINTLN(command);

	SerialWrite((const byte*)command, strlen(command));
	SerialWrite(CHAR_CR);
}

const char* WioLTE::ModuleSerial::WaitForResponse(const char* waitResponse, long timeout, const char* waitPattern, WaitForResponseFlag waitPatternFlag)
{
	WioLTE::Stopwatch sw;
	sw.Start();

	while (true) {
		if (!WaitForAvailable(&sw, timeout)) return NULL;

		const char* response = ReadResponse(waitPatternFlag & WFR_WITHOUT_DELIM ? waitPattern : NULL);

		if (!(waitPatternFlag & WFR_GET_NULL_STRING) && response[0] == '\0') continue;

		// waitResponse?
		if (waitResponse != NULL) {
			if (strcmp(response, waitResponse) == 0) {
				DEBUG_PRINT("-> ");
				DEBUG_PRINTLN(response);
				return response;
			}
		}

		// pattern?
		if (waitPattern != NULL) {
			if (waitPattern[0] == '\0') {
				DEBUG_PRINT("-> ");
				DEBUG_PRINTLN(response);
				return response;
			} else if (waitPatternFlag & WFR_START_WITH) {
				if (strncmp(response, waitPattern, strlen(waitPattern)) == 0) {
					DEBUG_PRINT("-> ");
					DEBUG_PRINTLN(response);

					return waitPatternFlag & WFR_REMOVE_START_WITH ? &response[strlen(waitPattern)] : response;
				}
			} else if (strcmp(response, waitPattern) == 0) {
				DEBUG_PRINT("-> ");
				DEBUG_PRINTLN(response);

				return waitPatternFlag & WFR_REMOVE_START_WITH ? &response[strlen(waitPattern)] : response;
			}
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

////////////////////////////////////////////////////////////////////////////////////////
