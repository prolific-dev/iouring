#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "if_stats.h"
#include "log.h"


void print_stats(if_stats *stats)
{
    log_msg(LOG_INFO,
        "\n============== Statistics ==============\n"
        "\n--- Program stats -----\n"
        "Reads requests     Reads success     Reads failed\n"
        "%-18llu %-18llu %-18llu\n"
        "Writes requests    Writes success    Writes failed\n"
        "%-18llu %-18llu %-18llu\n"
        "System calls\n"
        "%-18llu"
        "\n-----------------------\n"
        "\n--- Time stats -----\n"
        "real              cpu_time              user              sys\n"
        "%3ld.%06ld        %3ld.%06ld          %3ld.%06ld        %3ld.%06ld"
        "\n--------------------\n"
        "\n"
        "\n--- Resources ------\n"
        "CPU (Avg): %3d%%"
        "\n--------------------\n"
        "\n"
        "\n--- eth1 stats -----\n"
        "tx_packets         tx_bytes           tx_errors          tx_drops           tx_overrun\n"
        "%-18llu %-18llu %-18llu %-18llu %-18llu\n"
        "rx_packets         rx_bytes           rx_errors          rx_drops           rx_overrun\n"
        "%-18llu %-18llu %-18llu %-18llu %-18llu\n"
        "\n"
        "Packet/s           Bytes/s            Bits/s                Packet loss\n"
        "%-18llu %-18llu %-18llu      %.2f%%"
        "\n--------------------\n"
        "\n--- nat1 stats -----\n"
        "tx_packets         tx_bytes           tx_errors          tx_drops           tx_overrun\n"
        "%-18llu %-18llu %-18llu %-18llu %-18llu\n"
        "rx_packets         rx_bytes           rx_errors          rx_drops           rx_overrun\n"
        "%-18llu %-18llu %-18llu %-18llu %-18llu\n"
        "\n"
        "Packet/s           Bytes/s            Bits/s                Packet loss\n"
        "%-18llu %-18llu %-18llu      %.2f%%\n"
        "\n"
        "IOPS (total)       IOPS (CPU sec)\n"
        "%-18llu %-18llu"
        "\n--------------------\n"
        "\n--- eth2 stats -----\n"
        "tx_packets         tx_bytes           tx_errors          tx_drops           tx_overrun\n"
        "%-18llu %-18llu %-18llu %-18llu %-18llu\n"
        "rx_packets         rx_bytes           rx_errors          rx_drops           rx_overrun\n"
        "%-18llu %-18llu %-18llu %-18llu %-18llu\n"
        "\n"
        "Packet/s           Bytes/s            Bits/s                Packet loss\n"
        "%-18llu %-18llu %-18llu      %.2f%%"
        "\n--------------------\n"
        "\n--- [IGNORE] -----\n"
        "Number of context switches:           %ld\n"
        "Number of voluntary context switches: %ld\n"
        "Number of involuntary context switches: %ld\n",
        stats->reads_cnt, stats->reads_success, stats->reads_cnt - stats->reads_success,
        stats->writes_cnt, stats->writes_success, stats->writes_cnt - stats->writes_success,
        stats->syscall_cnt,
        stats->total.tv_sec, stats->total.tv_usec, stats->cpu_secs.tv_sec, stats->cpu_secs.tv_usec,
        stats->rusage.ru_utime.tv_sec, stats->rusage.ru_utime.tv_usec,
        stats->rusage.ru_stime.tv_sec, stats->rusage.ru_stime.tv_usec,
        stats->cpu_percent,
        stats->dev_stats_eth1.tx_packets, stats->dev_stats_eth1.tx_bytes,
        stats->dev_stats_eth1.tx_errors, stats->dev_stats_eth1.tx_drops,
        stats->dev_stats_eth1.tx_overrun,
        stats->dev_stats_eth1.rx_packets, stats->dev_stats_eth1.rx_bytes,
        stats->dev_stats_eth1.rx_errors, stats->dev_stats_eth1.rx_drops,
        stats->dev_stats_eth1.rx_overrun,
        stats->dev_stats_eth1.pps, stats->dev_stats_eth1.bps,
        stats->dev_stats_eth1.bitps, stats->dev_stats_eth1.packet_loss,
        stats->dev_stats_nat1.tx_packets, stats->dev_stats_nat1.tx_bytes,
        stats->dev_stats_nat1.tx_errors, stats->dev_stats_nat1.tx_drops,
        stats->dev_stats_nat1.tx_overrun,
        stats->dev_stats_nat1.rx_packets, stats->dev_stats_nat1.rx_bytes,
        stats->dev_stats_nat1.rx_errors, stats->dev_stats_nat1.rx_drops,
        stats->dev_stats_nat1.rx_overrun,
        stats->dev_stats_nat1.pps, stats->dev_stats_nat1.bps,
        stats->dev_stats_nat1.bitps, stats->dev_stats_nat1.packet_loss,
        stats->iops_nat1, stats->iopcpus_nat1,
        stats->dev_stats_eth2.tx_packets, stats->dev_stats_eth2.tx_bytes,
        stats->dev_stats_eth2.tx_errors, stats->dev_stats_eth2.tx_drops,
        stats->dev_stats_eth2.tx_overrun,
        stats->dev_stats_eth2.rx_packets, stats->dev_stats_eth2.rx_bytes,
        stats->dev_stats_eth2.rx_errors, stats->dev_stats_eth2.rx_drops,
        stats->dev_stats_eth2.rx_overrun,
        stats->dev_stats_eth2.pps, stats->dev_stats_eth2.bps,
        stats->dev_stats_eth2.bitps, stats->dev_stats_eth2.packet_loss,
        stats->rusage.ru_nvcsw + stats->rusage.ru_nivcsw,
        stats->rusage.ru_nvcsw, stats->rusage.ru_nivcsw
    );
}

