#pragma once

#include "WioLTEConfig.h"

#if __has_include("WioLTEHardwarePinCustom.h")
#include "WioLTEHardwarePinCustom.h"
#else
#include "WioLTEHardwarePin.h"
#endif

#include <Wire.h>

#if defined ARDUINO_ARCH_STM32F4

extern HardwareSerial& SerialModule;
extern HardwareSerial& SerialUART;
extern TwoWire& WireI2C;

#elif defined ARDUINO_ARCH_STM32

extern HardwareSerial SerialModule;
extern HardwareSerial& SerialUART;
extern TwoWire& WireI2C;

#endif
