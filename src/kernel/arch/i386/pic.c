#include "pic.h"

void pic_send_eoi(uint8_t irq)
{
	if(irq >= 8)
	{
		asm_outb(PIC2_COMMAND,PIC_EOI);
	}

	asm_outb(PIC1_COMMAND,PIC_EOI);
}

// Private defines 

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */


void pic_remap(uint offset1, uint offset2)
{
	uint8_t a1, a2;

	// Save masks
	a1 = asm_inb(PIC1_DATA);
	a2 = asm_inb(PIC2_DATA);

	// Starts initialization sequence (Cascade Mode)
	asm_outb(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);
	asm_io_wait();
	asm_outb(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);
	asm_io_wait();
	asm_outb(PIC1_DATA, offset1);	// ICW2: Master PIC vector offset
	asm_io_wait();
	asm_outb(PIC2_DATA, offset2);	// ICW2: Slave PIC vector offset
	asm_io_wait();
	asm_outb(PIC1_DATA, 4);			// ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	asm_io_wait();
	asm_outb(PIC2_DATA, 2);			// ICW3: tell Slave PIC its cascade identity (0000 0010)
	asm_io_wait();

	asm_outb(PIC1_DATA, ICW4_8086);
	asm_io_wait();
	asm_outb(PIC2_DATA, ICW4_8086);
	asm_io_wait();

	asm_outb(PIC1_DATA, a1);		// restore saved masks.
	asm_outb(PIC2_DATA, a2);
}

