#!/bin/bash


rm ../_build.inc.c

cat header.c \
	string.c \
	strcache.c \
	hash.c \
	strlist.c \
	fs.c \
	cprocs.c \
	pkgconfig.c \
	rglob.c \
	gcc.c \
	\
	>> ../_build.inc.c







