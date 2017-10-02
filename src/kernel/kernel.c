#include <stdbool.h> /* C doesn't have booleans by default. */
#include <stddef.h>
#include <stdint.h>

#include <string.h>

#include <tty.h>
#include <asmutil.h>
#include <kio.h>

#include "arch/i386/serial.h"

#include "arch/i386/tables/tables.h"
#include "multiboot/multiboot.h"

void kernel_main(multiboot_info_t* mbd, unsigned int magic) 
{
	
	asm_cli();

	ktty_init();
	ktty_clear();

	ktty_putf("%a[TOS-BOOT]%a VGA Initialized\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	ktty_putf("%a[TOS-BOOT]%a Checking multiboot (0x%x): ", VGA_BRIGHT(VGA_RED), VGA_GRAY, magic);

	if(magic == MULTIBOOT_BOOTLOADER_MAGIC)
	{
		ktty_putf("%aOK%a\n", VGA_BRIGHT(VGA_GREEN), VGA_GRAY);
	}
	else
	{
		ktty_putf("%aERROR%a\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);
	}

	serial_init();

	ktty_putf("%a[TOS-BOOT]%a Serial Initialized\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	gdt_prepare();

	ktty_putf("%a[TOS-BOOT]%a GDT loaded\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	idt_prepare();	
	idt_configure();

	ktty_putf("%a[TOS-BOOT]%a Loaded IDT. Enabling interrupts\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	asm_sti();

	ktty_putf("%a[TOS-BOOT]%a Basic Boot Finished\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);



	while(1)
	{
		;;
	}
}