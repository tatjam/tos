#pragma once
/*
		palloc.h -  Physical memory allocator to 
		seek which areas are available for allocating

		It uses a bitmap method, using very little RAM, and being
		RELATIVELY perfomant.

		Has 8 bits per 4Kb page, but could probably use as little as
		4 bits if space is required.
*/

#include "../multiboot/multiboot.h"
#include <tty.h>
#include <asmutil.h>
#include <kio.h>
#include "kalloc.h"
#include "pageutil.h"



/*
	palloc_init: Initializes the physical memory manager, dividing the memory
	in 4Kb sized and aligned sections.
	
	It correctly marks usable blocks as read from multiboot info, and allocates by itself
	the memory for the physical allocator
*/
void palloc_init(multiboot_info_t* mbd);

/*
	palloc_get: Obtains a single 4Kb memory segment and gets 
		you the start location in physical memory
*/
void* palloc_get();

/*
	palloc_free: Frees the 4Kb memory segment at given location.
		Returns true if successfull or false otherwise.
		It will fail on un-aligned pointers!
*/
bool palloc_free(void* ptr);

void dump_first_few(int j);