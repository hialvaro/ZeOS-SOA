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
  if(write(1, "\n------- \n", 9) == -1) perror();
  char * mesg;
  mesg = "\n [write] Write from user.c\n";
  if(write(1, mesg, strlen(mesg)) == -1) perror();
  if(write(1, "------- \n", 9) == -1) perror();
  

  /*****GETTIME********/
  mesg = "\n [gettime] Testing gettime() from user.c: ";
  if(write(1, mesg, strlen(mesg)) == -1) perror();
  itoa(gettime(), mesg); 
  if(write(1, mesg, strlen(mesg)) == -1) perror();
  mesg = " (gettime result)\n";
  if(write(1, mesg, strlen(mesg)) == -1) perror();
  mesg = "------- \n\n";
  if(write(1, mesg, strlen(mesg)) == -1) perror();

  /**********GETPID********/
  mesg = "\n [getpid] My PID: ";
  if(write(1, mesg, strlen(mesg)) == -1) perror();
  int pid = getpid();
  itoa(pid, mesg);
  if(write(1, mesg, strlen(mesg)) == -1) perror();
  mesg = "\n------- \n\n";
  if(write(1, mesg, strlen(mesg)) == -1) perror();

  /*********FORK*********
  mesg = "[fork] Forking... \n CHIDL PID: ";
  if(write(1, mesg, strlen(mesg)) == -1) perror();
  int pid_child = fork();
  itoa(pid_child, mesg);
  if(write(1, mesg, strlen(mesg)) == -1) perror();
  mesg = "\n[fork] Job is done. \n ------- \n\n";
  if(write(1, mesg, strlen(mesg)) == -1) perror();*/

  /*****FORK TRUE TEST***/
  int child = fork();
  if(child == 0){
    mesg="I am the CHILD and my PID is ";
    if(write(1, mesg, strlen(mesg)) == -1) perror();
    itoa(getpid(), mesg);
    if(write(1, mesg, strlen(mesg)) == -1) perror();
    mesg="\n";
    if(write(1, mesg, strlen(mesg)) == -1) perror();
    exit();
    mesg="Child exited with PID ";
    if(write(1, mesg, strlen(mesg)) == -1) perror();
    itoa(getpid(), mesg);
    if(write(1, mesg, strlen(mesg)) == -1) perror();
    mesg="\n";
    if(write(1, mesg, strlen(mesg)) == -1) perror();
  }
  else{
    mesg="I am the FATHER and my PID is ";
    if(write(1, mesg, strlen(mesg)) == -1) perror();
    itoa(getpid(), mesg);
    if(write(1, mesg, strlen(mesg)) == -1) perror();
    mesg="\n";
    if(write(1, mesg, strlen(mesg)) == -1) perror();
  }

  while(1) { 
  	//int val1 = add(0x42, 0x666);
  	//int val2 = addASM(0x42, 0x666);
  }
}
