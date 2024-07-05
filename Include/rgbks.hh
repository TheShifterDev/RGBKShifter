#ifndef RGBKS_HEAD_INCLUDE_BARRIER
#define RGBKS_HEAD_INCLUDE_BARRIER
#include <cstdint>
#include <fstream>
#include <png++/png.hpp> // local install via pacman
#include <string>
#include <iostream>
#include <vector>

namespace RGBKS {

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
	char Name[64] = "Unnamed";
	Resolution Size = {64, 64};
	Resolution Position = {0, 0};
};
struct Image {
	Resolution Size = {64, 64};
	std::vector<Colour> Pixels{};
	std::vector<Glyph> Glyphs{};
};
#define GETCORD(X, Y, XMAX) (X + (Y * XMAX))

float GetRGBColourDistance(Colour A, Colour B);
void PalletiseImage(Image &IMG, std::vector<Colour> PAL);
Image MergeImages(std::vector<Image> IMG);
std::vector<Colour> ExtractPallet_Image(Image IMG);
Image Read_png(std::string NAM);
Image Read_imgpac(std::string NAM);
void Write_png(Image IMG, std::string NAM);
void Write_imgpac(Image IMG, std::string NAM);

} // namespace RGBKS

#endif // RGBKS_HEAD_INCLUDE_BARRIER

#ifdef RGBKS_IMPLEM
#ifndef RGBKS_BODY_INCLUDE_BARRIER
#define RGBKS_BODY_INCLUDE_BARRIER

