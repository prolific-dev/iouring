#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "if_ctx.h"
#include "if_poll.h"

int register_epoll_routine(CTX *ctx)
{
    struct epoll_event ev;

    ctx->epfd = epoll_create1(0);
    if (ctx->epfd < 0)
    {
        log_msg(LOG_ERROR, "Epoll create error %d (errno=%d)", ctx->epfd, errno);
        return EPOLL_ERROR;
    }

    ev.events  = EPOLLIN;
    ev.data.fd = ctx->fd1;

    if (epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->fd1, &ev) < 0)
    {
        log_msg(LOG_ERROR, "Epoll control error %d (errno=%d)", ctx->epfd, errno);
        return EPOLL_ERROR;
    }

    ev.events  = EPOLLIN;
    ev.data.fd = ctx->stats_fd;

    if (epoll_ctl(ctx->epfd, EPOLL_CTL_ADD, ctx->stats_fd, &ev) < 0)
    {
        log_msg(LOG_ERROR, "Epoll control error %d (errno=%d)", ctx->epfd, errno);
        return EPOLL_ERROR;
    }

    return EPOLL_SUCCESS;
}

int enter_epoll_routine(CTX *ctx)
{
    int rc, i;

    while (1)
    {
        if (getppid() == 1)
        {
            log_msg(LOG_ERROR, "Parent process died, exiting...");
            return EPOLL_EXIT;
        }

        rc = epoll_wait(ctx->epfd, ctx->events, 2, ctx->cfg->poll_timeout);
        if (rc == 0)
        {
            if (ctx->cfg->use_uring == 7)
            {
                if (ctx->events[0].data.fd == ctx->fd1)
                {
                    log_msg(LOG_ERROR, "poll1 timeout on fd1, exiting...");
                    return EPOLL_SUCCESS;
                }

                if (ctx->events[1].data.fd == ctx->fd1)
                {
                    log_msg(LOG_ERROR, "poll2 timeout on fd1, exiting...");
                    return EPOLL_SUCCESS;
                }
            }
            continue;
        }

        if (rc < 0)
        {
            if (errno == EINTR)
                return EPOLL_SUCCESS;

            if (errno == EAGAIN)
                return EPOLL_SUCCESS;

            return EPOLL_ERROR;
        }

        for (i = 0; i < rc; i++)
        {
            if (ctx->events[i].data.fd == ctx->fd1)
                return EPOLL_SUCCESS;

            if (ctx->events[i].data.fd == ctx->stats_fd)
            {
                int rv;
                char dummy_buffer[1024];

                rv = read(ctx->stats_fd, dummy_buffer, sizeof(dummy_buffer) - 1);
                if (rv > 0)
                {
                    dummy_buffer[rv] = '\0';
                    if (dummy_buffer[rv - 1] == '\n')
                        dummy_buffer[rv - 1] = '\0';

                    if (strcmp(dummy_buffer, "exit") == 0)
                        return EPOLL_EXIT;

                    if (strcmp(dummy_buffer, "start capture") == 0)
                        if_stats_start(ctx->stats, ctx->cfg);

                    if (strcmp(dummy_buffer, "stop capture") == 0)
                        if_stats_stop(ctx->stats, ctx->cfg);

                    return EPOLL_SUCCESS;
                }
            }

        }
    }

    return EPOLL_EXIT;
}
