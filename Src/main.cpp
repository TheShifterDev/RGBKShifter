
#include "../Include/rgbks.hh"
#define RGBKS_IMPLEM
#include "../Include/rgbks.hh"

#include "MakeLowerCase.cpp"

#include <cstring>
#include <iostream>
#include <string>

enum class CommandEnum {
	Shift,
	Atlas,
	Paletise,

	ENDOF
};

std::string LowerCaseify(std::string INP);
void SliceOutLastOfChar(std::string INP, char TARG, std::string &OutStart,
						std::string &OutEnd);

bool CommandList[(uint8_t)CommandEnum::ENDOF] = {};

std::string OutName = "rgbkshifted"; // used in -n/--name
std::string OutDir = ""; // used in -o/--out
std::string PaletFile = "";

// clang-format off
std::string HelpText =
	"Arguments					Description\n"
	"-s/--shift					= shifts fed images colours to the extremes\n"
	"-a/--atlas					= packs all fed files into a single output atlas .stimpac file\n"
	"-p/--paletise <PALETFILE>	= paletises	all input images with the palet file\n"
	"-o/--out <DIRECTORY>		= DIRECTORY arg is the location of the writen files\n"
	"-n/--name <NAME>			= NAME arg is the saved name of the packed file OR the prefix of all shifted files\n"
	"-h/--help					= print this help text";
// clang-format on

int main(int argc, char *argv[]) {
	// parse args
	std::vector<std::string> files;
	std::vector<RGBKS::Image> images;
	std::string hold;
	int t_i = 1;
	if(argc == 1) {
		printf("No args input.\nUse -h or --help for help.\n");
		exit(1);
	}
	while(t_i < argc) {
		hold = argv[t_i];
		// hold = MakeLowerCase::Lower(hold);
		if(hold[0] == '-') {
			if(hold == "-h" || hold == "--help") {
				// help request
				printf(HelpText.c_str());
				exit(0);
			} else if(hold == "-n" || hold == "--name") {
				// output prefix
				OutName = argv[t_i + 1];
				t_i += 2;
			} else if(hold == "-o" || hold == "--out") {
				// output location
				OutDir = argv[t_i + 1];
				t_i += 2;
			} else if(hold == "-p" || hold == "--paletise") {
				// palett to use
				PaletFile = argv[t_i + 1];
				CommandList[(uint8_t)CommandEnum::Paletise] = true;
				t_i += 2;
			} else if(hold == "-s" || hold == "--shift") {
				// shift the values of all input files
				CommandList[(uint8_t)CommandEnum::Shift] = true;
				t_i++;
			} else if(hold == "-a" || hold == "--atlas") {
				// take all input files and output a .stimpac
				CommandList[(uint8_t)CommandEnum::Atlas] = true;
				t_i++;
			} else {
				// invalid - option
				t_i++;
			}
		} else { // its a file
			files.push_back(hold);
			t_i++;
		}
	}

	// read all images
	std::string name = "";
	std::string exten = "";
	std::string printstring = "";
	RGBKS::Image image;

	for(uint32_t i = 0; i < files.size(); i++) {
		// seperate filename and file extension
		SliceOutLastOfChar(files[i], '.', name, exten);
		if(exten == "png") {
			image = RGBKS::Read_png(name);
			images.push_back(image);
		} else if(exten == "stimpac") {
			image = RGBKS::Read_imgpac(name);
			images.push_back(image);
		} else {
			printstring = files[i] + " has an invalid extension of ." + exten;
			printf(printstring.c_str());
		}
	}

	// test if shifting was requested
	if(CommandList[(uint8_t)CommandEnum::Shift]) {
		for(uint32_t i = 0; i < images.size(); i++) {
			RGBKS::ShiftNearist(images[i]);
		}
	}
	// test if atlas packing was requested
	if(CommandList[(uint8_t)CommandEnum::Atlas]) {
	}
	// test if paletiseing was requested
	if(CommandList[(uint8_t)CommandEnum::Paletise]) {
	}

	// write file
	std::string writedir = "";
	std::string writenam = "";
	if(CommandList[(uint8_t)CommandEnum::Atlas]) {
		// write as .stimpac
		for(uint32_t i = 0; i < images.size(); i++) {
			SliceOutLastOfChar(images[i].Glyphs[0].Name, '/', writedir,
							   writenam);
			if (OutName != "") {writenam = OutName;}
			RGBKS::Write_imgpac(images[i],OutDir + writenam);
		}
	} else {
		// write as .png
		for(uint32_t i = 0; i < images.size(); i++) {
			// seperate name from dir
			SliceOutLastOfChar(images[i].Glyphs[0].Name, '/', writedir,
							   writenam);
			// write file
			RGBKS::Write_png(images[i],OutDir + OutName+ writenam);
		}
	}

	return 0;
}

void SliceOutLastOfChar(std::string INP, char TARG, std::string &OutStart,
						std::string &OutEnd) {
	uint32_t extenpos = 0;
	std::string holdstr = INP;
	extenpos = holdstr.size() - 1;
	while(extenpos > 0) {
		if(holdstr[extenpos] == TARG) {
			break;
		} else {
			extenpos--;
		}
	}
	if(extenpos == 0) {
		OutStart = "";
		OutEnd = holdstr;
	} else {
		OutStart = holdstr.substr(0, extenpos);
		OutEnd = holdstr.substr(extenpos + 1, holdstr.size());
	}
}
std::string LowerCaseify(std::string INP){
	for(uint32_t i = 0; i < INP.length(); i++) {
		if(INP[i] > 64 && INP[i] < 91) {INP[i] += 32;}
	}
	return INP;
}