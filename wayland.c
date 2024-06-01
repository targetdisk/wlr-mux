#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

#include <context.h>
#include <event.h>
#include <log.h>
#include <wayland.h>
#include <wlr-screencopy-unstable-v1-client-protocol.h>

static void on_loop_each(ctx_t * ctx);
static void on_loop_event(ctx_t * ctx);
static void on_registry_add(
    void * data, struct wl_registry * registry,
    uint32_t id, const char * interface, uint32_t version
);
static void on_registry_remove(
    void * data, struct wl_registry * registry,
    uint32_t id
);

static const struct wl_registry_listener registry_listener = {
    .global = on_registry_add,
    .global_remove = on_registry_remove
};

void init_wl(ctx_t * ctx) {
    // initialize context structure
    ctx->wl.display = NULL;
    ctx->wl.registry = NULL;

    ctx->wl.compositor = NULL;
    ctx->wl.compositor_id = 0;
    ctx->wl.seat = NULL;
    ctx->wl.seat_id = 0;
    ctx->wl.viewporter_id = 0;
    ctx->wl.fractional_scale_manager_id = 0;
    ctx->wl.wm_base_id = 0;
    ctx->wl.output_manager_id = 0;

    ctx->wl.shm = NULL;
    ctx->wl.shm_id = 0;
    ctx->wl.screencopy_manager = NULL;
    ctx->wl.screencopy_manager_id = 0;

    ctx->wl.current_output = NULL;
    ctx->wl.width = 0;
    ctx->wl.height = 0;
    ctx->wl.scale = 1.0;

    ctx->wl.event_handler.fd = -1;
    ctx->wl.event_handler.events = EPOLLIN;
    ctx->wl.event_handler.on_event = on_loop_event;
    ctx->wl.event_handler.on_each = on_loop_each;

    ctx->wl.last_surface_serial = 0;
    ctx->wl.xdg_surface_configured = false;
    ctx->wl.xdg_toplevel_configured = false;
    ctx->wl.configured = false;
    ctx->wl.closing = false;
    ctx->wl.initialized = true;

    // connect to display
    ctx->wl.display = wl_display_connect(NULL);
    if (ctx->wl.display == NULL) {
        log_error("wayland::init(): failed to connect to wayland\n");
        exit(1);
    }

    // register event loop
    ctx->wl.event_handler.fd = wl_display_get_fd(ctx->wl.display);
    event_add_fd(ctx, &ctx->wl.event_handler);

    // get registry handle
    ctx->wl.registry = wl_display_get_registry(ctx->wl.display);
    if (ctx->wl.registry == NULL) {
        log_error("wayland::init(): failed to get registry handle\n");
        exit(1);
    }

    // add registry event listener
    // - for add global event
    // - for remove global event
    wl_registry_add_listener(ctx->wl.registry, &registry_listener, (void *)ctx);

    // wait for registry events
    // - expecting add global events for all required protocols
    // - expecting add global events for all outputs
    wl_display_roundtrip(ctx->wl.display);

    // check for missing required protocols
    if (ctx->wl.compositor == NULL) {
        log_error("wayland::init(): compositor missing\n");
        //exit(1);
        exit(1);
    }
}

static void on_loop_each(ctx_t * ctx) {
    wl_display_flush(ctx->wl.display);
}

static void on_loop_event(ctx_t * ctx) {
    if (wl_display_dispatch(ctx->wl.display) == -1) {
        ctx->wl.closing = true;
    }
}

static void on_registry_add(
    void * data, struct wl_registry * registry,
    uint32_t id, const char * interface, uint32_t version
) {
    ctx_t * ctx = (ctx_t *)data;

    log_debug(ctx, "wayland::on_registry_add(): %s (version = %d, id = %d)\n", interface, version, id);

    // bind proxy object for each protocol we need
    // bind proxy object for each output
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        if (ctx->wl.compositor != NULL) {
            log_error("wayland::on_registry_add(): duplicate compositor\n");
            exit(1);
        }

        // bind compositor object
        ctx->wl.compositor = (struct wl_compositor *)wl_registry_bind(
            registry, id, &wl_compositor_interface, 4
        );
        ctx->wl.compositor_id = id;
    } else if (strcmp(interface, zwlr_screencopy_manager_v1_interface.name) == 0) {
        if (ctx->wl.screencopy_manager != NULL) {
            log_error("wayland::on_registry_add(): duplicate screencopy_manager\n");
            exit(1);
        }

        // bind screencopy manager object
        // - for mirror-screencopy backend
        ctx->wl.screencopy_manager = (struct zwlr_screencopy_manager_v1 *)wl_registry_bind(
            registry, id, &zwlr_screencopy_manager_v1_interface, 3
        );
        ctx->wl.screencopy_manager_id = id;
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        if (ctx->wl.shm != NULL) {
            log_error("wayland::on_registry_add(): duplicate shm\n");
            exit(1);
        }

        // bind shm object
        // - for mirror-screencopy backend
        ctx->wl.shm = (struct wl_shm *)wl_registry_bind(
            registry, id, &wl_shm_interface, 1
        );
        ctx->wl.shm_id = id;
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        // allocate output node
        output_list_node_t * node = malloc(sizeof (output_list_node_t));
        if (node == NULL) {
            log_error("wayland::on_registry_add(): failed to allocate output node\n");
            exit(1);
        }
    }

    (void)version;
}

static void on_registry_remove(
    void * data, struct wl_registry * registry,
    uint32_t id
) {
    ctx_t * ctx = (ctx_t *)data;

    // ensure protocols we need aren't removed
    // remove removed outputs from the output list
    if (id == ctx->wl.compositor_id) {
        log_error("wayland::on_registry_remove(): compositor disappeared\n");
        exit(1);
    } else if (id == ctx->wl.seat_id) {
        log_error("wayland::on_registry_remove(): seat disappeared\n");
        exit(1);
    } else if (id == ctx->wl.viewporter_id) {
        log_error("wayland::on_registry_remove(): viewporter disappeared\n");
        exit(1);
    } else if (id == ctx->wl.fractional_scale_manager_id) {
        log_error("wayland::on_registry_remove(): fractional_scale_manager disappeared\n");
        exit(1);
    } else if (id == ctx->wl.wm_base_id) {
        log_error("wayland::on_registry_remove(): wm_base disappeared\n");
        exit(1);
    } else if (id == ctx->wl.output_manager_id) {
        log_error("wayland::on_registry_remove(): output_manager disappeared\n");
        exit(1);
    }

    (void)registry;
}
