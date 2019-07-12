#pragma once

#include "WioLTEConfig.h"
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
