#ifndef CPU_H
#define CPU_H

#include <stddef.h>

#define CLINT_ADDR 0x2000000

// Microseconds per second
#define US_PER_SECOND 1000000ull

// The QEMU clock frequency is 0x989680 (= 10 MHz) according to our
// device tree
#define TICKS_PER_SECOND 0x989680ull

// See section 3.2.1 (machine timer registers) of RISC-V privileged spec
// for details on mtime, mtimecmp registers:
// https://github.com/riscv/riscv-isa-manual/releases/download/Priv-v1.12/riscv-privileged-20211203.pdf
//
// From our device tree, our QEMU virt RISC-V board has an
// SiFive-compatible CLINT
// Based on section 6.1 (CLINT Memory Map) of the SiFive E31 manual,
// the MMIO addresses of the two registers are as follows:
//
// - mtimecmp: 0x02004000
// - mtime: 0x0200bff8
//
// https://sifive.cdn.prismic.io/sifive%2Fc89f6e5a-cf9e-44c3-a3db-04420702dcc1_sifive+e31+manual+v19.08.pdf
#define MTIMECMP_ADDR 0x02004000ull
#define MTIME_ADDR 0x0200BFF8ull

#define GET_MSCRATCH() ({\
  size_t _mscratch;\
  asm volatile ("csrr %0, mscratch" : "=r"(_mscratch));\
  _mscratch;\
})

#define SET_MSCRATCH(mscratch) ({\
  asm volatile ("csrw mscratch, %0" :: "r"((size_t)(mscratch)));\
})

#define SET_SSCRATCH(sscratch) ({\
  asm volatile ("csrw sscratch, %0" :: "r"((size_t)(sscratch)));\
})

#define SET_MIE(mie) ({\
  asm volatile ("csrw mie, %0" :: "r"((size_t)(mie)));\
})

void set_timer_interrupt_delay_us(size_t);

#endif
