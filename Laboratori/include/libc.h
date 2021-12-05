/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */

#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

extern int errno;

int write(int fd, char *buffer, int size);

void itoa(int a, char *b);

int strlen(char *a);

void perror();

int getpid();

int fork();

void exit();

int yield();

int get_stats(int pid, struct stats *st);

/* Semaphores */
int sem_init(int sem_id, unsigned int count);
int sem_wait(int sem_id);
int sem_signal(int sem_id);
int sem_destroy(int sem_id);

#endif  /* __LIBC_H__ */
