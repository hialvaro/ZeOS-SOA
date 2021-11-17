#ifndef _SEMAPHORE
#define _SEMAPHORE

#include <list.h>

typedef struct {
		int sem_id;
		unsigned int count;
		int pid_owner;
		struct list_head semqueue;
}Sem;

#endif /* _SEMAPHORE */