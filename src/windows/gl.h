#ifndef GFX_GL_WINDOW_H
#define GFX_GL_WINDOW_H

#include "../window.h"

typedef void *(gfx_gl_load_addr_t)(const char *name);

typedef struct gfx_gl_window_s
{
	gfx_window_t window;
} gfx_gl_window_t;

bool gfx_gl_create_device(gfx_window_t *window, gfx_gl_load_addr_t *load_addr);

#endif
