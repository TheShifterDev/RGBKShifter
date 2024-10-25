
#include <StomaImagePack/StomaImagePack.hh>
#define STOMAIMAGEPACK_IMPLEM
#include <StomaImagePack/StomaImagePack.hh>

#include <png++/png.hpp> // local install via pacman

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H



// #include <freetype2/ft2build.h>

#include <string>

enum class ColourCommands{
	ASIS,// does nothing to file on read
	SHIFT,// shifts colours to closest extremes
	PALET,// paletises image to most recent colour palet
	ENDOF
};
enum class FileType {
	PNG,
	STIMPAC,

	ENDOF
};

bool InitdFreeType = false;
FT_Library FontLibrary;

void Write_png(StomaImagePack::Image IMG, std::string NAM);

StomaImagePack::Image Read_png(std::string NAM,StomaImagePack::GroupType TYPE);
StomaImagePack::Image Read_ttf(std::string NAM);
void SliceOutLastOfChar(std::string INP, char TARG, std::string &OutStart,
						std::string &OutEnd);
std::string LowerCaseify(std::string INP);
void WriteOut(StomaImagePack::Image IMG, std::string NAM);
// NOTE: NEVER USE MACROS LIKE
// >>> #define GETCORD(X, Y, XMAX) (X + (Y * XMAX)) <<<
// AS NOT INCLUDING '('&')' IN
// "GETCORD((MainXPos + SubXPos), (MainYPos + SubYPos),MainImage.Size.Width)"
// CAUSED A BUG THAT TOOK DAYS TO TRACK
inline uint32_t GetCoordinate(uint32_t XPOS, uint32_t YPOS, uint32_t MAXX);

std::vector<StomaImagePack::Image>
SeperateGlyphs(std::vector<StomaImagePack::Image> IMG);
std::vector<StomaImagePack::Image>
ReorderByVolume(std::vector<StomaImagePack::Image> IMG);
float GetRGBColourDistance(StomaImagePack::Colour A, StomaImagePack::Colour B);
void PalletiseImage(StomaImagePack::Image &IMG,
					std::vector<StomaImagePack::Colour> PAL);
StomaImagePack::Image MergeImages(std::vector<StomaImagePack::Image> IMG);
std::vector<StomaImagePack::Colour>
ExtractPallet_Image(StomaImagePack::Image IMG);

// commands
ColourCommands ChangeReadFileColour = ColourCommands::ASIS; 
FileType OutFileType = FileType::PNG;	
bool OutputAtlas = true;
//bool CommandList[(uint8_t)CommandEnum::ENDOF] = {};


