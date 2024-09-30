#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "cfg.h"
#include "list.h"
#include "sock.h"

#define COMMENT  '#'
#define VALDELIM '='
#define SECSTART '['
#define SECEND   ']'
#define MAXLINE  8192

List *ifnet_list    = NULL;
List *route_list    = NULL;
List *mode_list     = NULL;
List *liburing_list = NULL;
List *logging_list  = NULL;
List *stats_list    = NULL;

FILE *cfg_fopen(char *filename)
{
    FILE *fp = NULL;

    if (filename && strlen(filename) > 0)
        fp = fopen(filename, "r");

    return fp;
}

int cfg_fclose(FILE *fp)
{
    if (fp)
        fclose(fp);

    return 0;
}

static int parse_ifnet(Config *cfg)
{
    char *line;
    char *key;
    char *val;

    while (ifnet_list->head != NULL)
    {
        line = ifnet_list->head->data;
        key  = strtok(line, "=");
        val  = strtok(NULL, "=");

        if (strcmp(key, "name1") == 0)
            strcpy(cfg->if_name1, val);
        else if (strcmp(key, "name2") == 0)
            strcpy(cfg->if_name2, val);
        else if (strcmp(key, "addr1") == 0)
            cidrstr_to_addr_nmask(val, &cfg->if_addr1, &cfg->if_nmask1);
        else if (strcmp(key, "addr2") == 0)
            cidrstr_to_addr_nmask(val, &cfg->if_addr1, &cfg->if_nmask1);
        else if (strcmp(key, "txqlen") == 0)
            cfg->if_txqlen = atoi(val);
        else
            log_msg(LOG_WARNING, "Unknown configuration key: %s", key);

        list_del(ifnet_list, line);
    }

    return 0;
}

static int parse_route(Config *cfg)
{
    char *line;
    char *key;
    char *val;

    while (route_list->head != NULL)
    {
        line = route_list->head->data;
        key  = strtok(line, "=");
        val  = strtok(NULL, "=");

        if (strcmp(key, "route1") == 0)
            str_to_haddr(val, &cfg->rt_addr1);
        else if (strcmp(key, "route2") == 0)
            str_to_haddr(val, &cfg->rt_addr2);
        else if (strcmp(key, "route1_nmask") == 0)
            str_to_haddr(val, &cfg->rt_nmask1);
        else if (strcmp(key, "route2_nmask") == 0)
            str_to_haddr(val, &cfg->rt_nmask2);
        else
            log_msg(LOG_WARNING, "Unknown configuration key: %s", key);

        list_del(route_list, line);
    }

    return 0;
}

static int parse_mode(Config *cfg)
{
    char *line;
    char *key;
    char *val;

    while (mode_list->head != NULL)
    {
        line = mode_list->head->data;
        key  = strtok(line, "=");
        val  = strtok(NULL, "=");

        if (strcmp(key, "mode") == 0)
            cfg->use_uring = atoi(val);
        else if (strcmp(key, "polltimeout") == 0)
            cfg->poll_timeout = atoi(val);
        else if (strcmp(key, "polltrigger") == 0)
            cfg->poll_trigger = atoi(val);
        else if (strcmp(key, "cpu_affinity") == 0)
            cfg->cpu_affinity = atoi(val);
        else
            log_msg(LOG_WARNING, "Unknown configuration key: %s", key);

        if (cfg->use_uring == 3)
            cfg->parallel = 1;

        list_del(mode_list, line);
    }

    return 0;
}

static int parse_liburing(Config *cfg)
{
    char *line;
    char *key;
    char *val;

    while (liburing_list->head != NULL)
    {
        line = liburing_list->head->data;
        key  = strtok(line, "=");
        val  = strtok(NULL, "=");

        if (strcmp(key, "qdepth") == 0)
            cfg->queue_depth = atoi(val);
        else if (strcmp(key, "bufferSize") == 0)
            cfg->buffer_size = atoi(val);
        else
            log_msg(LOG_WARNING, "Unknown configuration key: %s", key);

        list_del(liburing_list, line);
    }

    return 0;
}

static int parse_logging(Config *cfg)
{
    char *line;
    char *key;
    char *val;

    while (logging_list->head != NULL)
    {
        line = logging_list->head->data;
        key  = strtok(line, "=");
        val  = strtok(NULL, "=");

        if (strcmp(key, "loglevel") == 0)
            cfg->log_level = log_str2lvl(val);
        else if (strcmp(key, "logfile") == 0)
            strncpy(cfg->log_path, val, sizeof(cfg->log_path));
        else
            log_msg(LOG_WARNING, "Unknown configuration key: %s", key);

        list_del(logging_list, line);
    }

    return 0;
}

static int parse_stats(Config *cfg)
{
    char *line;
    char *key;
    char *val;

    while (stats_list->head != NULL)
    {
        line = stats_list->head->data;
        key  = strtok(line, "=");
        val  = strtok(NULL, "=");

        if (strcmp(key, "statsfile") == 0)
            strncpy(cfg->stats_path, val, sizeof(cfg->stats_path));
        else if (strcmp(key, "statsname") == 0)
            strncpy(cfg->stats_name, val, sizeof(cfg->stats_name));
        else
            log_msg(LOG_WARNING, "Unknown configuration key: %s", key);

        list_del(stats_list, line);
    }

    return 0;
}

int cfg_parse(FILE *fp, Config *cfg)
{
    int   len;
    char  buffer[MAXLINE];
    List *list_ptr = NULL;

    if (!fp || !cfg)
        return -1;

    list_init(&ifnet_list);
    list_init(&route_list);
    list_init(&mode_list);
    list_init(&liburing_list);
    list_init(&logging_list);
    list_init(&stats_list);

    while (fgets(buffer, sizeof(buffer), fp))
    {
        if (buffer[0] == '\n' || buffer[0] == '\r')
            continue;

        if (buffer[0] == COMMENT)
            continue;

        len = strlen(buffer);
        while (len > 0)
        {
            if (buffer[len - 1] == '\n' || buffer[len - 1] == '\r' || buffer[len - 1] == ' ')
                buffer[--len] = '\0';
            else
                break;
        }

        if (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == ' '))
            buffer[len - 1] = '\0';


        if (buffer[0] == SECSTART)
        {
            if (strcmp(buffer, "[Network Interface]") == 0)
                list_ptr = ifnet_list;
            else if (strcmp(buffer, "[Route]") == 0)
                list_ptr = route_list;
            else if (strcmp(buffer, "[Mode]") == 0)
                list_ptr = mode_list;
            else if (strcmp(buffer, "[Liburing]") == 0)
                list_ptr = liburing_list;
            else if (strcmp(buffer, "[Logging]") == 0)
                list_ptr = logging_list;
            else if (strcmp(buffer, "[Stats]") == 0)
                list_ptr = stats_list;
            else
                list_ptr = NULL;

            continue;
        }

        if (list_ptr)
            list_add(list_ptr, strdup(buffer));
    }

    if (ifnet_list->size > 0)
        parse_ifnet(cfg);

    if (route_list->size > 0)
        parse_route(cfg);

    if (mode_list->size > 0)
        parse_mode(cfg);

    if (liburing_list->size > 0)
        parse_liburing(cfg);

    if (logging_list->size > 0)
        parse_logging(cfg);

    if (stats_list->size > 0)
        parse_stats(cfg);

    list_destroy(&ifnet_list);
    list_destroy(&route_list);
    list_destroy(&mode_list);
    list_destroy(&liburing_list);
    list_destroy(&logging_list);
    list_destroy(&stats_list);

    return 0;
}

