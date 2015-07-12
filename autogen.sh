#! /bin/sh

aclocal \
&& automake --add-missing -c \
&& autom4te -v --language=M4sh --no-cache  --output=configure autoconf/autoconf.m4 aclocal.m4 configure.ac \
&& rm -fr autom4te.cache