#ifndef GFX_VK_DEVICE_H
#define GFX_VK_DEVICE_H

#include "../device.h"
#include <vulkan/vulkan.h>

gfx_device_t *gfx_vk_device_new(gfx_window_t *window, VkInstance instance, VkSurfaceKHR surface);

void gfx_vk_set_swap_interval(gfx_device_t *device, int interval);
void gfx_vk_swap_buffers(gfx_device_t *device);

#endif
