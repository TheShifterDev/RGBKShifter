#ifndef RGBKS_HEAD_INCLUDE_BARRIER
#define RGBKS_HEAD_INCLUDE_BARRIER
#include <cstdint>
#include <fstream>
#include <png++/png.hpp> // local install via pacman
#include <string>
#include <iostream>
#include <vector>

namespace RGBKS {

enum class StimpacVer{
	V1,
	/*
	stimpac spec for V1
	Version						| uint8
	MasterImage Resolution		| 1
		Width					| uint32
		Height					| uint32
	RGBA Pixel Array			| (MasterImage width * height)
		Red						| uint8
		Green					| uint8
		Blue					| uint8
		Alpha					| uint8
	MasterImage GlyphCount		| uint32
	Glyph Array					| GlyphCount
		Name					| char*64
		Size					| 1
			Width				| uint32
			Height				| uint32
		Position				| 1
			Width				| uint32
			Height				| uint32
	*/
	ENDOF
};

struct Resolution {
	uint32_t Width = 0;
	uint32_t Height = 0;
};
struct Colour {
	uint8_t R = 0;
	uint8_t G = 0;
	uint8_t B = 0;
	uint8_t A = 0;
};
struct Glyph {
	std::string Name = "Unnamed";
	Resolution Size = {64, 64};
	Resolution Position = {0, 0};
};
struct Image {
	Resolution Size = {64, 64};
	std::vector<Colour> Pixels{};
	std::vector<Glyph> Glyphs{};
};

// NOTE: NEVER USE MACROS LIKE 
// >>> #define GETCORD(X, Y, XMAX) (X + (Y * XMAX)) <<<
// AS NOT INCLUDING '('&')' IN
// "GETCORD((MainXPos + SubXPos), (MainYPos + SubYPos),MainImage.Size.Width)"
// CAUSED A BUG THAT TOOK DAYS TO TRACK
std::vector<Image> SeperateGlyphs(std::vector<Image> IMG);
std::vector<Image> ReorderByVolume(std::vector<Image> IMG);
uint32_t GetCoordinate(uint32_t XPOS,uint32_t YPOS,uint32_t MAXX);
float GetRGBColourDistance(Colour A, Colour B);
void PalletiseImage(Image &IMG, std::vector<Colour> PAL);
Image MergeImages(std::vector<Image> IMG);
std::vector<Colour> ExtractPallet_Image(Image IMG);
Image Read_png(std::string NAM);
void Write_png(Image IMG, std::string NAM);
Image Read_stimpac(std::string NAM);
void Write_stimpac(Image IMG, std::string NAM);

} // namespace RGBKS

#endif // RGBKS_HEAD_INCLUDE_BARRIER

##ifdef RGBKS_IMPLEM
#ifndef RGBKS_BODY_INCLUDE_BARRIER
#define RGBKS_BODY_INCLUDE_BARRIER