namespace RGBKS {

void ReorderByVolume(std::vector<Image>& IMG){
	std::vector<Image> UnorderedImages = IMG;
	uint32_t hold;
	std::vector<uint32_t> volumest(IMG.size());
	// create volumest list
	for(uint32_t i = 0; i < IMG.size(); i++) {
		volumest[i] = i;
	}
	// reorder volumest
	for(uint32_t i = 1; i < IMG.size(); i++) {
		if(IMG[volumest[i - 1]].Pixels.size() <
		   IMG[volumest[i]].Pixels.size()) {
			// if smaller switch and restart
			hold = volumest[i];
			volumest[i] = volumest[i - 1];
			volumest[i - 1] = hold;
			i = 0;
		}
	}
	for(uint32_t i = 0; i < IMG.size(); i++) {
		IMG[i] = UnorderedImages[volumest[i]];
	}
}

Image MergeImages(std::vector<Image> SUBIMAGE) {
	// clang-format off
	// ---------------------SET_SIZE------------------------------------//
	//ReorderByVolume(IMG);
	std::vector<Resolution> ImageResolutions(SUBIMAGE.size());
	uint32_t RequiredVolume = 0;
	// get total required volume
	for(uint32_t i = 0; i < SUBIMAGE.size(); i++) {
		ImageResolutions[i] = SUBIMAGE[i].Size;
		RequiredVolume += (SUBIMAGE[i].Size.Width*SUBIMAGE[i].Size.Height);
	}
	// get base total size for new image
	Resolution ExpectedSize = SUBIMAGE[0].Size;
	while (ExpectedSize.Width*ExpectedSize.Height < RequiredVolume){
		ExpectedSize.Width++;ExpectedSize.Height++;
	}
	
	Image MainImage;
	MainImage.Size = ExpectedSize;
	uint32_t hold = ExpectedSize.Width*ExpectedSize.Height;
	MainImage.Pixels.resize(hold);
	std::vector<bool> OccupiedPositions(hold);
	// ---------------------------DRAWING--------------------------------//
	bool DrawSafe;
	bool DrawDone;
	uint32_t maspoint;
	uint32_t subpoint;
	uint32_t CheckLimitX;
	uint32_t CheckLimitY;
	// place shapes
	for(uint32_t CurrentShape = 0; CurrentShape < SUBIMAGE.size(); CurrentShape++) {
		DrawDone = false;
		CheckLimitY = (MainImage.Size.Height-(SUBIMAGE[CurrentShape].Size.Height-1));
		CheckLimitX = (MainImage.Size.Width -(SUBIMAGE[CurrentShape].Size.Width -1));
		for(uint32_t MainYPos = 0; MainYPos < CheckLimitY; MainYPos++) {
		for(uint32_t MainXPos = 0; MainXPos < CheckLimitX; MainXPos++) {
			if (OccupiedPositions[GETCORD(MainXPos, MainYPos, MainImage.Size.Width)] == false) {
				DrawSafe = true;
				// check if position collides with previously written positions
				for(uint32_t SubYPos=0;SubYPos<SUBIMAGE[CurrentShape].Size.Height;SubYPos++){
				for(uint32_t SubXPos=0;SubXPos<SUBIMAGE[CurrentShape].Size.Width ;SubXPos++){
					if(OccupiedPositions[GETCORD((MainXPos+SubXPos),(MainYPos+SubYPos),MainImage.Size.Width)] == true){
						DrawSafe = false;
						goto AlreadyWrittenBreakout;
					}
				}}
				AlreadyWrittenBreakout:;
				if (DrawSafe){
					// draw
					for(uint32_t SubYPos=0;SubYPos<SUBIMAGE[CurrentShape].Size.Height;SubYPos++){
					for(uint32_t SubXPos=0;SubXPos<SUBIMAGE[CurrentShape].Size.Width ;SubXPos++){
						maspoint = GETCORD(MainXPos + SubXPos, MainYPos + SubYPos,MainImage.Size.Width);
						subpoint = GETCORD(SubXPos, SubYPos, SUBIMAGE[CurrentShape].Size.Width);
						MainImage.Pixels[maspoint] = SUBIMAGE[CurrentShape].Pixels[subpoint];
						OccupiedPositions[maspoint] = true;
					}}
					// place glyphs
					Glyph GlyphHold;
					for(uint32_t sg = 0; sg < SUBIMAGE[CurrentShape].Glyphs.size();sg++){
						GlyphHold = SUBIMAGE[CurrentShape].Glyphs[sg];
						GlyphHold.Position.Width  += MainXPos;
						GlyphHold.Position.Height += MainYPos;
						MainImage.Glyphs.push_back(GlyphHold);
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
	uint32_t closest = 0;
	uint32_t t_len = (IMG.Size.Width * IMG.Size.Height);
	for(uint32_t i = 0; i < t_len; i++) {
		// gets pixels distance to pallet colours (RGB only)
		for(uint32_t q = 0; q < PAL.size(); q++) {
			distances[q] = GetRGBColourDistance(PAL[q], IMG.Pixels[i]);
		}
		// gets closest pallet colour
		closest = 0;
		for(uint32_t q = 1; q < PAL.size(); q++) {
			if(distances[q] < distances[closest]) {
				closest = q;
			}
		}
		// make pixel its closest pallet counterpart
		IMG.Pixels[i].R = PAL[closest].R;
		IMG.Pixels[i].G = PAL[closest].G;
		IMG.Pixels[i].B = PAL[closest].B;
	}
}

Image Read_png(std::string NAM) {
	// assumes .png has been trimmed off
	uint32_t t_pos;
	Image t_ret;
	png::rgba_pixel t_pix;
	png::image<png::rgba_pixel> t_file(NAM + ".png");

	t_ret.Size = {t_file.get_width(), t_file.get_height()};
	// pngs only have 1 glyph
	t_ret.Glyphs.resize(1);
	for(uint32_t i = 0; i < NAM.size(); i++) {
		t_ret.Glyphs[0].Name[i] = NAM[i];
	}
	t_ret.Glyphs[0].Size = t_ret.Size;
	// handle pixel array
	t_ret.Pixels.resize(t_ret.Size.Width * t_ret.Size.Height);
	for(uint32_t h = 0; h < t_ret.Size.Height; h++) {
		for(uint32_t w = 0; w < t_ret.Size.Width; w++) {
			t_pos = GETCORD(w, h, t_ret.Size.Width);
			t_pix = t_file.get_pixel(w, h);
			t_ret.Pixels[t_pos].R = t_pix.red;
			t_ret.Pixels[t_pos].G = t_pix.green;
			t_ret.Pixels[t_pos].B = t_pix.blue;
			t_ret.Pixels[t_pos].A = t_pix.alpha;
		}
	}
	return t_ret;
}
void Write_png(Image IMG, std::string NAM) {
	png::image<png::rgba_pixel> t_file;
	t_file.resize(IMG.Size.Width, IMG.Size.Height);
	png::rgba_pixel t_pix;
	uint32_t t_pos;
	for(uint32_t h = 0; h < IMG.Size.Height; h++) {
		for(uint32_t w = 0; w < IMG.Size.Width; w++) {
			t_pos = GETCORD(w, h, IMG.Size.Width);
			t_pix.red = IMG.Pixels[t_pos].R;
			t_pix.green = IMG.Pixels[t_pos].G;
			t_pix.blue = IMG.Pixels[t_pos].B;
			t_pix.alpha = IMG.Pixels[t_pos].A;
			t_file.set_pixel(w, h, t_pix);
		}
	}
	t_file.write(NAM + ".png");
}

void Write_imgpac(Image IMG, std::string NAM) {
	// evaluate needed size
	size_t t_size = 0;
	t_size += sizeof(Resolution);
	t_size += sizeof(Colour) * (IMG.Size.Width * IMG.Size.Height);
	t_size += sizeof(uint32_t);
	t_size += sizeof(Glyph) * IMG.Glyphs.size();
	uint32_t t_curpos = 0;
	uint32_t t_tarpos = 0;
	// allocate size
	std::vector<uint8_t> t_hold;
	t_hold.resize(t_size / sizeof(uint8_t));
	// dump image width and height into hold
	uint8_t t_res[8];
	t_res[0] = IMG.Size.Width;
	t_res[4] = IMG.Size.Height;
	t_tarpos += 8;
	while(t_curpos < t_tarpos) {
		t_hold[t_curpos] = t_res[t_curpos];
		t_curpos++;
	}
	// dump pixels in
	t_tarpos += (IMG.Pixels.size() * 4);
	uint32_t t_ite = 0;
	while(t_curpos < t_tarpos) {
		t_hold[t_curpos + 0] = IMG.Pixels[t_ite].R;
		t_hold[t_curpos + 1] = IMG.Pixels[t_ite].G;
		t_hold[t_curpos + 2] = IMG.Pixels[t_ite].B;
		t_hold[t_curpos + 3] = IMG.Pixels[t_ite].A;
		t_ite++;
		t_curpos += 4;
	}
	// dump glyphcount into hold
	t_tarpos += 4;
	uint8_t t_cou[4];
	t_ite = 0;
	t_cou[0] = IMG.Glyphs.size();
	while(t_curpos < t_tarpos) {
		t_hold[t_curpos] = t_cou[t_ite];
		t_ite++;
		t_curpos++;
	}
	// dump glyphs into hold
	t_ite = 0;
	while(t_curpos < t_tarpos) {
		for(uint32_t i = 0; i < 64; i++) {
			t_hold[t_curpos + i] = IMG.Glyphs[t_ite].Name[i];
		}
		t_curpos += 64;
		// size
		t_res[0] = IMG.Glyphs[t_ite].Size.Width;
		t_res[4] = IMG.Glyphs[t_ite].Size.Height;
		for(uint8_t i = 0; i < 8; i++) {
			t_hold[t_curpos + i] = t_res[i];
		}
		t_curpos += 8;
		// pos
		t_res[0] = IMG.Glyphs[t_ite].Position.Width;
		t_res[4] = IMG.Glyphs[t_ite].Position.Height;
		for(uint8_t i = 0; i < 8; i++) {
			t_hold[t_curpos + i] = t_res[i];
		}
		t_curpos += 8;
		t_ite++;
	}
	// write char vector to disk
	// .stimpac = "stoma image package"
	std::ofstream t_out(NAM + ".stimpac", std::ios::out);
	t_out << t_hold.data();
	t_out.close();
}

Image Read_imgpac(std::string NAM) {
	Image t_ret;
	std::ifstream t_fil;
	std::vector<uint8_t> t_hol;
	uint8_t t_chr;
	uint32_t t_curpos = 0;
	uint32_t t_tarpos = 0;
	uint32_t t_ite;
	// read file into char vector
	t_fil.open(NAM + ".stimpac");
	if(!t_fil.is_open()) {
		printf("could not read %s.\n", NAM.c_str());
		exit(1);
	}
	while(t_fil.good()) {
		t_fil >> t_chr;
		t_hol.push_back(t_chr);
	}
	// parse
	// read image size
	t_tarpos += 8;
	if(t_hol.size() < t_tarpos) {
		printf("%s has invalid length (too small for width and height "
			   "reading).\n",
			   NAM.c_str());
		exit(1);
	}
	// first 4 is the uint32 width
	// next  4 is the uint32 height
	uint8_t r_widhig[8];
	t_ite = 0;
	while(t_curpos < t_tarpos) {
		r_widhig[t_ite] = t_hol[t_curpos];
		t_curpos++;
		t_ite++;
	}
	t_ret.Size.Width = (uint32_t)r_widhig[0];
	t_ret.Size.Height = (uint32_t)r_widhig[4];

	// read image as (width*height)*uint8's in Colour
	Colour r_colour;
	t_tarpos += (t_ret.Size.Width * t_ret.Size.Height) * 4;
	if(t_hol.size() < t_tarpos) {
		printf("%s has invalid length (too small to hold the image's pixel "
			   "array).\n",
			   NAM.c_str());
		exit(1);
	}
	while(t_curpos < t_tarpos) {
		r_colour.R = t_hol[t_curpos + 0];
		r_colour.G = t_hol[t_curpos + 1];
		r_colour.B = t_hol[t_curpos + 2];
		r_colour.A = t_hol[t_curpos + 3];
		t_ret.Pixels.push_back(r_colour);
		t_curpos += 4;
	}

	// read glyph count
	uint8_t r_glcount[4];
	t_tarpos += 4;
	t_ite = 0;
	if(t_hol.size() < t_tarpos) {
		printf("%s has invalid length (too small for the glyphcount).\n",
			   NAM.c_str());
		exit(1);
	}
	while(t_curpos < t_tarpos) {
		r_glcount[t_ite] = t_hol[t_curpos];
		t_ite++;
		t_curpos++;
	}
	t_ret.Glyphs.resize((uint32_t)r_glcount[0]);
	// read in glyph data
	uint32_t glyphsize = sizeof(Glyph) / sizeof(uint8_t);
	std::vector<uint8_t> r_glyphchar;
	r_glyphchar.resize(glyphsize);
	Glyph r_glyph;
	t_ite = 0;
	t_tarpos += (glyphsize * t_ret.Glyphs.size());
	if(t_hol.size() < t_tarpos) {
		printf("%s has invalid length (too small for the glyph array).\n",
			   NAM.c_str());
		exit(1);
	}
	while(t_ite < t_ret.Glyphs.size()) {
		for(uint32_t i = 0; i < glyphsize; i++) {
			r_glyphchar[i] = t_hol[t_curpos];
			t_curpos++;
		}
		// build glyph name
		for(uint32_t i = 0; i < 64; i++) {
			r_glyph.Name[i] = r_glyphchar[i];
		}
		// build glyph size
		for(uint32_t i = 0; i < 8; i++) {
			r_widhig[i] = r_glyphchar[i + 64];
		}
		r_glyph.Size.Width = (uint32_t)r_widhig[0];
		r_glyph.Size.Height = (uint32_t)r_widhig[4];
		// build glyph position
		for(uint32_t i = 0; i < 8; i++) {
			r_widhig[i] = r_glyphchar[i + (64 + 8)];
		}
		r_glyph.Position.Width = (uint32_t)r_widhig[0];
		r_glyph.Position.Height = (uint32_t)r_widhig[4];

		t_ret.Glyphs[t_ite] = r_glyph;
		t_ite++;
	}

	return t_ret;
}
} // namespace RGBKS

#endif // RGBKS_BODY_INCLUDE_BARRIER
#endif // RGBKS_IMPLEM