#pragma once

#include "SerialAPI.h"
#include "Stopwatch.h"
#include <string>
#include <functional>

class WioLTE;

class AtSerial
{
private:
	SerialAPI* _Serial;
	WioLTE* _WioLTE;
	std::function<void()> _DoWorkInWaitForAvailable;

	bool ReadResponseInternal(const char* pattern, unsigned long timeout, std::string* response, int responseMaxLength);

public:
	AtSerial(SerialAPI* serial, WioLTE* wioLTE);

	void SetDoWorkInWaitForAvailableFunction(std::function<void()> func);

	bool WaitForAvailable(Stopwatch* sw, unsigned long timeout) const;

	void WriteBinary(const byte* data, int dataSize);
	bool ReadBinary(byte* data, int dataSize, unsigned long timeout);

	void WriteCommand(const char* command);
	bool ReadResponse(const char* pattern, unsigned long timeout, std::string* capture);
	bool WriteCommandAndReadResponse(const char* command, const char* pattern, unsigned long timeout, std::string* capture);

	bool ReadResponseQHTTPREAD(char* data, int dataSize, unsigned long timeout);

};
