#include <Arduino.h>
#include "WioLTEforArduino.h"

////////////////////////////////////////////////////////////////
// Internal firmware declarations.

extern "C"
{
	typedef struct dac_reg_map {
		__io uint32 CR;      /**< Control register */
		__io uint32 SWTRIGR; /**< Software trigger register */
		__io uint32 DHR12R1; /**< Channel 1 12-bit right-aligned data
								holding register */
		__io uint32 DHR12L1; /**< Channel 1 12-bit left-aligned data
								holding register */
		__io uint32 DHR8R1;  /**< Channel 1 8-bit left-aligned data
								holding register */
		__io uint32 DHR12R2; /**< Channel 2 12-bit right-aligned data
								holding register */
		__io uint32 DHR12L2; /**< Channel 2 12-bit left-aligned data
								holding register */
		__io uint32 DHR8R2;  /**< Channel 2 8-bit left-aligned data
								holding register */
		__io uint32 DHR12RD; /**< Dual DAC 12-bit right-aligned data
								holding register */
		__io uint32 DHR12LD; /**< Dual DAC 12-bit left-aligned data
								holding register */
		__io uint32 DHR8RD;  /**< Dual DAC 8-bit right-aligned data holding
								register */
		__io uint32 DOR1;    /**< Channel 1 data output register */
		__io uint32 DOR2;    /**< Channel 2 data output register */
	} dac_reg_map;

	typedef struct dac_dev {
		dac_reg_map *regs; /**< Register map */
	} dac_dev;

	extern const dac_dev *DAC;

	#define DAC_CH1 0x1
	#define DAC_CH2 0x2

	void dac_init(const dac_dev *dev, uint32 flags);

	void dac_write_channel(const dac_dev *dev, uint8 channel, uint16 val);
	void dac_enable_channel(const dac_dev *dev, uint8 channel);
	void dac_disable_channel(const dac_dev *dev, uint8 channel);
	void dac_enable_buffer(const dac_dev *dev, uint32 flags, int status);
}

////////////////////////////////////////////////////////////////

void WioLTEDac::Init(const WioLTEDac::DACChannel enableChannels)
{
	uint32 flags = 0;
	if (enableChannels & WioLTEDac::DAC1)
	{
		flags |= DAC_CH1;
	}
	if (enableChannels & WioLTEDac::DAC2)
	{
		flags |= DAC_CH2;
	}
	dac_init(DAC, flags);
}

void WioLTEDac::Write(const WioLTEDac::DACChannel channel, const uint16_t value)
{
	if (channel & WioLTEDac::DAC1)
	{
		dac_write_channel(DAC, DAC_CH1, value);
	}
	if (channel & WioLTEDac::DAC2)
	{
		dac_write_channel(DAC, DAC_CH2, value);
	}
}
