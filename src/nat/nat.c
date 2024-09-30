#define _GNU_SOURCE

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
/* REMOVE LATER */
#include <linux/if.h>
/* REMOVE LATER */
#include <netinet/in.h>
#include <sys/stat.h>

#include <sys/utsname.h>
#include <liburing.h>
#include <liburing/io_uring.h>

#include "sock.h"
#include "types.h"
#include "if_net.h"
#include "if_ip.h"
#include "if_poll.h"
#include "if_nat.h"

#include <sched.h>

/* Function Pointer NAT-Handler */
typedef int (*if_nat_handler)(CTX *ctx);

/* Config */
static Config cfg = (Config)
{
    .if_name1     = {'\0'},
    .if_name2     = {'\0'},
    .if_addr1     = 0,
    .if_addr2     = 0,
    .if_nmask1    = 0,
    .if_nmask2    = 0,
    .if_flags     = IFF_UP|IFF_POINTOPOINT|IFF_NOTRAILERS|IFF_NOARP|IFF_MULTICAST,
    .if_txqlen    = 1000,
    .rt_addr1     = 0,
    .rt_addr2     = 0,
    .rt_nmask1    = 0,
    .rt_nmask2    = 0,
    .poll_trigger = DEFAULT_POLL_TRIGGER,
    .poll_timeout = DEFAULT_POLL_TIMEOUT,
    .use_uring    = 0,
    .queue_depth  = 0,
    .buffer_size  = DEFAULT_LIBURING_BUFSIZE,
    .parallel     = 0,
    .cpu_affinity = 0,
    .log_level    = LOG_INFO,
    .log_path     = {'\0'},
    .stats_path   = {'\0'}
};

/* File Descriptors */
static int fd1      = -1;
static int fd2      = -1;
static int stats_fd = -1;

/* Context for handlers */
CTX ctx;

/* Statistics */
if_stats stats;

static const char *op_strs[33] = {
        "IORING_OP_NOP",
        "IORING_OP_READV",
        "IORING_OP_WRITEV",
        "IORING_OP_FSYNC",
        "IORING_OP_READ_FIXED",
        "IORING_OP_WRITE_FIXED",
        "IORING_OP_POLL_ADD",
        "IORING_OP_POLL_REMOVE",
        "IORING_OP_SYNC_FILE_RANGE",
        "IORING_OP_SENDMSG",
        "IORING_OP_RECVMSG",
        "IORING_OP_TIMEOUT",
        "IORING_OP_TIMEOUT_REMOVE",
        "IORING_OP_ACCEPT",
        "IORING_OP_ASYNC_CANCEL",
        "IORING_OP_LINK_TIMEOUT",
        "IORING_OP_CONNECT",
        "IORING_OP_FALLOCATE",
        "IORING_OP_OPENAT",
        "IORING_OP_CLOSE",
        "IORING_OP_FILES_UPDATE",
        "IORING_OP_STATX",
        "IORING_OP_READ",
        "IORING_OP_WRITE",
        "IORING_OP_FADVISE",
        "IORING_OP_MADVISE",
        "IORING_OP_SEND",
        "IORING_OP_RECV",
        "IORING_OP_OPENAT2",
        "IORING_OP_EPOLL_CTL",
        "IORING_OP_SPLICE",
        "IORING_OP_PROVIDE_BUFFERS",
        "IORING_OP_REMOVE_BUFFERS",
};

int probe_liburing()
{
    struct utsname u;
    struct io_uring_probe *probe;
    uname(&u);
    log_msg(LOG_INFO, "You are running kernel version: %s", u.release);
    probe = io_uring_get_probe();
    log_msg(LOG_INFO, "Report of your kernel's list of supported io_uring operations:");
    for (int i = 0; i < 33; i++ ) {
        if(io_uring_opcode_supported(probe, i))
            log_msg(LOG_INFO, "%s: yes.\n", op_strs[i]);
        else
            log_msg(LOG_INFO, "%s: no.\n", op_strs[i]);
    }
    return 0;
}

