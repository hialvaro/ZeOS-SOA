#include <asm.h>
ENTRY(write)
	push %ebp
	mov %esp,%ebp
	
	# Save to user stack
	push %edx
	push %ecx

	# Pass the parametes to the corresponding registers:
	mov 0x08(%ebp), %ebx # fd -> ebx
	mov 0x0c(%ebp), %ecx # buffer -> ecx
	mov 0x10(%ebp), %edx # size -> edx
	

	# Now we need to put the identified of the system call in the EAX register
	movl 0x04, %eax # Number 4 for write.
	
	# EDX and ECX will be modified by sysexit. We must save them so we can restore them.
	push %ecx
	push %edx

	# Fake dynamic link?
	push $return
	push %ebp
	mov %esp,%ebp

	# Entramos
	sysenter

return:
	# Comprobamos si hay error en la ejecución de la syscall
	cmpl $0, %eax
	jge no_error
	# Si hay error, preparamos el contexto para retornar correctamente el código.
	negl %eax # Negamos EAX para obtener el valor absoluto
	movl %eax, errno
	movl -1, %eax
no_error:
	pop %edx
	pop %ecx
	pop %ebp
	ret
