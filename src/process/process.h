#ifndef PROCESS_H
#define PROCESS_H

#include <stddef.h>

// Defined in src/asm/crt0.s
void switch_to_user(size_t, size_t, size_t);

#endif
