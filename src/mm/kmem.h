#ifndef KMEM_H
#define KMEM_H

#include <stddef.h>

/*
 * Here comes our byte-grained memory allocator
 *
 * KMMD stands for "kernel memory metadata"
 * The idea is that, for every block of memory allocated with
 * kmalloc(), the first 64 bits of that memory KMMD hold metadata
 * indicating:
 *
 * - Whether the block is taken in KMMD[63]
 * - The size of the block (including metadata) in KMMD[62:0]
 */
#define KMMD_TAKEN (1ull << 63)
#define KMMD_IS_TAKEN(block) (!!(*(const size_t *)(block) & KMMD_TAKEN))
#define KMMD_IS_FREE(block) (!KMMD_IS_TAKEN(block))
#define KMMD_SET_TAKEN(block) ({\
  *(size_t *)(block) |= KMMD_TAKEN;\
})
#define KMMD_SET_FREE(block) ({\
  *(size_t *)(block) &= ~KMMD_TAKEN;\
})
#define KMMD_SET_SIZE(block, sz) ({\
  size_t _k = KMMD_IS_TAKEN(block);\
  *(size_t *)(block) = (size_t)(sz) & ~KMMD_TAKEN;\
  if (_k)\
    KMMD_SET_TAKEN(block);\
})
#define KMMD_GET_SIZE(block) (*(const size_t *)(block) & ~KMMD_TAKEN)

void *kmem_get_head(void);
struct page_table *kmem_get_page_table(void);
size_t kmem_get_num_allocations(void);

void kmem_init(void);
void *kcalloc(size_t, size_t);
void *kmalloc(size_t);
void kfree(void *);

void kmem_print_table(void);

#endif
