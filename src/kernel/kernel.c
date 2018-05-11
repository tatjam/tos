#include <stdbool.h> /* C doesn't have booleans by default. */
#include <stddef.h>
#include <stdint.h>

#include <string.h>

#include <tty.h>
#include <asmutil.h>
#include <kio.h>

#include "arch/i386/pit.h"

#include "memory/pageutil.h"

#include "arch/i386/serial.h"

#include "arch/i386/tables/tables.h"
#include "multiboot/multiboot.h"

#include "memory/palloc.h"

void kernel_main(multiboot_info_t* mbd, unsigned int magic) 
{
	mbd = ((uint8_t*)mbd + 0xC0000000);

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

	ktty_putf("%a[TOS-BOOT]%a Serial Initialized (Using it now!)\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	gdt_prepare();

	klog("%a[TOS-BOOT]%a GDT loaded\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	page_init();
	
	klog("%a[TOS-BOOT]%a Loaded Basic Paging Tables\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);
		
	idt_prepare();	
	idt_configure();

	klog("%a[TOS-BOOT]%a Loaded IDT\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	pit_set_freq(100);

	palloc_init(mbd);
	
	klog("%a[TOS-BOOT]%a Finished Palloc\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);
	klog("%a[TOS-BOOT]%a Basic Boot Finished\n", VGA_BRIGHT(VGA_RED), VGA_GRAY);

	asm_sti();



	while(1)
	{
		;;
	}
}