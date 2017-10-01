#include "serial.h"

static uint8_t buffer[SERIAL_BUFFER_SIZE] = {0};

static size_t buffptr = 0;

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
		; // Wait... Please GCC dont optimize...
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