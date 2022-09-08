#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include "uart.h"

/*
 * Initialize NS16550A UART
 */
void uart_init(size_t base_addr) {
  volatile uint8_t *ptr = (uint8_t *)base_addr;

  // Set word length to 8 (LCR[1:0])
  const uint8_t LCR = 0b11;
  ptr[3] = LCR;

  // Enable FIFO (FCR[0])
  ptr[2] = 0b1;

  // Enable receiver buffer interrupts (IER[0])
  ptr[1] = 0b1;

  // Assuming clock rate of 22.729 MHz, set signaling rate to 2400 baud
  // divisor = ceil(CLOCK_HZ / (16 * BAUD_RATE))
  //         = ceil(22729000 / (16 * 2400))
  //         = 592
  uint16_t divisor = 592;
  uint8_t divisor_least = divisor & 0xFF;
  uint8_t divisor_most = divisor >> 8;
  ptr[3] = LCR | 0x80;
  ptr[0] = divisor_least;
  ptr[1] = divisor_most;
  ptr[3] = LCR;
}

static uint8_t uart_get(size_t base_addr) {
  volatile uint8_t *ptr = (uint8_t *)base_addr;

  // LSR[0] is data ready (DR)
  return (ptr[5] & 0b1) && ptr[0] || 0;
}

static void uart_put(size_t base_addr, uint8_t c) {
  volatile uint8_t *ptr = (uint8_t *)base_addr;
  ptr[0] = c;
}

int kputchar(int character) {
  uart_put(UART_ADDR, (uint8_t)character);
  return character;
}

int kputs(const char *str) {
  while (*str) {
    kputchar((int)*str);
    ++str;
  }
  kputchar((int)'\n');
  return 0;
}

// Limited version of vprintf() which only supports the following specifiers:
//
// - d/i: Signed decimal integer
// - u: Unsigned decimal integer
// - o: Unsigned octal
// - x: Unsigned hexadecimal integer
// - X: Unsigned hexadecimal integer (uppercase)
// - c: Character
// - s: String of characters
// - p: Pointer address
// - %: Literal '%'
//
// None of the sub-specifiers are supported for the sake of simplicity.
// The `n` specifier is not supported since that is a major source of
// security vulnerabilities. None of the floating-point specifiers are
// supported since floating point operations don't make sense in kernel
// space
//
// Anyway, this subset should suffice for printf debugging
static void kvprintf(const char *format, va_list arg) {
  while (*format) {
    if (*format == '%') {
      ++format;
      if (!*format)
	return;
      switch (*format) {
      case 'd':
      case 'i':
	// TODO
	break;
      case 'u':
	// TODO
	break;
      case 'o':
	// TODO
	break;
      case 'x':
	// TODO
	break;
      case 'X':
	// TODO
	break;
      case 'c':
	kputchar(va_arg(arg, int));
	break;
      case 's':
	// TODO
	break;
      case 'p':
	// TODO
	break;
      case '%':
	kputchar('%');
	break;
      }
    } else
      kputchar(*format);
    ++format;
  }
}

void kprintf(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  kvprintf(format, arg);
  va_end(arg);
}
