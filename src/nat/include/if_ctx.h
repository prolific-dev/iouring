#ifndef __IF_CTX_H__
#define __IF_CTX_H__

#include <sys/epoll.h>

#include "cfg.h"
#include "if_stats.h"

typedef struct CTX
{
    Config   *cfg;
    int       fd1;
    int       fd2;
    int       stats_fd;
    if_stats *stats;
    int       epfd;
    struct epoll_event events[2];
} CTX;

#endif // __IF_CTX_H__