#ifndef GFX_DEVICE_VTABLE_H
#define GFX_DEVICE_VTABLE_H

typedef struct gfx_device_vtable_s
{
	bool (*ctr)(gfx_device_t *device, gfx_window_t *window);
	void (*dtr)(gfx_device_t *device);
	void (*tick)(gfx_device_t *device);

	void (*clear_color)(gfx_device_t *device, const gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, vec4f_t color);
	void (*clear_depth_stencil)(gfx_device_t *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil);

	void (*draw_indexed_instanced)(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count);
	void (*draw_instanced)(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count);
	void (*draw_indexed)(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset);
	void (*draw)(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset);

	bool (*create_blend_state)(gfx_device_t *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a);
	void (*delete_blend_state)(gfx_device_t *device, gfx_blend_state_t *state);

	bool (*create_depth_stencil_state)(gfx_device_t *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass);
	void (*delete_depth_stencil_state)(gfx_device_t *device, gfx_depth_stencil_state_t *state);

	bool (*create_rasterizer_state)(gfx_device_t *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face, bool scissor);
	void (*delete_rasterizer_state)(gfx_device_t *device, gfx_rasterizer_state_t *state);

	bool (*create_buffer)(gfx_device_t *device, gfx_buffer_t *buffer, enum gfx_buffer_type type, const void *data, uint32_t size, enum gfx_buffer_usage usage);
	void (*set_buffer_data)(gfx_device_t *device, gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset);
	void (*delete_buffer)(gfx_device_t *device, gfx_buffer_t *buffer);

	bool (*create_attributes_state)(gfx_device_t *device, gfx_attributes_state_t *state, const gfx_attribute_bind_t *binds, uint32_t count, const gfx_buffer_t *index_buffer, enum gfx_index_type index_type);
	void (*bind_attributes_state)(gfx_device_t *device, const gfx_attributes_state_t *state, const gfx_input_layout_t *input_layout);
	void (*delete_attributes_state)(gfx_device_t *device, gfx_attributes_state_t *state);

	bool (*create_input_layout)(gfx_device_t *device, gfx_input_layout_t *input_layout, const gfx_input_layout_bind_t *binds, uint32_t count, const gfx_program_t *program);
	void (*delete_input_layout)(gfx_device_t *device, gfx_input_layout_t *input_layout);

	bool (*create_texture)(gfx_device_t *device, gfx_texture_t *texture, enum gfx_texture_type type, enum gfx_format format, uint8_t lod, uint32_t width, uint32_t height, uint32_t depth);
	void (*set_texture_data)(gfx_device_t *device, gfx_texture_t *texture, uint8_t lod, uint32_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t size, const void *data);
	void (*set_texture_addressing)(gfx_device_t *device, gfx_texture_t *texture, enum gfx_texture_addressing addressing_s, enum gfx_texture_addressing addressing_t, enum gfx_texture_addressing addressing_r);
	void (*set_texture_filtering)(gfx_device_t *device, gfx_texture_t *texture, enum gfx_filtering min_filtering, enum gfx_filtering mag_filtering, enum gfx_filtering mip_filtering);
	void (*set_texture_anisotropy)(gfx_device_t *device, gfx_texture_t *texture, uint32_t anisotropy);
	void (*set_texture_levels)(gfx_device_t *device, gfx_texture_t *texture, uint32_t min_level, uint32_t max_level);
	void (*delete_texture)(gfx_device_t *device, gfx_texture_t *texture);

	bool (*create_shader)(gfx_device_t *device, gfx_shader_t *shader, enum gfx_shader_type type, const uint8_t *data, uint32_t len);
	void (*delete_shader)(gfx_device_t *device, gfx_shader_t *shader);
	bool (*create_program)(gfx_device_t *device, gfx_program_t *program, const gfx_shader_t *vertex_shader, const gfx_shader_t *fragment_shader, const gfx_shader_t *geometry_shader, const gfx_program_attribute_t *attributes, const gfx_program_constant_t *constants, const gfx_program_sampler_t *samplers);
	void (*delete_program)(gfx_device_t *device, gfx_program_t *program);
	void (*bind_constant)(gfx_device_t *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset);
	void (*bind_samplers)(gfx_device_t *device, uint32_t start, uint32_t count, const gfx_texture_t **textures);

	bool (*create_render_target)(gfx_device_t *device, gfx_render_target_t *render_target);
	void (*delete_render_target)(gfx_device_t *device, gfx_render_target_t *render_target);
	void (*bind_render_target)(gfx_device_t *device, const gfx_render_target_t *render_target);
	void (*set_render_target_texture)(gfx_device_t *device, gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_texture_t *texture);
	void (*set_render_target_draw_buffers)(gfx_device_t *device, gfx_render_target_t *render_target, uint32_t *draw_buffers, uint32_t draw_buffers_count);
	void (*resolve_render_target)(gfx_device_t *device, const gfx_render_target_t *src, const gfx_render_target_t *dst, uint32_t buffers, uint32_t src_color, uint32_t dst_color);

	bool (*create_pipeline_state)(gfx_device_t *device, gfx_pipeline_state_t *state, const gfx_program_t *program, const gfx_rasterizer_state_t *rasterizer, const gfx_depth_stencil_state_t *depth_stencil, const gfx_blend_state_t *blend, const gfx_input_layout_t *input_layout);
	void (*delete_pipeline_state)(gfx_device_t *device, gfx_pipeline_state_t *state);
	void (*bind_pipeline_state)(gfx_device_t *device, const gfx_pipeline_state_t *state);

	void (*set_viewport)(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height);
	void (*set_scissor)(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height);
	void (*set_line_width)(gfx_device_t *device, float line_width);
	void (*set_point_size)(gfx_device_t *device, float point_size);
} gfx_device_vtable_t;

