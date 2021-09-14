#include "WioLTEConfig.h"
#include "WioLTEHardware.h"

#if defined ARDUINO_ARCH_STM32F4

HardwareSerial& SerialModule = Serial1;

HardwareSerial& SerialUART = Serial;
TwoWire& WireI2C = Wire;

#elif defined ARDUINO_ARCH_STM32

HardwareSerial SerialModule(WioLTEHardwarePin::MODULE_UART_RX_PIN, WioLTEHardwarePin::MODULE_UART_TX_PIN);

HardwareSerial& SerialUART = Serial;
TwoWire& WireI2C = Wire;

#endif
