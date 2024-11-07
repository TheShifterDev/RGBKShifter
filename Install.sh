#!/bin/bash

PROGNAME="rgbkshifter"
DEBGFLAG=" -O0 -ggdb"
COMPFLAG=" -pedantic -Wall -Wshadow"

PNGPP="  `libpng-config --cflags --ldflags` -DUSING_PNGPP"
#PNGHANDROLLED=" -DUSING_PNGHANDROLLED"

SANITYCHECKS=" -DUSING_SANITYCHECKS"

FREETYPE=" `pkg-config --libs --cflags freetype2`"

# compiles and puts a symlink to prog in /usr/bin/
COMMAND="$DEBGFLAG $COMPFLAG $PNGPP $FREETYPE $SANITYCHECKS"
#COMMAND="$DEBGFLAG $COMPFLAG $PNGHANDROLLED $FREETYPE"
######################
COMMAND=$COMMAND" Src/$PROGNAME.cpp -o Built/$PROGNAME.bin"

bear -- g++ $COMMAND
#g++ $COMMAND
sudo ln -f ./Built/$PROGNAME.bin /usr/bin/$PROGNAME
