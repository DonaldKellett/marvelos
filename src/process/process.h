#ifndef PROCESS_H
#define PROCESS_H

#include <stddef.h>
#include <stdint.h>
#include "../plic/trap_frame.h"

// Defined in src/asm/crt0.s
void switch_to_user(size_t, size_t, size_t);

// Number of pages per process stack
#define STACK_PAGES 2

// Start of virtual stack address (bottom)
#define STACK_ADDR 0x100000000ull

// Start of process virtual address space
#define PROCESS_STARTING_ADDR 0x80000000ull

// Init process - hardcoded for now, for testing purposes only
void init_process(void);

// Process states:
// - PROCESS_RUNNING: the process is ready to run whenever
//   the scheduler picks it
// - PROCESS_SLEEPING: the process is waiting for a certain
//   amount of time
// - PROCESS_WAITING: the process is waiting on I/O
// - PROCESS_DEAD: the process has finished and waiting to
//   be cleaned up
#define PROCESS_RUNNING (1 << 0)
#define PROCESS_SLEEPING (1 << 1)
#define PROCESS_WAITING (1 << 2)
#define PROCESS_DEAD (1 << 3)

// Process structure
// We need to know the exact sizes and positions
// of each field since we might need to access them
// in assembly
struct process {
  struct trap_frame *frame;	// process[535:0]
  void *stack;			// process[543:536]
  size_t pc;			// process[551:544]
  uint16_t pid;			// process[553:552]
  struct page_table *root;	// process[567:560]
  size_t state;			// process[575:568]
  size_t sleep_until;		// process[583:576]
};

// Create a new process from function pointer
struct process *create_process(void (*)(void));

#endif
