#include "io_time.h"
#include "log.h"

int io_time_log(struct io_time *io_time)
{
    log_msg(LOG_INFO,
        "\n============== Statistics ==============\n"
        "User time:\t%ld.%09ld seconds\n"
        "System time:\t%ld.%09ld seconds\n"
        "Total time:\t%ld.%09ld seconds\n",
        io_time->us_sum.tv_sec, io_time->us_sum.tv_nsec,
        io_time->sys_sum.tv_sec, io_time->sys_sum.tv_nsec,
        io_time->total_sum.tv_sec, io_time->total_sum.tv_nsec);

    return 0;
}

int io_time_stop_clock_sys(struct io_time *io_time)
{
    struct timespec tmp_sum;

    clock_gettime(io_time->clock_type, &io_time->sys_end);

    tmp_sum.tv_sec  = io_time->sys_end.tv_sec  - io_time->sys_start.tv_sec;
    tmp_sum.tv_nsec = io_time->sys_end.tv_nsec - io_time->sys_start.tv_nsec;

    if (tmp_sum.tv_nsec < 0)
    {
        tmp_sum.tv_sec--;
        tmp_sum.tv_nsec += 1000000000;
    }

    io_time->sys_sum.tv_sec  += tmp_sum.tv_sec;
    io_time->sys_sum.tv_nsec += tmp_sum.tv_nsec;

    if (io_time->sys_sum.tv_nsec >= 1000000000)
    {
        io_time->sys_sum.tv_sec++;
        io_time->sys_sum.tv_nsec -= 1000000000;
    }

    return 0;
}

int io_time_start_clock_sys(struct io_time *io_time)
{
    clock_gettime(io_time->clock_type, &io_time->sys_start);
    return 0;
}

int io_time_stop_clock_total(struct io_time *io_time)
{
    clock_gettime(io_time->clock_type, &io_time->total_end);

    io_time->total_sum.tv_sec  = io_time->total_end.tv_sec  - io_time->total_start.tv_sec;
    io_time->total_sum.tv_nsec = io_time->total_end.tv_nsec - io_time->total_start.tv_nsec;

    if (io_time->total_sum.tv_nsec < 0)
    {
        io_time->total_sum.tv_sec--;
        io_time->total_sum.tv_nsec += 1000000000;
    }

    io_time->us_sum.tv_sec  = io_time->total_sum.tv_sec  - io_time->sys_sum.tv_sec;
    io_time->us_sum.tv_nsec = io_time->total_sum.tv_nsec - io_time->sys_sum.tv_nsec;

    if (io_time->us_sum.tv_nsec < 0)
    {
        io_time->us_sum.tv_sec--;
        io_time->us_sum.tv_nsec += 1000000000;
    }

    return 0;
}

int io_time_start_clock_total(struct io_time *io_time)
{
    clock_gettime(io_time->clock_type, &io_time->total_start);
    return 0;
}

int io_time_init(struct io_time *io_time, int clock_type)
{
    io_time->clock_type        = clock_type;
    io_time->total_sum.tv_sec  = 0;
    io_time->total_sum.tv_nsec = 0;
    io_time->sys_sum.tv_sec    = 0;
    io_time->sys_sum.tv_nsec   = 0;
    io_time->us_sum.tv_sec     = 0;
    io_time->us_sum.tv_nsec    = 0;
    return 0;
}