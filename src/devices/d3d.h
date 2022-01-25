#ifndef GFX_D3D_DEVICE_H
#define GFX_D3D_DEVICE_H

#include "../device.h"

typedef struct gfx_d3d_device_s
{
	gfx_device_t device;
} gfx_d3d_device_t;

void gfx_d3d_errors(uint32_t err, const char *fn, const char *file, int line);

#endif
