#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>

#include <sched.h>

#include <liburing.h>

#include "log.h"
#include "sock.h"
#include "cfg.h"
#include "if_ip.h"
#include "if_poll.h"
#include "if_stats.h"
#include "if_nat.h"


/* Buffer Flags */
#define BUFFER_FREE  0
#define BUFFER_READ  1
#define BUFFER_WRITE 2

/* CPU Affinity */
#define IOWQ_CPU 0
#define SQPOLL_CPU 1

sig_atomic_t exit_liburing_handler = 0;

typedef struct buf_info_t {
    int   idx;
    int   state;
} buf_info_t;

void signal_handler(int signo)
{
    if (signo == SIGINT)
        exit_liburing_handler = 1;
    if (signo == SIGTERM)
        exit_liburing_handler = 1;
}

/* iouring I/O with liburing */

int if_nat_liburing_setup(CTX *ctx, struct io_uring *ring, struct io_uring_params *params, struct iovec *iovecs)
{
    int rc;

    set_cpu_affinity(ctx->cfg->cpu_affinity);

    if (params)
        rc = io_uring_queue_init_params(ctx->cfg->queue_depth, ring, params);
    else
        rc = io_uring_queue_init(ctx->cfg->queue_depth, ring, 0);

    if (rc != 0)
    {
        log_msg(LOG_ERROR, "io_uring_queue_init rc=%d", rc);
        return rc;
    }

    rc = io_uring_register_buffers(ring, iovecs, ctx->cfg->queue_depth);
    if (rc != 0)
    {
        log_msg(LOG_ERROR, "io_uring_buf_register rc=%d", rc);
        return rc;
    }

    rc = io_uring_register_files(ring, &ctx->fd1, 1);
    if (rc != 0)
    {
        log_msg(LOG_ERROR, "io_uring_register_files rc=%d", rc);
        return rc;
    }

    rc = register_epoll_routine(ctx);
    if (rc != 0)
    {
        log_msg(LOG_ERROR, "register_poll_routine failed");
        return rc;
    }

    rc = register_epoll_routine(ctx);
    if (rc != 0)
    {
        log_msg(LOG_ERROR, "register_poll_routine failed");
        return rc;
    }

    return rc;
}

int if_nat_liburing_clean(struct io_uring *ring)
{
    io_uring_unregister_buffers(ring);
    io_uring_unregister_files(ring);
    io_uring_queue_exit(ring);

    return 0;
}

