ENTRY(_start)
SECTIONS {
  . = 0;
  .text : { *(.text.start) *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) *(.sdata*) }
  _gp = ALIGN(16) + 0x7ff0;
  .got : { *(.got*) }
  __bss_start = .;
  .bss : { *(.sbss*) *(.bss*) *(COMMON) }
  __end = .;
  /DISCARD/ : { *(.reginfo) *(.MIPS.*) *(.pdr) *(.comment) *(.gnu.*) *(.note*) }
}
