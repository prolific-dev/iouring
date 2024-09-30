#ifndef __IF_POLL_H__
#define __IF_POLL_H__

#include "if_ctx.h"

#define DEFAULT_POLL_TRIGGER 1000
#define DEFAULT_POLL_TIMEOUT 1000

#define EPOLL_SUCCESS  0
#define EPOLL_ERROR    1
#define EPOLL_EXIT     2

int register_epoll_routine(CTX *ctx);
int enter_epoll_routine   (CTX *ctx);

#endif // __IF_POLL_H__