int if_nat_liburing_handler(CTX *ctx)
{
    int       rc       = 0;
    int       i        = 0;
    int       fd       = ctx->fd1;
    int       qd       = ctx->cfg->queue_depth;
    if_stats *stats    = ctx->stats;
    int       buf_size = ctx->cfg->buffer_size;
    int       no_reads = ctx->cfg->poll_trigger;
    U32BIT    src      = 0;
    U32BIT    dst      = 0;
    int       r_len    = 0;
    unsigned  head     = 0;
    int       cqe_cnt  = 0;

    struct io_uring         ring;
    struct io_uring_params  params; 
    struct io_uring_sqe    *sqe;
    struct io_uring_cqe    *cqe;
    struct iovec            iovecs[qd];

    void                   *buffers;
    void                   *buf;
    int                     buf_free[qd];
    unsigned int            len;
    unsigned long           user_data;
    unsigned int            idx;

    int fds[1];
    fds[0] = fd;

    set_cpu_affinity(ctx->cfg->cpu_affinity);

    memset(&params, 0, sizeof(params));

    params.flags    |= IORING_SETUP_SINGLE_ISSUER;
    params.flags    |= IORING_SETUP_SUBMIT_ALL;

    rc = io_uring_queue_init_params(qd, &ring, &params);
    if (rc < 0)
    {
        log_msg(LOG_ERROR, "io_uring_queue_init_params failed: %s", strerror(-rc));
        return rc;
    }
    /*
    if (ctx->cfg->use_uring == 3)
    {
    }
    else
    {
        rc = io_uring_queue_init(qd, &ring, 0);
        if (rc < 0)
        {
            log_msg(LOG_ERROR, "io_uring_queue_init failed: %s", strerror(-rc));
            return rc;
        }
    }
    */

    rc = io_uring_register_files(&ring, fds, 1);
    if (rc < 0)
    {
        log_msg(LOG_ERROR, "io_uring_register_files failed: %s", strerror(-rc));
        return rc;
    }

    posix_memalign(&buffers, buf_size, qd * buf_size);
    if (buffers == NULL)
    {
        log_msg(LOG_ERROR, "malloc buffer failed");
        return -1;
    }

    for (i = 0; i < qd; i++)
    {
        iovecs[i].iov_base = buffers + i * buf_size;
        iovecs[i].iov_len  = buf_size;

        buf_free[i] = BUFFER_FREE;
    }

    rc = io_uring_register_buffers(&ring, iovecs, qd);
    if (rc < 0)
    {
        log_msg(LOG_ERROR, "io_uring_register_buffers failed: %s", strerror(-rc));
        return rc;
    }

    rc = register_epoll_routine(ctx);
    if (rc < 0)
    {
        log_msg(LOG_ERROR, "register_epoll_routine failed: %s", strerror(-rc));
        return rc;
    }

    if (ctx->cfg->use_uring == 3)
        log_msg(LOG_INFO, "Running liburing advanced... Press Ctrl-C to stop.");
    else
        log_msg(LOG_INFO, "Running liburing... Press Ctrl-C to stop.");

    log_msg(LOG_INFO, "Running liburing advanced... Press Ctrl-C to stop.");
    while (fd)
    {
        if (no_reads >= ctx->cfg->poll_trigger)
        {
            rc = enter_epoll_routine(ctx);
            if (rc == EPOLL_EXIT)
                break;

            if (rc == EPOLL_ERROR)
                continue;
            
            if (rc == EPOLL_SUCCESS)
                no_reads = 0;
        }
        
        for (i = 0; i < qd; i++)
        {
            if (buf_free[i] == BUFFER_FREE)
            {
                buf = iovecs[i].iov_base;
                len = iovecs[i].iov_len;

                sqe         = io_uring_get_sqe(&ring);
                buf_free[i] = BUFFER_READ;

                io_uring_prep_read_fixed(sqe, 0, buf, len, 0, i);
                io_uring_sqe_set_data(sqe, (void *)(intptr_t)i);
                io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);

                if (stats->capture)
                    stats->reads_cnt++;
            }
        }

        rc = io_uring_submit(&ring);
        if (rc < 0)
            break;

        if (rc == 0)
            no_reads++;

        if (rc > 0 && stats->capture)
            stats->syscall_cnt++;

        while ((rc = io_uring_peek_cqe(&ring, &cqe)) == 0)
        {
            rc        = cqe->res;
            user_data = (unsigned long) io_uring_cqe_get_data(cqe);
            idx       = (unsigned int) user_data;

            io_uring_cq_advance(&ring, 1);

            /* If cqe result failed then reset used buffer */
            if (rc <= 0)
            {
                if (idx >= 0 && idx < qd)
                    buf_free[idx] = BUFFER_FREE;

                log_msg(LOG_ERROR, "cqe->res=%d errno: %s", rc, strerror(-rc));
                continue;
            }

            /* Check if previous op was write */
            if (buf_free[idx] == BUFFER_WRITE)
            {
                if (stats->capture)
                    stats->writes_success++;

                buf_free[idx] = BUFFER_FREE;
                continue;
            }

            r_len = rc;

            /* Check if previous op was read */
            if (buf_free[idx] == BUFFER_READ)
            {
                if (stats->capture)
                    stats->reads_success++;

                src = 0;
                dst = 0;

                rc = addr_config_check(ctx->cfg, (IP_HEAD *)iovecs[idx].iov_base, &src, &dst);
                if (rc != 0)
                {
                    buf_free[idx] = BUFFER_FREE;
                    continue;
                }

                ip_replace_addr((IP_HEAD *)iovecs[idx].iov_base, src, dst);

                buf_free[idx] = BUFFER_WRITE;
                sqe           = io_uring_get_sqe(&ring);

                if (!sqe)
                    continue;

                io_uring_prep_write_fixed(sqe, 0, iovecs[idx].iov_base, r_len, 0, idx);
                io_uring_sqe_set_data(sqe, (void *)(intptr_t)idx);
                io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);

                if (stats->capture)
                    stats->writes_cnt++;
            }
        }

        rc = io_uring_submit(&ring);
        if (rc < 0)
        {
            log_msg(LOG_ERROR,
                    "4: io_uring_submit write rc=%d, buffer %u, errno:%s",
                    rc, idx, strerror(-rc));
            break;
        }

        if (rc > 0 && stats->capture)
            stats->syscall_cnt++;
    }

    rc = if_nat_liburing_clean(&ring);

    free(buffers);

    return rc;
}

