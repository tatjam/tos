#include "palloc.h"

extern int _kernel_end_ph;
extern size_t kalloc_dumb_pl_address;


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

uint32_t *frames;
uint32_t nframes;

#define FRAME_INDEX_FROM_BIT(a) (a/(8*4))
#define FRAME_OFFSET_FROM_BIT(a) (a%(8*4))
#define FRAME_CALCULATE(addr, idname, offname) uint32_t idname = FRAME_INDEX_FROM_BIT(addr / 0x1000); \
	uint32_t offname = FRAME_OFFSET_FROM_BIT(addr / 0x1000);

// Useful, private functions (marked as static)
static void frame_set(uint32_t addr)
{
	FRAME_CALCULATE(addr, id, off);
	frames[id] |= (1 << off);
}

static void frame_clear(uint32_t addr)
{
	FRAME_CALCULATE(addr, id, off);
	frames[id] &= ~(1 << off);
}

static bool frame_test(uint32_t addr)
{
	FRAME_CALCULATE(addr, id, off);
	return frames[id] & (1 << off);
}

static uint32_t frame_find_free()
{
	for(uint32_t i = 0; i < FRAME_INDEX_FROM_BIT(nframes); i++)
	{
		// Ignore frames with all bits set
		if(frames[i] != 0xFFFFFFFF) 
		{
			for(uint32_t off = 0; off < 32; off++)
			{
				uint32_t test = 1 << off;
				if(!(frames[i] & test))
				{
 					return i*4*8+off;
				}
			}
		}
	}

	return (uint32_t)-1;
}


void frame_alloc(page_t* page, bool kernel, bool writeable)
{
	if(!page->frame)
	{
		// Page is already allocated
		return;
	}
	else
	{
		uint32_t id = frame_find_free();
		if(id == (uint32_t)-1)
		{
			klog("[TOS-PALLOC] Could not find a free frame!");
			// We should panic, TODO
			return;
		}
		frame_set(id * 0x1000);
		page->present = true;
		page->rw = writeable;
		page->user = !kernel;
		page->frame = id;
	}
}

void frame_free(page_t* page)
{
	uint32_t frame = page->frame;
	if(!frame)
	{
		// We were given an unallocated page
		return;
	}
	else
	{
		frame_clear(frame);
		page->frame = 0;
	}
}

void extract_sectors(multiboot_info_t* mbt, multiboot_memory_map_t* mmap, sector_t* sectors_out, size_t* i)
{
	// First pass: Finds usable blocks and stores them
	while(mmap < (multiboot_memory_map_t*)(mbt->mmap_addr + 0xC0000000 + mbt->mmap_length))
	{
		// Log message
		klog("%a[TOS-PALLOC]%a Reading entry r:(0x%p)...", VGA_BRIGHT(VGA_RED), VGA_GRAY, mmap->base_addr);
		if(mmap->type == 1 && mmap->base_addr > (size_t)(&_kernel_end_ph))
		{
			
			if(mmap->length / 1000000 <= 0)
				klog("%aUsable%a [%u Kb]\n", (VGA_GREEN), VGA_GRAY, mmap->length / 1000);
			else 
				klog("%aUsable%a [%u Mb]\n", (VGA_GREEN), VGA_GRAY, mmap->length / 1000000);
	

			sector_t out;
			out.valid = true;
			out.addr = mmap->base_addr;
			out.size = mmap->length;

			sectors_out[*i] = out;

			(*i)++;
		}
		else
		{
			if(mmap->length / 1000000 <= 0)
				klog("%aNot Usable%a [%u Kb]\n", (VGA_BROWN), VGA_GRAY, mmap->length / 1000);
			else 
				klog("%aNot Usable%a [%u Mb]\n", (VGA_BROWN), VGA_GRAY, mmap->length / 1000000);
		}


		mmap = (multiboot_memory_map_t*)((uint)mmap + mmap->size + sizeof(mmap->size));
	}
}

void palloc_init(multiboot_info_t* mbt)
{

	klog("%a[TOS-PALLOC]%a Reading mbt at: 0x%p\n", VGA_BRIGHT(VGA_RED), 
	VGA_GRAY, mbt);

	klog("%a[TOS-PALLOC]%a (Real: 0x%p) [Flags: 0x%x]\n", VGA_BRIGHT(VGA_RED), 
	VGA_GRAY, page_get_phys(mbt), (uint)mbt->flags);

	// Memory in kb
	uint mem_size = mbt->mem_upper * 1.024f;


	klog("%a[TOS-PALLOC]%a Memory -> [%u Mb, %u Kb]\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, mem_size / 1000, mem_size);
	klog("%a[TOS-PALLOC]%a Kernel End -> 0x%p\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, &_kernel_end_ph);

	multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)((uint8_t*)mbt->mmap_addr + 0xC0000000);

	// 64 seems like a reasonable limit
	sector_t sectors[64];

	// Initialize to invalid
	for(size_t i = 0; i < 64; i++){sectors[i].valid = false;}


	size_t i = 0;

	extract_sectors(mbt, mmap, sectors, &i);

	klog("%a[TOS-PALLOC]%a %u sectors usable\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, i);

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

	klog("%a[TOS-PALLOC]%a %u blocks (%u Kb) ready to work on\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, 
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

	// Set the dumb alloc pointer to here
	kalloc_dumb_pl_address = bitmap_target->addr;

	if(bitmap_target == NULL)
	{
		klog("%a[TOS-PALLOC]%a ERROR: Couldn't find sector to allocate bitmap! %a\n", VGA_BRIGHT(VGA_RED), VGA_RED, VGA_GRAY);
		return;
	}

	klog("%a[TOS-PALLOC]%a Found space for bitmap: r(0x%p)\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, bitmap_target->addr);
	
	// Allocate the bitmap, to do so we must manually map the memory into pages
	

}