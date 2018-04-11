#include "../WioLTEConfig.h"
#include "Debug.h"

#ifdef WIOLTE_DEBUG

void Debug::Print(const char* str)
{
	for (int i = 0; i < strlen(str); i++) SerialUSB.print(str[i]);
}

void Debug::Println (const char* str)
{
	Print(str);
	Print("\r\n");
}

#endif // WIOLTE_DEBUG
