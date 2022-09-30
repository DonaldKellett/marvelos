#include <stdbool.h>
#include "page.h"
#include "../common/common.h"
#include "../uart/uart.h"

extern size_t   HEAP_START;
extern size_t   HEAP_END;

static size_t   HEAP_BOTTOM = 0;
static size_t   HEAP_SIZE = 0;
static size_t   NUM_PAGES = 0;
static size_t   ALLOC_START = 0;
static size_t   ALLOC_END = 0;

// Align pointer to nearest PAGE_SIZE, rounded up
static size_t align_to_page(size_t n) {
    return n / PAGE_SIZE * PAGE_SIZE + (n % PAGE_SIZE ? PAGE_SIZE : 0);
}

// Get page address from page id
static size_t page_address_from_id(size_t id) {
    return ALLOC_START + PAGE_SIZE * id;
}

// Initialize the heap for page allocation
void mm_init(void) {
    HEAP_BOTTOM = HEAP_START;
    HEAP_SIZE = HEAP_END - HEAP_START;
    NUM_PAGES = HEAP_SIZE / PAGE_SIZE;
    struct page    *ptr = (struct page *) HEAP_BOTTOM;
    // Explicitly mark all pages as free
    for (size_t i = 0; i < NUM_PAGES; ++i)
	ptr[i].flags = 0;
    ALLOC_START =
	align_to_page(HEAP_BOTTOM + NUM_PAGES * sizeof(struct page));
    ALLOC_END = page_address_from_id(NUM_PAGES);

    // Re-compute ALLOC_END and NUM_PAGES as the heap should not
    // extend beyond our memory region
    size_t          error = ALLOC_END - (HEAP_BOTTOM + HEAP_SIZE);
    NUM_PAGES -= error / PAGE_SIZE;
    ALLOC_END = HEAP_BOTTOM + HEAP_SIZE;
    ASSERT(page_address_from_id(NUM_PAGES) <= ALLOC_END,
	   "mm_init(): Heap extends beyond our available memory region!");
}

// Attempts to allocate the specified number of contiguous free pages
// and returns a pointer to the beginning of the first page if successful
// All allocated pages are automatically zeroed if successful
// Otherwise, return NULL
void           *alloc_pages(size_t n) {
    ASSERT(n != 0, "alloc_pages(): attempted to allocate 0 pages");
    struct page    *ptr = (struct page *) HEAP_BOTTOM;
    for (size_t i = 0; i + n < NUM_PAGES + 1; ++i) {
	// Check that the next `n` pages are all free
	bool            found = true;
	for (size_t j = 0; j < n; ++j)
	    if (ptr[i + j].flags & PAGE_TAKEN) {
		found = false;
		break;
	    }
	if (!found)
	    continue;

	// Mark the next `n` pages as all taken and indicate
	// the last page
	for (size_t j = 0; j < n; ++j)
	    ptr[i + j].flags = PAGE_TAKEN;
	ptr[i + n - 1].flags |= PAGE_LAST;

	// Zero memory for all `n` pages and return a pointer to
	// the beginning of the 1st page
	// Do it in chunks of size_t bytes for efficiency
	size_t         *result = (size_t *) page_address_from_id(i);
	size_t          size = (PAGE_SIZE * n) / sizeof(size_t);
	for (size_t j = 0; j < size; ++j)
	    result[j] = 0;
	return (void *) result;
    }

    // Failed to find `n` contiguous free pages
    return NULL;
}

// Attempts to allocate a single zeroed free page; NULL otherwise
void           *alloc_page(void) {
    return alloc_pages(1);
}

// Deallocate a set of contiguous pages from a pointer returned
// from alloc_pages()
void dealloc_pages(void *ptr) {
    ASSERT(ptr != NULL, "dealloc_pages(): attempted to free NULL pointer");

    // Fetch corresponding page struct for given page address
    size_t          addr =
	HEAP_BOTTOM + ((size_t) ptr - ALLOC_START) / PAGE_SIZE;
    ASSERT(HEAP_BOTTOM <= addr
	   && addr < HEAP_BOTTOM + HEAP_SIZE,
	   "dealloc_pages(): Variable addr = %p outside heap range [%p, %p)",
	   addr, HEAP_BOTTOM, HEAP_BOTTOM + HEAP_SIZE);

    // Keep clearing pages until we hit the last page
    struct page    *p = (struct page *) addr;
    while ((p->flags & PAGE_TAKEN) && !(p->flags & PAGE_LAST)) {
	p->flags = 0;
	++p;
    }
    ASSERT(p->flags & PAGE_LAST,
	   "dealloc_pages(): Found a free page before reaching the "
	   "last page; possible double-free error occurred");

    // Clear the flags on the last page
    p->flags = 0;
}

void print_page_allocations(void) {
    struct page    *ptr = (struct page *) HEAP_BOTTOM;
    size_t          total = 0;
    size_t          TOTAL_BYTES = NUM_PAGES * PAGE_SIZE;
    kputchar('\n');
    kprintf("PAGE ALLOCATION TABLE\n");
    kprintf("TOTAL USABLE MEMORY: %d pages (%d bytes)\n", NUM_PAGES,
	    TOTAL_BYTES);
    kprintf("METADATA: [%p, %p)\n", ptr, &ptr[NUM_PAGES]);
    kprintf("PAGES: [%p, %p)\n", ALLOC_START, ALLOC_END);
    kprintf("========================================\n");
    for (size_t i = 0; i < NUM_PAGES; ++i) {
	if (ptr[i].flags & PAGE_TAKEN) {
	    size_t          start_addr = page_address_from_id(i);
	    if (ptr[i].flags & PAGE_LAST) {
		kprintf("[%p, %p): 1 page\n", start_addr,
			start_addr + PAGE_SIZE);
		++total;
		continue;
	    }
	    ++i;
	    while (i < NUM_PAGES && (ptr[i].flags & PAGE_TAKEN)
		   && !(ptr[i].flags & PAGE_LAST))
		++i;
	    ASSERT(i < NUM_PAGES,
		   "print_page_allocations(): reached end of metadata before finding the last page");
	    ASSERT(ptr[i].flags & PAGE_TAKEN,
		   "print_page_allocations(): found free page before reaching the "
		   "last page - possible double-free error");
	    size_t          end_addr = page_address_from_id(i + 1);
	    size_t          pages = (end_addr - start_addr) / PAGE_SIZE;
	    kprintf("[%p, %p): %d pages\n", start_addr, end_addr, pages);
	    total += pages;
	}
    }
    kprintf("========================================\n");
    size_t          ALLOC_BYTES = total * PAGE_SIZE;
    kprintf("TOTAL ALLOCATED: %d pages (%d bytes)\n", total, ALLOC_BYTES);
    kprintf("TOTAL FREE: %d pages (%d bytes)\n", NUM_PAGES - total,
	    TOTAL_BYTES - ALLOC_BYTES);
    kputchar('\n');
}
