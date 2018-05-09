#pragma once
#include <stdarg.h>


/*
			serial.h - Serial interface

	Requires the serial interrupt handlers, which will write
	to some buffers, used by this. This allows serial to work
	on a non-blocking, interrupt driven way.

	All functions are prefixed with 'serial'

	We only use COM1 (for now, maybe more ports will be needed)

*/


#include <asmutil.h>
#include <errdef.h>

/* Wathever bigger than 255 */
#define SERIAL_NODATA 0xBEEF

#define SERIAL_BUFFER_SIZE 512

#define SERIAL_COM1 0x3f8

/*
	serial_get_buffer: Gets the serial input buffer

	Returns:
		* The buffer (always, the buffer is statically allocated)
*/
uint8_t* serial_get_buffer();

/*
	serial_getc: Gets a single char from the serial buffer

	Returns:
		* SERIAL_NODATA - When no data available
		* (0-255) - When data available
*/
uint16_t serial_getc();

/*
	serial_putc: Writes single byte directly to serial

	Blocking and will probably take a few cycles, depending on IO latency
*/
void serial_putc(uint8_t byte);

/*
	serial_puts: Writes string to serial, 0 terminated string
		
	Returns number of characters written
*/
size_t serial_puts(char* str);

/*
	serial_putf: Writes formatted string to serial. Same as kprintf
*/
void serial_putf(char* format, ...);

/*
	serial_putf_list: Same as serial_putf but takes the va_list

	Interanlly used by serial_putf
*/
void serial_putf_list(char* fmt, va_list* args);

/*
	serial_transmit_empty: Is the serial ready to transmit new byte?

	Used internally, but can be used

*/
uint8_t serial_transmit_empty();

void serial_init();