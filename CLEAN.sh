#!/bin/bash

# aclocal
rm -f -r aclocal.m4 autom4te.cache
# autoheader
rm -f include/G-2301-05-P2-config.h.in
# automake
rm -f include/G-2301-05-P2-config.h
rm -f scripts/compile scripts/depcomp scripts/install-sh scripts/missing
rm -f -r src/.deps
rm -f -r G-2301-05-makefile.in
rm -f -r G-2301-05-makefile
# autoconf
rm -f configure
# configure
rm -f config.log config.status