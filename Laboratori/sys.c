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

  struct entry *e = &current()->tc[fd];
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
  //AQUÍ AÑADIMOS LAS PIPES QUE TENGA EL PADRE ASOCIADO
  // AÑADIMOS LA PIPE A LA TABLA DE CANALES.
  int files[MAX_TFAS] = {0};
  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------Pendiente de mirar talvez es una patinada
  for (int i = 2; i < NUM_CHANELS; ++i) {
    if (current()->tc[i].fd > 0) {
      struct openFile * file = current()->tc[i].file;
      int nfile = current()->tc[i].pos;
      if (files[i] == 0) {
        set_ss_pag(process_PT, PAG_LOG_INIT_DATA+NUM_PAG_DATA+nfile, file->frame);
        ++files[i];
      }
      else {
        if (current()->tc[i].mode==READ_ONLY) ++(file->nreaders);
        if (current()->tc[i].mode==WRITE_ONLY) ++(file->nwriters);
      }
      ++tfa.users[nfile];
    }
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

int sys_write(int fd, char *buff, int nbytes) {

  if (fd == 1) {
    char localbuffer [TAM_BUFFER];
    int bytes_left;
    int ret;

	   if ((ret = check_fd(fd, ESCRIPTURA)))
		   return ret;
	   if (nbytes < 0)
		   return -EINVAL;
	   if (!access_ok(VERIFY_READ, buff, nbytes))
		   return -EFAULT;

	   bytes_left = nbytes;
	   while (bytes_left > TAM_BUFFER) {
		     copy_from_user(buff, localbuffer, TAM_BUFFER);
		     ret = sys_write_console(localbuffer, TAM_BUFFER);
		     bytes_left-=ret;
		     buff+=ret;
	   }
	   if (bytes_left > 0) {
		     copy_from_user(buff, localbuffer,bytes_left);
		     ret = sys_write_console(localbuffer, bytes_left);
		     bytes_left-=ret;
	  }
	  return (nbytes-bytes_left);
  }
  else {
    int ret;
    ret = myCheck_fd(fd, WRITE_ONLY);
    if (ret != 0) return ret;

    if (buff == NULL) return -EFAULT; //BUSCAR MAS TARDE QUE PONER AQUÍ
    if (nbytes <= 0 || nbytes >= -EINVAL);
    struct openFile  * file = current()->tc[fd].file;
    int rest = nbytes;

    while (rest > 0) {

      if (file->availablebytes == 0) wait(& (file->semRead));
      if (file->nextWritten < file->nextRead) {
        if (file->nextWritten + rest >= file->nextRead) {
          printk("\nif 1 W. \n");
          copy_from_user( buff, (void *) &(file->nextWritten), (file->nextRead - file->nextWritten));
          file->nextWritten = file->nextRead;
          file->availablebytes = 0;
          rest -=  (file->nextRead - file->nextWritten);
          buff = (void *) (((char * ) buff) + (file->nextRead - file->nextWritten));
        }
        else {
          printk("\nif 2 W. \n");
          copy_from_user(buff, (void *) &(file->nextWritten), rest);
          file->nextWritten += rest;
          file->availablebytes -= rest;
          rest = 0;
        }
      }
      else {
        if (file->nextWritten + rest < file->initialPointer+4096){
          if (file->nextWritten == file->initialPointer) printk("\nescribe des de el comienzo.");
          else printk("\nescribe lee des de el comienzo.");
          printk("\nif 3 W. \n");
          copy_from_user((void *) buff, (void *) &(file->nextWritten), rest);
          file->nextWritten += rest;
          file->availablebytes -= rest;
          rest = 0;
        }
        else {
          printk("\nif 4 W. \n");
          copy_from_user(buff, (void *) &(file->nextWritten), (file->initialPointer + 4096 - file->nextWritten));
          file->nextWritten = file->initialPointer;
          file->availablebytes -=  (file->initialPointer + 4096 - file->nextWritten);
          rest -= (file->initialPointer + 4096 - file->nextWritten);
          buff = (void *) (((char * ) buff) + (file->initialPointer + 4096 - file->nextWritten));

        }
      }
    }
    if(file->semWrite.count <= 0) signal(& (file->semWrite));
    return 0;
    }
}


extern int zeos_ticks;

int sys_gettime() {
  return zeos_ticks;
}

void sys_exit() {
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
int sys_yield() {
  force_task_switch();
  return 0;
}

extern int remaining_quantum;

int sys_get_stats(int pid, struct stats *st) {
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

  return init(sem_id, count, &semaphores[pos]);
}

int sys_sem_wait(int sem_id) {

  int pos = get_sem_pos_from_id(sem_id);
  if (sem_id < 0 || (pos == -1)) return -1;          // El semaforo no existe.
  if (semaphores[pos].pid_owner == -1) return -1;

  return wait(&semaphores[pos]);
}

int sys_sem_signal(int sem_id) {

  int pos = get_sem_pos_from_id(sem_id);
  if (sem_id < 0 || (pos == -1)) return -1;                            // No existe el semaforo.
  if (semaphores[pos].pid_owner == -1) return -1;

  return signal(&semaphores[pos]);
}

int sys_sem_destroy(int sem_id) {

  int pos = get_sem_pos_from_id(sem_id);
  if (sem_id < 0 || (pos == -1)) return -1;               // No existe el semaforo.

  return destroy(&semaphores[pos]);
}


int sys_pipe(int * pd) {

  printk("\nEntro en sys_pipe.\n");
  //Primero de todo comprobamos si se puede crear una nueva pipe, y donde colocarlos en las tablas.
  short primer = 0, segon = 0;
  int nfile = -1;
  int i = 2;

  for (int i = 2; i < NUM_CHANELS; ++i) {
    if (current()->tc[i].fd == -1) {
      if (primer == 0) primer = i;
      else if (segon == 0) segon = i;
      else break;
    }
  }
  if (!segon || !primer) return -1;                 //CAMBIAR no hay más canales disponibles del proceso.}


  for (i= 0; i < MAX_TFAS && nfile == -1; ++i)
    if(tfa.users[i] == 0) nfile = i;
  if (nfile == -1) return -1;                         // CAMBIAR No se puede crear mas pipes

  int frame = alloc_frame();
  if (frame == -1) return -1;                       //CAMBIAR No hay memoria suficiente.

  // AÑADIMOS LA PIPE A LA TABLA DE CANALES.
  page_table_entry * pt = get_PT(current());
  int newLogicalPage = PAG_LOG_INIT_DATA+NUM_PAG_DATA+nfile;
  set_ss_pag(pt, newLogicalPage, frame);

  current()->tc[primer].fd = primer;
  current()->tc[segon].fd = segon;
  current()->tc[primer].mode = READ_ONLY;
  current()->tc[segon].mode = WRITE_ONLY;
  current()->tc[primer].pos = nfile;
  current()->tc[segon].pos  = nfile;
  current()->tc[primer].file = &(tfa.tfas[nfile]);
  current()->tc[segon].file = &(tfa.tfas[nfile]);


  // AÑADIMOS LA PIPE A LA TFA Y CREAMOS EL BUFFER.
  char * initPosPipe = (char*)(  ((unsigned long)(newLogicalPage)) << 12  );

  ini_fa(nfile, initPosPipe, frame);
  pd[0] = primer;
  pd[1] = segon;
  printk("\nLo hace todo sin petar ole los caracole.\n");

  return 0;
}

int sys_read(int fd, void * buff, int count) {

  // COMPROVAMOS QUE SE EXISTA EL CANAL Y EL BUFFER SEA VALIDO
  int ret;
  ret = myCheck_fd(fd, READ_ONLY);
  if (ret != 0) return ret;

  if (buff == NULL) return -EFAULT; //BUSCAR MAS TARDE QUE PONER AQUÍ
  if (count <= 0 || count >= -EINVAL);
  struct openFile  * file = current()->tc[fd].file;
  int rest = count;

  /*
  char * p = "Pasame esta.";
  void * d = & (file->nextRead);
  file->nextWritten += 12;
  file->availablebytes -= 12;
  copy_from_user((void *) p, d, 12);
  rest = 12;
  printk("\ncopiado al pipe");*/
  //comprobamos que haya datos que leer, sino nos bloqueamos
  while (rest > 0) {

    if (file->availablebytes == 4096) wait(& (file->semWrite));
    if (file->nextWritten > file->nextRead) {
      if (file->nextRead + rest >= file->nextWritten) {
        printk("\nif 1");
        copy_to_user((void *) &(file->nextRead), buff, (file->nextWritten - file->nextRead));
        file->nextRead = file->nextWritten;
        file->availablebytes = 4096;
        rest -=  (file->nextWritten - file->nextRead);
        buff = (void *) (((char * ) buff) + (file->nextWritten - file->nextRead));
      }
      else {
        printk("\nif 2\n");
        if (file->nextRead == file->initialPointer) printk("\nlee des de el comienzo.");
        else printk("\nNo lee des de el comienzo.");
        copy_to_user((void *) &(file->nextRead), buff, rest);
        file->nextRead += rest;
        file->availablebytes += rest;
        rest = 0;
      }
    }
    else {
      if (file->nextRead + rest < file->initialPointer+4096){
        printk("\nif 3");
        copy_to_user((void *) &(file->nextRead), buff, rest);
        file->nextRead += rest;
        file->availablebytes += rest;
        rest = 0;
      }
      else {
        printk("\nif 4");
        copy_to_user((void *) &(file->nextRead), buff, (file->initialPointer + 4096 - file->nextRead));
        file->nextRead = file->initialPointer;
        file->availablebytes +=  (file->initialPointer + 4096 - file->nextRead);
        rest -= (file->initialPointer + 4096 - file->nextRead);
        buff = (void *) (((char * ) buff) + (file->initialPointer + 4096 - file->nextRead));

      }
    }

  }
  if(file->semRead.count <= 0) signal(& (file->semRead));
  return 0;
}

void sys_close(int fd) {

  struct openFile  * file = current()->tc[fd].file;
  current()->tc[fd].fd = -1;

  if (current()->tc[fd].mode=READ_ONLY)   --(file->nreaders);
  if (current()->tc[fd].mode=WRITE_ONLY)  --(file->nwriters);
  --tfa.users[current()->tc[fd].pos];

}
