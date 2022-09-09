#include "uart/uart.h"
#include "syscon/syscon.h"
#include "common/common.h"

void kmain(void) {
  uart_init(UART_ADDR);

  panic("Oopsie - fatal error %p occurred\n"
    "Unrecoverable error; aborting ...",
    (size_t)0xdeadbeef);

  // This should be unreachable
  poweroff();
}
