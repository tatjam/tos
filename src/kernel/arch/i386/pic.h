#pragma once


/*
		pic.h -  PIC chip functions

*/

#include <asmutil.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

#define PIC1 0x20 // Master chip	
#define PIC2 0xA0 // Slave chip
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2+1)
#define PIC_EOI	0x20 // End Of Interrupt CMD	
#define PIC1_CMD PIC1
#define PIC2_CMD PIC2
#define PIC_READ_IRR 0x0a
#define PIC_READ_ISR 0x0b    // OCW3 irq service next CMD read

/*
	pic_remap: Remaps the PIC chips

	Arguments:
		* offset1: vector offset for master PIC
			vectors on the master become offset1..offset1+7
		* offset2: same for slave PIC: offset2..offset2+7
*/
void pic_remap(uint offset1, uint offset2);

/*
  	pic_send_eoi: Sends EOI command, takes IRQ handler numbersa
*/
void pic_send_eoi(uint8_t irq);

/*
	pic_set_mask: Sets mask for given interrupt
*/
void pic_set_mask(uint8_t irq, bool value);

/*
	pic_set_mask: Sets mask for given interrupt
*/




/* Some short functions */

static uint16_t pic_get_irq_reg(int ocw3)
{
	/* 
		OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
	   	represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain 
	*/
	
    asm_outb(PIC1_CMD, ocw3);
    asm_outb(PIC2_CMD, ocw3);
    return (asm_inb(PIC2_CMD) << 8) | asm_inb(PIC1_CMD);
}

inline uint16_t pic_get_irr(void)
{
    return pic_get_irq_reg(PIC_READ_IRR);
}
 
inline uint16_t pic_get_isr(void)
{
    return pic_get_irq_reg(PIC_READ_ISR);
}