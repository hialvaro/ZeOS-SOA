#ifndef STATS_H
#define STATS_H

/* Structure used by 'get_stats' function */
struct stats
{
  unsigned long user_ticks;
  unsigned long system_ticks;
  unsigned long blocked_ticks;
  unsigned long ready_ticks;
  unsigned long elapsed_total_ticks;
  unsigned long total_trans; /* Number of times the process has got the CPU: READY->RUN transitions */
  unsigned long remaining_ticks;
};
#endif /* !STATS_H */
