#include "uart/uart.h"
#include "syscon/syscon.h"
#include "common/common.h"
#include "mm/page.h"
#include "mm/sv39.h"
#include "mm/kmem.h"

// Define PLIC and CLINT here for now
// We'll move them into separate header files when we get there
#define PLIC_ADDR 0xc000000
#define CLINT_ADDR 0x2000000

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
extern size_t KERNEL_TABLE;

const char HELLO[] =
    "Hello World! This is a dynamically allocated string using the byte-grained allocator.\n";

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

uint64_t kinit(void) {
  // Initialize core subsystems
  uart_init(UART_ADDR);
  kputs("UART initialized");
  kputs("We are in M-mode!");
  page_init();
  kputs("Page-grained allocator initialized");
  kmem_init();
  kputs("Byte-grained allocator initialized");

  // Map heap allocations
  struct page_table *root = kmem_get_page_table();
  size_t kheap_head = (size_t)kmem_get_head();
  size_t total_pages = kmem_get_num_allocations();
  kprintf("\n\n");
  kprintf("INIT: [%p, %p)\n", INIT_START, INIT_END);
  kprintf("TEXT: [%p, %p)\n", TEXT_START, TEXT_END);
  kprintf("RODATA: [%p, %p)\n", RODATA_START, RODATA_END);
  kprintf("DATA: [%p, %p)\n", DATA_START, DATA_END);
  kprintf("BSS: [%p, %p)\n", BSS_START, BSS_END);
  kprintf("HEAP: [%p, %p)\n", kheap_head, kheap_head + total_pages * PAGE_SIZE);
  kprintf("STACK: [%p, %p)\n", KERNEL_STACK_START, KERNEL_STACK_END);
  id_map_range(root, kheap_head, kheap_head + total_pages * PAGE_SIZE, PTE_RW);

  // Map heap descriptors
  size_t num_pages = get_num_pages();
  id_map_range(root, HEAP_START, HEAP_START + num_pages, PTE_RW);

  // Map init section
  id_map_range(root, INIT_START, INIT_END, PTE_RX);

  // Map executable section
  id_map_range(root, TEXT_START, TEXT_END, PTE_RX);

  // Map rodata section
  id_map_range(root, RODATA_START, RODATA_END, PTE_READ);

  // Map data section
  id_map_range(root, DATA_START, DATA_END, PTE_RW);

  // Map bss section
  id_map_range(root, BSS_START, BSS_END, PTE_RW);

  // Map kernel stack
  id_map_range(root, KERNEL_STACK_START, KERNEL_STACK_END, PTE_RW);

  // Map UART
  map(root, UART_ADDR, UART_ADDR, PTE_RW, 0);

  // Map syscon registers
  map(root, SYSCON_ADDR, SYSCON_ADDR, PTE_RW, 0);

  // Map PLIC
  id_map_range(root, PLIC_ADDR, PLIC_ADDR + 0x2000, PTE_RW);
  id_map_range(root, PLIC_ADDR + 0x200000, PLIC_ADDR + 0x208000, PTE_RW);

  // Map CLINT
  map(root, CLINT_ADDR, CLINT_ADDR, PTE_RW, 0);

  print_page_allocations();

  // Let's see Sv39 address translation in action
  size_t vaddr = INIT_START;
  size_t paddr = virt_to_phys(root, vaddr);
  ASSERT(paddr != 0x0,
	 "kinit(): Failed to map INIT_START physical address to INIT_START physical address");
  kprintf("vaddr = %p maps to paddr = %p\n", vaddr, paddr);

  // Store the kernel table
  KERNEL_TABLE = (size_t)root;

  kputs("Exiting M-mode\n");
  return SATP_FROM(MODE_SV39, 0, ((size_t)root) >> PAGE_ORDER);
}

void kmain(void) {
  kputs("We are in S-mode!");
  kputchar('\n');

  char *hello = kcalloc(1 + sizeof(HELLO), sizeof(char));
  strcpy(hello, HELLO);
  kprintf("%s\n", hello);
  kfree(hello);

  int *one_to_five = kmalloc(5 * sizeof(int));
  for (int i = 0; i < 5; ++i)
    one_to_five[i] = i + 1;
  for (int i = 0; i < 5; ++i)
    kprintf("%d\n", one_to_five[i]);
  kfree(one_to_five);

  kprintf("Goodbye!\n");

  poweroff();
}
