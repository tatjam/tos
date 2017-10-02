#include "pageutil.h"

// These must be on boot.s
// They are arrays of 1024 uint32_t
// and should be aligned by boot.s

extern uint32_t* boot_pagedir;
extern uint32_t* boot_pagetab1;



void page_init()
{
    /*
    // Set all entries to not present
    for(int i = 0; i < 1024; i++)
    {
        boot_pagedir[i] = PAGE_FLAG_WRITE;
    }

    // Configure the 
    for(int i = 0; i < 1024; i++)
    {
        // As the address is page aligned, it will always leave 12 bits zeroed.
        boot_pagetab1[i] = (i * 0x1000) | PAGE_FLAG_WRITE | PAGE_FLAG_PRESENT;
    }

    boot_pagedir[0] = ((uint)boot_pagetab1 | PAGE_FLAG_WRITE | PAGE_FLAG_PRESENT);
    
    page_refresh();
    */
}

void* page_get_phys(void* addr)
{
    unsigned long pdindex = (unsigned long)addr >> 22;
    unsigned long ptindex = (unsigned long)addr >> 12 & 0x03FF;
 


    unsigned long * pd = (unsigned long *)0xFFFFF000;
    // Here you need to check whether the PD entry is present.
    

    unsigned long * pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
 
    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)addr & 0xFFF));
}