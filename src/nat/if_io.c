#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>

#include "log.h"
#include "cfg.h"
#include "sock.h"
#include "if_ip.h"
#include "if_poll.h"
#include "if_stats.h"

#include "if_nat.h"

/* Standard I/O read and write */

int if_nat_io_setup(CTX *ctx)
{
    int rc;

    set_cpu_affinity(ctx->cfg->cpu_affinity);
    set_cpu_affinity(0);

    rc = fcntl(ctx->fd1, F_GETFL, 0);
    if (rc < 0)
    {
        log_msg(LOG_ERROR, "fcntl F_GETFL failed");
        return rc;
    }

    rc = fcntl(ctx->fd1, F_SETFL, rc | O_NONBLOCK);
    if (rc < 0)
    {
        log_msg(LOG_ERROR, "fcntl F_SETFL failed");
        return rc;
    }

    rc = register_epoll_routine(ctx);
    if (rc != EPOLL_SUCCESS)
    {
        log_msg(LOG_ERROR, "register_poll_routine failed");
        return rc;
    }

    return rc;
}

int if_nat_io_handler(CTX *ctx)
{
    int rc          = 0;
    int i           = 0;
    int r_len       = 0;
    U32BIT src      = 0;
    U32BIT dst      = 0;

    if_stats *stats = ctx->stats;

    char data[2048];
    IP_HEAD *ip = (IP_HEAD *)data;

    rc = if_nat_io_setup(ctx);
    if (rc != 0)
        return rc;

    log_msg(LOG_INFO, "Running io... Press Ctrl-C to stop.");
    while (1)
    {
        rc = enter_epoll_routine(ctx);
        if (rc == EPOLL_EXIT) break;

        if (rc == EPOLL_ERROR) continue;

        if (rc == EPOLL_SUCCESS)
        {
            for (i = 0; i < ctx->cfg->poll_trigger; i++)
            {
                if (stats->capture)
                {
                    stats->reads_cnt++;
                    stats->syscall_cnt++;
                }

                rc = read(ctx->fd1, data, sizeof(data));
                if (rc <= 0)
                {
                    if (errno = EINTR || errno == EAGAIN)
                        break;

                    log_msg(LOG_ERROR, "Read error rc=%d (errno=%d)", rc, errno);
                    break;
                }

                r_len = rc;

                if (stats->capture)
                    stats->reads_success++;

                if (addr_config_check(ctx->cfg, ip, &src, &dst) != 0)
                {
                    log_msg(LOG_ERROR, "addr_config_check failed");
                    break;
                }

                ip_replace_addr(ip, src, dst);

                if (stats->capture)
                {
                    stats->writes_cnt++;
                    stats->syscall_cnt++;
                }

                rc = write(ctx->fd1, data, r_len);
                if (rc == 0)
                    break;

                if (rc < 0)
                {
                    log_msg(LOG_ERROR, "Write error (errno=%d)", errno);
                    break;
                }

                if (stats->capture)
                    stats->writes_success++;
            }
        }
    }
    return rc;
}