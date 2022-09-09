#include "uart/uart.h"
#include "syscon/syscon.h"

void kmain(void) {
  uart_init(UART_ADDR);

  // Signed decimal integers
  kprintf("%d %d %d\n", -2147483648, 0, 2147483647);
  kprintf("%d %d %d\n", -42, -1, 42);
  // Unsigned decimal integers
  kprintf("%u %u %u\n", 0, 0xdeadbeef, 0xffffffff);
  // Unsigned octals
  kprintf("%o %o %o\n", 0, 0xdeadbeef, 0xffffffff);
  // Unsigned hexadecimal integers
  kprintf("%x %x %x\n", 0, 0xdeadbeef, 0xffffffff);
  // Unsigned hexadecimal integers (uppercase)
  kprintf("%X %X %X\n", 0, 0xdeadbeef, 0xffffffff);
  // Characters
  kprintf("%c %c %c\n", 'a', 'B', '!');
  // Strings of characters
  kprintf("%s %s %s\n", "Hello World!", "%c", "%%");
  // Pointer addresses
  kprintf("%p %p %p\n", NULL, (size_t)0xDEADBEEF, -1);
  // Literal '%'
  kprintf("%%\n");

  poweroff();
}
