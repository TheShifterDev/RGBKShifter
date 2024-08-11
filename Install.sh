#!/bin/bash

PROGNAME="rgbkshifter"
DEBGFLAG=" -O0 -ggdb"
COMPFLAG=" -pedantic -Wall -Wshadow"
LIBPNG=" `libpng-config --cflags --ldflags`"
#TODO: replace ^ with the output of that incase libpngconfig does not exist

# IMPORTANT: only have 1 enable at a time or compiler will complain
<<<<<<< ours
U_PNGPP=true
=======
U_PNGPP=true 
# NOTE: you may need to add links to path in your bashrc
# "export PATH=$PATH:/usr/include/png++:/usr/include/libpng"
>>>>>>> theirs

# compiles and puts a symlink to prog in /usr/bin/
COMMAND="$DEBGFLAG $COMPFLAG"
######################
if $U_PNGPP; then # requires png++ (c++ wrapper for libpng)
COMMAND="$COMMAND $LIBPNG -DUSING_PNGPP"
fi
######################
COMMAND=$COMMAND" Src/main.cpp -o Built/$PROGNAME.bin"

g++ $COMMAND
sudo ln -f ./Built/$PROGNAME.bin /usr/bin/$PROGNAME

<<<<<<< ours
#copys rgbkShifters includes into /usr/include/StomaImagePack/
sudo rm -rf /usr/include/StomaImagePack
sudo mkdir /usr/include/StomaImagePack
sudo cp Include/* /usr/include/StomaImagePack/
=======
#copys rgbkShifters includes into /usr/include/rgbkShifter/
sudo rm -rf /usr/include/$PROGNAME
sudo mkdir /usr/include/$PROGNAME
sudo cp Include/* /usr/include/$PROGNAME/
>>>>>>> theirs
