#ifndef TRAP_FRAME_H
#define TRAP_FRAME_H

#define NUM_REGS 32
#define NUM_FREGS 32

struct trap_frame {
  // General purpose registers
  size_t regs[NUM_REGS];

  // Floating point registers
  size_t fregs[NUM_FREGS];

  size_t satp;
  void *trap_stack;
  size_t hartid;
};

#define ZERO_TRAP_FRAME ((struct trap_frame){\
  .regs = {},\
  .fregs = {},\
  .satp = 0,\
  .trap_stack = NULL,\
  .hartid = 0\
})

struct trap_frame *get_kernel_trap_frame(void);

#endif
