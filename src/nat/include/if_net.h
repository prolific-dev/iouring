#ifndef __IF_NET_H__
#define __IF_NET_H__

#include "types.h"
#include "if_ctx.h"

#define MIN_IFNAME_LEN 4
#define MAX_IFNAME_LEN IFNAMSIZ

#define DEFAULT_TXQLEN 1000

int if_net_open (char *if_name, U32BIT addr, U32BIT netmask, S16BIT flags, int txqlen);
int if_net_close(int fd, char* if_name);
int if_net_route(int fd, char *if_name, U32BIT addr, U32BIT netmask);

#endif
