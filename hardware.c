/*
 * hardware.c 
 */


#include <types.h>

extern unsigned int *p_rdtr ;
DWord get_eflags(void)
{
  register DWord flags;
__asm__ __volatile__(
  "pushfl\n\t"
  "popl %0"
  : "=q" (flags) );

  return flags;
}

void set_eflags(void)
{
__asm__ __volatile__(
  "pushl $0\n\t"
  "popfl" );
}

void set_idt_reg(Register * idt)
{
__asm__ __volatile__(
  "lidtl (%0)"
  : /*no output*/
  : "r" (idt) );
}

void set_gdt_reg(Register * gdt)
{
__asm__ __volatile__(
  "lgdtl (%0)"
  : /*no output*/
  : "r" (gdt) );
}

void set_ldt_reg(Selector ldt)
{
__asm__ __volatile__(
  "lldtw %0"
  : /*no output*/
  : "r" (ldt) );
}

void set_task_reg(Selector tr)
{
__asm__ __volatile__(
  "ltrw %0"
  : /*no output*/
  : "r" (tr) );
}


void return_gate(Word ds, Word ss, DWord esp, Word cs, DWord eip)
{
  __asm__ __volatile__ (
    "mov %0,%%es\n\t"
    "mov %0,%%ds\n\t"
    "movl %2, %%eax\n\t"
    "addl $12, %%eax\n\t"
    "movl %5,(%%eax)\n\t"
    "pushl %1\n\t"       /* user ss */
    "pushl %2\n\t"       /* user esp */
    "pushl %3\n\t"       /* user cs */
    "pushl %4\n\t"       /* user eip */
    "lret"
    : /*no output*/
    : "m" (ds), "m" (ss), "m" (esp), "m" (cs), "m" (eip), "d" (*p_rdtr));
}

/*
 * enable_int: Set interruput mask
 *
 *    register 0x21:
 *    7 6 5 4 3 2 1 0
 *    x x x x x x x x
 *    
 *    bit 0 : Timer
 *    bit 1 : Keyboard
 *    bit 2 : PIC cascading
 *    bit 3 : 2nd Serial Port
 *    bit 4 : 1st Serial Port
 *    bit 5 : Reserved
 *    bit 6 : Floppy disk
 *    bit 7 : Reserved
 * 
 *
 *   x = 0 -> enabled
 *   x = 1 -> disabled
 */

void enable_int(void)
{
__asm__ __volatile__(
  "movb %0,%%al\n\t"
  "outb %%al,$0x21\n\t"
  "call delay\n\t"
  "sti"
  : /*no output*/
  : "i" (0xfc)       /* 0xFF = 11111111 -> all bits disabled */
  : "%al" );
}

void delay(void)
{
__asm__ __volatile__(
  "jmp a\na:\t"
  : : );
}

