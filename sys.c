/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern int zeos_ticks;
extern struct list_head freequeue;
int pid_next = 2;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -EACCES; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -ENOSYS; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int ret_from_fork() {
	return 0;
}

int sys_fork()
{
	int PID = -1;
	/* Se comprueba si la freequeue está vacía. Si es así se retorna un error */
	if(list_empty(&freequeue) == 1) return -EAGAIN;

	/* Se coge el primer task struct de la freequeue y se elimina de la lista.
	Ya que va a ser ejecutado. */
	struct list_head *fh = list_first(&freequeue);
	list_del(fh);

	/* Se delcara el PCB del hijo */
	struct task_struct *child = list_head_to_task_struct(fh);

	/* Se copia el PCB del padre al hijo */
	copy_data(current(), child, sizeof(union task_union));

	/* Se inicializa el directorio del hijo */
	allocate_DIR(child);

	/* Ahora se van a buscar páginas en las que mapear las páginas lógicas del hijo. */
	/* Si no hay suficientes páginas libres, se devuelve un error. */
	page_table_entry  *child_PT = get_PT(child);

	int npages[NUM_PAG_DATA];
	for(int i = 0; i < NUM_PAG_DATA; ++i){
		// Se hace la alocatación de una página.
		// Si es correcta se guarda el número de página.
		// Si es incorrecta hay -1
		npages[i] = alloc_frame();
		
		if(npages[i] < 0){
			// Se liberan los frames que ya han sido asignados (hasta i)
			for(int j = 0; j < i; j++) free_frame(npages[j]);
			// Volvemos a encolar el pcb a la freequeue
			list_add_tail(&child->list, &freequeue);
			return -ENOMEM;
		}
	}

	/* e) Se heredan los datos del padre */
	page_table_entry *parent_PT = get_PT(current());

	// Creación del espacio de direcciones del hijo
	// A) Código de sistema y usuario
	unsigned int parent_page;
	for(int i = 0; i < NUM_PAG_KERNEL; i++){
		parent_page = get_frame(parent_PT, i);
		set_ss_pag(child_PT, i, parent_PT);
	}

	for(int i = 0; i < NUM_PAG_CODE; i++){
		parent_page = get_frame(parent_PT, PAG_LOG_INIT_CODE+i);
		set_ss_pag(child_PT, PAG_LOG_INIT_CODE+i, parent_PT);
	}

	// B) Apuntamos las nuevas páginas físicas a las direcciones lógicas del hijo.
	for(int i = 0; i < NUM_PAG_DATA; ++i){
		set_ss_pag(child_PT, PAG_LOG_INIT_DATA+i, npages[i]);
	}

	/* Se amplia el espacio de direcciones del padre para poder traducir desde el TLB las
	páginas físicas del hijo y poder copiar las del padre. */
	int SHARED_SPACE = NUM_PAG_KERNEL+NUM_PAG_CODE;
	int TOTAL_SPACE = NUM_PAG_CODE+NUM_PAG_KERNEL+NUM_PAG_DATA;

	for(int i = SHARED_SPACE; i < TOTAL_SPACE; ++i){
		unsigned int temp_page = i+PAG_LOG_INIT_DATA; //review 
		set_ss_pag(parent_PT, temp_page, npages[i]);
		// Las páginas estan alienadas a 4KB, por ese motivo se hace shift de 12bits para 
		// tener 3 ceros al final:
		copy_data((void *) (i << 12), (void *) (temp_page << 12), PAGE_SIZE);
		del_ss_pag(parent_PT, temp_page);
	}

	/* Forzamos un FLUSH del TLB para eliminar los accesos del padre al hijo del TLB */
	set_cr3(get_DIR(current()));

	/* f) Asignamos un nuevo PCB (y incrementamos el contador global de PID) */
	PID = pid_next++;
	child->PID = PID;

	/*     APARTADO g(i)    */
	/*??????????????????????*/

	/* h) Emulamos el contenido que espera una llamada al task switch para que se pueda ejecutar */
	// fake ebp
	// ret
	// @HANDLER
	// CTX SW
	// CTX HW
	// ------
	/* CTX HW (5 regs) + CTX SW (11 regs) + @handler = 17 espacios de la pila = 0x11*/
	union task_union * child_uniontu = (union task_union *) child;
	((unsigned long *)KERNEL_ESP(child_uniontu))[-0x13] = (unsigned long) 0; // Fake EBP
	((unsigned long *)KERNEL_ESP(child_uniontu))[-0x12] = (unsigned long) ret_from_fork; // @ret
	child->kernel_esp = &((unsigned long *)KERNEL_ESP(child_uniontu))[-0x13];

	/*j) Devolvemos el PID del hijo */
	return PID;
}

void sys_exit()
{  
}

sys_gettime(){
	return zeos_ticks;
}

char buffer_k[256];
#define BUFFER_SIZE 256

int sys_write(int fd, char * buffer, int size){

	// Si el valor es 1, es error.
	int fd_error = check_fd(fd, ESCRIPTURA);
	if(fd_error) return fd_error; // Si es error, retornem error (valor negatiu amb codi error).

	if(buffer == NULL) return -EFAULT; // EFAULT REPLACE
	if(size < 0) return -EINVAL; // EINVAL REPLACE

	int bytes = size;
	int written_bytes; 

	while(bytes > BUFFER_SIZE){
		copy_from_user(buffer+(size-bytes), buffer_k, BUFFER_SIZE);
		written_bytes = sys_write_console(buffer_k, BUFFER_SIZE);
		
		buffer = buffer+BUFFER_SIZE;
		bytes = bytes-written_bytes;
	}

	// Copy the left bytes (if there are less than 256 bytes left)
	copy_from_user(buffer+(size-bytes), buffer_k, bytes);
	written_bytes = sys_write_console(buffer_k, bytes);
	bytes = bytes-written_bytes;	

	return size-bytes;
}