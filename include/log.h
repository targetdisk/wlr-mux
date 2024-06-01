#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#ifdef DEBUG
#  define log_debug(ctx, fmt, ...) fprintf(stderr, "debug: " fmt, ##__VA_ARGS__)
#else
#  define log_debug(ctx, fmt, ...)
#endif
#define log_warn(fmt, ...) fprintf(stderr, "warning: " fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) fprintf(stderr, "error: " fmt, ##__VA_ARGS__)

#endif /* LOG_H */
