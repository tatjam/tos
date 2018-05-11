#pragma once
#include <asmutil.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <tty.h>
/*
		pit.h -  PIT chip functions (Timer)

*/

void pit_tick(void);

/*
	pit_set_freq: Sets the interrupt timer to given frequency
	Frequency must be given in Herts (Hz)
	Make sure interrupt handlers are already initialized
	Keep in mind very small values will error as the divisor will
	bee too big. Keep frequencies over 20
*/
void pit_set_freq(uint32_t freq);

/*
	pit_set_divider: Sets the divider used by the PIT
	1193180 is divided by that number in the hardware

*/
void pit_set_divider(uint16_t div);