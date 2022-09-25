# Disable generation of compressed instructions
# This is to avoid complications when setting values
# of CSRs such as mtvec and stvec which have alignment
# constraints
.option norvc

# Importation of linker symbols
.section .rodata
.global HEAP_START
HEAP_START: .dword __heap_start

.global HEAP_END
HEAP_END: .dword __heap_end

.section .init, "ax"
.global _start
_start:
  # Initialize CSRs for M-mode

  # Supervisor address translation and protection
  # SATP should already be zero, but just to make sure ...
  csrw satp, zero

  # Machine status
  # MPP = mstatus[12:11]
  #      MPP=3 (M-level access with no translation)
  li t0, 0b11 << 11
  csrw mstatus, t0

  # Machine exception program counter
  # Set this to kinit so executing mret jumps to kinit
  la t0, kinit
  csrw mepc, t0

  # Machine trap vector
  # Use our interrupt_handler for handling M-mode traps
  la t0, interrupt_handler
  csrw mtvec, t0

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

  # prepare_s_mode is where we'll continue after kinit
  la ra, prepare_s_mode

  # Now jump to kinit for M-mode initialization
  mret

prepare_s_mode:
  # Initialize CSRs for S-mode

  # Supervisor status
  # SPP: supervisor's previous protection mode
  # SPIE: supervisor's previous interrupt enable bit
  # Supervisor's interrupt-enable bit sstatus[1] will
  # be set to 1 after sret
  #      SPP=1      SPIE=1
  li t0, (1 << 8) | (1 << 5)
  csrw sstatus, t0

  # Supervisor exception program counter
  # Set to kmain - this is where executing sret will jump to
  la t0, kmain
  csrw sepc, t0

  # Machine interrupt delegate
  # Specifies which types of interrupts to delegate to S-mode
  #      external   timer      software
  li t0, (1 << 9) | (1 << 5) | (1 << 1)
  csrw mideleg, t0
  
  # Supervisor interrupt enable
  # Enable the same types of interrupts for S-mode
  csrw sie, t0

  # Supervisor trap vector
  # Like mtvec, set to our interrupt_handler
  la t0, interrupt_handler
  csrw stvec, t0

  # Use kinit return value as value of SATP register
  csrw satp, a0

  # Ensure our hart sees updated value of SATP after this point
  sfence.vma

  # Define PMP region to allow access to all physical memory in S-mode
  # By default, M-mode can access all physical memory and
  # no other modes can access any physical memory
  # pmp0cfg = pmpcfg0[7:0]
  #      A=TOR         X=1        W=1        R=1
  li t0, (0b01 << 3) | (1 << 2) | (1 << 1) | (1 << 0)
  csrw pmpcfg0, t0
  # Set all 1's for the top address (exclusive)
  # The bottom address (inclusive) is implicitly 0 when setting
  # pmpcfg0 and pmpaddr0
  li t0, -1
  csrw pmpaddr0, t0

  # If we ever return from kmain, just wait for interrupts indefinitely
  la ra, wait_for_interrupt

  # Now go to S-mode
  sret

# We're already done with everything - let's hang
wait_for_interrupt:
  wfi
  j wait_for_interrupt

# Interrupt handler (WIP)
interrupt_handler:
  csrr a0, mtval
  wfi
  j interrupt_handler
