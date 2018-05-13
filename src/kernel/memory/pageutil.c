#include "pageutil.h"
#include "palloc.h"

// These must be on boot.s
// They are arrays of 1024 uint32_t
// and should be aligned by boot.s

extern uint32_t* boot_pagedir;
extern uint32_t* boot_pagetab1;

static size_t old_addr;
static size_t act_addr;


// This page is mapped to the second topmost
// directory entry and is used for working on stuff
// Ignore the error, it is a valid gcc attribute, but clang invalid
page_table_t page_temp_page __attribute__((aligned (0x1000))) = {0};

void page_init()
{
    page_map_temp(0x0);
    old_addr = 0;
    act_addr = 0;
}


page_directory_entry_t* page_map_temp(void* phys)
{
    old_addr = act_addr;
    act_addr = (size_t)phys;

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

page_directory_entry_t* page_map_temp_restore()
{
    page_map_temp((void*)old_addr);
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

    // klog("ptindex: %p, pdindex: %p, pt: %p access: %p", ptindex, pdindex, pt, access);

    pt[ptindex] = ((unsigned long)phys) | (flags & 0xFFF) | 0x01; // Present
 
    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
}

static page_t page_linear(page_directory_t* pd, size_t address)
{
    size_t pd_address = address / 1024;
    size_t pt_address = address % 1024;

    if(pd->entries[pd_address].present)
    {
        page_map_temp(pd->entries[pd_address].frame << 12);
        page_table_t* pt = (page_table_t*)PAGE_TEMP_PAGE;
        page_t ret = pt->pages[pt_address];
        page_map_temp_restore();
        return ret;
    }
    else
    {
        page_t invalid;
        invalid.present = false;
        return invalid;
    }
}

static bool page_prepare(page_directory_t* pd, size_t index, size_t count, bool assign)
{
    for(size_t i = 0; i < count; i++)
    {
        size_t ri = index + i;
        size_t pd_address = ri / 1024;
        size_t pt_address = ri % 1024;
        
        // Check for valid page directory entry
        if(pd->entries[pd_address].present)
        {
            // Good! (Maybe check some other stuff TODO)
        }
        else
        {
            // We must allocate a page_table!
            void* phys = palloc_get();
            if(phys == NULL)
            {
                return false;  // Out of memory
            }

            //klog("Allocated page table @ 0x%p\n", phys);

            page_map_temp(phys);
            // Build the page table, clear memory (set to zero)
            memset(PAGE_TEMP_PAGE, 0, 0x1000);
            page_map_temp_restore();

            // Assign this entry to that new pagetable
            pd->entries[pd_address].present = true;
            pd->entries[pd_address].frame = (size_t)phys >> 12;
            pd->entries[pd_address].rw = 1;
            // TODO: User processes etc...
        }

        if(assign)
        {

            page_map_temp(pd->entries[pd_address].frame << 12);
            page_table_t* pt = (page_table_t*)PAGE_TEMP_PAGE;
            pt->pages[pt_address].present = true;
            page_map_temp_restore();
        }
    }

    asm_tlb_notify();

    return true;
}

static void page_find_free_low(page_directory_t* pd, page_path_t* out, size_t count, bool assign)
{
    size_t consecutive = 0;
    size_t ori = 0;

    // We ignore page 0 as it will confuse all allocators
    for(size_t i = 1; i < 1021 * 1024; i++)
    {
        // This goes page per page
        page_t pag = page_linear(pd, i);
        if(pag.present == 0)
        {
            consecutive++;
        }
        else
        {
            consecutive = 0;
            ori = i;
        }

        //klog("Consecutive: %u/%u\n", consecutive, count);

        if(consecutive >= count)
        {
           // klog("Found!");
            ori++;
            // Found enough consecutive pages!
            // Go over each of them making sure
            // they have a valid page_directory entry
            if(page_prepare(pd, ori, count, assign))
            {
                // Good, return it
                out->pd_index = ori / 1024;
                out->pt_index = ori % 1024;
                out->index = ori;
                return;
            }
            else
            {
                // Out of memory :(
                return;
            }
        }
    }
    
}

page_path_t page_find_free(page_directory_t* pd, size_t num, bool assign)
{
    page_path_t out;
    out.pd_index = -1;
    out.pt_index = -1;

    if(num == 0)
    {
        return out;
    }

    // We prefer not to create a page table
    // but if it's neccesary we will
    page_find_free_low(pd, &out, num, assign);

    return out;
}

page_directory_t* page_get_default_dir()
{
    return (page_directory_t*)0xFFFFF000;
}