std::string OutputName = "rgbkshifted"; // used in -n/--name
std::string OutputDirectory = "";		 // used in -o/--out
StomaImagePack::GroupType CurrentReadingType = StomaImagePack::GroupType::REGULAR;
// clang-format off
std::vector<StomaImagePack::Colour> ExtremeColours{
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
std::vector<StomaImagePack::Colour> PalletColours = ExtremeColours;
// clang-format on

// clang-format off
std::string HelpText =
	"Arguments							Description\n"
	"-s/--shift							= shifts fed images colours to the extremes !!!disables paletise!!!\n"
	"-p/--palet/--paletise <PALETFILE>	= paletises	all input images with the palet file !!!disables shift!!!\n"
	"-c/--cutup							= outputs any input file/s glyphs as images ! default ! !!!disables atlas!!!\n"
	"-a/--atlas							= packs all fed files into a single output file !!!disables cutup!!!\n"
	"-o/--out/--output <DIRECTORY>		= DIRECTORY arg is the location of the writen file/s\n"
	"-f/--format <FORMAT>				= FORMAT arg is the file type of the output file/s \"png\" or \"stimpac\"\n"
	"-n/--name <NAME>					= NAME arg is the saved name of the packed file OR the prefix of all shifted files\n"
	"-h/--help							= print this help text\n"
	"Notes\n"
	"1) arguments must be seperated, (-s -a) is correct, (-sa) will be ignored as an invalid option";
// clang-format on

int main(int argc, char *argv[]) {
	// parse args
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
				printf("%s\n", HelpText.c_str());
				exit(0);
			} else if(hold == "-n" || hold == "--name") {
				// output prefix
				OutputName = argv[t_i + 1];
				t_i += 2;
			} else if(hold == "-o" || hold == "--out"|| hold == "--output") {
				// output location
				OutputDirectory = argv[t_i + 1];
				OutputDirectory += '/';
				t_i += 2;
			} else if(hold == "-p" || hold == "--palet" || hold == "--paletise") {
				// palett to use
				std::string PalletName;
				std::string PalletExtension;
				StomaImagePack::Image PalletImage;
				SliceOutLastOfChar(argv[t_i + 1], '.', PalletName, PalletExtension);
				if(PalletExtension == "png") {
					PalletImage = Read_png(PalletName,StomaImagePack::GroupType::REGULAR);
					PalletColours = ExtractPallet_Image(PalletImage);
				}
				// TODO: change pallet
				ChangeReadFileColour = ColourCommands::PALET;
				t_i += 2;
			} else if(hold == "-s" || hold == "--shift") {
				// shift the values of all input files
				ChangeReadFileColour = ColourCommands::SHIFT;
				t_i++;
			} else if(hold == "-a" || hold == "--atlas") {
				OutputAtlas = true;
				t_i++;
			} else if(hold == "-c" || hold == "--cutup") {
				OutputAtlas = false;
				t_i++;
			} else if(hold == "-f" || hold == "--format") {
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
			std::string ImageName = "";
			std::string ImageExtension = "";
			std::string ReadLocation = "";
			StomaImagePack::Image TempImage;
			// split "./gorbinos/file.png" to "./gorbinos/file" & "png"
			SliceOutLastOfChar(hold, '.', ImageName, ImageExtension);
			if(ImageExtension == "png") {
				TempImage = Read_png(ImageName,CurrentReadingType);
			} else if(ImageExtension == "stimpac") {
				TempImage = StomaImagePack::ReadStimpac(ImageName);
			} else if(ImageExtension == "ttf"){
				TempImage = Read_ttf(ImageName);
				ImageList.push_back(TempImage);
				goto skippalet;
			} else{
				printf("format %s is not valid",ImageExtension.c_str());
				exit(111);
			}
			//
			switch (ChangeReadFileColour) {
				case(ColourCommands::SHIFT):{
					PalletiseImage(TempImage,ExtremeColours);
				};
				case(ColourCommands::PALET):{
					PalletiseImage(TempImage,PalletColours);
				}
				default:{break;}// asis should leave colours as is
			}
			ImageList.push_back(TempImage);
			skippalet:;
			t_i++;
		}
	}

	// write file
	if (OutputAtlas){
		// output single atlas file
		StomaImagePack::Image AtlasImage = MergeImages(ImageList);
		WriteOut(AtlasImage, OutputDirectory + OutputName);
	}else{
		// output as multiple glyph files
		std::vector<StomaImagePack::Image> GlyphImageList = SeperateGlyphs(ImageList);
		for(uint32_t i = 0; i < GlyphImageList.size(); i++) {
			WriteOut(GlyphImageList[i],
				OutputDirectory + OutputName +"_"+ GlyphImageList[i].Groups[0].Name +"_"+ GlyphImageList[i].Groups[0].Glyphs[0].Name);
		}
	}

	if(InitdFreeType){
		// TODO: dinit freetype here
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
StomaImagePack::Image Read_ttf(std::string NAM) {
	// NOTE: based on https://nikramakrishnan.github.io/freetype-web-jekyll/docs/tutorial/step1.html
	// and https://github.com/tsoding/ded
	// NOTE: even though we request a desired size freetype can just ignore it and return an oversized glyph
	StomaImagePack::Image ReturnImage;
	// prepare font texture
	uint32_t DesiredWidth = 64;
	uint32_t DesiredHeight = 64;
	
	// HACK: currently only gets chars within char range 
	uint32_t DesiredGlyphCount = 128-32;
	
	FT_Face TypeFace;
	FT_Error err;

	uint8_t TransposingColour;
	uint32_t TransposingColourPosition;
	uint32_t TargetColourPosition;
	uint32_t CurrentChar;

	uint32_t RequiredWidth = 0;
	uint32_t RequiredHeight = 0;
	if(!InitdFreeType){
		err = FT_Init_FreeType(&FontLibrary);
		if(err != 0){
			printf("freetype 'FT_Init_FreeType' returned error %i\n",err);
			exit(111);				
		}
		InitdFreeType = true;
	}
	std::string loc = NAM + ".ttf";
	// --- load in fonts as desired size
	err = FT_New_Face(FontLibrary,loc.c_str(),0,&TypeFace);
	if(err != 0){printf("freetype 'FT_New_Face' returned error %i\n",err);exit(111);}
	err = FT_Set_Pixel_Sizes(TypeFace,DesiredWidth,DesiredHeight);
	if(err != 0){printf("freetype 'FT_Set_Pixel_Sizes' returned error %i\n",err);exit(111);				}
	// --- prepare fontgroup
	ReturnImage.Groups.resize(1);
	ReturnImage.Groups[0].Name = NAM;
	ReturnImage.Groups[0].Type = StomaImagePack::GroupType::FONT;
	ReturnImage.Groups[0].Glyphs.resize(DesiredGlyphCount);
	// iterate to get size data for the glyphs to construct pixel buffer and glyphdata

	for(uint32_t gl = 0;gl<DesiredGlyphCount;gl++){
		CurrentChar = gl+32;
		err = FT_Load_Char(TypeFace,CurrentChar,FT_LOAD_RENDER|FT_LOAD_TARGET_(FT_RENDER_MODE_SDF));
		if(err != 0){printf("freetype 'FT_Load_Char' returned error %i\n",err);exit(111);}
		err = FT_Render_Glyph(TypeFace->glyph,FT_RENDER_MODE_NORMAL);
		if(err != 0){printf("freetype 'FT_Render_Glyph' returned error %i\n",err);exit(111);}
		// --- put bitmap info data into glyph
		ReturnImage.Groups[0].Glyphs[gl].Name = (char)gl+32;
		ReturnImage.Groups[0].Glyphs[gl].Offset = {RequiredWidth,0};
		ReturnImage.Groups[0].Glyphs[gl].Size = {TypeFace->glyph->bitmap.width,TypeFace->glyph->bitmap.rows};
		// --- increase required width and height
		RequiredWidth += TypeFace->glyph->bitmap.width;
		if(RequiredHeight < TypeFace->glyph->bitmap.rows){RequiredHeight = TypeFace->glyph->bitmap.rows;}
	}
	// --- produce the pixel buffer
	ReturnImage.Size = {RequiredWidth,RequiredHeight};
	ReturnImage.Pixels.resize(RequiredWidth*RequiredHeight);
	// --- iterate for pixel transposing
	for(uint32_t gl = 0;gl<DesiredGlyphCount;gl++){
		CurrentChar = gl+32;
		// --- have freetype draw char to bitmap buffer
		err = FT_Load_Char(TypeFace,CurrentChar,FT_LOAD_RENDER|FT_LOAD_TARGET_(FT_RENDER_MODE_SDF));
		if(err != 0){printf("freetype 'FT_Load_Char' returned error %i\n",err);exit(111);}
		err = FT_Render_Glyph(TypeFace->glyph,FT_RENDER_MODE_NORMAL);
		if(err != 0){printf("freetype 'FT_Render_Glyph' returned error %i\n",err);exit(111);}
		if((TypeFace->glyph->bitmap.width*TypeFace->glyph->bitmap.rows) > 0){
			// --- sanity check that the buffer exists
			if(TypeFace->glyph->bitmap.buffer == nullptr){printf("returned charbuffer is nullptr");exit(111);}
			// --- check if size is fucked
			if(TypeFace->glyph->bitmap.width != ReturnImage.Groups[0].Glyphs[gl].Size.Width
			|| TypeFace->glyph->bitmap.rows != ReturnImage.Groups[0].Glyphs[gl].Size.Height){
				printf("glyph %i (aka %c)'s size is different from its previous size, pw:%i ph:%i != cw:%i ch:%i\n",
					gl,CurrentChar,
					TypeFace->glyph->bitmap.width,
					TypeFace->glyph->bitmap.rows,
					ReturnImage.Groups[0].Glyphs[gl].Size.Width,
					ReturnImage.Groups[0].Glyphs[gl].Size.Height);
				exit(111);
			}
			if((TypeFace->glyph->bitmap.rows + ReturnImage.Groups[0].Glyphs[gl].Offset.Height) > ReturnImage.Size.Height){
				printf("glyph %i (aka %c)'s height is greater than the max size, off:%i + hig:%i > max:%i\n",
					gl,CurrentChar,
					TypeFace->glyph->bitmap.rows,
					ReturnImage.Groups[0].Glyphs[gl].Offset.Height,
					ReturnImage.Size.Height);
				exit(111);
			}
			if((TypeFace->glyph->bitmap.width + ReturnImage.Groups[0].Glyphs[gl].Offset.Width) > ReturnImage.Size.Width){
				printf("glyph %i (aka %c)'s width is greater than the max size, off:%i + wid:%i > max:%i\n",
		   			gl,CurrentChar,
					TypeFace->glyph->bitmap.width,
					ReturnImage.Groups[0].Glyphs[gl].Offset.Width,
					ReturnImage.Size.Width);
				exit(111);
			}
			// --- put bitmap buffer data into colour buffer in glyphs position
			for(uint32_t pixh = 0;pixh < TypeFace->glyph->bitmap.width;pixh++){
			for(uint32_t pixw = 0;pixw < TypeFace->glyph->bitmap.rows ;pixw++){
				TransposingColourPosition = GetCoordinate(
					pixw,
					pixh,
					TypeFace->glyph->bitmap.rows);
				TargetColourPosition = GetCoordinate(
					ReturnImage.Groups[0].Glyphs[gl].Offset.Width + pixw,
					ReturnImage.Groups[0].Glyphs[gl].Offset.Height + pixh,
					ReturnImage.Size.Width);
				TransposingColour = TypeFace->glyph->bitmap.buffer[TransposingColourPosition];
				ReturnImage.Pixels[TargetColourPosition] = {
					TransposingColour,
					TransposingColour,
					TransposingColour,
					255};
			}}
		}
	}
	
	return ReturnImage;
}
StomaImagePack::Image Read_png(std::string NAM,StomaImagePack::GroupType TYPE) {
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
	ReturnImage.Groups.resize(1);
	ReturnImage.Groups[0].Glyphs.resize(1);
	SliceOutLastOfChar(NAM, '/', SourceDir, GlyphName);
	ReturnImage.Groups[0].Name = "";
	ReturnImage.Groups[0].Type = TYPE;
	ReturnImage.Groups[0].Glyphs[0].Name = GlyphName;
	ReturnImage.Groups[0].Glyphs[0].Size = ReturnImage.Size;
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
			HoldPixel.red = 	IMG.Pixels[CurrentCoordinate].R;
			HoldPixel.green = 	IMG.Pixels[CurrentCoordinate].G;
			HoldPixel.blue = 	IMG.Pixels[CurrentCoordinate].B;
			HoldPixel.alpha = 	IMG.Pixels[CurrentCoordinate].A;
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

// clang-format off
std::vector<StomaImagePack::Image> SeperateGlyphs(std::vector<StomaImagePack::Image> FEDIMAGES) {
	// takes an array of stoma images and cuts every glyph out into its own image
	std::vector<StomaImagePack::Image> OutputImages;
	StomaImagePack::Image HoldingImage;
	StomaImagePack::Resolution NewSize;
	StomaImagePack::Resolution OldOffset;
	std::string OldGroupName;
	std::string OldGlyphName;
	uint32_t HoldPixelPosition;
	uint32_t FedPixelPosition;
	uint32_t NewPixelCount;
	uint32_t OldPixelCount;

	for(uint32_t img = 0; img < FEDIMAGES.size(); img++) {
		OldPixelCount = FEDIMAGES[img].Pixels.size();
	for(uint32_t grp = 0; grp < FEDIMAGES[img].Groups.size(); grp++) {
	for(uint32_t gly = 0; gly < FEDIMAGES[img].Groups[grp].Glyphs.size(); gly++) {
		NewSize = FEDIMAGES[img].Groups[grp].Glyphs[gly].Size;
		OldOffset = FEDIMAGES[img].Groups[grp].Glyphs[gly].Offset;
		OldGroupName = FEDIMAGES[img].Groups[grp].Name;
		OldGlyphName = FEDIMAGES[img].Groups[grp].Glyphs[gly].Name;
		NewPixelCount = (NewSize.Width * NewSize.Height);
		if (NewPixelCount != 0){
			HoldingImage.Size = NewSize;
			HoldingImage.Groups.resize(1);
			HoldingImage.Groups[0].Name = OldGroupName;
			HoldingImage.Groups[0].Glyphs.resize(1);
			HoldingImage.Groups[0].Glyphs[0].Name = OldGlyphName;
			HoldingImage.Groups[0].Glyphs[0].Offset.Width = 0;
			HoldingImage.Groups[0].Glyphs[0].Offset.Height = 0;
			HoldingImage.Groups[0].Glyphs[0].Size = NewSize;
			HoldingImage.Pixels.resize(NewPixelCount);
			// get pixels from old image into new image
			if((OldOffset.Width+NewSize.Width)		> FEDIMAGES[img].Size.Width){
				printf("Image[%i].Group[%i].Glyph[%i]'s offset+size width is greater than the width of the image, off:%i + size:%i > width:%i\n",img,grp,gly,OldOffset.Width,NewSize.Width,FEDIMAGES[img].Size.Width);exit(1212);
			}
			if((OldOffset.Height+NewSize.Height)	> FEDIMAGES[img].Size.Height){
				printf("Image[%i].Group[%i].Glyph[%i]'s offset+size height is greater than the height of the image, off:%i + size:%i > height:%i\n",img,grp,gly,OldOffset.Height,NewSize.Height,FEDIMAGES[img].Size.Height);exit(1212);
			}
			for(uint32_t h = 0; h < NewSize.Height; h++) {
			for(uint32_t w = 0; w < NewSize.Width; w++) {
				HoldPixelPosition = GetCoordinate(w, h, NewSize.Width);
				FedPixelPosition = 	GetCoordinate(
					OldOffset.Width + w,
					OldOffset.Height + h,
					FEDIMAGES[img].Size.Width);
				if(HoldPixelPosition > NewPixelCount){
					printf("new image is too small to take data from Image[%i].Group[%i].Glyph[%i]\n",img,grp,gly);exit(1212);
				}
				if(FedPixelPosition > OldPixelCount){
					printf("old image is too small for Image[%i].Group[%i].Glyph[%i]'s data to be valid\n",img,grp,gly);exit(1212);
				}
				HoldingImage.Pixels[HoldPixelPosition] = FEDIMAGES[img].Pixels[FedPixelPosition];
			}}
			OutputImages.push_back(HoldingImage);
		}
	}}}

	return OutputImages;
}
// clang-format on

StomaImagePack::Image
MergeImages(std::vector<StomaImagePack::Image> SUBIMAGES) {
	StomaImagePack::Image OutputImage;
	OutputImage.Groups.resize(1); 
	uint32_t MaxWRequired = 0;
	uint32_t MaxHRequired = 0;
	std::vector<StomaImagePack::Colour> Canvas;
	std::vector<bool> Occupied;

	// TODO: Create a better merge algarythm
	#define MERGEALGORITHM_DUMBDUMPANDTRIM
	#ifdef MERGEALGORITHM_DUMBDUMPANDTRIM
	// NOTE: this method results in a ton of wasted space as it creates a strip of images being written

	// iterate over all glyphs and get the max x and y required
	for(uint32_t im = 0; im<SUBIMAGES.size();im++){
	for(uint32_t gr = 0; gr<SUBIMAGES[im].Groups.size();gr++){
	for(uint32_t gl = 0; gl<SUBIMAGES[im].Groups[gr].Glyphs.size();gl++){
		MaxWRequired += SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Width;
		MaxHRequired += SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Height;
	}}}
	// create a canvas
	

	uint32_t CanvasSize = MaxWRequired*MaxHRequired;
	Canvas.resize(CanvasSize);
	Occupied.resize(CanvasSize);
	uint32_t CurrentGroup;
	
	bool DrawSafe;		
	// iterate over canvas every glyph and iterate over every position to find a good placement
	for (uint32_t im = 0;im < SUBIMAGES.size();im++) {
	for (uint32_t gr = 0;gr < SUBIMAGES[im].Groups.size();gr++) {
		// get group type
		switch (SUBIMAGES[im].Groups[gr].Type) {
			case(StomaImagePack::GroupType::REGULAR):{
				// put into new images group 0
				CurrentGroup = 0;
				break;
			}
			case(StomaImagePack::GroupType::NORMALMAP):{
				// put normals into group 1
				CurrentGroup = 1;
				break;
			}
			case(StomaImagePack::GroupType::FONT):{
				// create a new group
				CurrentGroup = OutputImage.Groups.size();
				OutputImage.Groups.resize(CurrentGroup+1);
				break;
			}
			default:{
				printf("Invalid GroupType on image %i\n",im);
				exit(122);
			}
		}
	for (uint32_t gl = 0;gl < SUBIMAGES[im].Groups[gr].Glyphs.size();gl++) {
		// check if its safe to draw in position
		for(uint32_t MYPos = 0; MYPos < MaxHRequired; MYPos++) {
		for(uint32_t MXPos = 0; MXPos < MaxWRequired; MXPos++) {
			// test if first pixel is empty
			if (Occupied[GetCoordinate(MXPos,MYPos,MaxWRequired)] == false) {
				DrawSafe = true;
				// check if position collides with previously written positions
				for(uint32_t SYPos=0;SYPos<SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Height;SYPos++){
				for(uint32_t SXPos=0;SXPos<SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Width ;SXPos++){
					if(Occupied[GetCoordinate(MXPos + SXPos,MYPos + SYPos,MaxWRequired)] == true){
						DrawSafe = false;
						goto AlreadyWrittenBreakout;
					}
				}}
				AlreadyWrittenBreakout:;
				if (DrawSafe){
					uint32_t MPoint,SPoint;
					// draw
					for(uint32_t SYPos=0;SYPos<SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Height;SYPos++){
					for(uint32_t SXPos=0;SXPos<SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Width ;SXPos++){
						MPoint = GetCoordinate(MXPos + SXPos,MYPos + SYPos,MaxWRequired);
						SPoint = GetCoordinate(SXPos, SYPos, SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Width);
						Canvas[MPoint] = SUBIMAGES[im].Pixels[SPoint];
						Occupied[MPoint] = true;
					}}
					
					// place glyph
					
					StomaImagePack::Glyph TransposingGlyph = {};
					TransposingGlyph.Name = SUBIMAGES[im].Groups[gr].Glyphs[gl].Name;
					TransposingGlyph.Offset.Width	= MXPos;
					TransposingGlyph.Offset.Height	= MYPos;
					TransposingGlyph.Size 			= SUBIMAGES[im].Groups[gr].Glyphs[gl].Size;
								
					OutputImage.Groups[CurrentGroup].Glyphs.push_back(TransposingGlyph);
					goto NextGlyph;
				}
			}
		}}
	}
	// endof glyph
	NextGlyph:;
	}
	// endof group
	}
	// endof image
	#endif
	// clang-format off
	// ---------------------------TRIM--------------------------------//
	// Trim unused lines or collumns
	uint32_t CropWSize = MaxWRequired;
	uint32_t CropHSize = MaxHRequired;
	
	//StomaImagePack::Resolution CropSize = OutputImage.Size;
	// get true used Heigth/Rows
	while (CropHSize > 1) {
		for(uint32_t w=0;w<CropWSize;w++){
			if (Occupied[GetCoordinate(w,CropHSize-1, MaxWRequired)]){
				goto PixelRowOccupiedBreakout;
			}
		}
		CropHSize--;
	}
	PixelRowOccupiedBreakout:;
	// get true used Width/Columns
	while (CropWSize > 1) {
		for(uint32_t h=0;h<CropHSize;h++){
			if (Occupied[GetCoordinate(CropWSize-1 , h, MaxWRequired)]){
				goto PixelColumnOccupiedBreakout;
			}
		}
		CropHSize--;
	}
	PixelColumnOccupiedBreakout:;
	// check if there are rows or columns to trim
	uint32_t NewPoint,OldPoint;
	OutputImage.Pixels.resize(CropWSize*CropHSize);
	if(CropWSize != MaxWRequired 
	|| CropHSize != MaxHRequired){
		// transpose canvas pixels onto trimmed output image
		for(uint32_t YPos = 0; YPos < CropHSize; YPos++) {
		for(uint32_t XPos = 0; XPos < CropWSize; XPos++) {
			NewPoint = GetCoordinate(XPos,YPos,CropWSize);
			OldPoint = GetCoordinate(XPos,YPos,MaxWRequired);
			OutputImage.Pixels[NewPoint] = Canvas[OldPoint];
		}}
		OutputImage.Size = {CropWSize,CropHSize};
	}else{
		// space is efficantly used already so just feed canvas to Image
		for(uint32_t i=0;i<OutputImage.Size.Width*OutputImage.Size.Height;i++){
			OutputImage.Pixels[i] = Canvas[i];
		}
	}
	
	return OutputImage;
	// clang-format on
}
float GetRGBColourDistance(StomaImagePack::Colour A, StomaImagePack::Colour B) {
	return (((((float)A.R - (float)B.R) * ((float)A.R - (float)B.R)) +
			 (((float)A.G - (float)B.G) * ((float)A.G - (float)B.G)) +
			 (((float)A.B - (float)B.B) * ((float)A.B - (float)B.B))) /
			2);
}
std::vector<StomaImagePack::Colour>
ExtractPallet_Image(StomaImagePack::Image IMG) {
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
void PalletiseImage(StomaImagePack::Image &IMG,
					std::vector<StomaImagePack::Colour> PAL) {
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
std::vector<StomaImagePack::Image>
ReorderByVolume(std::vector<StomaImagePack::Image> UNORDERED) {
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
