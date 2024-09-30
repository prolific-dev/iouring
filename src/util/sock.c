#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "sock.h"

int str_to_addr(char *src, U32BIT *addr)
{
    int rc;
    int d1, d2, d3, d4;

    rc = -1;

    if (src && addr)
    {
        rc = sscanf(src, "%d.%d.%d.%d", &d1, &d2, &d3, &d4);
        if (rc == 4 &&
            d1 >= 0 && d1 <= 255 &&
            d2 >= 0 && d2 <= 255 &&
            d3 >= 0 && d3 <= 255 &&
            d4 >= 0 && d4 <= 255)
        {
            *addr = (d1 << 24) | (d2 << 16) | (d3 << 8) | d4;
            rc = 0;
        }

        if (rc > 0)
            rc = -1;
    }

    return rc;
}

int str_to_haddr(char *src, U32BIT *addr)
{
    int rc;

    rc = -1;

    if (src && addr)
    {
        rc = str_to_addr(src, addr);
        if (rc == 0)
            *addr = htonl(*addr);
    }

    return rc;
}

int str_to_netmask(char *src, U32BIT *netmask)
{
    int rc;
    U32BIT tmp_mask, l_mask, r_mask;

    rc = -1;

    if (src && netmask)
    {
        rc = str_to_addr(src, &tmp_mask);
        if (rc == 0)
        {
            for (r_mask = 0, l_mask = 0x80000000; l_mask != 0; l_mask >>= 1)
            {
                if ((tmp_mask & l_mask) != 0)
                    r_mask |= l_mask;
                else
                    break;
            }

            if (r_mask != tmp_mask)
                rc = -1;
            else if (netmask)
                *netmask = tmp_mask;
        }
    }

    return rc;
}

int cidrstr_to_addr_nmask(char *src, U32BIT *addr, U32BIT *netmask)
{
    int   rc;
    char *delim;
    char *addr_str;
    char *mask_str;

    rc = -1;

    if (src && addr && netmask)
    {
        delim = strchr(src, '/');
        if (delim)
        {
            *delim   = '\0';
            addr_str = src;
            mask_str = delim + 1;

            if ((rc = str_to_haddr(addr_str, addr)) != 0)
                return rc;

            if ((rc = str_to_netmask(mask_str, netmask)) != 0)
                return rc;
            
            rc = 0;
        }
    }

    return rc;
}

char *addr_to_str(U32BIT addr)
{
    static char s_addr[MAX_ADDR_LEN + 1];

    sprintf(s_addr, "%d.%d.%d.%d",
            (addr & 0xff000000) >> 24,
            (addr & 0x00ff0000) >> 16,
            (addr & 0x0000ff00) >> 8,
            (addr & 0x000000ff) >> 0);

    return s_addr;
}

char *haddr_to_str(U32BIT addr)
{
    return addr_to_str(htonl(addr));
}

int str_to_route(char *src, U32BIT *rt_addr1, U32BIT *rt_addr2, U32BIT *rt_nmask1, U32BIT *rt_nmask2)
{
    int rc;
    char *delim;
    char *cidrstr1;
    char *cidrstr2;

    rc = -1;

    if (src && rt_addr1 && rt_addr2 && rt_nmask1 && rt_nmask2)
    {
        delim = strchr(src, '>');
        if (delim)
        {
            *delim   = '\0';
            cidrstr1 = src;
            cidrstr2 = delim + 1;

            if ((rc = cidrstr_to_addr_nmask(cidrstr1, rt_addr1, rt_nmask1)) != 0)
                return rc;

            if ((rc = cidrstr_to_addr_nmask(cidrstr2, rt_addr2, rt_nmask2)) != 0)
                return rc;

            rc = 0;
        }
    }

    return rc;
}

int udp_socket_bind(int fd, U16BIT port)
{
    struct sockaddr_in udp_addr;

    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family      = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port        = htons(port);

    if (bind(fd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0)
    {
        perror("error udp\n");
        return 1;
    }

    return 0;
}

int udp_socket_create()
{
    int fd;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        perror("error udp\n");

    return fd;
}