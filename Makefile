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
CFLAGS = -std=gnu99 $(GFLAGS) -Wall -Wextra -nostdlib -g

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
# Only build, dont run
build: build/tos.bin

debug: build/tos.bin
	$(QEMU) -kernel build/tos.bin -m 128 -gdb tcp::9000 -S

qdebug: build/tos.bin
	$(QEMU) -kernel build/tos.bin -m 128 -d guest_errors

debug-reset: build/tos.bin
	$(QEMU) -kernel build/tos.bin -m 128 -d cpu_reset,int


# To be used when changing small stuff in .h files or similar
force: clear run

.SECONDEXPANSION:

## COMPILE ALL .C FILES ##

# Useful message to make multile builds easier to read
$(warning ---------------------------------------------------)
$(warning Make Invoked)
$(warning ---------------------------------------------------)


#$(warning $(CFILES) -> $(OCFILES))
#$(warning $(SFILES) -> $(OSFILES))

$(OCFILES): $$(patsubst $(ODIR)/%.o,src/%.c,$$@)
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -c $< -o $@ $(CFLAGS) -lgcc



$(OSFILES): $$(patsubst $(ODIR)/%.o,src/%.s,$$@)
	mkdir -p $(@D)
	$(AS) $< -o $@ -g

# Not sure why this causes circular dependencies
# but it's 4AM and I just learned Makefiles :P
#$(OFILES): $(OCFILES) $(OSFILES)
# Enable if something goes wrong



build/tos.bin: $(OFILES)
	$(CC) -o $@ $(LFLAGS) $^ -lgcc

.PHONY: clear default run

clear: 
	rm -rf build

run: build/tos.bin
	$(QEMU) -kernel build/tos.bin -m 128 -serial file:build/outserial.log




