#ifndef GFX_GL_DEVICE_H
#define GFX_GL_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../device.h"
#include <GL/glcorearb.h>
#include <jks/array.h>
#include <pthread.h>
#include <limits.h>

typedef void *(gfx_gl_load_addr_t)(const char *name);

typedef struct gfx_gl_device_s
{
	gfx_device_t device;
	gfx_gl_load_addr_t *load_addr;
	uint32_t textures[16];
	jks_array_t delete_render_buffers; /* uint32_t */
	jks_array_t delete_frame_buffers; /* uint32_t */
	jks_array_t delete_vertex_arrays; /* uint32_t */
	jks_array_t delete_programs; /* uint32_t */
	jks_array_t delete_buffers; /* uint32_t */
	jks_array_t delete_shaders; /* uint32_t */
	jks_array_t delete_textures; /* uint32_t */
	pthread_mutex_t delete_mutex;
	/* blend */
	enum gfx_blend_equation blend_equation_c;
	enum gfx_blend_equation blend_equation_a;
	enum gfx_blend_function blend_src_c;
	enum gfx_blend_function blend_src_a;
	enum gfx_blend_function blend_dst_c;
	enum gfx_blend_function blend_dst_a;
	/* stencil */
	enum gfx_stencil_operation stencil_fail;
	enum gfx_stencil_operation stencil_pass;
	enum gfx_stencil_operation stencil_zfail;
	enum gfx_compare_function stencil_compare;
	uint32_t stencil_compare_mask;
	uint32_t stencil_write_mask;
	uint32_t stencil_reference;
	/* depth */
	enum gfx_compare_function depth_func;
	bool depth_mask;
	/* rasterizer */
	enum gfx_fill_mode fill_mode;
	enum gfx_cull_mode cull_mode;
	enum gfx_front_face front_face;
	bool scissor;
	/* attributes */
	const gfx_attributes_state_t *attributes_state;
	uint32_t active_texture;
	uint32_t vertex_array;
	uint32_t program;
	float line_width;
	float point_size;
	uint64_t state_idx;
	uint64_t blend_state;
	uint64_t stencil_state;
	uint64_t depth_state;
	uint64_t rasterizer_state;
	uint64_t pipeline_state;
	PFNGLDELETEBUFFERSPROC DeleteBuffers;
	PFNGLDELETERENDERBUFFERSPROC DeleteRenderbuffers;
	PFNGLDELETEFRAMEBUFFERSPROC DeleteFramebuffers;
	PFNGLDELETEVERTEXARRAYSPROC DeleteVertexArrays;
	PFNGLDELETEPROGRAMPROC DeleteProgram;
	PFNGLDELETESHADERPROC DeleteShader;
	PFNGLDELETETEXTURESPROC DeleteTextures;
	PFNGLGETINTERNALFORMATIVPROC GetInternalformativ;
	PFNGLGETINTEGERVPROC GetIntegerv;
	PFNGLENABLEPROC Enable;
	PFNGLDISABLEPROC Disable;
	PFNGLGETERRORPROC GetError;
	uint8_t states[(USHRT_MAX + 7) / 8];
} gfx_gl_device_t;

extern gfx_device_vtable_t gfx_gl_device_vtable;

# ifndef NDEBUG

#  define GL_CALL_DEBUG(fn) \
do \
{ \
	GLenum err; \
	while ((err = GL_DEVICE->GetError())) \
		gfx_gl_errors(err, #fn,  __FILE__, __LINE__); \
} while (0)

# else

#  define GL_CALL_DEBUG(fn) \
do \
{ \
} while (0)

# endif

# define GL_CALL(device, fn, ...) do { device->fn(__VA_ARGS__); GL_CALL_DEBUG(fn); } while (0)
# define GL_CALL_RET(ret, device, fn, ...) do { ret = device->fn(__VA_ARGS__); GL_CALL_DEBUG(fn); } while (0)

# define GL_LOAD_PROC(device, fn) \
do \
{ \
	gl_load_proc((gfx_device_t*)device, "gl" #fn, (void**)&device->fn); \
	if (device->fn == NULL) \
	{ \
		if (gfx_error_callback) \
			gfx_error_callback("failed to load gl" #fn); \
	} \
} while (0)

static inline void gl_load_proc(gfx_device_t *device, const char *name, void **ptr)
{
	*ptr = ((gfx_gl_device_t*)device)->load_addr(name);
}

extern const GLuint gfx_gl_primitives[3];
extern const GLenum gfx_gl_index_types[2];
extern const GLenum gfx_gl_compare_functions[8];
extern const GLenum gfx_gl_blend_functions[12];
extern const GLenum gfx_gl_blend_equations[5];
extern const GLenum gfx_gl_stencil_operations[8];
extern const GLenum gfx_gl_fill_modes[3];
extern const GLenum gfx_gl_front_faces[2];
extern const GLenum gfx_gl_cull_modes[3];
extern const GLenum gfx_gl_shader_types[3];
extern const GLenum gfx_gl_buffer_types[3];
extern const GLenum gfx_gl_texture_types[5];
extern const GLenum gfx_gl_texture_addressings[5];
extern const GLenum gfx_gl_filterings[3];
extern const GLenum gfx_gl_min_filterings[9];
extern const GLenum gfx_gl_buffer_usages[4];
extern const GLenum gfx_gl_render_target_attachments[6];
extern const GLenum gfx_gl_internal_formats[14];
extern const GLenum gfx_gl_formats[10];
extern const GLenum gfx_gl_format_types[10];
extern const GLint gfx_gl_index_sizes[2];
extern const bool gfx_gl_attribute_normalized[34];
extern const GLenum gfx_gl_attribute_types[34];
extern const GLint gfx_gl_attribute_nb[34];
extern const bool gfx_gl_attribute_float[34];

void gfx_gl_errors(uint32_t err, const char *fn, const char *file, int line);
void gfx_gl_enable(gfx_device_t *device, uint32_t value);
void gfx_gl_disable(gfx_device_t *device, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif
