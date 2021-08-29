#include "gl.h"
#include "../device_vtable.h"
#include "../window.h"
#include <inttypes.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define GL_DEVICE ((gfx_gl_device_t*)device)
#define GL3_DEVICE ((gfx_gl3_device_t*)device)

#ifndef GL_R
# define GL_R 0x2002
#endif

const GLuint gfx_gl_primitives[3] = {GL_TRIANGLES, GL_POINTS, GL_LINES};

const GLenum gfx_gl_index_types[2] = {GL_UNSIGNED_SHORT, GL_UNSIGNED_INT};

const GLenum gfx_gl_compare_functions[8] = {GL_NEVER, GL_LESS, GL_LEQUAL, GL_EQUAL, GL_GEQUAL, GL_GREATER, GL_NOTEQUAL, GL_ALWAYS};

const GLenum gfx_gl_blend_functions[12] = {GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR};

const GLenum gfx_gl_blend_equations[5] = {GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX};

const GLenum gfx_gl_stencil_operations[8] = {GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR, GL_DECR_WRAP, GL_INVERT};

const GLenum gfx_gl_fill_modes[3] = {GL_POINT, GL_LINE, GL_FILL};

const GLenum gfx_gl_front_faces[2] = {GL_CW, GL_CCW};

const GLenum gfx_gl_cull_modes[3] = {GL_BACK, GL_FRONT, GL_BACK};

const GLenum gfx_gl_shader_types[3] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER};

const GLenum gfx_gl_buffer_types[3] = {GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_UNIFORM_BUFFER};

const GLenum gfx_gl_texture_types[5] = {GL_TEXTURE_2D, GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_2D_ARRAY, GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_3D};

const GLenum gfx_gl_texture_addressings[5] = {GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_BORDER, GL_MIRROR_CLAMP_TO_EDGE};

const GLenum gfx_gl_filterings[3] = {GL_NEAREST, GL_NEAREST, GL_LINEAR};

const GLenum gfx_gl_min_filterings[9] = {GL_NEAREST, GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR};

const GLenum gfx_gl_buffer_usages[4] = {GL_STATIC_DRAW, GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW};

const GLenum gfx_gl_render_target_attachments[6] = {GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};

const GLenum gfx_gl_internal_formats[14] = {GL_DEPTH24_STENCIL8, GL_RGBA32F, GL_RGBA16F, GL_RGB32F, GL_RGBA8, GL_RGB5_A1, GL_RGBA4, GL_RGB, GL_RG8, GL_R8, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT};

const GLenum gfx_gl_formats[10] = {GL_DEPTH_STENCIL, GL_BGRA, GL_BGRA, GL_RGB, GL_BGRA, GL_BGRA, GL_BGRA, GL_RGB, GL_RG, GL_R};

const GLenum gfx_gl_format_types[10] = {GL_UNSIGNED_INT_24_8, GL_FLOAT, GL_UNSIGNED_BYTE, GL_FLOAT, GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_UNSIGNED_SHORT_5_6_5_REV, GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE};

const GLint gfx_gl_index_sizes[2] = {2, 4};

const bool gfx_gl_attribute_normalized[34] =
{
	true, false, false,
	true, false, false,
	true, false, false,
	true, false, false,
	true, true, true, false, false,
	true, true, true, false, false,
	true, true, false, false,
	true, true, false, false,
	true, true, false, false,
};

const GLenum gfx_gl_attribute_types[34] =
{
	GL_FLOAT, GL_UNSIGNED_INT, GL_INT,
	GL_FLOAT, GL_UNSIGNED_INT, GL_INT,
	GL_FLOAT, GL_UNSIGNED_INT, GL_INT,
	GL_FLOAT, GL_UNSIGNED_INT, GL_INT,
	GL_HALF_FLOAT, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_SHORT, GL_SHORT,
	GL_HALF_FLOAT, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_SHORT, GL_SHORT,
	GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_BYTE, GL_BYTE,
	GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_BYTE, GL_BYTE,
	GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_BYTE, GL_BYTE,
};

const GLint gfx_gl_attribute_nb[34] =
{
	4, 4, 4,
	3, 3, 3,
	2, 2, 2,
	1, 1, 1,
	4, 4, 4, 4, 4,
	2, 2, 2, 2, 2,
	4, 4, 4, 4,
	2, 2, 2, 2,
	1, 1, 1, 1,
};

