#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([libjkiss], [4])
AC_CONFIG_SRCDIR([lib/jkiss.c])
AC_CONFIG_HEADERS([config.h])

m4_include(m4/macros/with.m4)

ARG_WITH_SUBST([random-device],
  [/dev/urandom],
  [set the device to read real random data from])

AC_ARG_WITH([libbsd],
  [AS_HELP_STRING([--with-libbsd],[Include -lbsd])],
  [jkiss_libbsd="yes"],
  [])

if test "x$jkiss_libbsd" = "xyes"; then
  AC_DEFINE(JKISS_LIBBSD, 1, [Enable -lbsd])
fi
AM_CONDITIONAL(JKISS_LIBBSD, test "x$jkiss_libbsd" = "xyes")

AC_CONFIG_AUX_DIR([build-aux])
# NOTE older versions of auto* require that one manually mkdir this dir
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign]) # don't insert GNU files
LT_INIT([disable-static])

AC_SUBST(AM_CFLAGS,"-std=c99 -pedantic -Wall -fstack-protector-all")

# Checks for programs.
AC_PROG_CC_C99

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([arc4random])

# KLUGE copied from goptfoo, somewhat servicable means to setup rpath
# linking so do not need LD_LIBRARY_PATH foo
AC_ARG_ENABLE([macos-rpath],
    [  --enable-macos-rpath    enable macOS rpath flags for linker],
    [macos_rpath=${enableval}], [macos_rpath=no])
AC_ARG_ENABLE([the-rpath],
    [  --enable-the-rpath      enable rpath flag for linker],
    [the_rpath=${enableval}], [the_rpath=no])

if test "x${macos_rpath}" = xyes; then
    AC_SUBST([RPATH_FOO], ["-Wl,-macosx_version_min -Wl,10.11 -Wl,-rpath -Wl,\${libdir}"])
elif test "x${the_rpath}" = xyes; then
    AC_SUBST([RPATH_FOO], ["-Wl,-R \${libdir}"])
else
    AC_SUBST([RPATH_FOO], [""])
fi

AC_CONFIG_FILES([Makefile lib/Makefile man/Makefile jkiss.pc])
AC_OUTPUT
