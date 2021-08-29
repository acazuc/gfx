#ifndef GFX_DEVICE_H
# define GFX_DEVICE_H

# ifdef __cplusplus
extern "C" {
# endif

# include "./objects.h"
# include <jks/math/vec4.h>
# include <jks/math/vec2.h>
# include <stddef.h>

typedef struct gfx_device_vtable_s gfx_device_vtable_t;
typedef struct gfx_device_s gfx_device_t;
typedef struct gfx_window_s gfx_window_t;

struct gfx_device_s
{
	gfx_device_vtable_t *vtable;
	gfx_window_t *window;
	uint32_t draw_calls_count;
	uint32_t triangles_count;
	uint32_t points_count;
	uint32_t lines_count;
	uint32_t constant_alignment;
	uint32_t max_samplers;
	uint32_t max_msaa;
};

void gfx_device_delete(gfx_device_t *device);
void gfx_device_tick(gfx_device_t *device);
uint32_t gfx_get_uniform_buffer_size(gfx_device_t *device, uint32_t buffer_size);

void gfx_clear_color(gfx_device_t *device, const gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, vec4f_t color);
void gfx_clear_depth_stencil(gfx_device_t *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil);

void gfx_draw_indexed_instanced(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t primcount);
void gfx_draw_instanced(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t primcount);
void gfx_draw_indexed(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset);
void gfx_draw(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset);

void gfx_create_blend_state(gfx_device_t *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a);
void gfx_delete_blend_state(gfx_device_t *device, gfx_blend_state_t *state);

void gfx_create_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass);
void gfx_delete_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state);

void gfx_create_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face, bool scissor);
void gfx_delete_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state);

void gfx_create_buffer(gfx_device_t *device, gfx_buffer_t *buffer, enum gfx_buffer_type type, const void *data, uint32_t size, enum gfx_buffer_usage usage);
void gfx_set_buffer_data(gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset);
void gfx_delete_buffer(gfx_device_t *device, gfx_buffer_t *buffer);

void gfx_create_attributes_state(gfx_device_t *device, gfx_attributes_state_t *state, const gfx_attribute_bind_t *binds, uint32_t count, const gfx_buffer_t *index_buffer, enum gfx_index_type index_type);
void gfx_bind_attributes_state(gfx_device_t *device, const gfx_attributes_state_t *state, const gfx_input_layout_t *input_layout);
void gfx_delete_attributes_state(gfx_device_t *device, gfx_attributes_state_t *state);

void gfx_create_input_layout(gfx_device_t *device, gfx_input_layout_t *input_layout, const gfx_input_layout_bind_t *binds, uint32_t count, const gfx_program_t *program);
void gfx_delete_input_layout(gfx_device_t *device, gfx_input_layout_t *input_layout);

void gfx_create_texture(gfx_device_t *device, gfx_texture_t *texture, enum gfx_texture_type type, enum gfx_format format, uint8_t lod, uint32_t width, uint32_t height, uint32_t depth);
void gfx_set_texture_data(gfx_texture_t *texture, uint8_t lod, uint32_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t size, const void *data);
void gfx_set_texture_addressing(gfx_texture_t *texture, enum gfx_texture_addressing addressing_s, enum gfx_texture_addressing addressing_t, enum gfx_texture_addressing addressing_r);
void gfx_set_texture_filtering(gfx_texture_t *texture, enum gfx_filtering min_filtering, enum gfx_filtering mag_filtering, enum gfx_filtering mip_filtering);
void gfx_set_texture_anisotropy(gfx_texture_t *texture, uint32_t anisotropy);
void gfx_set_texture_levels(gfx_texture_t *texture, uint32_t min_level, uint32_t max_level);
void gfx_delete_texture(gfx_device_t *device, gfx_texture_t *texture);

bool gfx_create_shader(gfx_device_t *device, gfx_shader_t *shader, enum gfx_shader_type type, const uint8_t *data, uint32_t len);
void gfx_delete_shader(gfx_device_t *device, gfx_shader_t *shader);
bool gfx_create_program(gfx_device_t *device, gfx_program_t *program, const gfx_shader_t *vertex_shader, const gfx_shader_t *fragment_shader, const gfx_shader_t *geometry_shader, const gfx_program_attribute_t *attributes, const gfx_program_constant_t *constants, const gfx_program_sampler_t *samplers);
void gfx_delete_program(gfx_device_t *device, gfx_program_t *program);
void gfx_bind_constant(gfx_device_t *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset);
void gfx_bind_samplers(gfx_device_t *device, uint32_t start, uint32_t count, const gfx_texture_t **textures);

void gfx_create_render_target(gfx_device_t *device, gfx_render_target_t *render_target);
void gfx_delete_render_target(gfx_device_t *device, gfx_render_target_t *render_target);
void gfx_bind_render_target(gfx_device_t *device, const gfx_render_target_t *render_target);
void gfx_set_render_target_texture(gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_texture_t *texture);
void gfx_set_render_target_draw_buffers(gfx_render_target_t *render_target, uint32_t *draw_buffers, uint32_t draw_buffers_count);
void gfx_resolve_render_target(const gfx_render_target_t *src, const gfx_render_target_t *dst, uint32_t buffers, uint32_t src_color, uint32_t dst_color);

void gfx_create_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state, const gfx_program_t *program, const gfx_rasterizer_state_t *rasterizer, const gfx_depth_stencil_state_t *depth_stencil, const gfx_blend_state_t *blend, const gfx_input_layout_t *input_layout);
void gfx_delete_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state);
void gfx_bind_pipeline_state(gfx_device_t *device, const gfx_pipeline_state_t *pipeline);

void gfx_set_viewport(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height);
void gfx_set_scissor(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height);
void gfx_set_line_width(gfx_device_t *device, float line_width);
void gfx_set_point_size(gfx_device_t *device, float point_size);

# ifdef __cplusplus
}
# endif

#endif
