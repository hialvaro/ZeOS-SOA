#include <asm.h>
ENTRY(write)
	pushl %ebp
	mov %esp,%ebp
	
	# Save to user stack
	pushl %edx
	pushl %ecx

	# Pass the parametes to the corresponding registers:
	mov 0x08(%ebp), %ebx # fd -> ebx
	mov 0x0c(%ebp), %ecx # buffer -> ecx
	mov 0x10(%ebp), %edx # size -> edx
	

	# Now we need to put the identified of the system call in the EAX register
	movl $4, %eax # Number 4 for write.
	
	# EDX and ECX will be modified by sysexit. We must save them so we can restore them.
	# push %ecx
	# push %edx

	# Fake dynamic link?
	pushl $wr_return
	pushl %ebp
	mov %esp,%ebp

	# Entramos
	sysenter

wr_return:
	# Comprobamos si hay error en la ejecución de la syscall
	popl %ebp
	addl $4, %esp
	popl %edx
	popl %ecx
	cmpl $0, %eax
	jge wr_no_error
	# Si hay error, preparamos el contexto para retornar correctamente el código.
	negl %eax # Negamos EAX para obtener el valor absoluto
	movl %eax, errno
	movl -1, %eax
wr_no_error:
	popl %ebp
	ret

ENTRY(gettime)
	push %ebp
	mov %esp,%ebp
	
	# Save to user stack
	push %edx
	push %ecx
	

	# Now we need to put the identified of the system call in the EAX register
	movl $10, %eax
	
	# EDX and ECX will be modified by sysexit. We must save them so we can restore them.
	#push %ecx
	#push %edx

	# Fake dynamic link?
	push $gt_return
	push %ebp
	mov %esp,%ebp

	# Entramos
	sysenter

gt_return:
	# Comprobamos si hay error en la ejecución de la syscall
	pop %ebp
	add $4, %esp
	pop %edx
	pop %ecx
	cmp $0, %eax
	jge gt_no_error
	# Si hay error, preparamos el contexto para retornar correctamente el código.
	neg %eax # Negamos EAX para obtener el valor absoluto
	mov %eax, errno
	mov -1, %eax
gt_no_error:
	pop %ebp
	ret


ENTRY(getpid)
	push %ebp
	mov %esp,%ebp
	
	# Save to user stack
	push %edx
	push %ecx
	

	# Now we need to put the identified of the system call in the EAX register
	movl $20, %eax
	
	# EDX and ECX will be modified by sysexit. We must save them so we can restore them.
	#push %ecx
	#push %edx

	# Fake dynamic link?
	push $gp_return
	push %ebp
	mov %esp,%ebp

	# Entramos
	sysenter

gp_return:
	# Comprobamos si hay error en la ejecución de la syscall
	pop %ebp
	add $4, %esp
	pop %edx
	pop %ecx
	cmp $0, %eax
	jge gp_no_error
	# Si hay error, preparamos el contexto para retornar correctamente el código.
	neg %eax # Negamos EAX para obtener el valor absoluto
	mov %eax, errno
	mov -1, %eax
gp_no_error:
	pop %ebp
	ret