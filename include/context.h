#ifndef CONTEXT_H
#define CONTEXT_H

#include <wayland.h>

typedef struct ctx {
    ctx_event_t event;
    ctx_wl_t wl;
} ctx_t;

#endif /* CONTEXT_H */
