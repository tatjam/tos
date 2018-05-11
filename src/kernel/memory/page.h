#pragma once

typedef struct page
{
    unsigned int present    : 1;
    unsigned int rw         : 1;
    unsigned int user       : 1;
    unsigned int reserved   : 2;
    unsigned int accessed   : 1;
    unsigned int dirty      : 1;
    unsigned int reserved2  : 2;
    unsigned int available  : 3;
    unsigned int frame      : 20;
} page_t;

typedef struct page_table
{
    page_t pages[1024];

} page_table_t;

typedef struct page_directory_entry
{
    size_t present    : 1;
    size_t rw         : 1;
    size_t user       : 1;
    size_t w_through  : 1;
    size_t cache      : 1;
    size_t access     : 1;
    size_t reserved   : 1;
    size_t page_size  : 1;
    size_t global     : 1;
    size_t available  : 3;
    size_t frame      : 20;
} page_directory_entry_t;

typedef struct page_directory
{
    page_directory_entry_t entries[1024];
   /* uint32_t tables_phys[1024];

    uint32_t phys;*/
	
} page_directory_t;