#pragma once


/*
			errdef.h - Helpers for erro handling

	errdef.h standarizes error returns across the libk
	Everything is prefixed with KRET

*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "libkdef.h"

#define kerr(x) kerror(x)

/* kret_t is a generic error return type */
typedef uint16_t kret_t;

// Function for logging errors properly. Use the macro instead
void kerror(char msg[128]);

/* No error ocurred */
#define KRET_SUCCESS 0

/* Offbound error occurred */
#define KRET_OFFBOUNDS 1

/* Invalid mode of wathever (VGA, etc...) */
#define KRET_INVALIDMODE 2