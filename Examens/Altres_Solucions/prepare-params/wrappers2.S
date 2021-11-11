#include <asm.h>

ENTRY(prepare_params)
    movl 0x8(%ebp), %ebx;
	movl 0xC(%ebp), %ecx;
	movl 0x10(%ebp), %edx;
	movl 0x14(%ebp), %esi;
	movl 0x18(%ebp), %edi;
    ret

nok:
	neg %eax
	mov %eax, errno
	mov $-1, %eax
	ret

ENTRY(do_return_job)
	test %eax, %eax
	js nok	// if (eax < 0)
    ret