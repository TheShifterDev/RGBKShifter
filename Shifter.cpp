#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <string>

#include <png++/png.hpp> // local install via pacman
#include "MakeLowerCase.cpp"
#include <cstdlib>

namespace Shifter {

int ShiftNearist(char *InFileDir, char *OutFileDir) {
  png::image<png::rgb_pixel> image(InFileDir);
  for (int w = 0; w < image.get_width(); w++) {
    for (int h = 0; h < image.get_height(); h++) {
      if (image[w][h].red > 63 && image[w][h].green > 63 &&
          image[w][h].blue > 63) {
        // set white
        image[w][h] = png::rgb_pixel(255, 255, 255);
      } else if (image[w][h].red > 63) {
        // set red
        image[w][h] = png::rgb_pixel(255, 0, 0);
      } else if (image[w][h].green > 63) {
        // set green
        image[w][h] = png::rgb_pixel(0, 255, 0);
      } else if (image[w][h].blue > 63) {
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
  int Num = 0;

  png::image<png::rgb_pixel> image(InFileDir);

  while (!VALID_RESPONCE_RETURNED) {
    std::cout << "input A for one colour, B for two, etc for a max of 5"
              << "\n";

    std::cin >> Returned;
    std::cout << "\n";
    Returned = MakeLowerCase::Lower(Returned);

    if (Returned == "a") {
      VALID_RESPONCE_RETURNED = true;
    } else if (Returned == "b") {
      VALID_RESPONCE_RETURNED = true;
      Num = 1;
    } else if (Returned == "c") {
      VALID_RESPONCE_RETURNED = true;
      Num = 2;
    } else if (Returned == "d") {
      VALID_RESPONCE_RETURNED = true;
      Num = 3;
    } else if (Returned == "e") {
      VALID_RESPONCE_RETURNED = true;
      Num = 4;
    } else {
    }
  }
  int pixHeight = (image.get_height() * Num) * 8;

  int Current = -1;

  int inum = 0;
  for (int w = 0; w < image.get_width(); w++) {
    for (int h = 0; h < image.get_height(); h++) {
      if (inum % pixHeight * image.get_width() == 0) {
        if (Current == Num) {
          Current = 0;
        } else {
          Current++;
        }
      }
      if (Current == 0) {
        // red
        image[w][h] = png::rgb_pixel(255, 0, 0);
      } else if (Current == 1) {
        // green
        image[w][h] = png::rgb_pixel(0, 255, 0);
      } else if (Current == 2) {
        // blue
        image[w][h] = png::rgb_pixel(0, 0, 255);
      } else if (Current == 3) {
        // black
        image[w][h] = png::rgb_pixel(0, 0, 0);
      } else {
        // white
        image[w][h] = png::rgb_pixel(255, 255, 255);
      }
      inum++;
    }
  }
  image.write(OutFileDir);
  return 0;
}

int CreateLayeringNoise(char *InFileDir, char *OutFileDir) {
  bool VALID_RESPONCE_RETURNED = false;
  std::string Returned = "";
  int Num = 0;

  png::image<png::rgb_pixel> image(InFileDir);

  while (!VALID_RESPONCE_RETURNED) {
    std::cout << "input A for one colour, B for two, etc for a max of 5"
              << "\n";

    std::cin >> Returned;
    std::cout << "\n";
    Returned = MakeLowerCase::Lower(Returned);

    int Num = 0;

    if (Returned == "a") {
      VALID_RESPONCE_RETURNED = true;
    } else if (Returned == "b") {
      VALID_RESPONCE_RETURNED = true;
      Num = 1;
    } else if (Returned == "c") {
      VALID_RESPONCE_RETURNED = true;
      Num = 2;
    } else {
      VALID_RESPONCE_RETURNED = true;
      Num = 3;
    }
  }
  int ColR = 0;
  int ColG = 0;
  int ColB = 0;

  // pass A
  for (int w = 0; w < image.get_width(); w++) {
    for (int h = 0; h < image.get_height(); h++) {
      image[w][h] = png::rgb_pixel(ColR, ColG, ColB);
    }
  }

  for (int i = 0; i < Num + 1; i++) {
    switch (i) {
    case 0: {
      ColR = 255;
      ColG = 0;
      ColB = 0;
    }
    case 1: {
      ColR = 0;
      ColG = 255;
      ColB = 0;
    }
    case 2: {
      ColR = 0;
      ColG = 0;
      ColB = 255;
    }
    case 3: {
      ColR = 255;
      ColG = 255;
      ColB = 255;
    }
    default: {
      ColR = 0;
      ColG = 0;
      ColB = 0;
    }
    }
  }
  // perlin noise
  // gen norm noise
  int InitialNoise[image.get_width()][image.get_height()];
  for (int w = 0; w < image.get_width();w++){
    for (int h = 0; h < image.get_height();h++){
      InitialNoise[w][h] = std::rand() % 256;
      
        image[w][h] = png::rgb_pixel(InitialNoise[w][h], 0, 0);
    }
  }
  image.write("./output/shite.png");
  std::cout << "written\n";
  image.write(OutFileDir);
  return 0;
}

int LoadImageAndShift(int shiftType, char *InFileDir, char *OutFileDir) {
  // call rgb shift
  if (shiftType == 0) {
    ShiftNearist(InFileDir, OutFileDir);
  } else if (shiftType == 1) {
    CreateStripes(InFileDir, OutFileDir);
  } else {
    CreateLayeringNoise(InFileDir, OutFileDir);
  }

  return 0;
}
} // namespace Shifter
