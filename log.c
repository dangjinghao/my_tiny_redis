

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include "log.h"
static unsigned int local_level = LOG_INFO;
void log_errno(char *msg)
{
    fprintf(stderr, "%d:%s: %s\n", errno, strerror(errno), msg);
    exit(EXIT_FAILURE);
}

void set_log_level(LOG_LEVEL l)
{
    local_level = l;
}

void log_msg_debug(char *msg)
{
    if (local_level <= LOG_DEBUG)
    {
        fputs(msg, stderr);
        fputs("\n", stderr);
    }
}

void log_msg_info(char *msg)
{
    if (local_level <= LOG_INFO)
    {
        fputs(msg, stderr);
        fputs("\n", stderr);
    }
}

void log_msg_warn(char *msg)
{
    if (local_level <= LOG_WARNING)
    {
        fputs(msg, stderr);
        fputs("\n", stderr);
    }
}

void log_msg_fatal(char *msg)
{
    if (local_level <= LOG_FATAL)
    {
        fputs(msg, stderr);
        fputs("\n", stderr);
        exit(EXIT_FAILURE);
    }
}

void log_printf_debug(char *f, ...)
{
    if (local_level <= LOG_DEBUG)
    {
        va_list args;
        va_start(args, f);
        vfprintf(stderr, f, args);
        va_end(args);
    }
}

void log_printf_info(char *f, ...)
{
    if (local_level <= LOG_INFO)
    {
        va_list args;
        va_start(args, f);
        vfprintf(stderr, f, args);
        va_end(args);
    }
}
void log_printf_warn(char *f, ...)
{
    if (local_level <= LOG_WARNING)
    {
        va_list args;
        va_start(args, f);
        vfprintf(stderr, f, args);
        va_end(args);
    }
}
void log_printf_fatal(char *f, ...)
{
    if (local_level <= LOG_FATAL)
    {
        va_list args;
        va_start(args, f);
        vfprintf(stderr, f, args);
        va_end(args);
    }
}