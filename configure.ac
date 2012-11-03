#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.52)
AC_INIT([homebank], [3.3])


AM_CONFIG_HEADER(config.h)

#AM_INIT_AUTOMAKE([1.9 foreign])
AM_INIT_AUTOMAKE([1.9 -Wall -Werror foreign])

# If the source code has changed at all, increment GLADE_REVISION
# If any interfaces have been added, removed, or changed, increment GLADE_CURRENT, and set GLADE_REVISION to 0.
# If any interfaces have been added since the last public release, then increment GLADE_AGE.
# If any interfaces have been removed since the last public release, then set GLADE_AGE to 0.
# Reference: http://www.gnu.org/software/libtool/manual.html#Versioning

# Checks for programs.
AC_PROG_INTLTOOL

AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
PKG_CHECK_MODULES(DEPS, gtk+-2.0 >= 2.10 glib-2.0 >= 2.12)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

# Check for libofx
AC_CHECK_HEADER(libofx/libofx.h,,noofx=true)
AC_ARG_WITH(ofx,
    [  --without-ofx           build without ofx support [default=with]],
    [build_ofx=$withval],
    [build_ofx=yes]
)
if test x$build_ofx != xno
then
    if test x$noofx != xtrue
    then
        AC_CHECK_LIB(ofx, ofx_set_status_cb, OFX_0_7="-DOFX_0_7")
        LDFLAGS="${LDFLAGS} -lofx"
        CFLAGS="${CFLAGS} $OFX_0_7"
    else
        noofx=true
        AC_MSG_RESULT([Libofx header missing. Check your libofx installation])
	CFLAGS="${CFLAGS} -DNOOFX"
    fi
else
    noofx=true
    CFLAGS="${CFLAGS} -DNOOFX"
fi
AM_CONDITIONAL(NOOFX, test x$noofx = xtrue)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([libintl.h locale.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_STRUCT_TM

# Checks for library functions.
AC_CHECK_FUNCS([floor memset modf pow setlocale sqrt])

# International support
ALL_LINGUAS="es fr nl pt_BR ru zh_CN"
AM_GLIB_GNU_GETTEXT

GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["${GETTEXT_PACKAGE}"], [gettext domain])

AC_CONFIG_FILES([
Makefile
po/Makefile.in 
data/homebank.desktop.in
data/Makefile
data/icons/Makefile
data/icons/hicolor/Makefile
data/icons/hicolor/16x16/Makefile
data/icons/hicolor/16x16/apps/Makefile
data/icons/hicolor/24x24/Makefile
data/icons/hicolor/24x24/apps/Makefile
data/icons/hicolor/32x32/Makefile
data/icons/hicolor/32x32/apps/Makefile
data/icons/hicolor/48x48/Makefile
data/icons/hicolor/48x48/apps/Makefile
data/icons/hicolor/scalable/Makefile
data/icons/hicolor/scalable/apps/Makefile
mime/Makefile
src/Makefile 
images/Makefile
])

AC_OUTPUT

echo "

Configuration:

	Source code location:	${srcdir}
	Compiler:		${CC}
	datadir: ${datadir}
	pkgdatadir: ${pkgdatadir}
"
