#include "palloc.h"

typedef struct multiboot_memory_map {
	uint size;
	uint64_t base_addr;
	uint64_t length;
	uint type;
} multiboot_memory_map_t;
 

void palloc_init(multiboot_info_t* mbt)
{

	ktty_putf("%a[TOS-PALLOC]%a Reading mbt at: 0x%p\n", VGA_BRIGHT(VGA_RED), 
	VGA_GRAY, mbt);

	ktty_putf("%a[TOS-PALLOC]%a (Real: 0x%p) [Flags: 0x%x]\n", VGA_BRIGHT(VGA_RED), 
	VGA_GRAY, page_get_phys(mbt), (uint)mbt->flags);

	// Memory in kb
	uint mem_size = mbt->mem_upper * 1.024f;


	ktty_putf("%a[TOS-PALLOC]%a Memory -> [%u Mb, %u Kb]\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, mem_size / 1000, mem_size);

	multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)((uint8_t*)mbt->mmap_addr + 0xC0000000);
	
	while(mmap < mbt->mmap_addr + 0xC0000000 + mbt->mmap_length)
	{
		ktty_putf("%a[TOS-PALLOC]%a Reading entry at: 0x%p...", VGA_BRIGHT(VGA_RED), VGA_GRAY, mmap);
		if(mmap->type == 1)
		{
			// Log message
			if(mmap->length / 1000000 <= 0)
				ktty_putf("%aUsable%a [%u Kb]\n", (VGA_GREEN), VGA_GRAY, mmap->length / 1000);
			else 
				ktty_putf("%aUsable%a [%u Mb]\n", (VGA_GREEN), VGA_GRAY, mmap->length / 1000000);

				
		}
		else
		{
			ktty_putf("%aNot Usable%a\n", (VGA_BROWN), VGA_GRAY);
		}

		mmap = (multiboot_memory_map_t*)((uint)mmap + mmap->size + sizeof(mmap->size));
	}
}