#include "vk_x11.h"
#include "../window_vtable.h"
#include "../devices/vk.h"
#include "../config.h"
#include "x11.h"
#include <vulkan/vulkan_xlib.h>
#include <string.h>
#include <stdlib.h>

extern gfx_window_vtable_t gfx_window_vtable;

typedef struct gfx_vk_x11_window_s
{
	gfx_window_t window;
	gfx_x11_window_t x11;
} gfx_vk_x11_window_t;

#ifdef GLX_WINDOW
# undef GLX_WINDOW
#endif

#define X11_WINDOW (&VK_X11_WINDOW->x11)
#define VK_X11_WINDOW ((gfx_vk_x11_window_t*)window)

static bool vk_x11_ctr(gfx_window_t *window, gfx_window_properties_t *properties)
{
	if (!gfx_x11_ctr(X11_WINDOW, window))
		return false;
	return gfx_window_vtable.ctr(window, properties);
}

static void vk_x11_dtr(gfx_window_t *window)
{
	gfx_x11_dtr(X11_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool vk_x11_create_device(gfx_window_t *window)
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
					"VK_KHR_xlib_surface"
				};
				create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				create_info.pNext = NULL;
				create_info.flags = 0;
				create_info.pApplicationInfo = NULL;
				create_info.enabledLayerCount = 0;
				create_info.ppEnabledLayerNames = NULL;
				create_info.enabledExtensionCount = sizeof(extensions) / sizeof(*extensions);
				create_info.ppEnabledExtensionNames = extensions;
				result = vkCreateInstance(&create_info, NULL, &instance);
				if (result != VK_SUCCESS)
				{
					if (gfx_error_callback)
						gfx_error_callback("can't create vulkan instance");
					return false;
				}
			}
			{
				VkXlibSurfaceCreateInfoKHR create_info;
				create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
				create_info.pNext = NULL;
				create_info.flags = 0;
				create_info.dpy = X11_WINDOW->display;
				create_info.window = X11_WINDOW->window;
				result = vkCreateXlibSurfaceKHR(instance, &create_info, NULL, &surface);
				if (result != VK_SUCCESS)
				{
					if (gfx_error_callback)
						gfx_error_callback("can't create vulkan surface");
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

static void vk_x11_show(gfx_window_t *window)
{
	gfx_x11_show(X11_WINDOW);
}

static void vk_x11_hide(gfx_window_t *window)
{
	gfx_x11_hide(X11_WINDOW);
}

static void vk_x11_poll_events(gfx_window_t *window)
{
	gfx_x11_poll_events(X11_WINDOW);
}

static void vk_x11_wait_events(gfx_window_t *window)
{
	gfx_x11_wait_events(X11_WINDOW);
}

static void vk_x11_set_swap_interval(gfx_window_t *window, int interval)
{
	gfx_vk_set_swap_interval(window->device, interval);
}

static void vk_x11_swap_buffers(gfx_window_t *window)
{
	gfx_vk_swap_buffers(window->device);
}

static void vk_x11_make_current(gfx_window_t *window)
{
}

static void vk_x11_set_title(gfx_window_t *window, const char *title)
{
	gfx_x11_set_title(X11_WINDOW, title);
}

static void vk_x11_set_icon(gfx_window_t *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_x11_set_icon(X11_WINDOW, data, width, height);
}

static void vk_x11_resize(gfx_window_t *window, uint32_t width, uint32_t height)
{
	gfx_x11_resize(X11_WINDOW, width, height);
}

static void vk_x11_grab_cursor(gfx_window_t *window)
{
	gfx_x11_grab_cursor(X11_WINDOW);
}

static void vk_x11_ungrab_cursor(gfx_window_t *window)
{
	gfx_x11_ungrab_cursor(X11_WINDOW);
}

static char *vk_x11_get_clipboard(gfx_window_t *window)
{
	return gfx_x11_get_clipboard(X11_WINDOW);
}

static void vk_x11_set_clipboard(gfx_window_t *window, const char *text)
{
	gfx_x11_set_clipboard(X11_WINDOW, text);
}

static gfx_cursor_t vk_x11_create_native_cursor(gfx_window_t *window, enum gfx_native_cursor native_cursor)
{
	return gfx_x11_create_native_cursor(X11_WINDOW, native_cursor);
}

static gfx_cursor_t vk_x11_create_cursor(gfx_window_t *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_x11_create_cursor(X11_WINDOW, data, width, height, xhot, yhot);
}

static void vk_x11_delete_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	gfx_x11_delete_cursor(X11_WINDOW, cursor);
}

static void vk_x11_set_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	gfx_x11_set_cursor(X11_WINDOW, cursor);
}

static void vk_x11_set_mouse_position(gfx_window_t *window, int32_t x, int32_t y)
{
	gfx_x11_set_mouse_position(X11_WINDOW, x, y);
}

static gfx_window_vtable_t vk_x11_vtable =
{
	GFX_WINDOW_VTABLE_DEF(vk_x11)
};

gfx_window_t *gfx_vk_x11_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties)
{
	gfx_window_t *window = GFX_MALLOC(sizeof(gfx_vk_x11_window_t));
	if (!window)
	{
		if (gfx_error_callback)
			gfx_error_callback("allocation failed");
		return NULL;
	}
	memset(window, 0, sizeof(gfx_vk_x11_window_t));
	window->vtable = &vk_x11_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	return window;

err:
	window->vtable->dtr(window);
	GFX_FREE(window);
	return NULL;
}
