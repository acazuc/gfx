#ifndef GFX_WIN32_WINDOW_H
# define GFX_WIN32_WINDOW_H

# ifdef __cplusplus
extern "C" {
# endif

# include "../window.h"
# include <windows.h>

typedef struct gfx_win32_window_s
{
	gfx_window_t *winref;
	HINSTANCE hinstance;
	LPCSTR application_name;
	LPCSTR cursor;
	HWND window;
	int32_t prev_mouse_x;
	int32_t prev_mouse_y;
	bool hidden_cursor;
	bool mouse_hover;
	char *classname;
} gfx_win32_window_t;

bool gfx_win32_create_window(gfx_win32_window_t *window, const char *title, uint32_t width, uint32_t height);
void gfx_win32_ctr(gfx_win32_window_t *window, gfx_window_t *win_ref);
void gfx_win32_dtr(gfx_win32_window_t *window);
void gfx_win32_show(gfx_win32_window_t *window);
void gfx_win32_hide(gfx_win32_window_t *window);
void gfx_win32_set_title(gfx_win32_window_t *window, const char *title);
void gfx_win32_poll_events(gfx_win32_window_t *window);
void gfx_win32_wait_events(gfx_win32_window_t *window);
void gfx_win32_resize(gfx_win32_window_t *window, uint32_t width, uint32_t height);
void gfx_win32_grab_cursor(gfx_win32_window_t *window);
void gfx_win32_ungrab_cursor(gfx_win32_window_t *window);
char *gfx_win32_get_clipboard(gfx_win32_window_t *window);
void gfx_win32_set_clipboard(gfx_win32_window_t *window, const char *text);
void gfx_win32_set_native_cursor(gfx_win32_window_t *window, enum gfx_native_cursor cursor);
void gfx_win32_set_mouse_position(gfx_win32_window_t *window, int32_t x, int32_t y);

# ifdef __cplusplus
}
# endif

#endif