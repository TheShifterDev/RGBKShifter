#ifndef RGBKS_HEAD_INCLUDE_BARRIER
#define RGBKS_HEAD_INCLUDE_BARRIER
#include <cstdint>
#include <fstream>
#include <png++/png.hpp> // local install via pacman
#include <string>
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
	uint32_t GlyphCount = 1;
	std::vector<Glyph> Glyphs{};
};
#define GETCORD(X, Y, XMAX) (X + (Y * XMAX))

void ShiftNearist(Image &IMG);
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

void ShiftNearist(Image &IMG) {
	uint8_t t_bor = 127;
	Colour t_pix;
	// clang-format off
	for(uint32_t w = 0; w < IMG.Size.Width;  w++) {
	for(uint32_t h = 0; h < IMG.Size.Height; h++) {
		t_pix = IMG.Pixels[GETCORD(w, h, IMG.Size.Width)];
		if			(t_pix.R > t_bor && t_pix.G > t_bor && t_pix.B > t_bor) {	// set white
			t_pix.R = 255;t_pix.G = 255;t_pix.B = 255;
		} else if	(t_pix.R > t_bor && t_pix.G > t_bor) {						// set orange
			t_pix.R = 255;t_pix.G = 255;t_pix.B =   0;
		} else if	(t_pix.R > t_bor && t_pix.B > t_bor) {						// set purple
			t_pix.R = 255;t_pix.G =   0;t_pix.B = 255;
		} else if	(t_pix.G > t_bor && t_pix.B > t_bor) {						// set cyan
			t_pix.R =   0;t_pix.G = 255;t_pix.B = 255;
		} else if	(t_pix.R > t_bor) {											// set red
			t_pix.R = 255;t_pix.G =   0;t_pix.B =   0;
		} else if	(t_pix.G > t_bor) {											// set green
			t_pix.R =   0;t_pix.G = 255;t_pix.B =   0;
		} else if	(t_pix.B > t_bor) {											// set blue
			t_pix.R =   0;t_pix.G =   0;t_pix.B = 255;
		} else {																// set black
			t_pix.R =   0;t_pix.G =   0;t_pix.B =   0;
		}
	}}
	// clang-format on
}
Image Read_png(std::string NAM) {
	// assumes .png has been trimmed off
	uint32_t t_pos;
	Image t_ret;
	png::rgba_pixel t_pix;
	png::image<png::rgba_pixel> t_file(NAM+".png");

	t_ret.Size = {t_file.get_width(), t_file.get_height()};
	// pngs only have 1 glyph
	t_ret.GlyphCount = 1;
	t_ret.Glyphs.resize(t_ret.GlyphCount);
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
	t_file.write(NAM+".png");
}

void write_imgpac(Image IMG, std::string NAM) {
	// evaluate needed size
	size_t t_size = 0;
	t_size += sizeof(Resolution);
	t_size += sizeof(Colour) * (IMG.Size.Width * IMG.Size.Height);
	t_size += sizeof(uint32_t);
	t_size += sizeof(Glyph) * IMG.GlyphCount;
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
	t_cou[0] = IMG.GlyphCount;
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
	t_ret.GlyphCount = (uint32_t)r_glcount[0];
	// read in glyph data
	uint32_t glyphsize = sizeof(Glyph) / sizeof(uint8_t);
	std::vector<uint8_t> r_glyphchar;
	r_glyphchar.resize(glyphsize);
	Glyph r_glyph;
	t_ite = 0;
	t_tarpos += (glyphsize * t_ret.GlyphCount);
	if(t_hol.size() < t_tarpos) {
		printf("%s has invalid length (too small for the glyph array).\n",
			   NAM.c_str());
		exit(1);
	}
	while(t_ite < t_ret.GlyphCount) {
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

		t_ret.Glyphs.push_back(r_glyph);
		t_ite++;
	}

	return t_ret;
}
} // namespace RGBKS

#endif // RGBKS_BODY_INCLUDE_BARRIER
#endif // RGBKS_IMPLEM