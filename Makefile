# Build
CC=riscv64-elf-gcc
CFLAGS=-ffreestanding -nostartfiles -nostdlib -nodefaultlibs
CFLAGS+=-g -Wl,--gc-sections -mcmodel=medany -march=rv64g
CFLAGS+=-Wl,--no-warn-rwx-segments
RUNTIME=src/asm/crt0.s
LINKER_SCRIPT=src/lds/riscv64-virt.ld
KERNEL_IMAGE=kmain

# QEMU
QEMU=qemu-system-riscv64
MACH=virt
RUN=$(QEMU) -nographic -machine $(MACH)
RUN+=-bios none -kernel $(KERNEL_IMAGE)

# Format
INDENT_FLAGS=-linux -brf -i2

all: uart syscon common mm plic process kmain
	$(CC) *.o $(RUNTIME) $(CFLAGS) -T $(LINKER_SCRIPT) -o $(KERNEL_IMAGE)

uart:
	$(CC) -c src/uart/uart.c $(CFLAGS) -o uart.o

syscon:
	$(CC) -c src/syscon/syscon.c $(CFLAGS) -o syscon.o

common:
	$(CC) -c src/common/common.c $(CFLAGS) -o common.o

mm:
	$(CC) -c src/mm/page.c $(CFLAGS) -o page.o
	$(CC) -c src/mm/sv39.c $(CFLAGS) -o sv39.o
	$(CC) -c src/mm/kmem.c $(CFLAGS) -o kmem.o

plic:
	$(CC) -c src/plic/trap_frame.c $(CFLAGS) -o trap_frame.o
	$(CC) -c src/plic/cpu.c $(CFLAGS) -o cpu.o
	$(CC) -c src/plic/trap_handler.c $(CFLAGS) -o trap_handler.o
	$(CC) -c src/plic/plic.c $(CFLAGS) -o plic.o

process:
	$(CC) -c src/process/syscall.c $(CFLAGS) -o syscall.o
	$(CC) -c src/process/process.c $(CFLAGS) -o process.o
	$(CC) -c src/process/sched.c $(CFLAGS) -o sched.o

kmain:
	$(CC) -c src/kmain.c $(CFLAGS) -o kmain.o

run: all
	$(RUN)

debug: all
	$(RUN) -s -S

format:
	find . -name '*.h' -exec indent $(INDENT_FLAGS) '{}' \;
	find . -name '*.c' -exec indent $(INDENT_FLAGS) '{}' \;
	find . -name '*.h' -exec sed -i -r 's/(0) (b[01]+)/\1\2/g' '{}' \;
	find . -name '*.c' -exec sed -i -r 's/(0) (b[01]+)/\1\2/g' '{}' \;
	echo "You should now \`make run\` to confirm the project still builds and runs correctly"

clean:
	rm -vf *.o
	rm -vf $(KERNEL_IMAGE)
	find . -name '*~' -exec rm -vf '{}' \;
