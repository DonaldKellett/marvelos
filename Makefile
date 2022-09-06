# Build
CC=riscv64-elf-gcc
CFLAGS=-g -ffreestanding -nostartfiles -nostdlib -nodefaultlibs
CFLAGS+=-Wl,--gc-sections -Wl,-T,src/lds/riscv64-virt.ld
CRT=src/asm/crt0.s
ENTRYPOINT=src/kmain.c
KERNEL_IMAGE=kmain

# QEMU
QEMU=qemu-system-riscv64
MACH=virt
MEM=128M
DBG=tcp::1234
RUN=$(QEMU) -nographic -machine $(MACH) -m $(MEM)
RUN+=-bios none -kernel $(KERNEL_IMAGE)

all:
	$(CC) $(CFLAGS) $(CRT) $(ENTRYPOINT) -o $(KERNEL_IMAGE)

run: all
	$(RUN)

debug: all
	$(RUN) -gdb $(DBG) -S

clean:
	rm -vf $(KERNEL_IMAGE)
