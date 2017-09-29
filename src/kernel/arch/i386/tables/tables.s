/* 
		tables.s - Assembler code for GDT/IDT/... 

	Functions which require being implemented in Assembly
	related to the GDT, IDT, etc...


*/

.intel_syntax noprefix

.global idt_set
.global gdt_set


/* Dummy structure */
gdt_info:
    .int 0  /* Size */
    .long 0  /* Offset (address) */

/* Loads the GDT from given address and size */
gdt_set:
	mov eax, [esp + 4]
	mov [gdt_info + 2], eax 
	mov ax, [esp + 8]
	mov [gdt_info], ax
	lgdt [gdt_info]

	/* Seems to be required */
	call reload_segments

	ret 

reload_segments:
   /* Reload CS register containing code selector */
   jmp   0x08:reload_cs /* 0x08 points at the new code selector */

reload_cs:
   /* Reload data segment registers: */
   mov ax, 0x10 /* 0x10 points at the new data selector */
   mov ds, ax 
   mov es, ax 
   mov fs, ax 
   mov gs, ax 
   mov ss, ax
   ret


/* Dummy structure */
idt_info:
    .int 0  /* Size */
    .long 0  /* Offset (address) */

/* Loads the IDT from given address and size */
idt_set:
	mov eax, [esp + 4]
	mov [idt_info + 2], eax 
	mov ax, [esp + 8]
	mov [idt_info], ax
	lidt [idt_info]
	ret 

/* Interrupt handlers that call C code */

.global isr_key_wrap

isr_key_wrap:
    pushad
    cld
    call isr_key
    popad
    iret



