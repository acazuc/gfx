#include "gl.h"

#ifdef GFX_ENABLE_DEVICE_GL3
# include "../devices/gl3.h"
#endif

#ifdef GFX_ENABLE_DEVICE_GL4
# include "../devices/gl4.h"
#endif

bool gfx_gl_create_device(gfx_window_t *window, gfx_gl_load_addr_t *load_addr)
{
	switch (window->properties.device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_GL3
		case GFX_DEVICE_GL3:
			window->device = gfx_gl3_device_new(window, load_addr);
			return true;
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
		case GFX_DEVICE_GL4:
			window->device = gfx_gl4_device_new(window, load_addr);
			return true;
#endif
	}
	return false;
}