void write_csv(if_stats *stats, Config *cfg)
{
    FILE *fp;
    long  fsize;

    fp = fopen(cfg->stats_path, "r");
    if (fp == NULL)
    {
        log_msg(LOG_ERROR, "Failed to open stats.csv %p errno=%d", fp, errno);
        return;
    }

    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    fclose(fp);

    fp = fopen(cfg->stats_path, "a");
    if (fp == NULL)
    {
        log_msg(LOG_ERROR, "Failed to open stats.csv");
        return;
    }

    /* If file is empty, write header */
    if (fsize == 0)
    {
        fprintf(fp, "%s\n",
                "test_name,"
                "start_time,end_time,"
                "cfg_if_txqlen,cfg_use_uring,cfg_parallel,cfg_poll_trigger,cfg_poll_timeout,cfg_queue_depth,cfg_buffer_size,cpu_affinity,"
                "reads_cnt,reads_success,reads_failed,writes_cnt,writes_success,writes_failed,syscall_cnt,"
                "real,cpu_time,user,sys,"
                "cpu_percent,iops_nat1,iopcpus_nat1,"
                "eth1_tx_packets,eth1_tx_bytes,eth1_tx_errors,eth1_tx_drops,eth1_tx_overrun,"
                "eth1_rx_packets,eth1_rx_bytes,eth1_rx_errors,eth1_rx_drops,eth1_rx_overrun,"
                "eth1_pps,eth1_bps,eth1_bitps,eth1_packet_loss,"
                "nat1_tx_packets,nat1_tx_bytes,nat1_tx_errors,nat1_tx_drops,nat1_tx_overrun,"
                "nat1_rx_packets,nat1_rx_bytes,nat1_rx_errors,nat1_rx_drops,nat1_rx_overrun,"
                "nat1_pps,nat1_bps,nat1_bitps,nat1_packet_loss,"
                "eth2_tx_packets,eth2_tx_bytes,eth2_tx_errors,eth2_tx_drops,eth2_tx_overrun,"
                "eth2_rx_packets,eth2_rx_bytes,eth2_rx_errors,eth2_rx_drops,eth2_rx_overrun,"
                "eth2_pps,eth2_bps,eth2_bitps,eth2_packet_loss,"
                "context_switches,voluntary_context_switches,involuntary_context_switches"
        );
    }

    /* Write data */
    fprintf(fp, "%s,"
                "%ld.%ld,%ld.%ld,"
                "%d,%d,%d,%d,%d,%d,%d,%d,"
                "%llu,%llu,%llu,%llu,%llu,%llu,%llu,"
                "%ld.%ld,%ld.%ld,%ld.%ld,%ld.%ld,"
                "%d,%llu,%llu,"
                "%llu,%llu,%llu,%llu,%llu,"
                "%llu,%llu,%llu,%llu,%llu,"
                "%llu,%llu,%llu,%.2f,"
                "%llu,%llu,%llu,%llu,%llu,"
                "%llu,%llu,%llu,%llu,%llu,"
                "%llu,%llu,%llu,%.2f,"
                "%llu,%llu,%llu,%llu,%llu,"
                "%llu,%llu,%llu,%llu,%llu,"
                "%llu,%llu,%llu,%.2f,"
                "%ld,%ld,%ld\n",
                cfg->stats_name,
                stats->start.tv_sec, stats->start.tv_nsec, stats->end.tv_sec, stats->end.tv_nsec,
                cfg->if_txqlen, cfg->use_uring, cfg->parallel, cfg->poll_trigger, cfg->poll_timeout, cfg->queue_depth, cfg->buffer_size, cfg->cpu_affinity,
                stats->reads_cnt, stats->reads_success, stats->reads_cnt - stats->reads_success, stats->writes_cnt, stats->writes_success, stats->writes_cnt - stats->writes_success,
                stats->syscall_cnt,
                stats->total.tv_sec, stats->total.tv_usec, stats->cpu_secs.tv_sec, stats->cpu_secs.tv_usec,
                stats->rusage.ru_utime.tv_sec, stats->rusage.ru_utime.tv_usec, stats->rusage.ru_stime.tv_sec, stats->rusage.ru_stime.tv_usec,
                stats->cpu_percent, stats->iops_nat1, stats->iopcpus_nat1,
                stats->dev_stats_eth1.tx_packets, stats->dev_stats_eth1.tx_bytes, stats->dev_stats_eth1.tx_errors, stats->dev_stats_eth1.tx_drops, stats->dev_stats_eth1.tx_overrun,
                stats->dev_stats_eth1.rx_packets, stats->dev_stats_eth1.rx_bytes, stats->dev_stats_eth1.rx_errors, stats->dev_stats_eth1.rx_drops, stats->dev_stats_eth1.rx_overrun,
                stats->dev_stats_eth1.pps, stats->dev_stats_eth1.bps, stats->dev_stats_eth1.bitps, stats->dev_stats_eth1.packet_loss,
                stats->dev_stats_nat1.tx_packets, stats->dev_stats_nat1.tx_bytes, stats->dev_stats_nat1.tx_errors, stats->dev_stats_nat1.tx_drops, stats->dev_stats_nat1.tx_overrun,
                stats->dev_stats_nat1.rx_packets, stats->dev_stats_nat1.rx_bytes, stats->dev_stats_nat1.rx_errors, stats->dev_stats_nat1.rx_drops, stats->dev_stats_nat1.rx_overrun,
                stats->dev_stats_nat1.pps, stats->dev_stats_nat1.bps, stats->dev_stats_nat1.bitps, stats->dev_stats_nat1.packet_loss,
                stats->dev_stats_eth2.tx_packets, stats->dev_stats_eth2.tx_bytes, stats->dev_stats_eth2.tx_errors, stats->dev_stats_eth2.tx_drops, stats->dev_stats_eth2.tx_overrun,
                stats->dev_stats_eth2.rx_packets, stats->dev_stats_eth2.rx_bytes, stats->dev_stats_eth2.rx_errors, stats->dev_stats_eth2.rx_drops, stats->dev_stats_eth2.rx_overrun,
                stats->dev_stats_eth2.pps, stats->dev_stats_eth2.bps, stats->dev_stats_eth2.bitps, stats->dev_stats_eth2.packet_loss,
                stats->rusage.ru_nvcsw + stats->rusage.ru_nivcsw, stats->rusage.ru_nvcsw, stats->rusage.ru_nivcsw
    );

    fclose(fp);
}

