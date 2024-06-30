#!/bin/bash

PROGNAME="rgbkshifter"
# compiles and puts a symlink to prog in /usr/bin/
bear -- g++ -pedantic -Wall -Wshadow `libpng-config --cflags --ldflags` Src/main.cpp -o Built/$PROGNAME.bin
sudo ln -f -s Built/$PROGNAME.bin /usr/bin/$PROGNAME

#copys rgbkShifters includes into /usr/include/rgbkShifter/
sudo rm -rf /usr/include/$PROGNAME
sudo mkdir /usr/include/$PROGNAME
sudo cp Include/* /usr/include/$PROGNAME/