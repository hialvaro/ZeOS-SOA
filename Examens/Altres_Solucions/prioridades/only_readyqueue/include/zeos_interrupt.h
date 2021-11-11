#include <sched.h>

void zeos_show_clock();

void divide_error_routine();
void debug_routine();
void nm1_routine();
void breakpoint_routine();
void overflow_routine();
void bounds_check_routine();
void invalid_opcode_routine();
void device_not_available_routine();
void double_fault_routine();
void coprocessor_segment_overrun_routine();
void invalid_tss_routine();
void segment_not_present_routine();
void stack_exception_routine();
void general_protection_routine();
void page_fault_routine();
void intel_reserved_routine();
void floating_point_error_routine();
void alignment_check_routine();

void set_handlers();
