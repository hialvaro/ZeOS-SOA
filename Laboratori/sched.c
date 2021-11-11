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
int quantum_ticks;


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

void update_sched_data_rr(void)
{
	--quantum_ticks;
}

int needs_sched_rr(void)
{ // Si se ha superado el quantum devuelve 1.

	// Si el quantum es mayor que cero, se devuelve 0, aun puede seguir.
	if(quantum_ticks > 0) return 0;
	// Si el quantum no es mayor que cero y la lista de ready esta vacía
	// se hace reset del quantum y sigue.
	if(list_empty(&readyqueue)){
		quantum_ticks = current()->quantum;
		return 0;
	}
	// en caso contrario, se tiene que cambiar de proceso.
	return 1;
}

void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue)
{ //Elimina el proceso de su cola actual y lo inserta en una nueva cola.
	struct list_head * list_tmp = &t->list;
	/* Si el proceso esta dentro de una lista, va a tener un puntero al elemento previo y siguiente
	Así que solo comprobando esto podemos saber si es necesario eliminarlo de la lista dónde se encuentra*/
	if(!(list_tmp->prev == NULL && list_tmp->next == NULL)){
		list_del(list_tmp);
	}

	/* Si va a alguna lista se le añade. Si es running, se le deja sin colas. */
	if (dst_queue) list_add_tail(list_tmp, dst_queue);
}

void sched_next_rr(void)
{
	struct task_struct *next;

	// Si hay procesos en la lista de ready
	if(!list_empty(&readyqueue)){
		// Nos quedamos con el primero de la lista y lo eliminamos de esta.
		struct list_head *lf = list_first(&readyqueue);
		list_del(lf);
		next = list_head_to_task_struct(lf);
	}
	else{
		// Si la lista de ready esta vacía, se debe ejecutar el proceso idle.
		next = idle_task;
	}

	// Se pone el estado del proceso a RUN. - Creo que no es necesario
	// next->state=ST_RUN;

	// Se asigna la variable global de quantum el quantum del proceso siguiente.
	quantum_ticks = next->quantum;

	// Se hace el task_switch al nuevo proceso.
	task_switch(next);
}

