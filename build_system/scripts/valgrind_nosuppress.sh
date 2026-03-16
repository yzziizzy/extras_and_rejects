#!/bin/bash

source ./build-flags.sh

gcc $gcc_flags _build.c -lutil -o ._build -ggdb \
	&& ./._build -d \
	&&  valgrind --error-limit=no --track-origins=yes  __SED_TOKEN_EXE_REL_PATH $@
	
	
	
# --gen-suppressions=yes
