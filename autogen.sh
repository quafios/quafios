#! /bin/sh

# halt on error
set -e

# print commands
set -x

# configure.ac --> aclocal.m4
aclocal

# configure.ac --> config.h.in
autoheader

# configure.ac + aclocal.m4 --> configure
autoconf

# config.h + Makefile.am --> Makefile.in
automake -a --foreign

