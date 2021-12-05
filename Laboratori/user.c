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

  if (sem_init(-3,2) == 0) write(1, "\nT", 2);
  else write(1, "\nF", 2);

  if (sem_init(3,2) == 0) write(1, "\nT", 2);
  else write(1, "\nF", 2);

  if (sem_init(3,2) == 0) write(1, "\nT", 2);
  else write(1, "\nF", 2);

  if (sem_init(1,2) == 0) write(1, "\nT", 2);
  else write(1, "\nF", 2);



  write(1, "\nEntro el while", 15);
  while(1) { }
}
