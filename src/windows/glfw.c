#include "glfw.h"
#include "../window_vtable.h"
#include "../config.h"
#include <GLFW/glfw3.h>
#include <jks/utf8.h>
#include <stdlib.h>
#include <string.h>

typedef struct gfx_glfw_window_s
{
	gfx_gl_window_t gl;
	GLFWwindow *window;
} gfx_glfw_window_t;

#define GL_WINDOW ((gfx_gl_window_t*)window)
#define GLFW_WINDOW ((gfx_glfw_window_t*)window)

static const int cursors[GFX_CURSOR_LAST] =
{
	[GFX_CURSOR_ARROW] = GLFW_ARROW_CURSOR,
	[GFX_CURSOR_CROSS] = GLFW_CROSSHAIR_CURSOR,
	[GFX_CURSOR_HAND] = GLFW_HAND_CURSOR,
	[GFX_CURSOR_IBEAM] = GLFW_IBEAM_CURSOR,
	[GFX_CURSOR_NO] = GLFW_ARROW_CURSOR,
	[GFX_CURSOR_SIZEALL] = GLFW_ARROW_CURSOR,
	[GFX_CURSOR_VRESIZE] = GLFW_VRESIZE_CURSOR,
	[GFX_CURSOR_HRESIZE] = GLFW_HRESIZE_CURSOR,
	[GFX_CURSOR_WAIT] = GLFW_ARROW_CURSOR,
	[GFX_CURSOR_BLANK] = 0,
};

static void on_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void on_character_callback(GLFWwindow *window, unsigned int codepoint);
static void on_cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
static void on_cursor_enter_callback(GLFWwindow *window, int entered);
static void on_mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
static void on_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
static void on_window_focus_callback(GLFWwindow *window, int focused);
static void on_framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void on_window_pos_callback(GLFWwindow *window, int xpos, int ypos);
static void on_window_close_callback(GLFWwindow *window);
static void on_window_refresh_callback(GLFWwindow *window);
static enum gfx_key_code get_key_code(int key_code);
static enum gfx_mouse_button get_button(int button);
static uint32_t get_mods(int state);

static void *get_proc_address(const char *name)
{
	return (void*)glfwGetProcAddress(name);
}

static bool glfw_ctr(gfx_window_t *window, gfx_window_properties_t *properties)
{
	return gfx_window_vtable.ctr(window, properties);
}

static void glfw_dtr(gfx_window_t *window)
{
	glfwDestroyWindow(GLFW_WINDOW->window);
	gfx_window_vtable.dtr(window);
}

static bool glfw_create_device(gfx_window_t *window)
{
	return gfx_gl_create_device(window, get_proc_address);
}

static void glfw_show(gfx_window_t *window)
{
	glfwShowWindow(GLFW_WINDOW->window);
}

static void glfw_hide(gfx_window_t *window)
{
	glfwHideWindow(GLFW_WINDOW->window);
}

static void glfw_poll_events(gfx_window_t *window)
{
	(void)window;
	glfwPollEvents();
}

static void glfw_wait_events(gfx_window_t *window)
{
	(void)window;
	glfwWaitEvents();
}

