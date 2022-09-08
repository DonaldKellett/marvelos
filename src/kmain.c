#include "uart/uart.h"
#include "syscon/syscon.h"

void kmain(void) {
  uart_init(UART_ADDR);

  kputs("UART initialized successfully");
  kputs("Hello RISC-V World!");
  kputs("Hello OSDev World!");
  kputs("We are in M-mode!");

  poweroff();
}
