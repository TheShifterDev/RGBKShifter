
#include <StomaImagePack/StomaImagePack.hh>
#define STOMAIMAGEPACK_IMPLEM
#include <StomaImagePack/StomaImagePack.hh>

#include <png++/png.hpp> // local install via pacman

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H



// #include <freetype2/ft2build.h>

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
				printf("%s\n", HelpText.c_str());
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
			} else if(ImageExtension == "ttf"){
				TempImage = Read_ttf(ImageName);
				ImageList.push_back(TempImage);
			} else{
				printf("format %s is not valid",ImageExtension.c_str());
				exit(111);
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
	} else if(CommandList[(uint8_t)CommandEnum::Cutup]) {
		// output every glyph as unique files
		if(!CommandList[(uint8_t)CommandEnum::Format]) {
			OutFileType = FileType::PNG;
		}
		std::vector<StomaImagePack::Image> GlyphImageList =
			SeperateGlyphs(ImageList);
		for(uint32_t i = 0; i < GlyphImageList.size(); i++) {
			WriteOut(GlyphImageList[i],
					 OutDir + OutName + GlyphImageList[i].Groups[0].Name);
		}
	} else {
		// output all input files after processing
		if(!CommandList[(uint8_t)CommandEnum::Format]) {
			OutFileType = FileType::PNG;
		}
		for(uint32_t i = 0; i < ImageList.size(); i++) {
			WriteOut(ImageList[i],
					 OutDir + OutName + ImageList[i].Groups[0].Name);
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
StomaImagePack::Image Read_ttf(std::string NAM) {
	// NOTE: based on https://nikramakrishnan.github.io/freetype-web-jekyll/docs/tutorial/step1.html
	// and https://github.com/tsoding/ded
	StomaImagePack::Image ReturnImage;
	// TODO: add stuff here
	// prepare font texture
	{
		uint32_t charwidth,charheight;
		charwidth = 64;
		charheight = 64;
		//use freetype to generate a texture from font file
		// NOTE: NotoSans is a temporary font 
		FT_Library fontlibrary;
		FT_Error err;
		FT_Face typeface;
		err = FT_Init_FreeType(&fontlibrary);
		if(err != 0){
			printf("freetype 'FT_Init_FreeType' returned error %i\n",err);
			exit(111);				
		}
		std::string loc = NAM + ".ttf";
		err = FT_New_Face(fontlibrary,loc.c_str(),0,&typeface);
		if(err != 0){
			printf("freetype 'FT_New_Face' returned error %i\n",err);
			exit(111);				
		}
		// each glyph will be 64 size
		err = FT_Set_Pixel_Sizes(typeface,charwidth,charheight);
		if(err != 0){
			printf("freetype 'FT_Set_Pixel_Sizes' returned error %i\n",err);
			exit(111);				
		}
		// HACK: currently only gets chars within char range 
		uint32_t glyphcount = 128-32;
		//std::vector<StomaImagePack::Colour> FontTextureData;
		//FontTextureData.resize(64*64*glyphcount);
		StomaImagePack::Image FontImage;
		FontImage.Groups.resize((uint32_t)StomaImagePack::GroupType::ENDOF);
		StomaImagePack::GlyphGroup& FontGroup = FontImage.Groups[(uint32_t)StomaImagePack::GroupType::FONT];
		FontGroup.Name = NAM;
		FontGroup.Type = StomaImagePack::GroupType::FONT;
		FontGroup.Glyphs.resize(glyphcount);
		{
			uint32_t w = 0,h = 0;
			for(uint32_t gl = 0;gl<glyphcount;gl++){
				// NOTE: assumes fontimage will be 16 glyphs wide
				if(w > 15){w = 0;h++;}
				FontGroup.Glyphs[gl].Name = (char)gl+32;
				FontGroup.Glyphs[gl].Size = {charwidth,charheight};
				FontGroup.Glyphs[gl].Offset = {w*charwidth,h*charheight};
				w++;
			}
			FontImage.Size = {charwidth*16,charheight*(h+1)};
		}
		FontImage.Pixels.resize(charwidth*charheight);

		uint8_t TransposingColour;
		uint32_t TransposingColourPosition;
		uint32_t TargetColourPosition;
		uint32_t CurrentChar;
		
		for(uint32_t Target = 0;Target<(128-32);Target++){
			CurrentChar = Target+32;
			// BUG: FT_Load_Char causes "realloc(): invalid next size"
			err = FT_Load_Char(typeface,CurrentChar,FT_LOAD_RENDER|FT_LOAD_TARGET_(FT_RENDER_MODE_SDF));
			if(err != 0){
				printf("freetype 'FT_Load_Char' returned error %i\n",err);
				exit(111);				
			}
			err = FT_Render_Glyph(typeface->glyph,FT_RENDER_MODE_NORMAL);
			if(err != 0){
				printf("freetype 'FT_Render_Glyph' returned error %i\n",err);
				exit(111);				
			}
			if((typeface->glyph->bitmap.width * typeface->glyph->bitmap.rows) == 0){
				printf("width or height is 0 w:%i, h:%i, Skipping glyph %c\n",
				typeface->glyph->bitmap.width,
				typeface->glyph->bitmap.rows,((char)CurrentChar));
				goto skipglyph;
			}
			if(typeface->glyph->bitmap.buffer == nullptr){
				printf("returned charbuffer is nullptr");
				exit(111);
			}
			
			for(uint32_t pixh = 0;pixh < typeface->glyph->bitmap.width;pixh++){
			for(uint32_t pixw = 0;pixw < typeface->glyph->bitmap.rows ;pixw++){
				TransposingColourPosition = GetCoordinate(
					typeface->glyph->bitmap_left + pixw,
					typeface->glyph->bitmap_top + pixh,
					typeface->glyph->bitmap.rows);
				TargetColourPosition = GetCoordinate(
					FontGroup.Glyphs[Target].Offset.Width + pixw,
					FontGroup.Glyphs[Target].Offset.Height + pixh,
					FontImage.Size.Width);
				TransposingColour = typeface->glyph->bitmap.buffer[TransposingColourPosition];
				FontImage.Pixels[TargetColourPosition] = {
					TransposingColour,
					TransposingColour,
					TransposingColour,
					255};
			}}
			skipglyph:;
		}
	}
	return ReturnImage;
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
	ReturnImage.Groups.resize(1);
	ReturnImage.Groups[0].Glyphs.resize(1);
	SliceOutLastOfChar(NAM, '/', SourceDir, GlyphName);
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

std::vector<StomaImagePack::Image>
SeperateGlyphs(std::vector<StomaImagePack::Image> FEDIMAGES) {
	// takes an array of stoma images and cuts every glyph out into its own
	// image
	std::vector<StomaImagePack::Image> OutputImages;
	StomaImagePack::Image HoldingImage;
	StomaImagePack::Resolution NewSize;
	StomaImagePack::Resolution OldOffset;
	std::string OldGroupName;
	std::string OldGlyphName;

	for(uint32_t cimg = 0; cimg < FEDIMAGES.size(); cimg++) {
		for(uint32_t cgrp = 0; cgrp < FEDIMAGES[cimg].Groups.size(); cgrp++) {
			if(FEDIMAGES[cimg].Groups[cgrp].Glyphs.size() < 2) {
				if(FEDIMAGES[cimg].Groups[cgrp].Glyphs.size() == 1) {
					// skip as IMG[i] seperation is unnessesary
					OutputImages.push_back(FEDIMAGES[cimg]);
				} else {
					// IMG[i] is broken and will be skipped
				}
			} else {
				for(uint32_t cgly = 0;
					cgly < FEDIMAGES[cimg].Groups[cgrp].Glyphs.size(); cgly++) {
					NewSize = FEDIMAGES[cimg].Groups[cgrp].Glyphs[cgly].Size;
					OldOffset =
						FEDIMAGES[cimg].Groups[cgrp].Glyphs[cgly].Offset;
					OldGroupName = FEDIMAGES[cimg].Groups[cgrp].Name;
					OldGlyphName =
						FEDIMAGES[cimg].Groups[cgrp].Glyphs[cgly].Name;

					HoldingImage.Size = NewSize;
					HoldingImage.Groups.resize(1);
					HoldingImage.Groups[0].Name = OldGroupName;
					HoldingImage.Groups[0].Glyphs.resize(1);
					HoldingImage.Groups[0].Glyphs[0].Name = OldGlyphName;
					HoldingImage.Groups[0].Glyphs[0].Offset.Width = 0;
					HoldingImage.Groups[0].Glyphs[0].Offset.Height = 0;
					HoldingImage.Groups[0].Glyphs[0].Size = NewSize;

					HoldingImage.Pixels.resize(NewSize.Width * NewSize.Height);
					// get pixels from old image into new image
					for(uint32_t h = 0; h < NewSize.Height; h++) {
						for(uint32_t w = 0; w < NewSize.Width; w++) {
							HoldingImage
								.Pixels[GetCoordinate(w, h, NewSize.Width)] =
								FEDIMAGES[cimg].Pixels[GetCoordinate(
									OldOffset.Width + w, OldOffset.Height + h,
									FEDIMAGES[cimg].Size.Width)];
						}
					}
					OutputImages.push_back(HoldingImage);
				}
			}
		}
	}

	return OutputImages;
}
StomaImagePack::Image
MergeImages(std::vector<StomaImagePack::Image> SUBIMAGES) {
	StomaImagePack::Image OutputImage;
	// starts with 0 being regular images and 1 being normalmaps
	OutputImage.Groups.resize(2); 
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