const bool gfx_gl_attribute_float[34] =
{
	true, false, false,
	true, false, false,
	true, false, false,
	true, false, false,
	true, true, true, false, false,
	true, true, true, false, false,
	true, true, false, false,
	true, true, false, false,
	true, true, false, false,
};

void gfx_gl_errors(uint32_t err, const char *fn, const char *file, int line)
{
#define TEST_ERR(code) \
	case code: \
		out = #code; \
		break;

	const char *out;
	switch (err)
	{
		TEST_ERR(GL_INVALID_ENUM)
		TEST_ERR(GL_INVALID_VALUE)
		TEST_ERR(GL_INVALID_OPERATION)
		TEST_ERR(GL_STACK_OVERFLOW)
		TEST_ERR(GL_STACK_UNDERFLOW)
		TEST_ERR(GL_OUT_OF_MEMORY)
		TEST_ERR(GL_INVALID_FRAMEBUFFER_OPERATION)
		default:
			out = "unknown error";
			break;
	}
	if (gfx_error_callback)
		gfx_error_callback("%s@%s:%d %s", fn, file, line, out);

#undef TEST_ERR
}

void gfx_gl_enable(gfx_device_t *device, uint32_t value)
{
	if (value < USHRT_MAX)
	{
		if (GL_DEVICE->states[value / 8] & (1 << (value % 8)))
			return;
		GL_DEVICE->states[value / 8] |= 1 << (value % 8);
	}
	GL_DEVICE->Enable(value);
#ifndef NDEBUG
	GLenum err;
	while ((err = GL_DEVICE->GetError()))
	{
		char call_txt[128];
		snprintf(call_txt, sizeof(call_txt), "glEnable(%" PRIu32 ")", value);
		gfx_gl_errors(err, call_txt, __FILE__, __LINE__);
	}
#endif
}

void gfx_gl_disable(gfx_device_t *device, uint32_t value)
{
	if (value < USHRT_MAX)
	{
		if (!(GL_DEVICE->states[value / 8] & (1 << (value % 8))))
			return;
		GL_DEVICE->states[value / 8] &= ~(1 << (value % 8));
	}
	GL_DEVICE->Disable(value);
#ifndef NDEBUG
	GLenum err;
	while ((err = GL_DEVICE->GetError()))
	{
		char call_txt[128];
		snprintf(call_txt, sizeof(call_txt), "glDisable(%" PRIu32 ")", value);
		gfx_gl_errors(err, call_txt, __FILE__, __LINE__);
	}
#endif
}

//#define DEBUG_MESSAGE

#ifdef DEBUG_MESSAGE
static void message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const *message, void const *user_param)
{
	const char *severity_str;
	const char *type_str;
	const char *src_str;

	(void)length;
	(void)user_param;
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:
			src_str = "API";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			src_str = "WINDOW SYSTEM";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			src_str = "SHADER COMPILER";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			src_str = "THIRD PARTY";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			src_str = "APPLICATION";
			break;
		case GL_DEBUG_SOURCE_OTHER:
			src_str = "OTHER";
			break;
		default:
			src_str = "UNKNOWN";
			break;
	}

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:
			type_str = "ERROR";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			type_str = "DEPRECATED_BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			type_str =  "UNDEFINED_BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			type_str = "PORTABILITY";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			type_str = "PERFORMANCE";
			break;
		case GL_DEBUG_TYPE_MARKER:
			type_str = "MARKER";
			break;
		case GL_DEBUG_TYPE_OTHER:
			type_str = "OTHER";
			break;
		default:
			type_str = "UNKNOWN";
			break;
	}

	switch (severity)
	{
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			severity_str = "NOTIFICATION";
			break;
		case GL_DEBUG_SEVERITY_LOW:
			severity_str = "LOW";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			severity_str = "MEDIUM";
			break;
		case GL_DEBUG_SEVERITY_HIGH:
			severity_str = "HIGH";
			break;
		default:
			severity_str = "UNKNOWN";
			break;
	}

	printf("%s, %s, %s, %u: %s\n", src_str, type_str, severity_str, id, message);
}
#endif

static bool gl_ctr(gfx_device_t *device, gfx_window_t *window)
{
#ifdef DEBUG_MESSAGE
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(message_callback, NULL);
#endif
	if (!gfx_device_vtable.ctr(device, window))
		return false;
	GL_LOAD_PROC(GL_DEVICE, DeleteBuffers);
	GL_LOAD_PROC(GL_DEVICE, DeleteRenderbuffers);
	GL_LOAD_PROC(GL_DEVICE, DeleteFramebuffers);
	GL_LOAD_PROC(GL_DEVICE, DeleteVertexArrays);
	GL_LOAD_PROC(GL_DEVICE, DeleteProgram);
	GL_LOAD_PROC(GL_DEVICE, DeleteShader);
	GL_LOAD_PROC(GL_DEVICE, DeleteTextures);
	GL_LOAD_PROC(GL_DEVICE, GetInternalformativ);
	GL_LOAD_PROC(GL_DEVICE, GetIntegerv);
	GL_LOAD_PROC(GL_DEVICE, Enable);
	GL_LOAD_PROC(GL_DEVICE, Disable);
	GL_LOAD_PROC(GL_DEVICE, GetError);
	memset(GL_DEVICE->states, 0, sizeof(GL_DEVICE->states));
	jks_array_init(&GL_DEVICE->delete_render_buffers, sizeof(uint32_t), NULL);
	jks_array_init(&GL_DEVICE->delete_frame_buffers, sizeof(uint32_t), NULL);
	jks_array_init(&GL_DEVICE->delete_vertex_arrays, sizeof(uint32_t), NULL);
	jks_array_init(&GL_DEVICE->delete_programs, sizeof(uint32_t), NULL);
	jks_array_init(&GL_DEVICE->delete_buffers, sizeof(uint32_t), NULL);
	jks_array_init(&GL_DEVICE->delete_shaders, sizeof(uint32_t), NULL);
	jks_array_init(&GL_DEVICE->delete_textures, sizeof(uint32_t), NULL);
	memset(GL_DEVICE->textures, 0, sizeof(GL_DEVICE->textures));
	GL_DEVICE->blend_equation_c = GFX_EQUATION_ADD;
	GL_DEVICE->blend_equation_a = GFX_EQUATION_ADD;
	GL_DEVICE->blend_src_c = GFX_BLEND_ONE;
	GL_DEVICE->blend_src_a = GFX_BLEND_ONE;
	GL_DEVICE->blend_dst_c = GFX_BLEND_ZERO;
	GL_DEVICE->blend_dst_a = GFX_BLEND_ZERO;
	GL_DEVICE->stencil_fail = GFX_STENCIL_KEEP;
	GL_DEVICE->stencil_pass = GFX_STENCIL_KEEP;
	GL_DEVICE->stencil_zfail = GFX_STENCIL_KEEP;
	GL_DEVICE->stencil_compare = GFX_CMP_ALWAYS;
	GL_DEVICE->stencil_compare_mask = 0xffffffffu;
	GL_DEVICE->stencil_write_mask = 0xffffffffu;
	GL_DEVICE->stencil_reference = 0;
	GL_DEVICE->depth_func = GFX_CMP_LOWER;
	GL_DEVICE->depth_mask = true;
	GL_DEVICE->fill_mode = GFX_FILL_SOLID;
	GL_DEVICE->cull_mode = GFX_CULL_NONE;
	GL_DEVICE->front_face = GFX_FRONT_CCW;
	GL_DEVICE->active_texture = 0;
	GL_DEVICE->vertex_array = 0;
	GL_DEVICE->program = 0;
	GL_DEVICE->line_width = 1;
	GL_DEVICE->point_size = 1;
	GL_DEVICE->scissor = false;
	GL_DEVICE->state_idx = 0;
	GL_DEVICE->blend_state = 0;
	GL_DEVICE->stencil_state = 0;
	GL_DEVICE->depth_state = 0;
	GL_DEVICE->rasterizer_state = 0;
	GL_DEVICE->attributes_state = NULL;
	GL_DEVICE->pipeline_state = 0;
	pthread_mutex_init(&GL_DEVICE->delete_mutex, NULL);
	GL_CALL(GL_DEVICE, GetIntegerv, GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, (int32_t*)&device->constant_alignment);
	GL_CALL(GL_DEVICE, GetIntegerv, GL_MAX_TEXTURE_IMAGE_UNITS, (int32_t*)&device->max_samplers);
	GL_CALL(GL_DEVICE, GetIntegerv, GL_MAX_COLOR_TEXTURE_SAMPLES, (int32_t*)&device->max_msaa);
#ifndef NDEBUG
	for (uint32_t i = 1; i <= device->max_msaa; i *= 2)
		printf("MSAA %" PRIu32 "\n", i);
#endif
#ifndef NDEBUG
	for (uint32_t i = 0; i < sizeof(gfx_gl_internal_formats) / sizeof(*gfx_gl_internal_formats); ++i)
	{
		GLint ret = GL_FALSE;
		GL_CALL(GL_DEVICE, GetInternalformativ, GL_TEXTURE_2D, GL_RGBA4, GL_INTERNALFORMAT_SUPPORTED, sizeof(ret), &ret);
		printf("native format %" PRIu32 ": %s\n", i, ret ? "true" : "false");
	}
#endif
	return true;
}

