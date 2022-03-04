#ifndef GFX_VK_X11_WINDOW_H
#define GFX_VK_X11_WINDOW_H

#include "../window.h"

gfx_window_t *gfx_vk_x11_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties);

#endif
