#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/route.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <bits/ioctls.h>

#include <netinet/in.h>

#include <linux/if.h>
#include <linux/if_tun.h>

#include "log.h"
#include "sock.h"
#include "if_net.h"
#include "if_ip.h"

/* TUN path */
#define DEV_MPX "/dev/net/tun"

int if_net_open(char *if_name, U32BIT addr, U32BIT netmask, S16BIT flags, int txqlen)
{
    int     rc;
    int     fd;
    int     sock;

    struct ifreq        ifr;
    struct sockaddr_in *sin;

    rc = sock = fd = -1;

    log_msg(LOG_DEBUG, "Open network interface");

    if (if_name         != NULL &&
        strlen(if_name)  > 0    &&
        addr             > 0    &&
        netmask          > 0)
    {
        rc = 0;
    }

    if (rc == 0)
    {
        fd = open(DEV_MPX, O_RDWR);
        if (fd < 0)
            rc = -1;
    }

    if (rc == 0)
    {
        ifr.ifr_flags = IFF_TUN|IFF_NO_PI;
        strcpy(ifr.ifr_name, if_name);
        rc = ioctl(fd, TUNSETIFF, (void *)&ifr);
    }

    if (rc == 0)
    {
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (sock < 0)
            rc = -1;
    }                

    if (rc == 0)
    {
        /* Set address */
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_ifrn.ifrn_name, if_name);

        sin = (struct sockaddr_in *)&ifr.ifr_ifru.ifru_addr;
        memset(sin, 0, sizeof(*sin));

        sin->sin_family      = AF_INET;
        sin->sin_addr.s_addr = addr;

        rc = ioctl(sock, SIOCSIFADDR, (void *)&ifr);
    }
        
    if (rc == 0)
    {
        /* set netmask */
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_ifrn.ifrn_name, if_name);

        sin = (struct sockaddr_in *)&ifr.ifr_ifru.ifru_netmask;
        memset(sin, 0, sizeof(*sin));

        sin->sin_family      = AF_INET;
        sin->sin_addr.s_addr = netmask;

        rc = ioctl(sock, SIOCSIFNETMASK, (void *)&ifr);
    }

    if (rc == 0)
    {
        /* set flags */
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_ifrn.ifrn_name, if_name);

        ifr.ifr_ifru.ifru_flags = flags;

        rc = ioctl(sock, SIOCSIFFLAGS, (void *)&ifr);
    }

    if (rc == 0)
    {
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_ifrn.ifrn_name, if_name);

        ifr.ifr_ifru.ifru_ivalue = txqlen;

        rc = ioctl(sock, SIOCSIFTXQLEN, (void *)&ifr);
    }

    if (sock >= 0)
        close(sock);

    if (rc < 0)
    {
        if (fd >= 0)
            close(fd);
        fd = -1;
    }

    return fd;
}

int if_net_close(int fd, char* if_name)
{
    log_msg(LOG_DEBUG, "\nClosing network interface");
    
    if (fd >= 0)
        close(fd);
    
    return 0;
}

int if_net_route(int fd, char *if_name, U32BIT addr, U32BIT netmask)
{
    int    rc       =  0;
    int    sock     = -1;
    U32BIT hostmask =  0;

    struct rtentry rt;

    if (fd < 0 || !if_name || strlen(if_name) == 0 || addr == 0 || netmask == 0)
        rc = 1;
    
    if (rc == 0)
        rc = str_to_addr("255.255.255.255", &hostmask);
    
    if (rc == 0)
    {
        sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (sock < 0)
            rc = 1;
    }

    if (rc == 0)
    {
        struct sockaddr_in *sin;

        /* Clear rtentry */
        memset(&rt, 0, sizeof(rt));

        /* set addr */
        sin = (struct sockaddr_in *)&rt.rt_dst;
        memset(sin, 0, sizeof(*sin));
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = addr;

        /* set netmask */
        sin = (struct sockaddr_in *)&rt.rt_genmask;
        memset(sin, 0, sizeof(*sin));
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = netmask;

        /* set flags */
        rt.rt_flags = RTF_UP | RTF_STATIC;
        if (netmask == hostmask)
            rt.rt_flags |= RTF_HOST;
        rt.rt_metric  = 1;
        rt.rt_dev     = if_name;

        /* WHY? */
        errno = 0;

        rc = ioctl(sock, SIOCADDRT, (void*)&rt);
    }
    
    if (rc < 0)
        log_msg(LOG_ERROR, "route error");
    
    if (sock >= 0)
    {
        close(sock);
        sock = -1;
    }

    return rc;
}
