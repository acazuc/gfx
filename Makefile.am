lib_LTLIBRARIES = libgfx.la

if DEVICE_GL
GL_SRC = src/devices/gl.c
endif

if DEVICE_GL3
GL3_SRC = src/devices/gl3.c
endif

if DEVICE_GL4
GL4_SRC = src/devices/gl4.c
endif

if DEVICE_D3D
D3D_SRC = src/devices/d3d.c
endif

if DEVICE_D3D9
D3D11_SRC = src/devices/d3d9.c
endif

if DEVICE_D3D11
D3D11_SRC = src/devices/d3d11.c
endif

if WINDOW_GLX
GLX_SRC = src/windows/glx.c
endif

if WINDOW_X11
X11_SRC = src/windows/x11.c
endif

if WINDOW_GL
GLW_SRC = src/windows/gl.c
endif

if WINDOW_WIN32
WIN32_SRC = src/windows/win32.c
endif

if WINDOW_D3D
D3D_SRC = src/windows/d3d.c
endif

if WINDOW_GLFW
GLFW_SRC = src/windows/glfw.c
GLFW_LD = -lglfw
endif

libgfx_la_SOURCES = src/device.c src/window.c \
                    $(GL_SRC) $(GL3_SRC) $(GL4_SRC) \
                    $(D3D_SRC) $(D3D9_SRC) $(D3D11_SRC) \
                    $(GLX_SRC) $(X11_SRC) $(GLFW_SRC) \
                    $(WGL_SRC) $(WIN32_SRC) $(D3D_SRC) \
                    $(GLW_SRC)
libgfx_la_LDFLAGS = -ljks $(GLFW_LD)

pkgincludedir = $(includedir)/gfx
pkginclude_HEADERS = src/device.h src/events.h src/objects.h src/window.h

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
