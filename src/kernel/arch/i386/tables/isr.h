#pragma once

/*
		isr.h -  Interrupt Service Routines
	
		  Implements multimple ISRs

*/

#include "tables.h"
#include "../pic.h"
#include <tty.h>
#include "../serial.h"
#include "../../../driver/keyboard.h"

extern void isr_key_wrap(void);
extern void isr_unhandled_wrap(void);
extern void isr_com1_wrap(void);
extern void isr_com2_wrap(void);
extern void isr_pagefault_wrap(void);
extern void isr_irq0_wrap(void);

void isr_pagefault(uint32_t code, uint32_t cr2);

void isr_key(void);

void isr_unhandled(void);

void isr_com1(void);
void isr_com2(void);

void isr_irq0(void);

/*
	isr_generic_return: Handles PIC from all interrupt types

	Arguments:
		* inum: Interrupt number 
*/
void isr_generic_return(uint8_t inum);

/*
	isr_is_spurious: Checks if interrupt is spurious

	Arguments:
		* inum: Interrupt number 
	
	Returns:
		* 1 if spurious, 0 if not
*/
uint8_t isr_is_spurious(uint8_t inum);