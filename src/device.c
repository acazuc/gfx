#include "device.h"
#include "device_vtable.h"
#include "window.h"
#include "config.h"
#include <stdlib.h>

#if 0
# include <stdio.h>
# define DEV_DEBUG printf("%s@%s:%d\n", __func__, __FILE__, __LINE__)
#else
# define DEV_DEBUG
#endif

void gfx_device_delete(gfx_device_t *device)
{
	if (!device)
		return;
	device->vtable->dtr(device);
	GFX_FREE(device);
}

static bool ctr(gfx_device_t *device, gfx_window_t *window)
{
	device->window = window;
	device->draw_calls_count = 0;
	device->triangles_count = 0;
	device->points_count = 0;
	device->lines_count = 0;
	device->max_samplers = 0;
	return true;
}

static void dtr(gfx_device_t *device)
{
	(void)device;
}

static void tick(gfx_device_t *device)
{
	device->draw_calls_count = 0;
	device->triangles_count = 0;
	device->points_count = 0;
	device->lines_count = 0;
}

gfx_device_vtable_t gfx_device_vtable =
{
	.ctr  = ctr,
	.dtr  = dtr,
	.tick = tick,
};

void gfx_device_tick(gfx_device_t *device)
{
	DEV_DEBUG;
	device->vtable->tick(device);
	DEV_DEBUG;
}

uint32_t gfx_get_uniform_buffer_size(gfx_device_t *device, uint32_t buffer_size)
{
	buffer_size += device->constant_alignment - 1;
	buffer_size -= buffer_size % device->constant_alignment;
	return buffer_size;
}

void gfx_clear_color(gfx_device_t *device, const gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, vec4f_t color)
{
	DEV_DEBUG;
	device->vtable->clear_color(device, render_target, attachment, color);
	DEV_DEBUG;
}

void gfx_clear_depth_stencil(gfx_device_t *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil)
{
	DEV_DEBUG;
	device->vtable->clear_depth_stencil(device, render_target, depth, stencil);
	DEV_DEBUG;
}

void gfx_draw_indexed_instanced(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	DEV_DEBUG;
	device->vtable->draw_indexed_instanced(device, primitive, count, offset, prim_count);
	DEV_DEBUG;
}

void gfx_draw_instanced(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	DEV_DEBUG;
	device->vtable->draw_instanced(device, primitive, count, offset, prim_count);
	DEV_DEBUG;
}

void gfx_draw_indexed(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset)
{
	DEV_DEBUG;
	device->vtable->draw_indexed(device, primitive, count, offset);
	DEV_DEBUG;
}

void gfx_draw(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset)
{
	DEV_DEBUG;
	device->vtable->draw(device, primitive, count, offset);
	DEV_DEBUG;
}

bool gfx_create_blend_state(gfx_device_t *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a)
{
	DEV_DEBUG;
	bool ret = device->vtable->create_blend_state(device, state, enabled, src_c, dst_c, src_a, dst_a, equation_c, equation_a);
	DEV_DEBUG;
	return ret;
}

void gfx_delete_blend_state(gfx_device_t *device, gfx_blend_state_t *state)
{
	DEV_DEBUG;
	device->vtable->delete_blend_state(device, state);
	DEV_DEBUG;
}

bool gfx_create_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass)
{
	DEV_DEBUG;
	bool ret = device->vtable->create_depth_stencil_state(device, state, depth_write, depth_test, depth_compare, stencil_enabled, stencil_write_mask, stencil_compare, stencil_reference, stencil_compare_mask, stencil_fail, stencil_zfail, stencil_pass);
	DEV_DEBUG;
	return ret;
}

void gfx_delete_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state)
{
	DEV_DEBUG;
	device->vtable->delete_depth_stencil_state(device, state);
	DEV_DEBUG;
}

bool gfx_create_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face, bool scissor)
{
	DEV_DEBUG;
	bool ret = device->vtable->create_rasterizer_state(device, state, fill_mode, cull_mode, front_face, scissor);
	DEV_DEBUG;
	return ret;
}

void gfx_delete_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state)
{
	DEV_DEBUG;
	device->vtable->delete_rasterizer_state(device, state);
	DEV_DEBUG;
}

bool gfx_create_buffer(gfx_device_t *device, gfx_buffer_t *buffer, enum gfx_buffer_type type, const void *data, uint32_t size, enum gfx_buffer_usage usage)
{
	DEV_DEBUG;
	bool ret = device->vtable->create_buffer(device, buffer, type, data, size, usage);
	DEV_DEBUG;
	return ret;
}

