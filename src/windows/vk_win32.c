#include "vk_win32.h"
#include "../window_vtable.h"
#include "../devices/vk.h"
#include "../config.h"
#include "win32.h"
#include <vulkan/vulkan_win32.h>
#include <string.h>
#include <stdlib.h>

typedef struct gfx_vk_win32_window_s
{
	gfx_window_t window;
	gfx_win32_window_t win32;
} gfx_vk_win32_window_t;

#define WIN32_WINDOW (&VK_WIN32_WINDOW->win32)
#define VK_WIN32_WINDOW ((gfx_vk_win32_window_t*)window)

static bool vk_win32_ctr(gfx_window_t *window, gfx_window_properties_t *properties)
{
	return gfx_window_vtable.ctr(window, properties);
}

static void vk_win32_dtr(gfx_window_t *window)
{
	gfx_win32_dtr(WIN32_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool vk_win32_create_device(gfx_window_t *window)
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
					"VK_KHR_win32_surface"
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
					GFX_ERROR_CALLBACK("can't create vulkan instance");
					return false;
				}
			}
			{
				VkWin32SurfaceCreateInfoKHR create_info;
				create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
				create_info.pNext = NULL;
				create_info.flags = 0;
				create_info.hinstance = WIN32_WINDOW->hinstance;
				create_info.hwnd = WIN32_WINDOW->window;
				result = vkCreateWin32SurfaceKHR(instance, &create_info, NULL, &surface);
				if (result != VK_SUCCESS)
				{
					GFX_ERROR_CALLBACK("can't create vulkan surface");
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

static void vk_win32_show(gfx_window_t *window)
{
	gfx_win32_show(WIN32_WINDOW);
}

static void vk_win32_hide(gfx_window_t *window)
{
	gfx_win32_hide(WIN32_WINDOW);
}

static void vk_win32_poll_events(gfx_window_t *window)
{
	gfx_win32_poll_events(WIN32_WINDOW);
}

static void vk_win32_wait_events(gfx_window_t *window)
{
	gfx_win32_wait_events(WIN32_WINDOW);
}

static void vk_win32_set_swap_interval(gfx_window_t *window, int interval)
{
	gfx_vk_set_swap_interval(window->device, interval);
}

static void vk_win32_swap_buffers(gfx_window_t *window)
{
	gfx_vk_swap_buffers(window->device);
}

static void vk_win32_make_current(gfx_window_t *window)
{
}

static void vk_win32_set_title(gfx_window_t *window, const char *title)
{
	gfx_win32_set_title(WIN32_WINDOW, title);
}

static void vk_win32_set_icon(gfx_window_t *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_win32_set_icon(WIN32_WINDOW, data, width, height);
}

static void vk_win32_resize(gfx_window_t *window, uint32_t width, uint32_t height)
{
	gfx_win32_resize(WIN32_WINDOW, width, height);
}

static void vk_win32_grab_cursor(gfx_window_t *window)
{
	gfx_win32_grab_cursor(WIN32_WINDOW);
}

static void vk_win32_ungrab_cursor(gfx_window_t *window)
{
	gfx_win32_ungrab_cursor(WIN32_WINDOW);
}

static char *vk_win32_get_clipboard(gfx_window_t *window)
{
	return gfx_win32_get_clipboard(WIN32_WINDOW);
}

static void vk_win32_set_clipboard(gfx_window_t *window, const char *text)
{
	gfx_win32_set_clipboard(WIN32_WINDOW, text);
}

static gfx_cursor_t vk_win32_create_native_cursor(gfx_window_t *window, enum gfx_native_cursor native_cursor)
{
	return gfx_win32_create_native_cursor(WIN32_WINDOW, native_cursor);
}

static gfx_cursor_t vk_win32_create_cursor(gfx_window_t *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_win32_create_cursor(WIN32_WINDOW, data, width, height, xhot, yhot);
}

static void vk_win32_delete_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	gfx_win32_delete_cursor(WIN32_WINDOW, cursor);
}

static void vk_win32_set_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	gfx_win32_set_cursor(WIN32_WINDOW, cursor);
}

static void vk_win32_set_mouse_position(gfx_window_t *window, int32_t x, int32_t y)
{
	gfx_win32_set_mouse_position(WIN32_WINDOW, x, y);
}

static const gfx_window_vtable_t vk_win32_vtable =
{
	GFX_WINDOW_VTABLE_DEF(vk_win32)
};

gfx_window_t *gfx_vk_win32_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties)
{
	XVisualInfo *vi;
	gfx_window_t *window = GFX_MALLOC(sizeof(gfx_vk_win32_window_t));
	if (!window)
	{
		GFX_ERROR_CALLBACK("allocation failed");
		return NULL;
	}
	memset(window, 0, sizeof(gfx_vk_win32_window_t));
	window->vtable = &vk_win32_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	gfx_win32_ctr(WIN32_WINDOW, window);
	if (!gfx_win32_create_window(WIN32_WINDOW, title, width, height))
	{
		GFX_ERROR_CALLBACK("failed to create window");
		goto err;
	}
	return window;

err:
	window->vtable->dtr(window);
	GFX_FREE(window);
	return NULL;
}
