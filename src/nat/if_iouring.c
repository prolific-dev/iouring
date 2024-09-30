#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <sys/mman.h>

#include <linux/io_uring.h>

#include "log.h"
#include "cfg.h"
#include "sock.h"
#include "if_poll.h"
#include "if_ip.h"
#include "if_nat.h"

/* iouring I/O with low level interface */

struct nat_io_sq_ring
{
    unsigned *head;
    unsigned *tail;
    unsigned *ring_mask;
    unsigned *ring_entries;
    unsigned *flags;
    unsigned *array;
};

struct nat_io_cq_ring 
{
    unsigned *head;
    unsigned *tail;
    unsigned *ring_mask;
    unsigned *ring_entries;
    struct io_uring_cqe *cqes;
};

struct submitter
{
    int ring_fd;
    struct nat_io_sq_ring  sq_ring;
    struct io_uring_sqe   *sqes;
    struct nat_io_cq_ring  cq_ring;
};

int io_uring_setup(unsigned entries, struct io_uring_params *p)
{
    return (int) syscall(__NR_io_uring_setup, entries, p);
}

int io_uring_enter(int ring_fd, unsigned int to_submit, unsigned int min_complete, unsigned int flags)
{
    return (int) syscall(__NR_io_uring_enter, ring_fd, to_submit, min_complete, flags, NULL, 0);
}

int nat_setup_uring(struct submitter *s, int qd)
{
    int rc;
    struct nat_io_sq_ring *sring = &s->sq_ring;
    struct nat_io_cq_ring *cring = &s->cq_ring;
    struct io_uring_params p;
    void *sq_ptr, *cq_ptr;

    int sring_sz;
    int cring_sz;

    rc = 1;

    memset(&p, 0, sizeof(p));
    s->ring_fd = io_uring_setup(qd, &p);
    if (s->ring_fd >= 0)
        rc = 0;

    if (rc == 0)
    {
        sring_sz = p.sq_off.array + p.sq_entries * sizeof(unsigned);
        cring_sz = p.cq_off.cqes + p.cq_entries * sizeof(struct io_uring_cqe);

        if (p.features & IORING_FEAT_SINGLE_MMAP)
        {
            if (cring_sz > sring_sz)
                sring_sz = cring_sz;
            cring_sz = sring_sz;
        }

        sq_ptr = mmap(0, sring_sz, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_POPULATE,
                      s->ring_fd, IORING_OFF_SQ_RING);
        if (sq_ptr == MAP_FAILED)
            rc = 1;
    }

    if (rc == 0)
    {
        if (p.features & IORING_FEAT_SINGLE_MMAP)
        {
            cq_ptr = sq_ptr;
        }
        else
        {
            cq_ptr = mmap(0, cring_sz, PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_POPULATE,
                          s->ring_fd, IORING_OFF_CQ_RING);
            if (cq_ptr == MAP_FAILED)
                rc = 1;
        }
    }

    if (rc == 0)
    {
        sring->head         = sq_ptr + p.sq_off.head;
        sring->tail         = sq_ptr + p.sq_off.tail;
        sring->ring_mask    = sq_ptr + p.sq_off.ring_mask;
        sring->ring_entries = sq_ptr + p.sq_off.ring_entries;
        sring->flags        = sq_ptr + p.sq_off.flags;
        sring->array        = sq_ptr + p.sq_off.array;

        s->sqes = mmap(0, p.sq_entries * sizeof(struct io_uring_sqe),
                       PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE,
                       s->ring_fd, IORING_OFF_SQES);
        if (s->sqes == MAP_FAILED)
            rc = 1;
    }

    if (rc == 0)
    {
        cring->head         = cq_ptr + p.cq_off.head;
        cring->tail         = cq_ptr + p.cq_off.tail;
        cring->ring_mask    = cq_ptr + p.cq_off.ring_mask;
        cring->ring_entries = cq_ptr + p.cq_off.ring_entries;
        cring->cqes         = cq_ptr + p.cq_off.cqes;
    }

    return rc;
}

int nat_submit_to_sq(int fd, char *if_name, void *data, int len, struct submitter *s, int op)
{
    int rc;
    unsigned index, tail;

    struct nat_io_sq_ring *sring = &s->sq_ring;
    struct io_uring_sqe   *sqe;

    rc = index = tail = 0;

    sring = &s->sq_ring;
    tail  = *sring->tail;
    index = tail & *s->sq_ring.ring_mask;
    sqe   = &s->sqes[index];

    sqe->fd     = fd;
    sqe->flags  = 0;
    sqe->addr   = (unsigned long) data;
    sqe->opcode = op;
    sqe->len    = len;

    sring->array[index] = index;
    tail++;

    if (*sring->tail != tail)
        *sring->tail = tail;

    rc = io_uring_enter(s->ring_fd, 1, 1, IORING_ENTER_GETEVENTS);
    if (rc < 0)
        log_msg(LOG_ERROR, "io_uring_enter");
    else
        rc = 0;

    return rc;
}

int nat_read_from_cq(struct submitter *s)
{
    int rc;

    struct nat_io_cq_ring *cring = &s->cq_ring;
    struct io_uring_cqe   *cqe;

    unsigned head;

    head = *cring->head;

    if (head == *cring->tail)
        return -1;

    cqe = &cring->cqes[head & (*s->cq_ring.ring_mask)];
    
    rc = cqe->res;
    cqe->res = 0;

    if (rc < 0)
        rc = -1;

    head++;

    if (*cring->head != head)
        *cring->head = head;

    return rc;
}

int if_nat_iouring_handler(CTX *ctx)
{
    int rc;
    int i;
    int fd = ctx->fd1;
    char data[2048];
    struct submitter *s;


    int r_len;
    U32BIT src;
    U32BIT dst;

    IP_HEAD *ip = (IP_HEAD *)data;

    rc = register_epoll_routine(ctx);
    if (rc < 0)
        return rc;

    s  = malloc(sizeof(*s));
    if (s)
    {
        memset(s, 0, sizeof(*s));
        rc = 0;
    }

    if (rc == 0)
        rc = nat_setup_uring(s, ctx->cfg->queue_depth);

    log_msg(LOG_INFO, "Running io_uring... Press Ctrl-C to stop.");
    while (1)
    {
        rc = enter_epoll_routine(ctx);
        if (rc == EPOLL_EXIT)
            break;

        if (rc == EPOLL_ERROR)
            continue;

        if (rc == EPOLL_SUCCESS)
        {
            for (i = 0; i < ctx->cfg->poll_trigger; i++)
            {
                rc = nat_submit_to_sq(fd, ctx->cfg->if_name1, data, sizeof(data), s, IORING_OP_READ);
                rc = nat_read_from_cq(s);
                if (rc >= 0)
                {
                    r_len = rc;
                    rc = addr_config_check(ctx->cfg, ip, &src, &dst);
                }
                
                if (rc == 0)
                {
                    ip_replace_addr(ip, src, dst);
                    rc = nat_submit_to_sq(fd, ctx->cfg->if_name1, data, r_len, s, IORING_OP_WRITE);
                    rc = nat_read_from_cq(s);
                }

                if (rc < 0)
                    break;
            }
        }
    }

    free(s);
    return rc;
}