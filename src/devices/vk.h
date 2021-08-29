#ifndef GFX_VK_DEVICE_H
# define GFX_VK_DEVICE_H

# ifdef __cplusplus
extern "C" {
# endif

# include "../device.h"

gfx_device_t *gfx_vk_device_new(gfx_window_t *window);

# ifdef __cplusplus
}
# endif

#endif
