#include <utils.h>

void update_stats(unsigned long *v, unsigned long *elapsed)
{
  unsigned long current_ticks;
  
  current_ticks=get_ticks();
  
  *v += current_ticks - *elapsed;
  
  *elapsed=current_ticks;
  
}
