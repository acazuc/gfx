#include "wgl.h"
#include "../window_vtable.h"
#include "../config.h"
#include "win32.h"
#include <windows.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/wglext.h>

extern gfx_window_vtable_t gfx_window_vtable;

typedef struct gfx_wgl_window_s
{
	gfx_gl_window_t gl;
	gfx_win32_window_t win32;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
	PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	HGLRC context;
	HDC device;
	int32_t prev_mouse_x;
	int32_t prev_mouse_y;
	bool adaptive_vsync;
} gfx_wgl_window_t;

#define WIN32_WINDOW (&WGL_WINDOW->win32)
#define GL_WINDOW (&window->gl)
#define WGL_WINDOW ((gfx_wgl_window_t*)window)

static void *get_proc_address(const char *name)
{
	void *ret = (void*)wglGetProcAddress(name);
	if (ret == NULL || ret  == (void*)0x1 || ret == (void*)0x2 || ret == (void*)0x3 || ret == (void*)-1)
	{
		HMODULE module = LoadLibraryA("opengl32.dll");
		ret = (void*)GetProcAddress(module, name);
	}
	return ret;
}

static bool wgl_ctr(gfx_window_t *window, gfx_window_properties_t *properties)
{
	return gfx_window_vtable.ctr(window, properties);
}

static void wgl_dtr(gfx_window_t *window)
{
	gfx_win32_dtr(WIN32_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool wgl_create_device(gfx_window_t *window)
{
	return gfx_gl_create_device(window, get_proc_address);
}

static void wgl_show(gfx_window_t *window)
{
	gfx_win32_show(WIN32_WINDOW);
}

static void wgl_hide(gfx_window_t *window)
{
	gfx_win32_hide(WIN32_WINDOW);
}

static void wgl_set_title(gfx_window_t *window, const char *title)
{
	gfx_win32_set_title(WIN32_WINDOW, title);
}

static void wgl_set_icon(gfx_window_t *window, const void *data, uint32_t width, uint32_t height)
{
	gfx_win32_set_icon(WIN32_WINDOW, data, width, height);
}

static void wgl_poll_events(gfx_window_t *window)
{
	gfx_win32_poll_events(WIN32_WINDOW);
}

static void wgl_wait_events(gfx_window_t *window)
{
	gfx_win32_wait_events(WIN32_WINDOW);
}

static void wgl_set_swap_interval(gfx_window_t *window, int interval)
{
	WGL_WINDOW->wglSwapIntervalEXT(interval);
}

static void wgl_swap_buffers(gfx_window_t *window)
{
	SwapBuffers(WGL_WINDOW->device);
}

static void wgl_make_current(gfx_window_t *window)
{
	wglMakeCurrent(WGL_WINDOW->device, WGL_WINDOW->context);
}

static void wgl_resize(gfx_window_t *window, uint32_t width, uint32_t height)
{
	gfx_win32_resize(WIN32_WINDOW, width, height);
}

static void wgl_grab_cursor(gfx_window_t *window)
{
	gfx_win32_grab_cursor(WIN32_WINDOW);
}

static void wgl_ungrab_cursor(gfx_window_t *window)
{
	gfx_win32_ungrab_cursor(WIN32_WINDOW);
}

static char *wgl_get_clipboard(gfx_window_t *window)
{
	return gfx_win32_get_clipboard(WIN32_WINDOW);
}

static void wgl_set_clipboard(gfx_window_t *window, const char *text)
{
	gfx_win32_set_clipboard(WIN32_WINDOW, text);
}

static gfx_cursor_t wgl_create_native_cursor(gfx_window_t *window, enum gfx_native_cursor cursor)
{
	return gfx_win32_create_native_cursor(WIN32_WINDOW, cursor);
}

static gfx_cursor_t wgl_create_cursor(gfx_window_t *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	return gfx_win32_create_cursor(WIN32_WINDOW, data, width, height, xhot, yhot);
}

static void wgl_delete_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	gfx_win32_delete_cursor(WIN32_WINDOW, cursor);
}

static void wgl_set_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	gfx_win32_set_cursor(WIN32_WINDOW, cursor);
}

static void wgl_set_mouse_position(gfx_window_t *window, int32_t x, int32_t y)
{
	gfx_win32_set_mouse_position(WIN32_WINDOW, x, y);
}

