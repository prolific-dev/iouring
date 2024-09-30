#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "log.h"

static FILE     *log_file = NULL;
static LogLevel  global   = LOG_DEBUG;

LogLevel log_set_level(LogLevel level)
{
    LogLevel old = global;
    global = level;
    return old;
}

void log_set_file(char *filename)
{
    if (log_file && log_file != stdout)
        fclose(log_file);

    if (filename != NULL)
    {
        log_file = fopen(filename, "a");
        if (log_file == NULL)
            log_file = stdout;
    }
    else
    {
        log_file = stdout;
    }
}

void log_close_file()
{
    if (log_file && log_file != stdout)
        fclose(log_file);
}

char *log_lvl2str(LogLevel level, int *err)
{
    if (err)
        *err = 0;

    switch (level)
    {
        case LOG_ERROR:   return "ERROR";
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARN";
        case LOG_DEBUG:   return "DEBUG";
        case LOG_TRACE:   return "TRACE";
    }
    if (err)
        *err = 1;
    return "UNKNOWN";
}

LogLevel log_str2lvl(char *src)
{
    LogLevel lvl;

    if (strcmp(src, "ERROR") == 0)
        lvl = LOG_ERROR;
    else if (strcmp(src, "INFO") == 0)
        lvl = LOG_INFO;
    else if (strcmp(src, "WARN") == 0)
        lvl = LOG_WARNING;
    else if (strcmp(src, "DEBUG") == 0)
        lvl = LOG_DEBUG;
    else if (strcmp(src, "TRACE") == 0)
        lvl = LOG_TRACE;
    else
        lvl = LOG_DEBUG;

    return lvl;
}

void log_msg(LogLevel level, char *format, ...)
{
    if (level > global)
    {
        return;
    }
    else
    {
        time_t  now;
        char    time_str[20];
        va_list args;

        now = time(NULL);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

        va_start(args, format);
        fprintf (log_file ? log_file : stdout, "[%s] [pid %d] %s: ", time_str, getpid(), log_lvl2str(level, NULL));
        vfprintf(log_file ? log_file : stdout, format, args);
        fprintf (log_file ? log_file : stdout, "\n");
        fflush(log_file);
        va_end(args);
    }
}

void log_hexdump(void *data, size_t size)
{
    size_t i;
    unsigned char cnt  = (unsigned char )0;
    unsigned char *byte = (unsigned char *)data;

    printf("\n");
    for (i = 0; i < size; i++)
    {
        if ((i + 1) % 16 == 16)
        {
            printf("%02x ~ ", cnt);
            cnt += 16;
        }

        printf("%02x ", byte[i]);

        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}