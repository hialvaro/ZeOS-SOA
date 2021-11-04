#include <asm.h>
#include <segment.h>

ENTRY(task_switch)
	# Preparamos el contexto:
	# Se hace push del %ebp (stack pointer) actual a la pila para poder recuperarlo posteriormente.
	push %ebp # se guarda el frame pointer
	mov %esp, %ebp # Se pone el stack pointer apuntando a la base.

	# Se guardan los registros
	push %ebx
	push %esi
	push %edi

	# Hacemos un push del segundo parámetro 
	# (recoremos que es un union task union y el primer "int" es el pcb)
	push 8(%ebp)
	# Se hace una llamada al task_switch en C
	call inner_task_switch
	# Se restaura el ESP restándole 4
	addl $4, %esp

	# Recuperamos los valores que habíamos pusheado a la pila
	pop %edi
	pop %esi
	pop %ebx

	# restauramos el stack pointer para la funcion que nos ha llamado
	mov %ebp,%esp
	pop %ebp # restauramos también el antiguo stack pointer
	ret