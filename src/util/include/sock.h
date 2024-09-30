#ifndef __SOCK_H__
#define __SOCK_H__

#define MIN_ADDR_LEN 8
#define MAX_ADDR_LEN 16
#define MAX_CIDR_LEN 19

#include "types.h"

int str_to_addr   (char *src, U32BIT *addr);
int str_to_haddr   (char *src, U32BIT *addr);

int str_to_netmask(char *src, U32BIT *netmask);

int cidrstr_to_addr_nmask(char *src, U32BIT *addr, U32BIT *netmask);

int str_to_route(char *src, U32BIT *rt_addr1, U32BIT *rt_addr2, U32BIT *rt_nmask1, U32BIT *rt_nmask2);

char *addr_to_str(U32BIT addr);
char *haddr_to_str(U32BIT addr);

int udp_socket_create();
int udp_socket_bind  (int fd, U16BIT port);

#endif