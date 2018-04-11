#pragma once

#include <Arduino.h>

//#define WIOLTE_TYPE_JP_ES_V11	// Private version.
//#define WIOLTE_TYPE_JP_ES_V12	// Private version.
//#define WIOLTE_TYPE_V10		// 1st public version.
#define WIOLTE_TYPE_V11			// 2nd public version.
//#define WIOLTE_TYPE_V13		// 3rd public version.

#if defined WIOLTE_TYPE_JP_ES_V11
#define WIOLTE_SCHEMATIC_A
#elif defined WIOLTE_TYPE_JP_ES_V12 || defined WIOLTE_TYPE_V10 || defined WIOLTE_TYPE_V11 || defined WIOLTE_TYPE_V13
#define WIOLTE_SCHEMATIC_B
#endif

//#define WIOLTE_DEBUG
