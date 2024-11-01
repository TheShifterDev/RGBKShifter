#!/bin/bash

PROGNAME="rgbkshifter"
DEBGFLAG=" -O0 -ggdb"
COMPFLAG=" -pedantic -Wall -Wshadow"
LIBPNG=" `libpng-config --cflags --ldflags` -DUSING_LIBPNG"
PNGPP="  `libpng-config --cflags --ldflags` -DUSING_PNGPP"
FREETYPE=" `pkg-config --libs --cflags freetype2`"

U_PNGPP=true
# compiles and puts a symlink to prog in /usr/bin/
#COMMAND="$DEBGFLAG $COMPFLAG $PNGPP $FREETYPE"
COMMAND="$DEBGFLAG $COMPFLAG $FREETYPE"
######################
COMMAND=$COMMAND" Src/$PROGNAME.cpp -o Built/$PROGNAME.bin"

bear -- g++ $COMMAND
#g++ $COMMAND
sudo ln -f ./Built/$PROGNAME.bin /usr/bin/$PROGNAME
