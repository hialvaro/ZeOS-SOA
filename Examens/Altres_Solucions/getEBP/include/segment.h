/*
 * segment.h - Constants de segment per a les entrades de la GDT
 */

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

/* Segment Selectors */
/*********************/

#define __KERNEL_CS     0x10  /* 2 */
#define __KERNEL_DS     0x18  /* 3 */

#define __USER_CS       0x23  /* 4 */
#define __USER_DS       0x2B  /* 5 */

#define KERNEL_TSS   0x30  /* Entry  6 on GDT (TI = 0) with RPL = 00 */

#define GDT_START  0x901b3  /* bootsect + gdt */

#endif  /* __SEGMENT_H__ */