int if_nat_liburing_handler_sqpoll(CTX *ctx) {
    int rc;
    int i;

    U32BIT src;
    U32BIT dst;

    int queue_depth = ctx->cfg->queue_depth;
    int buffer_size = ctx->cfg->buffer_size;
    int poll_trigger = ctx->cfg->poll_trigger;

    unsigned head;

    ctx->cfg->poll_timeout = 1000;
    poll_trigger = poll_trigger;

    int no_cqes = poll_trigger;
    int nr_cqes;

    int fds[2];

    struct io_uring ring;
    struct io_uring_params params;

    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;

    void *buffers;
    struct iovec iovecs[queue_depth];
    int buf_free[queue_depth];

    int idx;
    int r_len;


    struct __kernel_timespec ts;

    memset(&ts, 0, sizeof(ts));

    ts.tv_sec = 1;

/*
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
*/

    memset(&params, 0, sizeof(params));

    params.flags |= IORING_SETUP_SINGLE_ISSUER;
    params.flags |= IORING_SETUP_SUBMIT_ALL;

    /*
    params.flags |= IORING_SETUP_SQPOLL;
    params.sq_thread_idle = 5000;
    */

    /*
    params.features |= IORING_FEAT_FAST_POLL;
    params.features |= IORING_FEAT_NODROP;
    */

    rc = io_uring_queue_init_params(queue_depth, &ring, &params);
    if (rc < 0) {
        log_msg(LOG_ERROR, "io_uring_queue_init_params failed: %s", strerror(-rc));
        return rc;
    }

    fds[0] = ctx->fd1;
    rc = io_uring_register_files(&ring, fds, 1);
    if (rc < 0) {
        log_msg(LOG_ERROR, "io_uring_register_files failed: %s", strerror(-rc));
        return rc;
    }

    rc = posix_memalign(&buffers, buffer_size, queue_depth * buffer_size);
    if (rc < 0) {
        log_msg(LOG_ERROR, "posix_memalign failed: %s", strerror(-rc));
        return rc;
    }

    memset(buffers, 0, queue_depth * buffer_size);

    for (i = 0; i < queue_depth; i++) 
    {
        iovecs[i].iov_base = buffers + i * buffer_size;
        iovecs[i].iov_len  = buffer_size;
        buf_free[i] = BUFFER_FREE;
    }

    rc = io_uring_register_buffers(&ring, iovecs, queue_depth);
    if (rc < 0) {
        log_msg(LOG_ERROR, "io_uring_register_buffers failed: %s", strerror(-rc));
        return rc;
    }

    rc = register_epoll_routine(ctx);
    if (rc < 0) {
        log_msg(LOG_ERROR, "register_epoll_routine failed: %s", strerror(-rc));
        return rc;
    }

    log_msg(LOG_INFO, "Running liburing SQ_POLL... Press Ctrl-C to stop.");
    while (exit_liburing_handler == 0)
    {
        if (no_cqes >= poll_trigger)
        {
            rc = enter_epoll_routine(ctx);
            if (rc == EPOLL_EXIT)
                break;

            if (rc == EPOLL_ERROR)
                no_cqes = 0;
            
            if (rc == EPOLL_SUCCESS)
                no_cqes = 0;
        }

        for (i = 0; i < queue_depth; i++)
        {
            if (buf_free[i] == BUFFER_FREE)
            {
                sqe = io_uring_get_sqe(&ring);
                if (!sqe)
                    continue;

                io_uring_prep_read_fixed(sqe, 0, iovecs[i].iov_base, buffer_size, 0, i);
                io_uring_sqe_set_data(sqe, (void *)(intptr_t)i);
                io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);
                buf_free[i] = BUFFER_READ;

                if (ctx->stats->capture)
                    ctx->stats->reads_cnt++;

                continue;
            }

            if (buf_free[i] == BUFFER_WRITE)
            {
                sqe = io_uring_get_sqe(&ring);
                if (!sqe)
                    continue;

                if (addr_config_check(ctx->cfg, (IP_HEAD *)iovecs[i].iov_base, &src, &dst) != 0)
                    continue;
                ip_replace_addr((IP_HEAD *)iovecs[i].iov_base, src, dst);

                io_uring_prep_write_fixed(sqe, 0, iovecs[i].iov_base, buffer_size, 0, i);
                io_uring_sqe_set_data(sqe, (void *)(intptr_t)i);
                io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);

                if (ctx->stats->capture)
                    ctx->stats->writes_cnt++;
            }
        }

        rc = io_uring_submit(&ring);
        if (rc > 0 && ctx->stats->capture)
            ctx->stats->syscall_cnt++;

        /*
        while ((rc = io_uring_wait_cqe_timeout(&ring, &cqe, &ts)) == 0)
        while ((rc = io_uring_peek_cqe(&ring, &cqe)) == 0)
        while ((rc = io_uring_wait_cqes(&ring, &cqe, 1, &ts, NULL)))
        */
        io_uring_for_each_cqe(&ring, head, cqe)
        {
            no_cqes= 0;
            nr_cqes++;

            if (cqe->res <= 0)
            {
                log_msg(LOG_ERROR, "io_uring_wait_cqe failed: %s", strerror(-cqe->res));
                continue;
            }

            idx = (int)(intptr_t)io_uring_cqe_get_data(cqe);

            io_uring_cqe_seen(&ring, cqe);

            if (buf_free[idx] == BUFFER_READ)
            {
                buf_free[idx] = BUFFER_WRITE;

                if (ctx->stats->capture)
                    ctx->stats->reads_success++;

                continue;
            }

            if (buf_free[idx] == BUFFER_WRITE)
            {
                buf_free[idx] = BUFFER_FREE;

                if (ctx->stats->capture)
                    ctx->stats->writes_success++;

                continue;
            }
        }

        if (nr_cqes == 0)
            no_cqes++;
        nr_cqes = 0;

    }

    free(buffers);
    io_uring_unregister_buffers(&ring);
    io_uring_unregister_files(&ring);
    io_uring_queue_exit(&ring);
    
    return rc;
}

