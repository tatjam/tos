// es_ES mapping
#include "../keyboard.h"

const keyb_mapping_t keyb_default =
{
	.normal = "12 34567890'¡\b\tqwertyuiop`+\n?asdfghjkln´\r\rzxcvbnm,.-\r",
	.shift =  "!\"·$%&/()=?¿\b\tQWERTYUIOP^*\n?ASDFGHJKLN¨\r\rZXCVBNM;:_\r",
	.altgr =  "|@ #~€¬\r\r\r\r\r\r\r\r\r\r€\r\r\r\r\r\r\r[]\r?\r\r\r\r\r\r\r\r\r\r{\r\r{",

	.shift_scans = {42, 54},
	.ctrl_scan = 29,
	.altgr_scan = 29, // The same as ctrl but followed by this
	.altgr_nextscan = 56,
	.alt_scan = 56,
	.caps_lock_scan = 58,
	.num_lock_scan = 69,
	.scroll_lock_scan = 70,
};