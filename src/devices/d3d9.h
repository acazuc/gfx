#ifndef GFX_D3D9_DEVICE_H
# define GFX_D3D9_DEVICE_H

# ifdef __cplusplus
extern "C" {
# endif

# include "./d3d.h"
# include <dxgi.h>

gfx_device_t *gfx_d3d9_device_new(gfx_window_t *window, DXGI_SWAP_CHAIN_DESC *swap_chain_desc, IDXGISwapChain **swap_chain);

# ifdef __cplusplus
}
# endif

#endif
