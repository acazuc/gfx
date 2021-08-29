#define COBJMACROS
#define CINTERFACE
#define D3D11_NO_HELPER
#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include "d3d11.h"
#include "../device_vtable.h"
#include "../window.h"
#include <d3dcompiler.h>
#include <inttypes.h>
#include <winerror.h>
#include <d3d11_4.h>
#include <assert.h>
#include <stdio.h>
#include <d3d9.h>

#define D3D_DEVICE ((gfx_d3d_device_t*)device)
#define D3D11_DEVICE ((gfx_d3d11_device_t*)device)

DEFINE_GUID(UIID_ID3D11Texture2D, 0x6f15aaf2, 0xd208, 0x4e89, 0x9a, 0xb4, 0x48, 0x95, 0x35, 0xd3, 0x4f, 0x9c);

static inline UINT D3D11CalcSubresource(UINT MipSlice, UINT ArraySlice, UINT MipLevels)
{
	return MipSlice + ArraySlice * MipLevels;
}

typedef struct gfx_d3d11_device_s
{
	gfx_device_t device;
	ID3D11DeviceContext4 *d3dctx;
	ID3D11Device4 *d3ddev;
	IDXGISwapChain *swap_chain;
	ID3D11DepthStencilView *default_depth_stencil_view;
	ID3D11RenderTargetView *default_render_target_view;
	const gfx_rasterizer_state_t *rasterizer_state;
	enum gfx_primitive_type primitive;
	uint64_t pipeline_state;
	uint64_t state_idx;
} gfx_d3d11_device_t;

static const D3D11_PRIMITIVE_TOPOLOGY primitives[3] = {D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, D3D11_PRIMITIVE_TOPOLOGY_LINELIST};

static const DXGI_FORMAT attribute_types[34] =
{
	DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT,
	DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32_SINT, DXGI_FORMAT_R32G32B32_UINT,
	DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT,
	DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT,
	DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_SINT,
	DXGI_FORMAT_R16G16_FLOAT, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_R16G16_SINT,
	DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_SNORM, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_SINT,
	DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_SNORM, DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_R8G8_SINT,
	DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_SNORM, DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8_SINT,
};

static const D3D11_COMPARISON_FUNC compare_functions[8] = {D3D11_COMPARISON_NEVER, D3D11_COMPARISON_LESS, D3D11_COMPARISON_LESS_EQUAL, D3D11_COMPARISON_EQUAL, D3D11_COMPARISON_GREATER_EQUAL, D3D11_COMPARISON_GREATER, D3D11_COMPARISON_NOT_EQUAL, D3D11_COMPARISON_ALWAYS};

static const D3D11_BLEND blend_functions[12] = {D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_SRC_COLOR, D3D11_BLEND_INV_SRC_COLOR, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_INV_DEST_COLOR, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_SRC1_COLOR, D3D11_BLEND_INV_SRC1_COLOR};

static const D3D11_BLEND_OP blend_equations[5] = {D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_SUBTRACT, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_OP_MIN, D3D11_BLEND_OP_MAX};

static const D3D11_STENCIL_OP stencil_operations[8] = {D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_ZERO, D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_INCR_SAT, D3D11_STENCIL_OP_INCR, D3D11_STENCIL_OP_DECR_SAT, D3D11_STENCIL_OP_DECR, D3D11_STENCIL_OP_INVERT};

static const D3D11_FILL_MODE fill_modes[3] = {D3D11_FILL_WIREFRAME, D3D11_FILL_WIREFRAME, D3D11_FILL_SOLID};

static const D3D11_CULL_MODE cull_modes[3] = {D3D11_CULL_BACK, D3D11_CULL_FRONT, D3D11_CULL_BACK};

static const D3D11_BIND_FLAG buffer_types[3] = {D3D11_BIND_VERTEX_BUFFER, D3D11_BIND_INDEX_BUFFER, D3D11_BIND_CONSTANT_BUFFER};

static const D3D11_USAGE buffer_usages[4] = {D3D11_USAGE_IMMUTABLE, D3D11_USAGE_DEFAULT, D3D11_USAGE_DYNAMIC, D3D11_USAGE_DYNAMIC};

static const DXGI_FORMAT formats[14] = {DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B5G5R5A1_UNORM, DXGI_FORMAT_B4G4R4A4_UNORM, DXGI_FORMAT_B5G6R5_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM};

static const D3D11_TEXTURE_ADDRESS_MODE texture_addressings[5] = {D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_MIRROR_ONCE};

static const DXGI_FORMAT index_types[2] = {DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R32_UINT};

static const D3D11_FILTER filtering[27] =
{
	D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
	D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
	D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
	D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
	D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
	D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, D3D11_FILTER_MIN_MAG_MIP_LINEAR,
};

static UINT row_pitches[14] = {4, 16, 8, 12, 4, 2, 2, 2, 2, 1, 2, 2, 4, 4};

# ifndef NDEBUG

#  define D3D11_CALL_DEBUG(fn, err) \
do \
{ \
	if (err) \
		d3d11_errors(err, #fn, __FILE__, __LINE__); \
} while (0)

# else

#  define D3D11_CALL_DEBUG(fn, res) \
do \
{ \
	(void)res; \
} while (0)

# endif

#define D3D11_CALL(fn, ...) do { HRESULT err = fn(__VA_ARGS__); D3D11_CALL_DEBUG(fn, err); } while (0)

static void d3d11_errors(uint32_t err, const char *fn, const char *file, int line)
{
#define TEST_ERR(code) \
	case code: \
		out = #code; \
		break;

	char buf[256];
	const char *out;
	switch (err)
	{
		/*TEST_ERR(D3D11_ERROR_FILE_NOT_FOUND)
		TEST_ERR(D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS)
		TEST_ERR(D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS)
		TEST_ERR(D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD)*/
		TEST_ERR(D3DERR_INVALIDCALL)
		TEST_ERR(D3DERR_WASSTILLDRAWING);
		TEST_ERR(E_FAIL)
		TEST_ERR(E_INVALIDARG)
		TEST_ERR(E_OUTOFMEMORY)
		TEST_ERR(E_NOTIMPL)
		TEST_ERR(S_FALSE)
		default:
			snprintf(buf, sizeof(buf), "unknown error: %" PRIu32, err);
			out = buf;
			break;
	}
	if (gfx_error_callback)
		gfx_error_callback("%s@%s:%d %s", fn, file, line, out);

#undef TEST_ERR
}

static bool create_default_render_target_view(gfx_device_t *device)
{
	ID3D11Texture2D *back_buffer_ptr = NULL;
	if (FAILED(IDXGISwapChain_GetBuffer(D3D11_DEVICE->swap_chain, 0, &UIID_ID3D11Texture2D, (LPVOID*)&back_buffer_ptr)))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to get swap chain buffer");
		goto err;
	}
	if (FAILED(ID3D11Device_CreateRenderTargetView(D3D11_DEVICE->d3ddev, (ID3D11Resource*)back_buffer_ptr, NULL, &D3D11_DEVICE->default_render_target_view)))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to create default render target view");
		goto err;
	}
	ID3D11Texture2D_Release(back_buffer_ptr);
	return true;

