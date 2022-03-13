#ifndef GFX_WINDOW_H
#define GFX_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "events.h"
#include <stdint.h>
#include <stddef.h>

typedef struct gfx_device_s gfx_device_t;
typedef struct gfx_window_s gfx_window_t;

enum gfx_device_backend
{
	GFX_DEVICE_GL3,
	GFX_DEVICE_GL4,
	GFX_DEVICE_D3D9,
	GFX_DEVICE_D3D11,
	GFX_DEVICE_VK,
};

enum gfx_window_backend
{
	GFX_WINDOW_X11,
	GFX_WINDOW_WIN32,
	GFX_WINDOW_WAYLAND,
	GFX_WINDOW_GLFW,
};

enum gfx_native_cursor
{
	GFX_CURSOR_ARROW,
	GFX_CURSOR_CROSS,
	GFX_CURSOR_HAND,
	GFX_CURSOR_IBEAM,
	GFX_CURSOR_NO,
	GFX_CURSOR_SIZEALL,
	GFX_CURSOR_VRESIZE,
	GFX_CURSOR_HRESIZE,
	GFX_CURSOR_WAIT,
	GFX_CURSOR_BLANK,
	GFX_CURSOR_LAST
};

typedef void* gfx_cursor_t;

typedef void(*gfx_key_down_callback_t)(gfx_key_event_t *event);
typedef void(*gfx_key_press_callback_t)(gfx_key_event_t *event);
typedef void(*gfx_key_up_callback_t)(gfx_key_event_t *event);
typedef void(*gfx_char_callback_t)(gfx_char_event_t *event);
typedef void(*gfx_mouse_down_callback_t)(gfx_mouse_event_t *event);
typedef void(*gfx_mouse_up_callback_t)(gfx_mouse_event_t *event);
typedef void(*gfx_scroll_callback_t)(gfx_scroll_event_t *event);
typedef void(*gfx_mouse_move_callback_t)(gfx_pointer_event_t *event);
typedef void(*gfx_cursor_enter_callback_t)();
typedef void(*gfx_cursor_leave_callback_t)();
typedef void(*gfx_focus_in_callback_t)();
typedef void(*gfx_focus_out_callback_t)();
typedef void(*gfx_resize_callback_t)(gfx_resize_event_t *event);
typedef void(*gfx_move_callback_t)(gfx_move_event_t *event);
typedef void(*gfx_expose_callback_t)();
typedef void(*gfx_close_callback_t)();
typedef void(*gfx_error_callback_t)(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

typedef struct gfx_memory_s
{
	void *(*malloc)(size_t size);
	void *(*realloc)(void *ptr, size_t size);
	void (*free)(void *ptr);
} gfx_memory_t;

#define GFX_MALLOC(size) (gfx_memory.malloc ? gfx_memory.malloc(size) : malloc(size))
#define GFX_REALLOC(ptr, size) (gfx_memory.realloc ? gfx_memory.realloc(ptr, size) : realloc(ptr, size))
#define GFX_FREE(ptr) (gfx_memory.free ? gfx_memory.free(ptr) : free(ptr))

#define GFX_ERROR_CALLBACK(...) \
do \
{ \
	if (gfx_error_callback) \
		gfx_error_callback(__VA_ARGS__); \
} while (0)

extern gfx_error_callback_t gfx_error_callback;
extern gfx_memory_t gfx_memory;

typedef struct gfx_window_properties_s
{
	enum gfx_window_backend window_backend;
	enum gfx_device_backend device_backend;
	int8_t depth_bits;
	int8_t stencil_bits;
	int8_t red_bits;
	int8_t green_bits;
	int8_t blue_bits;
	int8_t alpha_bits;
	int8_t samples;
	int8_t srgb;
	int8_t double_buffer;
	int8_t stereo;
} gfx_window_properties_t;

static const int GFX_WINDOW_PROPERTY_DONT_CARE = -1;

typedef struct gfx_window_vtable_s gfx_window_vtable_t;

struct gfx_window_s
{
	const gfx_window_vtable_t *vtable;
	gfx_key_down_callback_t key_down_callback;
	gfx_key_press_callback_t key_press_callback;
	gfx_key_up_callback_t key_up_callback;
	gfx_char_callback_t char_callback;
	gfx_mouse_down_callback_t mouse_down_callback;
	gfx_mouse_up_callback_t mouse_up_callback;
	gfx_scroll_callback_t scroll_callback;
	gfx_mouse_move_callback_t mouse_move_callback;
	gfx_cursor_enter_callback_t cursor_enter_callback;
	gfx_cursor_leave_callback_t cursor_leave_callback;
	gfx_focus_in_callback_t focus_in_callback;
	gfx_focus_out_callback_t focus_out_callback;
	gfx_resize_callback_t resize_callback;
	gfx_move_callback_t move_callback;
	gfx_expose_callback_t expose_callback;
	gfx_close_callback_t close_callback;
	gfx_window_properties_t properties;
	gfx_device_t *device;
	uint8_t keys[(GFX_KEY_LAST + 7) / 8];
	uint8_t mouse_buttons;
	double virtual_x;
	double virtual_y;
	int32_t mouse_x;
	int32_t mouse_y;
	int32_t height;
	int32_t width;
	int32_t x;
	int32_t y;
	bool close_requested;
	bool grabbed;
};

bool gfx_has_window_backend(enum gfx_window_backend backend);
bool gfx_has_device_backend(enum gfx_device_backend backend);

gfx_window_t *gfx_create_window(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties);
void gfx_delete_window(gfx_window_t *window);
bool gfx_create_device(gfx_window_t *window);
void gfx_window_show(gfx_window_t *window);
void gfx_window_hide(gfx_window_t *window);
void gfx_window_poll_events(gfx_window_t *window);
void gfx_window_wait_events(gfx_window_t *window);
void gfx_window_grab_cursor(gfx_window_t *window);
void gfx_window_ungrab_cursor(gfx_window_t *window);
void gfx_window_swap_buffers(gfx_window_t *window);
void gfx_window_make_current(gfx_window_t *window);
void gfx_window_set_swap_interval(gfx_window_t *window, int interval);
void gfx_window_set_title(gfx_window_t *window, const char *title);
void gfx_window_set_icon(gfx_window_t *window, const void *data, uint32_t width, uint32_t height);
void gfx_window_resize(gfx_window_t *window, uint32_t width, uint32_t height);
char *gfx_window_get_clipboard(gfx_window_t *window);
void gfx_window_set_clipboard(gfx_window_t *window, const char *clipboard);
gfx_cursor_t gfx_create_native_cursor(gfx_window_t *window, enum gfx_native_cursor cursor);
gfx_cursor_t gfx_create_cursor(gfx_window_t *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot);
void gfx_delete_cursor(gfx_window_t *window, gfx_cursor_t cursor);
void gfx_set_cursor(gfx_window_t *window, gfx_cursor_t cursor);
void gfx_set_mouse_position(gfx_window_t *window, int32_t x, int32_t y);
int32_t gfx_get_mouse_x(gfx_window_t *window);
int32_t gfx_get_mouse_y(gfx_window_t *window);
bool gfx_is_key_down(gfx_window_t *window, enum gfx_key_code key);
bool gfx_is_mouse_button_down(gfx_window_t *window, enum gfx_mouse_button mouse_button);

void gfx_window_properties_init(gfx_window_properties_t *properties);

#ifdef __cplusplus
}
#endif

#endif