void schedule(){
	update_sched_data_rr();
	if(needs_sched_rr()) {
		update_process_state_rr(current(), &readyqueue);
		sched_next_rr();
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
	pcb->quantum=QUANTUM;

	/* Ara utilitzem allocate_DIR per a inicialitzar el camp dir_pages_baseAddr
	   (on es troba l'adreça base del directori de pàgines del procés) amb un nou 
	   directori on guardarem l'espai d'adreces del procés. */
	allocate_DIR(pcb);

	/* A continuació inicialitzem un context d'execució per a poder restaurar-lo
	   quan idle s'assigni a la CPU i s'executi cpu_idle() */
	/* El procés idle no té context hardware ni context software. Només es necessita
	  	guardar la informació que necessitarà el task_switch per a posar en marxa un
	  	procés i fer el canvi. Aquesta informació és el ebp i l'adreça de retorn. */
	/* from Zeos.pdf: the context switch routine will be in
	charge of putting this process on execution. This means that we need to initialize the context of
	the idle process as the context switch routine requires to restore the context of any process. This
	context switch routine restores the execution of any process in the same state that it had before
	invoking it  */
	union task_union *tun = (union task_union*) pcb; // Task union corresponent al PCB de idle.
	tun -> stack[KERNEL_STACK_SIZE - 1] = (unsigned long) cpu_idle; // Store in the stack of the idle process the address of the code that it will execute (@ret)
	tun -> stack[KERNEL_STACK_SIZE - 2] = (unsigned long) 0; // the initial value that we want to assign to register ebp when undoing the dynamic link (it can be 0)
	tun -> task.kernel_esp = &(tun->stack[KERNEL_STACK_SIZE - 2]); // keep (in a field of its task_struct) the position of the stack where we have stored the initial value for the ebp register.
	
	/* Ara incialitzem la variable global idle_task, per a facilitar l'accés al 
	task_struct del procés idle.*/
	idle_task = pcb;
}

void init_task1(void)
{
	/* The code of this process is implemented in user.c.
	This function is called from main() in system.c*/

	/* Obtenim el primer element de la freequeue
	   utilitzant list_first(). La freequeue és
	   un vector de list_head(s).*/
	struct list_head *t = list_first(&freequeue);

	/* Ara aquest element ja no estarà lliure. Per tant
	 l'hem d'eliminar de la freequeue:*/
	list_del(t);

	/* Ara el convertim de list_head a task_struct: */
	struct task_struct *pcb = list_head_to_task_struct(t);

	/*Ara, com que és el procés init, li hem d'assignar el PID = 1*/
	pcb->PID = 1;
	pcb->quantum = QUANTUM;

	/* Ara utilitzem allocate_DIR per a inicialitzar el camp dir_pages_baseAddr
	   (on es troba l'adreça base del directori de pàgines del procés) amb un nou 
	   directori on guardarem l'espai d'adreces del procés. */
	allocate_DIR(pcb);

	/* Ara hem de inicialitzar el seu espai d'adreces. Utilitzem set_user_pages, que
	alocata les pàgines físiques que contindràn l'espai d'adreces d'usuari (codi + data)
	i afegeix a la taula de pàgines la traducció d'aquestes pàgines assignades.
	Cal recordar que la regió de kernel ja està configurada i és igual per a tots els processos. */
	set_user_pages(pcb);

	/* Ara cal actualitzar la TSS per fer que apunti a la pila de la nova tasca (new_task). */
	union task_union * tun1 = (union task_union*) pcb; // Obtenim el union task_union assignat al pcb de init.
	// Hi ha un define de KERNEL_ESP a sched.h que podem utilitzar per això. 
	tss.esp0 = KERNEL_ESP((union task_union *)pcb); //Fem que esp0 apunti a l'inici del codi d'usuari.
	// Modifiquem el registre SYSENTER_ESP_MSR, la pila de sistema operativa.
	writeMSR(0x175, (int) tss.esp0);

	/* Assignem la pàgina del directori del procés com la pàgina de directori actual en el sistema */
	set_cr3(pcb->dir_pages_baseAddr);
}

void inner_task_switch(union task_union *new_task){
	/* Primer cal actualitzar el punter de la pila de sistema a la TSS per a que apunti la pila de la 
	nova tasca. */
	//Fem que esp0 de la TSS apunti a l'inici del codi d'usuari.
	tss.esp0 = KERNEL_ESP((union task_union *)new_task); 
	// Modifiquem el registre SYSENTER_ESP_MSR, la pila de sistema operativa.
	writeMSR(0x175, (int) tss.esp0);

	/* A continuació canviem l'espai d'adreces d'usuari. Per a fer això s'ha d'actualitzar el directori
	de pàgines actual. Per a fer-ho, haurem de modificar el registre cr3 per a que apunti al directori de
	la nova tasca. Això també provoca un flush del TLB. */
	// get_DIR: Returns the Page Directory address for a task
	set_cr3(get_DIR(&(new_task->task)));

	/* Cambio a castellano ya que así lo podrá entender más gente en un futuro cuando haga esto público */
	/* Con asm se puede hacer uso de assembler en el mismo código de una función de C 
	   La primera variable se referencia como %0 y la segunda como %1 (si hay 2)*/
	/* Ahora hay que guardar el valor actual de EBP al  kernel_esp del PCB. EBP contiene la 
	dirección de la pila de sistema actual, donde empieza inner_task_switch */
	__asm__ __volatile__ ( 
		"mov %%ebp,%0" //%0 indica la variable
		: "=g" (current()->kernel_esp) // =g indica que la variable entre paréntesis se usa como destino.
		:); //No hay variable origen

	/* El siguiente paso es cambiar la pila de sistema actual haciendo que el registro EBP apunte al 
	valor del kernel_esp de new (guardado en el PCB de la nueva tarea) */
	// Esta funcion equivale a un mov del valor de kernel_esp del pcb a esp.
	__asm__ __volatile__ (
		"mov %0, %%esp"
		: // No hay variable destino
		: "g" (new_task->task.kernel_esp)); // g indica que la variable entre paréntesis es de origen

	/* Ahora se debe restaurar el registro EBP de la pila */
	__asm__ __volatile__ (
		"pop %%ebp"
		: //No hay destino
		:); //No hay origen

	/* Finalmente, retornamos a la rutina que ha llamado a inner_task_switch haciendo un RET */
	__asm__ __volatile__ (
		"ret"
		:
		:);
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

