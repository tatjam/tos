#pragma once
/*
		pageutil.h -  Useful functions for virtual mem

*/

#include <errdef.h>
#include <asmutil.h>
#include "page.h"


#define PAGE_FLAG_PRESENT 0x1 // Or NOT-PRESENT if not set
#define PAGE_FLAG_WRITE 0x2 // Or READ-ONLY if not set
#define PAGE_FLAG_USER 0x4 // Or KERNEL-ONLY if not set

#define PAGE_IS_ALIGN(addr) ((((uint32_t)(addr)) | 0xFFFFF000) == 0)
#define PAGE_ALIGN(addr) ((((uint32_t)(addr)) & 0xFFFFF000) + 0x1000)

#define PAGE_DIR_INDEX(vaddr) (((uint32_t)vaddr) >> 22)
#define PAGE_TBL_INDEX(vaddr) ((((uint32_t)vaddr) >>12) & 0x3ff)
#define PAGE_FRAME_INDEX(vaddr) (((uint32_t)vaddr) & 0xfff)

// Register manipulation
#define PAGE_SET_PGBIT(cr0) (cr0 = cr0 | 0x80000000)
#define PAGE_CLEAR_PSEBIT(cr4) (cr4 = cr4 & 0xffffffef)

extern size_t kalloc_dumb_pl_address;



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

/*
	page_set_page: Sets the active page directory to given
*/
void page_set_dir(page_directory_t* n_page);


/*
  page_get: Returns page at given directory and address, if make is true
	it will be created.
*/
page_t* page_get(uint32_t address, bool make, page_directory_t *dir);


/*
	page_get_phys: Maps a virtual address to a physical address

	Creates PT when necessary, and handles page alignment of
	arguments automatically.

	MUST TAKE PAGE ALIGNED POINTERS!
*/
void page_map(void* phys, void* virt, uint flags);