err:
	ID3D11Texture2D_Release(back_buffer_ptr);
	return false;
}

static bool create_default_depth_stencil_view(gfx_device_t *device)
{
	ID3D11Texture2D *depth_stencil_buffer = NULL;
	D3D11_TEXTURE2D_DESC tex_desc;
	D3D11_DEPTH_STENCIL_VIEW_DESC view_desc;

	memset(&tex_desc, 0, sizeof(tex_desc));
	tex_desc.Width = device->window->width;
	tex_desc.Height = device->window->height;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	if (FAILED(ID3D11Device_CreateTexture2D(D3D11_DEVICE->d3ddev, &tex_desc, NULL, &depth_stencil_buffer)))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to create default depth stencil texture");
		goto err;
	}

	memset(&view_desc, 0, sizeof(view_desc));
	view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	if (FAILED(ID3D11Device_CreateDepthStencilView(D3D11_DEVICE->d3ddev, (ID3D11Resource*)depth_stencil_buffer, &view_desc, &D3D11_DEVICE->default_depth_stencil_view)))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to create default depth stencil view");
		goto err;
	}
	ID3D11Texture2D_Release(depth_stencil_buffer);
	return true;

err:
	if (depth_stencil_buffer)
		ID3D11DepthStencilView_Release(depth_stencil_buffer);
	return false;
}

static void update_primitive_topology(gfx_device_t *device, enum gfx_primitive_type primitive)
{
	if (D3D11_DEVICE->primitive == primitive)
		return;
	ID3D11DeviceContext_IASetPrimitiveTopology(D3D11_DEVICE->d3dctx, primitives[primitive]);
	D3D11_DEVICE->primitive = primitive;
}

static bool d3d11_ctr(gfx_device_t *device, gfx_window_t *window)
{
	if (!gfx_device_vtable.ctr(device, window))
		return false;
	D3D11_DEVICE->default_depth_stencil_view = NULL;
	D3D11_DEVICE->default_render_target_view = NULL;
	D3D11_DEVICE->primitive = (enum gfx_primitive_type)-1;
	D3D11_DEVICE->rasterizer_state = NULL;
	D3D11_DEVICE->pipeline_state = 0;
	D3D11_DEVICE->state_idx = 0;
	if (!create_default_render_target_view(device))
		goto err;
	if (!create_default_depth_stencil_view(device))
		goto err;
	return true;

err:
	if (D3D11_DEVICE->default_depth_stencil_view)
		ID3D11DepthStencilView_Release(D3D11_DEVICE->default_depth_stencil_view);
	if (D3D11_DEVICE->default_render_target_view)
		ID3D11RenderTargetView_Release(D3D11_DEVICE->default_render_target_view);
	return false;
}

static void d3d11_dtr(gfx_device_t *device)
{
	gfx_device_vtable.dtr(device);
}

static void d3d11_tick(gfx_device_t *device)
{
	gfx_device_vtable.tick(device);
}

static void d3d11_clear_color(gfx_device_t *device, const gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, vec4f_t color)
{
	if (!render_target)
	{
		assert(attachment == GFX_RENDERTARGET_ATTACHMENT_COLOR0);
		ID3D11DeviceContext_ClearRenderTargetView(D3D11_DEVICE->d3dctx, D3D11_DEVICE->default_render_target_view, &color.x);
	}
	else
	{
		assert(render_target->handle.ptr);
		assert(render_target->colors[attachment - GFX_RENDERTARGET_ATTACHMENT_COLOR0].handle.ptr);
		ID3D11DeviceContext_ClearRenderTargetView(D3D11_DEVICE->d3dctx, (ID3D11RenderTargetView*)render_target->colors[attachment - GFX_RENDERTARGET_ATTACHMENT_COLOR0].handle.ptr, &color.x);
	}
}

