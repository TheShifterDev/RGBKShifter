
#include "../Include/StomaImagePack.hh"
#define STOMAIMAGEPACK_IMPLEM
#include "../Include/StomaImagePack.hh"

#include <string>

enum class CommandEnum {
	Shift,
	Atlas,
	Palet,
	Cutup,
	Format,

	ENDOF
};
enum class FileType {
	PNG,
	STIMPAC,

	ENDOF
};

void WriteOut(StomaImagePack::Image IMG, std::string NAM);


bool CommandList[(uint8_t)CommandEnum::ENDOF] = {};
FileType OutFileType;
std::string OutName = "rgbkshifted"; // used in -n/--name
std::string OutDir = "";			 // used in -o/--out
std::string PaletFile = "";
// clang-format off
std::vector<StomaImagePack::Colour> PalletColours{
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
	"-a/--atlas					= packs all fed files into a single output file !!!disables cutup!!!\n"
	"-c/--cutup					= outputs any input file/s glyphs as images !!!disables atlas!!!\n"
	"-o/--out <DIRECTORY>		= DIRECTORY arg is the location of the writen file/s\n"
	"-f/--format <FORMAT>		= FORMAT arg is the file type of the output file/s \"png\" or \"stimpac\"\n"
	"-n/--name <NAME>			= NAME arg is the saved name of the packed file OR the prefix of all shifted files\n"
	"-h/--help					= print this help text\n"
	"Notes\n"
	"1) arguments must be seperated, (-s -a) is correct, (-sa) will be ignored as an invalid option";
// clang-format on

int main(int argc, char *argv[]) {
	// parse args
	std::vector<std::string> FileList;
	std::vector<StomaImagePack::Image> ImageList;
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
				CommandList[(uint8_t)CommandEnum::Palet] = true;
				t_i += 2;
			} else if(hold == "-s" || hold == "--shift") {
				// shift the values of all input files
				CommandList[(uint8_t)CommandEnum::Palet] = false;
				CommandList[(uint8_t)CommandEnum::Shift] = true;
				t_i++;
			} else if(hold == "-a" || hold == "--atlas") {
				CommandList[(uint8_t)CommandEnum::Cutup] = false;
				CommandList[(uint8_t)CommandEnum::Atlas] = true;
				t_i++;
			} else if(hold == "-c" || hold == "--cutup") {
				CommandList[(uint8_t)CommandEnum::Cutup] = true;
				CommandList[(uint8_t)CommandEnum::Atlas] = false;
				t_i++;
			} else if(hold == "-f" || hold == "--format") {
				CommandList[(uint8_t)CommandEnum::Format] = true;
				std::string TempString = argv[t_i + 1];
				t_i += 2;
				if(TempString == "png") {
					OutFileType = FileType::PNG;
				} else if(TempString == "stimpac") {
					OutFileType = FileType::STIMPAC;
				}
			} else {
				// invalid - option
				t_i++;
			}
		} else { // its a file
			FileList.push_back(hold);
			t_i++;
		}
	}

	// read all images
	{
		std::string ImageName = "";
		std::string ImageExtension = "";
		std::string ReadLocation = "";
		StomaImagePack::Image TempImage;
		for(uint32_t i = 0; i < FileList.size(); i++) {
			// split "./gorbinos/file.png" to "./gorbinos/file" & "png"
			StomaImagePack::SliceOutLastOfChar(FileList[i], '.', ImageName, ImageExtension);
			if(ImageExtension == "png") {
				TempImage = StomaImagePack::Read_png(ImageName);
				ImageList.push_back(TempImage);
			} else if(ImageExtension == "stimpac") {
				TempImage = StomaImagePack::Read_stimpac(ImageName);
				ImageList.push_back(TempImage);
			}
		}
	}

	// test if shifting or paletising was requested
	if(CommandList[(uint8_t)CommandEnum::Shift] ||
	   CommandList[(uint8_t)CommandEnum::Palet]) {
		// test if paletiseing was requested
		if(CommandList[(uint8_t)CommandEnum::Palet]) {
			std::string PalletName;
			std::string PalletExtension;
			StomaImagePack::Image PalletImage;
			StomaImagePack::SliceOutLastOfChar(PaletFile, '.', PalletName, PalletExtension);
			if(PalletExtension == "png") {
				PalletImage = StomaImagePack::Read_png(PalletName);
				PalletColours = StomaImagePack::ExtractPallet_Image(PalletImage);
			}
		}
		for(uint32_t i = 0; i < ImageList.size(); i++) {
			StomaImagePack::PalletiseImage(ImageList[i], PalletColours);
		}
	}

	// write file
	if(CommandList[(uint8_t)CommandEnum::Atlas]) {
		// output single atlas file
		if(!CommandList[(uint8_t)CommandEnum::Format]) {
			OutFileType = FileType::STIMPAC;
		}
		StomaImagePack::Image AtlasImage = StomaImagePack::MergeImages(ImageList);
		WriteOut(AtlasImage, OutDir + OutName);
	} else
	if(CommandList[(uint8_t)CommandEnum::Cutup]) {
		// output every glyph as unique files
		if(!CommandList[(uint8_t)CommandEnum::Format]) {
			OutFileType = FileType::PNG;
		}
		std::vector<StomaImagePack::Image> GlyphImageList =
			StomaImagePack::SeperateGlyphs(ImageList);
		for(uint32_t i = 0; i < GlyphImageList.size(); i++) {
			WriteOut(GlyphImageList[i],
					 OutDir + OutName + GlyphImageList[i].Glyphs[0].Name);
		}
	} else {
		// output all input files after processing
		if(!CommandList[(uint8_t)CommandEnum::Format]) {
			OutFileType = FileType::PNG;
		}
		for(uint32_t i = 0; i < ImageList.size(); i++) {
			WriteOut(ImageList[i],
					 OutDir + OutName + ImageList[i].Glyphs[0].Name);
		}
	}

	return 0;
}

void WriteOut(StomaImagePack::Image IMG, std::string NAM) {
	if(OutFileType == FileType::PNG) {
		StomaImagePack::Write_png(IMG, NAM);
	} else {
		StomaImagePack::Write_stimpac(IMG, NAM);
	}
}
