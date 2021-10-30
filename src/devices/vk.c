#include "vk.h"
#include "../device_vtable.h"
#include <vulkan/vulkan.h>
#include <stdlib.h>

#define VK_DEVICE ((gfx_vk_device_t*)device)

typedef struct gfx_vk_device_s
{
	gfx_device_t device;
	VkCommandBuffer command_buffer;
	VkDevice vk_device;
} gfx_vk_device_t;

static bool vk_ctr(gfx_device_t *device, gfx_window_t *window)
{
	if (!gfx_device_vtable.ctr(device, window))
		return false;
	return true;
}

static void vk_dtr(gfx_device_t *device)
{
	gfx_device_vtable.dtr(device);
}

static void vk_tick(gfx_device_t *device)
{
	gfx_device_vtable.tick(device);
}

static void vk_clear_color(gfx_device_t *device, const gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, vec4f_t color)
{
	VkClearColorValue vk_color;
	vk_color.float32[0] = color.x;
	vk_color.float32[1] = color.y;
	vk_color.float32[2] = color.z;
	vk_color.float32[3] = color.w;
	VkImageSubresourceRange range;
	range.aspectMask = ;
	range.baseMipLevel = 0;
	range.levelCount = ;
	range.baseArrayLayer = ;
	range.layerCount = ;
	vkCmdClearColorImage(VK_DEVICE->command_buffer, image, image_layout, &vk_color, 1, &range);
}

static void vk_clear_depth_stencil(gfx_device_t *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil)
{
	VkClearDepthStencilValue vk_depth_stencil;
	vk_depth_stencil.depth = depth;
	vk_depth_stencil.stencil = stencil;
	VkImageSubresourceRange range;
	range.aspectMask = ;
	range.baseMipLevel = 0;
	range.levelCount = ;
	range.baseArrayLayer = ;
	range.layerCount = ;
	vkCmdClearDepthStencilImage(VK_DEVICE->command_buffer, image, image_layout, &vk_depth_stencil, 1, &range
}

static void vk_draw_indexed_instanced(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	vkCmdDrawIndexed(VK_DEVICE->command_buffer, count, prim_count, offset, 0, 0);
}

static void vk_draw_instanced(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	vkCmdDraw(VK_DEVICE->command_buffer, count, prim_count, offset, 0);
}

static void vk_draw_indexed(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset)
{
	vkCmdDrawIndexed(VK_DEVICE->command_buffer, count, 1, offset, 0, 0);
}

static void vk_draw(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset)
{
	vkCmdDraw(VK_DEVICE->command_buffer, count, 1, offset, 0);
}

static void vk_create_blend_state(gfx_device_t *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a)
{
	//VkPipelineColorBlendStateCreateInfo
}

static void vk_delete_blend_state(gfx_device_t *device, gfx_blend_state_t *state)
{
}

static void vk_create_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass)
{
	//VkPipelineDepthStencilStateCreateInfo
}

static void vk_delete_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state)
{
}

static void vk_create_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face)
{
	//VkPipelineRasterizationStateCreateInfo
}

static void vk_delete_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state)
{
}

static void vk_create_buffer(gfx_device_t *device, gfx_buffer_t *buffer, enum gfx_buffer_type type, const void *data, uint32_t size, enum gfx_buffer_usage usage)
{
}

static void vk_set_buffer_data(gfx_device_t *device, gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset)
{
}

static void vk_delete_buffer(gfx_device_t *device, gfx_buffer_t *buffer)
{
}

static void vk_create_attributes_state(gfx_device_t *device, gfx_attributes_state_t *state)
{
	//VkPipelineVertexInputStateCreateInfo
}

static void vk_bind_attributes_state(gfx_device_t *device, const gfx_attributes_state_t *state, const gfx_program_t *program)
{
}

static void vk_delete_attributes_state(gfx_device_t *device, gfx_attributes_state_t *state)
{
}

static void vk_create_texture(gfx_device_t *device, gfx_texture_t *texture, enum gfx_texture_type type, enum gfx_format format, uint8_t lod, uint32_t width, uint32_t height, uint32_t depth)
{
}

static void vk_set_texture_data(gfx_device_t *device, gfx_texture_t *texture, uint8_t lod, uint32_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t size, const void *data)
{
}

static void vk_set_texture_addressing(gfx_device_t *device, gfx_texture_t *texture, enum gfx_texture_addressing addressing_s, enum gfx_texture_addressing addressing_t, enum gfx_texture_addressing addressing_r)
{
}

static void vk_set_texture_filtering(gfx_device_t *device, gfx_texture_t *texture, enum gfx_filtering min_filtering, enum gfx_filtering mag_filtering, enum gfx_filtering mip_filtering)
{
}

static void vk_set_texture_anisotropy(gfx_device_t *device, gfx_texture_t *texture, uint32_t anisotropy)
{
}

static void vk_set_texture_levels(gfx_device_t *device, gfx_texture_t *texture, uint32_t min_level, uint32_t max_level)
{
}

