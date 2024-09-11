
#include <StomaImagePack/StomaImagePack.hh>
#define STOMAIMAGEPACK_IMPLEM
#include <StomaImagePack/StomaImagePack.hh>

#include <png++/png.hpp> // local install via pacman

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

void Write_png(StomaImagePack::Image IMG, std::string NAM);
StomaImagePack::Image Read_png(std::string NAM);
void SliceOutLastOfChar(std::string INP,
						char TARG,
						std::string &OutStart,
						std::string &OutEnd);
std::string LowerCaseify(std::string INP);
void WriteOut(StomaImagePack::Image IMG, std::string NAM);
// NOTE: NEVER USE MACROS LIKE
// >>> #define GETCORD(X, Y, XMAX) (X + (Y * XMAX)) <<<
// AS NOT INCLUDING '('&')' IN
// "GETCORD((MainXPos + SubXPos), (MainYPos + SubYPos),MainImage.Size.Width)"
// CAUSED A BUG THAT TOOK DAYS TO TRACK
inline uint32_t GetCoordinate(uint32_t XPOS, uint32_t YPOS, uint32_t MAXX);

std::vector<StomaImagePack::Image> SeperateGlyphs(std::vector<StomaImagePack::Image> IMG);
std::vector<StomaImagePack::Image> ReorderByVolume(std::vector<StomaImagePack::Image> IMG);
float GetRGBColourDistance(StomaImagePack::Colour A, StomaImagePack::Colour B);
void PalletiseImage(StomaImagePack::Image &IMG, std::vector<StomaImagePack::Colour> PAL);
StomaImagePack::Image MergeImages(std::vector<StomaImagePack::Image> IMG);
std::vector<StomaImagePack::Colour> ExtractPallet_Image(StomaImagePack::Image IMG);


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
				printf("%s\n",HelpText.c_str());
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
			SliceOutLastOfChar(FileList[i], '.', ImageName, ImageExtension);
			if(ImageExtension == "png") {
				TempImage = Read_png(ImageName);
				ImageList.push_back(TempImage);
			} else if(ImageExtension == "stimpac") {
				TempImage = StomaImagePack::ReadStimpac(ImageName);
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
			SliceOutLastOfChar(PaletFile, '.', PalletName, PalletExtension);
			if(PalletExtension == "png") {
				PalletImage = Read_png(PalletName);
				PalletColours = ExtractPallet_Image(PalletImage);
			}
		}
		for(uint32_t i = 0; i < ImageList.size(); i++) {
			PalletiseImage(ImageList[i], PalletColours);
		}
	}

	// write file
	if(CommandList[(uint8_t)CommandEnum::Atlas]) {
		// output single atlas file
		if(!CommandList[(uint8_t)CommandEnum::Format]) {
			OutFileType = FileType::STIMPAC;
		}
		StomaImagePack::Image AtlasImage = MergeImages(ImageList);
		WriteOut(AtlasImage, OutDir + OutName);
	} else
	if(CommandList[(uint8_t)CommandEnum::Cutup]) {
		// output every glyph as unique files
		if(!CommandList[(uint8_t)CommandEnum::Format]) {
			OutFileType = FileType::PNG;
		}
		std::vector<StomaImagePack::Image> GlyphImageList =
			SeperateGlyphs(ImageList);
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
		Write_png(IMG, NAM);
	} else {
		StomaImagePack::WriteStimpac(IMG, NAM);
	}
}

StomaImagePack::Image Read_png(std::string NAM) {
	// assumes .png has been trimmed off
	uint32_t HoldPosition;
	StomaImagePack::Image ReturnImage;
	png::rgba_pixel HoldPixel;
	std::string SourceDir;
	std::string GlyphName;
	png::image<png::rgba_pixel> PngFile(NAM + ".png");
	ReturnImage.Size.Width = PngFile.get_width();
	ReturnImage.Size.Height = PngFile.get_height();
	// pngs only have 1 glyph
	ReturnImage.Glyphs.resize(1);
	SliceOutLastOfChar(NAM, '/',SourceDir,GlyphName);
	ReturnImage.Glyphs[0].Name = GlyphName;
	ReturnImage.Glyphs[0].Size = ReturnImage.Size;
	// handle pixel array
	ReturnImage.Pixels.resize(ReturnImage.Size.Width * ReturnImage.Size.Height);
	for(uint32_t h = 0; h < ReturnImage.Size.Height; h++) {
		for(uint32_t w = 0; w < ReturnImage.Size.Width; w++) {
			HoldPosition = GetCoordinate(w, h, ReturnImage.Size.Width);
			HoldPixel = PngFile.get_pixel(w, h);
			ReturnImage.Pixels[HoldPosition].R = HoldPixel.red;
			ReturnImage.Pixels[HoldPosition].G = HoldPixel.green;
			ReturnImage.Pixels[HoldPosition].B = HoldPixel.blue;
			ReturnImage.Pixels[HoldPosition].A = HoldPixel.alpha;
		}
	}
	return ReturnImage;
}
void Write_png(StomaImagePack::Image IMG, std::string NAM) {
	png::image<png::rgba_pixel> WriteFile;
	WriteFile.resize(IMG.Size.Width, IMG.Size.Height);
	png::rgba_pixel HoldPixel;
	uint32_t CurrentCoordinate;
	for(uint32_t h = 0; h < IMG.Size.Height; h++) {
		for(uint32_t w = 0; w < IMG.Size.Width; w++) {
			CurrentCoordinate = GetCoordinate(w, h, IMG.Size.Width);
			HoldPixel.red = IMG.Pixels[CurrentCoordinate].R;
			HoldPixel.green = IMG.Pixels[CurrentCoordinate].G;
			HoldPixel.blue = IMG.Pixels[CurrentCoordinate].B;
			HoldPixel.alpha = IMG.Pixels[CurrentCoordinate].A;
			WriteFile.set_pixel(w, h, HoldPixel);
		}
	}
	WriteFile.write(NAM + ".png");
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
	for(uint32_t i = 0; i < INP.size(); i++) {
		if(INP[i] > 64 && INP[i] < 91) {
			INP[i] += 32;
		}
	}
	return INP;
}

