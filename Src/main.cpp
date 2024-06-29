#define RGBKS_IMAGE_IMPLEM
#include "../Include/Image.hh"

#include "Shifter.cpp"
// #include "MakeLowerCase.cpp"

#include <cstring>
#include <iostream>
#include <string>
#define HELPTEXT "\
		<files> <args>\
		Arguments		Description\n\
		-p/--pack		= packs all fed files into a single output file\n\
		-s/--shift		= shifts fed images colours to the extremes\n\
		-o/--out		= next arg is the saved name of the packed file OR the prefix of all shifted files\n\
		"

std::string OutPrefix = "";//used in -o/--out 

int main(int argc, char *argv[]) {
	std::string HelpText = HELPTEXT;
	bool VALID_RESPONCE_RETURNED = false;
	std::string Returned = "";
	std::string FILENAME = "";
	std::string FileToShift = "";
	std::string FileToOut = "";

	std::cout << "input name of image to shift\n";
	std::cin >> FILENAME;
	FileToShift = "./input/" + FILENAME;
	FileToOut = "./output/" + FILENAME;

	char *InCharString = new char[FileToShift.size() + 1];
	std::copy(FileToShift.begin(), FileToShift.end(), InCharString);
	InCharString[FileToShift.size()] = '\0';

	char *OutCharString = new char[FileToOut.size() + 1];
	std::copy(FileToOut.begin(), FileToOut.end(), OutCharString);
	OutCharString[FileToOut.size()] = '\0';

	while(!VALID_RESPONCE_RETURNED) {
		std::cout << "input A for rgb,\n input B for stripes,\n input C for "
					 "Perlin_Noise,\n Q to quit\n";
		std::cin >> Returned;
		std::cout << "\n";
		Returned = MakeLowerCase::Lower(Returned);

		if(Returned == "q") {
			std::cout << "quiting programme\n";
			VALID_RESPONCE_RETURNED = true;
			return 0;
		} else if(Returned == "a") {
			std::cout << "selected option A\n";
			VALID_RESPONCE_RETURNED = true;
			//Shifter::LoadImageAndShift(0, InCharString, OutCharString);
		} else if(Returned == "b") {
			std::cout << "selected option B\n";
			VALID_RESPONCE_RETURNED = true;
			//Shifter::LoadImageAndShift(1, InCharString, OutCharString);
		} else if(Returned == "c") {
			std::cout << "selected option C\n";
			VALID_RESPONCE_RETURNED = true;
			//Shifter::LoadImageAndShift(2, InCharString, OutCharString);
		} else {
			std::cout << "no valid responce sent\n";
		}
		// 0 = RGBK
		// 1 = stripes
	}
	return 0;
}
