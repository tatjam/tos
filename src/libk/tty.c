#include "tty.h"

static vga_cursor_t cursor;

static vga_color_t active_fg;
static vga_color_t active_bg;

char numbuff[128] = {0};

void ktty_init()
{
	vga_init();

	// Initialize cursor
	cursor.x = 0;
	cursor.y = 0;
	cursor.visible = true;

	active_fg = VGA_GRAY;
	active_bg = VGA_BLACK;
}

kret_t ktty_set(vga_pos_t x, vga_pos_t y, vga_text_entry_t to)
{
	if(x >= vga_get_width() || y >= vga_get_height())
	{
		return KRET_OFFBOUNDS;
	}
	else
	{
		vga_set_entry(ktty_index(x, y), to);
		return KRET_SUCCESS;
	}
}

vga_pos_t ktty_index(vga_pos_t x, vga_pos_t y)
{
	return y * vga_get_width() + x;
}


void ktty_set_active_color(vga_color_t fg, vga_color_t bg)
{
	active_fg = fg;
	active_bg = bg;
}

vga_color_t ktty_get_active_fg()
{
	return active_fg;
}

vga_color_t ktty_get_active_bg()
{
	return active_bg;
}

size_t ktty_puts(char* c)
{
	size_t put = 0;

	while(*c != 0)
	{
		ktty_putc(*c);
		c++;
		put++;
	}

	return put;
}

void ktty_putc(char c)
{
	if(c == '\n')
	{
		cursor.y++;
		cursor.x = 0;
	}
	else if(c == '\t')
	{
		cursor.x += 4;
	}
	else if(c == '\b')
	{
		// TODO: Delete char
		cursor.x -= 1;
	}
	else if(c < 0)
	{
		// Special codes
		uint8_t lower = c & 0x0F;
		uint8_t higher = c & 0xF0;
		higher = higher >> 4;

		if(higher == 0xF)
		{
			// Set FG
			active_fg = lower;
		}
		else if(higher == 0xB)
		{
			// Set BG
			active_bg = lower;
		}
	}
	else
	{
		// Write character and move cursor
		vga_text_entry_t entry;
		entry.c = c; entry.fg = active_fg; entry.bg = active_bg;

		ktty_set(cursor.x, cursor.y, entry);

		cursor.x++;
	}

	if(cursor.x >= vga_get_width())
	{
		cursor.x = 0;
		cursor.y++;
		
	}

	if(cursor.y >= vga_get_height())
	{
		cursor.x = 0;
		// Scroll
		ktty_scroll();
	}

	vga_upload_cursor(cursor);

}

/**************************************************************
	ktty_pack_color - Gets color as stored in the VGA buffer
*/
uint8_t ktty_pack_color(vga_color_t fg, vga_color_t bg)
{
	return ((fg & 0x0F) | ((bg & 0x0F) << 4));
}

/**************************************************************
	ktty_unpack_color - Unpacks packed color (ktty_pack_color) into given pointers

	You can leave any pointer NULL and it will not be written to
*/
void ktty_unpack_color(uint8_t packed, vga_color_t* fg, vga_color_t* bg)
{
	vga_color_t tfg, tbg;

	// FG is lower 4 bits
	// BG is higher 4 bits

	tfg = packed & 0x0F;
	tbg = (packed & 0xF0) >> 4;

	if(fg != NULL)
	{
		*fg = tfg;
	}
	
	if(bg != NULL)
	{
		*bg = tbg;
	}
}


void ktty_scroll()
{
	cursor.y--;
	
	for(vga_pos_t y = 0; y < vga_get_height(); y++)
	{
		for(vga_pos_t x = 0; x < vga_get_width(); x++)
		{
			if(y == vga_get_height() - 1)
			{
				// Set to clear
				vga_text_entry_t clear = ktty_get_clear();
				if(ktty_set(x, y, clear) != KRET_SUCCESS)
				{
					klog("[ERROR] VGA width invalid. Make sure you have initialized VGA");
				}
			}
			else
			{
				// Read from line below
				vga_text_entry_t below = vga_get_entry(ktty_index(x, y + 1));

				if(ktty_set(x, y, below) != KRET_SUCCESS)
				{
					klog("[ERROR] VGA width invalid. Make sure you have initialized VGA");
				}

			}
		}

	}
}

