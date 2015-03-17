#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ([2.69])
AC_INIT([redes2-server], [0.1.0], [manuel.blanc@estudiante.uam.es])
AC_CONFIG_AUX_DIR([scripts])
AC_CONFIG_MACRO_DIR([scripts])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([include/G-2301-05-P2-config.h])
AC_CONFIG_FILES([G-2301-05-makefile])

# Checks for programs.
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_RANLIB
AM_PROG_AR

# Checks for libraries.
AC_SEARCH_LIBS([ircredes])

# Checks for header files.
AC_CHECK_HEADERS([redes2/irc.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_OUTPUT
