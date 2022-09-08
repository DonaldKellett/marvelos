#include <stdint.h>
#include "syscon.h"
#include "../uart/uart.h"

void poweroff(void) {
  kputs("Poweroff requested");
  *(uint32_t *)SYSCON_ADDR = 0x5555;
}

void reboot(void) {
  kputs("Reboot requested");
  *(uint32_t *)SYSCON_ADDR = 0x7777;
}
