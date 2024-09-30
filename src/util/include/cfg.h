#ifndef __CFG_H__
#define __CFG_H__

#include <stdio.h>

#include "log.h"
#include "types.h"

typedef struct Config
{
    char     if_name1[32];
    char     if_name2[32];
    U32BIT   if_addr1;
    U32BIT   if_addr2;
    U32BIT   if_nmask1;
    U32BIT   if_nmask2;
    S16BIT   if_flags;
    int      if_txqlen;
    U32BIT   rt_addr1;
    U32BIT   rt_addr2;
    U32BIT   rt_nmask1;
    U32BIT   rt_nmask2;
    U32BIT   nat_ip1;
    U32BIT   nat_ip2;
    int      use_uring;
    int      parallel;
    int      poll_trigger;
    int      poll_timeout;
    int      queue_depth;
    int      buffer_size;
    int      cpu_affinity;
    LogLevel log_level;
    char     log_path[1024];
    char     stats_path[1024];
    char     stats_name[1024];
} Config;


FILE *cfg_fopen (char *filename);
int   cfg_fclose(FILE *fp);
int   cfg_parse (FILE *fp, Config *cfg);

#endif
