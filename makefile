
# Makefile para facilitar el desarrollo

#aclocal --install -I scripts
#include G-2301-05-makefile

.PHONY: all clean clobber
all clean clobber:
	make -f G-2301-05-makefile $@

G-2301-05-makefile.in: G-2301-05-makefile.am configure.ac
	automake

configure: G-2301-05-makefile.in configure.ac
	autoconf --include=scripts

G-2301-05-makefile: configure
	./configure

