/*
 *  ZeOS
 *  system.lds - Linker Script for ZeOS system image.
 */

ENTRY(main)
SECTIONS
{
  . = 0x10000;
    
  .text.main : { 
    /* reserved to store user code size */
    BYTE(24);
    *(.text.main) 
  }
                                     
  .text : { *(.text) }
  .rodata : { *(.rodata) }
  .data : { *(.data) }
  .bss : { *(.bss) }

  . = ALIGN(4096);              /* task_structs array*/
  .data.task : { *(.data.task) }

}
