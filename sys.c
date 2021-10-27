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

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
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