#include "wl.h"

bool gfx_wl_create_window(gfx_wl_window_t *window, const char *title, uint32_t width, uint32_t height)
{
}

bool gfx_wl_ctr(gfx_wl_window_t *window, gfx_window_t *win_ref)
{
}

void gfx_wl_dtr(gfx_wl_window_t *window)
{
}

void gfx_wl_show(gfx_wl_window_t *window)
{
}

void gfx_wl_hide(gfx_wl_window_t *window)
{
}

void gfx_wl_poll_events(gfx_wl_window_t *window)
{
}

void gfx_wl_wait_events(gfx_wl_window_t *window)
{
}

void gfx_wl_set_title(gfx_wl_window_t *window, const char *title)
{
}

void gfx_wl_set_icon(gfx_wl_window_t *window, const void *data, uint32_t width, uint32_t height)
{
}

void gfx_wl_resize(gfx_wl_window_t *window, uint32_t width, uint32_t height)
{
}

void gfx_wl_grab_cursor(gfx_wl_window_t *window)
{
}

void gfx_wl_ungrab_cursor(gfx_wl_window_t *window)
{
}

char *gfx_wl_get_clipboard(gfx_wl_window_t *window)
{
}

void gfx_wl_set_clipboard(gfx_wl_window_t *window, const char *text)
{
}

gfx_cursor_t gfx_wl_create_native_cursor(gfx_wl_window_t *window, enum gfx_native_cursor cursor)
{
}

gfx_cursor_t gfx_wl_create_cursor(gfx_wl_window_t *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
}

void gfx_wl_delete_cursor(gfx_wl_window_t *window, gfx_cursor_t cursor)
{
}

void gfx_wl_set_cursor(gfx_wl_window_t *window, gfx_cursor_t cursor)
{
}

void gfx_wl_set_mouse_position(gfx_wl_window_t *window, int32_t x, int32_t y)
{
}
