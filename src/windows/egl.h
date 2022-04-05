#ifndef GFX_EGL_WINDOW_H
#define GFX_EGL_WINDOW_H

#include "gl.h"

gfx_window_t *gfx_egl_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties);

void *gfx_egl_get_proc_address(const char *name);

#endif
