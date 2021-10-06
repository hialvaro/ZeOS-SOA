/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include <errno.h>

/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %ebx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %edx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %edx; \
      pushl %ecx; \
      pushl %ebx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

/*** Reverse order of SAVE_ALL ***/
#define RESTORE_ALL \
      popl %ebx; \
      popl %ecx; \
      popl %edx; \
      popl %esi; \
      popl %edi; \
      popl %ebp; \
      popl %eax; \
      popl %ds; \
      popl %es; \
      popl %fs; \
      popl %gs; \

#define EOI \
      movb $0x20, %al; \
      outb %al, $0x20;

ENTRY(clock_handler)
      SAVE_ALL
      EOI
      call clock_routine
      RESTORE_ALL
      iret

ENTRY(keyboard_handler)
      SAVE_ALL
      EOI
      call keyboard_routine
      RESTORE_ALL
      /*May need to remove error code (see page 26, point 5)*/
      iret

ENTRY(writeMSR)
      # Two parameters:
      # MSR Number
      # MSR Value
      push %ebp
      mov %esp, %ebp

      mov 0x8(%ebp), %ecx
      mov $0x0, %edx       # High part of the value is 0.
      mov 0xc(%ebp), %eax # Low part of the value is stored in the second parameter.
      wrmsr

      pop %ebp
      ret

ENTRY(syscall_handler_sysenter)
      push $__USER_DS
      push %EBP // User stack address
      pushfl
      push $__USER_CS
      push 4(%EBP) // User return address
      SAVE_ALL
      cmpl $0, %EAX
      jl sysenter_err
      cmpl $MAX_SYSCALL, %EAX
      jg sysenter_err
      call *sys_call_table(, %EAX, 0x04)
      jmp sysenter_fin
sysenter_err:
      movl $-ENOSYS, %EAX # ENOSYS WILL REPLACE
sysenter_fin:
      movl %EAX, 0x18(%ESP)
      RESTORE_ALL
      movl (%ESP), %EDX // Return address
      movl 12(%ESP), %ECX // User stack address
      sti // Enable interrupts again
      sysexit