static void gl_dtr(gfx_device_t *device)
{
	jks_array_destroy(&GL_DEVICE->delete_render_buffers);
	jks_array_destroy(&GL_DEVICE->delete_frame_buffers);
	jks_array_destroy(&GL_DEVICE->delete_vertex_arrays);
	jks_array_destroy(&GL_DEVICE->delete_programs);
	jks_array_destroy(&GL_DEVICE->delete_buffers);
	jks_array_destroy(&GL_DEVICE->delete_shaders);
	jks_array_destroy(&GL_DEVICE->delete_textures);
	gfx_device_vtable.dtr(device);
}

static void gl_tick(gfx_device_t *device)
{
	gfx_device_vtable.tick(device);
	pthread_mutex_lock(&GL_DEVICE->delete_mutex);
	if (GL_DEVICE->delete_buffers.size)
	{
		GL_CALL(GL_DEVICE, DeleteBuffers, GL_DEVICE->delete_buffers.size, (const GLuint*)GL_DEVICE->delete_buffers.data);
		jks_array_resize(&GL_DEVICE->delete_buffers, 0);
	}
	if (GL_DEVICE->delete_render_buffers.size)
	{
		GL_CALL(GL_DEVICE, DeleteRenderbuffers, GL_DEVICE->delete_render_buffers.size, (const GLuint*)GL_DEVICE->delete_render_buffers.data);
		jks_array_resize(&GL_DEVICE->delete_render_buffers, 0);
	}
	if (GL_DEVICE->delete_frame_buffers.size)
	{
		GL_CALL(GL_DEVICE, DeleteFramebuffers, GL_DEVICE->delete_frame_buffers.size, (const GLuint*)GL_DEVICE->delete_frame_buffers.data);
		jks_array_resize(&GL_DEVICE->delete_frame_buffers, 0);
	}
	if (GL_DEVICE->delete_vertex_arrays.size)
	{
		GL_CALL(GL_DEVICE, DeleteVertexArrays, GL_DEVICE->delete_vertex_arrays.size, (const GLuint*)GL_DEVICE->delete_vertex_arrays.data);
		jks_array_resize(&GL_DEVICE->delete_vertex_arrays, 0);
	}
	for (uint32_t i = 0; i < GL_DEVICE->delete_programs.size; ++i)
		GL_CALL(GL_DEVICE, DeleteProgram, *JKS_ARRAY_GET(&GL_DEVICE->delete_programs, i, uint32_t));
	jks_array_resize(&GL_DEVICE->delete_programs, 0);
	for (uint32_t i = 0; i < GL_DEVICE->delete_shaders.size; ++i)
		GL_CALL(GL_DEVICE, DeleteShader, *JKS_ARRAY_GET(&GL_DEVICE->delete_shaders, i, uint32_t));
	jks_array_resize(&GL_DEVICE->delete_shaders, 0);
	if (GL_DEVICE->delete_textures.size)
	{
		GL_CALL(GL_DEVICE, DeleteTextures, GL_DEVICE->delete_textures.size, (const GLuint*)GL_DEVICE->delete_textures.data);
		jks_array_resize(&GL_DEVICE->delete_textures, 0);
	}
	pthread_mutex_unlock(&GL_DEVICE->delete_mutex);
}

gfx_device_vtable_t gfx_gl_device_vtable =
{
	.ctr = gl_ctr,
	.dtr = gl_dtr,
	.tick = gl_tick,
};
