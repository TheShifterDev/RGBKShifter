#!/bin/bash

# compiles and puts a symlink to prog in /usr/bin/
bear -- g++ -pedantic -Wall -Wshadow `libpng-config --cflags --ldflags` Src/main.cpp -o Built/rgbkShifter.bin
#sudo ln -f -s Built/RGBKShifter /usr/bin/rgbkShifter

#copys rgbkShifters includes into /usr/include/rgbkShifter/
#sudo mkdir /usr/include/rgbkShifter
#sudo cp Include/* /usr/include/rgbkShifter/