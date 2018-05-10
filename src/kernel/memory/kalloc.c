#include "kalloc.h"

size_t kalloc_dumb_pl_address = 0;

void* kmalloc_dumb(size_t size, bool align, size_t* phys)
{
	if(align && (kalloc_dumb_pl_address & 0xFFFFF000))
	{
		// Align the address
		kalloc_dumb_pl_address &= 0xFFFFF000;
   		kalloc_dumb_pl_address += 0x1000;
	}

	if(phys)
	{
		*phys = kalloc_dumb_pl_address;
	}

	void* out = (void*)kalloc_dumb_pl_address;
	kalloc_dumb_pl_address += size;
	return out;
}