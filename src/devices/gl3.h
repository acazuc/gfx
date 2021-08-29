#ifndef GFX_GL3_DEVICE_H
# define GFX_GL3_DEVICE_H

# ifdef __cplusplus
extern "C" {
# endif

# include "./gl.h"

gfx_device_t *gfx_gl3_device_new(gfx_window_t *window, gfx_gl_load_addr_t *load_addr);

# ifdef __cplusplus
}
# endif

#endif
