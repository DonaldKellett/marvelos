#include <stddef.h>
#include "sv39.h"
#include "../common/common.h"
#include "../uart/uart.h"
#include "../mm/page.h"

/*
 * Map a virtual address to a physical address
 *
 * Parameters:
 *
 * - root: the root page table
 * - vaddr: virtual address to map from
 * - paddr: physical address to map to
 * - bits: bits to set on the leaf page table entry (PTE). Valid bits
 *   include R, W, X, U, G. The "V" bit is automatically added
 * - level: the level at which our leaf PTE should reside
 */
void map(struct page_table *root, size_t vaddr, size_t paddr, uint64_t bits,
	 int level) {
  // Root page table cannot be NULL
  ASSERT(root != NULL, "map(): root should not be NULL");

  // Virtual and physical addresses should not be 0 since that corresponds to
  // the NULL pointer
  ASSERT(vaddr != 0, "map(): virtual address should not be 0");
  ASSERT(paddr != 0, "map(): physical address should not be 0");

  // Ensure `bits` correspond to a leaf entry
  ASSERT(PTE_IS_LEAF(bits),
	 "map(): bits = %p does not correspond to a leaf PTE", bits);

  // Ensure `level` makes sense for Sv39
  // In Sv39, there are only 3 levels: 2, 1 and 0
  ASSERT(0 <= level
	 && level < 3,
	 "map(): level = %d is outside range [0, 3) permitted by Sv39", level);

  // Extract virtual page number (VPN) fields from virtual address
  const size_t VPN[3] = {
    // VPN[0] = vaddr[20:12]
    (vaddr >> 12) & 0x1FF,

    // VPN[1] = vaddr[29:21]
    (vaddr >> 21) & 0x1FF,

    // VPN[2] = vaddr[38:30]
    (vaddr >> 30) & 0x1FF
  };

  // Extract physical page number (PPN) fields from physical address
  const size_t PPN[3] = {
    // PPN[0] = paddr[20:12]
    (paddr >> 12) & 0x1FF,

    // PPN[1] = paddr[29:21]
    (paddr >> 21) & 0x1FF,

    // PPN[2] = paddr[55:30]
    (paddr >> 30) & 0x3FFFFFF
  };

  uint64_t *pte = &root->entries[VPN[2]];

  for (int i = 1; i >= level; --i) {
    if (PTE_IS_INVALID(*pte)) {
      void *page = alloc_page();
      *pte = ((uint64_t) page >> 2) | PTE_VALID;
    }
    uint64_t *entry = (uint64_t *) ((*pte & ~0x3FFull) << 2);
    pte = &entry[VPN[i]];
  }

  *pte = (PPN[2] << 28) |	// PPN[2] = PTE[53:28]
      (PPN[1] << 19) |		// PPN[1] = PTE[27:19]
      (PPN[0] << 10) |		// PPN[0] = PTE[18:10]
      bits | PTE_VALID;
}

/*
 * Unmap and free all memory associated with root page table
 * The root itself should be freed manually
 */
void unmap(struct page_table *root) {
  ASSERT(root != NULL, "unmap(): root should not be NULL");

  // Start with level 2
  for (size_t lv2 = 0; lv2 < PT_NUM_ENTRIES; ++lv2) {
    uint64_t entry_lv2 = root->entries[lv2];
    if (PTE_IS_VALID(entry_lv2) && PTE_IS_BRANCH(entry_lv2)) {
      // This is a valid entry, so drill down and free
      struct page_table *table_lv1 =
	  (struct page_table *)((entry_lv2 & ~0x3FFull) << 2);
      // Now repeat the process with level 1
      for (size_t lv1 = 0; lv1 < PT_NUM_ENTRIES; ++lv1) {
	uint64_t entry_lv1 = table_lv1->entries[lv1];
	if (PTE_IS_VALID(entry_lv1) && PTE_IS_BRANCH(entry_lv1))
	  // We can't have branches in level 0, so free directly
	  dealloc_pages((void *)((entry_lv1 & ~0x3FFull) << 2));
      }
      dealloc_pages((void *)table_lv1);
    }
  }
}

/*
 * Software implementation of Sv39 address translation logic
 * This is included despite the translation already implemented in hardware
 * for debugging purposes
 * If a page fault would occur, return the special address 0x0
 */
size_t virt_to_phys(struct page_table const *root, size_t vaddr) {
  ASSERT(root != NULL, "virt_to_phys(): root should not be NULL");
  ASSERT(vaddr != 0, "virt_to_phys(): virtual address should not be 0");

  const size_t VPN[3] = {
    (vaddr >> 12) & 0x1FF,
    (vaddr >> 21) & 0x1FF,
    (vaddr >> 30) & 0x1FF
  };

  uint64_t pte = root->entries[VPN[2]];
  for (int i = 2; i >= 0; --i) {
    if (PTE_IS_INVALID(pte))
      // Invalid entry - page fault
      break;
    else if (PTE_IS_LEAF(pte)) {
      // Page offset mask
      // The page offset is:
      //
      // - Level 0: VADDR[11:0]
      // - Level 1: VADDR[20:0]
      // - Level 2: VADDR[29:0]
      size_t pgoff_mask = (1ull << (12 + i * 9)) - 1;
      size_t vaddr_pgoff = vaddr & pgoff_mask;
      size_t addr = (size_t)(pte << 2) & ~pgoff_mask;
      return addr | vaddr_pgoff;
    }
    const uint64_t *entry = (const uint64_t *)((pte & ~0x3FFull) << 2);
    pte = entry[VPN[i - 1]];
  }

  // No valid mapping at this point - return 0
  return 0x0;
}
