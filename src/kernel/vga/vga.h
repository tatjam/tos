#pragma once


/*
			vga.h - VGA driver

	TTY.h handles a low level terminal which interacts directly
	with VGA. It works on VGA text mode only.

	Everything this file contains is prefixed with 'vga'

*/

#include <errdef.h>
#include <asmutil.h>

/* Type used to define positions in the terminal */
typedef uint32_t vga_pos_t;

/* Type used to define a 8-bit VGA color */
typedef uint8_t vga_color_t;

/* A single character + color of text-mode VGA */
typedef struct vga_text_entry
{
	char c;
	vga_color_t fg;
	vga_color_t bg;

} vga_text_entry_t;


typedef struct vga_cursor
{
	vga_pos_t x, y;
	bool visible;

} vga_cursor_t;



#define VGA_BRIGHT(x) vga_set_bright(x, true)
#define VGA_DARK(x) vga_set_bright(x, false)

/* Contains the base colors, not including bright colors */
enum vga_basecolor
{
	VGA_BLACK = 0,
	VGA_BLUE = 1,
	VGA_GREEN = 2,
	VGA_CYAN = 3,
	VGA_RED = 4,
	VGA_MAGENTA = 5,
	VGA_BROWN = 6,
	VGA_GRAY = 7
};

/**************************************************************
	vga_init - Inits the VGA internal system
*/
void vga_init();

/**************************************************************
	vga_set_bright - Changes bright bit of color

	Given a valid color, will change the bright parameter

*/
vga_color_t vga_set_bright(vga_color_t c, bool bright);

/**************************************************************
	vga_is_bright - Check bright bit of color

	Returns false if color is not bright and true if it is

*/
bool vga_is_bright(vga_color_t c);

/**************************************************************
	vga_set_entry - Sets a single entry of the terminal

	Can return error code KRET_OFFBOUNDS if given offbounds index,
	KRET_INVALIDMODE if called not in compatible mode

*/
kret_t vga_set_entry(vga_pos_t i, vga_text_entry_t entry);

vga_text_entry_t vga_get_entry(vga_pos_t i);

vga_pos_t vga_get_width();
vga_pos_t vga_get_height();

void vga_upload_cursor(vga_cursor_t cursor);