extern const gfx_device_vtable_t gfx_device_vtable;

#define GFX_DEVICE_VTABLE_DEF(prefix) \
	.ctr  = prefix##_ctr, \
	.dtr  = prefix##_dtr, \
	.tick = prefix##_tick, \
	.clear_color         = prefix##_clear_color, \
	.clear_depth_stencil = prefix##_clear_depth_stencil, \
	.draw_indexed_instanced = prefix##_draw_indexed_instanced, \
	.draw_instanced         = prefix##_draw_instanced, \
	.draw_indexed           = prefix##_draw_indexed, \
	.draw                   = prefix##_draw, \
	.create_blend_state = prefix##_create_blend_state, \
	.delete_blend_state = prefix##_delete_blend_state, \
	.create_depth_stencil_state = prefix##_create_depth_stencil_state, \
	.delete_depth_stencil_state = prefix##_delete_depth_stencil_state, \
	.create_rasterizer_state = prefix##_create_rasterizer_state, \
	.delete_rasterizer_state = prefix##_delete_rasterizer_state, \
	.create_buffer   = prefix##_create_buffer, \
	.set_buffer_data = prefix##_set_buffer_data, \
	.delete_buffer   = prefix##_delete_buffer, \
	.create_attributes_state = prefix##_create_attributes_state, \
	.bind_attributes_state   = prefix##_bind_attributes_state, \
	.delete_attributes_state = prefix##_delete_attributes_state, \
	.create_input_layout = prefix##_create_input_layout, \
	.delete_input_layout = prefix##_delete_input_layout, \
	.create_texture         = prefix##_create_texture, \
	.set_texture_data       = prefix##_set_texture_data, \
	.set_texture_addressing = prefix##_set_texture_addressing, \
	.set_texture_filtering  = prefix##_set_texture_filtering, \
	.set_texture_anisotropy = prefix##_set_texture_anisotropy, \
	.set_texture_levels     = prefix##_set_texture_levels, \
	.delete_texture         = prefix##_delete_texture, \
	.create_shader  = prefix##_create_shader, \
	.delete_shader  = prefix##_delete_shader, \
	.create_program = prefix##_create_program, \
	.delete_program = prefix##_delete_program, \
	.bind_constant  = prefix##_bind_constant, \
	.bind_samplers  = prefix##_bind_samplers, \
	.create_render_target            = prefix##_create_render_target, \
	.delete_render_target            = prefix##_delete_render_target, \
	.bind_render_target              = prefix##_bind_render_target, \
	.set_render_target_texture       = prefix##_set_render_target_texture, \
	.set_render_target_draw_buffers  = prefix##_set_render_target_draw_buffers, \
	.resolve_render_target           = prefix##_resolve_render_target, \
	.create_pipeline_state = prefix##_create_pipeline_state, \
	.delete_pipeline_state = prefix##_delete_pipeline_state, \
	.bind_pipeline_state   = prefix##_bind_pipeline_state, \
	.set_viewport   = prefix##_set_viewport, \
	.set_scissor    = prefix##_set_scissor, \
	.set_line_width = prefix##_set_line_width, \
	.set_point_size = prefix##_set_point_size,

#endif
