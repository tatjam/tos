#pragma once
#include <stddef.h>
#include <stdint.h>
/* Read c-docs */

char* get_str();

void* memset(void* ptr, int value, size_t num); 

int memcpm(void* a, void* b, size_t num);