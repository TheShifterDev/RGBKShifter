
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

int CreateStripes(char *InFileDir, char *OutFileDir) {
	// stripe
	bool VALID_RESPONCE_RETURNED = false;
	std::string Returned = "";
	uint8_t Num = 0;

	png::image<png::rgb_pixel> image(InFileDir);

	while(!VALID_RESPONCE_RETURNED) {
		std::cout << "input A for one colour, B for two, etc for a max of 5"
				  << "\n";

		std::cin >> Returned;
		std::cout << "\n";
		Returned = MakeLowerCase::Lower(Returned);

		VALID_RESPONCE_RETURNED = true;
		// clang-format off
		switch(Returned[0]) {
		case('a'): {Num = 0;break;};
		case('b'): {Num = 1;break;};
		case('c'): {Num = 2;break;};
		case('d'): {Num = 3;break;};
		case('e'): {Num = 4;break;};
		default: {VALID_RESPONCE_RETURNED = false;break;}
		}
		// clang-format on
	}
	int pixHeight = (image.get_height() * Num) * 8;

	int Current = -1;

	int inum = 0;
	for(uint32_t w = 0; w < image.get_width(); w++) {
		for(uint32_t h = 0; h < image.get_height(); h++) {
			if(inum % pixHeight * image.get_width() == 0) {
				if(Current == Num) {
					Current = 0;
				} else {
					Current++;
				}
			}
			// clang-format off
			switch(Current){
				case(0):{image[w][h] = png::rgb_pixel(255,  0,  0);break;}
				case(1):{image[w][h] = png::rgb_pixel(  0,255,  0);break;}
				case(2):{image[w][h] = png::rgb_pixel(  0,  0,255);break;}
				case(3):{image[w][h] = png::rgb_pixel(  0,  0,  0);break;}
				default:{image[w][h] = png::rgb_pixel(255,255,255);break;}
			}
			// clang-format on
			inum++;
		}
	}
	image.write(OutFileDir);
	return 0;
}

int CreateLayeringNoise(char *InFileDir, char *OutFileDir) {
	bool VALID_RESPONCE_RETURNED = false;
	std::string Returned = "";
	uint8_t Num = 0;

	png::image<png::rgb_pixel> image(InFileDir);

	while(!VALID_RESPONCE_RETURNED) {
		std::cout << "input A for one colour, B for two, etc for a max of 5"
				  << "\n";

		std::cin >> Returned;
		std::cout << "\n";
		Returned = MakeLowerCase::Lower(Returned);

		Num = 0;
	
		VALID_RESPONCE_RETURNED = true;
		// clang-format off
		switch (Returned[0]){
			case('b'):{Num=1;break;}
			case('c'):{Num=2;break;}
			case('d'):{Num=3;break;}
			default  :{Num=0;break;}
		}
		// clang-format on
	}
	uint8_t ColR = 0;
	uint8_t ColG = 0;
	uint8_t ColB = 0;

	// pass A
	for(uint32_t w = 0; w < image.get_width(); w++) {
		for(uint32_t h = 0; h < image.get_height(); h++) {
			image[w][h] = png::rgb_pixel(ColR, ColG, ColB);
		}
	}

	for(uint32_t i = 0; i < Num + 1; i++) {
		// clang-format off
		switch(i) {
		case(0): {ColR = 255;ColG =   0;ColB =   0;}
		case(1): {ColR =   0;ColG = 255;ColB =   0;}
		case(2): {ColR =   0;ColG =   0;ColB = 255;}
		case(3): {ColR = 255;ColG = 255;ColB = 255;}
		default: {ColR =   0;ColG =   0;ColB =   0;}
		}
		// clang-format on
	}
	// perlin noise
	// gen norm noise
	uint8_t* InitialNoise = (uint8_t*)malloc(sizeof(uint8_t) * (image.get_width()*image.get_height()));
	for(uint32_t w = 0; w < image.get_width(); w++) {
		for(uint32_t h = 0; h < image.get_height(); h++) {
			InitialNoise[w+(h*image.get_width())] = std::rand() % 256;

			image[w][h] = png::rgb_pixel(
				InitialNoise[w+(h*image.get_width())],
				InitialNoise[w+(h*image.get_width())],
				InitialNoise[w+(h*image.get_width())]);
		}
	}
	image.write("./output/shite.png");
	std::cout << "written\n";
	image.write(OutFileDir);
	free(InitialNoise);
	return 0;
}

int LoadImageAndShift(int shiftType, char *InFileDir, char *OutFileDir) {
	// call rgb shift
	if(shiftType == 0) {
		ShiftNearist(InFileDir, OutFileDir);
	} else if(shiftType == 1) {
		CreateStripes(InFileDir, OutFileDir);
	} else {
		CreateLayeringNoise(InFileDir, OutFileDir);
	}

	return 0;
}
} // namespace Shifter