static void print_usage()
{
    fprintf(stderr,
            "Usage: ./main -c <config_file_path>                                                       \n"
            "       ./main -i <name> -a <addr/netmask> -r <route>                                      \n"
            "       ./main -p <name1>,<addr1/netmask1>:<name2>,<addr2/netmask2> -r <route>             \n"
            "                                                                                          \n"
            "Configuration over file:                                                                  \n"
            "        -c <config_file_path>       Config File Path.                                     \n"
            "                                                                                          \n"
            "Configuration over arguments:                                                             \n"
            "        -i <name>                   Interface Name.                                       \n"
            "        -a <addr/netmask>           Interface Address and Netmask.                        \n"
            "        -r <src_route>,<dest_route> Interface Routes.                                     \n"
            "                                                                                          \n"
            "Optional:                                                                                 \n"
            "        -l <loglevel>               Log Level.                                            \n"
            "        -L <log_file_path>          File Path for Logging.                                \n"
            "        -h                          Print Help.                                           \n"
            "        -?                          Print Help.                                           \n"
            "                                                                                          \n"
            "If configuration file is in use, other parameters won't be allowed.                       \n"
            "                                                                                          \n"
            "                                                                                          \n"
            "Default:                                                                                  \n"
            "       name            = nat1                                                             \n"
            "       address/netmask = 1.1.1.1/32                                                       \n"
            "       route           = 17.0.0.1/24>18.0.0.1/24                                          \n"
            "                                                                                          \n"
            "                                                                                          \n"
            "Examples:                                                                                 \n"
            "       ./main -c /tmp/nat.conf                                                            \n"
            "       ./main -i nat1 -a 1.1.1.1/32 -r 17.0.0.1/24>18.0.0.1/24                            \n"
            "                                                                                          \n"
            "       ./main -p nat1,1.1.1.1/32:nat2,2.2.2.2/32 -r 17.0.0.1/24>18.0.0.1/24               \n"
            "                                                                                          \n"
            "                                                                                          \n");
    fflush(stderr);
}

int set_default_interface(Config *cfg)
{
    int rc = 0;

    if (strlen(cfg->if_name1) == 0)
        strcpy(cfg->if_name1, "nat1");

    if (strlen(cfg->if_name2) == 0)
        strcpy(cfg->if_name2, "nat2");
    
    if (cfg->if_addr1 == 0)
        str_to_haddr("1.1.1.1", &cfg->if_addr1);

    if (cfg->if_nmask1 == 0)
        str_to_haddr("255.255.255.255", &cfg->if_nmask1);

    if (cfg->if_addr2 == 0)
        str_to_haddr("2.2.2.2", &cfg->if_addr2);

    if (cfg->if_nmask2 == 0)
        str_to_haddr("255.255.255.255", &cfg->if_nmask2);

    if (cfg->rt_addr1 == 0)
        str_to_haddr("17.0.0.0", &cfg->rt_addr1);

    if (cfg->rt_nmask1 == 0)
        str_to_haddr("255.255.255.0", &cfg->rt_nmask1);

    if (cfg->rt_addr2 == 0)
        str_to_haddr("18.0.0.0", &cfg->rt_addr2);

    if (cfg->rt_nmask2 == 0)
        str_to_haddr("255.255.255.0", &cfg->rt_nmask2);

    if (cfg->nat_ip1 == 0)
        str_to_haddr("172.16.31.221", &cfg->nat_ip1);

    if (cfg->nat_ip2 == 0)
        str_to_haddr("172.16.32.223", &cfg->nat_ip2);

    return rc;
}

