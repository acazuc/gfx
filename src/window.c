#include "window.h"
#include "./window_vtable.h"
#include "./device.h"
#include <stdlib.h>
#include <string.h>

#if defined(GFX_ENABLE_WINDOW_X11)
# if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4)
#  include "windows/glx.h"
# endif
#endif

#if defined(GFX_ENABLE_WINDOW_WAYLAND)
# include "windows/wayland.h"
#endif

#if defined(GFX_ENABLE_WINDOW_WIN32)
# if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4)
#  include "windows/wgl.h"
# endif
# if defined(GFX_ENABLE_DEVICE_D3D9) || defined(GFX_ENABLE_DEVICE_D3D11)
#  include "windows/d3d.h"
# endif
#endif

#if defined(GFX_ENABLE_WINDOW_GLFW)
# include "windows/glfw.h"
#endif

#if 0
# include <stdio.h>
#define WIN_DEBUG printf("%s@%s:%d\n", __func__, __FILE__, __LINE__)
#else
#define WIN_DEBUG
#endif

gfx_memory_t gfx_memory = {NULL};
gfx_error_callback_t gfx_error_callback = NULL;

static bool ctr(gfx_window_t *window, gfx_window_properties_t *properties)
{
	window->key_down_callback = NULL;
	window->key_press_callback = NULL;
	window->key_up_callback = NULL;
	window->char_callback = NULL;
	window->mouse_down_callback = NULL;
	window->mouse_up_callback = NULL;
	window->scroll_callback = NULL;
	window->mouse_move_callback = NULL;
	window->cursor_enter_callback = NULL;
	window->cursor_leave_callback = NULL;
	window->focus_in_callback = NULL;
	window->focus_out_callback = NULL;
	window->resize_callback = NULL;
	window->move_callback = NULL;
	window->expose_callback = NULL;
	window->close_callback = NULL;
	window->properties = *properties;
	window->virtual_x = 0;
	window->virtual_y = 0;
	window->mouse_x = 0;
	window->mouse_y = 0;
	window->height = 0;
	window->width = 0;
	window->x = 0;
	window->y = 0;
	window->close_requested = false;
	window->grabbed = false;
	window->keys = GFX_MALLOC((GFX_KEY_LAST + 7) / 8);
	if (!window->keys)
	{
		if (gfx_error_callback)
			gfx_error_callback("allocation failed");
		return false;
	}
	memset(window->keys, 0, (GFX_KEY_LAST + 7) / 8);
	window->device = NULL;
	return true;
}

static void dtr(gfx_window_t *window)
{
	GFX_FREE(window->keys);
	if (window->device)
		gfx_device_delete(window->device);
}

gfx_window_vtable_t gfx_window_vtable =
{
	.ctr = ctr,
	.dtr = dtr,
};

gfx_window_t *gfx_create_window(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties)
{
	switch (properties->window_backend)
	{
		case GFX_WINDOW_X11:
#if defined(GFX_ENABLE_WINDOW_X11)
#if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4)
			if (properties->device_backend == GFX_DEVICE_GL3 || properties->device_backend == GFX_DEVICE_GL4)
				return gfx_glx_window_new(title, width, height, properties);
#endif
#endif
			break;
		case GFX_WINDOW_WIN32:
#if defined(GFX_ENABLE_WINDOW_WIN32)
#if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4)
			if (properties->device_backend == GFX_DEVICE_GL3 || properties->device_backend == GFX_DEVICE_GL4)
				return gfx_wgl_window_new(title, width, height, properties);
#endif
#if defined(GFX_ENABLE_DEVICE_D3D9) || defined(GFX_ENABLE_DEVICE_D3D11)
			if (properties->device_backend == GFX_DEVICE_D3D9 || properties->device_backend == GFX_DEVICE_D3D11)
				return gfx_d3d_window_new(title, width, height, properties);
#endif
#endif
			break;
		case GFX_WINDOW_WAYLAND:
#if defined(GFX_ENABLE_WINDOW_WAYLAND)
#if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_VK)
			if (properties->device_backend == GFX_DEVICE_GL3 || properties->device_backend == GFX_DEVICE_GL4 || properties->device_backend == GFX_DEVICE_VK)
				return gfx_wl_window_new(title, width, height, properties);
#endif
#endif
			break;
		case GFX_WINDOW_GLFW:
#if defined(GFX_ENABLE_WINDOW_GLFW)
#if defined(GFX_ENABLE_DEVICE_GL3) || defined(GFX_ENABLE_DEVICE_GL4) || defined(GFX_ENABLE_DEVICE_VK)
			if (properties->device_backend == GFX_DEVICE_GL3 || properties->device_backend == GFX_DEVICE_GL4 || properties->device_backend == GFX_DEVICE_VK)
				return gfx_glfw_window_new(title, width, height, properties);
#endif
#endif
			break;
	}
	return NULL;
}

void gfx_delete_window(gfx_window_t *window)
{
	if (window == NULL)
		return;
	WIN_DEBUG;
	window->vtable->dtr(window);
	GFX_FREE(window);
	WIN_DEBUG;
}

