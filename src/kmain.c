#include "uart/uart.h"
#include "syscon/syscon.h"
#include "common/common.h"
#include "mm/page.h"
#include "mm/sv39.h"
#include "mm/kmem.h"
#include "plic/trap_frame.h"
#include "plic/cpu.h"
#include "plic/plic.h"
#include "process/process.h"

extern const size_t INIT_START;
extern const size_t INIT_END;
extern const size_t TEXT_START;
extern const size_t TEXT_END;
extern const size_t RODATA_START;
extern const size_t RODATA_END;
extern const size_t DATA_START;
extern const size_t DATA_END;
extern const size_t BSS_START;
extern const size_t BSS_END;
extern const size_t KERNEL_STACK_START;
extern const size_t KERNEL_STACK_END;
extern const size_t HEAP_START;
extern const size_t HEAP_SIZE;
extern const size_t MAKE_SYSCALL;
extern size_t KERNEL_TABLE;

const char HELLO[] =
    "Hello World! This is a dynamically allocated string using the byte-grained allocator.";

// Identity map range
// Takes a contiguous allocation of memory and maps it using PAGE_SIZE
// `start` must not exceed `end`
void id_map_range(struct page_table *root, size_t start, size_t end,
		  uint64_t bits) {
  ASSERT(root != NULL, "id_map_range(): root page table cannot be NULL");
  ASSERT(start <= end, "id_map_range(): start must not exceed end");
  ASSERT(PTE_IS_LEAF(bits),
	 "id_map_range(): Provided bits must correspond to leaf entry");
  size_t memaddr = start & ~(PAGE_SIZE - 1);
  size_t num_kb_pages = (align_val(end, PAGE_ORDER) - memaddr) / PAGE_SIZE;
  for (size_t i = 0; i < num_kb_pages; ++i) {
    map(root, memaddr, memaddr, bits, 0);
    memaddr += PAGE_SIZE;
  }
}

void kmain(void) {
  uart_init();
  page_init();
  kmem_init();

  PLIC_SET_THRESHOLD(0);
  PLIC_ENABLE(PLIC_UART);
  PLIC_SET_PRIO(PLIC_UART, 1);

  kprintf("Starting our first process ...\n");
  struct process *pid1 = create_process(init_process);
  ASSERT(pid1 != NULL,
	 "kmain(): failed to allocate structure for init process\n");
  switch_to_user((size_t)pid1->frame, pid1->pc,
		 SATP_FROM(MODE_SV39, pid1->pid,
			   (size_t)pid1->root >> PAGE_ORDER));
  PANIC("kmain(): failed to start our first process!\n");
}
