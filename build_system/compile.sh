#!/bin/bash


rm scripts/_build.inc.c

cat parts/strlist.c parts/header.c \
	parts/string.c \
	parts/strcache.c \
	parts/hash.c \
	parts/fs.c \
	parts/pkgconfig.c \
	parts/init.c \
	parts/cprocs.c \
	parts/rglob.c \
	parts/spirv.c \
	parts/gcc.c \
	\
	>> scripts/_build.inc.c







