#ifndef GFX_D3D11_DEVICE_H
#define GFX_D3D11_DEVICE_H

#include "./d3d.h"
#include <dxgi.h>

gfx_device_t *gfx_d3d11_device_new(gfx_window_t *window, DXGI_SWAP_CHAIN_DESC *swap_chain_desc, IDXGISwapChain **swap_chain);
void gfx_d3d11_resize(gfx_device_t *device);

#endif
