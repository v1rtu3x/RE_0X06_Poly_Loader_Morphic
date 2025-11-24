SECTIONS
{
  . = 0x400000;

  .text :
  {
    *(.text*)
  }

  .rodata :
  {
    *(.rodata*)
  }

  .data :
  {
    *(.data*)
  }

  .bss :
  {
    *(.bss*)
  }

  .packmeta :
  {
    *(.packmeta)
  }

  .packed :
  {
    __packed_start = .;
    *(.packed)
    __packed_end = .;
  }
}
