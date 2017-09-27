##################################################
#      TOS Makefile - Could use improvement      #
##################################################


# Compiler path/name
# (Set this to i686-elf-gcc on linux, or to your path to the cross compiler on windows)
CC = C:/MinGW/ghost-i686-elf-tools-win32/ghost-i686-elf-tools/bin/i686-elf-gcc.exe

# Assembler path/name
# (Set this to i686-elf-as on linux, or to your path to the cross compiler on windows)
AS = C:/MinGW/ghost-i686-elf-tools-win32/ghost-i686-elf-tools/bin/i686-elf-as.exe

# QEMU path/name
# (Set this to the path to qemu, or simply qemu-system-i386 in linux)
QEMU = "C:/Program Files/qemu/qemu-system-i386.exe"


# General flags. Used widely
GFLAGS = -ffreestanding -O2

# Flags for compiling
CFLAGS = $(CPPFLAGS) $(GFLAGS) -std=gnu99  -Wall -Wextra

# Flags for linking
LFLAGS = -T src/kernel/linker.ld -o build/tos.bin $(GFLAGS) -nostdlib $(OBJFULL_ALL) -lgcc

LIBK_INCLUDE = "src/kernel/libk"
LIBC_INCLUDE = "src/libc"

# Flags for the C PreProcessor, includes clib and klib
CPPFLAGS = -I$(LIBC_INCLUDE) -I$(LIBK_INCLUDE)

ODIR = build/obj

# Weird stuff ahead that showcases my lack of 'make' knowledge

ALL_C = $(KERNEL_C)
ALL_S = $(KERNEL_S)

ALL_O_C = $(ALL_C:.c=.o)
ALL_O_S = $(ALL_S:.s=.o)

ALL_O = $(ALL_O_C) $(ALL_O_S)

OBJDIRS = $(basename $(OBJFULL_ALL))
OBJFULL = $(patsubst %, $(ODIR)/%, $(ALL_O_C))
OBJFULL_SO = $(patsubst %, $(ODIR)/%, $(ALL_O_S))
OBJFULL_ALL = $(OBJFULL) $(OBJFULL_SO)

#################################################
# KERNEL FILES								    #
#################################################
KERNEL_H = src/kernel/multiboot/multiboot.h
KERNEL_C = src/kernel/kernel.c
KERNEL_S = src/kernel/arch/i386/boot.s


.PHONY: all 
# If called with no arguments will call wathever task is there:
all: run


# Makes all .o files coming from .c files
$(OBJFULL): $(KERNEL_C)
	@echo "Make_C"
	$(CC) -c $< -o $@ $(CFLAGS)

# Makes all .o files coming from .s files
$(OBJFULL_SO): $(KERNEL_S)
	@echo "Make ASM"
	$(AS) $< -o $@ 


# Makes all folders for build
.PHONY: folders
folders: $(OBJDIRS)

$(OBJDIRS): 
	mkdir -p $@


# Make targets


.PHONY: kernel
kernel: folders $(OBJFULL) $(OBJFULL_SO)  
	$(CC) $(LFLAGS)

.PHONY: run
run: kernel
	$(QEMU) -kernel build/tos.bin -m 128

.PHONY: clear
clear:
	rm -rf build