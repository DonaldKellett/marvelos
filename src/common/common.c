#include <stdarg.h>
#include "common.h"
#include "../uart/uart.h"

int toupper(int c) {
  return 'a' <= c && c <= 'z' ? c + 'A' - 'a' : c;
}

void panic(const char *format, ...) {
  kputs("Kernel panic!");
  kputs("Reason:");
  va_list arg;
  va_start(arg, format);
  kvprintf(format, arg);
  va_end(arg);
  asm volatile ("wfi");
}
