.section .init, "ax"
.global _start
_start:
  .cfi_startproc
  .cfi_undefined ra

  # Initialize satp, mepc CSRs
  csrw satp, zero
  la t0, kmain
  csrw mepc, t0

  # Zero the BSS section
  la t0, __bss_start
  la t1, __bss_end
__bss_zero_loop_start:
  bgeu t0, t1, __bss_zero_loop_end
  sd zero, 0(t0)
  addi t0, t0, 8
  j __bss_zero_loop_start
__bss_zero_loop_end:

  # Initialize global pointer register
  .option push
  .option norelax
  la gp, __global_pointer
  .option pop

  # Initialize stack and frame pointer registers
  la sp, __stack_top
  mv fp, sp

  # Jump to our kernel
  j kmain

  .cfi_endproc
  .end
