#include <stddef.h>
#include "sched.h"
#include "../common/common.h"
#include "process.h"
#include "../mm/kmem.h"

static struct process_ll *PROCESSES = NULL;

void sched_init(void) {
  ASSERT(PROCESSES == NULL,
	 "sched_init(): should only be called once at system startup\n");
  sched_enqueue(init_process);
}

void sched_enqueue(void (*func)(void)) {
  struct process_ll *nd = kmalloc(sizeof(struct process_ll));
  ASSERT(nd != NULL,
	 "sched_enqueue(): failed to allocate linked list node for new process\n");
  nd->process = create_process(func);
  if (PROCESSES == NULL) {
    nd->prev = nd;
    nd->next = nd;
    PROCESSES = nd;
  } else {
    nd->prev = PROCESSES->prev;
    nd->next = PROCESSES;
    PROCESSES->prev->next = nd;
    PROCESSES->prev = nd;
  }
}

struct process *sched_schedule(void) {
  ASSERT(PROCESSES != NULL,
	 "sched_schedule(): cannot schedule a process from an empty process list - did you call sched_init()?\n");
  struct process *process = PROCESSES->process;
  ASSERT(process != NULL,
	 "sched_schedule(): process structure from head of process list was unexpectedly NULL\n");
  PROCESSES = PROCESSES->next;
  return process;
}
