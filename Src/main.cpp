#define RGBKS_IMPLEM
#include "../Include/rgbks.hh"

#include "MakeLowerCase.cpp"

#include <cstring>
#include <iostream>
#include <string>

enum class CommandEnum{
	Shift,
	Atlas,
	Paletise,

	ENDOF
};

bool CommandList[(uint8_t)CommandEnum::ENDOF];

std::string OutPrefix = "rgbkshifted_";//used in -o/--out
std::string PaletFile = "";
std::string HelpText = 
		"<files> <args>"
		"Arguments					Description\n"
		"-s/--shift					= shifts fed images colours to the extremes\n"
		"-a/--atlas					= packs all fed files into a single output atlas .stimpac file\n"
		"-p/--paletise <PALETFILE>	= paletises	all input images with the palet file\n"
		"-o/--out <NAME>				= NAME arg is the saved name of the packed file OR the prefix of all shifted files\n"
		"-h/--help					= print this help text";

int main(int argc, char *argv[]) {
	// parse args
	std::vector<std::string> files;
	std::vector<RGBKS::Image> images;
	std::string hold;
	int t_i = 1;
	if (argc == 1){
		printf("No args input.\nUse -h or --help for help.\n");
		exit(1);
	}
	while(t_i<argc){
		hold = argv[t_i];
		hold = MakeLowerCase::Lower(hold);
		if(hold[0] == '-'){
			if (hold == "-h" || hold == "--help") {
				// help request
				printf(HelpText.c_str());
				exit(0);			
			}else if(hold == "-o" || hold == "--out"){
				// output name
				OutPrefix = argv[t_i+1];
				t_i+=2;
			}else if(hold == "-p" || hold == "--paletise"){
				// output name
				PaletFile = argv[t_i+1];
				CommandList[(uint8_t)CommandEnum::Paletise] = true;
				t_i+=2;				
			}else if(hold == "-s" || hold == "--shift"){
				// shift the values of all input files
				CommandList[(uint8_t)CommandEnum::Shift] = true;
				t_i++;
			}else if(hold == "-a" || hold == "--atlas"){
				// take all input files and output a .stimpac
				CommandList[(uint8_t)CommandEnum::Atlas] = true;
				t_i++;
			}else{
				// invalid - option
				t_i++;
			}
		}else{ // its a file
			files.push_back(hold);
			t_i++;
		}
	}

	// read all images
	std::string name;
	std::string exten;
	uint32_t extenpos;
	RGBKS::Image image;
	std::string printstring;
	for (uint32_t i = 0;i < files.size();i++){
		// seperate filename and file extension
		extenpos = files[i].size()-1;
		while (extenpos > 0){
			if (files[i][extenpos] == '.'){
				break;
			}else{extenpos--;}
		}
		if (extenpos == 0) {
			printstring = files[i]+" has no extension";
			printf(printstring.c_str());
		}else{
			name = files[i].substr(0,extenpos);
			exten = files[i].substr(extenpos,files[i].size());
			if (exten == ".png") {
				image = RGBKS::Read_png(name);
				images.push_back(image);
			}else if (exten == ".stimpac"){
				image = RGBKS::Read_imgpac(name);
				images.push_back(image);
			}else{
				printstring = files[i]+" has an invalid extension of "+exten;
				printf(printstring.c_str());
			}
		}
	}

	// test if shifting was requested
	if (CommandList[(uint8_t)CommandEnum::Shift] == 0) {
		for (uint32_t i = 0;i < images.size();i++){
			RGBKS::ShiftNearist(images[i]);
		}
	}
	// test if atlas packing was requested
	if (CommandList[(uint8_t)CommandEnum::Atlas] == 0) {
	
	}
	// test if paletiseing was requested
	if (CommandList[(uint8_t)CommandEnum::Paletise] == 0) {
	
	}

	return 0;
}
