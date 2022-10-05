#ifndef SV39_H
#define SV39_H

#include <stdint.h>

// MODE=8 encodes Sv39 paging in SATP register
#define MODE_SV39 8

// Page table entry (PTE) bits
#define PTE_NONE 0
#define PTE_VALID (1 << 0)
#define PTE_READ (1 << 1)
#define PTE_WRITE (1 << 2)
#define PTE_EXECUTE (1 << 3)
#define PTE_USER (1 << 4)
#define PTE_GLOBAL (1 << 5)
#define PTE_ACCESS (1 << 6)
#define PTE_DIRTY (1 << 7)

// Common PTE bit combinations
#define PTE_RW (PTE_READ | PTE_WRITE)
#define PTE_RX (PTE_READ | PTE_EXECUTE)
#define PTE_RWX (PTE_READ | PTE_WRITE | PTE_EXECUTE)

// Common PTE bit combinations (user mode)
#define PTE_USER_RW (PTE_USER | PTE_RW)
#define PTE_USER_RX (PTE_USER | PTE_RX)
#define PTE_USER_RWX (PTE_USER | PTE_RWX)

// Common PTE checks
#define PTE_IS_VALID(entry) ((entry) & PTE_VALID)
#define PTE_IS_INVALID(entry) (!PTE_IS_VALID(entry))
#define PTE_IS_LEAF(entry) ((entry) & 0xE)
#define PTE_IS_BRANCH(entry) (!PTE_IS_LEAF(entry))

// Construct SATP from MODE, ASID and PPN fields
#define SATP_FROM(mode, asid, ppn) (((size_t)(mode) << 60) | ((size_t)(asid) << 44) | ppn)

// A page table is exactly 4096 / 8 = 512 64-bit entries
#define PT_NUM_ENTRIES 512
struct page_table {
  uint64_t entries[PT_NUM_ENTRIES];
};

void map(struct page_table *, size_t, size_t, uint64_t, int);
void unmap(struct page_table *);
size_t virt_to_phys(struct page_table const *, size_t);

#endif