void gfx_set_buffer_data(gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset)
{
	DEV_DEBUG;
	buffer->device->vtable->set_buffer_data(buffer->device, buffer, data, size, offset);
	DEV_DEBUG;
}

void gfx_delete_buffer(gfx_device_t *device, gfx_buffer_t *buffer)
{
	DEV_DEBUG;
	device->vtable->delete_buffer(device, buffer);
	DEV_DEBUG;
}

bool gfx_create_attributes_state(gfx_device_t *device, gfx_attributes_state_t *state, const gfx_attribute_bind_t *binds, uint32_t count, const gfx_buffer_t *index_buffer, enum gfx_index_type index_type)
{
	DEV_DEBUG;
	bool ret = device->vtable->create_attributes_state(device, state, binds, count, index_buffer, index_type);
	DEV_DEBUG;
	return ret;
}

void gfx_bind_attributes_state(gfx_device_t *device, const gfx_attributes_state_t *state, const gfx_input_layout_t *input_layout)
{
	DEV_DEBUG;
	device->vtable->bind_attributes_state(device, state, input_layout);
	DEV_DEBUG;
}

void gfx_delete_attributes_state(gfx_device_t *device, gfx_attributes_state_t *state)
{
	DEV_DEBUG;
	device->vtable->delete_attributes_state(state->device, state);
	DEV_DEBUG;
}

bool gfx_create_input_layout(gfx_device_t *device, gfx_input_layout_t *input_layout, const gfx_input_layout_bind_t *binds, uint32_t count, const gfx_program_t *program)
{
	DEV_DEBUG;
	bool ret = device->vtable->create_input_layout(device, input_layout, binds, count, program);
	DEV_DEBUG;
	return ret;
}

void gfx_delete_input_layout(gfx_device_t *device, gfx_input_layout_t *input_layout)
{
	DEV_DEBUG;
	device->vtable->delete_input_layout(device, input_layout);
	DEV_DEBUG;
}

bool gfx_create_texture(gfx_device_t *device, gfx_texture_t *texture, enum gfx_texture_type type, enum gfx_format format, uint8_t lod, uint32_t width, uint32_t height, uint32_t depth)
{
	DEV_DEBUG;
	bool ret = device->vtable->create_texture(device, texture, type, format, lod, width, height, depth);
	DEV_DEBUG;
	return ret;
}

void gfx_set_texture_data(gfx_texture_t *texture, uint8_t lod, uint32_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t size, const void *data)
{
	DEV_DEBUG;
	texture->device->vtable->set_texture_data(texture->device, texture, lod, offset, width, height, depth, size, data);
	DEV_DEBUG;
}

void gfx_set_texture_addressing(gfx_texture_t *texture, enum gfx_texture_addressing addressing_s, enum gfx_texture_addressing addressing_t, enum gfx_texture_addressing addressing_r)
{
	DEV_DEBUG;
	texture->device->vtable->set_texture_addressing(texture->device, texture, addressing_s, addressing_t, addressing_r);
	DEV_DEBUG;
}

void gfx_set_texture_filtering(gfx_texture_t *texture, enum gfx_filtering min_filtering, enum gfx_filtering mag_filtering, enum gfx_filtering mip_filtering)
{
	DEV_DEBUG;
	texture->device->vtable->set_texture_filtering(texture->device, texture, min_filtering, mag_filtering, mip_filtering);
	DEV_DEBUG;
}

void gfx_set_texture_anisotropy(gfx_texture_t *texture, uint32_t anisotropy)
{
	DEV_DEBUG;
	texture->device->vtable->set_texture_anisotropy(texture->device, texture, anisotropy);
	DEV_DEBUG;
}

void gfx_set_texture_levels(gfx_texture_t *texture, uint32_t min_level, uint32_t max_level)
{
	DEV_DEBUG;
	texture->device->vtable->set_texture_levels(texture->device, texture, min_level, max_level);
	DEV_DEBUG;
}

void gfx_delete_texture(gfx_device_t *device, gfx_texture_t *texture)
{
	DEV_DEBUG;
	device->vtable->delete_texture(device, texture);
	DEV_DEBUG;
}

bool gfx_create_shader(gfx_device_t *device, gfx_shader_t *shader, enum gfx_shader_type type, const uint8_t *data, uint32_t len)
{
	DEV_DEBUG;
	bool ret = device->vtable->create_shader(device, shader, type, data, len);
	DEV_DEBUG;
	return ret;
}