bool gfx_create_device(gfx_window_t *window)
{
	WIN_DEBUG;
	bool ret = window->vtable->create_device(window);
	WIN_DEBUG;
	return ret;
}

void gfx_window_show(gfx_window_t *window)
{
	WIN_DEBUG;
	window->vtable->show(window);
	WIN_DEBUG;
}

void gfx_window_hide(gfx_window_t *window)
{
	WIN_DEBUG;
	window->vtable->hide(window);
	WIN_DEBUG;
}

void gfx_window_poll_events(gfx_window_t *window)
{
	WIN_DEBUG;
	window->vtable->poll_events(window);
	WIN_DEBUG;
}

void gfx_window_wait_events(gfx_window_t *window)
{
	WIN_DEBUG;
	window->vtable->wait_events(window);
	WIN_DEBUG;
}

void gfx_window_grab_cursor(gfx_window_t *window)
{
	WIN_DEBUG;
	window->vtable->grab_cursor(window);
	WIN_DEBUG;
}

void gfx_window_ungrab_cursor(gfx_window_t *window)
{
	WIN_DEBUG;
	window->vtable->ungrab_cursor(window);
	WIN_DEBUG;
}

void gfx_window_swap_buffers(gfx_window_t *window)
{
	WIN_DEBUG;
	window->vtable->swap_buffers(window);
	WIN_DEBUG;
}

void gfx_window_make_current(gfx_window_t *window)
{
	WIN_DEBUG;
	window->vtable->make_current(window);
	WIN_DEBUG;
}

void gfx_window_set_swap_interval(gfx_window_t *window, int interval)
{
	WIN_DEBUG;
	window->vtable->set_swap_interval(window, interval);
	WIN_DEBUG;
}

void gfx_window_set_title(gfx_window_t *window, const char *title)
{
	WIN_DEBUG;
	window->vtable->set_title(window, title);
	WIN_DEBUG;
}

void gfx_window_set_icon(gfx_window_t *window, const void *data, uint32_t width, uint32_t height)
{
	WIN_DEBUG;
	window->vtable->set_icon(window, data, width, height);
	WIN_DEBUG;
}

void gfx_window_resize(gfx_window_t *window, uint32_t width, uint32_t height)
{
	WIN_DEBUG;
	window->vtable->resize(window, width, height);
	WIN_DEBUG;
}

char *gfx_window_get_clipboard(gfx_window_t *window)
{
	WIN_DEBUG;
	char *ret = window->vtable->get_clipboard(window);
	WIN_DEBUG;
	return ret;
}

void gfx_window_set_clipboard(gfx_window_t *window, const char *clipboard)
{
	WIN_DEBUG;
	window->vtable->set_clipboard(window, clipboard);
	WIN_DEBUG;
}

gfx_cursor_t gfx_create_native_cursor(gfx_window_t *window, enum gfx_native_cursor native_cursor)
{
	WIN_DEBUG;
	gfx_cursor_t cursor = window->vtable->create_native_cursor(window, native_cursor);
	WIN_DEBUG;
	return cursor;
}

gfx_cursor_t gfx_create_cursor(gfx_window_t *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	WIN_DEBUG;
	gfx_cursor_t cursor = window->vtable->create_cursor(window, data, width, height, xhot, yhot);
	WIN_DEBUG;
	return cursor;
}

void gfx_delete_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	WIN_DEBUG;
	window->vtable->delete_cursor(window, cursor);
	WIN_DEBUG;
}

void gfx_set_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	WIN_DEBUG;
	window->vtable->set_cursor(window, cursor);
	WIN_DEBUG;
}

void gfx_set_mouse_position(gfx_window_t *window, int32_t x, int32_t y)
{
	WIN_DEBUG;
	window->vtable->set_mouse_position(window, x, y);
	WIN_DEBUG;
}

int32_t gfx_get_mouse_x(gfx_window_t *window)
{
	if (window->grabbed)
		return window->virtual_x;
	return window->mouse_x;
}

int32_t gfx_get_mouse_y(gfx_window_t *window)
{
	if (window->grabbed)
		return window->virtual_y;
	return window->mouse_y;
}

bool gfx_is_key_down(gfx_window_t *window, enum gfx_key_code key)
{
	if (key >= GFX_KEY_LAST)
		return false;
	return window->keys[key / 8] & (1 << (key % 8));
}

void gfx_window_properties_init(gfx_window_properties_t *properties)
{
	properties->depth_bits = GFX_WINDOW_PROPERTY_DONT_CARE;
	properties->stencil_bits = GFX_WINDOW_PROPERTY_DONT_CARE;
	properties->red_bits = GFX_WINDOW_PROPERTY_DONT_CARE;
	properties->green_bits = GFX_WINDOW_PROPERTY_DONT_CARE;
	properties->blue_bits = GFX_WINDOW_PROPERTY_DONT_CARE;
	properties->alpha_bits = GFX_WINDOW_PROPERTY_DONT_CARE;
	properties->samples = 0;
	properties->srgb = GFX_WINDOW_PROPERTY_DONT_CARE;
	properties->double_buffer = 1;
	properties->stereo = 0;
}

