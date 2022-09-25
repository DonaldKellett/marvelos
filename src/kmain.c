#include "uart/uart.h"
#include "syscon/syscon.h"
#include "common/common.h"
#include "mm/page.h"

uint64_t kinit(void) {
  uart_init(UART_ADDR);
  kputs("UART initialized");
  kputs("We are in M-mode!");

  mm_init();
  kputs("Page-grained allocator initialized");

  kputs("Exiting M-mode\n");
  return 0;
}

void kmain(void) {
  kputs("We are in S-mode!");

  print_page_allocations();

  void *p1 = alloc_pages(7);
  kputs("Allocated 7 pages to p1");
  void *p2 = alloc_page();
  kputs("Allocated 1 page to p2");
  void *p3 = alloc_pages(2);
  kputs("Allocated 2 pages to p3");
  void *p4 = alloc_pages(9);
  kputs("Allocated 9 pages to p4");
  void *p5 = alloc_page();
  kputs("Allocated 1 page to p5");

  print_page_allocations();

  dealloc_pages(p3);
  kputs("Deallocated p3");

  print_page_allocations();

  p3 = alloc_page();
  kputs("Allocated 1 page to p3");
  void *p6 = alloc_page();
  kputs("Allocated 1 page to p6");

  print_page_allocations();

  dealloc_pages(p4);
  kputs("Deallocated p4");

  print_page_allocations();

  dealloc_pages(p5);
  kputs("Deallocated p5");

  print_page_allocations();

  dealloc_pages(p1);
  kputs("Deallocated p1");

  print_page_allocations();

  dealloc_pages(p2);
  kputs("Deallocated p2");
  dealloc_pages(p3);
  kputs("Deallocated p3");
  dealloc_pages(p6);
  kputs("Deallocated p6");

  print_page_allocations();

  poweroff();
}
