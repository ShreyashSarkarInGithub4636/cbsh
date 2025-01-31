#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cbsh], [1.2], [shreyashmotunew@gmail.com])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_SRCDIR([main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_GREP

# Allow user-specified CC, default to gcc
AC_ARG_VAR([CC], [C compiler (default: gcc)])
test -z "$CC" && CC=gcc
AC_SUBST([CC])

# Check for C99 and C11 support
AC_MSG_CHECKING([for C99 support])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#if __STDC_VERSION__ < 199901L
#error "C99 not supported"
#endif]], [[;]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_ERROR([C99 support required.])])

AC_MSG_CHECKING([for C11 support (optional)])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#if __STDC_VERSION__ < 201112L
#error "C11 not supported"
#endif]], [[;]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_WARN([C11 not supported, continuing without it.])])

# Checks for required libraries.
AC_CHECK_LIB([ncurses], [initscr], [], [AC_MSG_ERROR([ncurses library not found])])
AC_CHECK_LIB([readline], [readline], [], [AC_MSG_ERROR([readline library not found])])

# Static linking support
AC_ARG_ENABLE([static],
    [AS_HELP_STRING([--enable-static], [Enable static linking])],
    [enable_static=$enableval], [enable_static=no])

if test "x$enable_static" = "xyes"; then
    AC_MSG_CHECKING([for static ncurses])
    if test -f "/usr/lib/libncurses.a" || test -f "/usr/lib64/libncurses.a"; then
        AC_MSG_RESULT([yes])
        STATIC_LIBS="-static -lncurses"
    else
        AC_MSG_ERROR([Static ncurses library not found])
    fi

    AC_MSG_CHECKING([for static readline])
    if test -f "/usr/lib/libreadline.a" || test -f "/usr/lib64/libreadline.a"; then
        AC_MSG_RESULT([yes])
        STATIC_LIBS="$STATIC_LIBS -static -lreadline"
    else
        AC_MSG_ERROR([Static readline library not found])
    fi

    AC_MSG_CHECKING([for static libc])
    if test -f "/usr/lib/libc.a" || test -f "/usr/lib64/libc.a"; then
        AC_MSG_RESULT([yes])
        STATIC_LIBS="$STATIC_LIBS -static"
    else
        AC_MSG_ERROR([Static libc library not found])
    fi

    # Add -static to CFLAGS if --static is specified
    CFLAGS="$CFLAGS -static"
fi

AC_SUBST([STATIC_LIBS])

# Allow user-specified LDFLAGS, CFLAGS
AC_ARG_VAR([LDFLAGS], [Linker flags])
AC_ARG_VAR([CFLAGS], [Compiler flags])

# Allow user to specify TARGET and BUILD_HOST
AC_ARG_VAR([TARGET], [Target system])
AC_ARG_VAR([BUILD_HOST], [Build host system])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h ctype.h stdbool.h ncurses.h readline/readline.h readline/history.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strcasecmp strchr strcspn strncpy strtod])

# Menuconfig using dialog
AC_ARG_ENABLE([menuconfig],
    [AS_HELP_STRING([--enable-menuconfig], [Enable interactive configuration using menuconfig])],
    [enable_menuconfig=$enableval], [enable_menuconfig=no])

if test "x$enable_menuconfig" = "xyes"; then
    AC_MSG_CHECKING([for dialog])
    AC_CHECK_PROG([DIALOG], [dialog], [yes], [no])

    if test "x$DIALOG" = "xno"; then
        AC_MSG_ERROR([Dialog is required for menuconfig])
    fi

    AC_MSG_RESULT([yes])

    AC_CONFIG_FILES([menuconfig.sh])
fi

# Ensure the Makefile.in is generated correctly
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
