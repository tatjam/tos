/* 
		util.s - Util Assembly Library

	Useful functions used across the kernel 
	which require to be implemented in assembly,
	or are better that way.


*/

.intel_syntax noprefix

.global asm_cli 
.global asm_sti
.global asm_int

/* Disables interrupts */
asm_cli:
	cli
	ret 

/* Enables interrupts */
asm_sti:
	sti
	ret 

asm_int:
	int 8
	ret