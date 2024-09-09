#!/bin/bash

PROGNAME="rgbkshifter"
DEBGFLAG=" -O0 -ggdb"
COMPFLAG=" -pedantic -Wall -Wshadow"
LIBPNG=" `libpng-config --cflags --ldflags`"

U_PNGPP=true
# compiles and puts a symlink to prog in /usr/bin/
COMMAND="$DEBGFLAG $COMPFLAG $LIBPNG -DUSING_PNGPP"
######################
COMMAND=$COMMAND" Src/main.cpp -o Built/$PROGNAME.bin"

bear -- g++ $COMMAND
sudo ln -f ./Built/$PROGNAME.bin /usr/bin/$PROGNAME
