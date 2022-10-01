#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_TAKEN (1 << 0)
#define PAGE_LAST (1 << 1)
#define PAGE_ORDER 12
#define PAGE_SIZE (1 << PAGE_ORDER)

struct page {
  uint8_t flags;
};

size_t get_num_pages(void);

size_t align_val(size_t, size_t);

void page_init(void);
void *alloc_pages(size_t);
void *alloc_page(void);
void dealloc_pages(void *);
void print_page_allocations(void);

#endif