static void d3d11_clear_depth_stencil(gfx_device_t *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil)
{
	if (!render_target)
	{
		ID3D11DeviceContext_ClearDepthStencilView(D3D11_DEVICE->d3dctx, D3D11_DEVICE->default_depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}
	else
	{
		assert(render_target->handle.ptr);
		assert(render_target->depth_stencil.handle.ptr);
		ID3D11DeviceContext_ClearDepthStencilView(D3D11_DEVICE->d3dctx, (ID3D11DepthStencilView*)render_target->depth_stencil.handle.ptr, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}
}

static void d3d11_draw_indexed_instanced(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	update_primitive_topology(device, primitive);
	ID3D11DeviceContext_DrawIndexedInstanced(D3D11_DEVICE->d3dctx, count, prim_count, offset, 0, 0);
#ifndef NDEBUG
	switch (primitive)
	{
		case GFX_PRIMITIVE_TRIANGLES:
			device->triangles_count += count / 3 * prim_count;
			break;
		case GFX_PRIMITIVE_POINTS:
			device->points_count += count * prim_count;
			break;
		case GFX_PRIMITIVE_LINES:
			device->lines_count += count / 2 * prim_count;
			break;
	}
	device->draw_calls_count++;
#endif
}

static void d3d11_draw_instanced(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	update_primitive_topology(device, primitive);
	ID3D11DeviceContext_DrawInstanced(D3D11_DEVICE->d3dctx, count, prim_count, offset, 0);
#ifndef NDEBUG
	switch (primitive)
	{
		case GFX_PRIMITIVE_TRIANGLES:
			device->triangles_count += count / 3;
			break;
		case GFX_PRIMITIVE_POINTS:
			device->points_count += count;
			break;
		case GFX_PRIMITIVE_LINES:
			device->lines_count += count / 2;
			break;
	}
	device->draw_calls_count++;
#endif
}

static void d3d11_draw_indexed(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset)
{
	update_primitive_topology(device, primitive);
	ID3D11DeviceContext_DrawIndexed(D3D11_DEVICE->d3dctx, count, offset, 0);
#ifndef NDEBUG
	switch (primitive)
	{
		case GFX_PRIMITIVE_TRIANGLES:
			device->triangles_count += count / 3;
			break;
		case GFX_PRIMITIVE_POINTS:
			device->points_count += count;
			break;
		case GFX_PRIMITIVE_LINES:
			device->lines_count += count / 2;
			break;
	}
	device->draw_calls_count++;
#endif
}

static void d3d11_draw(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset)
{
	update_primitive_topology(device, primitive);
	ID3D11DeviceContext_Draw(D3D11_DEVICE->d3dctx, count, offset);
#ifndef NDEBUG
	switch (primitive)
	{
		case GFX_PRIMITIVE_TRIANGLES:
			device->triangles_count += count / 3;
			break;
		case GFX_PRIMITIVE_POINTS:
			device->points_count += count;
			break;
		case GFX_PRIMITIVE_LINES:
			device->lines_count += count / 2;
			break;
	}
	device->draw_calls_count++;
#endif
}

static void d3d11_create_blend_state(gfx_device_t *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a)
{
	assert(!state->handle.ptr);
	state->device = device;
	state->enabled = enabled;
	state->src_c = src_c;
	state->dst_c = dst_c;
	state->src_a = src_a;
	state->dst_a = dst_a;
	state->equation_c = equation_c;
	state->equation_a = equation_a;
	D3D11_BLEND_DESC desc;
	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = true;
	for (uint32_t i = 0; i < 8; ++i)
	{
		desc.RenderTarget[i].BlendEnable = enabled;
		desc.RenderTarget[i].SrcBlend = blend_functions[src_c];
		desc.RenderTarget[i].DestBlend = blend_functions[dst_c];
		desc.RenderTarget[i].BlendOp = blend_equations[equation_c];
		desc.RenderTarget[i].SrcBlendAlpha = blend_functions[src_a];
		desc.RenderTarget[i].DestBlendAlpha = blend_functions[dst_a];
		desc.RenderTarget[i].BlendOpAlpha = blend_equations[equation_a];
		desc.RenderTarget[i].RenderTargetWriteMask = 0xFF;
	}
	D3D11_CALL(ID3D11Device_CreateBlendState, D3D11_DEVICE->d3ddev, &desc, (ID3D11BlendState**)&state->handle.ptr);
}

static void d3d11_bind_blend_state(gfx_device_t *device, const gfx_blend_state_t *state)
{
	const float blend_factor[4] = {1, 1, 1, 1};
	const uint32_t sample_mask = 0xffffffffu;
	ID3D11DeviceContext_OMSetBlendState(D3D11_DEVICE->d3dctx, (ID3D11BlendState*)state->handle.ptr, blend_factor, sample_mask);
}

static void d3d11_delete_blend_state(gfx_device_t *device, gfx_blend_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	ID3D11BlendState_Release((ID3D11BlendState*)state->handle.ptr);
	//queue ?
	state->handle.ptr = NULL;
}

static void d3d11_create_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass)
{
	assert(!state->handle.ptr);
	state->device = device;
	state->depth_write = depth_write;
	state->depth_test = depth_test;
	state->depth_compare = depth_compare;
	state->stencil_enabled = stencil_enabled;
	state->stencil_write_mask = stencil_write_mask;
	state->stencil_compare = stencil_compare;
	state->stencil_reference = stencil_reference;
	state->stencil_compare_mask = stencil_compare_mask;
	state->stencil_fail = stencil_fail;
	state->stencil_zfail = stencil_zfail;
	state->stencil_pass = stencil_pass;
	D3D11_DEPTH_STENCIL_DESC desc;
	desc.DepthEnable = depth_test;
	desc.DepthWriteMask = depth_write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = compare_functions[depth_compare];
	desc.StencilEnable = stencil_enabled;
	desc.StencilReadMask = stencil_compare_mask;
	desc.StencilWriteMask = stencil_write_mask;
	desc.FrontFace.StencilFailOp = stencil_operations[stencil_fail];
	desc.FrontFace.StencilDepthFailOp = stencil_operations[stencil_zfail];
	desc.FrontFace.StencilPassOp = stencil_operations[stencil_pass];
	desc.FrontFace.StencilFunc = compare_functions[stencil_compare];
	desc.BackFace.StencilFailOp = stencil_operations[stencil_fail];
	desc.BackFace.StencilDepthFailOp = stencil_operations[stencil_zfail];
	desc.BackFace.StencilPassOp = stencil_operations[stencil_pass];
	desc.BackFace.StencilFunc = compare_functions[stencil_compare];
	D3D11_CALL(ID3D11Device_CreateDepthStencilState, D3D11_DEVICE->d3ddev, &desc, (ID3D11DepthStencilState**)&state->handle.ptr);
}

static void d3d11_bind_depth_stencil_state(gfx_device_t *device, const gfx_depth_stencil_state_t *state)
{
	assert(state->handle.ptr);
	ID3D11DeviceContext_OMSetDepthStencilState(D3D11_DEVICE->d3dctx, (ID3D11DepthStencilState*)state->handle.ptr, state->stencil_reference);
}

static void d3d11_delete_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	ID3D11DepthStencilState_Release((ID3D11DepthStencilState*)state->handle.ptr);
	//queue ?
	state->handle.ptr = NULL;
}

static void d3d11_create_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face, bool scissor)
{
	assert(!state->handle.ptr);
	state->device = device;
	state->fill_mode = fill_mode;
	state->cull_mode = cull_mode;
	state->front_face = front_face;
	D3D11_RASTERIZER_DESC desc;
	desc.FillMode = fill_modes[fill_mode];
	desc.CullMode = cull_modes[cull_mode];
	desc.FrontCounterClockwise = (front_face == GFX_FRONT_CCW);
	desc.DepthBias = 0;
	desc.DepthBiasClamp = 0;
	desc.SlopeScaledDepthBias = 0;
	desc.DepthClipEnable = true;
	desc.ScissorEnable = scissor;
	desc.MultisampleEnable = true;
	desc.AntialiasedLineEnable = true;
	D3D11_CALL(ID3D11Device_CreateRasterizerState, D3D11_DEVICE->d3ddev, &desc, (ID3D11RasterizerState**)&state->handle.ptr);
}

