#include "../WioLTEConfig.h"
#include "Debug.h"

#ifdef WIO_DEBUG

void Debug::Print(const char* str)
{
	for (int i = 0; i < strlen(str); i++) SerialUSB.print(str[i]);
}

void Debug::Println (const char* str)
{
	Print(str);
	Print("\r\n");
}

#endif // WIO_DEBUG
