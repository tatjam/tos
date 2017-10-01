#include <stdbool.h> /* C doesn't have booleans by default. */
#include <stddef.h>
#include <stdint.h>

#include <string.h>

#include <tty.h>
#include <asmutil.h>
#include "arch/i386/serial.h"

#include "arch/i386/tables/tables.h"


void kernel_main(void) 
{
	
	asm_cli();

	ktty_init();
	ktty_clear();

	ktty_putf("%a[TOS-BOOT]%a VGA Initialized\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	serial_init();

	ktty_putf("%a[TOS-BOOT]%a Loading GDT\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	gdt_prepare();

	ktty_putf("%a[TOS-BOOT]%a Loading IDT\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	idt_prepare();	pic_remap(PIC1_OFFSET, PIC2_OFFSET);

	for(size_t i = 0; i < 256; i++)
	{
		idt_load_handler(i, isr_unhandled_wrap);
	}

	idt_load_handler(PIC1_OFFSET + 1, isr_key_wrap);
	idt_load_handler(PIC1_OFFSET + 3, isr_com2_wrap);
	idt_load_handler(PIC1_OFFSET + 4, isr_com1_wrap);

	asm_sti();

	while(1)
	{
		;;
	}
}