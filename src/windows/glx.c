#include "glx.h"
#include "../window_vtable.h"
#include "./x11.h"
#include <GL/glx.h>
#include <string.h>
#include <stdlib.h>

extern gfx_window_vtable_t gfx_window_vtable;

typedef struct gfx_glx_window_s
{
	gfx_gl_window_t gl;
	gfx_x11_window_t x11;
	PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB;
	PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT;
	GLXContext context;
	GLXWindow window;
	bool adaptive_vsync;
} gfx_glx_window_t;

#ifdef GLX_WINDOW
# undef GLX_WINDOW
#endif

#define X11_WINDOW (&GLX_WINDOW->x11)
#define GLX_WINDOW ((gfx_glx_window_t*)window)

static void *get_proc_address(const char *name)
{
	return (void*)glXGetProcAddress((const GLubyte*)name);
}

static bool glx_ctr(gfx_window_t *window, gfx_window_properties_t *properties)
{
	if (!gfx_x11_ctr(X11_WINDOW, window))
		return false;
	return gfx_window_vtable.ctr(window, properties);
}

static void glx_dtr(gfx_window_t *window)
{
	gfx_x11_dtr(X11_WINDOW);
	gfx_window_vtable.dtr(window);
}

static bool glx_create_device(gfx_window_t *window)
{
	return gfx_gl_create_device(window, get_proc_address);
}

static void glx_show(gfx_window_t *window)
{
	gfx_x11_show(X11_WINDOW);
}

static void glx_hide(gfx_window_t *window)
{
	gfx_x11_hide(X11_WINDOW);
}

static void glx_set_title(gfx_window_t *window, const char *title)
{
	gfx_x11_set_title(X11_WINDOW, title);
}

static void glx_poll_events(gfx_window_t *window)
{
	gfx_x11_poll_events(X11_WINDOW);
}

static void glx_wait_events(gfx_window_t *window)
{
	gfx_x11_wait_events(X11_WINDOW);
}

static void glx_set_swap_interval(gfx_window_t *window, int interval)
{
	if (GLX_WINDOW->glXSwapIntervalEXT)
	{
		if (interval < 0 && !GLX_WINDOW->adaptive_vsync)
			interval = 1;
		GLX_WINDOW->glXSwapIntervalEXT(X11_WINDOW->display, GLX_WINDOW->window, interval);
	}
}

static void glx_swap_buffers(gfx_window_t *window)
{
	glXSwapBuffers(X11_WINDOW->display, GLX_WINDOW->window);
}

static void glx_make_current(gfx_window_t *window)
{
	glXMakeContextCurrent(X11_WINDOW->display, GLX_WINDOW->window, GLX_WINDOW->window, GLX_WINDOW->context);
}

static void glx_resize(gfx_window_t *window, uint32_t width, uint32_t height)
{
	gfx_x11_resize(X11_WINDOW, width, height);
}

static void glx_grab_cursor(gfx_window_t *window)
{
	gfx_x11_grab_cursor(X11_WINDOW);
}

static void glx_ungrab_cursor(gfx_window_t *window)
{
	gfx_x11_ungrab_cursor(X11_WINDOW);
}

static char *glx_get_clipboard(gfx_window_t *window)
{
	return gfx_x11_get_clipboard(X11_WINDOW);
}

static void glx_set_clipboard(gfx_window_t *window, const char *text)
{
	gfx_x11_set_clipboard(X11_WINDOW, text);
}

static void glx_set_native_cursor(gfx_window_t *window, enum gfx_native_cursor cursor)
{
	gfx_x11_set_native_cursor(X11_WINDOW, cursor);
}

static void glx_set_mouse_position(gfx_window_t *window, int32_t x, int32_t y)
{
	gfx_x11_set_mouse_position(X11_WINDOW, x, y);
}

static gfx_window_vtable_t glx_vtable =
{
	GFX_WINDOW_VTABLE_DEF(glx)
};

static void load_extensions(gfx_window_t *window)
{
	const char *extensions = glXQueryExtensionsString(X11_WINDOW->display, 0);
	if (strstr(extensions, "GLX_EXT_swap_control"))
		GLX_WINDOW->glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
	if (strstr(extensions, "GLX_ARB_create_context"))
		GLX_WINDOW->glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
	GLX_WINDOW->adaptive_vsync = (strstr(extensions, "EXT_swap_control_tear") != NULL);
}

