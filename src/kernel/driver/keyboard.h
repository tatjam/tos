#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <kio.h>

/*
			keyboard.h - PS/2 Keyboard

	Transforms scancodes, handles the lights, etc...

*/

typedef struct keyb_mapping
{
	char normal[128];
	char shift[128];
	char altgr[128];

	uint8_t shift_scans[2];
	uint8_t ctrl_scan;
	uint8_t alt_scan;
	uint8_t altgr_scan;
	uint8_t altgr_nextscan;
	uint8_t caps_lock_scan;
	uint8_t num_lock_scan;
	uint8_t scroll_lock_scan;
} keyb_mapping_t;

typedef struct keyb_status
{
	bool keys[128];

	bool shift_down;
	bool ctrl_down;
	bool altgr_down;
	bool alt_down;
	bool caps_lock;
	bool num_lock;
	bool scroll_lock;
	
} keyb_status_t;

// Defaults to my keyboard :P
extern const keyb_mapping_t keyb_default;


/*
	keyb_interrupt: Called when a keyboard interrupt happens
*/
void keyb_interrupt(uint8_t keycode);

/*
	keyb_status: Obtains the status of the keyboard
*/
keyb_status_t keyb_status();

/*
	keyb_load: Loads given keyb_mapping.
*/
void keyb_load(keyb_mapping_t nmap);


/*
	keyb_map: Transforms a scancode into its actual
		character representation.
*/
char keyb_map(uint8_t scancode);