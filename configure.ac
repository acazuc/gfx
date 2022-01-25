AC_INIT([libgfx], [1.0])

AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall foreign])

############
# OpenGL
############
AC_ARG_ENABLE(device-gl3,
	[  --enable-device-gl3     Turn on OpenGL3 device backend],
	[case "${enableval}" in
		yes) device_gl3=true ;;
		no)  device_gl3=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-device-gl3) ;;
	esac],
	[device_gl3=false]
)
if test "x$device_gl3" != "xfalse"
then
  AC_DEFINE([GFX_ENABLE_DEVICE_GL3], [1], [Define to enable OpenGL3 device backend])
fi
AM_CONDITIONAL(DEVICE_GL3, test "x$device_gl3" = "xtrue")

AC_ARG_ENABLE(device-gl4,
	[  --enable-device-gl4     Turn on OpenGL4 device backend],
	[case "${enableval}" in
		yes) device_gl4=true ;;
		no)  device_gl4=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-device-gl4) ;;
	esac],
	[device_gl4=false]
)
if test "x$device_gl4" != "xfalse"
then
  AC_DEFINE([GFX_ENABLE_DEVICE_GL4], [1], [Define to enable OpenGL4 device backend])
fi
AM_CONDITIONAL(DEVICE_GL4, test "x$device_gl4" = "xtrue")

############
# Direct3D
############
AC_ARG_ENABLE(device-d3d9,
	[  --enable-device-d3d9    Turn on Direct3D9 device backend],
	[case "${enableval}" in
		yes) device_d3d9=true ;;
		no)  device_d3d9=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-device-d3d9) ;;
	esac],
	[device_d3d9=false]
)
if test "x$device_d3d9" != "xfalse"
then
  AC_DEFINE([GFX_ENABLE_DEVICE_D3D9], [1], [Define to enable Direct3D9 device backend])
fi
AM_CONDITIONAL(DEVICE_D3D9, test "x$device_d3d9" = "xtrue")

AC_ARG_ENABLE(device-d3d11,
	[  --enable-device-d3d11   Turn on Direct3D11 device backend],
	[case "${enableval}" in
		yes) device_d3d11=true ;;
		no)  device_d3d11=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-device-d3d11) ;;
	esac],
	[device_d3d11=false]
)
if test "x$device_d3d11" != "xfalse"
then
  AC_DEFINE([GFX_ENABLE_DEVICE_D3D11], [1], [Define to enable Direct3D11 device backend])
fi
AM_CONDITIONAL(DEVICE_D3D11, test "x$device_d3d11" = "xtrue")

############
# Windows
############
AC_ARG_ENABLE(window-x11,
	[  --enable-window-x11     Turn on X11 window backend],
	[case "${enableval}" in
		yes) window_x11=true ;;
		no)  window_x11=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-window-x11) ;;
	esac],
	[window_x11=false]
)
if test "x$window_x11" != "xfalse"
then
  AC_DEFINE([GFX_ENABLE_WINDOW_GLX], [1], [Define to enable X11 window backend])
fi
AM_CONDITIONAL(WINDOW_X11, test "x$window_x11" = "xtrue")

AC_ARG_ENABLE(window-win32,
	[  --enable-window-win32     Turn on WIN32 window backend],
	[case "${enableval}" in
		yes) window_win32=true ;;
		no)  window_win32=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-window-win32) ;;
	esac],
	[window_win32=false]
)
if test "x$window_win32" != "xfalse"
then
  AC_DEFINE([GFX_ENABLE_WINDOW_WIN32], [1], [Define to enable WIN32 window backend])
fi
AM_CONDITIONAL(WINDOW_WIN32, test "x$window_win32" = "xtrue")

AC_ARG_ENABLE(window-glfw,
	[  --enable-window-glfw    Turn on GLFW window backend],
	[case "${enableval}" in
		yes) window_glfw=true ;;
		no)  window_glfw=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-window-glfw) ;;
	esac],
	[window_glfw=false]
)
if test "x$window_glfw" != "xfalse"
then
  AC_DEFINE([GFX_ENABLE_WINDOW_GLFW], [1], [Define to enable GLFW window backend])
fi
AM_CONDITIONAL(WINDOW_GLFW, test "x$window_glfw" = "xtrue")

# Aggregate for individual files
AM_CONDITIONAL(DEVICE_GL, test "x$device_gl3$device_gl4" != "xfalsefalse")
AM_CONDITIONAL(DEVICE_D3D, test "x$device_d3d9$device_d3d11" != "xfalsefalse")
AM_CONDITIONAL(WINDOW_GL, test "x$window_glfw$window_x11$window_win32" != "xfalsefalsefalse" -a "x$device_gl3$device_gl4" != "xfalsefalse")
AM_CONDITIONAL(WINDOW_GLX, test "x$window_x11" != "xfalse" -a "x$device_gl3$device_gl4" != "xfalsefalse")
AM_CONDITIONAL(WINDOW_WGL, test "x$window_win32" != "xfalse" -a "x$device_gl3$device_gl4" != "xfalse")
AM_CONDITIONAL(WINDOW_D3D, test "x$window_win32" != "xfalse" -a "x$device_d3d9$device_d3d11" != "xfalse")

AC_ARG_WITH([jks],
	[AS_HELP_STRING([--without-jks],
		[disable support for readline])],
		[],
	[with_readline=yes])

AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AM_PROG_AR
AM_PROG_LIBTOOL

LT_INIT

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
