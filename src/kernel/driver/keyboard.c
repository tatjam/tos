#include "keyboard.h"

keyb_mapping_t map;
static keyb_status_t status;

static uint8_t instant_prev;


static inline void keyb_special_cases(uint8_t keycode)
{
	if(keycode == map.altgr_nextscan && instant_prev == map.altgr_scan + 128){status.altgr_down = true;}
	else if(keycode == map.altgr_nextscan + 128 && instant_prev == map.altgr_scan + 128){status.altgr_down = false;
	}
	else if(keycode == map.shift_scans[0] || keycode == map.shift_scans[1]){status.shift_down = true;}
	else if(keycode == map.shift_scans[0] + 128 || keycode == map.shift_scans[1] + 128){status.shift_down = false;}

	else if(keycode == map.ctrl_scan){status.ctrl_down = true;}
	else if(keycode == map.ctrl_scan + 128){status.ctrl_down = false;}

	else if(keycode == map.alt_scan){status.alt_down = true;}
	else if(keycode == map.alt_scan + 128){status.alt_down = false;}

	// Toggle keys
	else if(keycode == map.caps_lock_scan){status.caps_lock = !status.caps_lock;}
	else if(keycode == map.num_lock_scan){status.num_lock = !status.num_lock;}
	else if(keycode == map.scroll_lock_scan){status.scroll_lock = !status.scroll_lock;}
}

void keyb_interrupt(uint8_t keycode)
{
	keyb_special_cases(keycode);

	if(keycode < 128)
	{
		status.keys[keycode] = true;
	}
	else
	{
		status.keys[keycode] = false;
	}

	instant_prev = keycode;
	klog("[%u]", keycode);
}


void keyb_load(keyb_mapping_t nmap)
{
	map = nmap;
}

char keyb_map(uint8_t scancode)
{
	if(scancode > 128)
	{
		scancode -= 128;
	}

	return map.normal[scancode];
}