#ifndef _SCHEDPERF_H_
#define _SCHEDPERF_H_
#include <sched.h>

#define RR 0
#define FCFS 1

extern struct list_head readyqueue;
extern struct list_head console_blocked;

extern void (*sched_next)(void);
extern void (*update_process_state) (struct task_struct *, struct list_head *) ;
extern int (*needs_sched)();
extern void (*update_sched_data)();

void sched_next_fcfs();
void update_process_state_fcfs(struct task_struct * t, struct list_head *dest);
int needs_sched_fcfs();
void update_sched_data_fcfs();

void sched_next_rr();
void update_process_state_rr(struct task_struct * t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();

void block_process(struct list_head *block_queue);
void unblock_process(struct task_struct *blocked);

void init_sched_policy();
struct stats * get_task_stats(struct task_struct *t);
struct list_head * get_task_list(struct task_struct *t);

void zeos_console_init();
void zeos_update_read_console_emul();

#endif
