#include "trap_handler.h"
#include "../uart/uart.h"
#include "../common/common.h"
#include "cpu.h"

size_t m_mode_trap_handler(size_t epc, size_t tval, size_t cause, size_t hart,
			   size_t status, struct trap_frame *frame) {
  size_t return_pc = epc;
  size_t exception_code = CAUSE_EXCEPTION_CODE(cause);
  if (CAUSE_IS_INTERRUPT(cause)) {
    kprintf("Asynchronous trap\n");
    switch (exception_code) {
    case 7:
      kprintf("Machine timer interrupt\n");
      set_timer_interrupt_delay_us(1 * US_PER_SECOND);
      break;
    default:
      kprintf("Exception code: %d\n", exception_code);
    }
  } else {
    kprintf("Synchronous trap\n");
    return_pc += 4;
    switch (exception_code) {
    case 13:
      kprintf("Load page fault: attempted to dereference address %p\n", tval);
      break;
    case 15:
      kprintf("Store/AMO page fault: attempted to dereference address %p\n",
	      tval);
      break;
    default:
      kprintf("Exception code: %d\n", exception_code);
    }
  }
  return return_pc;
}
