#ifndef RGBKS_IMAGE_HEAD_INCLUDE_BARRIER
#define RGBKS_IMAGE_HEAD_INCLUDE_BARRIER
#include <cstdint>
#include <fstream>
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

Image Read_imgpac(std::string NAM);
void write_imgpac(Image IMG, std::string NAM);

} // namespace RGBKS

#endif // RGBKS_IMAGE_HEAD_INCLUDE_BARRIER

##ifdef RGBKS_IMAGE_IMPLEM
#ifndef RGBKS_IMAGE_BODY_INCLUDE_BARRIER
#define RGBKS_IMAGE_BODY_INCLUDE_BARRIER

	namespace RGBKS {

	void write_imgpac(Image IMG, std::string NAM) {
		// evaluate needed size
		
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
		t_fil.open(NAM);
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
}

#endif // RGBKS_IMAGE_BODY_INCLUDE_BARRIER
#endif // RGBKS_IMAGE_IMPLEM