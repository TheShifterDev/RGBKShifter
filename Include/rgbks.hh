#ifndef RGBKS_HEAD_INCLUDE_BARRIER
#define RGBKS_HEAD_INCLUDE_BARRIER
#include <cstdint>
#include <fstream>
#include <iostream>
#include <png++/png.hpp> // local install via pacman
#include <string>
#include <vector>

namespace RGBKS {

enum class StimpacVer {
	V1,
	//	stimpac spec for V1
	//	Version						| uint8 	| 1
	//	MasterImage Resolution		| 1
	//		Width					| uint32 	| 4
	//		Height					| uint32 	| 4
	//	RGBA Pixel Array			| (MasterImage width * height)
	//		Red						| uint8		| 1
	//		Green					| uint8		| 1
	//		Blue					| uint8		| 1
	//		Alpha					| uint8		| 1
	//	MasterImage GlyphCount		| uint32	| 4
	//	Glyph Array					| GlyphCount
	//		CharCount				| uint32	| 4
	//		Name					| CharCount
	//		Size					| 1
	//			Width				| uint32	| 4
	//			Height				| uint32	| 4
	//		Position				| 1
	//			Width				| uint32	| 4
	//			Height				| uint32	| 4
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
uint32_t GetCoordinate(uint32_t XPOS, uint32_t YPOS, uint32_t MAXX);
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

#define RGBKS_IMPLEM
#ifdef RGBKS_IMPLEM
#ifndef RGBKS_BODY_INCLUDE_BARRIER
#define RGBKS_BODY_INCLUDE_BARRIER

namespace RGBKS {
// TODO: fix stimpac read/write resulting in mangled image sizes

void Write_stimpac(Image IMG, std::string NAM) {
	std::vector<uint8_t> CharVector;
	uint32_t GlyphCount;
	uint8_t *CharVoodoo;

	// clang-format off
//	Version						| uint8 	| 1
	CharVector.push_back(((uint8_t)StimpacVer::V1));
//	MasterImage Resolution		| 1
//		Width					| uint32 	| 4
//		Height					| uint32 	| 4
	CharVoodoo = (uint8_t*)&IMG.Size;
	for (uint32_t i=0;i<8;i++){
		CharVector.push_back(CharVoodoo[i]);
	}
//	RGBA Pixel Array			| (MasterImage width * height)
//		Red						| uint8		| 1
//		Green					| uint8		| 1
//		Blue					| uint8		| 1
//		Alpha					| uint8		| 1
	for(uint32_t i=0;i<IMG.Pixels.size();i++){
		CharVector.push_back(IMG.Pixels[i].R);
		CharVector.push_back(IMG.Pixels[i].G);
		CharVector.push_back(IMG.Pixels[i].B);
		CharVector.push_back(IMG.Pixels[i].A);
	}
//	MasterImage GlyphCount		| uint32	| 4
	GlyphCount = IMG.Glyphs.size();
	CharVoodoo = (uint8_t*)&GlyphCount;
	for(uint32_t i=0;i<4;i++){
		CharVector.push_back(CharVoodoo[i]);
	}
	
//	Glyph Array					| GlyphCount
	for(uint32_t q=0;q<IMG.Glyphs.size();q++){
//		CharCount				| uint32	| 4
		uint32_t CharCount = IMG.Glyphs[q].Name.length();
		CharVoodoo = (uint8_t*)&CharCount;
		for(uint32_t i=0;i<4;i++){
			CharVector.push_back(CharVoodoo[i]);
		}
//		Name					| CharCount
		for(uint32_t i=0;i<CharCount;i++){
			CharVector.push_back(IMG.Glyphs[q].Name[i]);
		}
//		Size					| 1
//			Width				| uint32	| 4
//			Height				| uint32	| 4
		CharVoodoo = (uint8_t*)&IMG.Glyphs[q].Size;
		for (uint32_t i=0;i<8;i++){
			CharVector.push_back(CharVoodoo[i]);
		}
//		Position				| 1
//			Width				| uint32	| 4
//			Height				| uint32	| 4
		CharVoodoo = (uint8_t*)&IMG.Glyphs[q].Size;
		for (uint32_t i=0;i<8;i++){
			CharVector.push_back(CharVoodoo[i]);
		}
	}
	
	// write char vector to disk
	std::ofstream WriteFile(NAM + ".stimpac", std::ios::out);
	for(uint32_t i=0;i<CharVector.size();i++){
		WriteFile << CharVector[i];
	}
	WriteFile.close();
	// clang-format on
}

Image Read_stimpac(std::string NAM) {
	Image OutputImage;
	StimpacVer CurrentVersion;
	std::ifstream ReadFile;
	std::vector<uint8_t> CharVector;
	char CharArray[64];
	uint8_t *CharVoodoo;
	char HoldChar;
	uint32_t CurrentPosition = 0;
	Colour HoldColour;
	uint32_t GlyphCount;
	uint32_t CharCount;

	// clang-format off
	
	// read file into char vector
	ReadFile.open(NAM + ".stimpac");
	if(!ReadFile.is_open()) {
		printf("could not read %s.\n", NAM.c_str());
		exit(1);
	}
	while(ReadFile.good()) {
		ReadFile >> HoldChar;
		CharVector.push_back(HoldChar);
	}
	ReadFile.close();
	CurrentPosition = 0;
//	Version						| uint8 	| 1
	CurrentVersion = (StimpacVer)CharVector[0];
	if(CurrentVersion >= StimpacVer::ENDOF){
		exit(12); // file is either invalid or 
	}
	CurrentPosition+=1;
//	MasterImage Resolution		| 1
//		Width					| uint32 	| 4
//		Height					| uint32 	| 4
	CharVoodoo = (uint8_t*)&OutputImage.Size.Width;
	for(uint32_t i=0;i<4;i++) {CharVoodoo[i]=CharVector[CurrentPosition+i];}
	CurrentPosition+=4;
	CharVoodoo = (uint8_t*)&OutputImage.Size.Height;
	for(uint32_t i=0;i<4;i++) {CharVoodoo[i]=CharVector[CurrentPosition+i];}
	CurrentPosition+=4;
//	RGBA Pixel Array			| (MasterImage width * height)
//		Red						| uint8		| 1
//		Green					| uint8		| 1
//		Blue					| uint8		| 1
//		Alpha					| uint8		| 1
	OutputImage.Pixels.resize(OutputImage.Size.Width * OutputImage.Size.Height);
	for(uint32_t i=0;i<OutputImage.Pixels.size();i++) {
		HoldColour.R=CharVector[CurrentPosition+0+(i*4)];
		HoldColour.G=CharVector[CurrentPosition+1+(i*4)];
		HoldColour.B=CharVector[CurrentPosition+2+(i*4)];
		HoldColour.A=CharVector[CurrentPosition+3+(i*4)];
		OutputImage.Pixels[i] = HoldColour;
		CurrentPosition+=4;
	}
//	MasterImage GlyphCount		| uint32	| 4
	CharVoodoo = (uint8_t*)&GlyphCount;
	for(uint32_t i=0;i<4;i++) {CharVoodoo[i] = CharVector[CurrentPosition+i];}
	OutputImage.Glyphs.resize(GlyphCount);
	CurrentPosition+=4;
//	Glyph Array					| GlyphCount
	for (uint32_t q=0;q<OutputImage.Glyphs.size();q++) {
//		CharCount				| uint32	| 4
		CharVoodoo = (uint8_t*)&CharCount;
		for(uint32_t i=0;i<4;i++) {CharVoodoo[i] = CharVector[CurrentPosition+i];}
		OutputImage.Glyphs[q].Name.resize(CharCount);
//		Name					| CharCount
	for (uint32_t i=0;i<CharCount;i++) {
			CharArray[i]=CharVector[CurrentPosition+i];
		}
		OutputImage.Glyphs[q].Name = CharArray;
		CurrentPosition+=64;
// BUG: glyphs are reading with broken Size and Position
//		Size					| 1
//			Width				| uint32	| 4
		CharVoodoo = (uint8_t*)&OutputImage.Glyphs[q].Size.Width;
		for(uint32_t i=0;i<4;i++) {CharVoodoo[i]=CharVector[CurrentPosition+i];}
		CurrentPosition+=4;
//			Height				| uint32	| 4
		CharVoodoo = (uint8_t*)&OutputImage.Glyphs[q].Size.Height;
		for(uint32_t i=0;i<4;i++) {CharVoodoo[i]=CharVector[CurrentPosition+i];}
		CurrentPosition+=4;
//		Position				| 1
//			Width				| uint32	| 4
		CharVoodoo = (uint8_t*)&OutputImage.Glyphs[q].Position.Width;
		for(uint32_t i=0;i<4;i++) {CharVoodoo[i]=CharVector[CurrentPosition+i];}
		CurrentPosition+=4;
//			Height				| uint32	| 4
		CharVoodoo = (uint8_t*)&OutputImage.Glyphs[q].Position.Height;
		for(uint32_t i=0;i<4;i++) {CharVoodoo[i]=CharVector[CurrentPosition+i];}
		CurrentPosition+=4;
	}
	return OutputImage;
	// clang-format on
}

std::vector<Image> SeperateGlyphs(std::vector<Image> IMG) {
	std::vector<Image> OutputImages;
	Image HoldingImage;
	Resolution NewRes;
	Resolution Offset;
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
				NewRes = IMG[i].Glyphs[q].Size;
				Offset = IMG[i].Glyphs[q].Position;
				HoldingImage.Size = NewRes;
				HoldingImage.Glyphs.resize(1);
				HoldingImage.Glyphs[0].Position = Resolution{0, 0};
				HoldingImage.Glyphs[0].Size = NewRes;
				HoldingImage.Pixels.resize(NewRes.Width * NewRes.Height);
				for(uint32_t h = 0; h < NewRes.Height; h++) {
				for(uint32_t w = 0; w < NewRes.Width; w++) {
					HoldingImage.Pixels[GetCoordinate(w, h, NewRes.Width)] =
						IMG[i].Pixels[GetCoordinate(Offset.Width + w,Offset.Height + h,IMG[i].Size.Width)];
				}}
				OutputImages.push_back(HoldingImage);
			}
		}
	}

