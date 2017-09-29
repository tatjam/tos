#include <stdbool.h> /* C doesn't have booleans by default. */
#include <stddef.h>
#include <stdint.h>

#include <string.h>

#include <tty.h>
#include <asmutil.h>

#include "arch/i386/tables/tables.h"


void kernel_main(void) 
{
	
	asm_cli();

	ktty_init();
	ktty_clear();

	ktty_putf("%a[TOS-BOOT]%a VGA Initialized\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);


	ktty_putf("%a[TOS-BOOT]%a Loading GDT\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	gdt_prepare();

	ktty_putf("%a[TOS-BOOT]%a Loading IDT\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	idt_prepare();

	for(size_t i = 0; i < 256; i++)
	{
		idt_load_handler(i, isr_key_wrap);
	}

	ktty_putf("We alive");



	while(1)
	{
		asm_int();
		// Loop, waiting for interrupts to test...
		ktty_putf("Still alive!");
	}



}