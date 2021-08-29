#ifndef GFX_X11_WINDOW_H
# define GFX_X11_WINDOW_H

# ifdef __cplusplus
extern "C" {
# endif

#include "../window.h"
#include <X11/extensions/XInput2.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

enum gfx_x11_atom
{
	X11_ATOM_DELETE,
	X11_ATOM_ICON,
	X11_ATOM_NAME,
	X11_ATOM_ICON_NAME,
	X11_ATOM_UTF8,
	X11_ATOM_PID,
	X11_ATOM_PING,
	X11_ATOM_WM_PROTOCOLS,
	X11_ATOM_CLIPBOARD,
	X11_ATOM_SELECTION,
	X11_ATOM_INCR,
	X11_ATOM_XSEL_DATA,
	X11_ATOM_TARGETS,
	X11_ATOM_LAST
};

typedef struct gfx_x11_window_s
{
	gfx_window_t *winref;
	Display *display;
	Cursor cursors[GFX_CURSOR_LAST];
	Window window;
	Window root;
	Atom atoms[X11_ATOM_LAST];
	XIC xic;
	XIM xim;
	int32_t prev_mouse_x;
	int32_t prev_mouse_y;
	char *clipboard;
} gfx_x11_window_t;

bool gfx_x11_create_window(gfx_x11_window_t *window, const char *title, uint32_t width, uint32_t height, XVisualInfo *vi);
bool gfx_x11_ctr(gfx_x11_window_t *window, gfx_window_t *win_ref);
void gfx_x11_dtr(gfx_x11_window_t *window);
void gfx_x11_show(gfx_x11_window_t *window);
void gfx_x11_hide(gfx_x11_window_t *window);
void gfx_x11_set_title(gfx_x11_window_t *window, const char *title);
void gfx_x11_poll_events(gfx_x11_window_t *window);
void gfx_x11_wait_events(gfx_x11_window_t *window);
void gfx_x11_resize(gfx_x11_window_t *window, uint32_t width, uint32_t height);
void gfx_x11_grab_cursor(gfx_x11_window_t *window);
void gfx_x11_ungrab_cursor(gfx_x11_window_t *window);
char *gfx_x11_get_clipboard(gfx_x11_window_t *window);
void gfx_x11_set_clipboard(gfx_x11_window_t *window, const char *text);
void gfx_x11_set_native_cursor(gfx_x11_window_t *window, enum gfx_native_cursor cursor);
void gfx_x11_set_mouse_position(gfx_x11_window_t *window, int32_t x, int32_t y);

# ifdef __cplusplus
}
# endif

#endif