void start_dev_stats(if_stats *stats)
{
    FILE *fp;
    char line[256];

    if_dev_stats *dev_stats_eth1 = &stats->dev_stats_eth1;
    if_dev_stats *dev_stats_eth2 = &stats->dev_stats_eth2;
    if_dev_stats *dev_stats_nat1 = &stats->dev_stats_nat1;

    memset(dev_stats_eth1, 0, sizeof(if_dev_stats));
    memset(dev_stats_eth2, 0, sizeof(if_dev_stats));
    memset(dev_stats_nat1, 0, sizeof(if_dev_stats));

    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL)
    {
        log_msg(LOG_ERROR, "Failed to open /proc/net/dev");
        return;
    }

    /* Skip first two lines */
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp))
    {
        char iface[32];
        char *colon;

        sscanf(line, " %31s", iface);
        colon = strchr(iface, ':');

        if (colon)
            *colon = '\0';

        if (strcmp(iface, "nat1") == 0)
            sscanf(line, " %*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                &dev_stats_nat1->rx_bytes, &dev_stats_nat1->rx_packets, &dev_stats_nat1->rx_errors,
                &dev_stats_nat1->rx_drops, &dev_stats_nat1->rx_overrun, &dev_stats_nat1->rx_frame,
                &dev_stats_nat1->rx_compressed, &dev_stats_nat1->rx_multicast, &dev_stats_nat1->tx_bytes,
                &dev_stats_nat1->tx_packets, &dev_stats_nat1->tx_errors, &dev_stats_nat1->tx_drops,
                &dev_stats_nat1->tx_overrun, &dev_stats_nat1->tx_collisions);
        else if (strcmp(iface, "eth1") == 0)
            sscanf(line, " %*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                &dev_stats_eth1->rx_bytes, &dev_stats_eth1->rx_packets, &dev_stats_eth1->rx_errors,
                &dev_stats_eth1->rx_drops, &dev_stats_eth1->rx_overrun, &dev_stats_eth1->rx_frame,
                &dev_stats_eth1->rx_compressed, &dev_stats_eth1->rx_multicast, &dev_stats_eth1->tx_bytes,
                &dev_stats_eth1->tx_packets, &dev_stats_eth1->tx_errors, &dev_stats_eth1->tx_drops,
                &dev_stats_eth1->tx_overrun, &dev_stats_eth1->tx_collisions);
        else if (strcmp(iface, "eth2") == 0)
            sscanf(line, " %*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                &dev_stats_eth2->rx_bytes, &dev_stats_eth2->rx_packets, &dev_stats_eth2->rx_errors,
                &dev_stats_eth2->rx_drops, &dev_stats_eth2->rx_overrun, &dev_stats_eth2->rx_frame,
                &dev_stats_eth2->rx_compressed, &dev_stats_eth2->rx_multicast, &dev_stats_eth2->tx_bytes,
                &dev_stats_eth2->tx_packets, &dev_stats_eth2->tx_errors, &dev_stats_eth2->tx_drops,
                &dev_stats_eth2->tx_overrun, &dev_stats_eth2->tx_collisions);
        else
        {
            log_msg(LOG_TRACE, "Unknown interface %s", iface);
            continue;
        }
    }

    fclose(fp);
}

