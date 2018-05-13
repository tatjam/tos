#pragma once

typedef struct page
{
    size_t present    : 1;
    size_t rw         : 1;
    size_t user       : 1;
    size_t reserved   : 2;
    size_t accessed   : 1;
    size_t dirty      : 1;
    size_t reserved2  : 2;
    size_t available  : 3;
    size_t frame      : 20;
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