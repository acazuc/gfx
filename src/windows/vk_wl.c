#include "vk_wl.h"
#include "../window_vtable.h"
#include "../devices/vk.h"
#include "../config.h"
#include "wl.h"
#include <vulkan/vulkan_wayland.h>
#include <string.h>
#include <stdlib.h>

typedef struct gfx_vk_wl_window_s
{
	gfx_window_t window;
	gfx_wl_window_t wl;
} gfx_vk_wl_window_t;

#define WL_WINDOW (&VK_WL_WINDOW->wl)
#define VK_WL_WINDOW ((gfx_vk_wl_window_t*)window)

static bool vk_wl_ctr(gfx_window_t *window, gfx_window_properties_t *properties)
{
	if (!gfx_wl_ctr(WL_WINDOW, window))
		return false;
	return gfx_window_vtable.ctr(window, properties);
}

static void vk_wl_dtr(gfx_window_t *window)
{
	gfx_wl_dtr(WL_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool vk_wl_create_device(gfx_window_t *window)
{
	switch (window->properties.device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_VK
		case GFX_DEVICE_VK:
		{
			VkResult result;
			VkInstance instance;
			VkSurfaceKHR surface;
			{
				VkInstanceCreateInfo create_info;
				static const char *extensions[] =
				{
					"VK_KHR_surface",
					"VK_KHR_wayland_surface"
				};
				create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				create_info.pNext = NULL;
				create_info.flags = 0;
				create_info.pApplicationInfo = NULL;
				const char *layers[] =
				{
				};
				create_info.enabledLayerCount = sizeof(layers) / sizeof(*layers);
				create_info.ppEnabledLayerNames = layers;
				create_info.enabledExtensionCount = sizeof(extensions) / sizeof(*extensions);
				create_info.ppEnabledExtensionNames = extensions;
				result = vkCreateInstance(&create_info, NULL, &instance);
				if (result != VK_SUCCESS)
				{
					GFX_ERROR_CALLBACK("can't create wayland vulkan instance");
					return false;
				}
			}
			{
				VkWaylandSurfaceCreateInfoKHR create_info;
				create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
				create_info.pNext = NULL;
				create_info.flags = 0;
				create_info.display = WL_WINDOW->display;
				create_info.surface = WL_WINDOW->surface;
				result = vkCreateWaylandSurfaceKHR(instance, &create_info, NULL, &surface);
				if (result != VK_SUCCESS)
				{
					GFX_ERROR_CALLBACK("can't create wayland vulkan surface");
					vkDestroyInstance(instance, NULL);
					return false;
				}
			}
			window->device = gfx_vk_device_new(window, instance, surface);
			return true;
		}
#endif
		default:
			break;
	}
	return false;
}

static void vk_wl_show(gfx_window_t *window)
{
	gfx_wl_show(WL_WINDOW);
}

static void vk_wl_hide(gfx_window_t *window)
{
	gfx_wl_hide(WL_WINDOW);
}

static void vk_wl_poll_events(gfx_window_t *window)
{
	gfx_wl_poll_events(WL_WINDOW);
}

static void vk_wl_wait_events(gfx_window_t *window)
{
	gfx_wl_wait_events(WL_WINDOW);
}

static void vk_wl_set_swap_interval(gfx_window_t *window, int interval)
{
	gfx_vk_set_swap_interval(window->device, interval);
}

static void vk_wl_swap_buffers(gfx_window_t *window)
{
	gfx_vk_swap_buffers(window->device);
}

static void vk_wl_make_current(gfx_window_t *window)
{
}

static void vk_wl_set_title(gfx_window_t *window, const char *title)
{
	gfx_wl_set_title(WL_WINDOW, title);
}

static void vk_wl_set_icon(gfx_window_t *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_wl_set_icon(WL_WINDOW, data, width, height);
}

static void vk_wl_resize(gfx_window_t *window, uint32_t width, uint32_t height)
{
	gfx_wl_resize(WL_WINDOW, width, height);
}

static void vk_wl_grab_cursor(gfx_window_t *window)
{
	gfx_wl_grab_cursor(WL_WINDOW);
}

static void vk_wl_ungrab_cursor(gfx_window_t *window)
{
	gfx_wl_ungrab_cursor(WL_WINDOW);
}

static char *vk_wl_get_clipboard(gfx_window_t *window)
{
	return gfx_wl_get_clipboard(WL_WINDOW);
}

static void vk_wl_set_clipboard(gfx_window_t *window, const char *text)
{
	gfx_wl_set_clipboard(WL_WINDOW, text);
}

static gfx_cursor_t vk_wl_create_native_cursor(gfx_window_t *window, enum gfx_native_cursor native_cursor)
{
	return gfx_wl_create_native_cursor(WL_WINDOW, native_cursor);
}

static gfx_cursor_t vk_wl_create_cursor(gfx_window_t *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_wl_create_cursor(WL_WINDOW, data, width, height, xhot, yhot);
}

static void vk_wl_delete_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	gfx_wl_delete_cursor(WL_WINDOW, cursor);
}

static void vk_wl_set_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	gfx_wl_set_cursor(WL_WINDOW, cursor);
}

static void vk_wl_set_mouse_position(gfx_window_t *window, int32_t x, int32_t y)
{
	gfx_wl_set_mouse_position(WL_WINDOW, x, y);
}

static const gfx_window_vtable_t vk_wl_vtable =
{
	GFX_WINDOW_VTABLE_DEF(vk_wl)
};

gfx_window_t *gfx_vk_wl_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties)
{
	gfx_window_t *window = GFX_MALLOC(sizeof(gfx_vk_wl_window_t));
	if (!window)
	{
		GFX_ERROR_CALLBACK("allocation failed");
		return NULL;
	}
	memset(window, 0, sizeof(gfx_vk_wl_window_t));
	window->vtable = &vk_wl_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	//XXX
	return window;

err:
	window->vtable->dtr(window);
	GFX_FREE(window);
	return NULL;
}
