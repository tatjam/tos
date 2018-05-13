#include "pageutil.h"
#include "palloc.h"

// These must be on boot.s
// They are arrays of 1024 uint32_t
// and should be aligned by boot.s

extern uint32_t* boot_pagedir;
extern uint32_t* boot_pagetab1;


// This page is mapped to the second topmost
// directory entry and is used for working on stuff
// Ignore the error, it is a valid gcc attribute, but clang invalid
page_table_t page_temp_page __attribute__((aligned (0x1000))) = {0};

void page_init()
{
    page_map_temp(0x0);
}

page_directory_entry_t* page_map_temp(void* phys)
{
    if(!PAGE_IS_ALIGN((size_t)phys))
    {
        klog("Could not!");
        return NULL;
    }

   // phys = (size_t)phys >> 12;

    for(size_t i = 0; i < 1024; i++)
    {
        memset(&page_temp_page.pages[i], 0, 4);
        page_temp_page.pages[i].frame = ((size_t)phys >> 12) + i;
        page_temp_page.pages[i].present = true;
        page_temp_page.pages[i].rw = true;
    }

    page_directory_t* pd = (page_directory_t*)0xFFFFF000;
    memset(&pd->entries[1022], 0, 4);

    pd->entries[1022].frame = (size_t)(((size_t)(&page_temp_page) - (size_t)0xC0000000) >> (size_t)(12));
    pd->entries[1022].rw = true;
    pd->entries[1022].present = true;

    asm_tlb_notify();

    return &pd->entries[1022];
}

void* page_get_phys(void* addr)
{
    size_t pdindex = (size_t)addr >> 22;
    size_t ptindex = (size_t)addr >> 12 & 0x03FF;
 


    //size_t * pd = (size_t *)0xFFFFF000;
    // Here you need to check whether the PD entry is present.
    

    size_t* pt = ((size_t *)0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
 
    return (void *)((pt[ptindex] & ~0xFFF) + ((size_t)addr & 0xFFF));
}

void page_map(void* phys, void* virt, uint flags)
{
    // Make sure that both addresses are page-aligned.
 
    size_t pdindex = (size_t)virt >> 22;
    size_t ptindex = (size_t)virt >> 12 & 0x03FF;
 
    //size_t * pd = (size_t *)0xFFFFF000;
    // Here you need to check whether the PD entry is present.
    // When it is not present, you need to create a new empty PT and
    // adjust the PDE accordingly.
 
    size_t * pt = ((size_t *)0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
    // When it is, then there is already a mapping present. What do you do now?

    size_t access = (size_t)(pt + ptindex); 

    klog("ptindex: %p, pdindex: %p, pt: %p access: %p", ptindex, pdindex, pt, access);

    pt[ptindex] = ((unsigned long)phys) | (flags & 0xFFF) | 0x01; // Present
 
    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
}

static void page_find_free_low(page_directory_t* pd, page_path_t* out, int start_search)
{
    size_t i = 0;
    if(start_search > 0)
    {
        i = (size_t)start_search;
    }
    // 1022 and up are kernel reserved
    for(i; i < 1021; i++)    
    {
        if(pd->entries[i].present)
        {
            size_t ptr = pd->entries[i].frame;
            page_table_t* pt = (page_table_t*)(ptr << 12);
            page_map_temp(pt); // We know pt is page aligned
            page_table_t* ptv = (page_table_t*)PAGE_TEMP_PAGE;
            for(size_t j = 0; j < 1024; j++)
            {
                if(!ptv->pages[j].present && i != 768)
                {
                    out->pd_index = i;
                    out->pt_index = j;
                    return;
                }
            }
        }
    }
}

// Returns the index of the newly created pagetable
// If none found returns a negative number (-1)
static int page_create_anywhere(page_directory_t* pd)
{
    int out = -1;
    for(size_t i = 0; i < 1024; i++)    
    {
        if(!pd->entries[i].present)
        {
            // Ask the physical manager for a 4Kb chunk (physical address)
            void* phpos = palloc_get();

            // (We know it's page aligned)
            page_map_temp(phpos);
            klog("[Allocated at 0x%p]", phpos);
            // Zero the area
            memset((void*)PAGE_TEMP_PAGE, 0, 1024 * 4);
            pd->entries[i].present = true;
            pd->entries[i].frame = (size_t)phpos >> 12;

            return i;
        }
    }

    return out;
}

/*
	page_find_free_page: Finds a free page in a pagetable
*/
page_path_t page_find_free(page_directory_t* pd)
{
    page_path_t out;
    out.pd_index = -1;
    out.pt_index = -1;

    // We prefer not to create a page table
    // but if it's neccesary we will
    page_find_free_low(pd, &out, -1);

    if(out.pd_index == -1 || out.pt_index == -1)
    {
        klog("FIND...");
        // We have not returned yet, try to allocate a new pagetable
        int loc = page_create_anywhere(pd);

        klog("FOUND: %i\n", loc);
        if(loc >= 0)
        {
            page_find_free_low(pd, &out, loc);
        }
    }

    return out;
}

page_directory_t* page_get_default_dir()
{
    return (page_directory_t*)0xFFFFF000;
}