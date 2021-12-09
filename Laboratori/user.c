#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */


  //Test de semaforos inicializar
  char * mesg = "\nTest de los los semaforos:";
  write(1, mesg, strlen(mesg));

  // Invalid semaphore: negative id.
  if (sem_init(-3,2) == 0) write(1, "\nT", 2);
  else write(1, "\nInvalid", strlen("\nInvalid"));

  if (sem_init(3,2) == 0) write(1, "\nT", 2);
  else write(1, "\nInvalid", strlen("\nInvalid"));

  // Invalid semaphore: repeated id.
  if (sem_init(3,2) == 0) write(1, "\nT", 2);
  else write(1, "\nInvalid", strlen("\nInvalid"));

  if (sem_init(1,2) == 0) write(1, "\nT", 2);
  else write(1, "\nInvalid", strlen("\nInvalid"));

  if (sem_init(0,0) == 0) write(1, "\nSemaphore created", strlen("\nSemaphore created"));
  else write(1, "\nInvalid", strlen("\nInvalid"));

  char * msg;
  pid = fork();

  sem_wait(0);
  if(pid > 0){
    write(1, "\nfather says", strlen("\nfather says"));
    msg = "\nhi";
  }
  else{
    write(1, "\nson says", strlen("\nson says"));
    msg = "\nbye";
  }  
  write(1, msg, strlen(msg));
}
