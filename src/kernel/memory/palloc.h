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
#include "pageutil.h"



/*
	palloc_init: Initializes the physical memory manager, dividing the memory
	in 4Kb sized and aligned sections.
	
	It correctly marks usable blocks as read from multiboot info, and allocates by itself
	the memory for the physical allocator
*/
void palloc_init(multiboot_info_t* mbd);

/*
	frame_alloc: Allocates a frame
*/
void frame_alloc(page_t* page, bool kernel, bool writeable);

/*
	frame_free: Frees a frame
*/
void frame_free(page_t* page);