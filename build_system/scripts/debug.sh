#!/bin/bash

gcc _build.c -lutil -o ._build -ggdb \
	&& ./._build -d \
	&& gdb -x ~/.gdbinit -ex=r --args __SED_TOKEN_EXE_REL_PATH $@
