# Disable generation of compressed instructions
# This is to avoid complications when setting values
# of CSRs such as mtvec and stvec which have alignment
# constraints
.option norvc

# Common symbols
.set NUM_GP_REGS, 32
.set REG_SIZE, 8

# Use alternative macro syntax (see GNU assembler docs for details)
.altmacro

# Common macros
.macro save_gp i, basereg=t6
  sd x\i, ((\i) * REG_SIZE)(\basereg)
.endm
.macro load_gp i, basereg=t6
  ld x\i, ((\i) * REG_SIZE)(\basereg)
.endm

# Importation of linker symbols
.section .rodata
.global HEAP_START
HEAP_START: .dword __heap_start

.global HEAP_SIZE
HEAP_SIZE: .dword __heap_size

.global INIT_START
INIT_START: .dword __init_start

.global INIT_END
INIT_END: .dword __init_end

.global TEXT_START
TEXT_START: .dword __text_start

.global TEXT_END
TEXT_END: .dword __text_end

.global RODATA_START
RODATA_START: .dword __rodata_start

.global RODATA_END
RODATA_END: .dword __rodata_end

.global DATA_START
DATA_START: .dword __data_start

.global DATA_END
DATA_END: .dword __data_end

.global BSS_START
BSS_START: .dword __bss_start

.global BSS_END
BSS_END: .dword __bss_end

.global KERNEL_STACK_START
KERNEL_STACK_START: .dword __kernel_stack_start

.global KERNEL_STACK_END
KERNEL_STACK_END: .dword __kernel_stack_end

.global MAKE_SYSCALL
MAKE_SYSCALL: .dword make_syscall

.section .data
.global KERNEL_TABLE
KERNEL_TABLE: .dword 0

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
  # Set this to kmain so executing mret jumps to kmain
  la t0, kmain
  csrw mepc, t0

  # Do not allow interrupts in M-mode
  csrw mie, zero

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
  la sp, __kernel_stack_end
  mv fp, sp

  # If kmain returns, we're done with everything so halt forever
  la ra, halt_forever

  # Now jump to kmain for M-mode initialization
  mret

# We're already done with everything - let's halt forever
halt_forever:
  csrw mie, zero
  wfi
  j halt_forever

# Interrupt handler
interrupt_handler:
  # Save all general purpose registers into kernel trap frame
  # No need to save floating point registers since we haven't used them yet
  # No need to save satp, trap_stack since we don't modify them
  # No need to save hartid since that is always 0
  # (we only have a single CPU core)
  # This requires a bit of trickery to do correctly:
  # 
  # 0. mscratch has address of kernel trap frame - see kinit() for details
  # 1. Atomically swap mscratch and t6 registers
  #    Now t6 has address of kernel trap frame, and mscratch the
  #    original value of t6
  # 2. Now save registers x1-x30 into kernel trap frame, using
  #    t6=x31 as base
  #    No need to save zero=x0 since that is read-only zero
  # 3. Move address of kernel trap frame to t5=x30 so we don't lose it
  # 4. Move mscratch (= original value of t6) back into t6 and save that
  # 5. Write address of kernel trap frame from t5 back into mscratch
  csrrw t6, mscratch, t6
  .set i, 1
  .rept 30
    save_gp %i
    .set i, i + 1
  .endr
  mv t5, t6
  csrr t6, mscratch
  save_gp 31, t5
  csrw mscratch, t5

  # Now invoke our M-mode trap handler
  csrr a0, mepc
  csrr a1, mtval
  csrr a2, mcause
  csrr a3, mhartid
  csrr a4, mstatus
  mv a5, t5 # t5 still contains copy of mscratch
  # Point to the kernel trap stack instead of the stack of
  # whatever brought us here, so that we don't clobber the
  # original stack
  # In src/plic/trap_frame.h, we have:
  # 
  # - trap_frame.regs = trap_frame[255:0]
  # - trap_frame.fregs = trap_frame[511:256]
  # - trap_frame.satp = trap_frame[519:512]
  # - trap_frame.trap_stack = trap_frame[527:520]
  # - trap_frame.hartid = trap_frame[535:528]
  # 
  # for a total of 536 bytes
  # So the field trap_frame.trap_stack containing the address
  # of the top of our kernel trap stack is located 520 bytes
  # away from the start of trap_frame
  ld sp, 520(a5)
  call m_mode_trap_handler

  # m_mode_trap_handler returns the PC value via a0
  csrw mepc, a0

  # Restore registers and return
  # This is more straightforward, since we can overwrite t6=x31 at the end
  csrr t6, mscratch
  .set i, 1
  .rept 31
    load_gp %i
    .set i, i + 1
  .endr

  # Continue execution at the given PC value
  mret

.global make_syscall
make_syscall:
  ecall
  ret

.global switch_to_user
switch_to_user:
  # a0 - frame address
  csrw mscratch, a0
  
  # Set MPP=0 (U-mode), MPIE=1, SPIE=1
  li t0, (0b00 << 11) | (1 << 7) | (1 << 5)
  csrw mstatus, t0

  # a1 - program counter
  csrw mepc, a1

  # a2 - SATP register
  csrw satp, a2
  
  # Enable external, timer and software interrupts from
  # M-mode and S-mode alike
  li t0, 0xAAA
  csrw mie, t0

  # Set interrupt handler
  la t0, interrupt_handler
  csrw mtvec, t0

  # Sync SATP for all address spaces, for all harts
  sfence.vma

  # Load process context frame
  mv t6, a0
  .set i, 1
  .rept 31
    load_gp %i, t6
    .set i, i + 1
  .endr

  # Now jump to U-mode
  mret
