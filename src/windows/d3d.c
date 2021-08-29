#define COBJMACROS
#define CINTERFACE
#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include "d3d.h"
#include "../window_vtable.h"
#include "./win32.h"

#ifdef GFX_ENABLE_DEVICE_D3D9
# include "../devices/d3d9.h"
#endif

#ifdef GFX_ENABLE_DEVICE_D3D11
# include "../devices/d3d11.h"
#endif

DEFINE_GUID(UIID_IDXGIFactory, 0x7b7166ec, 0x21c7, 0x44ae, 0xb2, 0x1a, 0xc9, 0xae, 0x32, 0x1a, 0xe3, 0x69);

extern gfx_window_vtable_t gfx_window_vtable;

typedef struct gfx_d3d_window_s
{
	gfx_window_t window;
	gfx_win32_window_t win32;
	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	IDXGISwapChain *swap_chain;
	int interval;
} gfx_d3d_window_t;

#define WIN32_WINDOW (&D3D_WINDOW->win32)
#define D3D_WINDOW ((gfx_d3d_window_t*)window)

static bool d3d_ctr(gfx_window_t *window, gfx_window_properties_t *properties)
{
	D3D_WINDOW->interval = 1;
	return gfx_window_vtable.ctr(window, properties);
}

static void d3d_dtr(gfx_window_t *window)
{
	if (D3D_WINDOW->swap_chain)
		IDXGISwapChain_Release(D3D_WINDOW->swap_chain);
	gfx_win32_dtr(&D3D_WINDOW->win32);
	gfx_window_vtable.dtr(window);
}

static bool d3d_create_device(gfx_window_t *window)
{
	switch (window->properties.device_backend)
	{
#ifdef GFX_ENABLE_DEVICE_D3D9
		case GFX_DEVICE_D3D9:
			window->device = gfx_d3d9_device_new(&D3D_WINDOW->window, &D3D_WINDOW->swap_chain_desc, &D3D_WINDOW->swap_chain);
			return true;
#endif
#ifdef GFX_ENABLE_DEVICE_D3D11
		case GFX_DEVICE_D3D11:
			window->device = gfx_d3d11_device_new(&D3D_WINDOW->window, &D3D_WINDOW->swap_chain_desc, &D3D_WINDOW->swap_chain);
			return true;
#endif
	}
	return false;
}

static void d3d_show(gfx_window_t *window)
{
	gfx_win32_show(WIN32_WINDOW);
}

static void d3d_hide(gfx_window_t *window)
{
	gfx_win32_hide(WIN32_WINDOW);
}

static void d3d_set_title(gfx_window_t *window, const char *title)
{
	gfx_win32_set_title(WIN32_WINDOW, title);
}

static void d3d_poll_events(gfx_window_t *window)
{
	gfx_win32_poll_events(WIN32_WINDOW);
}

static void d3d_wait_events(gfx_window_t *window)
{
	gfx_win32_wait_events(WIN32_WINDOW);
}

static void d3d_set_swap_interval(gfx_window_t *window, int interval)
{
	if (interval == 0)
		D3D_WINDOW->interval = 0;
	else
		D3D_WINDOW->interval = 1;
}

static void d3d_swap_buffers(gfx_window_t *window)
{
	IDXGISwapChain_Present(D3D_WINDOW->swap_chain, D3D_WINDOW->interval, 0);
}

static void d3d_make_current(gfx_window_t *window)
{
	//XXX
}

static void d3d_resize(gfx_window_t *window, uint32_t width, uint32_t height)
{
	gfx_win32_resize(WIN32_WINDOW, width, height);
}

static void d3d_grab_cursor(gfx_window_t *window)
{
	gfx_win32_grab_cursor(WIN32_WINDOW);
}

static void d3d_ungrab_cursor(gfx_window_t *window)
{
	gfx_win32_ungrab_cursor(WIN32_WINDOW);
}

static char *d3d_get_clipboard(gfx_window_t *window)
{
	return gfx_win32_get_clipboard(WIN32_WINDOW);
}

static void d3d_set_clipboard(gfx_window_t *window, const char *text)
{
	gfx_win32_set_clipboard(WIN32_WINDOW, text);
}

