SECTIONS
{
  . = 0x400000;

  .text :
  {
    *(.text .text.*)
  }

  .rodata :
  {
    *(.rodata .rodata.*)
  }

  .data :
  {
    *(.data .data.*)
  }

  .bss :
  {
    *(.bss .bss.* COMMON)
  }

  .packed :
  {
    __packed_start = .;
    *(.packed)
    __packed_end = .;
  }

  /DISCARD/ :
  {
    *(.comment)
  }
}
