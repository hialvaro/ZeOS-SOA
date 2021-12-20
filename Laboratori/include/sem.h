#ifndef _SEMAPHORE
#define _SEMAPHORE

#include <list.h>

struct Semaphore {
		int sem_id;
		unsigned int count;
		int pid_owner;
		struct list_head semqueue;
};

int init(int sem_id, unsigned int count, struct Semaphore *s);
int wait(struct Semaphore *s);
int signal(struct Semaphore *s);
int destroy(struct Semaphore *s);

#endif /* _SEMAPHORE */