void calc_dev_stats_delta(if_dev_stats *new, if_dev_stats *old)
{
    new->rx_bytes      -= old->rx_bytes;
    new->rx_packets    -= old->rx_packets;
    new->rx_errors     -= old->rx_errors;
    new->rx_drops      -= old->rx_drops;
    new->rx_overrun    -= old->rx_overrun;
    new->rx_frame      -= old->rx_frame;
    new->rx_compressed -= old->rx_compressed;
    new->rx_multicast  -= old->rx_multicast;
    new->tx_bytes      -= old->tx_bytes;
    new->tx_packets    -= old->tx_packets;
    new->tx_errors     -= old->tx_errors;
    new->tx_drops      -= old->tx_drops;
    new->tx_overrun    -= old->tx_overrun;
    new->tx_collisions -= old->tx_collisions;
    new->tx_carrier    -= old->tx_carrier;
    new->tx_compressed -= old->tx_compressed;
}

void stop_dev_stats(if_stats *stats)
{
    if_dev_stats eth1_tmp;
    if_dev_stats eth2_tmp;
    if_dev_stats nat1_tmp;

    eth1_tmp = stats->dev_stats_eth1;
    eth2_tmp = stats->dev_stats_eth2;
    nat1_tmp = stats->dev_stats_nat1;

    start_dev_stats(stats);

    calc_dev_stats_delta(&stats->dev_stats_eth1, &eth1_tmp);
    calc_dev_stats_delta(&stats->dev_stats_eth2, &eth2_tmp);
    calc_dev_stats_delta(&stats->dev_stats_nat1, &nat1_tmp);
}

