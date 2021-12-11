#ifndef __TFA_H__
#define __TFA_H__


#define MAX_TFAS 30
#include <list.h>

struct tfa {

  //TTY info
  char * nextRead;
  char * nextWritten;
  int availableWritters;
  int nreaders;
  int nwriters;
  int semaphore;
};

struct tfas_table {

  int users [MAX_TFAS];
  struct tfa tfas [MAX_TFAS];
};

struct tfas_table tfas_table;


//Funciones
void init_tfas_table();

#endif /* TABLA DE FICHEROS ABIERTOS */
