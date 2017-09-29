#pragma once


/*
			tty.h - Low level terminal 

	TTY.h handles a low level terminal which interacts directly
	with VGA. It works on VGA text mode only.

	Everything this file contains is prefixed with 'ktty'

	Special Codes:

	\n -  Skips to next line, setting cursor.x to 0.
	\t - Tabulator (4 spaces by default)
	\b - Delete char
	\xF[x] - Sets foreground color to [x]
	\xB[x] - Sets background color to [x]

*/

#include "errdef.h"
#include "../kernel/vga/vga.h"

#include <stdarg.h>



typedef struct ktty_cursor
{
	vga_pos_t x, y;
	bool visible;

} ktty_cursor_t;



/**************************************************************
	ktty_init - Initializes the terminal

	Must be called before all other operations or they
	may not work properly.
*/
void ktty_init();

/**************************************************************
	ktty_cursor - Obtains the global cursor

	Returns a pointer to the terminal cursor, which can
	be modified freely, reflecting changes in the temrminal

*/
ktty_cursor_t* ktty_cursor();

/**************************************************************
	ktty_set - Sets character to given

	Sets a cell of the terminal to given char. 
	Invalid coordinates will cause no change to the
	terminal, but will return an error code.

	Returns either KRET_SUCCESS or KRET_OFFBOUNDS

*/
kret_t ktty_set(vga_pos_t x, vga_pos_t y, vga_text_entry_t to);

/**************************************************************
	ktty_index - Gets index of 2D coordinates

	Even if given invalid coordinates it will return
	a correct value (assuming that the terminal was bigger).

*/
vga_pos_t ktty_index(vga_pos_t x, vga_pos_t y);

void ktty_set_active_color(vga_color_t fg, vga_color_t bg);

vga_color_t ktty_get_active_fg();
vga_color_t ktty_get_active_bg();

/**************************************************************
	ktty_pack_color - Gets colors as stored in the VGA buffer
*/
uint8_t ktty_pack_color(vga_color_t fg, vga_color_t bg);

/**************************************************************
	ktty_unpack_color - Unpacks packed color (ktty_pack_color) into given pointers

	You can leave any pointer NULL and it will not be written to
*/
void ktty_unpack_color(uint8_t packed, vga_color_t* fg, vga_color_t* bg);

void ktty_clear();

/**************************************************************
	ktty_puts - Writes NUL-terminated strng. Handles special codes

	Returns number of characters written
*/

size_t ktty_puts(char* str);
 
/**************************************************************
	ktty_putf - printf of ktty. 

	Formatting (not the standard printf but similar): 

	'%%' - '%' 
	'%c' - character (char)
	'%s' - NUL terminated string (char*)
	'%i' - signed integer (int)
	'%u' - unsigned integer (uint)
	'%x' - unsigned integer as hexa (uint)
	'%f' - floating point number (float)
	'%n' - number of characters written (size_t*)
	'%p' - pointer ([x]*)
	'%a' - Changes (a)ctive color foreground to given
	'%A' - Changes (A)ctive color background to given
	'%C' - Changes active color to given packed color (ktty_pack_color)
*/
void ktty_putf(char *fmt, ...);

/**************************************************************
	ktty_putc - Writes single character. Handles special codes
*/

void ktty_putc(char c);

/**************************************************************
	ktty_scroll - Scrolls a single line
*/

void ktty_scroll();

/**************************************************************
	ktty_get_clear - obtains the clear entry
*/
vga_text_entry_t ktty_get_clear();