int check_config(Config *cfg)
{
    char  msg[4096];
    int   len;
    int   rc = 0;

    char s_if_addr1 [MAX_ADDR_LEN];
    char s_if_addr2 [MAX_ADDR_LEN];
    char s_if_nmask1[MAX_ADDR_LEN];
    char s_if_nmask2[MAX_ADDR_LEN];
    char s_rt_addr1 [MAX_ADDR_LEN];
    char s_rt_addr2 [MAX_ADDR_LEN];
    char s_rt_nmask1[MAX_ADDR_LEN];
    char s_rt_nmask2[MAX_ADDR_LEN];

    log_set_level(cfg->log_level);
    if (strlen(cfg->log_path) > 0)
        log_set_file(cfg->log_path);

    rc = set_default_interface(cfg);

    if (rc == 0)
    {
        strcpy(s_if_addr1,  haddr_to_str(cfg->if_addr1));
        strcpy(s_if_addr2,  haddr_to_str(cfg->if_addr2));
        strcpy(s_if_nmask1, haddr_to_str(cfg->if_nmask1));
        strcpy(s_if_nmask2, haddr_to_str(cfg->if_nmask2));
        strcpy(s_rt_addr1,  haddr_to_str(cfg->rt_addr1));
        strcpy(s_rt_addr2,  haddr_to_str(cfg->rt_addr2));
        strcpy(s_rt_nmask1, haddr_to_str(cfg->rt_nmask1));
        strcpy(s_rt_nmask2, haddr_to_str(cfg->rt_nmask2));

        len = 0;
        len += snprintf(msg + len, sizeof(msg) - len,
                "\n=== Configuration parameter ======================\n"
                "[Mode]\n"
                "Use io_uring:\t\t%d\n"
                "Poll Timeout:\t\t%d\n"
                "Poll Trigger:\t\t%d\n"
                "CPU Affinity:\t\t%d\n"
                "----------------------------------------------\n"
                "[Liburing]\n"
                "Queue Depth:\t\t%d\n"
                "Buffer Size:\t\t%d\n"
                "Parallel:\t\t%d\n"
                "----------------------------------------------\n"
                "[Logging]\n"
                "Log Level:\t\t%s\n"
                "Log Path:\t\t%s\n"
                "----------------------------------------------\n"
                "[Stats]\n"
                "Stats Path:\t\t%s\n"
                "Test Name:\t\t%s\n"
                "----------------------------------------------\n",
                cfg->use_uring, cfg->poll_timeout, cfg->poll_trigger, cfg->cpu_affinity,
                cfg->queue_depth, cfg->buffer_size, cfg->parallel,
                log_lvl2str(cfg->log_level, NULL), cfg->log_path, cfg->stats_path, cfg->stats_name);

        if (cfg->parallel)
        {
            len += snprintf(msg + len, sizeof(msg) - len,
                    "[Network Interface 1]\n"
                    "Interface 1 Name:\t%s\n"
                    "Interface 1 Addr:\t%s\n"
                    "Interface 1 Netmask:\t%s\n"
                    "Interface 1 Route:\t%s -> %s\n"
                    "----------------------------------------------\n"
                    "[Network Interface 2]\n"
                    "Interface 2 Name:\t%s\n"
                    "Interface 2 Addr:\t%s\n"
                    "Interface 2 Netmask:\t%s\n"
                    "Interface 2 Route:\t%s -> %s\n"
                    "==================================================\n",
                    cfg->if_name1, s_if_addr1, s_if_nmask1, s_rt_addr1, s_rt_addr2,
                    cfg->if_name2, s_if_addr2, s_if_nmask2, s_rt_addr2, s_rt_addr1);
        }
        else
        {
            len += snprintf(msg + len, sizeof(msg) - len,
                    "[Network Interface]\n"
                    "Interface Name:\t\t%s\n"
                    "Interface Addr:\t\t%s\n"
                    "Interface Netmask:\t%s\n"
                    "Interface Route:\t%s <-> %s\n"
                    "==================================================\n",
                    cfg->if_name1, s_if_addr1, s_if_nmask1, s_rt_addr1, s_rt_addr2);
        }

        log_msg(LOG_INFO, "%s", msg);
    }

    return rc;
}

int set_interface(Config *cfg, int *fd1, int *fd2)
{
    int rc = 1;

    *fd1 = if_net_open(cfg->if_name1, cfg->if_addr1, cfg->if_nmask1, cfg->if_flags, cfg->if_txqlen);
    if (*fd1 >= 0)
    {
        rc = if_net_route(*fd1, cfg->if_name1, cfg->rt_addr1, cfg->rt_nmask1);
        if (cfg->parallel)
        {
            *fd2 = if_net_open(cfg->if_name2, cfg->if_addr2, cfg->if_nmask2, cfg->if_flags, cfg->if_txqlen);
            if (*fd2 >= 0)
                rc = if_net_route(*fd2, cfg->if_name2, cfg->rt_addr2, cfg->rt_nmask2);
        }
        else
        {
            rc = if_net_route(*fd1, cfg->if_name1, cfg->rt_addr2, cfg->rt_nmask2);
        }
    }

    return rc;
}

int setup_nat(Config *cfg)
{
    int rc;

    /* Log summarize of config + additionally checking if parameters are in correct format */
    rc = check_config(cfg);

    /* Open net interface and specify route */
    if (rc == 0)
        rc = set_interface(cfg, &fd1, &fd2);

    if (rc == 0)
    {
        if ((stats_fd = udp_socket_create()) < 0)
            rc = 1;

        if (rc >= 0 && (udp_socket_bind(stats_fd, 12000)) < 0)
            rc = 1;
    }

    return rc;
}

