#include <asm.h>

ENTRY(syscall_sysenter)
	push %ecx
	push %edx
	push $SYSENTER_RETURN
	push %ebp
	mov %esp, %ebp
	sysenter
ENTRY(SYSENTER_RETURN)
	pop %ebp
	pop %edx
	pop %edx
	pop %ecx
	ret


/* int write(int fd, char *buffer, int size) */
ENTRY(write)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $4, %eax
	movl 0x8(%ebp), %ebx;	//fd
	movl 0xC(%ebp), %ecx;	//buffer
	movl 0x10(%ebp), %edx;	//size
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) ->
	popl %ebp
	ret

/* Common code for negative return */
nok:
	neg %eax
	mov %eax, errno
	mov $-1, %eax
	popl %ebp
	ret

/* int gettime() */
ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp
	movl $10, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int getpid() */
ENTRY(getpid)
	pushl %ebp
	movl %esp, %ebp
	movl $20, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int fork() */
ENTRY(fork)
	pushl %ebp
	movl %esp, %ebp
	movl $2, %eax
	call syscall_sysenter
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* void exit() */
ENTRY(exit)
	pushl %ebp
	movl %esp, %ebp
	movl $1, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int yield() */
ENTRY(yield)
	pushl %ebp
	movl %esp, %ebp
	movl $13, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* int get_stats(int pid, struct stats *st) */
ENTRY(get_stats)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $35, %eax
	movl 0x8(%ebp), %ebx;	//pid
	movl 0xC(%ebp), %ecx;	//st
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret

/* Semaphores */
ENTRY(sem_init)
	push %ebp
	mov %esp, %ebp
	push %ecx
	push %ebx
	mov 12(%ebp), %ecx   // count
	mov 8(%ebp), %ebx    // sem_id
	lea ret_from_sem, %eax
	push %eax
	push %ebp
	movl %esp, %ebp
	mov $21, %eax // Syscall 21 is sem_init
	sysenter

// Common Semaphores code
ret_from_sem:
    pop %ebp
	add $4, %esp
	cmp $0, %eax
	je end_sem // Si no hay error, salta a fin
	lea errno, %ebx
	mov %eax, (%ebx)
	mov $-1, %eax
end_sem:
	pop %ebx
	pop %ecx
	mov %ebp, %esp
	pop %ebp
	ret

ENTRY(sem_wait)
	push %ebp
	mov %esp, %ebp
	push %ecx
	push %ebx
	//mov 12(%ebp), %ecx   // count
	mov 8(%ebp), %ebx    // sem_id
	lea ret_from_sem, %eax
	push %eax
	push %ebp
	movl %esp, %ebp
	mov $22, %eax // Syscall 21 is sem_init
	sysenter


ENTRY(sem_signal)
	push %ebp
	mov %esp, %ebp
	push %ecx
	push %ebx
	//mov 12(%ebp), %ecx   // count
	mov 8(%ebp), %ebx    // sem_id
	lea ret_from_sem, %eax
	push %eax
	push %ebp
	movl %esp, %ebp
	mov $23, %eax // Syscall 21 is sem_init
	sysenter


ENTRY(sem_destroy)
	push %ebp
	mov %esp, %ebp
	push %ecx
	push %ebx
	//mov 12(%ebp), %ecx   // count
	mov 8(%ebp), %ebx    // sem_id
	lea ret_from_sem, %eax
	push %eax
	push %ebp
	movl %esp, %ebp
	mov $24, %eax // Syscall 21 is sem_init
	sysenter

	/*int pipe(int * fd)*/
ENTRY(pipe)
	push %ebp
	mov %esp, %ebp
	push %ecx
	push %ebx
	//mov 12(%ebp), %ecx   // count
	mov 8(%ebp), %ebx    // fd
	lea ret_from_sem, %eax
	push %eax
	push %ebp
	movl %esp, %ebp
	mov $15, %eax // Syscall 21 is sem_init
	sysenter

	/*int read(int fd, char * user_buff, int count)*/
ENTRY(read)
	push %ebp
	mov %esp, %ebp
	push %ecx
	push %ebx
	movl 0x8(%ebp), %ebx;	//fd
	movl 0xC(%ebp), %ecx;	//buffer
	movl 0x10(%ebp), %edx;	//size
	lea ret_from_read, %eax
	push %eax
	push %ebp
	movl %esp, %ebp
	mov $17, %eax // Syscall 21 is sem_init
	sysenter


	ENTRY(close)
		push %ebp
		mov %esp, %ebp
		push %ecx
		push %ebx
		//mov 12(%ebp), %ecx   // count
		mov 8(%ebp), %ebx    // sem_id
		lea ret_from_sem, %eax
		push %eax
		push %ebp
		movl %esp, %ebp
		mov $16, %eax // Syscall 21 is sem_init
		sysenter


// Common Semaphores code
ret_from_read:
  pop %ebp
	add $4, %esp
	cmpl $-1, %eax
	jg end_read // Si no hay error, salta a fin
	lea errno, %ebx
	mov %eax, (%ebx)
	mov $-1, %eax
end_read:
	pop %ebx
	pop %ecx
	mov %ebp, %esp
	pop %ebp
	ret
