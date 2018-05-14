#include "string.h"


char* get_str()
{
	return "String for testing... !";
}

// Hope this matches the c standard!
void* memset(void* ptr, int value, size_t num)
{
	size_t c = 0;
	uint8_t* uptr = ptr;

	while(c < num)
	{
		*uptr = (uint8_t)value;
		uptr++;
		c++;
	}

	return ptr;
}

// gcc implementation: https://github.com/gcc-mirror/gcc/blob/master/libiberty/memcmp.c
// Seems to be the cleanest, but could use my self written one, does not matter
int memcpm(void* a, void* b, size_t num)
{
	uint8_t* ac = (uint8_t*)a;
	uint8_t* bc = (uint8_t*)b;

	while(num-- > 0)
	{
		if(*ac++ != *bc++)
		{
			return ac[-1] < bc[-1] ? -1 : 1;
		}
	}

	return 0;
}