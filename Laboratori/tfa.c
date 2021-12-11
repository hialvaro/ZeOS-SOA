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
void init_tfas_table() {
  for(int i = 0; i < MAX_TFAS; ++i){
    tfas_table.users[i] = 0;
  }
  printk("\n Tabla de ficheros abiertos inicializada. :D \n");
}
