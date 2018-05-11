#include "pit.h"

uint32_t pit_counter = 0;

void pit_tick()
{
	pit_counter++;
}

void pit_set_freq(uint32_t freq)
{
	// 1193180 is a magic PIT number
	uint32_t div = (uint32_t)1193180 / freq;
	if(div > 0xFFFF)
	{
		klog("%a[TOS-PIT]%a Invalid freq (%u), divisor: (%u) > 16bits\n", VGA_BRIGHT(VGA_RED), VGA_GRAY,
			freq, div);
	}
	pit_set_divider(div);
}

void pit_set_divider(uint16_t div)
{
	// Command
	asm_outb(0x43, 0x36);

	// Divide the divisor into 2 bytes
	uint8_t low = (uint8_t)(div & 0xFF);
	uint8_t high = (uint8_t)((div>>8) & 0xFF);

	asm_outb(0x40, low);
	asm_outb(0x40, high);
}