void set_cpu_affinity(int cpu)
{
    pid_t pid;
    cpu_set_t cpuset;
    
    if (cpu == 0)
        return;

    pid = getpid();

    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    if (sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset) == -1)
    {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    int     rc       =  0;
    int     opt      =  0;
    int     errFlag  =  0;
    char   *cfg_path = NULL;

    if_nat_handler handler;

    while ((opt = getopt(argc, argv, "c:i:a:n:r:l:L:q:u:pPh?")) != EOF)
    {
        switch (opt)
        {
            case 'c':
                cfg_path = optarg;
                break;

            case 'i':
                if (strlen(cfg.if_name1) > 0        ||
                    strlen(optarg) < MIN_IFNAME_LEN ||
                    strlen(optarg) > MAX_IFNAME_LEN)
                    errFlag = 1;
                else
                    strcpy(cfg.if_name1, optarg);
                break;

            case 'a':
                if (cfg.if_addr1   > 0 ||
                    cfg.if_nmask1  > 0 ||
                    strlen(optarg) > MAX_CIDR_LEN)
                {
                    errFlag = 1;
                }
                else
                {
                    if (cidrstr_to_addr_nmask(optarg, &cfg.if_addr1, &cfg.if_nmask1) != 0)
                        errFlag = 1;
                }
                break;

            case 'r':

                if (strlen(optarg) < ((MIN_ADDR_LEN * 2) - 1) ||
                    strlen(optarg) > ((MAX_ADDR_LEN * 2) - 1))
                {
                    errFlag = 1;
                }
                else
                {
                    if (str_to_route(optarg, &cfg.rt_addr1, &cfg.rt_addr2, &cfg.rt_nmask1, &cfg.rt_nmask2) != 0)
                        errFlag = 1;
                }
                break;
            
            case 'l':
                if (cfg.log_level != LOG_INFO || strlen(optarg) > MAX_LOG_LEVEL_LEN)
                    errFlag = 1;
                else
                    cfg.log_level = log_str2lvl(optarg);
                break;
            
            case 'L':
                if (strlen(cfg.log_path) > 0 || strlen(optarg) > MAX_LOG_PATH_LEN)
                    errFlag = 1;
                else
                    strcpy(cfg.log_path, optarg);
                break;
            
            case 'u':
                if (cfg.use_uring > 0)
                    errFlag = 1;
                cfg.use_uring = atoi(optarg);
                if (cfg.use_uring > 6)
                    errFlag = 1;
                break;

            case 'q':
                if (cfg.queue_depth > 0)
                    errFlag = 1;
                cfg.queue_depth = atoi(optarg);
                break;

            case 'p':
                if (cfg.parallel > 0)
                    errFlag = 1;
                cfg.parallel = 1;
                break;

            case 'P':
                probe_liburing();
                exit(0);

            case 'h':
            case '?':
                print_usage();
                exit(0);

            default:
                errFlag = 1;
        }
    }

    if (cfg_path &&
        (strlen(cfg.if_name1)  > 0 ||
         strlen(cfg.if_name2)  > 0 ||
         cfg.if_addr1          > 0 ||
         cfg.if_addr2          > 0 ||
         cfg.if_nmask1         > 0 ||
         cfg.if_nmask2         > 0 ||
         cfg.rt_addr1          > 0 ||
         cfg.rt_addr2          > 0 ||
         cfg.use_uring         > 0 ||
         cfg.parallel          > 0 ||
         strlen(cfg.log_path)  > 0 ||
         cfg.log_level        != LOG_INFO))
    {
        log_msg(LOG_ERROR,
                "Error: Invalid arguments. No overload of parameters "
                "allowed when using config file.\n\n");
        errFlag = 1;
    }

    if (cfg_path && !errFlag)
    {
        FILE *cfg_fp = cfg_fopen(cfg_path);
        if (cfg_fp == NULL)
        {
            log_msg(LOG_ERROR, "Error: Unable to open config file.\n\n");
            errFlag = 1;
        }

        if (!errFlag)
        {
            if (cfg_parse(cfg_fp, &cfg) < 0)
                errFlag = 1;
            cfg_fclose(cfg_fp);
        }
    }

    if (errFlag)
    {
        print_usage();
        exit(1);
    }

    rc = setup_nat(&cfg);
    if (rc == 0)
    {
        cfg.use_uring = 3;
        /*
        if (cfg.parallel)
            handler = if_nat_parallel_liburing_handler;
        */
        if (cfg.use_uring == 0)
            handler = if_nat_io_handler;
        else if (cfg.use_uring == 1)
            handler = if_nat_iouring_handler;
        else if (cfg.use_uring == 2 || cfg.use_uring == 3)
            handler = if_nat_liburing_handler;
        else if (cfg.use_uring == 4)
            handler = if_nat_liburing_handler_single;
        else if (cfg.use_uring == 5)
            handler = if_nat_liburing_handler_sqpoll;
        else
            handler = if_nat_io_handler;

        ctx.cfg      = &cfg;
        ctx.fd1      = fd1;
        ctx.fd2      = fd2;
        ctx.stats_fd = stats_fd;
        ctx.stats    = &stats;

        handler(&ctx);
    }

    if (fd1 >= 0)
        if_net_close(fd1, cfg.if_name1);

    if (fd2 >= 0)
        if_net_close(fd2, cfg.if_name2);

    if (stats_fd >= 0)
        close(stats_fd);

    log_close_file();

    return rc;
}
