#ifndef __IO_TIME_H__
#define __IO_TIME_H__

#include <time.h>

typedef struct io_time
{
    int clock_type;

    struct timespec total_start;
    struct timespec total_end;
    struct timespec sys_start;
    struct timespec sys_end;
    struct timespec total_sum;
    struct timespec sys_sum;
    struct timespec us_sum;
} io_time;

int io_time_init(struct io_time *io_time, int clock_type);

int io_time_start_clock_total(struct io_time *io_time);
int io_time_stop_clock_total (struct io_time *io_time);
int io_time_start_clock_sys  (struct io_time *io_time);
int io_time_stop_clock_sys   (struct io_time *io_time);

int io_time_log(struct io_time *io_time);

#endif