static GLXFBConfig *get_configs(gfx_window_t *window, gfx_window_properties_t *properties)
{
	int attributes[30];
	int attributes_nb = 0;
	attributes[attributes_nb++] = GLX_RENDER_TYPE;
	attributes[attributes_nb++] = GLX_RGBA_BIT;
	attributes[attributes_nb++] = GLX_X_RENDERABLE;
	attributes[attributes_nb++] = True;
	attributes[attributes_nb++] = GLX_DRAWABLE_TYPE;
	attributes[attributes_nb++] = GLX_WINDOW_BIT;
	if (properties->depth_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
	{
		attributes[attributes_nb++] = GLX_DEPTH_SIZE;
		attributes[attributes_nb++] = properties->depth_bits;
	}
	if (properties->stencil_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
	{
		attributes[attributes_nb++] = GLX_STENCIL_SIZE;
		attributes[attributes_nb++] = properties->stencil_bits;
	}
	if (properties->red_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
	{
		attributes[attributes_nb++] = GLX_RED_SIZE;
		attributes[attributes_nb++] = properties->red_bits;
	}
	if (properties->green_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
	{
		attributes[attributes_nb++] = GLX_GREEN_SIZE;
		attributes[attributes_nb++] = properties->green_bits;
	}
	if (properties->blue_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
	{
		attributes[attributes_nb++] = GLX_BLUE_SIZE;
		attributes[attributes_nb++] = properties->blue_bits;
	}
	if (properties->alpha_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
	{
		attributes[attributes_nb++] = GLX_ALPHA_SIZE;
		attributes[attributes_nb++] = properties->alpha_bits;
	}
	if (properties->double_buffer != GFX_WINDOW_PROPERTY_DONT_CARE)
	{
		attributes[attributes_nb++] = GLX_DOUBLEBUFFER;
		attributes[attributes_nb++] = properties->double_buffer ? True : False;
	}
	attributes[attributes_nb++] = None;
	attributes[attributes_nb++] = None;
	int configs_count;
	GLXFBConfig *configs = glXChooseFBConfig(X11_WINDOW->display, 0, attributes, &configs_count);
	return configs;
}

static void create_context(gfx_window_t *window, gfx_window_properties_t *properties, XVisualInfo *vi, GLXFBConfig *configs, gfx_window_t *shared_window)
{
	if (GLX_WINDOW->glXCreateContextAttribsARB)
	{
		int attributes[10];
		int attributes_nb = 0;
#ifdef GFX_ENABLE_DEVICE_GL3
		if (properties->device_backend == GFX_DEVICE_GL3)
		{
			attributes[attributes_nb++] = GLX_CONTEXT_MAJOR_VERSION_ARB;
			attributes[attributes_nb++] = 3;
			attributes[attributes_nb++] = GLX_CONTEXT_MINOR_VERSION_ARB;
			attributes[attributes_nb++] = 3;
		}
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
		if (properties->device_backend == GFX_DEVICE_GL4)
		{
			attributes[attributes_nb++] = GLX_CONTEXT_MAJOR_VERSION_ARB;
			attributes[attributes_nb++] = 4;
			attributes[attributes_nb++] = GLX_CONTEXT_MINOR_VERSION_ARB;
			attributes[attributes_nb++] = 5;
		}
#endif
		attributes[attributes_nb++] = None;
		attributes[attributes_nb++] = None;
		GLX_WINDOW->context = GLX_WINDOW->glXCreateContextAttribsARB(X11_WINDOW->display, configs[0], shared_window ? ((gfx_glx_window_t*)shared_window)->context : NULL, true, attributes);
	}
	else
	{
		GLX_WINDOW->context = glXCreateContext(X11_WINDOW->display, vi, shared_window ? ((gfx_glx_window_t*)shared_window)->context : NULL, true);
	}
}

gfx_window_t *gfx_glx_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties, gfx_window_t *shared_window)
{
	XVisualInfo *vi = NULL;
	GLXFBConfig *configs;
	gfx_window_t *window = calloc(sizeof(gfx_glx_window_t), 1);
	if (!window)
	{
		if (gfx_error_callback)
			gfx_error_callback("allocation failed");
		return NULL;
	}
	window->vtable = &glx_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	load_extensions(window);
	configs = get_configs(window, properties);
	if (!configs)
		goto err;
	vi = glXGetVisualFromFBConfig(X11_WINDOW->display, configs[0]);
	if (!gfx_x11_create_window(X11_WINDOW, title, width, height, vi))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to create window");
		goto err;
	}
	GLX_WINDOW->window = glXCreateWindow(X11_WINDOW->display, configs[0], X11_WINDOW->window, NULL);
	create_context(window, properties, vi, configs, shared_window);
	return window;

err:
	window->vtable->dtr(window);
	free(window);
	XFree(vi);
	return NULL;
}
