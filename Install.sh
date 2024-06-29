#!/bin/bash


bear -- g++ -pedantic -Wall -Wshadow `libpng-config --cflags --ldflags` main.cpp -o Built/rgbkShifter.bin
#sudo ln -f -s Built/RGBKShifter /usr/bin/rgbkShifter