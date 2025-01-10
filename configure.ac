#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cbsh], [1.0], [shreyashmotunew@gmail.com])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_SRCDIR([main.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h ctype.h stdbool.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strcasecmp strchr strcspn strncpy strtod])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
