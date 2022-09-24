ENTRY(_start)

. = 0x80000000;

SECTIONS {
  .text : ALIGN(4K) {
    *(.init);
    *(.text);
  }
  .data : ALIGN(4K) {
    *(.data);
  }
  .bss : ALIGN(4K) {
    *(.bss);
    PROVIDE(__global_pointer = .);
    PROVIDE(__heap_start = .);
  }
  . = 0x88000000;
  PROVIDE(__heap_end = .);
  PROVIDE(__stack_top = .);
}
