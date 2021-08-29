#ifndef GFX_GLFW_WINDOW_H
# define GFX_GLFW_WINDOW_H

# ifdef __cplusplus
extern "C" {
# endif

# include "./gl.h"

gfx_window_t *gfx_glfw_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties, gfx_window_t *shared_context);

void *gfx_glfw_get_proc_address(const char *name);

# ifdef __cplusplus
}
# endif

#endif