#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*
		kalloc.h - Simple dumb memory allocators for early boot
*/


/*
  kmalloc_dumb: Simple, dumb, allocator. Takes all arguments
*/
void* kmalloc_dumb(size_t size, bool align, size_t* phys); 