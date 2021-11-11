#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  int retPid1=fork(200);
  if(retPid1 ==0){
    char c[10];
    itoa(getpid(),&c);
     write(1,"PID del proceso:",sizeof("PID del proceso:"));
    write(1,c,sizeof(c));
    write(1,"\n",1);
    int retPid2=fork(200);
     if(retPid2 ==0){
    char c[10];
    itoa(getpid(),&c);
     write(1,"PID del proceso:",sizeof("PID del proceso:"));
    write(1,c,sizeof(c));
    write(1,"\n",1);
  }
  }
  if(retPid1 !=0){
    char c[10];
    itoa(getpid(),&c);
     write(1,"PID del proceso padre:",sizeof("PID del proceso padre:"));
    write(1,c,sizeof(c));
    write(1,"\n",1);
  }
  while(1) { }
}
