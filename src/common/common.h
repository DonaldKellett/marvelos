#ifndef COMMON_H
#define COMMON_H

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

int toupper(int);
char *strcpy(char *, const char *);

#endif
