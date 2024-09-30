#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include <stddef.h>

#define MAX_LOG_PATH_LEN  1024
#define MAX_LOG_LEVEL_LEN 16

typedef enum
{
    LOG_ERROR,
    LOG_INFO,
    LOG_WARNING,
    LOG_DEBUG,
    LOG_TRACE,
} LogLevel;

LogLevel  log_set_level (LogLevel level);
void      log_set_file  (char *filename);
void      log_close_file();

LogLevel  log_str2lvl  (char *src);
char     *log_lvl2str  (LogLevel level, int *err);

void      log_msg      (LogLevel level, char *format, ...);
void      log_hexdump  (void *data, size_t len);

#endif
