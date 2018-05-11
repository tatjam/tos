/* 
		tables.s - Assembler code for GDT/IDT/... 

	Functions which require being implemented in Assembly
	related to the GDT, IDT, etc...


*/

.intel_syntax noprefix

.global idt_set
.global gdt_set

.align 4


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
.global isr_unhandled_wrap
.global isr_com1_wrap
.global isr_com2_wrap
.global isr_pagefault_wrap
.global isr_irq0_wrap

.align 4


isr_unhandled_wrap:
    pushad
    cld
    call isr_unhandled
	push 4
	call isr_generic_return
	add esp, 4
    popad
   	iretd


isr_com1_wrap:
    pushad
	/* Check for spurious */
	push 0x24
	call isr_is_spurious
	add esp, 4
	cmp eax, 0
	jne isr_com1_wrap_exit
    cld
    call isr_com1
	jmp isr_com1_wrap_exit
isr_com1_wrap_exit:
	push 0x24
	call isr_generic_return
	add esp, 4
    popad
   	iretd


isr_irq0_wrap:
    pushad
	/* Check for spurious */
	push 0x23
	call isr_is_spurious
	add esp, 4
	cmp eax, 0
	jne isr_irq0_wrap_exit
    cld
    call isr_irq0
	jmp isr_irq0_wrap_exit
isr_irq0_wrap_exit:
	push 0x23
	call isr_generic_return
	add esp, 4
    popad
   	iretd


isr_com2_wrap:
    pushad
	/* Check for spurious */
	push 0x23
	call isr_is_spurious
	add esp, 4
	cmp eax, 0
	jne isr_com2_wrap_exit
    cld
    call isr_com2
	jmp isr_com2_wrap_exit
isr_com2_wrap_exit:
	push 0x23
	call isr_generic_return
	add esp, 4
    popad
   	iretd



isr_key_wrap:
    pushad
	/* Check for spurious */
	push 0x21
	call isr_is_spurious
	add esp, 4
	cmp eax, 0
	jne isr_key_wrap_exit
    cld
    call isr_key
	jmp isr_key_wrap_exit
isr_key_wrap_exit:
	push 0x21
	call isr_generic_return
	add esp, 4
    popad
   	iretd


isr_pagefault_wrap:
	pushad
	push 0xE
	call isr_is_spurious
	add esp, 4
	cmp eax, 0
	jne isr_pagefault_wrap_exit
	cld 
	call isr_pagefault
	jmp isr_pagefault_wrap_exit
isr_pagefault_wrap_exit:
	push 0xE
	call isr_generic_return
	add esp, 8
	popad 
	iretd

