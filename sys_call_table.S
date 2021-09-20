#include <asm.h>
#include <segment.h>

ENTRY (sys_call_table)
	.long sys_ni_syscall//0
