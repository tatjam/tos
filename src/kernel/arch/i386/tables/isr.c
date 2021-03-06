#include "isr.h"
#include "../pit.h"


void isr_key(void)
{
	uint8_t scancode = asm_inb(0x60);
	keyb_interrupt(scancode);
}

void isr_irq0(void)
{
	pit_tick();
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


void isr_com1(void)
{
	// TODO: We can't test
	uint8_t byte = asm_inb(SERIAL_COM1);
	ktty_putc(byte);
}

void isr_com2(void)
{
	
}

void isr_pagefault(uint32_t code, uint32_t cr2)
{
	ktty_putf("\n%a[KERNEL_ERROR] PAGE FAULT: %a 0x%x\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, code);

	// Decompose and form error message

	bool pbit = code & 0x01;
	bool wbit = (code >> 1) & 0x01;
	bool ubit = (code >> 2) & 0x01;

	ktty_putf("\tBITS: [present?: %u] [read/write: %u] [kernel/user: %u] [cr2: %p]\n", pbit, wbit, ubit, cr2);

	while(1)
	{
		;;
	}
	
}