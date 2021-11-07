#ifndef GFX_D3D_WINDOW_H
# define GFX_D3D_WINDOW_H

# ifdef __cplusplus
extern "C" {
# endif

# include "../window.h"

gfx_window_t *gfx_d3d_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties);

# ifdef __cplusplus
}
# endif

#endif
