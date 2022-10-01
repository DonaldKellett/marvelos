/* Entry symbol is _start */
ENTRY(_start)

/* RAM starts at 0x80000000 - see misc/riscv64-virt.dts for details */
. = 0x80000000;

SECTIONS {
  /**
   * ELF file layout: text section at bottom, followed by data and bss
   * sections
   *
   * Global pointer starts where BSS ends
   * The heap also starts right after the BSS section, growing upwards
   * The stack grows downwards, starting from the end of RAM
   * This mean there's a chance stack and heap collide, but we'll assume
   * it never happens
   */
  .init : ALIGN(4K) {
    PROVIDE(__init_start = .);
    *(.init);
    PROVIDE(__init_end = .);
  }
  .text : ALIGN(4K) {
    PROVIDE(__text_start = .);
    *(.text);
    PROVIDE(__text_end = .);
  }
  .rodata : ALIGN(4K) {
    PROVIDE(__rodata_start = .);
    *(.rodata);
    PROVIDE(__rodata_end = .);
  }
  .data : ALIGN(4K) {
    PROVIDE(__data_start = .);
    *(.data);
    PROVIDE(__data_end = .);
  }
  .bss : ALIGN(4K) {
    PROVIDE(__bss_start = .);
    *(.bss);
    PROVIDE(__bss_end = .);
  }
  PROVIDE(__global_pointer = .);
  PROVIDE(__heap_start = .);
  . = 0x88000000;
  PROVIDE(__kernel_stack_start = . - 0x80000);
  PROVIDE(__kernel_stack_end = .);
  PROVIDE(__heap_size = __kernel_stack_start - __heap_start);
}
