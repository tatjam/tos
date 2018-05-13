#pragma once
/*
		pageutil.h -  Useful functions for virtual mem

*/

#include <errdef.h>
#include <asmutil.h>
#include <string.h>
#include "page.h"


#define PAGE_FLAG_PRESENT 0x1 // Or NOT-PRESENT if not set
#define PAGE_FLAG_WRITE 0x2 // Or READ-ONLY if not set
#define PAGE_FLAG_USER 0x4 // Or KERNEL-ONLY if not set

#define PAGE_IS_ALIGN(addr) (((((size_t)(addr)) | 0xFFFFF000) & 0x00000FFF) == 0)
#define PAGE_ALIGN(addr) ((((size_t)(addr)) & 0xFFFFF000) + 0x1000)

#define PAGE_DIR_INDEX(vaddr) (((uint32_t)vaddr) >> 22)
#define PAGE_TBL_INDEX(vaddr) ((((uint32_t)vaddr) >>12) & 0x3ff)
#define PAGE_FRAME_INDEX(vaddr) (((uint32_t)vaddr) & 0xfff)

// Register manipulation
#define PAGE_SET_PGBIT(cr0) (cr0 = cr0 | 0x80000000)
#define PAGE_CLEAR_PSEBIT(cr4) (cr4 = cr4 & 0xffffffef)

extern size_t kalloc_dumb_pl_address;

// The temporal page we use for work on other stuff
// Address by adding the address you want in the range
#define PAGE_TEMP_PAGE (size_t)0xFF800000 //0x400000 * 1022

// If any of the indexes are negative
// it means it found no page!
typedef struct page_path
{
	// Index in the page directory
	int32_t pd_index;
	// Index in the page table
	int32_t pt_index;
} page_path_t;

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
	page_map_temp: Maps the temporal (work) pagetable
		to given physical address. Maps the whole
		pagetable to increasing pointers. The page is located
		from 0xFF7FFFFF to 0xFFBFFFFF
	
	MUST TAKE PAGE ALIGNED POINTER!

	Feel free to change flags on the returned pointer

	Returns NULL if given pointer for some reason is invalid,
	such as being not aligned
*/
page_directory_entry_t* page_map_temp(void* phys);

/*
	page_find_free_page: Finds a free page in a pagetable
*/
page_path_t page_find_free(page_directory_t* pt);

page_directory_t* page_get_default_dir();