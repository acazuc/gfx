#ifndef GFX_WINDOW_VTABLE_H
#define GFX_WINDOW_VTABLE_H

typedef struct gfx_window_vtable_s
{
	bool (*ctr)(gfx_window_t *window, gfx_window_properties_t *properties);
	void (*dtr)(gfx_window_t *window);
	bool (*create_device)(gfx_window_t *window);
	void (*show)(gfx_window_t *window);
	void (*hide)(gfx_window_t *window);
	void (*poll_events)(gfx_window_t *window);
	void (*wait_events)(gfx_window_t *window);
	void (*grab_cursor)(gfx_window_t *window);
	void (*ungrab_cursor)(gfx_window_t *window);
	void (*swap_buffers)(gfx_window_t *window);
	void (*make_current)(gfx_window_t *window);
	void (*set_swap_interval)(gfx_window_t *window, int interval);
	void (*set_title)(gfx_window_t *window, const char *title);
	void (*set_icon)(gfx_window_t *window, const void *data, uint32_t width, uint32_t height);
	void (*resize)(gfx_window_t *window, uint32_t width, uint32_t height);
	char *(*get_clipboard)(gfx_window_t *window);
	void (*set_clipboard)(gfx_window_t *window, const char *clipboard);
	gfx_cursor_t (*create_native_cursor)(gfx_window_t *window, enum gfx_native_cursor cursor);
	gfx_cursor_t (*create_cursor)(gfx_window_t *window, const void *data, uint32_t width, uint32_t height, uint32_t xhot, uint32_t yhot);
	void (*delete_cursor)(gfx_window_t *window, gfx_cursor_t cursor);
	void (*set_cursor)(gfx_window_t *window, gfx_cursor_t cursor);
	void (*set_mouse_position)(gfx_window_t *window, int32_t x, int32_t y);
} gfx_window_vtable_t;

#define GFX_WINDOW_VTABLE_DEF(prefix) \
	.ctr                  = prefix##_ctr, \
	.dtr                  = prefix##_dtr, \
	.create_device        = prefix##_create_device, \
	.show                 = prefix##_show, \
	.hide                 = prefix##_hide, \
	.poll_events          = prefix##_poll_events, \
	.wait_events          = prefix##_wait_events, \
	.grab_cursor          = prefix##_grab_cursor, \
	.ungrab_cursor        = prefix##_ungrab_cursor, \
	.swap_buffers         = prefix##_swap_buffers, \
	.make_current         = prefix##_make_current, \
	.set_swap_interval    = prefix##_set_swap_interval, \
	.set_title            = prefix##_set_title, \
	.set_icon             = prefix##_set_icon, \
	.resize               = prefix##_resize, \
	.get_clipboard        = prefix##_get_clipboard, \
	.set_clipboard        = prefix##_set_clipboard, \
	.create_native_cursor = prefix##_create_native_cursor, \
	.create_cursor        = prefix##_create_cursor, \
	.delete_cursor        = prefix##_delete_cursor, \
	.set_cursor           = prefix##_set_cursor, \
	.set_mouse_position   = prefix##_set_mouse_position,

#endif
