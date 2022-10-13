#include "syscall.h"
#include "../common/common.h"
#include "../uart/uart.h"

size_t do_syscall(size_t mepc, struct trap_frame *frame) {
  // a0 = x10
  size_t syscall_number = frame->regs[10];
  switch (syscall_number) {
  case 0:
    // TODO: Program exit
    PANIC("do_syscall(): exit() system call not implemented (syscall %d)\n",
	  syscall_number);
  case 1:
    // Test syscall
    kprintf("Test syscall\n");
    return mepc + 4;
  default:
    // FIXME: handle this gracefully as errors in user space should not
    // bring down the system
    PANIC("do_syscall(): unknown system call %d\n", syscall_number);
  }
}
