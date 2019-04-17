#if defined ARDUINO_ARCH_STM32

#include "WioSK6812.h"

#include <stm32f4xx_hal.h>

#define LED_GPIO_PORT		(GPIOB)
#define LED_GPIO_PIN		(GPIO_PIN_1)

#define LED_GPIO_HIGH()		(LED_GPIO_PORT->BSRR = LED_GPIO_PIN)
#define LED_GPIO_LOW()		(LED_GPIO_PORT->BSRR = (uint32_t)LED_GPIO_PIN << 16U)

extern "C" void DelayLoop(int32_t iterations);

asm volatile (
	".syntax unified\n\t"
	".arch armv7-m\n\t"
	".thumb\n\t"
	".global  DelayLoop\n\t"
	"@AREA ||i.DelayLoop||, CODE, READONLY @ void DelayLoop(UINT32 count)\n\t"
	".section SectionForCodeDelayLoop, \"ax\", %progbits\n\t"
	".thumb_func\n\t"
	"DelayLoop:\n\t"
	"subs    r0, r0, #3          @@ 1 cycle\n\t"
	"bgt     DelayLoop           @@ 3 cycles taken, 1 cycle not taken.\n\t"
	"bx lr                       @@ 3 cycles\n\t"
	);

void WioSK6812::SetBit(bool on)
{
	if (!on) {
		LED_GPIO_HIGH();
		DelayLoop(50);
		LED_GPIO_LOW();
		DelayLoop(150);
	}
	else {
		LED_GPIO_HIGH();
		DelayLoop(100);
		LED_GPIO_LOW();
		DelayLoop(100);
	}
}

void WioSK6812::SetByte(uint8_t val)
{
	for (int i = 0; i < 8; i++) {
		SetBit(val & (1 << (7 - i)));
	}
}

void WioSK6812::Reset()
{
	LED_GPIO_LOW();
	DelayLoop(13333);
}

void WioSK6812::SetSingleLED(uint8_t r, uint8_t g, uint8_t b)
{
	SetByte(g);
	SetByte(r);
	SetByte(b);
}

#endif // ARDUINO_ARCH_STM32
