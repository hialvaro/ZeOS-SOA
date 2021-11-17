/*
 * hardware.h - Rutines hardware per manegar els accesos a baix nivell
 */

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <types.h>

DWord get_eflags(void);
void set_eflags(void);
void set_idt_reg(Register * idt);
void set_gdt_reg(Register * gdt);
void set_ldt_reg(Selector ldt);
void set_task_reg(Selector tr);
void return_gate(Word ds, Word ss, DWord esp, Word cs, DWord eip);

/*
 * enable_int: Assigna la mascara d'interrupcions
 *
 *    registro 0x21:
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
 *   x = 0 -> activada
 *   x = 1 -> no activada
 */

void enable_int(void);
void delay(void);
#endif  /* __HARDWARE_H__ */