void gfx_delete_shader(gfx_device_t *device, gfx_shader_t *shader)
{
	DEV_DEBUG;
	device->vtable->delete_shader(device, shader);
	DEV_DEBUG;
}

bool gfx_create_program(gfx_device_t *device, gfx_program_t *program, const gfx_shader_t *vertex_shader, const gfx_shader_t *fragment_shader, const gfx_shader_t *geometry_shader, const gfx_program_attribute_t *attributes, const gfx_program_constant_t *constants, const gfx_program_sampler_t *samplers)
{
	DEV_DEBUG;
	bool ret = device->vtable->create_program(device, program, vertex_shader, fragment_shader, geometry_shader, attributes, constants, samplers);
	DEV_DEBUG;
	return ret;
}

void gfx_delete_program(gfx_device_t *device, gfx_program_t *program)
{
	DEV_DEBUG;
	device->vtable->delete_program(device, program);
	DEV_DEBUG;
}

void gfx_bind_constant(gfx_device_t *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
	DEV_DEBUG;
	device->vtable->bind_constant(device, bind, buffer, size, offset);
	DEV_DEBUG;
}

void gfx_bind_samplers(gfx_device_t *device, uint32_t start, uint32_t count, const gfx_texture_t **textures)
{
	DEV_DEBUG;
	device->vtable->bind_samplers(device, start, count, textures);
	DEV_DEBUG;
}

bool gfx_create_render_target(gfx_device_t *device, gfx_render_target_t *render_target)\
{
	DEV_DEBUG;
	bool ret = device->vtable->create_render_target(device, render_target);
	DEV_DEBUG;
	return ret;
}

void gfx_delete_render_target(gfx_device_t *device, gfx_render_target_t *render_target)
{
	DEV_DEBUG;
	device->vtable->delete_render_target(device, render_target);
	DEV_DEBUG;
}

void gfx_bind_render_target(gfx_device_t *device, const gfx_render_target_t *render_target)
{
	DEV_DEBUG;
	device->vtable->bind_render_target(device, render_target);
	DEV_DEBUG;
}

void gfx_set_render_target_texture( gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_texture_t *texture)
{
	DEV_DEBUG;
	render_target->device->vtable->set_render_target_texture(render_target->device, render_target, attachment, texture);
	DEV_DEBUG;
}

void gfx_set_render_target_draw_buffers(gfx_render_target_t *render_target, uint32_t *draw_buffers, uint32_t draw_buffers_count)
{
	DEV_DEBUG;
	render_target->device->vtable->set_render_target_draw_buffers(render_target->device, render_target, draw_buffers, draw_buffers_count);
	DEV_DEBUG;
}

void gfx_resolve_render_target(const gfx_render_target_t *src, const gfx_render_target_t *dst, uint32_t buffers, uint32_t color_src, uint32_t color_dst)
{
	DEV_DEBUG;
	src->device->vtable->resolve_render_target(src->device, src, dst, buffers, color_src, color_dst);
	DEV_DEBUG;
}


bool gfx_create_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state, const gfx_program_t *program, const gfx_rasterizer_state_t *rasterizer, const gfx_depth_stencil_state_t *depth_stencil, const gfx_blend_state_t *blend, const gfx_input_layout_t *input_layout)
{
	DEV_DEBUG;
	bool ret = device->vtable->create_pipeline_state(device, state, program, rasterizer, depth_stencil, blend, input_layout);
	DEV_DEBUG;
	return ret;
}

void gfx_delete_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state)
{
	DEV_DEBUG;
	device->vtable->delete_pipeline_state(device, state);
	DEV_DEBUG;
}

void gfx_bind_pipeline_state(gfx_device_t *device, const gfx_pipeline_state_t *state)
{
	DEV_DEBUG;
	device->vtable->bind_pipeline_state(device, state);
	DEV_DEBUG;
}

void gfx_set_viewport(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	DEV_DEBUG;
	device->vtable->set_viewport(device, x, y, width, height);
	DEV_DEBUG;
}

void gfx_set_scissor(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	DEV_DEBUG;
	device->vtable->set_scissor(device, x, y, width, height);
	DEV_DEBUG;
}

void gfx_set_line_width(gfx_device_t *device, float line_width)
{
	DEV_DEBUG;
	device->vtable->set_line_width(device, line_width);
	DEV_DEBUG;
}

void gfx_set_point_size(gfx_device_t *device, float point_size)
{
	DEV_DEBUG;
	device->vtable->set_point_size(device, point_size);
	DEV_DEBUG;
}
