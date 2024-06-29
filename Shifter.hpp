#include <string>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <string>
#include <cstdlib>
#include <png++/png.hpp> // local install via pacman
#include "MakeLowerCase.cpp"

namespace Shifter{
	int LoadImageAndShift(int shiftType, char* InFileDir, char* OutFileDir);
	int RGBKShift(char* InFileDir, char* OutFileDir);
}
