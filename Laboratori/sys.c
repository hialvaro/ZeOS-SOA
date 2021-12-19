/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#include <sem.h>

#include <tfa.h>

#define LECTURA 0
#define ESCRIPTURA 1

void * get_ebp();

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF;
  if (permissions!=ESCRIPTURA) return -EACCES;
  return 0;
}

int myCheck_fd(int fd, enum mode_t mode) {

  struct entry *e = &current()->Channels.entrys[fd];
  if (! e->fd) return -EBADF;
  if (! (e->mode == mode)) return -EACCES;
  return 0;
}
void user_to_system(void)
{
  update_stats(&(current()->p_stats.user_ticks), &(current()->p_stats.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->p_stats.system_ticks), &(current()->p_stats.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS;
}

int sys_getpid()
{
	return current()->PID;
}

int global_PID=1000;

int ret_from_fork()
{
  return 0;
}

int sys_fork(void)
{
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;

  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);

  list_del(lhcurrent);

  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);

  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));

  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);

  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  for (pag=0; pag<NUM_PAG_DATA; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);

      /* Return error */
      return -EAGAIN;
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag=NUM_PAG_KERNEL+NUM_PAG_CODE; pag<NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, pag+NUM_PAG_DATA, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, pag+NUM_PAG_DATA);
  }
  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));

  uchild->task.PID=++global_PID;
  uchild->task.state=ST_READY;

  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.register_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp=*(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=(DWord)&ret_from_fork;
  uchild->task.register_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.register_esp)=temp_ebp;

  /* Set stats to 0 */
  init_stats(&(uchild->task.p_stats));

  /* Queue child process into readyqueue */
  uchild->task.state=ST_READY;
  list_add_tail(&(uchild->task.list), &readyqueue);

  return uchild->task.PID;
}

#define TAM_BUFFER 512

int sys_write(int fd, char *buffer, int nbytes) {
char localbuffer [TAM_BUFFER];
int bytes_left;
int ret;

	if ((ret = check_fd(fd, ESCRIPTURA)))
		return ret;
	if (nbytes < 0)
		return -EINVAL;
	if (!access_ok(VERIFY_READ, buffer, nbytes))
		return -EFAULT;

	bytes_left = nbytes;
	while (bytes_left > TAM_BUFFER) {
		copy_from_user(buffer, localbuffer, TAM_BUFFER);
		ret = sys_write_console(localbuffer, TAM_BUFFER);
		bytes_left-=ret;
		buffer+=ret;
	}
	if (bytes_left > 0) {
		copy_from_user(buffer, localbuffer,bytes_left);
		ret = sys_write_console(localbuffer, bytes_left);
		bytes_left-=ret;
	}
	return (nbytes-bytes_left);
}


extern int zeos_ticks;

int sys_gettime()
{
  return zeos_ticks;
}

void sys_exit()
{
  int i;

  page_table_entry *process_PT = get_PT(current());

  // Deallocate all the propietary physical pages
  for (i=0; i<NUM_PAG_DATA; i++)
  {
    free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
    del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
  }

  /* Free task_struct */
  list_add_tail(&(current()->list), &freequeue);

  current()->PID=-1;

  /* Restarts execution of the next process */
  sched_next_rr();
}

/* System call to force a task switch */
int sys_yield()
{
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st)
{
  int i;

  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT;

  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.p_stats.remaining_ticks=remaining_quantum;
      copy_to_user(&(task[i].task.p_stats), st, sizeof(struct stats));
      return 0;
    }
  }
  return -ESRCH; /*ESRCH */
}

/* SEMAPHORES */

int get_sem_pos_from_id(int sem_id) {
  // Gets a semaphore id and returns its position.
	for (int i = 0; i < NR_SEMAPHORES; i++){
		if (semaphores[i].sem_id == sem_id) return i;
  }
	return -1;
}

int get_free_sem() { //Returns the position of a free semaphore.
	for (int i = 0; i < NR_SEMAPHORES; i++){
		if (semaphores[i].sem_id < 0) return i;
  }
	return -1;
}


int sys_sem_init(int sem_id, unsigned int count) {

  if (sem_id < 0) return -1;                          // El id del semaforo no es valido
	if (get_sem_pos_from_id(sem_id) != -1) return -1;   // El identrificardot ya esta en uso
	int pos = get_free_sem();
	if (pos == -1) return -1;                           // No hay espació para crear un semaforo.

  return sem_init(sem_id, count, semaphores[pos]);
}

int sys_sem_wait(int sem_id) {

  int pos = get_sem_pos_from_id(sem_id);
  if (sem_id < 0 || (pos == -1)) return -1;          // El semaforo no existe.
  if (semaphores[pos].pid_owner == -1) return -1;

  return sem_wait(semaphores[pos]);
}

int sys_sem_signal(int sem_id) {

  int pos = get_sem_pos_from_id(sem_id);
  if (sem_id < 0 || (pos == -1)) return -1;                            // No existe el semaforo.
  if (semaphores[pos].pid_owner == -1) return -1;

  return sem_singal();
}

