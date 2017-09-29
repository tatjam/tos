#include "vga.h"


static vga_pos_t width = 80;
static vga_pos_t height = 25;

uint16_t* vmem = (uint16_t*)0xB8000;

bool vga_is_bright(vga_color_t c)
{
	return c >= 8;
}

void vga_init()
{
	vmem = (uint16_t*)0xB8000;
	width = 80;
	height = 25;
}

// We implement this using a simple set of if blocks
// which should be faster than bit setting (but who knows)
vga_color_t vga_set_bright(vga_color_t c, bool bright)
{
	if(vga_is_bright(c))
	{
		if(bright)
		{
			return c;
		}
		else
		{
			return c + 8;
		}
	}
	else
	{
		if(bright)
		{
			return c - 8;
		}
		else
		{
			return c;
		}
	}
}

kret_t vga_set_entry(vga_pos_t i, vga_text_entry_t entry)
{
	// TODO: Check mode
	if(i >= vga_get_width() * vga_get_height())
	{
		return KRET_OFFBOUNDS;
	}
	else
	{
		uint16_t* final = vmem + i;
		*final = (uint16_t)entry.c | (uint16_t)(((entry.fg & 0x0F) | ((entry.bg & 0x0F) << 4)) << 8);

		return KRET_SUCCESS;
	}
}


vga_text_entry_t vga_get_entry(vga_pos_t i)
{
	if(i >= vga_get_width() * vga_get_height())
	{
		vga_text_entry_t out;
		return out;
	}
	else
	{
		uint16_t* final = vmem + i;

		vga_text_entry_t out;

		out.c = *final & 0xFF;
		out.fg = (*final & 0xFF00) >> 8;
		out.bg = (*final & 0xFF00) >> 8;

		out.fg = out.fg & 0x0F;
		out.bg = (out.bg & 0xF0) >> 4;

		return out;
	}
}


vga_pos_t vga_get_width()
{
	return width;
}

vga_pos_t vga_get_height()
{
	return height;
}