unsigned int calc_cpu_percent(if_stats *stats)
{
    long total_usec = stats->total.tv_sec * 1000000 + stats->total.tv_usec;
    long cpu_usec   = stats->cpu_secs.tv_sec * 1000000 + stats->cpu_secs.tv_usec;

    return (unsigned int)(cpu_usec * 100 / total_usec);
}

void calc_dev_stats(if_stats *stats)
{
    unsigned long long drops;
    unsigned long long packets;

    unsigned long long total_ms;
    unsigned long long cpu_ms;

    long double result_double;

    if (stats->total.tv_sec < 1)
        return;

    total_ms = (unsigned long long) (stats->total.tv_sec * 1000 + (stats->total.tv_usec / 1000));
    cpu_ms   = (unsigned long long) (stats->cpu_secs.tv_sec * 1000 + (stats->cpu_secs.tv_usec / 1000));

    drops         = stats->dev_stats_eth1.rx_drops;
    packets       = stats->dev_stats_eth1.rx_packets + stats->dev_stats_eth1.rx_drops;
    result_double = (long double)drops / packets;

    stats->dev_stats_eth1.packet_loss = result_double * 100.0;

    drops         = stats->dev_stats_nat1.tx_drops;
    packets       = stats->dev_stats_nat1.tx_packets + stats->dev_stats_nat1.tx_drops;
    result_double = (long double)drops / packets;

    stats->dev_stats_nat1.packet_loss = result_double * 100.0;

    drops         = stats->dev_stats_eth2.tx_drops;
    packets       = stats->dev_stats_eth2.tx_packets + stats->dev_stats_eth2.tx_drops;
    result_double = (long double)drops / packets;

    stats->dev_stats_eth2.packet_loss = result_double * 100.0;

    stats->dev_stats_eth1.bps = (stats->dev_stats_eth1.rx_bytes / total_ms) * 1000;
    stats->dev_stats_eth1.pps = (stats->dev_stats_eth1.rx_packets / total_ms) * 1000;

    stats->dev_stats_nat1.bps = (stats->dev_stats_nat1.tx_bytes / total_ms) * 1000;
    stats->dev_stats_nat1.pps = (stats->dev_stats_nat1.tx_packets / total_ms) * 1000;

    stats->dev_stats_eth2.bps = (stats->dev_stats_eth2.tx_bytes / total_ms) * 1000;
    stats->dev_stats_eth2.pps = (stats->dev_stats_eth2.tx_packets / total_ms) * 1000;

    stats->dev_stats_eth1.bitps = stats->dev_stats_eth1.bps * 8;
    stats->dev_stats_nat1.bitps = stats->dev_stats_nat1.bps * 8;
    stats->dev_stats_eth2.bitps = stats->dev_stats_eth2.bps * 8;

    stats->iops_nat1    = ((stats->dev_stats_nat1.rx_packets * 2) / total_ms) * 1000;

    if (stats->total.tv_sec >= 1)
        stats->iopcpus_nat1 = ((stats->dev_stats_nat1.rx_packets * 2) / cpu_ms) * 1000;
}