static gfx_window_vtable_t wgl_vtable =
{
	GFX_WINDOW_VTABLE_DEF(wgl)
};

gfx_window_t *gfx_wgl_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties)
{
	gfx_window_t *window = GFX_MALLOC(sizeof(gfx_wgl_window_t));
	if (!window)
	{
		if (gfx_error_callback)
			gfx_error_callback("malloc failed");
		return NULL;
	}
	memset(window, 0, sizeof(gfx_wgl_window_t));
	window->vtable = &wgl_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	gfx_win32_ctr(WIN32_WINDOW, window);
	if (!gfx_win32_create_window(WIN32_WINDOW, title, width, height))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to create window");
		goto err;
	}
	if (!(WGL_WINDOW->device = GetDC(WIN32_WINDOW->window)))
	{
		if (gfx_error_callback)
			gfx_error_callback("GetDC failed");
		goto err;
	}
	HGLRC tmp_ctx;
	{
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA,
			32,
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,
			8,
			0,
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};
		int pformat = ChoosePixelFormat(WGL_WINDOW->device, &pfd);
		SetPixelFormat(WGL_WINDOW->device, pformat, &pfd);
		tmp_ctx = wglCreateContext(WGL_WINDOW->device);
		wglMakeCurrent(WGL_WINDOW->device, tmp_ctx);
	}
	WGL_WINDOW->wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	WGL_WINDOW->wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
	WGL_WINDOW->wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	wglDeleteContext(tmp_ctx);
	if (!WGL_WINDOW->wglGetExtensionsStringARB)
	{
		if (gfx_error_callback)
			gfx_error_callback("no wglGetExtensionsStringARB");
		goto err;
	}
	if (!WGL_WINDOW->wglSwapIntervalEXT)
	{
		if (gfx_error_callback)
			gfx_error_callback("no wglSwapIntervalEXT");
		goto err;
	}
	{
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
		if (properties->double_buffer)
			pfd.dwFlags |= PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		if (properties->alpha_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
			pfd.cAlphaBits = properties->alpha_bits;
		pfd.cColorBits = 32;
		if (properties->depth_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
			pfd.cDepthBits = properties->depth_bits;
		if (properties->stencil_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
			pfd.cStencilBits = properties->stencil_bits;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int pformat = ChoosePixelFormat(WGL_WINDOW->device, &pfd);
		SetPixelFormat(WGL_WINDOW->device, pformat, &pfd);
	}
	if (WGL_WINDOW->wglCreateContextAttribsARB)
	{
		int attributes[20];
		size_t attributes_nb = 0;
#ifdef GFX_ENABLE_DEVICE_GL3
		if (properties->device_backend == GFX_DEVICE_GL3)
		{
			attributes[attributes_nb++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
			attributes[attributes_nb++] = 3;
			attributes[attributes_nb++] = WGL_CONTEXT_MINOR_VERSION_ARB;
			attributes[attributes_nb++] = 3;
		}
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
		if (properties->device_backend == GFX_DEVICE_GL4)
		{
			attributes[attributes_nb++] = WGL_CONTEXT_MAJOR_VERSION_ARB;
			attributes[attributes_nb++] = 4;
			attributes[attributes_nb++] = WGL_CONTEXT_MINOR_VERSION_ARB;
			attributes[attributes_nb++] = 5;
		}
#endif
		attributes[attributes_nb++] = 0;
		attributes[attributes_nb++] = 0;
		WGL_WINDOW->context = WGL_WINDOW->wglCreateContextAttribsARB(WGL_WINDOW->device, NULL, attributes);
		if (!WGL_WINDOW->context)
		{
			if (gfx_error_callback)
				gfx_error_callback("wglCreateContextAttribsARB failed: %u", (unsigned)GetLastError());
			goto err;
		}
	}
	else
	{
		WGL_WINDOW->context = wglCreateContext(WGL_WINDOW->device);
		if (!WGL_WINDOW->context)
		{
			if (gfx_error_callback)
				gfx_error_callback("wglCreateContext failed: %u", (unsigned)GetLastError());
			goto err;
		}
	}
	return window;

err:
	window->vtable->dtr(window);
	GFX_FREE(window);
	return NULL;
}
