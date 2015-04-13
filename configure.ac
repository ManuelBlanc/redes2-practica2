#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#AC_PREREQ([2.69])
AC_INIT([redes2-server], [0.1.0], [manuel.blanc@estudiante.uam.es])
AC_CONFIG_AUX_DIR([scripts])
AC_CONFIG_MACRO_DIR([scripts])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_MAINTAINER_MODE([enable])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([include/G-2301-05-P2-config.h])
AC_CONFIG_FILES([G-2301-05-makefile])

# Checks for programs.
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_RANLIB
AM_PROG_AR

# Checks for flags.
AX_CHECK_COMPILE_FLAG([-fdiagnostics-color=auto], [CFLAGS="$CFLAGS -fdiagnostics-color=auto"])

# Checks for libraries.
AC_SEARCH_LIBS([IRC_CommandQuery], [ircredes], [], [
	AC_MSG_WARN([the symbol IRC_CommandQuery could not be found in ircredes])
])

# Checks for header files.
AC_CHECK_HEADERS([redes2/irc.h], [], [
	AC_MSG_WARN([the header redes2/irc.h was not found])
])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# Configuration constants
AC_DEFINE([IRC_MAX_CMD_LEN],      	512,	[Maximum command length])       	# RFC1459 2.3
AC_DEFINE([CHANNEL_MAX_NAME_LEN], 	50, 	[Maximum channel name length])  	# RFC2812 1.3
AC_DEFINE([CHANNEL_MAX_TOPIC_LEN],	50, 	[Maximum channel topic length]) 	#
AC_DEFINE([CHANNEL_MAX_KEY_LEN],  	50, 	[Maximum channel key length])   	#
AC_DEFINE([USER_MAX_AWAY_LEN],    	50, 	[Maximum away message length])  	#
AC_DEFINE([USER_MAX_HOST_LEN],    	64, 	[Maximum host name length])	  	#
AC_DEFINE([USER_MAX_NICK_LEN],    	9,  	[Maximum user length])          	# RFC1459 1.2
AC_DEFINE([USER_MAX_NAME_LEN],    	9,  	[Maximum username length])      	#
AC_DEFINE([USER_MAX_RNAME_LEN],   	50, 	[Maximum user real name length])	#
AC_DEFINE([USER_MAX_PRE_LEN],     	83, 	[Maximum user prefix length])   	# RFC2812 2.3
AC_DEFINE([SERVER_MAX_NAME_LEN],  	50, 	[Maximum server name length])   	#
AC_DEFINE([SERVER_MAX_USERS],  		150, 	[Maximum connections])		   	#
AC_DEFINE([SERVER_MAX_OUT],  		50, 	[Maximum info disconnections saved])		   	#

AC_OUTPUT
