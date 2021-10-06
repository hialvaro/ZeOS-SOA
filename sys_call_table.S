#include <asm.h>
#include <segment.h>

ENTRY (sys_call_table)
	.long sys_ni_syscall//0
	.long sys_ni_syscall//1
	.long sys_ni_syscall//2
	.long sys_ni_syscall//3
	.long sys_write//4
	.long sys_ni_syscall//5
	.long sys_ni_syscall//6
	.long sys_ni_syscall//7
	.long sys_ni_syscall//8
	.long sys_ni_syscall//9
	.long sys_gettime//10

.globl MAX_SYSCALL
MAX_SYSCALL = (. - sys_call_table)/4
