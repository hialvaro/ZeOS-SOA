#include <types.h>
#include <hardware.h>
#include <segment.h>
#include <sched.h>
#include <mm.h>
#include <io.h>
#include <utils.h>
#include <p_stats.h>
#include <sem.h>
#include <tfa.h>

//Inicializacion de la tabla de ficheros abiertos.
int init_tfas_table() {
  for(int i = 0; i < MAX_TFAS; ++i)
    tfa.users[i] = 0;

  printk("\n Tabla de ficheros abiertos inicializada. :D \n");
  return 0;
}

int ini_fa(int n, char * iniP, int frame) {

  tfa.users[n] = 2;
  tfa.tfas[n].nextWritten = iniP;
  tfa.tfas[n].nextRead = iniP;
  tfa.tfas[n].availablebytes = 4096;
  tfa.tfas[n].nreaders = 1;
  tfa.tfas[n].nwriters = 1;

  init(0, 0, &tfa.tfas[n].semRead);          // TODAVIA NO ESTOY SEGURO DE COMO UTILIZAR LAS PIPES
  init(1, 0, &tfa.tfas[n].semWrite);
  tfa.tfas[n].frame = frame;
  tfa.tfas[n].initialPointer = iniP;

  return 0;
}
