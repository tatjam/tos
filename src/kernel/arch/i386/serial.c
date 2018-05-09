#include "serial.h"
#include "../../../libk/kio.h"

static uint8_t buffer[SERIAL_BUFFER_SIZE] = {0};

static size_t buffptr = 0;

char serial_numbuff[128] = {'B'};

uint8_t* serial_get_buffer()
{
	return buffer;
}

uint16_t serial_getc()
{
	if(buffptr == 0)
	{
		return SERIAL_NODATA;
	}
	else
	{
		buffptr--;
		return buffer[buffptr];
	}
}

void serial_putc(uint8_t byte)
{
	while(serial_transmit_empty() == 0)
	{
		// Stop gcc from optimizing this out
		asm("");
	}

	asm_outb(SERIAL_COM1, byte);
}

uint8_t serial_transmit_empty()
{
	return asm_inb(SERIAL_COM1 + 5) & 0x20;
}

void serial_init()
{
	// Initialize buffers
	buffptr = 0;
	for(size_t i = 0; i < SERIAL_BUFFER_SIZE; i++)
	{
		buffer[i] = 0;
	}

	// Initialize device
	asm_outb(SERIAL_COM1 + 1, 0x00);    // Disable all interrupts
	asm_outb(SERIAL_COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	asm_outb(SERIAL_COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	asm_outb(SERIAL_COM1 + 1, 0x00);    //                  (hi byte)
	asm_outb(SERIAL_COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
	asm_outb(SERIAL_COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	asm_outb(SERIAL_COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
 }

 size_t serial_puts(char* str)
 {
	 size_t i = 0;
	 while(str[i] != 0)
	 {
		 serial_putc(str[i]);
		 i++;
	 }
	 return i;
 }

 void serial_putf(char* fmt, ...)
 {
	va_list parameters;
	va_start(parameters, fmt);

	serial_putf_list(fmt, &parameters);
 }

 void serial_putf_list(char* fmt, va_list* parameters)
 {
	size_t written = 0;

	size_t i = 0;

	while(fmt[i] != 0)
	{
		if(fmt[i] == '%')
		{
			i++;
			
			// Begin a fmt
			if(fmt[i] == '%')
			{
				serial_putc('%');
				written++;
			}
			else if(fmt[i] == 'c')
			{
				char c = (char)va_arg(*parameters, int);
				serial_putc(c);
				written++;
			}
			else if(fmt[i] == 's')
			{
				// Put string
				char* str = (char*)va_arg(*parameters, char*);
				size_t w = serial_puts(str);
				written += w;
			}
			else if(fmt[i] == 'i')
			{
				// TODO
				int num = (int)va_arg(*parameters, int); 
				kitoa_buf(serial_numbuff, sizeof(serial_numbuff), num, 10);
				size_t w = serial_puts(serial_numbuff);
				written += w;
			}
			else if(fmt[i] == 'u')
			{
				uint num = va_arg(*parameters, uint);
				kutoa_buf(serial_numbuff, sizeof(serial_numbuff), num, 10);
				size_t w = serial_puts(serial_numbuff);
				written += w;
			}
			else if(fmt[i] == 'x')
			{
				uint num = va_arg(*parameters, uint);
				kutoa_buf(serial_numbuff, sizeof(serial_numbuff), num, 16);
				size_t w = serial_puts(serial_numbuff);
				written += w;
			}
			else if(fmt[i] == 'f')
			{
				float num = (float)va_arg(*parameters, double);
				serial_putc((char)num);
			}
			else if(fmt[i] == 'n')
			{
				size_t* target = va_arg(*parameters, size_t*);

				*target = written;
			}
			else if(fmt[i] == 'p')
			{
				void* ptr = va_arg(*parameters, void*);
				kutoa_buf(serial_numbuff, sizeof(serial_numbuff), (uint)ptr, 16);
				size_t w = serial_puts(serial_numbuff);
				written += w;
			}
			else
			{
				// Pull the argument into void, it must be the colors!
				uint arg = (uint)va_arg(*parameters, uint);
			}
		}
		else
		{
			serial_putc(fmt[i]);
			written++;
		}

		i++;
	}
 }