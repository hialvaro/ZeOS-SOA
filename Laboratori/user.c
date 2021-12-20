#include <libc.h>

//char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  /*
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
  if (sem_init(4,3) == 0)   mesg = "\nValid ID 4";
  else mesg = "\nInvalid ID 4: REPEATED";
  write(1, mesg, strlen(mesg));

  mesg = "\n\nTrying to destroy the SEMAPHORE with ID=4";
  write(1, mesg, strlen(mesg));
  if (sem_destroy(4) == 0)   mesg = "\nDestroy OK";
  else mesg = "\nDestroy Invalid";
  write(1, mesg, strlen(mesg));

  mesg = "\n\nTrying to REcreate the SEMAPHORE with ID=4 after destroy";
  write(1, mesg, strlen(mesg));
  if (sem_init(4,1) == 0)   mesg = "\nValid ID 4";
  else mesg = "\nInvalid ID 4";
  write(1, mesg, strlen(mesg));

  mesg = "\n\nsem_wait and sem_signal test with sem_id=4";
  write(1, mesg, strlen(mesg));
  pid = fork();
  if(sem_wait(4) == 0) write(1, "\nWait success", strlen("\nWait success"));
  if(pid > 0){
    write(1, "\nfather says", strlen("\nfather says"));
    mesg = "\nhi";
  }
  else{
    write(1, "\nson says", strlen("\nson says"));
    mesg = "\nbye";
  }
  write(1, mesg, strlen(mesg));
  sem_signal(4);*/

  int pd[2], p;
  p = pipe(pd);
  int finhijo = 0;
  pid = fork();
  if (pid>0) {
    char *a = "\nPorque no funciona, puta vida.";
    write(pd[1], a, 31);
    write(1, "\nsoy el padre.", strlen("\nsoy el padre."));
  }
  else {
    char b[31];
    void *q = b;
    read(pd[0],q, 31);
    write(1, "\nsoy el hijo.", strlen("\nsoy el hijo."));
    write(1,b,31);
    finhijo = 1;
  }
  //while (! finhijo);
  char t[31];
  void *d = t;
  //write(1,"\nVoy a entrar al while.", strlen("\nVoy a entrar al while."));
  //close(pd[0]);

  if (read(pd[0], d, 31) >= 0) write(1, d, 31);

  while(1){}
}
