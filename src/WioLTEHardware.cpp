#include "WioLTEConfig.h"
#include "WioLTEHardware.h"

#if defined ARDUINO_ARCH_STM32F4

HardwareSerial& SerialModule = Serial1;

HardwareSerial& SerialUART = Serial;
TwoWire& WireI2C = Wire;

#elif defined ARDUINO_ARCH_STM32

#define PINNAME_TO_PIN(port, pin) ((port - 'A') * 16 + pin)
#define MODULE_UART_TX_PIN  PINNAME_TO_PIN('A', 2)	// out
#define MODULE_UART_RX_PIN  PINNAME_TO_PIN('A', 3)	// in

HardwareSerial SerialModule(MODULE_UART_RX_PIN, MODULE_UART_TX_PIN);

HardwareSerial& SerialUART = Serial1;
TwoWire& WireI2C = Wire;

#endif
