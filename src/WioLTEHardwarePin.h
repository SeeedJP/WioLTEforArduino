#pragma once

class WioLTEHardwarePin
{
public:
	WioLTEHardwarePin() = delete;

public:
	static constexpr int MODULE_UART_TX_PIN = 2;	// Required - PA2	Note: Fixed to PA2 by ARDUINO_ARCH_STM32F4
	static constexpr int MODULE_UART_RX_PIN = 3;	// Required - PA3	Note: Fixed to PA3 by ARDUINO_ARCH_STM32F4
	static constexpr int PWR_KEY_PIN		= 36;	// Required - PC4
	static constexpr int RESET_MODULE_PIN	= 35;	// Required - PC3
	static constexpr int STATUS_PIN			= 31;	// Optional - PB15
	static constexpr int DTR_PIN			= 1;	// Optional - PA1
	static constexpr int WAKEUP_IN_PIN		= 32;	// Optional - PC0
	static constexpr int W_DISABLE_PIN		= 34;	// Optional - PC2
	static constexpr int AP_READY_PIN		= 33;	// Optional - PC1

	static constexpr int RGB_LED_PIN		= 17;	// Optional - PB1	Note: Fixed to PB1 by ARDUINO_ARCH_STM32

	static constexpr int MODULE_PWR_PIN		= 21;	// Optional - PB5
	static constexpr int ANT_PWR_PIN		= 28;	// Optional - PB12
	static constexpr int ENABLE_VCCB_PIN	= 26;	// Optional - PB10
	static constexpr int SD_POWR_PIN		= 15;	// Optional - PA15
	static constexpr int RGB_LED_PWR_PIN	= 8;	// Optional - PA8
};