namespace RGBKS {
void Write_stimpac(Image IMG, std::string NAM) {
	std::vector<uint8_t> CharVector;
	uint8_t ImageResolution[8];
	uint8_t GlyphCount[4];
	// Version
	CharVector.push_back(((uint8_t)StimpacVer::V1));
	// ImageResolution
	ImageResolution[0] = *(uint8_t*)IMG.Size.Width;
	ImageResolution[4] = *(uint8_t*)IMG.Size.Height;
	for (uint32_t i=0;i<8;i++){
		CharVector.push_back(ImageResolution[i]);
	}
	// Pixels
	for(uint32_t i=0;i<IMG.Pixels.size();i++){
		CharVector.push_back(IMG.Pixels[i].R);
		CharVector.push_back(IMG.Pixels[i].G);
		CharVector.push_back(IMG.Pixels[i].B);
		CharVector.push_back(IMG.Pixels[i].A);
	}
	// Glyph Count
	GlyphCount[0] = IMG.Glyphs.size();
	for(uint32_t i=0;i<4;i++){
		CharVector.push_back(GlyphCount[i]);
	}
	// Glyph
	for(uint32_t q=0;q<IMG.Glyphs.size();q++){
		// Glyph Name
		uint32_t UsedChars = 0;
		if(IMG.Glyphs[q].Name.length()>64){UsedChars = 64;}
		else{UsedChars = IMG.Glyphs[q].Name.length();}
		for(uint32_t i=0;i<UsedChars;i++){
			CharVector.push_back(IMG.Glyphs[q].Name[i]);
		}
		for(uint32_t i=UsedChars;i<64;i++){
			CharVector.push_back(0);
		}
		// Glyph Size
		for(uint32_t i=0;i<8;i++){
			ImageResolution[i] = *(uint8_t*)IMG.Glyphs[q].Size[i];
		}
		//ImageResolution[0] = *(uint8_t*)IMG.Glyphs[q].Size.Width;
		//ImageResolution[4] = *(uint8_t*)IMG.Glyphs[q].Size.Height;
		for (uint32_t i=0;i<8;i++){
			CharVector.push_back(ImageResolution[i]);
		}
		// Glyph Position
		for(uint32_t i=0;i<8;i++){
			ImageResolution[i] = *(uint8_t*)IMG.Glyphs[q].Size[i];
		}
		//ImageResolution[0] = *(uint8_t*)IMG.Glyphs[q].Position.Width;
		//ImageResolution[4] = *(uint8_t*)IMG.Glyphs[q].Position.Height;
		for (uint32_t i=0;i<8;i++){
			CharVector.push_back(ImageResolution[i]);
		}
	}
	
	// write char vector to disk
	std::ofstream WriteFile(NAM + ".stimpac", std::ios::out);
	for(uint32_t i=0;i<CharVector.size();i++){
		WriteFile << CharVector[i];
	}
	WriteFile.close();
}
Image Read_stimpac(std::string NAM) {
	Image OutputImage;
	StimpacVer CurrentVersion;
	std::ifstream t_fil;
	std::vector<uint8_t> CharVector;
	char HoldChar;
	uint32_t CurrentPosition = 0;
	uint8_t ImageResolution[8];
	Colour HoldColour;
	Glyph r_glyph;
	uint8_t GlyphCount[4];
	
	// read file into char vector
	t_fil.open(NAM + ".stimpac");
	if(!t_fil.is_open()) {
		printf("could not read %s.\n", NAM.c_str());
		exit(1);
	}
	while(t_fil.good()) {
		t_fil >> HoldChar;
		CharVector.push_back(HoldChar);
	}
	// Version
	CurrentVersion = (StimpacVer)CharVector[0];
	CurrentPosition=1;
	// Resolution
	for(uint32_t i=0;i<8;i++) {ImageResolution[i] = CharVector[CurrentPosition+i];}
	OutputImage.Size.Width		= *(uint32_t*)ImageResolution[0];
	OutputImage.Size.Height		= *(uint32_t*)ImageResolution[4];
	CurrentPosition+=8;
	// Pixels
	OutputImage.Pixels.resize(OutputImage.Size.Width * OutputImage.Size.Height);
	for(uint32_t i=0;i<OutputImage.Pixels.size();i++) {
		HoldColour.R = CharVector[CurrentPosition + 0 + (i * 4)];
		HoldColour.G = CharVector[CurrentPosition + 1 + (i * 4)];
		HoldColour.B = CharVector[CurrentPosition + 2 + (i * 4)];
		HoldColour.A = CharVector[CurrentPosition + 3 + (i * 4)];
		OutputImage.Pixels[i] = HoldColour;
		CurrentPosition+=4;
	}

	// Glyph Count
	for(uint32_t i=0;i<4;i++) {GlyphCount[i] = CharVector[CurrentPosition+i];}
	OutputImage.Glyphs.resize(*(uint32_t*)GlyphCount[0]);
	CurrentPosition+=4;
	// Glyph
	for (uint32_t q=0;q<OutputImage.Glyphs.size();q++) {
		// Name
		for (uint32_t i=0;i<64;i++) {
			OutputImage.Glyphs[q].Name[i] = CharVector[CurrentPosition+i];
		}
		CurrentPosition+=64;
		// Size
		for(uint32_t i=0;i<8;i++) {ImageResolution[i] = CharVector[CurrentPosition+i];}
		OutputImage.Glyphs[q].Size.Width	= *(uint32_t*)ImageResolution[0];
		OutputImage.Glyphs[q].Size.Height	= *(uint32_t*)ImageResolution[4];
		CurrentPosition+=8;
		// Position
		for(uint32_t i=0;i<8;i++) {ImageResolution[i] = CharVector[CurrentPosition+i];}
		OutputImage.Glyphs[q].Position.Width	= *(uint32_t*)ImageResolution[0];
		OutputImage.Glyphs[q].Position.Height	= *(uint32_t*)ImageResolution[4];
		CurrentPosition+=8;
	}
	return OutputImage;
}
std::vector<Image> SeperateGlyphs(std::vector<Image> IMG){
	std::vector<Image> OutputImages;
	Image HoldingImage;
	for(uint32_t i=0;i<IMG.size();i++){
	for(uint32_t q=0;q<IMG[i].Glyphs.size();q++){
		HoldingImage.Size = IMG[i].Glyphs[q].Size;
		HoldingImage.Glyphs.resize(1);
		HoldingImage.Glyphs[0].Position = Resolution{0,0}; 
		HoldingImage.Glyphs[0].Size = IMG[i].Glyphs[q].Size; 
		HoldingImage.Pixels.resize(IMG[i].Glyphs[q].Size.Width*IMG[i].Glyphs[q].Size.Height);
		for(uint32_t h=0;h<IMG[i].Glyphs[q].Size.Height;h++){
		for(uint32_t w=0;w<IMG[i].Glyphs[q].Size.Width;w++){
			HoldingImage.Pixels[GetCoordinate(
				w,
				h,
				IMG[i].Glyphs[q].Size.Width
				)] = IMG[i].Pixels[GetCoordinate(
					IMG[i].Glyphs[q].Position.Width+w,
					IMG[i].Glyphs[q].Position.Height+h,
					IMG[i].Size.Width)];
		}}
		OutputImages.push_back(HoldingImage);
	}}


	return OutputImages;
}
Image MergeImages(std::vector<Image> SUBIMAGES) {
	// clang-format off
	// ---------------------SET_SIZE------------------------------------//
	std::vector<Image> OrderedSubImages;
	OrderedSubImages = ReorderByVolume(SUBIMAGES);
	std::vector<Resolution> ImageResolutions(SUBIMAGES.size());
	// get total required volume

	Resolution ExpectedSize = 	{0,0};
	Resolution MaxSize = 		{0,0};
	for(uint32_t i = 0; i < OrderedSubImages.size(); i++) {
		ImageResolutions[i] = OrderedSubImages[i].Size;
		MaxSize.Width  += OrderedSubImages[i].Size.Width;
		MaxSize.Height += OrderedSubImages[i].Size.Height;
	}
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
	
	Image MainImage;
	MainImage.Size = ExpectedSize;
	uint32_t hold = ExpectedSize.Width*ExpectedSize.Height;
	MainImage.Pixels.resize(hold);
	std::vector<bool> OccupiedPositions(hold);
	// ---------------------------DRAWING--------------------------------//
	bool DrawSafe;
	bool DrawDone;
	uint32_t MPoint;
	uint32_t SPoint;
	uint32_t CheckLimitX;
	uint32_t CheckLimitY;
	uint32_t MYPos = 0;
	uint32_t MXPos = 0;
	uint32_t SYPos = 0;
	uint32_t SXPos = 0;
	uint32_t CurrentShape = 0;
	uint32_t CurrentGlyphCount = 0;
	uint32_t TargetGlyphCount = 0;
	uint32_t CopyGlyphID = 0;
	// place shapes
	for(CurrentShape = 0; CurrentShape < OrderedSubImages.size(); CurrentShape++) {
		DrawDone = false;
		CheckLimitY = (MainImage.Size.Height-(OrderedSubImages[CurrentShape].Size.Height-1));
		CheckLimitX = (MainImage.Size.Width -(OrderedSubImages[CurrentShape].Size.Width -1));
		for(MYPos = 0; MYPos < CheckLimitY; MYPos++) {
		for(MXPos = 0; MXPos < CheckLimitX; MXPos++) {
			if (OccupiedPositions[GetCoordinate(MXPos, MYPos, MainImage.Size.Width)] == false) {
				DrawSafe = true;
				// check if position collides with previously written positions
				for(SYPos=0;SYPos<OrderedSubImages[CurrentShape].Size.Height;SYPos++){
				for(SXPos=0;SXPos<OrderedSubImages[CurrentShape].Size.Width ;SXPos++){
					if(OccupiedPositions[GetCoordinate(MXPos + SXPos,MYPos + SYPos,MainImage.Size.Width)] == true){
						DrawSafe = false;
						goto AlreadyWrittenBreakout;
					}
				}}
				AlreadyWrittenBreakout:;
				if (DrawSafe){
					// draw
					for(SYPos=0;SYPos<OrderedSubImages[CurrentShape].Size.Height;SYPos++){
					for(SXPos=0;SXPos<OrderedSubImages[CurrentShape].Size.Width ;SXPos++){
						MPoint = GetCoordinate(MXPos + SXPos,MYPos + SYPos,MainImage.Size.Width);
						SPoint = GetCoordinate(SXPos, SYPos, OrderedSubImages[CurrentShape].Size.Width);
						MainImage.Pixels[MPoint] = OrderedSubImages[CurrentShape].Pixels[SPoint];
						OccupiedPositions[MPoint] = true;
					}}
					// place glyphs
					CurrentGlyphCount = MainImage.Glyphs.size();
					TargetGlyphCount = CurrentGlyphCount+OrderedSubImages[CurrentShape].Glyphs.size()-1;
					CopyGlyphID = 0;
					MainImage.Glyphs.resize(TargetGlyphCount);
					while(CurrentGlyphCount < TargetGlyphCount){
						MainImage.Glyphs[CurrentGlyphCount].Name 			= OrderedSubImages[CurrentShape].Glyphs[CopyGlyphID].Name;
						MainImage.Glyphs[CurrentGlyphCount].Position.Width 	= OrderedSubImages[CurrentShape].Glyphs[CopyGlyphID].Position.Width +MXPos;
						MainImage.Glyphs[CurrentGlyphCount].Position.Height = OrderedSubImages[CurrentShape].Glyphs[CopyGlyphID].Position.Height+MYPos;
						MainImage.Glyphs[CurrentGlyphCount].Size 			= OrderedSubImages[CurrentShape].Glyphs[CopyGlyphID].Size;
						CopyGlyphID++;
						CurrentGlyphCount++;
					}
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
	ExpectedSize = MainImage.Size;
	// get true used Heigth/Rows
	while (ExpectedSize.Height > 1) {
		for(uint32_t w=0;w<ExpectedSize.Width;w++){
			if (OccupiedPositions[GetCoordinate(w, ExpectedSize.Height-1, MainImage.Size.Width)]){
				goto PixelRowOccupiedBreakout;
			}
		}
		ExpectedSize.Height--;
	}
	PixelRowOccupiedBreakout:;
	// get true used Width/Columns
	while (ExpectedSize.Width > 1) {
		for(uint32_t h=0;h<ExpectedSize.Height;h++){
			if (OccupiedPositions[GetCoordinate(ExpectedSize.Width-1 , h, MainImage.Size.Width)]){
				goto PixelColumnOccupiedBreakout;
			}
		}
		ExpectedSize.Width--;
	}
	PixelColumnOccupiedBreakout:;
	// check if there are rows or columns to trim
	if(ExpectedSize.Width  != MainImage.Size.Width 
	|| ExpectedSize.Height != MainImage.Size.Height){
		Image CopyImage = MainImage;
		MainImage.Size = ExpectedSize;
		MainImage.Pixels.resize(ExpectedSize.Width*ExpectedSize.Height);
		for(MYPos = 0; MYPos < CheckLimitY; MYPos++) {
		for(MXPos = 0; MXPos < CheckLimitX; MXPos++) {
			MPoint = GetCoordinate(MXPos,MYPos,MainImage.Size.Width);
			SPoint = GetCoordinate(MXPos,MYPos,CopyImage.Size.Width);
			MainImage.Pixels[MPoint] = CopyImage.Pixels[SPoint];
		}}
	}
	return MainImage;
	// clang-format on
}
float GetRGBColourDistance(Colour A, Colour B) {
	float t_ret;
	float t_px, t_py, t_pz;
	float t_cA[3];
	float t_cB[3];
	t_cA[0] = (float)A.R;
	t_cA[1] = (float)A.G;
	t_cA[2] = (float)A.B;
	t_cB[0] = (float)B.R;
	t_cB[1] = (float)B.G;
	t_cB[2] = (float)B.B;
	t_px = ((t_cA[0] - t_cB[0]) * (t_cA[0] - t_cB[0]));
	t_py = ((t_cA[1] - t_cB[1]) * (t_cA[1] - t_cB[1]));
	t_pz = ((t_cA[2] - t_cB[2]) * (t_cA[2] - t_cB[2]));
	t_ret = (t_px + t_py + t_pz) / 2;
	if(t_ret < 0) {
		return -t_ret;
	} // shouldnt be needed but whatever
	return t_ret;
}
std::vector<Colour> ExtractPallet_Image(Image IMG) {
	// brute force iterates over fed image
	uint32_t t_len = IMG.Size.Width * IMG.Size.Height;
	std::vector<Colour> t_ret;
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
void PalletiseImage(Image &IMG, std::vector<Colour> PAL) {
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
Image Read_png(std::string NAM) {
	// assumes .png has been trimmed off
	uint32_t t_pos;
	Image ReturnImage;
	png::rgba_pixel t_pix;
	png::image<png::rgba_pixel> t_file(NAM + ".png");

	ReturnImage.Size = {t_file.get_width(), t_file.get_height()};
	// pngs only have 1 glyph
	ReturnImage.Glyphs.resize(1);
	ReturnImage.Glyphs[0].Name = NAM;
	ReturnImage.Glyphs[0].Size = ReturnImage.Size;
	// handle pixel array
	ReturnImage.Pixels.resize(ReturnImage.Size.Width * ReturnImage.Size.Height);
	for(uint32_t h = 0; h < ReturnImage.Size.Height; h++) {
		for(uint32_t w = 0; w < ReturnImage.Size.Width; w++) {
			t_pos = GetCoordinate(w, h, ReturnImage.Size.Width);
			t_pix = t_file.get_pixel(w, h);
			ReturnImage.Pixels[t_pos].R = t_pix.red;
			ReturnImage.Pixels[t_pos].G = t_pix.green;
			ReturnImage.Pixels[t_pos].B = t_pix.blue;
			ReturnImage.Pixels[t_pos].A = t_pix.alpha;
		}
	}
	return ReturnImage;
}
void Write_png(Image IMG, std::string NAM) {
	png::image<png::rgba_pixel> WriteFile;
	WriteFile.resize(IMG.Size.Width, IMG.Size.Height);
	png::rgba_pixel HoldPixel;
	uint32_t CurrentCoordinate;
	for(uint32_t h = 0; h < IMG.Size.Height; h++) {
	for(uint32_t w = 0; w < IMG.Size.Width; w++) {
		CurrentCoordinate = GetCoordinate(w, h, IMG.Size.Width);
		HoldPixel.red	 = IMG.Pixels[CurrentCoordinate].R;
		HoldPixel.green	 = IMG.Pixels[CurrentCoordinate].G;
		HoldPixel.blue	 = IMG.Pixels[CurrentCoordinate].B;
		HoldPixel.alpha	 = IMG.Pixels[CurrentCoordinate].A;
		WriteFile.set_pixel(w, h, HoldPixel);
	}}
	WriteFile.write(NAM + ".png");
}
inline uint32_t GetCoordinate(uint32_t XPOS,uint32_t YPOS,uint32_t MAXX){
	return (XPOS + (YPOS * MAXX));
}
std::vector<Image> ReorderByVolume(std::vector<Image> UNORDERED){
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
	std::vector<Image> OrderedImages;
	for(uint32_t i = 0; i < UNORDERED.size(); i++) {
		OrderedImages.push_back(UNORDERED[volumest[i]]);
	}
	return OrderedImages;
}
} // namespace RGBKS

#endif // RGBKS_BODY_INCLUDE_BARRIER
#endif // RGBKS_IMPLEM
