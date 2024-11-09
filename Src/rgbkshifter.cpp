
#include <StomaImagePack/StomaImagePack.hh>
#include <cstdio>
#define STOMAIMAGEPACK_IMPLEM
#include <StomaImagePack/StomaImagePack.hh>
#ifdef USING_PNGPP
#include <png++/png.hpp> // local install via pacman
#endif
#ifdef USING_PNGHANDROLLED
// fallback handcoded png reading


#endif
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <string>

enum ExitCode{
	NOERROR = 0,
	NOARGS,
	INVALIDOUTPUTFILEFORMAT,
	FILECORRUPTION,

	FREETYPEERROREXIT,

	PNGPPREADERROREXIT,
	PNGPPWRITEERROREXIT,

	PNGREADERROREXIT,
	PNGWRITEERROREXIT,

	STOMAIMAGEERROREXIT,

	ENDOF
};


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

std::vector<StomaImagePack::Image> SeperateGlyphs(StomaImagePack::Image IMG);
std::vector<StomaImagePack::Image> ReorderByVolume(std::vector<StomaImagePack::Image> IMG);
float GetRGBColourDistance(StomaImagePack::Colour A, StomaImagePack::Colour B);
void PalletiseImage(StomaImagePack::Image &IMG,
					std::vector<StomaImagePack::Colour> PAL);
					
StomaImagePack::Image TrimImage(StomaImagePack::Image INPUT);
StomaImagePack::Image MergeImages(std::vector<StomaImagePack::Image> IMG);
std::vector<StomaImagePack::Colour>
ExtractPallet_Image(StomaImagePack::Image IMG);

