#pragma once
#include <stdio.h>
#include <stdlib.h>

typedef const enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_FATAL
} LOG_LEVEL;

void log_errno(char *msg);

void set_log_level(LOG_LEVEL l);

void log_msg_debug(char *msg);

void log_msg_info(char *msg);

void log_msg_warn(char *msg);

void log_msg_fatal(char *msg);

void log_printf_debug(char *f, ...);

void log_printf_info(char *f, ...);

void log_printf_warn(char *f, ...);

void log_printf_fatal(char *f, ...);