static void d3d11_bind_rasterizer_state(gfx_device_t *device, const gfx_rasterizer_state_t *state)
{
	if (D3D11_DEVICE->rasterizer_state == state)
		return;
	D3D11_DEVICE->rasterizer_state = state;
	ID3D11DeviceContext_RSSetState(D3D11_DEVICE->d3dctx, (ID3D11RasterizerState*)state->handle.ptr);
}

static void d3d11_delete_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	ID3D11RasterizerState_Release((ID3D11RasterizerState*)state->handle.ptr);
	//queue ?
}

static void d3d11_create_buffer(gfx_device_t *device, gfx_buffer_t *buffer, enum gfx_buffer_type type, const void *data, uint32_t size, enum gfx_buffer_usage usage)
{
	assert(!buffer->handle.ptr);
	buffer->device = device;
	buffer->usage = usage;
	buffer->type = type;
	buffer->size = size;
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	if (!size)
		size = 1;
	if (type == GFX_BUFFER_UNIFORM)
	{
		uint32_t md = size % 16;
		if (md)
			size += 16 - md;
	}
	desc.ByteWidth = size;
	desc.Usage = buffer_usages[usage];
	desc.BindFlags = buffer_types[type];
	desc.CPUAccessFlags = (usage == GFX_BUFFER_IMMUTABLE ? 0 : D3D11_CPU_ACCESS_WRITE);
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA init_data;
	if (data)
	{
		init_data.pSysMem = data;
		init_data.SysMemPitch = 0;
		init_data.SysMemSlicePitch = 0;
	}
	D3D11_CALL(ID3D11Device_CreateBuffer, D3D11_DEVICE->d3ddev, &desc, data ? &init_data : NULL, (ID3D11Buffer**)&buffer->handle.ptr);
}

static void d3d11_set_buffer_data(gfx_device_t *device, gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset)
{
	assert(buffer->handle.ptr);
	D3D11_MAPPED_SUBRESOURCE sub_resource;
	D3D11_CALL(ID3D11DeviceContext_Map, D3D11_DEVICE->d3dctx, (ID3D11Resource*)buffer->handle.ptr, 0, D3D11_MAP_WRITE, 0, &sub_resource);
	memcpy(((uint8_t*)sub_resource.pData) + offset, data, size);
	ID3D11DeviceContext_Unmap(D3D11_DEVICE->d3dctx, (ID3D11Resource*)buffer->handle.ptr, 0);
}

static void d3d11_delete_buffer(gfx_device_t *device, gfx_buffer_t *buffer)
{
	if (!buffer || !buffer->handle.ptr)
		return;
	ID3D11Buffer_Release((ID3D11Buffer*)buffer->handle.ptr);
	//queue ?
	buffer->handle.ptr = NULL;
}

static void d3d11_create_attributes_state(gfx_device_t *device, gfx_attributes_state_t *state, const gfx_attribute_bind_t *binds, uint32_t count, const gfx_buffer_t *index_buffer, enum gfx_index_type index_type)
{
	assert(!state->handle.ptr);
	state->device = device;
	memcpy(state->binds, binds, sizeof(*binds) * count);
	state->count = count;
	state->index_buffer = index_buffer;
	state->index_type = index_type;
	state->handle.u64 = 1;
}

static void d3d11_bind_attributes_state(gfx_device_t *device, const gfx_attributes_state_t *state, const gfx_input_layout_t *input_layout)
{
	assert(state->handle.ptr);
	assert(input_layout->handle.ptr);
	ID3D11Buffer *buffers[8];
	unsigned strides[8];
	unsigned offsets[8];
	for (size_t i = 0; i < sizeof(state->binds) / sizeof(*state->binds); ++i)
	{
		if (state->binds[i].buffer)
		{
			buffers[i] = (ID3D11Buffer*)state->binds[i].buffer->handle.ptr;
			strides[i] = state->binds[i].stride;
			offsets[i] = state->binds[i].offset;
		}
		else
		{
			buffers[i] = NULL;
		}
	}
	ID3D11DeviceContext_IASetInputLayout(D3D11_DEVICE->d3dctx, (ID3D11InputLayout*)input_layout->handle.ptr);
	ID3D11DeviceContext_IASetVertexBuffers(D3D11_DEVICE->d3dctx, 0, 8, &buffers[0], strides, offsets);
	if (state->index_buffer)
		ID3D11DeviceContext_IASetIndexBuffer(D3D11_DEVICE->d3dctx, (ID3D11Buffer*)state->index_buffer->handle.ptr, index_types[state->index_type], 0);
}

static void d3d11_delete_attributes_state(gfx_device_t *device, gfx_attributes_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	//XXX async
	state->handle.ptr = NULL;
}

static void d3d11_create_input_layout(gfx_device_t *device, gfx_input_layout_t *input_layout, const gfx_input_layout_bind_t *binds, uint32_t count, const gfx_program_t *program)
{
	assert(!input_layout->handle.ptr);
	input_layout->device = device;
	memcpy(input_layout->binds, binds, sizeof(*binds) * count);
	input_layout->count = count;
	D3D11_INPUT_ELEMENT_DESC desc[8];
	uint32_t desc_nb = 0;
	for (uint32_t i = 0; i < count; ++i)
	{
		if (binds[i].type == GFX_ATTR_DISABLED)
			continue;
		desc[desc_nb].SemanticName = "VS_INPUT";
		desc[desc_nb].SemanticIndex = desc_nb; //XXX: handle matrix
		desc[desc_nb].Format = attribute_types[binds[i].type];
		desc[desc_nb].InputSlot = i;
		desc[desc_nb].AlignedByteOffset = 0;
		desc[desc_nb].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		desc[desc_nb].InstanceDataStepRate = 0;
		desc_nb++;
	}
	D3D11_CALL(ID3D11Device_CreateInputLayout, D3D11_DEVICE->d3ddev, desc, desc_nb, program->code, program->code_size, (ID3D11InputLayout**)&input_layout->handle.ptr);
}