static void substract_time(struct timespec *start, struct timespec *end, struct timespec *delta)
{
    delta->tv_sec  = end->tv_sec - start->tv_sec;
    delta->tv_nsec = end->tv_nsec - start->tv_nsec;

    if (delta->tv_nsec < 0)
    {
        delta->tv_sec--;
        delta->tv_nsec += 1000000000;
    }
}

static void add_time(struct timespec *start, struct timespec *end, struct timespec *delta)
{
    delta->tv_sec  = end->tv_sec + start->tv_sec;
    delta->tv_nsec = end->tv_nsec + start->tv_nsec;

    if (delta->tv_nsec >= 1000000000)
    {
        delta->tv_sec++;
        delta->tv_nsec -= 1000000000;
    }
}

int if_nat_liburing_handler_single(CTX *ctx)
{
    int rc, i, fd, no_cqes, nr_cqes, idx, r_len, queue_depth, buffer_size, poll_trigger;
    U32BIT src, dst;

    int fds[1];

    void *buffers;

    struct iovec iovecs[ctx->cfg->queue_depth];
    struct buf_info_t buf_info[ctx->cfg->queue_depth];

    buf_info_t *buf_info_ptr;

    struct io_uring ring;
    struct io_uring_params params;

    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;

    struct timespec         ts_start;
    struct timespec         ts_end;
    struct timespec         ts_delta;
    struct timespec         ts_read;
    struct timespec         ts_write;

    queue_depth = ctx->cfg->queue_depth;
    buffer_size = ctx->cfg->buffer_size;
    poll_trigger = ctx->cfg->poll_trigger;
    no_cqes = poll_trigger;
    fd = ctx->fd1;

    fds[0] = fd;

    memset(&ts_start, 0, sizeof(ts_start));
    memset(&ts_end, 0, sizeof(ts_end));
    memset(&ts_delta, 0, sizeof(ts_delta));
    memset(&ts_read, 0, sizeof(ts_read));
    memset(&ts_write, 0, sizeof(ts_write));

    set_cpu_affinity(0);

    rc = io_uring_queue_init(queue_depth, &ring, 0);
    if (rc < 0)
    {
        log_msg(LOG_ERROR, "io_uring_queue_init failed: %s", strerror(-rc));
        return rc;
    }

    rc = io_uring_register_files(&ring, fds, 1);
    if (rc < 0)
    {
        log_msg(LOG_ERROR, "io_uring_register_files failed: %s", strerror(-rc));
        return rc;
    }

    posix_memalign(&buffers, buffer_size, queue_depth * buffer_size);
    if (buffers == NULL)
    {
        log_msg(LOG_ERROR, "posix_memalign failed");
        return -1;
    }

    for (i = 0; i < queue_depth; i++)
    {
        iovecs[i].iov_base = buffers + i * buffer_size;
        iovecs[i].iov_len  = buffer_size;

        buf_info[i].idx   = i;
        buf_info[i].state = BUFFER_FREE;
    }

    rc = io_uring_register_buffers(&ring, iovecs, queue_depth);
    if (rc < 0)
    {
        log_msg(LOG_ERROR, "io_uring_register_buffers failed: %s", strerror(-rc));
        return rc;
    }

    rc = register_epoll_routine(ctx);
    if (rc < 0)
    {
        log_msg(LOG_ERROR, "register_epoll_routine failed: %s", strerror(-rc));
        return rc;
    }

    buf_info_ptr = &buf_info[0];

    log_msg(LOG_INFO, "Running liburing single... Press Ctrl-C to stop.");
    while (1)
    {
        log_msg(LOG_INFO, "Read: %ld.%09ld, Write: %ld.%09ld",
            ts_read.tv_sec, ts_read.tv_nsec,
            ts_write.tv_sec, ts_write.tv_nsec);

        if (no_cqes >= poll_trigger)
        {
            rc = enter_epoll_routine(ctx);
            if (rc == EPOLL_EXIT)
                break;

            if (rc == EPOLL_SUCCESS || rc == EPOLL_ERROR)
                no_cqes = 0;
        }

        for (i = 0; i < poll_trigger; i++)
        {

            if (buf_info_ptr->state == BUFFER_FREE)
            {
                sqe = io_uring_get_sqe(&ring);
                if (!sqe)
                    continue;
                io_uring_prep_read_fixed(sqe, 0, iovecs[0].iov_base, buffer_size, 0, 0);
                io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);

                buf_info_ptr->state = BUFFER_READ;

                if (ctx->stats->capture)
                {
                    ctx->stats->reads_cnt++;
                    ctx->stats->syscall_cnt++;
                }
            }

            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts_start);
            rc =io_uring_submit_and_wait(&ring, 1);
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts_end);
            if (rc < 0)
                break;

            substract_time(&ts_start, &ts_end, &ts_delta);
            add_time(&ts_read, &ts_delta, &ts_read);

            rc = io_uring_wait_cqe(&ring, &cqe);
            if (rc < 0)
            {
                no_cqes++;
                continue;
            }

            if (cqe->res <= 0)
                break;

            if (ctx->stats->capture)
                ctx->stats->reads_success++;

            r_len = cqe->res;
            io_uring_cq_advance(&ring, 1);

            src = 0;
            dst = 0;

            if (addr_config_check(ctx->cfg, (IP_HEAD *)iovecs[0].iov_base, &src, &dst) != 0)
                break;

            ip_replace_addr((IP_HEAD *)iovecs[0].iov_base, src, dst);

            sqe = io_uring_get_sqe(&ring);
            if (!sqe)
                break;

            io_uring_prep_write_fixed(sqe, 0, iovecs[0].iov_base, r_len, 0, 0);
            io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);

            if (ctx->stats->capture)
            {
                ctx->stats->writes_cnt++;
                ctx->stats->syscall_cnt++;
            }

            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts_start);
            rc = io_uring_submit_and_wait(&ring, 1);
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts_end);

            substract_time(&ts_start, &ts_end, &ts_delta);
            add_time(&ts_write, &ts_delta, &ts_write);

            if (rc < 0)
                break;

            rc = io_uring_wait_cqe(&ring, &cqe);
            if (rc < 0)
                break;

            buf_info_ptr->state = BUFFER_FREE;

            if (cqe->res <= 0)
                break;

            io_uring_cq_advance(&ring, 1);
            if (ctx->stats->capture)
                ctx->stats->writes_success++;
        }
    }

    free(buffers);
    io_uring_unregister_buffers(&ring);
    io_uring_unregister_files(&ring);
    io_uring_queue_exit(&ring);

    return rc;
}

