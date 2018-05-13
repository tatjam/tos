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