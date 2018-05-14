#include "pageutil.h"
#include "palloc.h"

// These must be on boot.s
// They are arrays of 1024 uint32_t
// and should be aligned by boot.s

extern uint32_t boot_pagedir;
extern uint32_t boot_pagetab1;

static size_t old_addr;
static size_t act_addr;
static size_t old_size;
static size_t act_size;



// This page is mapped to the second topmost
// directory entry and is used for working on stuff
// Ignore the error, it is a valid gcc attribute, but clang invalid
page_table_t page_temp_page __attribute__((aligned (0x1000)));

void page_init()
{
    page_map_temp(0x0, 1024);
    old_addr = 0;
    act_addr = 0;

}

void page_lateinit()
{
    // Move to our clone
    page_directory_t* clone = page_create_task((uint8_t*)(&boot_pagedir) - (size_t)0xC0000000);
    page_load_dir((page_directory_t*)clone);
    //page_load_dir((uint8_t*)(&boot_pagedir) - (size_t)0xC0000000);
}

void page_load_dir(page_directory_t* dir)
{
    klog("Switching to 0x%p\n", dir);
    asm volatile("mov %0, %%cr3" :: "r"(dir));
}


page_directory_entry_t* page_map_temp(void* phys, size_t count)
{
    old_addr = act_addr;
    old_size = act_size;
    act_addr = (size_t)phys;
    act_size = count;

    if(!PAGE_IS_ALIGN((size_t)phys))
    {
        klog("Could not!");
        return NULL;
    }

   // phys = (size_t)phys >> 12;

    for(size_t i = 0; i < count; i++)
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
    return page_map_temp((void*)old_addr, old_size);
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
        page_map_temp(pd->entries[pd_address].frame << 12, 1);
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

            page_map_temp(phys, 1);
            // Build the page table, clear memory (set to zero)
            memset((void*)PAGE_TEMP_PAGE, 0, 0x1000);
            page_map_temp_restore();

            // Assign this entry to that new pagetable
            pd->entries[pd_address].present = true;
            pd->entries[pd_address].frame = (size_t)phys >> 12;
            pd->entries[pd_address].rw = 1;
            // TODO: User processes etc...
        }

        if(assign)
        {

            page_map_temp(pd->entries[pd_address].frame << 12, 1);
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

page_directory_t* page_get_kernel_dir()
{
    return (page_directory_t*)boot_pagedir;
}


#define PAGE_CLONE_BUFFER 128

static void* page_clone_table_data(void* frame)
{
    // Allocate the physical space
    void* dout = palloc_get();

    // Work in chunks of X bytes for speed
    uint8_t buff[PAGE_CLONE_BUFFER] = {0};

    for(size_t i = 0; i < 0x1000; i += PAGE_CLONE_BUFFER)
    {
        page_map_temp(frame, 1);
        // Load x bytes
        for(size_t j = 0; j < PAGE_CLONE_BUFFER; j++)
        {
            buff[j] = ((uint8_t*)PAGE_TEMP_PAGE)[j + i];
        }

        page_map_temp(dout, 1);
        // Write x bytes
        for(size_t j = 0; j < PAGE_CLONE_BUFFER; j++)
        {
            ((uint8_t*)(PAGE_TEMP_PAGE))[j + i] = buff[j];
        }
    }

    return dout;
}

// Takes and returns physical addresses
static page_table_t* page_clone_table(page_table_t* src)
{
    page_table_t* ntable = palloc_get();

    // Zero it
    page_map_temp(ntable, 1);
    memset((void*)PAGE_TEMP_PAGE, 0, 0x1000);

    page_table_t* src_v = (page_table_t*)PAGE_TEMP_PAGE;
    page_table_t* ntable_v = (page_table_t*)PAGE_TEMP_PAGE;
    page_map_temp(src, 1);

    for(size_t i = 0; i < 1024; i++)
    {
        page_t dat = src_v->pages[i];
        if(dat.present)
        {
            page_map_temp(ntable, 1);
            ntable_v->pages[i] = dat;
            size_t nplace = (size_t)page_clone_table_data((void*)(dat.frame << 12)) >> 12;
            page_map_temp(ntable, 1);
            ntable_v->pages[i].frame = nplace;
        }        
    }

    return ntable;
}

page_directory_t* page_create_task(page_directory_t* src)
{
    // Allocate space for the new directory
    page_directory_t* ndir = palloc_get();
    // Zero it
    page_map_temp(ndir, 1);
    memset((void*)PAGE_TEMP_PAGE, 0, 0x1000);

    page_map_temp(src, 1);
    page_directory_t* src_v = (page_directory_t*)PAGE_TEMP_PAGE;
    page_directory_t* kernel_v = (page_directory_t*)PAGE_TEMP_PAGE;

    klog("\n == WORKING ON %p TO %p ==\n", src, ndir);

    for(size_t i = 0; i < 1024; i++)
    {
        page_map_temp(src, 1);
        if(src_v->entries[i].present)
        {
            klog("[%u]", i);
            if(i == 1023)
            {
                // Super special case as we must adjust this to map to our new page
                klog("ID_MAP");

                page_directory_entry_t entry = src_v->entries[i];
                page_table_t* ntable = page_clone_table(entry.frame << 12);
                page_map_temp(ndir, 1);
                src_v->entries[i] = entry;
                src_v->entries[i].frame = (size_t)ntable >> 12;
                src_v->entries[i].present = true;

                page_map_temp(ntable, 1);
                page_table_t* ntable_v = (page_table_t*)PAGE_TEMP_PAGE;
                memset(ntable_v, 0, 1024);
                ntable_v->pages[1023].frame = (size_t)ndir >> 12;
                ntable_v->pages[1023].present = true;
                ntable_v->pages[1023].rw = true;
                // Done :)
            }
            else
            {
                // Obtain kernel
                page_map_temp((uint8_t*)(&boot_pagedir) - (size_t)0xC0000000, 1);
                page_directory_entry_t entry = kernel_v->entries[i];
                page_map_temp(src, 1);

                klog("KERNEL: %p | SRC: %p\n", entry.frame, src_v->entries[i].frame);

                // Note that we don't need to map it
                // We also ignore the 1022 page as it's the shared workpage
                if(memcpm(&src_v->entries[i], &entry, 4) == 0 || i == 1022)
                {
                    klog("KERNEL");
                    // Kernel (note that we change the page so src_v "changes")
                    // ndir is NOT A TYPO
                    page_directory_entry_t entry = src_v->entries[i];
                    page_map_temp(ndir, 1); 
                        src_v->entries[i] = entry; // src_v is actually ndir
                    page_map_temp_restore();

                }
                else
                {
                    klog("USER");
                    // Not-Kernel, copy it, allocating a new frame and all
                    // so the father process does not see child's changes
                    page_directory_entry_t entry = src_v->entries[i];
                    page_table_t* ntable = page_clone_table(entry.frame << 12);
                    page_map_temp(ndir, 1);
                        src_v->entries[i] = entry;
                        src_v->entries[i].frame = (size_t)ntable >> 12;
                    page_map_temp_restore();
                }
            }

            klog("\n");
        }
    }

    klog("OUTDIR: %p\n", ndir);

    return ndir;
}