static void d3d11_delete_input_layout(gfx_device_t *device, gfx_input_layout_t *input_layout)
{
	if (!input_layout || !input_layout->handle.ptr)
		return;
	ID3D11InputLayout_Release((ID3D11InputLayout*)input_layout->handle.ptr);
}

static void d3d11_create_texture(gfx_device_t *device, gfx_texture_t *texture, enum gfx_texture_type type, enum gfx_format format, uint8_t lod, uint32_t width, uint32_t height, uint32_t depth)
{
	assert(!texture->handle.ptr);
	texture->device = device;
	texture->format = format;
	texture->type = type;
	texture->width = width;
	texture->height = height;
	texture->depth = depth;
	texture->lod = lod;
	texture->addressing_s = GFX_TEXTURE_ADDRESSING_REPEAT;
	texture->addressing_t = GFX_TEXTURE_ADDRESSING_REPEAT;
	texture->addressing_r = GFX_TEXTURE_ADDRESSING_REPEAT;
	texture->min_filtering = GFX_FILTERING_NEAREST;
	texture->mag_filtering = GFX_FILTERING_LINEAR;
	texture->mip_filtering = GFX_FILTERING_LINEAR;
	texture->anisotropy = 1;
	texture->min_level = 0;
	texture->max_level = 1000;
	switch (type)
	{
		case GFX_TEXTURE_3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			memset(&desc, 0, sizeof(desc));
			desc.Width = width;
			desc.Height = height;
			desc.Depth = depth;
			desc.MipLevels = lod;
			desc.Format = formats[format];
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			D3D11_CALL(ID3D11Device_CreateTexture3D, D3D11_DEVICE->d3ddev, &desc, NULL, (ID3D11Texture3D**)&texture->handle.ptr);
			break;
		}
		case GFX_TEXTURE_2D_ARRAY_MS:
			/* FALLTHROUGH */
		case GFX_TEXTURE_2D_MS:
			/* FALLTHROUGH */
		case GFX_TEXTURE_2D_ARRAY:
			/* FALLTHROUGH */
		case GFX_TEXTURE_2D:
		{
			D3D11_TEXTURE2D_DESC desc;
			memset(&desc, 0, sizeof(desc));
			desc.Width = width;
			desc.Height = height;
			desc.MipLevels = (type == GFX_TEXTURE_2D_MS || type == GFX_TEXTURE_2D_ARRAY_MS) ? 1 : lod;
			desc.ArraySize = (type == GFX_TEXTURE_2D || type == GFX_TEXTURE_2D_MS) ? 1 : depth;
			desc.Format = formats[format];
			desc.SampleDesc.Count = (type == GFX_TEXTURE_2D || type == GFX_TEXTURE_2D_ARRAY) ? 1 : lod;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			if (format == GFX_DEPTH24_STENCIL8)
			{
				desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
				desc.CPUAccessFlags = 0;
			}
			else
			{
				desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				if (format != GFX_BC1_RGB && format != GFX_BC1_RGBA && format != GFX_BC2_RGBA && format != GFX_BC3_RGBA && format != GFX_B4G4R4A4)
					desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
				if (type == GFX_TEXTURE_2D_MS || type == GFX_TEXTURE_2D_ARRAY_MS)
					desc.CPUAccessFlags = 0;
				else
					desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			desc.MiscFlags = 0;
			D3D11_CALL(ID3D11Device_CreateTexture2D, D3D11_DEVICE->d3ddev, &desc, NULL, (ID3D11Texture2D**)&texture->handle.ptr);
			break;
		}
	}
	texture->sampler.u64 = 1;
	texture->view.u64 = 1;
}

static void d3d11_set_texture_data(gfx_device_t *device, gfx_texture_t *texture, uint8_t lod, uint32_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t size, const void *data)
{
	assert(texture->handle.ptr);
	uint32_t row_pitch = texture->width;
	for (uint32_t i = 0; i < lod; ++i)
		row_pitch >>= 1;
	if (!row_pitch)
		row_pitch = 1;
	row_pitch *= row_pitches[texture->format];
	switch (texture->type)
	{
		case GFX_TEXTURE_3D:
			ID3D11DeviceContext_UpdateSubresource(D3D11_DEVICE->d3dctx, (ID3D11Resource*)texture->handle.ptr, D3D11CalcSubresource(lod, 0, texture->lod), NULL, data, row_pitch, row_pitch * texture->height);
			break;
		case GFX_TEXTURE_2D_ARRAY_MS:
			assert(!"invalid type");
			break;
		case GFX_TEXTURE_2D_ARRAY:
			ID3D11DeviceContext_UpdateSubresource(D3D11_DEVICE->d3dctx, (ID3D11Resource*)texture->handle.ptr, D3D11CalcSubresource(lod, offset, texture->lod), NULL, data, row_pitch, 0);
			break;
		case GFX_TEXTURE_2D_MS:
			assert(!"invalid type");
			break;
		case GFX_TEXTURE_2D:
			ID3D11DeviceContext_UpdateSubresource(D3D11_DEVICE->d3dctx, (ID3D11Resource*)texture->handle.ptr, D3D11CalcSubresource(lod, 0, texture->lod), NULL, data, row_pitch, 0);
			break;
	}
}

static void d3d11_set_texture_addressing(gfx_device_t *device, gfx_texture_t *texture, enum gfx_texture_addressing addressing_s, enum gfx_texture_addressing addressing_t, enum gfx_texture_addressing addressing_r)
{
	(void)device;
	assert(texture->handle.ptr);
	texture->addressing_s = addressing_s;
	texture->addressing_t = addressing_t;
	texture->addressing_r = addressing_r;
}

static void d3d11_set_texture_filtering(gfx_device_t *device, gfx_texture_t *texture, enum gfx_filtering min_filtering, enum gfx_filtering mag_filtering, enum gfx_filtering mip_filtering)
{
	(void)device;
	assert(texture->handle.ptr);
	texture->min_filtering = min_filtering;
	texture->mag_filtering = mag_filtering;
	texture->mip_filtering = mip_filtering;
}

