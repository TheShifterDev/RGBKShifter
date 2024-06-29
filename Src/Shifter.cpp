
#include "Shifter.hpp"
namespace Shifter {

int ShiftNearist(char *InFileDir, char *OutFileDir) {
	png::image<png::rgb_pixel> image(InFileDir);
	for(uint32_t w = 0; w < image.get_width(); w++) {
		for(uint32_t h = 0; h < image.get_height(); h++) {
			if(image[w][h].red > 63 && image[w][h].green > 63 &&
			   image[w][h].blue > 63) {
				// set white
				image[w][h] = png::rgb_pixel(255, 255, 255);
			} else if(image[w][h].red > 63) {
				// set red
				image[w][h] = png::rgb_pixel(255, 0, 0);
			} else if(image[w][h].green > 63) {
				// set green
				image[w][h] = png::rgb_pixel(0, 255, 0);
			} else if(image[w][h].blue > 63) {
				// set blue
				image[w][h] = png::rgb_pixel(0, 0, 255);
			} else {
				// set black
				image[w][h] = png::rgb_pixel(0, 0, 0);
			}
		}
	}
	image.write(OutFileDir);
	std::cout << OutFileDir << "\n";
	return 0;
}

} // namespace Shifter
