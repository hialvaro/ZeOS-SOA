#ifndef __TFA_H__
#define __TFA_H__


#define MAX_TFAS 30
#include <list.h>
#include <sem.h>

struct openFile {

  //TTY info
  char * nextRead;
  char * nextWritten;
  int availablebytes;
  int nreaders;
  int nwriters;
  struct Semaphore semRead;
  struct Semaphore semWrite;
  int frame;
  char * initialPointer;
};

struct openFileTable {

  int users [MAX_TFAS];
  struct openFile tfas [MAX_TFAS];
};

int ini_fa(int n, char * iniP, int frame);


#endif /* TABLA DE FICHEROS ABIERTOS */
