#include "pageutil.h"

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
    // Write some good stuff to it
    for(size_t i = 0; i < 1024; i++)
    {
        page_t npage = {0};
        npage.present = 1;
        npage.rw = 1;
        npage.frame = 0xBABE; // 0xFFFF_F000
        page_temp_page.pages[i] = npage;
    }


    // Second topmost directory entry
    page_directory_t* pd = (page_directory_t*)0xFFFFF000;
    pd->entries[1022].page_size = false;
    pd->entries[1022].cache = false;
    pd->entries[1022].frame = (size_t)((size_t)(&page_temp_page) - (size_t)0xC0000000) >> (size_t)(4*3);
    pd->entries[1022].present = true;

    

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