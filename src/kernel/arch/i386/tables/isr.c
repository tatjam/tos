#include "isr.h"



void isr_key(void)
{
	uint8_t scancode = asm_inb(0x60);
	ktty_putc(scancode);
}


void isr_unhandled(void)
{
	// Do nothing, we could log an error or something
}

void isr_generic_return(uint8_t inum)
{
	// Check for spurious interrupt
	pic_send_eoi(inum);
}

uint8_t isr_is_spurious(uint8_t inum)
{
	return 0;
}