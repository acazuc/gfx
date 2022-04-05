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

if DEVICE_VK
DEV_VK_SRC = src/devices/vk.c
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
WIN_X11_LD = -lX11 -lXi -lXcursor
endif

if WINDOW_EGL
WIN_EGL_SRC = src/windows/egl.c
WIN_EGL_LD = -lEGL
endif

if WINDOW_WL
WIN_WL_SRC = src/windows/wl.c
WIN_WL_LD = -lwayland-client
endif

if WINDOW_GL
WIN_GL_SRC = src/windows/gl.c
WIN_GL_LD = -lGL
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

if WINDOW_VK_X11
WIN_VK_X11_SRC = src/windows/vk_x11.c
WIN_VK_X11_LD = -lvulkan
endif

if WINDOW_VK_WL
WIN_VK_WL_SRC = src/windows/vk_wl.c
WIN_VK_WL_LD = -lvulkan
endif

if WINDOW_VK_WIN32
WIN_VK_WIN32_SRC = src/windows/vk_win32.c
WIN_VK_WIN32_LD = -lvulkan
endif

libgfx_la_SOURCES = src/device.c src/window.c \
                    $(DEV_GL_SRC) $(DEV_GL3_SRC) $(DEV_GL4_SRC) \
                    $(DEV_D3D_SRC) $(DEV_D3D9_SRC) $(DEV_D3D11_SRC) \
                    $(DEV_VK_SRC) $(WIN_GLX_SRC) $(WIN_X11_SRC) \
                    $(WIN_GLFW_SRC) $(WIN_WGL_SRC) $(WIN_WIN32_SRC) \
                    $(WIN_D3D_SRC) $(WIN_GL_SRC) $(WIN_WL_SRC) \
                    $(WIN_EGL_SRC)  $(WIN_VK_X11_SRC) $(WIN_VK_WL_SRC) \
                    $(WIN_VK_WIN32_SRC)

libgfx_la_LDFLAGS = -ljks $(GLFW_LD) $(WIN_X11_LD) $(WIN_GL_LD) $(WIN_WL_LD) \
                    $(WIN_VK_X11_LD) $(WIN_VK_WIN32_LD) $(WIN_VK_WL_LD) \
                    $(WIN_EGL_LD)

pkgincludedir = $(includedir)/gfx
pkginclude_HEADERS = src/device.h src/events.h src/objects.h src/window.h

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
