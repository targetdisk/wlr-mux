#include <stdlib.h>

#include <context.h>
#include <log.h>
#include <event.h>

static void add_handler(ctx_t * ctx, event_handler_t * handler) {
    handler->next = ctx->event.handlers;
    ctx->event.handlers = handler;
}

void event_add_fd(ctx_t * ctx, event_handler_t * handler) {
    struct epoll_event event;
    event.events = handler->events;
    event.data.ptr = handler;

    if (epoll_ctl(ctx->event.pollfd, EPOLL_CTL_ADD, handler->fd, &event) == -1) {
        log_error("event::add_fd(): failed to add fd to epoll instance\n");
        exit(1);
    }

    add_handler(ctx, handler);
}
