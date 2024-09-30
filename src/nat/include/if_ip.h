#ifndef __IF_IP_H__
#define __IF_IP_H__

#include "types.h"
#include "cfg.h"

#define IP_VERSION(ip)    (((ip)->ver_len & 0xf0) >> 4)
#define IP_HEADER_LEN(ip) (((ip)->ver_len & 0x0f) * 4)
#define IP_MY_TOS(ip)     ((ip)->TOS)
#define IP_TOTAL_LEN(ip)  (htons((ip)->total_len))
#define IP_ID(ip)         (htons((ip)->id))
#define IP_FLAGS(ip)      (((ip)->flag_offset & 0xe0) >> 5)
#define IP_FLAG_DF        0x02
#define IP_FLAG_FRAG      0x01
#define IP_OFFSET(ip)     ((((U16BIT)((ip)->flag_offset & 0x1f)) * 256 + (ip)->offset_len) * 8)
#define IP_MY_TTL(ip)     ((ip)->TTL)
#define IP_PROTO(ip)      ((ip)->proto)
#define IP_CHKSUM(ip)     ((ip)->chksum)
#define IP_SRC(ip)        ((ip)->src)
#define IP_DST(ip)        ((ip)->dst)

#define ICMP_TYPE(icmp)   ((icmp)->type)
#define ICMP_CODE(icmp)   ((icmp)->code)

#define UDP_SRCPORT(udp)  (htons((udp)->src_port))
#define UDP_DSTPORT(udp)  (htons((udp)->dst_port))
#define UDP_LEN(udp)      (htons((udp)->len))
#define UDP_CHKSUM(udp)   ((udp)->chksum)

#define TCP_SRCPORT(tcp)    (htons((tcp)->src_port))
#define TCP_DSTPORT(tcp)    (htons((tcp)->dst_port))
#define TCP_HEADER_LEN(tcp) ((((tcp)->offset & 0xf0) >> 4) * 4)
#define TCP_CHKSUM(tcp)     ((tcp)->chksum)

typedef struct IP_HEAD
{
    U8BIT  ver_len;
    U8BIT  TOS;
    U16BIT total_len;
    U16BIT id;
    U8BIT  flag_offset;
    U8BIT  offset_len;
    U8BIT  TTL;
    U8BIT  proto;
    U16BIT chksum;
    U32BIT src;
    U32BIT dst;
} IP_HEAD;

typedef struct ICMP_HEAD
{
    U8BIT type;
    U8BIT code;
} ICMP_HEAD;

typedef struct UDP_HEAD
{
    U16BIT src_port;
    U16BIT dst_port;
    U16BIT len;
    U16BIT chksum;
} UDP_HEAD;

typedef struct TCP_HEAD
{
    U16BIT src_port;
    U16BIT dst_port;
    U32BIT seq;
    U32BIT ack;
    U8BIT  offset;
    U8BIT  flags;
    U16BIT window;
    U16BIT chksum;
    U16BIT urgent;
} TCP_HEAD;

typedef struct PACKET
{
    U8BIT  *d_start;
    U32BIT  d_len;
    U8BIT   data[66000];
} PACKET;


char *ip_proto2str(unsigned char proto);

void ip_replace_addr(IP_HEAD *ip, U32BIT src, U32BIT dst);

U16BIT ip_checksum(U16BIT *addr, int len);

void ip_show  (IP_HEAD *ip, char *dst);
void icmp_show(ICMP_HEAD *icmp, char *dst);
void tcp_show (TCP_HEAD *tcp, char *dst);
void udp_show (UDP_HEAD *udp, char *dst);

int addr_config_check(Config *cfg, IP_HEAD *ip, U32BIT *src, U32BIT *dst);

#endif