/*
int if_nat_parallel_liburing_handler(CTX *ctx)
{
    pid_t pid1, pid2;

    pid1 = fork();
    if (pid1 == 0)
    {
        if_nat_liburing_handler(ctx);
        exit(0);
    }
    else if (pid1 < 0)
    {
        log_msg(LOG_ERROR, "Child 1 fork failed");
        return -1;
    }

    pid2 = fork();
    if (pid2 == 0)
    {
        ctx->fd1               = ctx->fd2;
        ctx->cfg->if_addr1     = ctx->cfg->if_addr2;
        ctx->cfg->if_nmask1    = ctx->cfg->if_nmask2;
        ctx->cfg->cpu_affinity = ctx->cfg->cpu_affinity + 1;

        memset(ctx->cfg->if_name1, 0, sizeof(ctx->cfg->if_name1));
        strcpy(ctx->cfg->if_name1, ctx->cfg->if_name2);

        if_nat_liburing_handler(ctx);

        exit(0);
    }
    else if (pid2 < 0)
    {
        log_msg(LOG_ERROR, "Child 2 fork failed");
        return -1;
    }

    while (!exit_liburing_handler)
        sleep(1);

    if (kill(pid1, SIGINT) == -1)
        log_msg(LOG_ERROR, "Sending SIGINT to child %d failed", pid1);

    if (kill(pid2, SIGINT) == -1)
        log_msg(LOG_ERROR, "Sending SIGINT to child %d failed", pid1);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}
*/
