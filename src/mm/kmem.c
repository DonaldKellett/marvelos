#include <stdbool.h>
#include "kmem.h"
#include "page.h"
#include "../common/common.h"
#include "../uart/uart.h"

// Head of allocation. Start here when searching for free memory location
static size_t *KMEM_HEAD = NULL;
// Keep track of memory footprint
static size_t KMEM_ALLOC = 0;
static struct page_table *KMEM_PAGE_TABLE = NULL;

void *kmem_get_head(void) {
  return (void *)KMEM_HEAD;
}
struct page_table *kmem_get_page_table(void) {
  return KMEM_PAGE_TABLE;
}
size_t kmem_get_num_allocations(void) {
  return KMEM_ALLOC;
}

// Initialize kernel memory
// This memory must not be allocated to userspace processes!
void kmem_init(void) {
  void *k_alloc = alloc_pages(64);
  ASSERT(k_alloc != NULL,
	 "kmem_init(): got NULL pointer when requesting pages for kernel");
  KMEM_ALLOC = 64;
  KMEM_HEAD = (size_t *)k_alloc;
  KMMD_SET_FREE(KMEM_HEAD);
  KMMD_SET_SIZE(KMEM_HEAD, KMEM_ALLOC * PAGE_SIZE);
  KMEM_PAGE_TABLE = (struct page_table *)alloc_page();
  ASSERT(KMEM_PAGE_TABLE != NULL,
	 "kmem_init(): got NULL pointer when requesting single page for kernel page table");
}

void *kcalloc(size_t num, size_t size) {
  size_t total_size = align_val(num * size, 3);
  uint8_t *result = (uint8_t *) kmalloc(total_size);
  if (result != NULL)
    for (size_t i = 0; i < total_size; ++i)
      result[i] = 0;
  return (void *)result;
}

void *kmalloc(size_t sz) {
  size_t size = align_val(sz, 3) + sizeof(size_t);
  size_t *head = KMEM_HEAD;
  size_t *tail = (size_t *)&((uint8_t *) KMEM_HEAD)[KMEM_ALLOC * PAGE_SIZE];
  while ((size_t)head < (size_t)tail)
    if (KMMD_IS_FREE(head) && size <= KMMD_GET_SIZE(head)) {
      size_t chunk_size = KMMD_GET_SIZE(head);
      size_t remaining = chunk_size - size;
      KMMD_SET_TAKEN(head);
      if (remaining > sizeof(size_t)) {
	size_t *next = (size_t *)&((uint8_t *) head)[size];
	KMMD_SET_FREE(next);
	KMMD_SET_SIZE(next, remaining);
	KMMD_SET_SIZE(head, size);
      } else
	KMMD_SET_SIZE(head, chunk_size);
      return (void *)&head[1];
    } else
      head = (size_t *)&((uint8_t *) head)[KMMD_GET_SIZE(head)];
  return NULL;
}

// Merge adjacent free blocks into one big free block
static void coalesce(void) {
  size_t *head = KMEM_HEAD;
  size_t *tail = (size_t *)&((uint8_t *) KMEM_HEAD)[KMEM_ALLOC * PAGE_SIZE];
  while ((size_t)head < (size_t)tail) {
    bool merged = false;
    // Get next block
    size_t *next = (size_t *)&((uint8_t *) head)[KMMD_GET_SIZE(head)];
    ASSERT(head != next,
	   "coalesce(): found block of 0 bytes - possible double free error");
    if (next >= tail)
      // `head` was the last block - nothing to coalesce
      break;
    else if (KMMD_IS_FREE(head) && KMMD_IS_FREE(next)) {
      // Both adjacent blocks are free - merge into `head`
      KMMD_SET_SIZE(head, KMMD_GET_SIZE(head) + KMMD_GET_SIZE(next));
      merged = true;
    }
    if (!merged)
      head = (size_t *)&((uint8_t *) head)[KMMD_GET_SIZE(head)];
  }
}

void kfree(void *ptr) {
  if (ptr != NULL) {
    size_t *p = &((size_t *)ptr)[-1];
    if (KMMD_IS_TAKEN(p))
      KMMD_SET_FREE(p);
    coalesce();
  }
}

// Print the kmem table for debugging
void kmem_print_table(void) {
  kputchar('\n');
  kprintf("KMEM ALLOCATION TABLE\n");
  size_t *head = KMEM_HEAD;
  size_t *tail = (size_t *)&((uint8_t *) KMEM_HEAD)[KMEM_ALLOC * PAGE_SIZE];
  while ((size_t)head < (size_t)tail) {
    kprintf("%p: size = %d, taken = %d\n", head, KMMD_GET_SIZE(head),
	    KMMD_IS_TAKEN(head));
    head = (size_t *)&((uint8_t *) head)[KMMD_GET_SIZE(head)];
  }
  kputchar('\n');
}
