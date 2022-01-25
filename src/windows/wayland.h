#ifndef GFX_WAYLAND_WINDOW_H
#define GFX_WAYLAND_WINDOW_H

#include "window.h"

typedef struct gfx_wayland_window_s
{
	gfx_gl_window_t gl;
} gfx_wayland_window_t;

gfx_window_t *gfx_wayland_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties);

#endif
