#include <stddef.h>
#include "trap_frame.h"

static struct trap_frame KERNEL_TRAP_FRAME = ZERO_TRAP_FRAME;

struct trap_frame *get_kernel_trap_frame(void) {
  return &KERNEL_TRAP_FRAME;
}
