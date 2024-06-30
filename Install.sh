#!/bin/bash

PROGNAME="rgbkshifter"
DEBGFLAG=" -O0 -ggdb"
COMPFLAG=" -pedantic -Wall -Wshadow"
LIBPNG=" `libpng-config --cflags --ldflags`"
# compiles and puts a symlink to prog in /usr/bin/
bear -- g++ $DEBGFLAG $COMPFLAG $LIBPNG Src/main.cpp -o Built/$PROGNAME.bin
sudo ln -f -s Built/$PROGNAME.bin /usr/bin/$PROGNAME

#copys rgbkShifters includes into /usr/include/rgbkShifter/
sudo rm -rf /usr/include/$PROGNAME
sudo mkdir /usr/include/$PROGNAME
sudo cp Include/* /usr/include/$PROGNAME/