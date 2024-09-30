#ifndef __IF_NAT_H__
#define __IF_NAT_H__

#include "if_ctx.h"

#define DEFAULT_LIBURING_QDEPTH  32
#define DEFAULT_LIBURING_BUFSIZE 2048



int if_nat_io_handler               (CTX *ctx);
int if_nat_iouring_handler          (CTX *ctx);
int if_nat_liburing_handler         (CTX *ctx);
int if_nat_liburing_handler_sqpoll  (CTX *ctx);
int if_nat_liburing_handler_single  (CTX *ctx);

/*
int if_nat_parallel_liburing_handler(CTX *ctx);
*/

void set_cpu_affinity(int cpu);

#endif