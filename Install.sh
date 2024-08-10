#!/bin/bash

PROGNAME="rgbkshifter"
DEBGFLAG=" -O0 -ggdb"
COMPFLAG=" -pedantic -Wall -Wshadow"
LIBPNG=" `libpng-config --cflags --ldflags`"


# PREMADE = false
# IMPORTANT: only have 1 enable at a time or compiler will complain
U_PNGPP=true


# compiles and puts a symlink to prog in /usr/bin/
#bear -- g++ $DEBGFLAG $COMPFLAG $LIBPNG Src/main.cpp -o Built/$PROGNAME.bin
COMMAND="$DEBGFLAG $COMPFLAG"
######################
if $U_PNGPP; then # requires png++ (c++ wrapper for libpng)
COMMAND="$COMMAND $LIBPNG -DUSING_PNGPP"
fi
######################
COMMAND=$COMMAND" Src/main.cpp -o Built/$PROGNAME.bin"
g++ $COMMAND
sudo ln -f ./Built/$PROGNAME.bin /usr/bin/$PROGNAME

#copys rgbkShifters includes into /usr/include/rgbkShifter/
sudo rm -rf /usr/include/$PROGNAME
sudo mkdir /usr/include/$PROGNAME
sudo cp Include/* /usr/include/$PROGNAME/