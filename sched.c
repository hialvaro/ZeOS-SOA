/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;
struct list_head freequeue, readyqueue;
struct task_struct * idle_task;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	;
	}
}

void init_idle (void)
{
	/* Obtenim el primer element de la freequeue
	   utilitzant list_first(). La freequeue és
	   un vector de list_head(s).*/
	struct list_head *t = list_first(&freequeue);

	/* Ara aquest element ja no estarà lliure. Per tant
	 l'hem d'eliminar de la freequeue:*/
	list_del(t);

	/* Ara el convertim de list_head a task_struct: */
	struct task_struct *pcb = list_head_to_task_struct(t);

	/*Ara, com que és el procés idle, li hem d'assignar el PID = 0*/
	pcb->PID = 0;

	/* Ara utilitzem allocate_DIR per a inicialitzar el camp dir_pages_baseAaddr
	   (on es troba l'adreça base del directori de pàgines del procés) amb un nou 
	   directori on guardarem l'espai d'adreces del procés. */
	allocate_DIR(pcb);

	/* A continuació inicialitzem un context d'execució per a poder restaurar-lo
	   quan idle s'assigni a la CPU i s'executi cpu_idle() */
	/* El procés idle no té context hardware ni context software. Només es necessita
	  	guardar la informació que necessitarà el task_switch per a posar en marxa un
	  	procés i fer el canvi. Aquesta informació és el ebp i l'adreça de retorn. */
	union task_union *tun = (union task_union*) pcb; // Task union corresponent al PCB de idle.
	tun -> stack[KERNEL_STACK_SIZE - 1] = (unsigned long) cpu_idle; // @ret
	tun -> stack[KERNEL_STACK_SIZE - 2] = (unsigned long) 0; // ebp (fake)
	tun -> task.kernel_esp = &(tun->stack[KERNEL_STACK_SIZE - 2]); // mov ebp -> esp

	/* Ara incialitzem la variable global idle_task, per a facilitar l'accés al 
	task_struct del procés idle.*/
	idle_task = pcb;
}

void init_task1(void)
{
}


void init_sched()
{
	INIT_LIST_HEAD(&freequeue);
	INIT_LIST_HEAD(&readyqueue);
	int i;
	for(i = 0; i<NR_TASKS;i++){
		list_add( &(task[i].task.list), &freequeue);
	}
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