	return OutputImages;
}
Image MergeImages(std::vector<Image> SUBIMAGES) {
	// clang-format off
	// ---------------------SET_SIZE------------------------------------//
	Image OutputImage;
	std::vector<Image> OrderedSubImages;
	OrderedSubImages = ReorderByVolume(SUBIMAGES);
	std::vector<Resolution> ImageResolutions(SUBIMAGES.size());
	bool DrawSafe;
	bool DrawDone;
	uint32_t MPoint;
	uint32_t SPoint;
	uint32_t CheckLimitX;
	uint32_t CheckLimitY;
	uint32_t CurrentShape = 0;
	// get total required volume
	Resolution ExpectedSize = 	{0,0};
	Resolution MaxSize = 		{0,0};

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
						OutputImage.Glyphs[CurrentGlyphs+CurrentGlyphID].Position.Width = OrderedSubImages[CurrentShape].Glyphs[CurrentGlyphID].Position.Width +MXPos;
						OutputImage.Glyphs[CurrentGlyphs+CurrentGlyphID].Position.Height= OrderedSubImages[CurrentShape].Glyphs[CurrentGlyphID].Position.Height+MYPos;
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
		Image CopyImage = OutputImage;
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
			HoldPixel.red = IMG.Pixels[CurrentCoordinate].R;
			HoldPixel.green = IMG.Pixels[CurrentCoordinate].G;
			HoldPixel.blue = IMG.Pixels[CurrentCoordinate].B;
			HoldPixel.alpha = IMG.Pixels[CurrentCoordinate].A;
			WriteFile.set_pixel(w, h, HoldPixel);
		}
	}
	WriteFile.write(NAM + ".png");
}
inline uint32_t GetCoordinate(uint32_t XPOS, uint32_t YPOS, uint32_t MAXX) {
	return (XPOS + (YPOS * MAXX));
}
std::vector<Image> ReorderByVolume(std::vector<Image> UNORDERED) {
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
