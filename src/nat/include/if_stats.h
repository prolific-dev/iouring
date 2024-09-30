#ifndef __IF_STATS_H__
#define __IF_STATS_H__

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "cfg.h"

typedef struct if_dev_stats
{
    unsigned long long rx_packets;
    unsigned long long rx_bytes;
    unsigned long long rx_errors;
    unsigned long long rx_drops;
    unsigned long long rx_overrun;
    unsigned long long rx_frame;
    unsigned long long rx_compressed;
    unsigned long long rx_multicast;
    unsigned long long tx_packets;
    unsigned long long tx_bytes;
    unsigned long long tx_errors;
    unsigned long long tx_drops;
    unsigned long long tx_overrun;
    unsigned long long tx_collisions;
    unsigned long long tx_carrier;
    unsigned long long tx_compressed;
    unsigned long long bps;
    unsigned long long bitps;
    unsigned long long pps;
    double             packet_loss;
} if_dev_stats;

typedef struct if_stats
{
    int                  capture;
    struct timespec      start;
    struct timespec      end;
    struct timeval       total;
    struct timeval       cpu_secs;
    unsigned int         cpu_percent;
    struct rusage        rusage;
    if_dev_stats         dev_stats_eth1;
    if_dev_stats         dev_stats_nat1;
    if_dev_stats         dev_stats_eth2;
    unsigned long long   iops_nat1;
    unsigned long long   iopcpus_nat1;
    unsigned long long   reads_cnt;
    unsigned long long   writes_cnt;
    unsigned long long   reads_success;
    unsigned long long   writes_success;
    unsigned long long   syscall_cnt;
} if_stats;

void if_stats_start(if_stats *stats, Config *cfg);
void if_stats_stop(if_stats *stats, Config *cfg);

#endif