static void d3d11_set_texture_anisotropy(gfx_device_t *device, gfx_texture_t *texture, uint32_t anisotropy)
{
	(void)device;
	assert(texture->handle.ptr);
	texture->anisotropy = anisotropy;
}

static void d3d11_set_texture_levels(gfx_device_t *device, gfx_texture_t *texture, uint32_t min_level, uint32_t max_level)
{
	(void)device;
	assert(texture->handle.ptr);
	texture->min_level = min_level;
	texture->max_level = max_level;
}

static void d3d11_delete_texture(gfx_device_t *device, gfx_texture_t *texture)
{
	if (!texture || !texture->handle.ptr)
		return;
	switch (texture->type)
	{
		case GFX_TEXTURE_3D:
			ID3D11Texture3D_Release((ID3D11Texture3D*)texture->handle.ptr);
			break;
		case GFX_TEXTURE_2D_ARRAY_MS:
			/* FALLTHROUGH */
		case GFX_TEXTURE_2D_ARRAY:
			/* FALLTHROUGH */
		case GFX_TEXTURE_2D_MS:
			/* FALLTHROUGH */
		case GFX_TEXTURE_2D:
			ID3D11Texture2D_Release((ID3D11Texture2D*)texture->handle.ptr);
			break;
	}
	//queue ?
	texture->handle.ptr = NULL;
}

static bool d3d11_create_shader(gfx_device_t *device, gfx_shader_t *shader, enum gfx_shader_type type, const uint8_t *data, uint32_t len)
{
	assert(!shader->handle.ptr);
	shader->device = device;
	shader->type = type;
	ID3D10Blob *error_message = NULL;
	ID3D10Blob *shader_data;
	const char *type_str;
	switch (type)
	{
		case GFX_SHADER_VERTEX:
			type_str = "vs_5_0";
			break;
		case GFX_SHADER_FRAGMENT:
			type_str = "ps_5_0";
			break;
		case GFX_SHADER_GEOMETRY:
			type_str = "gs_5_0";
			break;
		default:
			return false;
	}
	if (D3DCompile(data, len, NULL, NULL, NULL, "main", type_str, D3D10_SHADER_ENABLE_STRICTNESS, 0, &shader_data, &error_message) != S_OK)
	{
		if (error_message)
		{
			if (gfx_error_callback)
				gfx_error_callback("%s", (char*)ID3D10Blob_GetBufferPointer(error_message));
			ID3D10Blob_Release(error_message);
		}
		else
		{
			if (gfx_error_callback)
				gfx_error_callback("failed to compile shader");
		}
		return false;
	}
	switch (type)
	{
		case GFX_SHADER_VERTEX:
			D3D11_CALL(ID3D11Device_CreateVertexShader, D3D11_DEVICE->d3ddev, ID3D10Blob_GetBufferPointer(shader_data), ID3D10Blob_GetBufferSize(shader_data), NULL, (ID3D11VertexShader**)&shader->handle.ptr);
			break;
		case GFX_SHADER_FRAGMENT:
			D3D11_CALL(ID3D11Device_CreatePixelShader, D3D11_DEVICE->d3ddev, ID3D10Blob_GetBufferPointer(shader_data), ID3D10Blob_GetBufferSize(shader_data), NULL, (ID3D11PixelShader**)&shader->handle.ptr);
			break;
		case GFX_SHADER_GEOMETRY:
			D3D11_CALL(ID3D11Device_CreateGeometryShader, D3D11_DEVICE->d3ddev, ID3D10Blob_GetBufferPointer(shader_data), ID3D10Blob_GetBufferSize(shader_data), NULL, (ID3D11GeometryShader**)&shader->handle.ptr);
			break;
	}
	shader->code_size = ID3D10Blob_GetBufferSize(shader_data);
	shader->code = (uint8_t*)malloc(shader->code_size);
	if (!shader->code)
		return false;
	memcpy(shader->code, ID3D10Blob_GetBufferPointer(shader_data), ID3D10Blob_GetBufferSize(shader_data));
	if (error_message)
		ID3D10Blob_Release(error_message);
	ID3D10Blob_Release(shader_data);
	return true;
}

static void d3d11_delete_shader(gfx_device_t *device, gfx_shader_t *shader)
{
	if (!shader || !shader->handle.ptr)
		return;
	free(shader->code);
	ID3D11PixelShader_Release((ID3D11PixelShader*)shader->handle.ptr);
	shader->handle.ptr = NULL;
	//queue
}

static bool d3d11_create_program(gfx_device_t *device, gfx_program_t *program, const gfx_shader_t *vertex_shader, const gfx_shader_t *fragment_shader, const gfx_shader_t *geometry_shader, const gfx_program_attribute_t *attributes, const gfx_program_constant_t *constants, const gfx_program_sampler_t *samplers)
{
	(void)attributes;
	(void)constants;
	(void)samplers;
	assert(!program->handle.u64);
	assert(vertex_shader->handle.u64);
	assert(fragment_shader->handle.u64);
	program->device = device;
	program->vertex_shader = vertex_shader->handle;
	ID3D11VertexShader_AddRef((ID3D11VertexShader*)program->vertex_shader.ptr);
	program->fragment_shader = fragment_shader->handle;
	ID3D11PixelShader_AddRef((ID3D11PixelShader*)program->fragment_shader.ptr);
	if (geometry_shader)
	{
		program->geometry_shader = geometry_shader->handle;
		ID3D11GeometryShader_AddRef((ID3D11GeometryShader*)program->geometry_shader.ptr);
	}
	else
	{
		program->geometry_shader.ptr = NULL;
	}
	program->code_size = vertex_shader->code_size;
	program->code = (uint8_t*)malloc(program->code_size);
	if (!program->code)
		return false;
	memcpy(program->code, vertex_shader->code, program->code_size);
	return true;
}

static void d3d11_bind_program(gfx_device_t *device, const gfx_program_t *program)
{
	ID3D11DeviceContext_VSSetShader(D3D11_DEVICE->d3dctx, (ID3D11VertexShader*)program->vertex_shader.ptr, NULL, 0);
	ID3D11DeviceContext_PSSetShader(D3D11_DEVICE->d3dctx, (ID3D11PixelShader*)program->fragment_shader.ptr, NULL, 0);
	if (program->geometry_shader.ptr)
		ID3D11DeviceContext_GSSetShader(D3D11_DEVICE->d3dctx, (ID3D11GeometryShader*)program->fragment_shader.ptr, NULL, 0);
}

