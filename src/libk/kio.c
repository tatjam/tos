#include "kio.h"
#include "tty.h"
#include "../kernel/arch/i386/serial.h"

char* digits = {"0123456789ABCDEF"};

// 128b buffer for general usage by these functions
char crossbuffer[128] = {0};

void* kboot_to_virt(void* ptr)
{
	return (void*)((size_t)ptr + 0xC0000000);
}

// Reverse 0 terminated string, internal
void __reverse(char* str)
{
	// Find size
	size_t size = 0;
	while(str[size] != 0)
	{
		size++;
	}

	size--;

	// Double iterator method
	size_t l, r;
	l = 0; r = size;

	while(l < r)
	{
		char rchar = str[r];
		str[r] = str[l];
		str[l] = rchar;
		l++;
		r--;
	}
}

void __rshift(char* str)
{
	// Find size
	size_t size = 0;
	while(str[size] != 0)
	{
		size++;
	}

	size_t i = size;

	while(i >= 1)
	{
		str[i] = str[i - 1];
		i--;
	}

	str[0] = 'X';
}

kret_t kitoa_buf(char* buffer, size_t bufsize, int num, uint base)
{
	bool neg = false;
	// mod base
	if(num < 0)
	{
		num = -num;
		neg = true;
	}

	uint numu = (uint)num;
	if(neg)
	{
		kret_t res;

		if((res = kutoa_buf(buffer, bufsize - 1, numu, base)) != KRET_SUCCESS)
		{
			return res;
		}
		// Shift string right
		__rshift(buffer);
	
		// Add sign
		buffer[0] = '-';
	}
	else
	{
		return kutoa_buf(buffer, bufsize, numu, base);
	}

	return KRET_SUCCESS;

}

kret_t kutoa_buf(char* buffer, size_t bufsize, uint num, uint base)
{
	// Works by reversing the string at the end

	if(base > 16 || base <= 1)
	{
		return KRET_INVALID_ARGUMENTS;
	}

	size_t bi = 0;

	uint numb = num;

	uint digit = 0;

	while(numb >= base)
	{
		digit = numb % base;
		numb /= base;

		buffer[bi] = digits[digit];

		bi++;

		if(bi >= bufsize - 2)
		{
			// Early finish
			buffer[bi + 1] = 0;
			return KRET_OVERFLOW;
		}
	}

	buffer[bi] = digits[numb];

	buffer[bi + 1] = 0;

	/* Reverse string */
	__reverse(buffer);

	return KRET_SUCCESS;
}

kret_t kprintf_buf(char* buffer, size_t bufsize, char* fmt, ...)
{
	va_list parameters;
	va_start(parameters, fmt);

	size_t written = 0;

	size_t i = 0;

	// Buffer for integer to string conversions. Uses buffer that is probably as big as required

	while(fmt[i] != 0)
	{
		if(written >= bufsize - 1)
		{
			buffer[written + 1] = 0;
			return KRET_OFFBOUNDS;
		}
		if(fmt[i] == '%')
		{
			// Begin command 
			i++;
			
			// Begin a fmt
			if(fmt[i] == '%')
			{
				buffer[written] = '%';
				written++;
			}
			else if(fmt[i] == 'c')
			{
				char c = (char)va_arg(parameters, int);
				buffer[written] = c;
				written++;
			}
			else if(fmt[i] == 's')
			{
				// Put string
				char* str = va_arg(parameters, char*);
				size_t w = 0;
				while(str[w] != 0)
				{
					buffer[written] = str[w];
					written++;
					w++;
				}
			}
			else if(fmt[i] == 'i')
			{
				// TODO
				int num = va_arg(parameters, int); 
				kitoa_buf(crossbuffer, sizeof(crossbuffer), num, 10);
				// Write the string
				size_t w = 0;
				while(crossbuffer[w] != 0)
				{
					buffer[written] = crossbuffer[w];
					written++;
					w++;
				}
			}
			else if(fmt[i] == 'u')
			{
				uint num = va_arg(parameters, uint);
				kutoa_buf(crossbuffer, sizeof(crossbuffer), num, 10);
				// Write the string
				size_t w = 0;
				while(crossbuffer[w] != 0)
				{
					buffer[written] = crossbuffer[w];
					written++;
					w++;
				}
			}
			else if(fmt[i] == 'x')
			{
				uint num = va_arg(parameters, uint);
				kutoa_buf(crossbuffer, sizeof(crossbuffer), num, 16);
				// Write the string
				size_t w = 0;
				while(crossbuffer[w] != 0)
				{
					buffer[written] = crossbuffer[w];
					written++;
					w++;
				}
			}
			else if(fmt[i] == 'f')
			{
				float num = (float)va_arg(parameters, double);
				// TODO
			}
			else if(fmt[i] == 'n')
			{
				size_t* target = va_arg(parameters, size_t*);

				*target = written;
			}
			else if(fmt[i] == 'p')
			{
				void* ptr = va_arg(parameters, void*);
				uint addr = (uint)ptr;
				kutoa_buf(crossbuffer, sizeof(crossbuffer), addr, 16);
				// Write the string
				size_t w = 0;
				while(crossbuffer[w] != 0)
				{
					buffer[written] = crossbuffer[w];
					written++;
					w++;
				}
			}
		}
		else
		{
			buffer[written] = fmt[i];
			written++;
		}

		i++;
	}

	va_end(parameters);

	return KRET_SUCCESS;
			
}


// Console, serial and wathever else
void klog(char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	serial_putf_list(format, &args);

	va_start(args, format);

	ktty_putf_list(format, &args);

	va_end(args);

}