// commands
ColourCommands ChangeReadFileColour = ColourCommands::ASIS; 
FileType OutFileType = FileType::PNG;	
bool OutputAtlas = true;

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
		exit((uint32_t)ExitCode::NOARGS);
	}
	while(t_i < argc) {
		hold = argv[t_i];
		// hold = MakeLowerCase::Lower(hold);
		if(hold[0] == '-') {
			if(hold == "-h" || hold == "--help") {
				// help request
				printf("%s\n", HelpText.c_str());
				exit((ExitCode::NOERROR));
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
				exit(ExitCode::INVALIDOUTPUTFILEFORMAT);
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
		StomaImagePack::Image AtlasImage;
		AtlasImage = MergeImages(ImageList);
		AtlasImage = TrimImage(AtlasImage);
		WriteOut(AtlasImage, OutputDirectory + OutputName);
	}else{
		// output as multiple glyph files
		for(uint32_t q=0;q<ImageList.size();q++){
			std::vector<StomaImagePack::Image> GlyphImageList = SeperateGlyphs(ImageList[q]);
			for(uint32_t i = 0; i < GlyphImageList.size(); i++) {
				WriteOut(GlyphImageList[i],
					OutputDirectory + OutputName +"_"+ GlyphImageList[i].Groups[0].Name +"_"+ GlyphImageList[i].Groups[0].Glyphs[0].Name);
			}
		}
	}

	if(InitdFreeType){
		// TODO: dinit freetype here
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
	for(uint32_t i = 0; i < INP.size(); i++) {
		if(INP[i] > 64 && INP[i] < 91) {
			INP[i] += 32;
		}
	}
	return INP;
}

bool SanityCheckImageData(StomaImagePack::Image IMG,std::string FUNCNAME){
	// NOTE: Returns false if data is valid and no errors were found
	
	// TEST: Image has a valid width and height
	if((IMG.Size.Width*IMG.Size.Height) == 0){
		printf("%s has an image with an invalid width(%i) and height(%i)\n",FUNCNAME.c_str(),
		IMG.Size.Width,IMG.Size.Height);
		return true;
	} 
	// TEST: Image has more than 0 pixels
	if(IMG.Pixels.size() == 0){
		printf("%s has an image with 0 pixels\n",FUNCNAME.c_str());
		return true;
	}
	// TEST: if Image has more than 0 groups as 1 is the minimum
	if(IMG.Groups.size() == 0){
		printf("%s has an image with 0 groups\n",FUNCNAME.c_str());
		return true;
	}
	// TEST: if groups have more than 0 glyphs as 1 is the minimum
	for(uint32_t gr=0;gr<IMG.Groups.size();gr++){
		if(IMG.Groups[gr].Glyphs.size() == 0){
			printf("%s has an image with a group[%i] containing 0 glyphs\n",FUNCNAME.c_str(),gr);
			return true;
		}
	}
	// TEST: if any glyphs have an offset larger than size of image
	for(uint32_t gr=0;gr<IMG.Groups.size();gr++){
	for(uint32_t gl=0;gl<IMG.Groups[gr].Glyphs.size();gl++){
		if(	(IMG.Groups[gr].Glyphs[gl].Offset.Width > IMG.Size.Width)
		||	(IMG.Groups[gr].Glyphs[gl].Offset.Height > IMG.Size.Height)){
			printf("%s has an image.group[%i].glyph[%i].offset(w:%i,h:%i) out of image size(w:%i,h:%i) bounds\n",FUNCNAME.c_str(),
			gr,gl,
			IMG.Groups[gr].Glyphs[gl].Offset.Width,
			IMG.Groups[gr].Glyphs[gl].Offset.Height,
			IMG.Size.Width,
			IMG.Size.Height);
			return true;
		}
	}}
	// TEST: if any glyphs have a size larger than size of image
	for(uint32_t gr=0;gr<IMG.Groups.size();gr++){
	for(uint32_t gl=0;gl<IMG.Groups[gr].Glyphs.size();gl++){
		if(	(IMG.Groups[gr].Glyphs[gl].Size.Width > IMG.Size.Width)
		||	(IMG.Groups[gr].Glyphs[gl].Size.Height > IMG.Size.Height)){
			printf("%s has an image.group[%i].glyph[%i].size(w:%i,h:%i) out of image size(w:%i,h:%i) bounds\n",FUNCNAME.c_str(),
			gr,gl,
			IMG.Groups[gr].Glyphs[gl].Size.Width,
			IMG.Groups[gr].Glyphs[gl].Size.Height,
			IMG.Size.Width,
			IMG.Size.Height);
			return true;
		}
	}}

	// TEST: if any glyphs have a (offset+size) larger than size of image
	{
	uint32_t wmax;
	uint32_t hmax;
	for(uint32_t gr=0;gr<IMG.Groups.size();gr++){
	for(uint32_t gl=0;gl<IMG.Groups[gr].Glyphs.size();gl++){
		wmax = IMG.Groups[gr].Glyphs[gl].Offset.Width+IMG.Groups[gr].Glyphs[gl].Size.Width;
		hmax = IMG.Groups[gr].Glyphs[gl].Offset.Height+IMG.Groups[gr].Glyphs[gl].Size.Height;
		if(	(wmax > IMG.Size.Width)
		||	(hmax > IMG.Size.Height)){
			printf("%s has an image.group[%i].glyph[%i].(offset+size)(w:%i,h:%i) out of image size(w:%i,h:%i) bounds\n",FUNCNAME.c_str(),
			gr,gl,
			wmax,hmax,
			IMG.Size.Width,
			IMG.Size.Height);
			return true;
		}
	}}
	}

	// no errors found
	return false;
}

// clang-format off
std::vector<StomaImagePack::Image> SeperateGlyphs(StomaImagePack::Image IMG) {
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

	
	#ifdef USING_SANITYCHECKS
	{
		StomaImagePack::Image& TestTarget = IMG;
		if(SanityCheckImageData(TestTarget,"SeperateGlyphs Input")){
			exit(ExitCode::FILECORRUPTION);
		};
	}
	#endif

	OldPixelCount = IMG.Pixels.size();
	for(uint32_t grp = 0; grp < IMG.Groups.size(); grp++) {
	for(uint32_t gly = 0; gly < IMG.Groups[grp].Glyphs.size(); gly++) {
		NewSize = IMG.Groups[grp].Glyphs[gly].Size;
		OldOffset = IMG.Groups[grp].Glyphs[gly].Offset;
		OldGroupName = IMG.Groups[grp].Name;
		OldGlyphName = IMG.Groups[grp].Glyphs[gly].Name;
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
			if((OldOffset.Width+NewSize.Width)		> IMG.Size.Width){
				printf("Image.Group[%i].Glyph[%i]'s offset+size width is greater than the width of the image, off:%i + size:%i > width:%i\n",grp,gly,OldOffset.Width,NewSize.Width,IMG.Size.Width);
				exit((uint32_t)ExitCode::STOMAIMAGEERROREXIT);
			}
			if((OldOffset.Height+NewSize.Height)	> IMG.Size.Height){
				printf("Image.Group[%i].Glyph[%i]'s offset+size height is greater than the height of the image, off:%i + size:%i > height:%i\n",grp,gly,OldOffset.Height,NewSize.Height,IMG.Size.Height);
				exit((uint32_t)ExitCode::STOMAIMAGEERROREXIT);
			}
			for(uint32_t h = 0; h < NewSize.Height; h++) {
			for(uint32_t w = 0; w < NewSize.Width; w++) {
				HoldPixelPosition = GetCoordinate(w, h, NewSize.Width);
				FedPixelPosition = 	GetCoordinate(
					OldOffset.Width + w,
					OldOffset.Height + h,
					IMG.Size.Width);
				if(HoldPixelPosition > NewPixelCount){
					printf("new image is too small to take data from Image.Group[%i].Glyph[%i]\n",grp,gly);
					exit((uint32_t)ExitCode::STOMAIMAGEERROREXIT);
				}
				if(FedPixelPosition > OldPixelCount){
					printf("old image is too small for Image.Group[%i].Glyph[%i]'s data to be valid\n",grp,gly);
					exit((uint32_t)ExitCode::STOMAIMAGEERROREXIT);
				}
				HoldingImage.Pixels[HoldPixelPosition] = IMG.Pixels[FedPixelPosition];
			}}
			OutputImages.push_back(HoldingImage);
		}
	}}
	#ifdef USING_SANITYCHECKS
	{
		std::vector<StomaImagePack::Image>& TestTarget = OutputImages;
		if((uint32_t)TestTarget.size() == 0){
			printf("SeperateGlyphs has created a OutputImages count of 0\n");
			exit(ExitCode::FILECORRUPTION);
		}
		for(uint32_t im=0;im<TestTarget.size();im++){
			if(SanityCheckImageData(TestTarget[im],"SeperateGlyphs Output")){
				exit(ExitCode::FILECORRUPTION);
			};
		}
	}
	#endif
	return OutputImages;
}
// clang-format on

StomaImagePack::Image TrimImage(StomaImagePack::Image INPUT){
	#ifdef USING_SANITYCHECKS
	{
		StomaImagePack::Image& TestTarget = INPUT;
		if(SanityCheckImageData(TestTarget,"TrimImage Input")){
			exit(ExitCode::FILECORRUPTION);
		};
	}
	#endif

	// NOTE: this was inside of mergeimages but it was additional bug surface area
	// might change back at some point but for now its isolated into its own segment
	StomaImagePack::Image OutputImage;
	// construct an atlas of occupied points
	std::vector<bool> OccupiedPixels{};
	//StomaImagePack::Glyph& CurrentGlyph = INPUT.Groups[0].Glyphs[0];
	uint32_t NewPoint,OldPoint;
	
	OccupiedPixels.resize(INPUT.Pixels.size());
	{
		for (uint32_t gr=0; gr<INPUT.Groups.size();gr++) {
		for (uint32_t gl=0; gl<INPUT.Groups[gr].Glyphs.size();gl++) {
			if((INPUT.Groups[gr].Glyphs[gl].Size.Width+INPUT.Groups[gr].Glyphs[gl].Offset.Width)>INPUT.Size.Width
			|| (INPUT.Groups[gr].Glyphs[gl].Size.Height+INPUT.Groups[gr].Glyphs[gl].Offset.Height)>INPUT.Size.Height){
				printf("Trim Image oob check for Group[%i].glyph[%i] is oob as xoff[%i] + xsiz[%i] > imgxsize[%i] or yoff[%i] + ysiz[%i] > imgysize[%i]\n",
						gr,gl,
						INPUT.Groups[gr].Glyphs[gl].Offset.Width,
						INPUT.Groups[gr].Glyphs[gl].Size.Width,
						INPUT.Size.Width,
						INPUT.Groups[gr].Glyphs[gl].Offset.Height,
						INPUT.Groups[gr].Glyphs[gl].Size.Height,
						INPUT.Size.Height);
				exit(ExitCode::FILECORRUPTION);
			}
			if((INPUT.Groups[gr].Glyphs[gl].Size.Width*INPUT.Groups[gr].Glyphs[gl].Size.Height)!=0){
				for (uint32_t Y=0;Y<INPUT.Groups[gr].Glyphs[gl].Size.Height;Y++) {
				for (uint32_t X=0;X<INPUT.Groups[gr].Glyphs[gl].Size.Width;X++) {
					uint32_t XPos = INPUT.Groups[gr].Glyphs[gl].Offset.Width + X;
					uint32_t YPos = INPUT.Groups[gr].Glyphs[gl].Offset.Height + Y;
					OldPoint = GetCoordinate(XPos, YPos, INPUT.Size.Width);
					OccupiedPixels[OldPoint] = true;
				}}
			}
		}}
	}
	uint32_t CropWSize;
	uint32_t CropHSize;
	{
		// get true used Heigth/Rows
		for(CropHSize=INPUT.Size.Height;CropHSize>0;CropHSize--){
		for(uint32_t w=0;w<INPUT.Size.Width;w++){
			if (OccupiedPixels[GetCoordinate(w,(CropHSize-1),INPUT.Size.Width)]){
				goto HHit;
			}
		}}
		HHit:;
		if(CropHSize == 0){
			printf("TrimImage has trimmed the height to 0\n");
			exit(ExitCode::FILECORRUPTION);
		}
		// get true used Width/Columns
		for(CropWSize=INPUT.Size.Width;CropWSize>0;CropWSize--){
		for(uint32_t h=0;h<CropHSize;h++){
			if (OccupiedPixels[GetCoordinate((CropWSize-1),h,INPUT.Size.Width)]){
				goto WHit;
			}
		}}
		WHit:;
		if(CropWSize == 0){
			printf("TrimImage has trimmed the width to 0\n");
			exit(ExitCode::FILECORRUPTION);
		}
	}
	// check if there are rows or columns to trim
	uint32_t CropedSize = (CropWSize*CropHSize);
	if(CropedSize == 0){
		printf("croped sizes are invalid w:%i and h:%i\n",CropWSize,CropHSize);
		exit(ExitCode::FILECORRUPTION);
	}
	if(CropedSize == (INPUT.Size.Width*INPUT.Size.Height)){
		// crop unessesary
		printf("croping was not needed\n");
		return INPUT;
	}else{
		// crop nessesary
		printf("croping was needed as croped w:%i h:%i != input w:%i h:%i\n",
			CropWSize,CropHSize,
			INPUT.Size.Width,INPUT.Size.Height);
		OutputImage.Pixels.resize(CropedSize);
		// transpose canvas pixels onto trimmed output image
		for(uint32_t YPos = 0; YPos < CropHSize; YPos++) {
		for(uint32_t XPos = 0; XPos < CropWSize; XPos++) {
			NewPoint = GetCoordinate(XPos,YPos,CropWSize);
			OldPoint = GetCoordinate(XPos,YPos,INPUT.Size.Width);
			OutputImage.Pixels[NewPoint] = INPUT.Pixels[OldPoint];
		}}
		OutputImage.Size = {CropWSize,CropHSize};
	}

	OutputImage.Groups = INPUT.Groups;

	#ifdef USING_SANITYCHECKS
	{
		StomaImagePack::Image& TestTarget = OutputImage;
		if(SanityCheckImageData(TestTarget,"TrimImage Output")){
			exit(ExitCode::FILECORRUPTION);
		};
	}
	#endif

	return OutputImage;
}

StomaImagePack::Image
MergeImages(std::vector<StomaImagePack::Image> SUBIMAGES) {
	#ifdef USING_SANITYCHECKS
	{
		if(SUBIMAGES.size() == 0){
			printf("MergeImages recived 0 images\n");
			exit(ExitCode::FILECORRUPTION);
		}
		for(uint32_t im=0;im<SUBIMAGES.size();im++){
			if(SanityCheckImageData(SUBIMAGES[im],"MergeImages Input")){
				exit(ExitCode::FILECORRUPTION);
			};
		}
	}
	#endif
	StomaImagePack::Image OutputImage;
	OutputImage.Groups.resize(1); 
	std::vector<bool> Occupied;

	// TODO: Create a better merge algarythm
	#define MERGEALGORITHM_DUMBDUMPANDTRIM
	#ifdef MERGEALGORITHM_DUMBDUMPANDTRIM
	// NOTE: this method results in a ton of wasted space as it creates a strip of images being written
	// thus requiring an additional processing step

	// iterate over all glyphs and get the max x and y required
	for(uint32_t im = 0; im<SUBIMAGES.size();im++){
	for(uint32_t gr = 0; gr<SUBIMAGES[im].Groups.size();gr++){
	for(uint32_t gl = 0; gl<SUBIMAGES[im].Groups[gr].Glyphs.size();gl++){
		OutputImage.Size.Width  += SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Width;
		OutputImage.Size.Height += SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Height;
	}}}
	// create a canvas
	uint32_t CanvasSize = OutputImage.Size.Width*OutputImage.Size.Height;
	OutputImage.Pixels.resize(CanvasSize);
	Occupied.resize(CanvasSize);
	uint32_t CurrentGroup;
	
	bool DrawSafe;		
	// iterate over canvas every glyph and iterate over every position to find a good placement
	for (uint32_t im = 0;im < SUBIMAGES.size();im++) {
	for (uint32_t gr = 0;gr < SUBIMAGES[im].Groups.size();gr++) {
		{
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
					exit((uint32_t)ExitCode::STOMAIMAGEERROREXIT);
				}
			}
		}
	for (uint32_t gl = 0;gl < SUBIMAGES[im].Groups[gr].Glyphs.size();gl++) {
		// check if glyph is bogus
		if((SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Width*SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Width) != 0){
			// check if its safe to draw in position
			for(uint32_t MYPos = 0; MYPos < OutputImage.Size.Height; MYPos++) {
			for(uint32_t MXPos = 0; MXPos < OutputImage.Size.Width; MXPos++) {
				// test if first pixel is empty
				if (Occupied[GetCoordinate(MXPos,MYPos,OutputImage.Size.Width)] == false) {
					DrawSafe = true;
					// check if position collides with previously written positions
					for(uint32_t SYPos=0;SYPos<SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Height;SYPos++){
					for(uint32_t SXPos=0;SXPos<SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Width ;SXPos++){
						if(Occupied[GetCoordinate(MXPos + SXPos,MYPos + SYPos,OutputImage.Size.Width)] == true){
							DrawSafe = false;
							goto AlreadyWrittenIn;
						}
					}}
					if (DrawSafe){
						// draw
						for(uint32_t SYPos=0;SYPos<SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Height;SYPos++){
						for(uint32_t SXPos=0;SXPos<SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Width ;SXPos++){
							uint32_t MPoint = GetCoordinate(MXPos + SXPos,MYPos + SYPos,OutputImage.Size.Width);
							uint32_t SPoint = GetCoordinate(SXPos, SYPos, SUBIMAGES[im].Groups[gr].Glyphs[gl].Size.Width);
							OutputImage.Pixels[MPoint] = SUBIMAGES[im].Pixels[SPoint];
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
				AlreadyWrittenIn:;
			}}
		}
	// endof glyph
	NextGlyph:;
	}
	// endof glyphs
	}
	// endof group
	}
	// endof image
	#endif
	// clang-format off
	
	#ifdef USING_SANITYCHECKS
	{
		if(SanityCheckImageData(OutputImage,"MergeImages Input")){
			exit(ExitCode::FILECORRUPTION);
		};
	}
	#endif
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
void WriteOut(StomaImagePack::Image IMG, std::string NAM) {
	// BUG: running 'test font reading' test crashes before getting here
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
			exit((uint32_t)ExitCode::FREETYPEERROREXIT);				
		}
		InitdFreeType = true;
	}
	std::string loc = NAM + ".ttf";
	// --- load in fonts as desired size
	err = FT_New_Face(FontLibrary,loc.c_str(),0,&TypeFace);
	if(err != 0){
		printf("freetype 'FT_New_Face' returned error %i\n",err);
		exit((uint32_t)ExitCode::FREETYPEERROREXIT);
	}
	err = FT_Set_Pixel_Sizes(TypeFace,DesiredWidth,DesiredHeight);
	if(err != 0){
		printf("freetype 'FT_Set_Pixel_Sizes' returned error %i\n",err);
		exit((uint32_t)ExitCode::FREETYPEERROREXIT);
	}
	// --- prepare fontgroup
	ReturnImage.Groups.resize(1);
	ReturnImage.Groups[0].Name = NAM;
	ReturnImage.Groups[0].Type = StomaImagePack::GroupType::FONT;
	ReturnImage.Groups[0].Glyphs.resize(DesiredGlyphCount);
	// iterate to get size data for the glyphs to construct pixel buffer and glyphdata

	for(uint32_t gl = 0;gl<DesiredGlyphCount;gl++){
		CurrentChar = gl+32;
		err = FT_Load_Char(TypeFace,CurrentChar,FT_LOAD_RENDER|FT_LOAD_TARGET_(FT_RENDER_MODE_SDF));
		if(err != 0){
			printf("freetype 'FT_Load_Char' returned error %i\n",err);
			exit((uint32_t)ExitCode::FREETYPEERROREXIT);
		}
		err = FT_Render_Glyph(TypeFace->glyph,FT_RENDER_MODE_NORMAL);
		if(err != 0){
			printf("freetype 'FT_Render_Glyph' returned error %i\n",err);
			exit((uint32_t)ExitCode::FREETYPEERROREXIT);
		}
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
		if(err != 0){
			printf("freetype 'FT_Load_Char' returned error %i\n",err);
			exit((uint32_t)ExitCode::FREETYPEERROREXIT);
		}
		err = FT_Render_Glyph(TypeFace->glyph,FT_RENDER_MODE_NORMAL);
		if(err != 0){
			printf("freetype 'FT_Render_Glyph' returned error %i\n",err);
			exit((uint32_t)ExitCode::FREETYPEERROREXIT);
		}
		if((TypeFace->glyph->bitmap.width*TypeFace->glyph->bitmap.rows) > 0){
			// --- sanity check that the buffer exists
			if(TypeFace->glyph->bitmap.buffer == nullptr){
				printf("returned charbuffer is nullptr");
				exit((uint32_t)ExitCode::FREETYPEERROREXIT);
			}
			// --- check if size is fucked
			if(TypeFace->glyph->bitmap.width != ReturnImage.Groups[0].Glyphs[gl].Size.Width
			|| TypeFace->glyph->bitmap.rows != ReturnImage.Groups[0].Glyphs[gl].Size.Height){
				printf("glyph %i (aka %c)'s size is different from its previous size, pw:%i ph:%i != cw:%i ch:%i\n",
					gl,CurrentChar,
					TypeFace->glyph->bitmap.width,
					TypeFace->glyph->bitmap.rows,
					ReturnImage.Groups[0].Glyphs[gl].Size.Width,
					ReturnImage.Groups[0].Glyphs[gl].Size.Height);
				exit((uint32_t)ExitCode::FREETYPEERROREXIT);
			}
			if((TypeFace->glyph->bitmap.rows + ReturnImage.Groups[0].Glyphs[gl].Offset.Height) > ReturnImage.Size.Height){
				printf("glyph %i (aka %c)'s height is greater than the max size, off:%i + hig:%i > max:%i\n",
					gl,CurrentChar,
					TypeFace->glyph->bitmap.rows,
					ReturnImage.Groups[0].Glyphs[gl].Offset.Height,
					ReturnImage.Size.Height);
				exit((uint32_t)ExitCode::FREETYPEERROREXIT);
			}
			if((TypeFace->glyph->bitmap.width + ReturnImage.Groups[0].Glyphs[gl].Offset.Width) > ReturnImage.Size.Width){
				printf("glyph %i (aka %c)'s width is greater than the max size, off:%i + wid:%i > max:%i\n",
		   			gl,CurrentChar,
					TypeFace->glyph->bitmap.width,
					ReturnImage.Groups[0].Glyphs[gl].Offset.Width,
					ReturnImage.Size.Width);
				exit((uint32_t)ExitCode::FREETYPEERROREXIT);
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

	#ifdef USING_SANITYCHECKS
	{
		StomaImagePack::Image& TestTarget = ReturnImage;
		if(SanityCheckImageData(TestTarget,"Read_ttf Output")){
			exit(ExitCode::FILECORRUPTION);
		};
	}
	#endif
	
	return ReturnImage;
}
// -------------------------------------PNG++
#ifdef USING_PNGPP
StomaImagePack::Image Read_png(std::string NAM,StomaImagePack::GroupType TYPE) {
	// assumes .png has been trimmed off
	// NOTE: using pngpp as libpng is a classic needlessly complex library
	StomaImagePack::Image ReturnImage;
	std::string SourceDir;
	std::string GlyphName;
	uint32_t HoldPosition;
	
	std::string FileName = NAM+".png";
	printf("'FileName' = %s\n",FileName.c_str());
	{
		png::rgba_pixel HoldPixel;
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
		}}
	}
	#ifdef USING_SANITYCHECKS
	{
		StomaImagePack::Image& TestTarget = ReturnImage;
		if(SanityCheckImageData(TestTarget,"Read_png Output")){
			exit(ExitCode::FILECORRUPTION);
		};
	}
	#endif
	return ReturnImage;// returns read image 
}
void Write_png(StomaImagePack::Image IMG, std::string NAM) {
	#ifdef USING_SANITYCHECKS
	{
		StomaImagePack::Image& TestTarget = IMG;
		if(SanityCheckImageData(TestTarget,"Write_png Input")){
			exit(ExitCode::FILECORRUPTION);
		};
	}
	#endif
	{
		png::image<png::rgba_pixel> WriteFile;
		if(WriteFile.get_width() != IMG.Size.Width
		|| WriteFile.get_height()!= IMG.Size.Height){
			WriteFile.resize(IMG.Size.Width, IMG.Size.Height);
		}
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
		}}
		WriteFile.write(NAM + ".png");
	}
}
#endif
#ifdef USING_PNGHANDROLLED
StomaImagePack::Image Read_png(std::string NAM,StomaImagePack::GroupType TYPE) {
	// assumes .png has been trimmed off
	// NOTE: this was created to holdout while pngpp was having glibc error exits


	StomaImagePack::Image ReturnImage;
	std::string SourceDir;
	std::string GlyphName;
	
	std::string FileName = NAM+".png";
	printf("'FileName' = %s\n",FileName.c_str());
	// reads entire file to char array
	std::ifstream ReadFile;
	std::vector<uint8_t> CharVector;
	char HoldChar;
	uint32_t CurrentPosition;

	// IHDR info
	bool IHDRTaken = false;
	uint8_t BitsPerColourChannel = 0;
	uint8_t ColourType = 0;
	uint8_t Compression = 0;
	uint8_t Filter = 0;
	uint8_t Interlace = 0;
	uint32_t SizeWidth = 0;
	uint32_t SizeHeight = 0;
	// PLTE info
	std::vector<uint8_t> ColourData;
	// IDAT info
	std::vector<uint8_t> ImageData = {};



	ReadFile.open(FileName,std::ios::binary);
	if(!ReadFile.is_open()) {
		printf("could not read %s.\n", NAM.c_str());
		exit(1);
	}
	while(ReadFile.get(HoldChar)) {
		CharVector.push_back(HoldChar);
	}
	if(!ReadFile.eof()){exit(10);}
	ReadFile.close();
	// get file signiture
	CurrentPosition = 0;
	{
		uint8_t Desired[8]{
		0x89,
		0x50,
		0x4E,
		0x47,
		0x0D,
		0x0A,
		0x1A,
		0x0A
		};
		uint8_t Signature[8];
		for (uint32_t i = 0;i<8;i++) {
			Signature[i] = CharVector[CurrentPosition];CurrentPosition++;
		}
		uint64_t a = *Signature;
		uint64_t b = *Desired;
		if(a!=b){
			printf("file %s is not a valid png as signiture (%lu) != desired (%lu) \n",FileName.c_str(),a,b);
			exit(ExitCode::PNGREADERROREXIT);
		}
	}
	{
		// iterate over chunks until eof flag or end of charvector 
		bool FileEndHit = false;
		uint8_t* CharVoodoo;
		uint32_t ChunkLength;
		char ChunkType[5];
		std::vector<uint8_t> ChunkData;
		uint8_t ChunkCRC[5];

		ChunkType[4] = 0;
		ChunkCRC[4] = 0;
		// NOTE: pngs are big endian and are thus read "left to right"
		while(FileEndHit == false){
			ChunkData.resize(0);
			// read length // 4 bytes
			// NOTE: these are backwards... nothing else is... but this is
			uint8_t RawChunkLength[4];
			RawChunkLength[0] = CharVector[CurrentPosition+3];
			RawChunkLength[1] = CharVector[CurrentPosition+2];
			RawChunkLength[2] = CharVector[CurrentPosition+1];
			RawChunkLength[3] = CharVector[CurrentPosition+0];
			CurrentPosition+=4;
			uint32_t* intermediate = (uint32_t*)RawChunkLength;
			ChunkLength = *intermediate;
			// read type // 4 bytes
			for (uint32_t i = 0;i<4;i++) {ChunkType[i] = CharVector[CurrentPosition];CurrentPosition++;}
			// read chunk data
			ChunkData.resize(ChunkLength);
			if(ChunkLength>CharVector.size()){
				printf("ChunkLen for %s in %s is %i out of CharVectors size of %i\n",ChunkType,FileName.c_str(),ChunkLength,(uint32_t)CharVector.size());
				exit(ExitCode::PNGREADERROREXIT);
			}
			for(uint32_t i = 0;i<ChunkLength;i++){
				ChunkData[i] = CharVector[CurrentPosition];CurrentPosition++;
			}
			// read crc
			for (uint32_t i = 0;i<4;i++) {
				ChunkCRC[i] = CharVector[CurrentPosition];CurrentPosition++;
			}
			// handle chunk
			{
				// data chunk aka IHDR
				if(ChunkType[0] == 'I'
				&& ChunkType[1] == 'H'
				&& ChunkType[2] == 'D'
				&& ChunkType[3] == 'R'){
					if(IHDRTaken == true){goto NextChunk;}
					// datachunk already taken and we only read the first image
					IHDRTaken = true;
					uint32_t datapos = 0;
					// get width
					// these are read backwards due to cancer
					CharVoodoo = (uint8_t*)&SizeWidth;
					CharVoodoo[3] = ChunkData[datapos+0];
					CharVoodoo[2] = ChunkData[datapos+1];
					CharVoodoo[1] = ChunkData[datapos+2];
					CharVoodoo[0] = ChunkData[datapos+3];
					datapos+=4;
					// get height
					CharVoodoo = (uint8_t*)&SizeHeight;
					CharVoodoo[3] = ChunkData[datapos+0];
					CharVoodoo[2] = ChunkData[datapos+1];
					CharVoodoo[1] = ChunkData[datapos+2];
					CharVoodoo[0] = ChunkData[datapos+3];
					datapos+=4;
					// get bit depth
					CharVoodoo = (uint8_t*)&BitsPerColourChannel;
					for(uint32_t i = 0;i<1;i++){
						CharVoodoo[i] = ChunkData[datapos];datapos++;
					}
					// get colour type
					CharVoodoo = (uint8_t*)&ColourType;
					for(uint32_t i = 0;i<1;i++){
						CharVoodoo[i] = ChunkData[datapos];datapos++;
					}
					// get Compression
					CharVoodoo = (uint8_t*)&Compression;
					for(uint32_t i = 0;i<1;i++){
						CharVoodoo[i] = ChunkData[datapos];datapos++;
					}
					// get Filter
					CharVoodoo = (uint8_t*)&Filter;
					for(uint32_t i = 0;i<1;i++){
						CharVoodoo[i] = ChunkData[datapos];datapos++;
					}
					// get interlace
					CharVoodoo = (uint8_t*)&Interlace;
					for(uint32_t i = 0;i<1;i++){
						CharVoodoo[i] = ChunkData[datapos];datapos++;
					}
					goto NextChunk;
				}
				// palete chunk aka PLTE			
				if(ChunkType[0] == 'P'
				&& ChunkType[1] == 'L'
				&& ChunkType[2] == 'T'
				&& ChunkType[3] == 'E'){
					ColourData = ChunkData;
					goto NextChunk;				
				}
				// image data aka IDAT
				if(ChunkType[0] == 'I'
				&& ChunkType[1] == 'D'
				&& ChunkType[2] == 'A'
				&& ChunkType[3] == 'T'){
					// NOTE: as multiple IDAT chunks can exist we shall append data to previous data
					for(uint32_t i = 0;i<ChunkLength;i++){
						ImageData.push_back(ChunkData[i]);
					}
					goto NextChunk;
				}
				// file end chunk aka IEND
				if(ChunkType[0] == 'I'
				&& ChunkType[1] == 'E'
				&& ChunkType[2] == 'N'
				&& ChunkType[3] == 'D'){
					FileEndHit = true;
					goto NextChunk;
				
				}
				// TODO: add code to handle optional chunks

				//
				NextChunk:;
				// endof handling this chunk
			}		
		}
		ReturnImage.Groups.resize(1);
		ReturnImage.Size = {SizeWidth,SizeHeight};
		ReturnImage.Groups[0].Name = NAM;
		ReturnImage.Groups[0].Type = StomaImagePack::GroupType::REGULAR;
		ReturnImage.Groups[0].Glyphs.resize(1);
		ReturnImage.Groups[0].Glyphs[0].Name = NAM;
		ReturnImage.Groups[0].Glyphs[0].Offset = {0,0};
		ReturnImage.Groups[0].Glyphs[0].Size = {SizeWidth,SizeHeight};
	
		// handle data
		/*
		Color type			Channels	Bits per channel
										1	2	4	8	16
		Indexed				1			1	2	4	8	
		Grayscale			1			1	2	4	8	16
		Grayscale and alpha	2						16	32
		Truecolor			3						24	48
		Truecolor and alpha	4						32	64
		*/
		
		uint8_t ColourStep;
		switch (ColourType) {
			// rgb = 2
			// rgba = 6
			case(2):{
				ColourStep = 3;
				goto HandleRGB;
			}
			case(6):{
				ColourStep = 4;
				HandleRGB:;
				uint32_t current = 0;
				uint32_t PixelCount = ReturnImage.Size.Width*ReturnImage.Size.Height;
				ReturnImage.Pixels = {};
				if(BitsPerColourChannel == 16){
					PixelCount*=2;
					// doubles the target to allow for skipping minors
					// so for BPCC of 16 is
					//    major	   minor
					// [uint64_t][uint64_t]
					// but BPCC of 8 is 
					//    major
					// [uint64_t]
					// as uint64_t is 8 bytes
				}
				// colours are floats
				StomaImagePack::Colour TransposingColour;
				if(PixelCount>ColourData.size()){
					printf("target w:%i h:%i (%i) greater than provided colour data count(%i)\n",
					SizeWidth,SizeHeight,PixelCount,
					(uint32_t)ColourData.size());
					exit(ExitCode::PNGREADERROREXIT);
				}
				while(current<PixelCount){
					TransposingColour.R = ColourData[(current*ColourStep)+0];
					TransposingColour.G = ColourData[(current*ColourStep)+1];
					TransposingColour.B = ColourData[(current*ColourStep)+2];
					if(ColourStep == 4){TransposingColour.A = ColourData[(current*ColourStep)+3];}
					else{TransposingColour.A = -1;}
					ReturnImage.Pixels.push_back(TransposingColour);
					if(BitsPerColourChannel == 16){current+=2;}else{current++;}
				}
				
				break;
			};

			// greyscale
			case(0):{};
			// greyscale + alpha
			case(4):{};

			// indexed pallete
			case(3):{};

			default:{
				printf("png has an invalid colour type of %i \n",ColourType);
				exit(ExitCode::PNGREADERROREXIT);
			};

		}
	}
	
	return ReturnImage; 
}
//
void Write_png(StomaImagePack::Image IMG, std::string NAM) {
	// TODO: do this properly and generate a real crc instead of creating bogus ones
	if((IMG.Size.Width*IMG.Size.Height) != (uint32_t)IMG.Pixels.size()){
		printf("write_png has recived a StomaImagePack::Image with size corruption w:%i,h:%i > pc:%i\n",
		IMG.Size.Width,IMG.Size.Height,(uint32_t)IMG.Pixels.size());
		exit(ExitCode::FILECORRUPTION);
	}
	// create charbuffer with header
	std::vector<uint8_t> CharBuffer{
		0x89,0x50,0x4E,0x47,
		0x0D,0x0A,0x1A,0x0A};
	uint8_t* CharVoodoo;
	uint32_t Length;
	// write info chunk
	{
		// length
		// 13 for IHDR
		Length = 13;
		CharVoodoo = (uint8_t*)&Length;
		for(uint32_t i=0;i<4;i++){CharBuffer.push_back(CharVoodoo[i]);}
		// type
		CharBuffer.push_back('I');
		CharBuffer.push_back('H');
		CharBuffer.push_back('D');
		CharBuffer.push_back('R');
		// data
		CharVoodoo = (uint8_t*)&IMG.Size.Width;
		for(uint32_t i=0;i<4;i++){CharBuffer.push_back(CharVoodoo[i]);}
		CharVoodoo = (uint8_t*)&IMG.Size.Height;
		for(uint32_t i=0;i<4;i++){CharBuffer.push_back(CharVoodoo[i]);}
		CharBuffer.push_back(8); // always write 8 bitdepth
		CharBuffer.push_back(6); // always write RGBA
		CharBuffer.push_back(0); // always no compression 
		CharBuffer.push_back(0); // always no filter
		CharBuffer.push_back(0); // always no interlace
		// CRC
		for(uint32_t i=0;i<4;i++){CharBuffer.push_back(0);}
	}
	// write data chunk
	{
		uint32_t t_pixelcount = (IMG.Size.Width*IMG.Size.Height);
		uint32_t t_pixelements = 4;// 4 as rgba 
		uint32_t t_charperelement = 1;// 1 as using uint8_t for colours 
		// length
		Length = t_pixelcount*t_pixelements*t_charperelement;
		CharVoodoo = (uint8_t*)&Length;
		for(uint32_t i=0;i<4;i++){CharBuffer.push_back(CharVoodoo[i]);}
		// type
		CharBuffer.push_back('I');
		CharBuffer.push_back('D');
		CharBuffer.push_back('A');
		CharBuffer.push_back('T');
		// data
		for (uint32_t i = 0;i<t_pixelcount;i++) {
			CharBuffer.push_back(IMG.Pixels[i].R);
			CharBuffer.push_back(IMG.Pixels[i].G);
			CharBuffer.push_back(IMG.Pixels[i].B);
			CharBuffer.push_back(IMG.Pixels[i].A);
		}
		// CRC
		for(uint32_t i=0;i<4;i++){CharBuffer.push_back(0);}
	}
	// write end chunk
	{
		// length
		// NOTE: end always has 0 length
		Length = 0;
		CharVoodoo = (uint8_t*)&Length;
		for(uint32_t i=0;i<4;i++){CharBuffer.push_back(CharVoodoo[i]);}
		// type
		CharBuffer.push_back('I');
		CharBuffer.push_back('H');
		CharBuffer.push_back('D');
		CharBuffer.push_back('R');
		// data
		// NOTE: end always has 0 data
		// CRC
		for(uint32_t i=0;i<4;i++){CharBuffer.push_back(0);}
	}
	// open file
	std::ofstream WriteFile(NAM + ".png",std::ios::binary);
	// write charbuffer
	WriteFile.write((char*)CharBuffer.data(),CharBuffer.size());
	// close file
	WriteFile.close();
	
	

	//
}
#endif
