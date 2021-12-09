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
  mesg = "\n\nTrying to create a SEMAPHORE with ID=-3";
  write(1, mesg, strlen(mesg));

  if (sem_init(-3,2) == 0)   mesg = "\nValid ID -3";
  else mesg = "\nInvalid ID -3";
  write(1, mesg, strlen(mesg));

  mesg = "\n\nTrying to destroy a SEMAPHORE with ID=-3";
  write(1, mesg, strlen(mesg));
  if (sem_destroy(-3) == 0)   mesg = "\nDestroy OK";
  else mesg = "\nDestroy Invalid";
  write(1, mesg, strlen(mesg));

  mesg = "\n\nTrying to create a SEMAPHORE with ID=4";
  write(1, mesg, strlen(mesg));
  if (sem_init(4,2) == 0)   mesg = "\nValid ID 4";
  else mesg = "\nInvalid ID 4";
  write(1, mesg, strlen(mesg));

  mesg = "\n\nTrying to REcreate a SEMAPHORE with ID=4";
  write(1, mesg, strlen(mesg));
  if (sem_init(4,2) == 0)   mesg = "\nValid ID 4";
  else mesg = "\nInvalid ID 4: REPEATED";
  write(1, mesg, strlen(mesg));

  mesg = "\n\nTrying to destroy the SEMAPHORE with ID=4";
  write(1, mesg, strlen(mesg));
  if (sem_destroy(4) == 0)   mesg = "\nDestroy OK";
  else mesg = "\nDestroy Invalid";
  write(1, mesg, strlen(mesg));

  mesg = "\n\nTrying to REcreate the SEMAPHORE with ID=4 after destroy";
  write(1, mesg, strlen(mesg));
  if (sem_init(4,2) == 0)   mesg = "\nValid ID 4";
  else mesg = "\nInvalid ID 4";
  write(1, mesg, strlen(mesg));

  mesg = "\n\nCreating a SEMAPHORE with ID=5 to test";
  write(1, mesg, strlen(mesg));
  if (sem_init(5,1) == 0)   mesg = "\nValid ID 5";
  else mesg = "\nInvalid ID 5";
  write(1, mesg, strlen(mesg));

  mesg = "\n\nsem_wait and sem_signal test";
  write(1, mesg, strlen(mesg));
  pid = fork();
  if(sem_wait(5) == 0) write(1, "\nWait success", strlen("\nWait success"));
  if(pid > 0){
    write(1, "\nfather says", strlen("\nfather says"));
    mesg = "\nhi";
  }
  else{
    write(1, "\nson says", strlen("\nson says"));
    mesg = "\nbye";
  }  
  write(1, mesg, strlen(mesg));
  sem_signal(5);
  while(1){}
}
