#pragma once

/*
		kio.h -  Kernel equivalent of the stdio.h header

		Also includes some utility stuff that would fit in stdlib.h and 
		such

		'buf' suffixed stuff means it works on buffers, instead of allocating and returning
		strings. You must provide the buffer.

*/

#include "libkdef.h"
#include "errdef.h"
#include <stdarg.h>


/*
	kvirt: Converts boot-level pointer to virtual
*/
void* kboot_to_virt(void* ptr);

/*
	kftoa_buf: Converts float to 0 terminated string

	kftoa that writes to a buffer. On case of overflow it instantly stops
	writing. (Guarantees writing the 0 byte)
	
	Returns KRET_OVERFLOW in case of overflow, KRET_SUCCESS on success and...

	Base can be up to 16, and can't be 0, otherwise it will return KRET_INVALID_ARGUMENTS
*/
kret_t kftoa_buf(char* buffer, size_t bufsize, float num, uint base);

/*
	kitoa_buf: Converts int to 0 terminated string

	kitoa that writes to a buffer. On case of overflow it instantly stops
	writing. (Guarantees writing the 0 byte)

	Returns KRET_OVERFLOW in case of overflow, KRET_SUCCESS on success and...

	Base can be up to 16, and can't be 0, otherwise it will return KRET_INVALID_ARGUMENTS
*/
kret_t kitoa_buf(char* buffer, size_t bufsize, int num, uint base);

/*
	kutoa_buf: Converts uint to 0 terminated string

	kutoa that writes to a buffer. On case of overflow it instantly stops
	writing. (Guarantees writing the 0 byte)

	Returns KRET_OVERFLOW in case of overflow, KRET_SUCCESS on success and...

	Base can be up to 16, and can't be 0, otherwise it will return KRET_INVALID_ARGUMENTS
*/
kret_t kutoa_buf(char* buffer, size_t bufsize, uint num, uint base);

/*
	kprintf_buf: kprintf to buffer

	Formatted print that writes to a buffer. On case of overflow it instantly stops
	writing. Uses same codes as kprintf. (Guarantees writing the 0 byte)

	Returns KRET_OVERFLOW in case of overflow, KRET_SUCCESS on success and...

	Base can be up to 16, and can't be 0, otherwise it will return KRET_INVALID_ARGUMENTS
*/
kret_t kprintf_buf(char* buffer, size_t bufsize, char* format, ...);

/*
	klog: Function for logging to console, serial, etc...
*/
void klog(char* format, ...);