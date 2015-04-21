#!/bin/sh

aclocal --install -I scripts
autoconf --include=scripts
automake
./configure