int sys_sem_destroy(int sem_id) {

  int pos = get_sem_pos_from_id(sem_id);
  if (sem_id < 0 || (pos == -1)) return -1;               // No existe el semaforo.

  return sem_destroy(semaphores[pos]);
}


int sys_pipe(int * pd) {
  printk("\nEntro en sys_pipe.\n");
  //Primero de todo comprobamos si se puede crear una nueva pipe, y donde colocarlos en las tablas.
  short primer= 0, segon = 0;
  int tfa = -1;
  int i = 2;
  while ((!primer && !segon) || i < NUM_CHANELS) {    //<----- ESTO ESTA MAL,
    if (current()->Channels.entrys[i].fd == -1 && !segon && !primer) primer = i;
    else if( current()->Channels.entrys[i].fd == -1 && primer) segon = i;
    ++i;
  }

  if (!segon) return -1;  //CAMBIAR no hay más canales disponibles del proceso.}
  for (i= 0; i < MAX_TFAS && tfa == -1; ++i) {
    if(tfas_table.users[i] == 0) tfa = i;
  }

  if (tfa == -1) return -1; // CAMBIAR No se puede crear mas pipes

  int frame = alloc_frame();
  if (frame == -1) return -1; //CAMBIAR No hay memoria suficiente.

  int freeSem = get_free_sem();
  if (freeSem == -1) return -1; //CAMBIAR no hay semaforos disponilbes.

  page_table_entry * pt = get_PT(current());
  int newLogicalPage = PAG_LOG_INIT_DATA+NUM_PAG_DATA+tfa;
  set_ss_pag(pt, newLogicalPage, frame);

  current()->Channels.entrys[primer].fd = primer;
  current()->Channels.entrys[segon].fd = segon;
  current()->Channels.entrys[primer].mode = READ_ONLY;
  current()->Channels.entrys[primer].mode = WRITE_ONLY;
  current()->Channels.entrys[primer].file = &(tfas_table.tfas[tfa]);
  current()->Channels.entrys[segon].file = &(tfas_table.tfas[tfa]);


  char * initPosPipe = (char*)(  ((unsigned long)(newLogicalPage)) << 12  );

  tfas_table.users[tfa] = 1;
  tfas_table.tfas[tfa].nextWritten = initPosPipe;
  tfas_table.tfas[tfa].nextRead = initPosPipe;
  tfas_table.tfas[tfa].availablebytes = 4096;
  tfas_table.tfas[tfa].nreaders = 1;
  tfas_table.tfas[tfa].nwriters = 1;
  int sem = sys_sem_init(123, 1);
  //if (sem != 0) printk("No se puede crear la pipe");
  tfas_table.tfas[tfa].semaphore =  sys_sem_init(123, 1);
  tfas_table.tfas[tfa].frame = frame;
  tfas_table.tfas[tfa].initialPointer = initPosPipe;

  pd[0] = primer;
  pd[1] = segon;
  printk("\nLo hace todo sin petar ole los caracole.\n");

  return 0;
}

int sys_read(int fd, void * buff, int count) {

  /*int ret;
  ret = check_fd(fd, READ_ONLY);
  if (ret != 0) return ret;
  if (buff == NULL) return -EFAULT; //BUSCAR MAS TARDE QUE PONER AQUÍ
  if (count <= 0 || count >= -EINVAL);

  struct tfa  * file = current()->Channels.entrys[fd].file;
  int rest = count;

  char * buff = "mensaje que se tiene que leer";
  //comprobamos que haya datos que leer, sino nos bloqueamos
  if (file->availablebytes == 4096) {
    sys_sem_wait(file->semaphore);
  }
  else {
    if (file->nextWritten > file->nextRead) {
      if (file->nextRead + count > file->nextWritten) {
        copy_to_user((void *) file->nextRead, buff, (file->nextWritten - file->nextRead));
        file->nextRead = file->nextWritten;
        file->availablebytes = 4096;
      }
      else {
        copy_to_user((void *) file->nextRead, buff, count);
        file->nextRead += count;
      }
    }
    else {
      if (file->nextRead + count < file->initialPointer+4096){
        copy_to_user((void *) file->nextRead, buff, count);
        file->nextRead += count;
        file->availablebytes -= count;
      }
      else {
        copy_to_user((void *) file->nextRead, buff, (file->initialPointer + 4096 - file->nextRead));
        file->nextRead = file->initialPointer;
        file->availablebytes -=  (file->initialPointer + 4096 - file->nextRead);
        rest = count - (file->initialPointer + 4096 - file->nextRead);
        if (file->nextRead + rest > file->nextWritten) {
          copy_to_user((void *) file->nextRead, buff, (file->nextWritten - file->nextRead));
          file->nextRead = file->nextWritten;
          file->availablebytes = 4096;
        }
        else {
          copy_to_user(file->nextRead, buff, rest);
          file->nextRead += count;
          file->availablebytes -= rest;
        }
      }
    }
  }*/
  return 0;
}
