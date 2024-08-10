#!/bin/bash

PREMADE=false # skips compiling and use premade version (Not Recommended)

if $PREMADE; then 
# PREMADE = false
# links premade bin to usr/bin
sudo ln -f ./Premade/$PROGNAME.bin /usr/bin/$PROGNAME

else 
# PREMADE = false
# IMPORTANT: only have 1 enable at a time or compiler will complain
U_PNGPP=true

PROGNAME="rgbkshifter"
DEBGFLAG=" -O0 -ggdb"
COMPFLAG=" -pedantic -Wall -Wshadow"
LIBPNG=" `libpng-config --cflags --ldflags`"


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

fi # endof PREMADE = false

#copys rgbkShifters includes into /usr/include/rgbkShifter/
sudo rm -rf /usr/include/$PROGNAME
sudo mkdir /usr/include/$PROGNAME
sudo cp Include/* /usr/include/$PROGNAME/