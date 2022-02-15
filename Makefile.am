lib_LTLIBRARIES = libgfx.la

if DEVICE_GL
DEV_GL_SRC = src/devices/gl.c
endif

if DEVICE_GL3
DEV_GL3_SRC = src/devices/gl3.c
endif

if DEVICE_GL4
DEV_GL4_SRC = src/devices/gl4.c
endif

if DEVICE_D3D
DEV_D3D_SRC = src/devices/d3d.c
endif

if DEVICE_D3D9
DEV_D3D11_SRC = src/devices/d3d9.c
endif

if DEVICE_D3D11
DEV_D3D11_SRC = src/devices/d3d11.c
endif

if WINDOW_GLX
WIN_GLX_SRC = src/windows/glx.c
endif

if WINDOW_X11
WIN_X11_SRC = src/windows/x11.c
endif

if WINDOW_GL
WIN_GL_SRC = src/windows/gl.c
endif

if WINDOW_WIN32
WIN_WIN32_SRC = src/windows/win32.c
endif

if WINDOW_D3D
WIN_D3D_SRC = src/windows/d3d.c
endif

if WINDOW_WGL
WIN_WGL_SRC = src/windows/wgl.c
endif

if WINDOW_GLFW
WIN_GLFW_SRC = src/windows/glfw.c
WIN_GLFW_LD = -lglfw
endif

libgfx_la_SOURCES = src/device.c src/window.c \
                    $(DEV_GL_SRC) $(DEV_GL3_SRC) $(DEV_GL4_SRC) \
                    $(DEV_D3D_SRC) $(DEV_D3D9_SRC) $(DEV_D3D11_SRC) \
                    $(WIN_GLX_SRC) $(WIN_X11_SRC) $(WIN_GLFW_SRC) \
                    $(WIN_WGL_SRC) $(WIN_WIN32_SRC) $(WIN_D3D_SRC) \
                    $(WIN_GL_SRC)
libgfx_la_LDFLAGS = -ljks $(GLFW_LD)

pkgincludedir = $(includedir)/gfx
pkginclude_HEADERS = src/device.h src/events.h src/objects.h src/window.h

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
