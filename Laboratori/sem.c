#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <p_stats.h>

#include <errno.h>

#include <sem.h>

#include <tfa.h>

//#include <sys.h>


int init(int sem_id, unsigned int count, struct Semaphore *s) {

  s->sem_id = sem_id;
  s->count = count;
  s->pid_owner = current()->PID;
  INIT_LIST_HEAD(&(s->semqueue));

  return 0;
}

int wait(struct Semaphore *s){

  if (s->count <= 0) {
    list_add_tail(&(current()->list), &(s->semqueue));
    sched_next_rr();
  }
  else --(s->count);

  return 0;
}

int signal(struct Semaphore *s) {

  if (list_empty(&(s->semqueue))) ++(s->count);
  else {
    struct list_head *new = list_first(& (s->semqueue));
    list_del(new);
    struct task_struct * task = list_head_to_task_struct(new);
    update_process_state_rr(task,&readyqueue);

  }
  return 0;
}

int destroy(struct Semaphore *s) {

  if (current()->PID == s->pid_owner) {
    s->sem_id = -1;
    s->count = -1;
    while(!list_empty(&(s->semqueue))) {
      struct list_head * new = list_first(&(s->semqueue));
      list_del(new);
      struct task_struct * task = list_head_to_task_struct(new);
      update_process_state_rr(task, &readyqueue);
    }
  }
  else return -1;
  return 0;
}
