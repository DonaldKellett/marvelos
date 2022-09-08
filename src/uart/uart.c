#include <stddef.h>
#include <stdint.h>
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
