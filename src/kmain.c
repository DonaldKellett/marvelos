#include "uart/uart.h"
#include "syscon/syscon.h"

void kmain(void) {
  uart_init(UART_ADDR);

  // %a % b d%c
  kprintf("%%%c %% %c %c%%c", 'a', 'b', 'd');
  kprintf("\nkprintf is working ... for now ;-)\n");
  kprintf("%c\n", 'e');
  kprintf("Good!\n");

  poweroff();
}
