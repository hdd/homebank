dnl Process this file with autoconf to produce a configure script.

AC_INIT(src/homebank.c)
AM_CONFIG_HEADER(config.h)

PACKAGE=homebank
VERSION=3.2

AM_INIT_AUTOMAKE($PACKAGE,$VERSION)

AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_INSTALL

PKG_CHECK_MODULES(DEPS, gtk+-2.0 >= 2.8 glib-2.0 >= 2.2)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

ALL_LINGUAS="es fr nl ru"
AM_GLIB_GNU_GETTEXT

GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["${GETTEXT_PACKAGE}"], [gettext domain])

AC_OUTPUT([
Makefile 
src/Makefile 
images/Makefile
po/Makefile.in 
])
