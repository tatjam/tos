.intel_syntax noprefix


.global page_refresh

page_refresh:
	# Reload crc3 to force a TLB flush so the changes to take effect.
	mov ecx, cr3
	mov cr3, ecx
	ret