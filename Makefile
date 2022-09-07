# Build
CC=riscv64-elf-gcc
CFLAGS=-ffreestanding -nostartfiles -nostdlib -nodefaultlibs
CFLAGS+=-g -Wl,--gc-sections
RUNTIME=src/asm/crt0.s
ENTRYPOINT=src/kmain.c
LINKER_SCRIPT=src/lds/riscv64-virt.ld
KERNEL_IMAGE=kmain

# QEMU
QEMU=qemu-system-riscv64
MACH=virt
MEM=128M
RUN=$(QEMU) -nographic -machine $(MACH) -m $(MEM)
RUN+=-bios none -kernel $(KERNEL_IMAGE)

# QEMU (debug)
GDB_PORT=1234

all: kmain
	$(CC) $(CFLAGS) $(RUNTIME) *.o -T $(LINKER_SCRIPT) -o $(KERNEL_IMAGE)

kmain:
	$(CC) -c $(CFLAGS) $(ENTRYPOINT) -o kmain.o

run: all
	$(RUN)

debug: all
	$(RUN) -gdb tcp::$(GDB_PORT) -S

clean:
	rm -vf *.o
	rm -vf $(KERNEL_IMAGE)