void calc_time_stats(if_stats *stats)
{
    struct timespec ts;

    ts.tv_sec = stats->end.tv_sec - stats->start.tv_sec;
    ts.tv_nsec = stats->end.tv_nsec - stats->start.tv_nsec;
    if (ts.tv_nsec < 0)
    {
        ts.tv_sec--;
        ts.tv_nsec += 1000000000L;
    }

    stats->total.tv_sec  = ts.tv_sec;
    stats->total.tv_usec = ts.tv_nsec / 1000;

    stats->cpu_secs.tv_sec  = stats->rusage.ru_utime.tv_sec + stats->rusage.ru_stime.tv_sec;
    stats->cpu_secs.tv_usec = stats->rusage.ru_utime.tv_usec + stats->rusage.ru_stime.tv_usec;
    if (stats->cpu_secs.tv_usec >= 1000000L)
    {
        stats->cpu_secs.tv_sec++;
        stats->cpu_secs.tv_usec -= 1000000L;
    }

    stats->cpu_percent = calc_cpu_percent(stats);
}

void start_rusage(if_stats *stats)
{
    memset(&stats->rusage, 0, sizeof(struct rusage));
    getrusage(RUSAGE_SELF, &stats->rusage);
}

void stop_rusage(if_stats *stats)
{
    struct rusage rusage;

    memset(&rusage, 0, sizeof(struct rusage));
    getrusage(RUSAGE_SELF, &rusage);

    stats->rusage.ru_utime.tv_sec  = rusage.ru_utime.tv_sec  - stats->rusage.ru_utime.tv_sec;
    stats->rusage.ru_utime.tv_usec = rusage.ru_utime.tv_usec - stats->rusage.ru_utime.tv_usec;
    if (stats->rusage.ru_utime.tv_usec < 0)
    {
        stats->rusage.ru_utime.tv_sec--;
        stats->rusage.ru_utime.tv_usec += 1000000L;
    }

    stats->rusage.ru_stime.tv_sec  = rusage.ru_stime.tv_sec  - stats->rusage.ru_stime.tv_sec;
    stats->rusage.ru_stime.tv_usec = rusage.ru_stime.tv_usec - stats->rusage.ru_stime.tv_usec;
    if (stats->rusage.ru_stime.tv_usec < 0)
    {
        stats->rusage.ru_stime.tv_sec--;
        stats->rusage.ru_stime.tv_usec += 1000000L;
    }

    stats->rusage.ru_nvcsw  = rusage.ru_nvcsw  - stats->rusage.ru_nvcsw;
    stats->rusage.ru_nivcsw = rusage.ru_nivcsw - stats->rusage.ru_nivcsw;
}

void if_stats_stop(if_stats *stats, Config *cfg)
{
    stats->capture = 0;
    clock_gettime(CLOCK_MONOTONIC, &stats->end);

    stop_rusage    (stats);
    stop_dev_stats (stats);

    calc_time_stats(stats);
    calc_dev_stats (stats);

    print_stats    (stats);
    write_csv      (stats, cfg);
}

void if_stats_start(if_stats *stats, Config *cfg)
{
    memset(stats, 0, sizeof(if_stats));

    start_dev_stats(stats);
    start_rusage   (stats);

    clock_gettime(CLOCK_MONOTONIC, &stats->start);
    stats->capture = 1;
}
