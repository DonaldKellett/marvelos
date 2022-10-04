#ifndef TRAP_HANDLER_H
#define TRAP_HANDLER_H

#include <stddef.h>
#include "trap_frame.h"

#define CAUSE_IS_INTERRUPT(cause) (((size_t)(cause) >> 63) & 1)
#define CAUSE_EXCEPTION_CODE(cause) ((size_t)(cause) & 0x7FFFFFFFFFFFFFFFull)

size_t m_mode_trap_handler(size_t, size_t, size_t, size_t, size_t,
			   struct trap_frame *);

#endif
