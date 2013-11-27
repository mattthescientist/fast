#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT(fast, 0.6, [m.ruffoni@imperial.ac.uk])
AC_CONFIG_SRCDIR([src/xgline.h])
#AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([gslcblas], cblas_strsv, [],[
	echo "The GNU Scientific Library (GSL) is required to compile FAST"
	exit -1])
PKG_CHECK_MODULES([GTKMM], [gtkmm-2.4 >= 2.4.0])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([pow select sqrt])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
