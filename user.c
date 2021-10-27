#include <libc.h>

char buff[24];

int pid;
int zeos_ticks;

int add(int p1, int p2)
{
	return p1 + p2;
}

int addASM(int, int);

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  
  /********WRITE********/
  if(write(1, "------- \n", 9) == -1) perror();
  if(write(1, "\nWrite from user.c\n", 19) == -1) perror();
  if(write(1, "------- \n", 9) == -1) perror();
  

  /*****GETTIME********/
  char * time_text;
  time_text = "\nTesting gettime() from user.c: ";
  if(write(1, time_text, strlen(time_text)) == -1) perror();
  itoa(gettime(), time_text); 
  if(write(1, time_text, strlen(time_text)) == -1) perror();
  if(write(1, " (gettime result)\n", 18) == -1) perror();
  if(write(1, "------- \n", 9) == -1) perror();


  while(1) { 
  	//int val1 = add(0x42, 0x666);
  	//int val2 = addASM(0x42, 0x666);
  }
}
