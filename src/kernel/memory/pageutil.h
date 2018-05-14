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
	// Real index
	int32_t index;
} page_path_t;

/*
	page_init: Initializes the paging system. Will do some rebundant stuff
		that is already done on boot.s!
*/
void page_init();

void page_lateinit();

/*
	page_get_phys: Gets physical address of given virtual address
*/
void* page_get_phys(void* addr);


/*	
	page_map_temp: Maps the temporal (work) pagetable
		to given physical address. Maps the whole
		pagetable to increasing pointers. Maps as many 
		pages as you tell it to
	
	MUST TAKE PAGE ALIGNED POINTER!

	Feel free to change flags on the returned pointer

	Returns NULL if given pointer for some reason is invalid,
	such as being not aligned

*/
page_directory_entry_t* page_map_temp(void* phys, size_t count);

/*
	page_map_temp_restore: Restores the previous temporal mapping
*/
page_directory_entry_t* page_map_temp_restore();


/*
	page_find_free: Finds ``num`` consecutive free pages in the pagedirectory

		If assign is true it will mark the new pages as used
*/
page_path_t page_find_free(page_directory_t* pd, size_t num, bool assign);

/*
	page_get_default_dir: Returns the loaded directory
		This one returns a virtual address!
*/
page_directory_t* page_get_default_dir();

/*
	page_get_kernel_dir: Returns the kernel directory
		Note that this one returns physical address
*/
page_directory_t* page_get_kernel_dir();

/*
	page_load_dir: Loads a paging directory. 
		Make sure the page directory you give
		has its last entry properly mapped to itself
		so the system can modify it!
*/
void page_load_dir(page_directory_t* dir);

/*
	page_create_task: Creates a new page directory for a task,
		cloning the stack and linking the kernel.

		Takes physical address and returns a physical address!
*/
page_directory_t* page_create_task(page_directory_t* actual);