static void d3d_set_native_cursor(gfx_window_t *window, enum gfx_native_cursor cursor)
{
	gfx_win32_set_native_cursor(WIN32_WINDOW, cursor);
}

static void d3d_set_mouse_position(gfx_window_t *window, int32_t x, int32_t y)
{
	gfx_win32_set_mouse_position(WIN32_WINDOW, x, y);
}

static gfx_window_vtable_t d3d_vtable =
{
	GFX_WINDOW_VTABLE_DEF(d3d)
};

gfx_window_t *gfx_d3d_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties, gfx_window_t *shared_context)
{
	gfx_window_t *window = calloc(sizeof(gfx_d3d_window_t), 1);
	if (!window)
	{
		if (gfx_error_callback)
			gfx_error_callback("malloc failed");
		return NULL;
	}
	window->vtable = &d3d_vtable;
	if (!window->vtable->ctr(window, properties))
	{
		window->vtable->dtr(window);
		free(window);
		return NULL;
	}
	gfx_win32_ctr(WIN32_WINDOW, window);
	if (!gfx_win32_create_window(WIN32_WINDOW, title, width, height))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to create window");
		window->vtable->dtr(window);
		free(window);
		return NULL;
	}
	IDXGIFactory *factory;
	IDXGIAdapter *adapter;
	IDXGIOutput* adapter_output;
	DXGI_ADAPTER_DESC adapter_desc;
	unsigned num_modes;
	unsigned numerator;
	unsigned denominator;
	DXGI_MODE_DESC *modes = NULL;
	if (FAILED(CreateDXGIFactory(&UIID_IDXGIFactory, (void**)&factory)))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to create DXGI factory");
		goto err;
	}
	if (FAILED(IDXGIFactory_EnumAdapters(factory, 0, &adapter)))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to enumerate adapters");
		goto err;
	}
	if (FAILED(IDXGIAdapter_EnumOutputs(adapter, 0, &adapter_output)))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to enumerate outputs");
		goto err;
	}
	if (FAILED(IDXGIOutput_GetDisplayModeList(adapter_output, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes, NULL)))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to get display mode list number");
		goto err;
	}
	modes = (DXGI_MODE_DESC*)malloc(sizeof(*modes) * num_modes);
	if (!modes)
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to malloc dxgi modes");
		goto err;
	}
	if (FAILED(IDXGIOutput_GetDisplayModeList(adapter_output, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes, modes)))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to get dispaly mode list");
		goto err;
	}
	for (uint32_t i = 0; i < num_modes; ++i)
	{
		if (modes[i].Width == width && modes[i].Height == height)
		{
			numerator = modes[i].RefreshRate.Numerator;
			denominator = modes[i].RefreshRate.Denominator;
		}
	}
	if (FAILED(IDXGIAdapter_GetDesc(adapter, &adapter_desc)))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to get desc");
		goto err;
	}
	D3D_WINDOW->swap_chain_desc.BufferCount = 1;
	D3D_WINDOW->swap_chain_desc.BufferDesc.Width = width;
	D3D_WINDOW->swap_chain_desc.BufferDesc.Height = height;
	D3D_WINDOW->swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D_WINDOW->swap_chain_desc.BufferDesc.RefreshRate.Numerator = numerator;
	D3D_WINDOW->swap_chain_desc.BufferDesc.RefreshRate.Denominator = denominator;
	D3D_WINDOW->swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	D3D_WINDOW->swap_chain_desc.OutputWindow = WIN32_WINDOW->window;
	D3D_WINDOW->swap_chain_desc.SampleDesc.Count = 1;
	D3D_WINDOW->swap_chain_desc.SampleDesc.Quality = 0;
	D3D_WINDOW->swap_chain_desc.Windowed = true;
	D3D_WINDOW->swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	D3D_WINDOW->swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	D3D_WINDOW->swap_chain_desc.Flags = 0;
	IDXGIOutput_Release(adapter_output);
	IDXGIFactory_Release(factory);
	free(modes);
	return window;

err:
	if (adapter_output)
		IDXGIOutput_Release(adapter_output);
	if (factory)
		IDXGIFactory_Release(factory);
	free(modes);
	window->vtable->dtr(window);
	free(window);
	return NULL;
}

