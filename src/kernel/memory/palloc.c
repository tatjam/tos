#include "palloc.h"

extern int _kernel_end_ph;
extern int _kernel_end;
extern size_t kalloc_dumb_pl_address;
extern int boot_pagedir;


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

uint8_t* frames_phys;
uint8_t* frames;
uint32_t nframes;

#define FRAME_CALCULATE(addr, idname, offname) uint32_t idname = addr / 8; \
	uint32_t offname = addr % 8;

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

uint32_t frame_find_free()
{
	for(uint32_t i = 0; i < nframes / 8; i++)
	{
		// Ignore frames with all bits set
		if(frames[i] != 0xFF) 
		{
			//klog("Found at addr: 0x%p [%u]\n", frames_phys + i, nframes / 8);
			for(uint8_t off = 0; off < 8; off++)
			{
				uint8_t test = 1 << off;
				if(!(frames[i] & test))
				{
 					return i*8+off;
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
		// Check that we contain some stuff not fully used by the kernel
		if(mmap->type == 1 
			&& (mmap->base_addr + mmap->length > PAGE_ALIGN((size_t)(&_kernel_end_ph))))
		{
			
			if(mmap->length / 1000000 <= 0)
				klog("%aUsable%a [%u Kb]", (VGA_GREEN), VGA_GRAY, mmap->length / 1000);
			else 
				klog("%aUsable%a [%u Mb]", (VGA_GREEN), VGA_GRAY, mmap->length / 1000000);
	

			sector_t out;
			out.valid = true;
			out.addr = mmap->base_addr;
			// Adjust for kernel
			if(out.addr < &_kernel_end_ph)
			{
				out.addr = PAGE_ALIGN(&_kernel_end_ph);
				klog("[ADJ: %p]", out.addr);
			}
			out.size = mmap->length;

			sectors_out[*i] = out;

			(*i)++;
		}
		else
		{
			if(mmap->length / 1000000 <= 0)
				klog("%aNot Usable%a [%u Kb]", (VGA_BROWN), VGA_GRAY, mmap->length / 1000);
			else 
				klog("%aNot Usable%a [%u Mb]", (VGA_BROWN), VGA_GRAY, mmap->length / 1000000);
		}
		klog("\n");


		mmap = (multiboot_memory_map_t*)((uint)mmap + mmap->size + sizeof(mmap->size));
	}
}

bool set_work_page()
{
	//klog("MAXKEKKING!");
	if(!page_map_temp(frames_phys, 1024))
	{
		klog("%a[TOS-PALLOC]%a ERROR: Could not map work page %a\n", VGA_BRIGHT(VGA_RED), VGA_RED, VGA_GRAY);
		return false;
	}

	frames = PAGE_TEMP_PAGE;

	return true;
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
	klog("%a[TOS-PALLOC]%a Kernel End -> 0x%p virt(0x%p)\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, &_kernel_end_ph, &_kernel_end);

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

	klog("%a[TOS-PALLOC]%a %u blocks (%u bytes, %u Kb) ready to work on\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, 
	total_blocks, total_blocks / 8, total_blocks / 8000);

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

	// Not needed?
	//bitmap_target->addr = PAGE_ALIGN(bitmap_target->addr);

	klog("%a[TOS-PALLOC]%a Found space for bitmap: r(0x%p)\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, bitmap_target->addr);
	klog("\nPage directory location: 0x%p -> 0x%p\n", &boot_pagedir, ((size_t)(&boot_pagedir) + 0x1000));
	// Allocate the bitmap, to do so we must manually map the memory into pages wt 
	frames_phys = (void*)((size_t)bitmap_target->addr);
	frames = (void*)PAGE_TEMP_PAGE;


	if(set_work_page())
	{
		klog("FRAME ADDRESS: 0x%p >0x%p<\n", frames_phys, PAGE_TEMP_PAGE);

		nframes = total_blocks;
		size_t bitmap_top = total_blocks / 8 / 4096;
		bitmap_top = (bitmap_top == 0) ? 1 : bitmap_top;
		
		size_t final = 0;
		// Set all as safe to use
		for(size_t i = 0; i < total_blocks; i++)
		{
			// Except those used by the bitmap
			if(i < bitmap_top)
			{
				frame_set(i);
			}
			else
			{
				frame_clear(i);
				final++;
			}	


		}

		klog("%a[TOS-PALLOC]%a Final blocks %u (%u used by bitmap)\n", VGA_BRIGHT(VGA_RED), VGA_GRAY, final, total_blocks - final);
	}

}

void* palloc_get()
{
	set_work_page();
	size_t id = frame_find_free();
	frame_set(id);
	//klog("{ID: %u}\n", id);
	return (void*)(frames_phys + id * 0x1000);
}

bool palloc_free(void* ptr)
{
	set_work_page();

	if(!PAGE_IS_ALIGN(ptr))
	{
		return false;
	}
	else
	{
		size_t id = ((size_t)ptr - (size_t)frames_phys) / 0x1000;
		frame_clear(id);
		return true;
	}

	return false;
}


void dump_first_few(int j)
{
	set_work_page();
	
	klog("\n");
	for(uint32_t i = j; i < j + 2; i++)
	{
		for(uint8_t off = 0; off < 8; off++)
		{
			uint8_t test = 1 << off;
			if(frames[i] & test)
			{
				klog("X");
			}
			else
			{
				klog(".");
			}
		}
		klog(" | 0x%p\n", page_get_phys(frames + i));
	}
}

void* palloc(page_directory_t* pd, size_t count, bool user, bool writeable)
{
	//klog("palloc called\n");
	if(pd == NULL)
	{
		pd = page_get_default_dir();
	}

	// We assign ourselves
	page_path_t path = page_find_free(pd, count, false);
	if(path.pd_index < 0 || path.pt_index < 0)
	{
		return NULL;
	}

	//klog("Found %i pages\n", count);
	
	for(size_t i = 0; i < count; i++)
	{
		size_t ri = i + path.index;

		size_t pd_index = ri / 1024;
		size_t pt_index = ri % 1024;

		// Find a free physical memory piece
		void* phloc = palloc_get();
		if(phloc == NULL)
		{
			return NULL;
		}

		// Claim the page and assign the physical memory
		page_map_temp(pd->entries[pd_index].frame << 12, 1);
		page_table_t* pt = (page_table_t*)(PAGE_TEMP_PAGE);

		memset(&pt->pages[pt_index], 0, 4);
		pt->pages[pt_index].frame = ((size_t)phloc >> 12);
		pt->pages[pt_index].present = true;
		pt->pages[pt_index].user = user;
		pt->pages[pt_index].rw = writeable;

		//klog("[0x%p](0x%p, 0x%p) -> [0x%p] [0x%p]\n", i, pd_index, pt_index, phloc, pt->pages[pt_index].frame);

	}


	// Return the virtual address
	// 	- Each page directory entry increases the virtual address by 0x400000
	//	- As each directory entry contains 1024 pages, each page must
	//		increase the virtual address by 0x400000 / 1024 = 4096 = 0x1000
	size_t out = (size_t)path.pd_index * (size_t)0x400000 + (size_t)path.pt_index * (size_t)0x1000;

	//klog("Done! out: %p (%p/%p)\n", out, path.pd_index, path.pt_index);

	asm_tlb_notify();

	return (void*)out;
}

void pfree(void* mem, size_t count, page_directory_t* pd)
{
	if(pd == NULL)
	{
		pd = page_get_default_dir();
	}
	// Find its physical location in order to 
	// free the physical memory:
	uint8_t* ph = page_get_phys(mem);
	uint8_t* memc = (uint8_t*)mem;

	for(size_t i = 0; i < count; i++)
	{
		ph += 0x1000;
		memc += 0x1000;

		// Free the physical memory
		palloc_free(ph);
	

		// Now free the actual page and set as unassigned
		size_t pd_index = (size_t)memc / (size_t)0x400000;
		size_t pt_index = ((size_t)memc % (size_t)0x400000) / 0x1000;

		page_map_temp((void*)(pd->entries[pd_index].frame << 12), 1);
		page_table_t* pt = (page_table_t*)PAGE_TEMP_PAGE;
		memset(&pt->pages[pt_index], 0, 4);
	}

	// Done!
}