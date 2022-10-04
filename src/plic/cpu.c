#include <stddef.h>
#include "cpu.h"
#include "../common/common.h"

// Set timer interrupt to fire `us` microseconds from now
void set_timer_interrupt_delay_us(size_t us) {
  *(size_t *)MTIMECMP_ADDR =
      *(size_t *)MTIME_ADDR + us * (TICKS_PER_SECOND / US_PER_SECOND);
}
