#include "palloc.h"

typedef struct multiboot_memory_map {
	uint size;
	uint64_t base_addr;
	uint64_t length;
	uint type;
} multiboot_memory_map_t;
 

// Sector that is usable 
typedef struct sector {

	uint64_t addr;
	uint64_t size;
	uint64_t blocks;
	bool valid;

} sector_t;

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

	// 64 seems like a reasonable limit
	sector_t sectors[64];

	// Initialize to invalid
	for(size_t i = 0; i < 64; i++){sectors[i].valid = false;}


	size_t i = 0;

	// First pass: Finds usable blocks and stores them
	while(mmap < mbt->mmap_addr + 0xC0000000 + mbt->mmap_length)
	{
		// Log message
		ktty_putf("%a[TOS-PALLOC]%a Reading entry at: 0x%p...", VGA_BRIGHT(VGA_RED), VGA_GRAY, mmap);
		if(mmap->type == 1)
		{
			
			if(mmap->length / 1000000 <= 0)
				ktty_putf("%aUsable%a [%u Kb]\n", (VGA_GREEN), VGA_GRAY, mmap->length / 1000);
			else 
				ktty_putf("%aUsable%a [%u Mb]\n", (VGA_GREEN), VGA_GRAY, mmap->length / 1000000);
	

			sector_t out;
			out.valid = true;
			out.addr = mmap->base_addr;
			out.size = mmap->length;

			sectors[i] = out;

			i++;
		}
		else
		{
			ktty_putf("%aNot Usable%a\n", (VGA_BROWN), VGA_GRAY);
		}


		mmap = (multiboot_memory_map_t*)((uint)mmap + mmap->size + sizeof(mmap->size));
	}

	ktty_putf("%a[TOS-PALLOC]%a %u sectors usable\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, i);

	// Now calculate how many blocks can we fit inside of each sector

	i = 0;

	size_t total_blocks = 0;

	while(sectors[i].valid != false)
	{
		// A block is 4Kb, size is given in bytes so blocks = size / 4096
		sectors[i].blocks = sectors[i].size / 4096;
		total_blocks += sectors[i].blocks;
		i++;
	}

	ktty_putf("%a[TOS-PALLOC]%a %u blocks (%u Kb) ready to work on\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, 
	total_blocks, total_blocks / 1000);

	// Now find a sector with enough space to hold the bitmap

	sector_t* bitmap_target = NULL;
	i = 0;
	while(sectors[i].valid != false)
	{
		if(sectors[i].size > total_blocks)
		{
			bitmap_target = &sectors[i];
			break;
		}
		i++;
	}

	if(bitmap_target == NULL)
	{
		ktty_putf("%a[TOS-PALLOC]%a ERROR: Couldn't find sector to allocate bitmap! %a(Should never happen...)\n", VGA_BRIGHT(VGA_RED), VGA_RED, VGA_GRAY);
		return;
	}

	// We use a temporary Page Table to write all of the required data, and once 
}