std::vector<StomaImagePack::Image> SeperateGlyphs(std::vector<StomaImagePack::Image> IMG) {
	std::vector<StomaImagePack::Image> OutputImages;
	StomaImagePack::Image HoldingImage;
	StomaImagePack::Resolution NewSize;
	StomaImagePack::Resolution NewOffset;
	std::string NewName;
	for(uint32_t i = 0; i < IMG.size(); i++) {
		if(IMG[i].Glyphs.size() < 2) {
			if(IMG[i].Glyphs.size() == 0){
				// IMG[i] is broken
				exit(8);
			}
			// skip as IMG[i] seperation is unnessesary
			OutputImages.push_back(IMG[i]);
		} else {
			for(uint32_t q = 0; q < IMG[i].Glyphs.size(); q++) {
				NewSize = IMG[i].Glyphs[q].Size;
				NewOffset = IMG[i].Glyphs[q].Offset;
				NewName =  IMG[i].Glyphs[q].Name;
				HoldingImage.Size = NewSize;
				HoldingImage.Glyphs.resize(1);
				HoldingImage.Glyphs[0].Name = NewName;
				HoldingImage.Glyphs[0].Offset.Width = 0;
				HoldingImage.Glyphs[0].Offset.Height = 0;
				HoldingImage.Glyphs[0].Size = NewSize;
				HoldingImage.Pixels.resize(NewSize.Width * NewSize.Height);
				for(uint32_t h = 0; h < NewSize.Height; h++) {
				for(uint32_t w = 0; w < NewSize.Width; w++) {
					HoldingImage.Pixels[GetCoordinate(w, h, NewSize.Width)] =
						IMG[i].Pixels[GetCoordinate(NewOffset.Width + w,NewOffset.Height + h,IMG[i].Size.Width)];
				}}
				OutputImages.push_back(HoldingImage);
			}
		}
	}

	return OutputImages;
}
StomaImagePack::Image MergeImages(std::vector<StomaImagePack::Image> SUBIMAGES) {
	// clang-format off
	// ---------------------SET_SIZE------------------------------------//
	StomaImagePack::Image OutputImage;
	std::vector<StomaImagePack::Image> OrderedSubImages;
	OrderedSubImages = ReorderByVolume(SUBIMAGES);
	std::vector<StomaImagePack::Resolution> ImageResolutions(SUBIMAGES.size());
	bool DrawSafe;
	bool DrawDone;
	uint32_t MPoint;
	uint32_t SPoint;
	uint32_t CheckLimitX;
	uint32_t CheckLimitY;
	uint32_t CurrentShape = 0;
	// get total required volume
	StomaImagePack::Resolution ExpectedSize;
	ExpectedSize.Width = 0;
	ExpectedSize.Height = 0;
	StomaImagePack::Resolution MaxSize;
	MaxSize.Width = 0;
	MaxSize.Height = 0;

	uint32_t CurrentGlyphs = 0;
	uint32_t MaxGlyphs = 0;

	for(uint32_t i = 0; i < OrderedSubImages.size(); i++) {
		ImageResolutions[i] = OrderedSubImages[i].Size;
		MaxSize.Width  += OrderedSubImages[i].Size.Width;
		MaxSize.Height += OrderedSubImages[i].Size.Height;
		MaxGlyphs += OrderedSubImages[i].Glyphs.size();
	}
	OutputImage.Glyphs.resize(MaxGlyphs);
	// get base total size for new image
	for (uint32_t i = 0;i<OrderedSubImages.size();i++){
		// width
		ExpectedSize.Width += OrderedSubImages[i].Size.Width;
		if (ExpectedSize.Width>=(MaxSize.Width - ExpectedSize.Width)){break;}
	}
	for (uint32_t i = 0;i<OrderedSubImages.size();i++){
		// height
		ExpectedSize.Height += OrderedSubImages[i].Size.Height;
		if (ExpectedSize.Height>=(MaxSize.Height - ExpectedSize.Height)){break;}		
	}
	
	OutputImage.Size = ExpectedSize;
	uint32_t hold = ExpectedSize.Width*ExpectedSize.Height;
	OutputImage.Pixels.resize(hold);
	std::vector<bool> OccupiedPositions(hold);
	// ---------------------------DRAWING--------------------------------//
	
	// place shapes
	for(CurrentShape = 0; CurrentShape < OrderedSubImages.size(); CurrentShape++) {
		DrawDone = false;
		CheckLimitY = (OutputImage.Size.Height-(OrderedSubImages[CurrentShape].Size.Height-1));
		CheckLimitX = (OutputImage.Size.Width -(OrderedSubImages[CurrentShape].Size.Width -1));
		for(uint32_t MYPos = 0; MYPos < CheckLimitY; MYPos++) {
		for(uint32_t MXPos = 0; MXPos < CheckLimitX; MXPos++) {
			if (OccupiedPositions[GetCoordinate(MXPos, MYPos, OutputImage.Size.Width)] == false) {
				DrawSafe = true;
				// check if position collides with previously written positions
				for(uint32_t SYPos=0;SYPos<OrderedSubImages[CurrentShape].Size.Height;SYPos++){
				for(uint32_t SXPos=0;SXPos<OrderedSubImages[CurrentShape].Size.Width ;SXPos++){
					if(OccupiedPositions[GetCoordinate(MXPos + SXPos,MYPos + SYPos,OutputImage.Size.Width)] == true){
						DrawSafe = false;
						goto AlreadyWrittenBreakout;
					}
				}}
				AlreadyWrittenBreakout:;
				if (DrawSafe){
					// draw
					for(uint32_t SYPos=0;SYPos<OrderedSubImages[CurrentShape].Size.Height;SYPos++){
					for(uint32_t SXPos=0;SXPos<OrderedSubImages[CurrentShape].Size.Width ;SXPos++){
						MPoint = GetCoordinate(MXPos + SXPos,MYPos + SYPos,OutputImage.Size.Width);
						SPoint = GetCoordinate(SXPos, SYPos, OrderedSubImages[CurrentShape].Size.Width);
						OutputImage.Pixels[MPoint] = OrderedSubImages[CurrentShape].Pixels[SPoint];
						OccupiedPositions[MPoint] = true;
					}}
					// place glyphs
					uint32_t TargetGlyphID = OrderedSubImages[CurrentShape].Glyphs.size();
					for(uint32_t CurrentGlyphID = 0;CurrentGlyphID < TargetGlyphID;CurrentGlyphID++){
						OutputImage.Glyphs[CurrentGlyphs+CurrentGlyphID].Name 			= OrderedSubImages[CurrentShape].Glyphs[CurrentGlyphID].Name;
						OutputImage.Glyphs[CurrentGlyphs+CurrentGlyphID].Offset.Width	= OrderedSubImages[CurrentShape].Glyphs[CurrentGlyphID].Offset.Width +MXPos;
						OutputImage.Glyphs[CurrentGlyphs+CurrentGlyphID].Offset.Height	= OrderedSubImages[CurrentShape].Glyphs[CurrentGlyphID].Offset.Height+MYPos;
						OutputImage.Glyphs[CurrentGlyphs+CurrentGlyphID].Size 			= OrderedSubImages[CurrentShape].Glyphs[CurrentGlyphID].Size;
					}
					CurrentGlyphs += TargetGlyphID;
					DrawDone = true;
					goto DrawDoneBreakout;
				}
			}// end of occupied position[offsetw,offseth] check
		}}
		DrawDoneBreakout:;
		if(DrawDone == false){
			exit(5);
		}
	}
	// ---------------------------TRIM--------------------------------//
	// Trim unused lines or collumns
	ExpectedSize = OutputImage.Size;
	// get true used Heigth/Rows
	while (ExpectedSize.Height > 1) {
		for(uint32_t w=0;w<ExpectedSize.Width;w++){
			if (OccupiedPositions[GetCoordinate(w, ExpectedSize.Height-1, OutputImage.Size.Width)]){
				goto PixelRowOccupiedBreakout;
			}
		}
		ExpectedSize.Height--;
	}
	PixelRowOccupiedBreakout:;
	// get true used Width/Columns
	while (ExpectedSize.Width > 1) {
		for(uint32_t h=0;h<ExpectedSize.Height;h++){
			if (OccupiedPositions[GetCoordinate(ExpectedSize.Width-1 , h, OutputImage.Size.Width)]){
				goto PixelColumnOccupiedBreakout;
			}
		}
		ExpectedSize.Width--;
	}
	PixelColumnOccupiedBreakout:;
	// check if there are rows or columns to trim
	if(ExpectedSize.Width  != OutputImage.Size.Width 
	|| ExpectedSize.Height != OutputImage.Size.Height){
		StomaImagePack::Image CopyImage = OutputImage;
		OutputImage.Size = ExpectedSize;
		OutputImage.Pixels.resize(ExpectedSize.Width*ExpectedSize.Height);
		for(uint32_t MYPos = 0; MYPos < CheckLimitY; MYPos++) {
		for(uint32_t MXPos = 0; MXPos < CheckLimitX; MXPos++) {
			MPoint = GetCoordinate(MXPos,MYPos,OutputImage.Size.Width);
			SPoint = GetCoordinate(MXPos,MYPos,CopyImage.Size.Width);
			OutputImage.Pixels[MPoint] = CopyImage.Pixels[SPoint];
		}}
	}
	return OutputImage;
	// clang-format on
}
float GetRGBColourDistance(StomaImagePack::Colour A, StomaImagePack::Colour B) {
	return (((((float)A.R - (float)B.R) * ((float)A.R - (float)B.R))+
			 (((float)A.G - (float)B.G) * ((float)A.G - (float)B.G))+
			 (((float)A.B - (float)B.B) * ((float)A.B - (float)B.B))) / 2);
}
std::vector<StomaImagePack::Colour> ExtractPallet_Image(StomaImagePack::Image IMG) {
	// brute force iterates over fed image
	uint32_t t_len = IMG.Size.Width * IMG.Size.Height;
	std::vector<StomaImagePack::Colour> t_ret;
	bool t_add;
	t_ret.push_back(IMG.Pixels[0]);
	for(uint32_t i = 1; i < t_len; i++) {
		t_add = true;
		for(uint32_t q = 0; q < t_ret.size(); q++) {
			if(t_ret[q].R == IMG.Pixels[i].R && t_ret[q].G == IMG.Pixels[i].G &&
			   t_ret[q].B == IMG.Pixels[i].B) {
				t_add = false;
				break;
			}
		}
		if(t_add) {
			t_ret.push_back(IMG.Pixels[i]);
		}
	}
	return t_ret;
}
void PalletiseImage(StomaImagePack::Image &IMG, std::vector<StomaImagePack::Colour> PAL) {
	// iterate over every pixel
	std::vector<float> distances;
	distances.resize(PAL.size());
	uint32_t ClosestPalletColour = 0;
	uint32_t PixelCount = (IMG.Size.Width * IMG.Size.Height);
	for(uint32_t i = 0; i < PixelCount; i++) {
		// gets pixels distance to pallet colours (RGB only)
		for(uint32_t q = 0; q < PAL.size(); q++) {
			distances[q] = GetRGBColourDistance(PAL[q], IMG.Pixels[i]);
		}
		// gets closest pallet colour
		ClosestPalletColour = 0;
		for(uint32_t q = 1; q < PAL.size(); q++) {
			if(distances[q] < distances[ClosestPalletColour]) {
				ClosestPalletColour = q;
			}
		}
		// make pixel its closest pallet counterpart
		IMG.Pixels[i].R = PAL[ClosestPalletColour].R;
		IMG.Pixels[i].G = PAL[ClosestPalletColour].G;
		IMG.Pixels[i].B = PAL[ClosestPalletColour].B;
	}
}
std::vector<StomaImagePack::Image> ReorderByVolume(std::vector<StomaImagePack::Image> UNORDERED) {
	std::vector<uint32_t> volumest(UNORDERED.size());
	uint32_t hold;
	// create volumest list
	for(uint32_t i = 0; i < UNORDERED.size(); i++) {
		volumest[i] = i;
	}
	// reorder volumest
	for(uint32_t i = 1; i < UNORDERED.size(); i++) {
		if(UNORDERED[volumest[i - 1]].Pixels.size() <
		   UNORDERED[volumest[i]].Pixels.size()) {
			// if smaller switch and restart
			hold = volumest[i];
			volumest[i] = volumest[i - 1];
			volumest[i - 1] = hold;
			i = 0;
		}
	}
	std::vector<StomaImagePack::Image> OrderedImages;
	for(uint32_t i = 0; i < UNORDERED.size(); i++) {
		OrderedImages.push_back(UNORDERED[volumest[i]]);
	}
	return OrderedImages;
}

inline uint32_t GetCoordinate(uint32_t XPOS, uint32_t YPOS, uint32_t MAXX) {
	return (XPOS + (YPOS * MAXX));
}