static void vk_delete_texture(gfx_device_t *device, gfx_texture_t *texture)
{
}

static bool vk_create_shader(gfx_device_t *device, gfx_shader_t *shader, enum gfx_shader_type type, const uint8_t *data, uint32_t len)
{
	VkShaderModuleCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = len;
	create_info.pCode = (const uint32_t*)data;
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(VK_DEVICE->vk_device, &create_info, NULL, (VkShaderModule*)&shader->handle.ptr) != VK_SUCCESS)
		return false;
	return true;
}

static void vk_delete_shader(gfx_device_t *device, gfx_shader_t *shader)
{
	if (!shader || !shader->handle.ptr)
		return;
	vkDestroyShaderModule(VK_DEVICE->vk_device, (VkShaderModule)shader->handle.ptr, NULL);
}

static bool vk_create_program(gfx_device_t *device, gfx_program_t *program, const gfx_shader_t *vertex_shader, const gfx_shader_t *fragment_shader, const gfx_shader_t *geometry_shader, const gfx_program_attribute_t *attributes, const gfx_program_constant_t *constants, const gfx_program_sampler_t *samplers)
{
	VkPipelineShaderStageCreateInfo vert_info;
	vert_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_info.module = (VkShaderModule)vertex_shader->handle.ptr;
	vert_info.pName = "main";
	VkPipelineShaderStageCreateInfo frag_info;
	frag_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_info.module = (VkShaderModule)fragment_shader->handle.ptr;
	frag_info.pName = "main";
	VkPipelineShaderStageCreateInfo shaderStages[] = {vert_info, frag_info};
	return true;
}

static void vk_bind_program(gfx_device_t *device, const gfx_program_t *program)
{
}

static void vk_delete_program(gfx_device_t *device, gfx_program_t *program)
{
}

static void vk_bind_constant(gfx_device_t *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
	//vkCmdBindDescriptorSets
}

static void vk_bind_samplers(gfx_device_t *device, uint32_t start, uint32_t count, const gfx_texture_t **texture)
{
	//vkCmdBindDescriptorSets
}

static void vk_create_render_target(gfx_device_t *device, gfx_render_target_t *render_target)
{
}

static void vk_delete_render_target(gfx_device_t *device, gfx_render_target_t *render_target)
{
}

static void vk_bind_render_target(gfx_device_t *device, const gfx_render_target_t *render_target)
{
}

static void vk_blit_render_target(gfx_device_t *device, const gfx_render_target_t *src, vec2f_t src_pos, vec2f_t src_size, const gfx_render_target_t *dst, vec2f_t dst_pos, vec2f_t dst_size, uint32_t buffers, uint32_t color_src, uint32_t color_dst, bool linear)
{
}

static void vk_set_render_target_texture(gfx_device_t *device, gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_texture_t *texture)
{
}

static void vk_set_render_target_render_buffer(gfx_device_t *device, gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_render_buffer_t *render_buffer)
{
}

static void vk_set_render_target_draw_buffers(gfx_device_t *device, gfx_render_target_t *render_target, uint32_t *draw_buffers, uint32_t draw_buffers_count)
{
}

static void vk_create_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state)
{
	//VkGraphicsPipelineCreateInfo
	//VkPipeline graphicsPipeline;
	//vkCreateGraphicsPipelines
}

static void vk_bind_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state)
{
}

static void vk_delete_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state)
{
}

static void vk_set_viewport(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	VkViewport viewports[8];
	for (size_t i = 0; i < 8; ++i)
	{
		viewports[i].x = x;
		viewports[i].y = y;
		viewports[i].width = width;
		viewports[i].height = height;
		viewports[i].minDepth = 0;
		viewports[i].maxDepth = 1;
	}
	vkCmdSetViewport(VK_DEVICE->command_buffer, 0, 8, viewports);
}

static void vk_set_scissor(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	VkRect2D rect;
	recv.offset.x = x;
	rect.offset.y = y;
	rect.extent.width = width;
	rect.extent.height = height;
	vkCmdSetScissor(VK_DEVICE->command_buffer, 0, 1, &rect);
}

static void vk_set_line_width(gfx_device_t *device, float line_width)
{
	vkCmdSetLineWidth(VK_DEVICE->command_buffer, line_width);
}

static void vk_set_point_size(gfx_device_t *device, float point_size)
{
	(void)device;
	(void)point_size;
}

static gfx_device_vtable_t vk_vtable =
{
	GFX_DEVICE_VTABLE_DEF(vk)
};

gfx_device_t *gfx_vk_device_new(gfx_window_t *window)
{
	gfx_vk_device_t *device = GFX_MALLOC(sizeof(*device));
	if (!device)
		return NULL;
	gfx_device_t *dev = &device->device;
	dev->vtable = &vk_vtable;
	if (!dev->vtable->ctr(dev, window))
	{
		dev->vtable->dtr(dev);
		GFX_FREE(device);
		return NULL;
	}
	return dev;
}
