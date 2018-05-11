#include "tables.h"

static GDT_t gdt;
static IDT_t idt;

GDT_t* gdt_prepare()
{
	gdt.descriptors[0] = gdt_descriptor(0, 0, 0);
	gdt.descriptors[1] = gdt_descriptor(0, 0xFFFFFFFF, (GDT_CODE_PL0));
    gdt.descriptors[2] = gdt_descriptor(0, 0xFFFFFFFF, (GDT_DATA_PL0));
    
	gdt_set(&gdt, sizeof(gdt) - 1);

	return &gdt;
}


uint64_t gdt_descriptor(uint32_t base, uint32_t limit, uint16_t flag)
{
    uint64_t descriptor;
 
    // Create the high 32 bit segment
    descriptor  =  limit       & 0x000F0000;         // set limit bits 19:16
    descriptor |= (flag <<  8) & 0x00F0FF00;         // set type, p, dpl, s, g, d/b, l and avl fields
    descriptor |= (base >> 16) & 0x000000FF;         // set base bits 23:16
    descriptor |=  base        & 0xFF000000;         // set base bits 31:24
 
    // Shift by 32 to allow for low part of segment
    descriptor = descriptor << 32;
 
    // Create the low 32 bit segment
    descriptor |= base  << 16;                       // set base bits 15:0
    descriptor |= limit  & 0x0000FFFF;               // set limit bits 15:0
 
    return descriptor;
}


void idt_load_handler(uint8_t inum, ihandler_t handler)
{
    uint32_t offset = (uint32_t)handler;

    idt.descriptors[inum].present = 1;

    idt.descriptors[inum].offset_low = offset & 0xFFFF;
    idt.descriptors[inum].offset_high = (offset >> 16) & 0xFFFF;

    idt.descriptors[inum].type = 0xE;

    // What to set selector to?
    idt.descriptors[inum].selector = 8;
}

IDT_t* idt_prepare()
{
    /* Zero that thing */
    for(size_t i = 0; i < IDT_DESC_COUNT; i++)
    {
        idt.descriptors[i].offset_high = 0;
        idt.descriptors[i].offset_low = 0;
        idt.descriptors[i].zero = 0;
        idt.descriptors[i].type = 0;
        idt.descriptors[i].present = 0;
        idt.descriptors[i].dpl = 0;
        idt.descriptors[i].selector = 0;
    }


    idt_set(&idt, sizeof(idt) - 1);

    return &idt;
}

void idt_configure()
{
    pic_remap(PIC1_OFFSET, PIC2_OFFSET);
    
    for(size_t i = 0; i < 256; i++)
    {
        idt_load_handler(i, isr_unhandled_wrap);
    }

    idt_load_handler(PIC1_OFFSET + 1, isr_key_wrap);
    idt_load_handler(PIC1_OFFSET + 3, isr_com2_wrap);
    idt_load_handler(PIC1_OFFSET + 4, isr_com1_wrap);
    idt_load_handler(32, isr_irq0_wrap); //32 is irq0
    idt_load_handler(0xE, isr_pagefault_wrap);
}