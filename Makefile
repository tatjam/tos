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
CFLAGS = -std=gnu99 $(GFLAGS) -Wall -Wextra -nostdlib

# Flags for linking
LFLAGS = -T src/kernel/linker.ld -o build/tos.bin $(GFLAGS) -nostdlib

LIBK_INCLUDE = "src/libk"
LIBC_INCLUDE = "src/libc"

# Flags for the C PreProcessor, includes clib and klib
CPPFLAGS = -I$(LIBC_INCLUDE) -I$(LIBK_INCLUDE)

ODIR = build/obj

# Recursive wildcard function
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))


## FIND ALL .C FILES ##
CFILES = $(filter %.c,$(call rwildcard,src/,*c))
#OCFILES = $(CFILES:.c=.o)
OCFILES = $(CFILES:src/%.c=$(ODIR)/%.o)

## FIND ALL .S FILES ##
SFILES = $(filter %.s,$(call rwildcard,src/,*s))
#OSFILES = $(SFILES:.s=.o)
OSFILES = $(SFILES:src/%.s=$(ODIR)/%.o)

OFILES = $(OCFILES) $(OSFILES)

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

default: run

build: build/tos.bin

.SECONDEXPANSION:

## COMPILE ALL .C FILES ##

$(warning $(CFILES) -> $(OCFILES))
$(warning $(SFILES) -> $(OSFILES))

$(OCFILES): $$(patsubst $(ODIR)/%.o,src/%.c,$$@)
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -c $< -o $@ $(CFLAGS)

	
$(OSFILES): $$(patsubst $(ODIR)/%.o,src/%.s,$$@)
	mkdir -p $(@D)
	$(AS) $< -o $@ 

$(OFILES): $(OCFILES) $(OSFILES)



build/tos.bin: $(OFILES)
	$(CC) -o $@ $(LFLAGS) $^

.PHONY: clear default run

clear: 
	rm -rf build

run: build/tos.bin
	$(QEMU) -kernel build/tos.bin -m 128




