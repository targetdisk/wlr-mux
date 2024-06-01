#ifndef WAYLAND_H
#define WAYLAND_H

#include <context.h>
#include <event.h>
#include <wayland-client-protocol.h>
#include <wlr-screencopy-unstable-v1-client-protocol.h>

struct ctx;

typedef struct output_list_node {
    struct output_list_node * next;
    struct ctx * ctx;
    char * name;
    struct wl_output * output;
    struct zxdg_output_v1 * xdg_output;
    uint32_t output_id;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    int32_t scale;
    enum wl_output_transform transform;
} output_list_node_t;

void init_wl(struct ctx * ctx);

typedef struct ctx_wl {
    struct wl_display * display;
    struct wl_registry * registry;

    // registry objects
    struct wl_compositor * compositor;
    struct wl_seat * seat;
    // registry ids
    uint32_t compositor_id;
    uint32_t seat_id;
    uint32_t viewporter_id;
    uint32_t fractional_scale_manager_id;
    uint32_t wm_base_id;
    uint32_t output_manager_id;

    // screencopy backend objects
    struct wl_shm * shm;
    struct zwlr_screencopy_manager_v1 * screencopy_manager;
    uint32_t shm_id;
    uint32_t screencopy_manager_id;

    // buffer size
    output_list_node_t * current_output;
    uint32_t width;
    uint32_t height;
    double scale;

    // event handler
    event_handler_t event_handler;

    // state flags
    uint32_t last_surface_serial;
    bool xdg_surface_configured;
    bool xdg_toplevel_configured;
    bool configured;
    bool closing;
    bool initialized;
} ctx_wl_t;

#endif /* WAYLAND_H */
