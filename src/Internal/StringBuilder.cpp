#include "../WioLTEConfig.h"
#include "StringBuilder.h"

#include <stdio.h>
#include <stdarg.h>

#define STRING_MAX_SIZE		(200)

StringBuilder::StringBuilder()
{
	_Buffer.push_back('\0');
}

void StringBuilder::Clear()
{
	_Buffer.clear();
	_Buffer.push_back('\0');
}

int StringBuilder::Length() const
{
	return _Buffer.size() - 1;
}

const char* StringBuilder::GetString() const
{
	return &_Buffer[0];
}

void StringBuilder::Write(const char* str)
{
	int i = Length();
	_Buffer.resize(_Buffer.size() + strlen(str));
	strcpy(&_Buffer[i], str);
}

void StringBuilder::Write(const char* str, int length)
{
	int i = Length();
	_Buffer.resize(_Buffer.size() + length);
	memcpy(&_Buffer[i], str, length);
	_Buffer[i + length] = '\0';
}

bool StringBuilder::WriteFormat(const char* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	char str[STRING_MAX_SIZE];
	int length = vsnprintf(str, sizeof (str), format, arglist);
	va_end(arglist);

	if (length < 0 || sizeof (str) - 1 <= length) return false;

	Write(str);

	return true;
}
