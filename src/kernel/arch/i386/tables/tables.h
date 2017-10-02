#pragma once


/*
		tables.h -  C programs for GDT/IDT/etc
	
		  Code to handle GTD, IDT, etc...
		  Parts implemented in assembler

	Functions are prefixed with the area they work on

*/

#include <stddef.h>
#include <stdint.h>
#include "gdtdef.h"

#include <tty.h>

#include "isr.h"

/* Dummy GDT table with up to 8 descriptors */
typedef struct GDT 
{
	uint64_t descriptors[8];
} GDT_t;


typedef struct idt_desc 
{
	uint16_t offset_low:16; // offset bits 0..15
	uint16_t selector:16; // a code segment selector in GDT or LDT
	uint8_t zero:8;      // set to 0

	uint8_t type:5;
	uint8_t dpl:2;
	uint8_t present:1;

	uint16_t offset_high:16; // offset bits 16..31

 } idt_desc_t;

#define IDT_DESC_COUNT 256

typedef struct IDT 
{
	idt_desc_t descriptors[IDT_DESC_COUNT];
} IDT_t;

typedef void (*ihandler_t)(void);


/*
  	Loads a IDT, takes a pointer to the table and 
	the size of the table 
*/
extern void idt_set(void* offset, size_t size);

/*
	Creates a single descriptor. Thanks OSDev
*/
uint64_t gdt_descriptor(uint32_t base, uint32_t limit, uint16_t flag);


/* Prepares a dummy GDT, loads it and returns it */
GDT_t* gdt_prepare();

/* Prepares an empty IDT, loads it and returns it*/
IDT_t* idt_prepare();

/* Configures all IDT stuff to defaults, loads ISR */
void idt_configure();

/* Loads a handler function into given interrupt */
void idt_load_handler(uint8_t inum, ihandler_t handler);

/*
  	Loads a GDT, takes a pointer to the table and 
	the size of the table 
*/
extern void gdt_set(void* offset, size_t size);