
#include "../Include/rgbks.hh"
#define RGBKS_IMPLEM
#include "../Include/rgbks.hh"

#include <cstring>
#include <iostream>
#include <string>

enum class CommandEnum {
	Shift,
	Atlas,
	Paletise,
	Format,

	ENDOF
};
enum class FileType{
	PNG,
	STIMPAC,

	ENDOF
};

std::string LowerCaseify(std::string INP);
void SliceOutLastOfChar(std::string INP, char TARG, std::string &OutStart,
						std::string &OutEnd);

bool CommandList[(uint8_t)CommandEnum::ENDOF] = {};
FileType OutFileType;
std::string OutName = "rgbkshifted"; // used in -n/--name
std::string OutDir = "";			 // used in -o/--out
std::string PaletFile = "";
// clang-format off
std::vector<RGBKS::Colour> PalletColours{
	// shifter pallet
	{255, 255, 255, 255}, // white
	{0, 0, 0, 255},		  // black
	{0, 255, 255, 255},	  // cyan
	{255, 0, 255, 255},	  // purple
	{255, 255, 0, 255},	  // orange
	{255, 0, 0, 255},	  // red
	{0, 255, 0, 255},	  // green
	{0, 0, 255, 255},	  // blue
};
// clang-format on

// clang-format off
std::string HelpText =
	"Arguments					Description\n"
	"-s/--shift					= shifts fed images colours to the extremes !!!disables paletise!!!\n"
	"-p/--paletise <PALETFILE>	= paletises	all input images with the palet file !!!disables shift!!!\n"
	"-a/--atlas					= packs all fed files into a single output atlas .stimpac file\n"
	"-o/--out <DIRECTORY>		= DIRECTORY arg is the location of the writen file/s\n"
	"-f/--format <FORMAT>		= FORMAT arg is the file type of the output file/s \"png\" or \"stimpac\"\n"
	"-n/--name <NAME>			= NAME arg is the saved name of the packed file OR the prefix of all shifted files\n"
	"-h/--help					= print this help text\n"
	"Notes\n"
	"1) arguments must be seperated, (-s -a) is correct, (-sa) will be ignored as an invalid option";
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
				OutDir += '/';
				t_i += 2;
			} else if(hold == "-p" || hold == "--paletise") {
				// palett to use
				PaletFile = argv[t_i + 1];
				CommandList[(uint8_t)CommandEnum::Shift] = false;
				CommandList[(uint8_t)CommandEnum::Paletise] = true;
				t_i += 2;
			} else if(hold == "-s" || hold == "--shift") {
				// shift the values of all input files
				CommandList[(uint8_t)CommandEnum::Paletise] = false;
				CommandList[(uint8_t)CommandEnum::Shift] = true;
				t_i++;
			} else if(hold == "-a" || hold == "--atlas") {
				// take all input files and output a .stimpac
				CommandList[(uint8_t)CommandEnum::Atlas] = true;
				t_i++;
			} else if(hold == "-f" || hold == "--format") {
				CommandList[(uint8_t)CommandEnum::Format] = true;
				std::string TempString = argv[t_i + 1];
				if(TempString == "png")		{OutFileType = FileType::PNG;} else
				if(TempString == "stimpac")	{OutFileType = FileType::STIMPAC;}
				t_i+=2;
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
	std::string PrintString = "";
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
			PrintString = files[i] + " has an invalid extension of ." + exten;
			printf(PrintString.c_str());
			exit(1);
		}
	}

	// test if shifting or paletising was requested
	if(CommandList[(uint8_t)CommandEnum::Shift] ||
	   CommandList[(uint8_t)CommandEnum::Paletise]) {
		// test if paletiseing was requested
		if(CommandList[(uint8_t)CommandEnum::Paletise]) {
			SliceOutLastOfChar(PaletFile, '.', name, exten);
			if(exten == "png") {
				RGBKS::Image t_image = RGBKS::Read_png(name);
				PalletColours = RGBKS::ExtractPallet_Image(t_image);
			} else {
				PrintString =
					PaletFile + " has an invalid extension of ." + exten;
				printf(PrintString.c_str());
				exit(1);
			}
		}
		for(uint32_t i = 0; i < images.size(); i++) {
			RGBKS::PalletiseImage(images[i], PalletColours);
		}
	}

	// test if atlas packing was requested
	if(CommandList[(uint8_t)CommandEnum::Atlas]) {
	}

	// write file
	std::string writedir = "";
	std::string writenam = "";
	if(CommandList[(uint8_t)CommandEnum::Atlas]) {
		// write as .stimpac
		for(uint32_t i = 0; i < images.size(); i++) {
			SliceOutLastOfChar(images[i].Glyphs[0].Name, '/', writedir,
							   writenam);
			if(OutName != "") {
				writenam = OutName;
			}
		}
		RGBKS::Image t_outima = RGBKS::MergeImages(images);
		if (CommandList[(uint8_t)CommandEnum::Format]){
			if (OutFileType == FileType::PNG) {
				RGBKS::Write_png(t_outima, OutDir + writenam);
			}else{
				RGBKS::Write_imgpac(t_outima, OutDir + writenam);
			}
		}else{
			//RGBKS::Write_imgpac(t_outima, OutDir + writenam);
			RGBKS::Write_png(t_outima, OutDir + writenam);
		}
	} else {
		// write as .png
		for(uint32_t i = 0; i < images.size(); i++) {
			// seperate name from dir
			SliceOutLastOfChar(images[i].Glyphs[0].Name, '/', writedir,
							   writenam);
			// write file
			if (CommandList[(uint8_t)CommandEnum::Format]){
				if (OutFileType == FileType::PNG) {
					RGBKS::Write_png(images[i], OutDir +OutName + writenam);
				}else{
					RGBKS::Write_imgpac(images[i], OutDir +OutName + writenam);
				}
			}else{
				RGBKS::Write_png(images[i], OutDir +OutName + writenam);
			}
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
std::string LowerCaseify(std::string INP) {
	for(uint32_t i = 0; i < INP.length(); i++) {
		if(INP[i] > 64 && INP[i] < 91) {
			INP[i] += 32;
		}
	}
	return INP;
}