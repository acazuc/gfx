#ifndef GFX_WGL_WINDOW_H
# define GFX_WGL_WINDOW_H

# ifdef __cplusplus
extern "C" {
# endif

# include "./gl.h"

gfx_window_t *gfx_wgl_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties);

void *gfx_wgl_get_proc_address(const char *name);

# ifdef __cplusplus
}
# endif

#endif
