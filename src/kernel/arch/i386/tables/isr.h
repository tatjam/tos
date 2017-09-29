#pragma once

/*
		isr.h -  Interrupt Service Routines
	
		  Implements multimple ISRs

*/

#include "tables.h"
#include <tty.h>

extern void isr_key_wrap(void);

void isr_key(void);