vga_text_entry_t ktty_get_clear()
{
	vga_text_entry_t clear; clear.c = ' '; clear.fg = active_fg; clear.bg = active_bg;
	return clear;
}

void ktty_clear()
{
	vga_text_entry_t clear = ktty_get_clear();

	for(vga_pos_t y = 0; y < vga_get_height(); y++)
	{
		for(vga_pos_t x = 0; x < vga_get_width(); x++)
		{
			vga_set_entry(ktty_index(x, y), clear);
		}
	}

	cursor.x = 0;
	cursor.y = 0;

}


/**************************************************************
	ktty_putf - printf of ktty. 

	Formatting (not the standard printf but similar): 

	'%%' - '%' 
	'%c' - character (char)
	'%s' - NUL terminated string (char*)
	'%i' - signed integer (int32_t)
	'%u' - unsigned integer (uint32_t)
	'%x' - unsigned integer as hexa (uint32_t)
	'%f' - floating point number (float)
	'%n' - number of characters written (size_t*)
	'%p' - pointer ([x]*)
	'%a' - Changes (a)ctive color foreground to given
	'%A' - Changes (A)ctive color background to given
	'%C' - Changes active color to given packed color (ktty_pack_color)
*/
void ktty_putf(char *fmt, ...)
{
	va_list parameters;
	va_start(parameters, fmt);

	ktty_putf_list(fmt, &parameters);

	va_end(parameters);
}

void ktty_putf_list(char* fmt, va_list* parameters)
{
	size_t written = 0;

	size_t i = 0;

	while(fmt[i] != 0)
	{
		if(fmt[i] == '%')
		{
			i++;
			
			// Begin a fmt
			if(fmt[i] == '%')
			{
				ktty_putc('%');
				written++;
			}
			else if(fmt[i] == 'c')
			{
				char c = (char)va_arg(*parameters, int);
				ktty_putc(c);
				written++;
			}
			else if(fmt[i] == 's')
			{
				// Put string
				char* str = va_arg(*parameters, char*);
				size_t w = ktty_puts(str);
				written += w;
			}
			else if(fmt[i] == 'i')
			{
				int num = va_arg(*parameters, int); 
				kitoa_buf(numbuff, sizeof(numbuff), num, 10);
				size_t w = ktty_puts(numbuff);
				written += w;
			}
			else if(fmt[i] == 'u')
			{
				uint num = va_arg(*parameters, uint);
				kutoa_buf(numbuff, sizeof(numbuff), num, 10);
				size_t w = ktty_puts(numbuff);
				written += w;
			}
			else if(fmt[i] == 'x')
			{
				uint num = va_arg(*parameters, uint);
				kutoa_buf(numbuff, sizeof(numbuff), num, 16);
				size_t w = ktty_puts(numbuff);
				written += w;
			}
			else if(fmt[i] == 'f')
			{
				float num = (float)va_arg(*parameters, double);
				ktty_putc((char)num);
			}
			else if(fmt[i] == 'n')
			{
				size_t* target = va_arg(*parameters, size_t*);

				*target = written;
			}
			else if(fmt[i] == 'p')
			{
				void* ptr = va_arg(*parameters, void*);
				kutoa_buf(numbuff, sizeof(numbuff), (uint)ptr, 16);
				size_t w = ktty_puts(numbuff);
				written += w;
			}
			else if(fmt[i] == 'a')
			{
				vga_color_t fg = (vga_color_t)va_arg(*parameters, uint);
				ktty_set_active_color(fg, ktty_get_active_bg());
			}
			else if(fmt[i] == 'A')
			{
				vga_color_t bg = (vga_color_t)va_arg(*parameters, uint);
				ktty_set_active_color(ktty_get_active_fg(), bg);
			}
			else if(fmt[i] == 'C')
			{
				uint8_t packed = (uint8_t)va_arg(*parameters, uint);
				vga_color_t fg, bg;
				ktty_unpack_color(packed, &fg, &bg);
				ktty_set_active_color(fg, bg);

			}
		}
		else
		{
			ktty_putc(fmt[i]);
			written++;
		}

		i++;
	}
}