static void glfw_grab_cursor(gfx_window_t *window)
{
	glfwSetInputMode(GLFW_WINDOW->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	window->grabbed = true;
}

static void glfw_ungrab_cursor(gfx_window_t *window)
{
	glfwSetInputMode(GLFW_WINDOW->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	window->grabbed = false;
}

static void glfw_swap_buffers(gfx_window_t *window)
{
	glfwSwapBuffers(GLFW_WINDOW->window);
}

static void glfw_make_current(gfx_window_t *window)
{
	glfwMakeContextCurrent(GLFW_WINDOW->window);
}

static void glfw_set_swap_interval(gfx_window_t *window, int interval)
{
	(void)window;
	glfwSwapInterval(interval);
}

static void glfw_set_title(gfx_window_t *window, const char *title)
{
	glfwSetWindowTitle(GLFW_WINDOW->window, title);
}

static void glfw_set_icon(gfx_window_t *window, const void *data, uint32_t width, uint32_t height)
{
	GLFWimage image;
	image.width = width;
	image.height = height;
	image.pixels = (unsigned char*)data;
	glfwSetWindowIcon(GLFW_WINDOW->window, 1, &image);
}

static void glfw_resize(gfx_window_t *window, uint32_t width, uint32_t height)
{
	glfwSetWindowSize(GLFW_WINDOW->window, width, height);
}

static char *glfw_get_clipboard(gfx_window_t *window)
{
	return strdup(glfwGetClipboardString(GLFW_WINDOW->window));
}

static void glfw_set_clipboard(gfx_window_t *window, const char *text)
{
	glfwSetClipboardString(GLFW_WINDOW->window, text);
}

static gfx_cursor_t glfw_create_native_cursor(gfx_window_t *window, enum gfx_native_cursor cursor)
{
	(void)window;
	return glfwCreateStandardCursor(cursors[cursor]);
}

static gfx_cursor_t glfw_create_cursor(gfx_window_t *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot)
{
	(void)window;
	GLFWimage image;
	image.width = width;
	image.height = height;
	image.pixels = (unsigned char*)data;
	return glfwCreateCursor(&image, xhot, yhot);
}

static void glfw_delete_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	(void)window;
	if (!cursor)
		return;
	glfwDestroyCursor((GLFWcursor*)cursor);
}

static void glfw_set_cursor(gfx_window_t *window, gfx_cursor_t cursor)
{
	glfwSetCursor(GLFW_WINDOW->window, cursor);
}

static void glfw_set_mouse_position(gfx_window_t *window, int32_t x, int32_t y)
{
	glfwSetCursorPos(GLFW_WINDOW->window, x, y);
}

static const gfx_window_vtable_t glfw_vtable =
{
	GFX_WINDOW_VTABLE_DEF(glfw)
};

gfx_window_t *gfx_glfw_window_new(const char *title, uint32_t width, uint32_t height, gfx_window_properties_t *properties)
{
	if (!glfwInit())
		return NULL;
	gfx_window_t *window = GFX_MALLOC(sizeof(gfx_glfw_window_t));
	if (!window)
		return NULL;
	memset(window, 0, sizeof(gfx_glfw_window_t));
	window->vtable = &glfw_vtable;
	if (!window->vtable->ctr(window, properties))
		goto err;
	glfwDefaultWindowHints();
	if (properties->depth_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
		glfwWindowHint(GLFW_DEPTH_BITS, properties->depth_bits);
	if (properties->stencil_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
		glfwWindowHint(GLFW_STENCIL_BITS, properties->stencil_bits);
	if (properties->red_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
		glfwWindowHint(GLFW_RED_BITS, properties->red_bits);
	if (properties->green_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
		glfwWindowHint(GLFW_GREEN_BITS, properties->green_bits);
	if (properties->blue_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
		glfwWindowHint(GLFW_BLUE_BITS, properties->blue_bits);
	if (properties->alpha_bits != GFX_WINDOW_PROPERTY_DONT_CARE)
		glfwWindowHint(GLFW_ALPHA_BITS, properties->alpha_bits);
	glfwWindowHint(GLFW_SAMPLES, properties->samples);
	if (properties->srgb != GFX_WINDOW_PROPERTY_DONT_CARE)
		glfwWindowHint(GLFW_SRGB_CAPABLE, properties->srgb ? GLFW_TRUE : GLFW_FALSE);
	if (properties->double_buffer != GFX_WINDOW_PROPERTY_DONT_CARE)
		glfwWindowHint(GLFW_DOUBLEBUFFER, properties->double_buffer ? GLFW_TRUE : GLFW_FALSE);
	if (properties->stereo != GFX_WINDOW_PROPERTY_DONT_CARE)
		glfwWindowHint(GLFW_STEREO, properties->stereo ? GLFW_TRUE : GLFW_FALSE);
#ifdef GFX_ENABLE_DEVICE_GL3
	if (properties->device_backend == GFX_DEVICE_GL3)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	}
#endif
#ifdef GFX_ENABLE_DEVICE_GL4
	if (properties->device_backend == GFX_DEVICE_GL4)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	}
#endif
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	window->width = width;
	window->height = height;
	GLFW_WINDOW->window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!GLFW_WINDOW->window)
		goto err;
	glfwSetWindowUserPointer(GLFW_WINDOW->window, window);
	glfwSetKeyCallback(GLFW_WINDOW->window, on_key_callback);
	glfwSetCharCallback(GLFW_WINDOW->window, on_character_callback);
	glfwSetCursorPosCallback(GLFW_WINDOW->window, on_cursor_position_callback);
	glfwSetCursorEnterCallback(GLFW_WINDOW->window, on_cursor_enter_callback);
	glfwSetMouseButtonCallback(GLFW_WINDOW->window, on_mouse_button_callback);
	glfwSetScrollCallback(GLFW_WINDOW->window, on_scroll_callback);
	glfwSetWindowFocusCallback(GLFW_WINDOW->window, on_window_focus_callback);
	glfwSetFramebufferSizeCallback(GLFW_WINDOW->window, on_framebuffer_size_callback);
	glfwSetWindowPosCallback(GLFW_WINDOW->window, on_window_pos_callback);
	glfwSetWindowCloseCallback(GLFW_WINDOW->window, on_window_close_callback);
	glfwSetWindowRefreshCallback(GLFW_WINDOW->window, on_window_refresh_callback);
	return window;

err:
	window->vtable->dtr(window);
	GFX_FREE(window);
	return NULL;
}

static void on_key_callback(GLFWwindow *win, int key, int scancode, int action, int mods)
{
	gfx_window_t *window = glfwGetWindowUserPointer(win);
	(void)scancode;
	enum gfx_key_code key_code = get_key_code(key);
	switch (action)
	{
		case GLFW_PRESS:
			window->keys[key_code / 8] |= 1 << (key_code % 8);
			if (window->key_down_callback)
			{
				gfx_key_event_t event;
				event.used = false;
				event.key = key_code;
				event.mods = get_mods(mods);
				window->key_down_callback(&event);
			}
			break;
		case GLFW_RELEASE:
			window->keys[key_code / 8] &= ~(1 << (key_code % 8));
			if (window->key_up_callback)
			{
				gfx_key_event_t event;
				event.used = false;
				event.key = key_code;
				event.mods = get_mods(mods);
				window->key_up_callback(&event);
			}
			break;
		case GLFW_REPEAT:
			if (window->key_press_callback)
			{
				gfx_key_event_t event;
				event.used = false;
				event.key = key_code;
				event.mods = get_mods(mods);
				window->key_press_callback(&event);
			}
			break;
	}
}

static void on_character_callback(GLFWwindow *win, unsigned int codepoint)
{
	gfx_window_t *window = glfwGetWindowUserPointer(win);
	if (window->char_callback)
	{
		gfx_char_event_t event;
		event.used = false;
		event.codepoint = codepoint;
		memset(event.utf8, 0, sizeof(event.utf8));
		char *pos = &event.utf8[0];
		if (utf8_encode(&pos, codepoint))
		{
			window->char_callback(&event);
		}
		else
		{
			GFX_ERROR_CALLBACK("invalid input codepoint");
		}
	}
}

static void on_cursor_position_callback(GLFWwindow *win, double xpos, double ypos)
{
	gfx_window_t *window = glfwGetWindowUserPointer(win);
	if (window->grabbed)
	{
		window->virtual_x = xpos;
		window->virtual_y = ypos;
	}
	else
	{
		window->mouse_x = xpos;
		window->mouse_y = ypos;
	}
	if (window->mouse_move_callback)
	{
		gfx_pointer_event_t event;
		event.used = false;
		event.x = xpos;
		event.y = ypos;
		window->mouse_move_callback(&event);
	}
}

static void on_cursor_enter_callback(GLFWwindow *win, int entered)
{
	gfx_window_t *window = glfwGetWindowUserPointer(win);
	if (entered)
	{
		if (window->cursor_enter_callback)
			window->cursor_enter_callback();
	}
	else
	{
		if (window->cursor_leave_callback)
			window->cursor_leave_callback();
	}
}

static void on_mouse_button_callback(GLFWwindow *win, int button, int action, int mods)
{
	gfx_window_t *window = glfwGetWindowUserPointer(win);
	switch (action)
	{
		case GLFW_PRESS:
			if (window->mouse_down_callback)
			{
				gfx_mouse_event_t event;
				event.used = false;
				event.x = gfx_get_mouse_x(window);
				event.y = gfx_get_mouse_y(window);
				event.button = get_button(button);
				event.mods = get_mods(mods);
				window->mouse_down_callback(&event);
			}
			break;
		case GLFW_RELEASE:
			if (window->mouse_up_callback)
			{
				gfx_mouse_event_t event;
				event.used = false;
				event.x = gfx_get_mouse_x(window);
				event.y = gfx_get_mouse_y(window);
				event.button = get_button(button);
				event.mods = get_mods(mods);
				window->mouse_up_callback(&event);
			}
			break;
	}
}

static void on_scroll_callback(GLFWwindow *win, double xoffset, double yoffset)
{
	gfx_window_t *window = glfwGetWindowUserPointer(win);
	if (window->scroll_callback)
	{
		gfx_scroll_event_t event;
		event.used = false;
		event.mouse_x = gfx_get_mouse_x(window);
		event.mouse_y = gfx_get_mouse_y(window);
		event.x = xoffset;
		event.y = yoffset;
		window->scroll_callback(&event);
	}
}

static void on_window_focus_callback(GLFWwindow *win, int focused)
{
	gfx_window_t *window = glfwGetWindowUserPointer(win);
	if (focused)
	{
		if (window->focus_in_callback)
			window->focus_in_callback();
	}
	else
	{
		if (window->focus_out_callback)
			window->focus_out_callback();
	}
}

static void on_framebuffer_size_callback(GLFWwindow *win, int width, int height)
{
	gfx_window_t *window = glfwGetWindowUserPointer(win);
	window->width = width;
	window->height = height;
	if (window->resize_callback)
	{
		gfx_resize_event_t event;
		event.used = false;
		event.width = width;
		event.height = height;
		window->resize_callback(&event);
	}
}

static void on_window_pos_callback(GLFWwindow *win, int xpos, int ypos)
{
	gfx_window_t *window = glfwGetWindowUserPointer(win);
	window->x = xpos;
	window->y = ypos;
	if (window->move_callback)
	{
		gfx_move_event_t event;
		event.used = false;
		event.x = xpos;
		event.y = ypos;
		window->move_callback(&event);
	}
}

static void on_window_close_callback(GLFWwindow *win)
{
	gfx_window_t *window = glfwGetWindowUserPointer(win);
	window->close_requested = true;
	if (window->close_callback)
		window->close_callback();
}

static void on_window_refresh_callback(GLFWwindow *win)
{
	gfx_window_t *window = glfwGetWindowUserPointer(win);
	if (window->expose_callback)
		window->expose_callback();
}

static enum gfx_key_code get_key_code(int key_code)
{
	switch (key_code)
	{
		case GLFW_KEY_A:		return GFX_KEY_A;
		case GLFW_KEY_B:		return GFX_KEY_B;
		case GLFW_KEY_C:		return GFX_KEY_C;
		case GLFW_KEY_D:		return GFX_KEY_D;
		case GLFW_KEY_E:		return GFX_KEY_E;
		case GLFW_KEY_F:		return GFX_KEY_F;
		case GLFW_KEY_G:		return GFX_KEY_G;
		case GLFW_KEY_H:		return GFX_KEY_H;
		case GLFW_KEY_I:		return GFX_KEY_I;
		case GLFW_KEY_J:		return GFX_KEY_J;
		case GLFW_KEY_K:		return GFX_KEY_K;
		case GLFW_KEY_L:		return GFX_KEY_L;
		case GLFW_KEY_M:		return GFX_KEY_M;
		case GLFW_KEY_N:		return GFX_KEY_N;
		case GLFW_KEY_O:		return GFX_KEY_O;
		case GLFW_KEY_P:		return GFX_KEY_P;
		case GLFW_KEY_Q:		return GFX_KEY_Q;
		case GLFW_KEY_R:		return GFX_KEY_R;
		case GLFW_KEY_S:		return GFX_KEY_S;
		case GLFW_KEY_T:		return GFX_KEY_T;
		case GLFW_KEY_U:		return GFX_KEY_U;
		case GLFW_KEY_V:		return GFX_KEY_V;
		case GLFW_KEY_W:		return GFX_KEY_W;
		case GLFW_KEY_X:		return GFX_KEY_X;
		case GLFW_KEY_Y:		return GFX_KEY_Y;
		case GLFW_KEY_Z:		return GFX_KEY_Z;
		case GLFW_KEY_0:		return GFX_KEY_0;
		case GLFW_KEY_1:		return GFX_KEY_1;
		case GLFW_KEY_2:		return GFX_KEY_2;
		case GLFW_KEY_3:		return GFX_KEY_3;
		case GLFW_KEY_4:		return GFX_KEY_4;
		case GLFW_KEY_5:		return GFX_KEY_5;
		case GLFW_KEY_6:		return GFX_KEY_6;
		case GLFW_KEY_7:		return GFX_KEY_7;
		case GLFW_KEY_8:		return GFX_KEY_8;
		case GLFW_KEY_9:		return GFX_KEY_9;
		case GLFW_KEY_KP_0:		return GFX_KEY_KP_0;
		case GLFW_KEY_KP_1:		return GFX_KEY_KP_1;
		case GLFW_KEY_KP_2:		return GFX_KEY_KP_2;
		case GLFW_KEY_KP_3:		return GFX_KEY_KP_3;
		case GLFW_KEY_KP_4:		return GFX_KEY_KP_4;
		case GLFW_KEY_KP_5:		return GFX_KEY_KP_5;
		case GLFW_KEY_KP_6:		return GFX_KEY_KP_6;
		case GLFW_KEY_KP_7:		return GFX_KEY_KP_7;
		case GLFW_KEY_KP_8:		return GFX_KEY_KP_8;
		case GLFW_KEY_KP_9:		return GFX_KEY_KP_9;
		case GLFW_KEY_KP_DIVIDE:	return GFX_KEY_KP_DIVIDE;
		case GLFW_KEY_KP_MULTIPLY:	return GFX_KEY_KP_MULTIPLY;
		case GLFW_KEY_KP_SUBTRACT:	return GFX_KEY_KP_SUBTRACT;
		case GLFW_KEY_KP_ADD:		return GFX_KEY_KP_ADD;
		case GLFW_KEY_KP_EQUAL:		return GFX_KEY_KP_EQUAL;
		case GLFW_KEY_KP_DECIMAL:	return GFX_KEY_KP_DECIMAL;
		case GLFW_KEY_KP_ENTER:		return GFX_KEY_KP_ENTER;
		case GLFW_KEY_F1:		return GFX_KEY_F1;
		case GLFW_KEY_F2:		return GFX_KEY_F2;
		case GLFW_KEY_F3:		return GFX_KEY_F3;
		case GLFW_KEY_F4:		return GFX_KEY_F4;
		case GLFW_KEY_F5:		return GFX_KEY_F5;
		case GLFW_KEY_F6:		return GFX_KEY_F6;
		case GLFW_KEY_F7:		return GFX_KEY_F7;
		case GLFW_KEY_F8:		return GFX_KEY_F8;
		case GLFW_KEY_F9:		return GFX_KEY_F9;
		case GLFW_KEY_F10:		return GFX_KEY_F10;
		case GLFW_KEY_F11:		return GFX_KEY_F11;
		case GLFW_KEY_F12:		return GFX_KEY_F12;
		case GLFW_KEY_F13:		return GFX_KEY_F13;
		case GLFW_KEY_F14:		return GFX_KEY_F14;
		case GLFW_KEY_F15:		return GFX_KEY_F15;
		case GLFW_KEY_F16:		return GFX_KEY_F16;
		case GLFW_KEY_F17:		return GFX_KEY_F17;
		case GLFW_KEY_F18:		return GFX_KEY_F18;
		case GLFW_KEY_F19:		return GFX_KEY_F19;
		case GLFW_KEY_F20:		return GFX_KEY_F20;
		case GLFW_KEY_F21:		return GFX_KEY_F21;
		case GLFW_KEY_F22:		return GFX_KEY_F22;
		case GLFW_KEY_F23:		return GFX_KEY_F23;
		case GLFW_KEY_F24:		return GFX_KEY_F24;
		case GLFW_KEY_LEFT_SHIFT:	return GFX_KEY_LSHIFT;
		case GLFW_KEY_RIGHT_SHIFT:	return GFX_KEY_RSHIFT;
		case GLFW_KEY_LEFT_CONTROL:	return GFX_KEY_LCONTROL;
		case GLFW_KEY_RIGHT_CONTROL:	return GFX_KEY_RCONTROL;
		case GLFW_KEY_LEFT_ALT:		return GFX_KEY_LALT;
		case GLFW_KEY_RIGHT_ALT:	return GFX_KEY_RALT;
		case GLFW_KEY_LEFT_SUPER:	return GFX_KEY_LSUPER;
		case GLFW_KEY_RIGHT_SUPER:	return GFX_KEY_RSUPER;
		case GLFW_KEY_LEFT:		return GFX_KEY_LEFT;
		case GLFW_KEY_RIGHT:		return GFX_KEY_RIGHT;
		case GLFW_KEY_UP:		return GFX_KEY_UP;
		case GLFW_KEY_DOWN:		return GFX_KEY_DOWN;
		case GLFW_KEY_SPACE:		return GFX_KEY_SPACE;
		case GLFW_KEY_BACKSPACE:	return GFX_KEY_BACKSPACE;
		case GLFW_KEY_ENTER:		return GFX_KEY_ENTER;
		case GLFW_KEY_TAB:		return GFX_KEY_TAB;
		case GLFW_KEY_ESCAPE:		return GFX_KEY_ESCAPE;
		case GLFW_KEY_PAUSE:		return GFX_KEY_PAUSE;
		case GLFW_KEY_DELETE:		return GFX_KEY_DELETE;
		case GLFW_KEY_INSERT:		return GFX_KEY_INSERT;
		case GLFW_KEY_HOME:		return GFX_KEY_HOME;
		case GLFW_KEY_PAGE_UP:		return GFX_KEY_PAGE_UP;
		case GLFW_KEY_PAGE_DOWN:	return GFX_KEY_PAGE_DOWN;
		case GLFW_KEY_END:		return GFX_KEY_END;
		case GLFW_KEY_COMMA:		return GFX_KEY_COMMA;
		case GLFW_KEY_PERIOD:		return GFX_KEY_PERIOD;
		case GLFW_KEY_SLASH:		return GFX_KEY_SLASH;
		case GLFW_KEY_APOSTROPHE:	return GFX_KEY_APOSTROPHE;
		case GLFW_KEY_SEMICOLON:	return GFX_KEY_SEMICOLON;
		case GLFW_KEY_GRAVE_ACCENT:	return GFX_KEY_GRAVE;
		case GLFW_KEY_LEFT_BRACKET:	return GFX_KEY_LBRACKET;
		case GLFW_KEY_RIGHT_BRACKET:	return GFX_KEY_RBRACKET;
		case GLFW_KEY_BACKSLASH:	return GFX_KEY_BACKSLASH;
		case GLFW_KEY_EQUAL:		return GFX_KEY_EQUAL;
		case GLFW_KEY_MINUS:		return GFX_KEY_SUBTRACT;
		case GLFW_KEY_SCROLL_LOCK:	return GFX_KEY_SCROLL_LOCK;
		case GLFW_KEY_NUM_LOCK:		return GFX_KEY_NUM_LOCK;
		case GLFW_KEY_CAPS_LOCK:	return GFX_KEY_CAPS_LOCK;
		case GLFW_KEY_PRINT_SCREEN:	return GFX_KEY_PRINT;
	}
	return GFX_KEY_UNKNOWN;
}

static uint32_t get_mods(int state)
{
	uint32_t mods = 0;
	if (state & GLFW_MOD_CONTROL)
		mods |= GFX_KEY_MOD_CONTROL;
	if (state & GLFW_MOD_SHIFT)
		mods |= GFX_KEY_MOD_SHIFT;
	if (state & GLFW_MOD_CAPS_LOCK)
		mods |= GFX_KEY_MOD_CAPS_LOCK;
	if (state & GLFW_MOD_ALT)
		mods |= GFX_KEY_MOD_ALT;
	if (state & GLFW_MOD_NUM_LOCK)
		mods |= GFX_KEY_MOD_NUM_LOCK;
	if (state & GLFW_MOD_SUPER)
		mods |= GFX_KEY_MOD_SUPER;
	return mods;
}

static enum gfx_mouse_button get_button(int button)
{
	switch (button)
	{
		case GLFW_MOUSE_BUTTON_LEFT:
			return GFX_MOUSE_BUTTON_LEFT;
		case GLFW_MOUSE_BUTTON_RIGHT:
			return GFX_MOUSE_BUTTON_RIGHT;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			return GFX_MOUSE_BUTTON_MIDDLE;
	}
	return (enum gfx_mouse_button)button;
}
