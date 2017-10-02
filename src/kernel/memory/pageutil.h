#pragma once
/*
		pageutil.h -  Useful functions for virtual mem

*/

#include <errdef.h>
#include <asmutil.h>


#define PAGE_FLAG_PRESENT 0x1 // Or NOT-PRESENT if not set
#define PAGE_FLAG_WRITE 0x2 // Or READ-ONLY if not set
#define PAGE_FLAG_USER 0x4 // Or KERNEL-ONLY if not set

/*
	page_init: Initializes the paging system. Will do some rebundant stuff
		that is already done on boot.s!
*/
void page_init();
/*
	page_get_phys: Gets physical address of given virtual address
*/
void* page_get_phys(void* addr);

/*
	page_update: Forces updating of the paging system
*/
extern void page_refresh();
