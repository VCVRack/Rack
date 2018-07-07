/*
 * STM32F103R8T6
 */

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08004000, LENGTH = 112K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

SECTIONS
{
  .text :
  {
    _start = .;
    KEEP(*(.isr_vector))
    *(.text*)
    *(.rodata*)
    *(.flashtext*)
    KEEP (*(.init))
    KEEP (*(.fini))
    _etext = .;
    _sidata = .;
  } > FLASH

  .data : AT (ADDR(.text) + SIZEOF(.text))
  {
    _sdata = .;
    *(.ramtext*)
    . = ALIGN(4);
    *(.data*)
    _edata = .;
  } > RAM

  .bss :
  {
    _sstack = .;
    . = . + 512;
    _estack = .;
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM
}
