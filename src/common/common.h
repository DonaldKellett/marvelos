#ifndef COMMON_H
#define COMMON_H

#define PLIC_ADDR 0xc000000
#define CLINT_ADDR 0x2000000

// Microseconds per second
#define US_PER_SECOND 1000000ull

#define PANIC(format, ...) ({\
  kprintf("Kernel panic at %s:%d:\n" format,\
    __FILE__,\
    __LINE__ \
    __VA_OPT__(,) __VA_ARGS__);\
  asm volatile ("wfi");\
})

#define ASSERT(condition, format, ...) ({\
  if (!(condition))\
    PANIC("Failed asserting that %s\n" format,\
      #condition \
      __VA_OPT__(,) __VA_ARGS__);\
})

#define SATP_FROM(mode, asid, ppn) (((size_t)(mode) << 60) | ((size_t)(asid) << 44) | ppn)

int toupper(int);
char *strcpy(char *, const char *);

#endif