static void d3d11_delete_program(gfx_device_t *device, gfx_program_t *program)
{
	ID3D11VertexShader_Release((ID3D11VertexShader*)program->vertex_shader.ptr);
	ID3D11PixelShader_Release((ID3D11PixelShader*)program->fragment_shader.ptr);
	if (program->geometry_shader.ptr)
		ID3D11GeometryShader_Release((ID3D11GeometryShader*)program->geometry_shader.ptr);
}

static void d3d11_bind_constant(gfx_device_t *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
	assert(offset % 16 == 0);
	offset /= 16;
	offset /= 16;
	size_t md = size % 16;
	if (md)
		size += 16 - md;
	ID3D11DeviceContext4_VSSetConstantBuffers1(D3D11_DEVICE->d3dctx, bind, 1, (ID3D11Buffer**)&buffer->handle.ptr, &offset, &size);
	ID3D11DeviceContext4_PSSetConstantBuffers1(D3D11_DEVICE->d3dctx, bind, 1, (ID3D11Buffer**)&buffer->handle.ptr, &offset, &size);
	ID3D11DeviceContext4_GSSetConstantBuffers1(D3D11_DEVICE->d3dctx, bind, 1, (ID3D11Buffer**)&buffer->handle.ptr, &offset, &size);
}

static void d3d11_bind_samplers(gfx_device_t *device, uint32_t start, uint32_t count, const gfx_texture_t **textures)
{
	ID3D11SamplerState *sampler_states[16];
	ID3D11ShaderResourceView *resource_views[16];
	for (size_t i = 0; i < count; ++i)
	{
		if (textures[i])
		{
			if (textures[i]->view.u64 == 1)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC desc;
				desc.Format = formats[textures[i]->format];
				switch (textures[i]->type)
				{
					case GFX_TEXTURE_3D:
						desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
						desc.Texture3D.MostDetailedMip = 0;
						desc.Texture3D.MipLevels = -1;
						break;
					case GFX_TEXTURE_2D_ARRAY_MS:
						desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
						desc.Texture2DMSArray.FirstArraySlice = 0;
						desc.Texture2DMSArray.ArraySize = textures[i]->depth;
						break;
					case GFX_TEXTURE_2D_ARRAY:
						desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
						desc.Texture2DArray.MostDetailedMip = 0;
						desc.Texture2DArray.MipLevels = -1;
						desc.Texture2DArray.FirstArraySlice = 0;
						desc.Texture2DArray.ArraySize = textures[i]->depth;
						break;
					case GFX_TEXTURE_2D_MS:
						desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
						break;
					case GFX_TEXTURE_2D:
						desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
						desc.Texture2D.MostDetailedMip = 0;
						desc.Texture2D.MipLevels = -1;
						break;
				}
				D3D11_CALL(ID3D11Device_CreateShaderResourceView, D3D11_DEVICE->d3ddev, (ID3D11Resource*)textures[i]->handle.ptr, &desc, (ID3D11ShaderResourceView**)&textures[i]->view.ptr);
				assert(textures[i]->view.u64 != 1);
			}
			if (textures[i]->sampler.u64 == 1)
			{
				D3D11_SAMPLER_DESC desc;
				if (textures[i]->anisotropy)
					desc.Filter = D3D11_FILTER_ANISOTROPIC;
				else
					desc.Filter = filtering[textures[i]->min_filtering + 3 * (textures[i]->mag_filtering + 3 * textures[i]->mip_filtering)];
				desc.AddressU = texture_addressings[textures[i]->addressing_s];
				desc.AddressV = texture_addressings[textures[i]->addressing_t];
				desc.AddressW = texture_addressings[textures[i]->addressing_r];
				desc.MipLODBias = 0;
				desc.MaxAnisotropy = textures[i]->anisotropy;
				desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
				desc.BorderColor[0] = 0;
				desc.BorderColor[1] = 0;
				desc.BorderColor[2] = 0;
				desc.BorderColor[3] = 0;
				desc.MinLOD = textures[i]->min_level;
				desc.MaxLOD = textures[i]->max_level;
				D3D11_CALL(ID3D11Device_CreateSamplerState, D3D11_DEVICE->d3ddev, &desc, (ID3D11SamplerState**)&textures[i]->sampler.ptr);
				assert(textures[i]->sampler.u64 != 1);
			}
			sampler_states[i] = (ID3D11SamplerState*)textures[i]->sampler.ptr;
			resource_views[i] = (ID3D11ShaderResourceView*)textures[i]->view.ptr;
		}
		else
		{
			sampler_states[i] = NULL;
			resource_views[i] = NULL;
		}
	}
	ID3D11DeviceContext_VSSetSamplers(D3D11_DEVICE->d3dctx, start, count, sampler_states);
	ID3D11DeviceContext_PSSetSamplers(D3D11_DEVICE->d3dctx, start, count, sampler_states);
	ID3D11DeviceContext_GSSetSamplers(D3D11_DEVICE->d3dctx, start, count, sampler_states);
	ID3D11DeviceContext_VSSetShaderResources(D3D11_DEVICE->d3dctx, start, count, resource_views);
	ID3D11DeviceContext_PSSetShaderResources(D3D11_DEVICE->d3dctx, start, count, resource_views);
	ID3D11DeviceContext_GSSetShaderResources(D3D11_DEVICE->d3dctx, start, count, resource_views);
}

static void d3d11_create_render_target(gfx_device_t *device, gfx_render_target_t *render_target)
{
	assert(!render_target->handle.ptr);
	render_target->device = device;
	render_target->handle.ptr = (void*)1;
	for (size_t i = 0; i < sizeof(render_target->colors) / sizeof(*render_target->colors); ++i)
		render_target->colors[i].handle.ptr = NULL;
	render_target->depth_stencil.handle.ptr = NULL;
}

static void d3d11_delete_render_target(gfx_device_t *device, gfx_render_target_t *render_target)
{
	if (!render_target || !render_target->handle.ptr)
		return;
	for (size_t i = 0; i < sizeof(render_target->colors) / sizeof(*render_target->colors); ++i)
	{
		if (render_target->colors[i].handle.ptr)
			ID3D11RenderTargetView_Release((ID3D11RenderTargetView*)render_target->colors[i].handle.ptr);
	}
	if (render_target->depth_stencil.handle.ptr)
		ID3D11DepthStencilView_Release((ID3D11DepthStencilView*)render_target->depth_stencil.handle.ptr);
}

