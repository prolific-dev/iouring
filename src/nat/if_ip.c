#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "if_ip.h"
#include "sock.h"
#include "log.h"
#include "cfg.h"


int addr_config_check(Config *cfg, IP_HEAD *ip, U32BIT *src, U32BIT *dst)
{
    if (IP_SRC(ip) == cfg->nat_ip1 && IP_DST(ip) == cfg->rt_addr1)
    {
        *src = cfg->rt_addr2; 
        *dst = cfg->nat_ip2; 
        return 0;
    }

    if (IP_SRC(ip) == cfg->nat_ip2 && IP_DST(ip) == cfg->rt_addr2)
    {
        *src = cfg->rt_addr1; 
        *dst = cfg->nat_ip1; 
        return 0;
    }

   return 1;
}

char *ip_proto2str(unsigned char proto)
{
    static char s_proto[8];

    switch (proto)
    {
    case 1:  return "icmp";
    case 6:  return "tcp";
    case 17: return "udp";
    
    default: 
        sprintf(s_proto, "n/a:%d", proto);
        return s_proto;
    }
}

U16BIT ip_checksum(U16BIT *addr, int len)
{
    int sum;

    for (sum = 0; len > 1; len -= 2)
        sum += *addr++;
    
    if (len == 1)
        sum += *(unsigned char *)addr;

    sum  = (sum >> 16) + (sum & 0x0000ffff);
    sum += (sum >> 16);

    return (U16BIT) (~sum);
}

U16BIT recalc_checksum(U16BIT checksum, U32BIT remove, U32BIT insert)
{
    int i, sum, upper, lower;
    U16BIT *rmv; 
    U16BIT *ins;

    rmv = (U16BIT *)&remove;
    ins = (U16BIT *)&insert;

    for (i = 0; i <= 1; i++, rmv++, ins++)
    {
        sum = 0x0000ffff & (~checksum);
        if (sum < *rmv)
        {
            lower = sum;
            lower--;
            upper = 1;
            sum = (upper << 16) | (lower & 0x0000ffff);
        }

        sum -= *rmv;
        sum += *ins;
        sum  = (sum >> 16) + (sum & 0x0000ffff);
        sum += (sum >> 16);
        
        checksum = (U16BIT) (~sum);
    }

    if (checksum == 0)
        checksum = 0xffff;

    return checksum;
}

void ip_replace_addr(IP_HEAD *ip, U32BIT src, U32BIT dst)
{
    int            proto, offset;
    unsigned char *data;

    proto   = IP_PROTO(ip);
    offset  = IP_OFFSET(ip);
    data    = (unsigned char *)ip;
    data   += IP_HEADER_LEN(ip);

    if (proto == 6  && offset == 0)
    {
        TCP_HEAD *tcp;

        tcp         = (TCP_HEAD *)data;
        tcp->chksum = recalc_checksum(tcp->chksum, ip->src, src);
        tcp->chksum = recalc_checksum(tcp->chksum, ip->dst, dst);
    }

    if (proto == 17 && offset == 0)
    {
        UDP_HEAD *udp;

        udp         = (UDP_HEAD *)data;
        udp->chksum = recalc_checksum(udp->chksum, ip->src, src);
        udp->chksum = recalc_checksum(udp->chksum, ip->dst, dst);
    }

    ip->chksum = recalc_checksum(ip->chksum, ip->src, src);
    ip->chksum = recalc_checksum(ip->chksum, ip->dst, dst);

    ip->src = src;
    ip->dst = dst;
}

void udp_show(UDP_HEAD *udp, char *dest)
{
    sprintf(dest, "[UDP %d->%d]", UDP_SRCPORT(udp), UDP_DSTPORT(udp));
}

void tcp_show(TCP_HEAD *tcp, char *dest)
{
    sprintf(dest, "[TCP checksum:%d %d->%d]", TCP_CHKSUM(tcp), TCP_SRCPORT(tcp), TCP_DSTPORT(tcp));
}

void icmp_show(ICMP_HEAD *icmp, char *dest)
{
    sprintf(dest, "[ICMP Type:%u Code:%u]", ICMP_TYPE(icmp), ICMP_CODE(icmp));
}

void ip_show(IP_HEAD *ip, char *dest)
{
    U16BIT         chksum;
    unsigned char *data;
    int            offset, dest_len, proto, header_len;
    char           src_s[MAX_ADDR_LEN];
    char           dst_s[MAX_ADDR_LEN];
/*
    int            is_frag;
    int            data_len;
*/
    strcpy(src_s, haddr_to_str(IP_SRC(ip)));
    strcpy(dst_s, haddr_to_str(IP_DST(ip)));

    proto      = IP_PROTO(ip);
    offset     = IP_OFFSET(ip);
    header_len = IP_HEADER_LEN(ip);
    chksum     = ip_checksum((U16BIT *)ip, header_len);

    dest_len = sprintf(dest, "[IPv%d HeaderLen:%d TOS:%d TotalLen:%d ID:%d "
                             "Flag:%02x Offset:%d TTL:%d Protocol:%d/%s "
                             "Checksum:%d(%s) %s->%s]",
                             IP_VERSION(ip), header_len, IP_MY_TOS(ip),
                             IP_TOTAL_LEN(ip), IP_ID(ip), IP_FLAGS(ip),
                             offset, IP_MY_TTL(ip), proto, ip_proto2str(proto),
                             IP_CHKSUM(ip), chksum == 0 ? "OK" : "FAIL",
                             src_s, dst_s);
     
    data = (unsigned char *)ip;
    data += header_len;

    if (proto == 1  && offset == 0) { icmp_show((ICMP_HEAD *)data, dest + dest_len); }
    if (proto == 4  && offset == 0) { ip_show  ((IP_HEAD   *)data, dest + dest_len); }
    if (proto == 6  && offset == 0) { tcp_show ((TCP_HEAD  *)data, dest + dest_len); }
    if (proto == 17 && offset == 0) { udp_show ((UDP_HEAD  *)data, dest + dest_len); }
}

