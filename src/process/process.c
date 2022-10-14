#include "process.h"
#include "syscall.h"
#include "../mm/kmem.h"
#include "../common/common.h"
#include "../mm/page.h"
#include "../mm/sv39.h"

extern const size_t MAKE_SYSCALL;

static uint16_t NEXT_PID = 1;

// This is just a temporary measure
// Ideally, we want to move our hardcoded init process
// out of the kernel as soon as possible
void init_process(void) {
  while (1) {
    for (size_t i = 0; i < 70000000; ++i) ;
    make_syscall(1);
  }
}

struct process *create_process(void (*func)(void)) {
  size_t func_paddr = (size_t)func;	// determine process physical address
  size_t func_vaddr = func_paddr;	// set process virtual address

  // Initialize process structure
  struct process *process = kmalloc(sizeof(struct process));
  ASSERT(process != NULL,
	 "create_process(): failed to allocate memory for process structure\n");
  process->frame = (struct trap_frame *)alloc_page();
  ASSERT(process->frame != NULL,
	 "create_process(): failed to allocate page for process context frame\n");
  process->stack = alloc_pages(STACK_PAGES);
  ASSERT(process->stack != NULL,
	 "create_process(): failed to allocate %d pages for process stack\n",
	 STACK_PAGES);
  process->pc = func_vaddr;
  process->pid = NEXT_PID++;
  process->root = (struct page_table *)alloc_page();
  ASSERT(process->root != NULL,
	 "create_process(): failed to allocate page for process root page table\n");
  process->state = PROCESS_RUNNING;
  process->sleep_until = 0;

  size_t stack_paddr = (size_t)process->stack;	// obtain stack physical address
  // Set stack pointer to point to top of process stack
  process->frame->regs[2] = STACK_ADDR + PAGE_SIZE * STACK_PAGES;	// sp = x2

  // Map process stack to virtual memory
  for (size_t i = 0; i < STACK_PAGES; ++i)
    map(process->root, STACK_ADDR + i * PAGE_SIZE, stack_paddr + i * PAGE_SIZE,
	PTE_USER_RW, 0);

  // Map user program to virtual memory
  for (size_t i = 0; i < 100; ++i)
    map(process->root, func_vaddr + i * PAGE_SIZE, func_paddr + i * PAGE_SIZE,
	PTE_USER_RX, 0);

  // Map make_syscall() to virtual memory
  // This is required since otherwise user programs cannot make
  // system calls from user space
  map(process->root, MAKE_SYSCALL, MAKE_SYSCALL, PTE_USER_RX, 0);

  return process;
}