static void d3d11_bind_render_target(gfx_device_t *device, const gfx_render_target_t *render_target)
{
	ID3D11RenderTargetView *views[sizeof(render_target->colors) / sizeof(*render_target->colors)];
	ID3D11DepthStencilView *depth_stencil;
	if (render_target)
	{
		assert(render_target->handle.ptr);
		for (size_t i = 0; i < sizeof(render_target->colors) / sizeof(*render_target->colors); ++i)
		{
			if (render_target->draw_buffers[i])
				views[i] = (ID3D11RenderTargetView*)render_target->colors[i].handle.ptr;
			else
				views[i] = NULL;
		}
		depth_stencil = (ID3D11DepthStencilView*)render_target->depth_stencil.handle.ptr;
	}
	else
	{
		views[0] = D3D11_DEVICE->default_render_target_view;
		for (size_t i = 1; i < 8; ++i)
			views[i] = NULL;
		depth_stencil = D3D11_DEVICE->default_depth_stencil_view;
	}
	ID3D11DeviceContext_OMSetRenderTargets(D3D11_DEVICE->d3dctx, 8, views, depth_stencil);
}

static void d3d11_set_render_target_texture(gfx_device_t *device, gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_texture_t *texture)
{
	assert(render_target->handle.ptr);
	if (attachment == GFX_RENDERTARGET_ATTACHMENT_DEPTH_STENCIL)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.Format = formats[texture->format];
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Flags = 0;
		desc.Texture2D.MipSlice = 0;
		D3D11_CALL(ID3D11Device_CreateDepthStencilView, D3D11_DEVICE->d3ddev, (ID3D11Resource*)texture->handle.ptr, &desc, (ID3D11DepthStencilView**)&render_target->depth_stencil.handle.ptr);
	}
	else
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.Format = formats[texture->format];
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;
		D3D11_CALL(ID3D11Device_CreateRenderTargetView, D3D11_DEVICE->d3ddev, (ID3D11Resource*)texture->handle.ptr, &desc, (ID3D11RenderTargetView**)&render_target->colors[attachment - GFX_RENDERTARGET_ATTACHMENT_COLOR0].handle.ptr);
	}
}

static void d3d11_set_render_target_draw_buffers(gfx_device_t *device, gfx_render_target_t *render_target, uint32_t *draw_buffers, uint32_t draw_buffers_count)
{
	assert(render_target->handle.ptr);
	for (size_t i = 0; i < draw_buffers_count; ++i)
		render_target->draw_buffers[i] = draw_buffers[i];
	render_target->draw_buffers_nb = draw_buffers_count;
}

static void d3d11_resolve_render_target(gfx_device_t *device, const gfx_render_target_t *src, const gfx_render_target_t *dst, uint32_t buffers, uint32_t color_src, uint32_t color_dst)
{
	//XXX
}

static void d3d11_create_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state, const gfx_program_t *program, const gfx_rasterizer_state_t *rasterizer, const gfx_depth_stencil_state_t *depth_stencil, const gfx_blend_state_t *blend, const gfx_input_layout_t *input_layout)
{
	assert(!state->handle.u64);
	state->handle.u64 = ++D3D11_DEVICE->state_idx;
	state->program = program;
	state->rasterizer_state = rasterizer;
	state->depth_stencil_state = depth_stencil;
	state->blend_state = blend;
	state->input_layout = input_layout;
}

static void d3d11_delete_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state)
{
	if (!state || !state->handle.u64)
		return;
	state->handle.u64 = 0;
}

static void d3d11_bind_pipeline_state(gfx_device_t *device, const gfx_pipeline_state_t *state)
{
	assert(state->handle.u64);
	if (D3D11_DEVICE->pipeline_state == state->handle.u64)
		return;
	D3D11_DEVICE->pipeline_state = state->handle.u64;
	d3d11_bind_program(device, state->program);
	d3d11_bind_rasterizer_state(device, state->rasterizer_state);
	d3d11_bind_depth_stencil_state(device, state->depth_stencil_state);
	d3d11_bind_blend_state(device, state->blend_state);
	//d3d11_bind_input_layout(device, state->input_layout);
}

static void d3d11_set_viewport(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = x;
	viewport.TopLeftY = y;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	ID3D11DeviceContext_RSSetViewports(D3D11_DEVICE->d3dctx, 1, &viewport);
}

static void d3d11_set_scissor(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	D3D11_RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;
	ID3D11DeviceContext_RSSetScissorRects(D3D11_DEVICE->d3dctx, 1, &rect);
	//enable in rasterizer state
}

static void d3d11_set_line_width(gfx_device_t *device, float line_width)
{
	(void)device;
	(void)line_width;
}

static void d3d11_set_point_size(gfx_device_t *device, float point_size)
{
	(void)device;
	(void)point_size;
}

gfx_device_vtable_t d3d11_vtable =
{
	GFX_DEVICE_VTABLE_DEF(d3d11)
};

gfx_device_t *gfx_d3d11_device_new(gfx_window_t *window, DXGI_SWAP_CHAIN_DESC *swap_chain_desc, IDXGISwapChain **swap_chain)
{
	gfx_d3d11_device_t *device = (gfx_d3d11_device_t*)malloc(sizeof(*device));
	if (!device)
		return NULL;
	gfx_device_t *dev = &device->device;
	dev->vtable = &d3d11_vtable;
	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_1;
	uint32_t creation_flags = 0;
#ifndef NDEBUG
	creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creation_flags, &feature_level, 1, D3D11_SDK_VERSION, swap_chain_desc, swap_chain, (ID3D11Device**)&device->d3ddev, NULL, (ID3D11DeviceContext**)&device->d3dctx)))
	{
		if (gfx_error_callback)
			gfx_error_callback("failed to create d3d device");
		goto err;
	}
	device->swap_chain = *swap_chain;
	if (!dev->vtable->ctr(dev, window))
		goto err;
	return dev;

err:
	dev->vtable->dtr(dev);
	free(device);
	return NULL;
}
