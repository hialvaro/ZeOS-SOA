# 1 "kernel-utils.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "kernel-utils.S"
# 1 "include/asm.h" 1
# 2 "kernel-utils.S" 2


.globl switch_stack; .type switch_stack, @function; .align 0; switch_stack:
 mov 4(%esp), %eax
 mov %ebp, (%eax)
 mov 8(%esp), %esp
 popl %ebp
 ret


.globl task_switch; .type task_switch, @function; .align 0; task_switch:
 pushl %ebp
 movl %esp, %ebp
 pushl %esi
 pushl %edi
 pushl %ebx
 pushl 8(%ebp)
 call inner_task_switch
 addl $4, %esp
 popl %ebx
 popl %edi
 popl %esi
 popl %ebp
 ret


.globl get_ebp; .type get_ebp, @function; .align 0; get_ebp:
 mov %ebp, %eax
 ret


.globl setMSR; .type setMSR, @function; .align 0; setMSR:
 push %ebp
 movl %esp, %ebp
 movl 0x10(%ebp), %eax
 movl 0xc(%ebp), %edx
 movl 0x8(%esp), %ecx
 wrmsr
 pop %ebp
 ret
