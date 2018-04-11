#pragma once

#ifdef WIOLTE_DEBUG

#define DEBUG_PRINT(str)			Debug::Print(str)
#define DEBUG_PRINTLN(str)			Debug::Println(str)

class Debug
{
public:
	static void Print(const char* str);
	static void Println(const char* str);

};

#else

#define DEBUG_PRINT(str)
#define DEBUG_PRINTLN(str)

#endif // WIOLTE_DEBUG
