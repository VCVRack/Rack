/*
LodePNG Benchmark

Copyright (c) 2005-2014 Lode Vandevenne

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

//g++ lodepng.cpp lodepng_benchmark.cpp -Wall -Wextra -pedantic -ansi -lSDL -O3
//g++ lodepng.cpp lodepng_benchmark.cpp -Wall -Wextra -pedantic -ansi -lSDL -O3 && ./a.out

#include "lodepng.h"

#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>

#include <SDL/SDL.h> //SDL is used for timing.

#define NUM_DECODE 5 //set to 0 for not benchmarking encoding at all, 1 for normal, higher for decoding multiple times to measure better

double total_dec_time = 0;
double total_enc_time = 0;
size_t total_enc_size = 0;
size_t total_in_size = 0; // This is the uncompressed data in the raw color format

bool verbose = false;

////////////////////////////////////////////////////////////////////////////////

double getTime() {
  return SDL_GetTicks() / 1000.0;
}

void fail() {
  throw 1; //that's how to let a unittest fail
}

template<typename T, typename U>
void assertEquals(const T& expected, const U& actual, const std::string& message = "") {
  if(expected != (T)actual) {
    std::cout << "Error: Not equal! Expected " << expected << " got " << actual << "." << std::endl;
    std::cout << "Message: " << message << std::endl;
    fail();
  }
}

void assertTrue(bool value, const std::string& message = "") {
  if(!value) {
    std::cout << "Error: expected true." << std::endl;
    std::cout << "Message: " << message << std::endl;
    fail();
  }
}

//Test image data
struct Image {
  std::vector<unsigned char> data;
  unsigned width;
  unsigned height;
  LodePNGColorType colorType;
  unsigned bitDepth;
};

//Utility for debug messages
template<typename T>
std::string valtostr(const T& val) {
  std::ostringstream sstream;
  sstream << val;
  return sstream.str();
}

template<typename T>
void printValue(const std::string& name, const T& value, const std::string& unit = "") {
  std::cout << name << ": " << value << unit << std::endl;
}

template<typename T, typename U>
void printValue(const std::string& name, const T& value, const std::string& s2, const U& value2, const std::string& unit = "") {
  std::cout << name << ": " << value << s2 << value2 << unit << std::endl;
}

//Test LodePNG encoding and decoding the encoded result, using the C interface
void doCodecTest(Image& image) {
  unsigned char* encoded = 0;
  size_t encoded_size = 0;
  unsigned char* decoded = 0;
  unsigned decoded_w;
  unsigned decoded_h;

  double t_enc0 = getTime();

  unsigned error_enc = lodepng_encode_memory(&encoded, &encoded_size, &image.data[0],
                                             image.width, image.height, image.colorType, image.bitDepth);

  double t_enc1 = getTime();

  assertEquals(0, error_enc, "encoder error C");

  double t_dec0 = getTime();
  for(int i = 0; i < NUM_DECODE; i++) {
    unsigned error_dec = lodepng_decode_memory(&decoded, &decoded_w, &decoded_h,
                                               encoded, encoded_size, image.colorType, image.bitDepth);
    assertEquals(0, error_dec, "decoder error C");
  }
  double t_dec1 = getTime();


  assertEquals(image.width, decoded_w);
  assertEquals(image.height, decoded_h);

  total_enc_size += encoded_size;
  total_enc_time += (t_enc1 - t_enc0);
  total_dec_time += (t_dec1 - t_dec0);
  LodePNGColorMode colormode;
  colormode.colortype = image.colorType;
  colormode.bitdepth = image.bitDepth;
  total_in_size += lodepng_get_raw_size(image.width, image.height, &colormode);

  if(verbose) {
    printValue("encoding time", t_enc1 - t_enc0, "s");
    std::cout << "compression: " << ((double)(encoded_size) / (double)(image.data.size())) * 100 << "%"
              << " ratio: " << ((double)(image.data.size()) / (double)(encoded_size))
              << " size: " << encoded_size << std::endl;
    if(NUM_DECODE> 0) printValue("decoding time", t_dec1 - t_dec0, "/", NUM_DECODE, " s");
    std::cout << std::endl;
  }

  //LodePNG_saveFile(encoded, encoded_size, "test.png");

  free(encoded);
  free(decoded);
}

static const int IMGSIZE = 4096;

void testPatternSine() {
  if(verbose) std::cout << "sine pattern" << std::endl;

  /*
  There's something annoying about this pattern: it encodes worse, slower and with worse compression,
  when adjusting the parameters, while all other images go faster and higher compression, and vice versa.
  It responds opposite to optimizations...
  */

  Image image;
  int w = IMGSIZE / 2;
  int h = IMGSIZE / 2;
  image.width = w;
  image.height = h;
  image.colorType = LCT_RGBA;
  image.bitDepth = 8;
  image.data.resize(w * h * 4);
  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++) {
    //pattern 1
    image.data[4 * w * y + 4 * x + 0] = (unsigned char)(127 * (1 + std::sin((                    x * x +                     y * y) / (w * h / 8.0))));
    image.data[4 * w * y + 4 * x + 1] = (unsigned char)(127 * (1 + std::sin(((w - x - 1) * (w - x - 1) +                     y * y) / (w * h / 8.0))));
    image.data[4 * w * y + 4 * x + 2] = (unsigned char)(127 * (1 + std::sin((                    x * x + (h - y - 1) * (h - y - 1)) / (w * h / 8.0))));
    image.data[4 * w * y + 4 * x + 3] = (unsigned char)(127 * (1 + std::sin(((w - x - 1) * (w - x - 1) + (h - y - 1) * (h - y - 1)) / (w * h / 8.0))));
  }

  doCodecTest(image);
}

void testPatternSineNoAlpha() {
  if(verbose) std::cout << "sine pattern w/o alpha" << std::endl;

  /*
  There's something annoying about this pattern: it encodes worse, slower and with worse compression,
  when adjusting the parameters, while all other images go faster and higher compression, and vice versa.
  It responds opposite to optimizations...
  */

  Image image;
  int w = IMGSIZE / 2;
  int h = IMGSIZE / 2;
  image.width = w;
  image.height = h;
  image.colorType = LCT_RGB;
  image.bitDepth = 8;
  image.data.resize(w * h * 3);
  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++) {
    //pattern 1
    image.data[3 * w * y + 3 * x + 0] = (unsigned char)(127 * (1 + std::sin((                    x * x +                     y * y) / (w * h / 8.0))));
    image.data[3 * w * y + 3 * x + 1] = (unsigned char)(127 * (1 + std::sin(((w - x - 1) * (w - x - 1) +                     y * y) / (w * h / 8.0))));
    image.data[3 * w * y + 3 * x + 2] = (unsigned char)(127 * (1 + std::sin((                    x * x + (h - y - 1) * (h - y - 1)) / (w * h / 8.0))));
  }

  doCodecTest(image);
}

void testPatternXor() {
  if(verbose) std::cout << "xor pattern" << std::endl;

  Image image;
  int w = IMGSIZE;
  int h = IMGSIZE;
  image.width = w;
  image.height = h;
  image.colorType = LCT_RGB;
  image.bitDepth = 8;
  image.data.resize(w * h * 3);
  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++) {
    image.data[3 * w * y + 3 * x + 0] = x ^ y;
    image.data[3 * w * y + 3 * x + 1] = x ^ y;
    image.data[3 * w * y + 3 * x + 2] = x ^ y;
  }

  doCodecTest(image);
}

static unsigned int m_w = 1;
static unsigned int m_z = 2;

//"Multiply-With-Carry" generator of G. Marsaglia
unsigned int getRandomUint() {
  m_z = 36969 * (m_z & 65535) + (m_z >> 16);
  m_w = 18000 * (m_w & 65535) + (m_w >> 16);
  return (m_z << 16) + m_w;  //32-bit result
}

void testPatternPseudoRan() {
  if(verbose) std::cout << "pseudorandom pattern" << std::endl;

  Image image;
  int w = IMGSIZE / 2;
  int h = IMGSIZE / 2;
  image.width = w;
  image.height = h;
  image.colorType = LCT_RGB;
  image.bitDepth = 8;
  image.data.resize(w * h * 3);
  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++) {
    unsigned int random = getRandomUint();
    image.data[3 * w * y + 3 * x + 0] = random % 256;
    image.data[3 * w * y + 3 * x + 1] = (random >> 8) % 256;
    image.data[3 * w * y + 3 * x + 2] = (random >> 16) % 256;
  }

  doCodecTest(image);
}

void testPatternSineXor() {
  if(verbose) std::cout << "sine+xor pattern" << std::endl;

  Image image;
  int w = IMGSIZE / 2;
  int h = IMGSIZE / 2;
  image.width = w;
  image.height = h;
  image.colorType = LCT_RGBA;
  image.bitDepth = 8;
  image.data.resize(w * h * 4);
  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++) {
    //pattern 1
    image.data[4 * w * y + 4 * x + 0] = (unsigned char)(127 * (1 + std::sin((                    x * x +                     y * y) / (w * h / 8.0))));
    image.data[4 * w * y + 4 * x + 1] = (unsigned char)(127 * (1 + std::sin(((w - x - 1) * (w - x - 1) +                     y * y) / (w * h / 8.0))));
    image.data[4 * w * y + 4 * x + 2] = (unsigned char)(127 * (1 + std::sin((                    x * x + (h - y - 1) * (h - y - 1)) / (w * h / 8.0))));
    image.data[4 * w * y + 4 * x + 3] = (unsigned char)(127 * (1 + std::sin(((w - x - 1) * (w - x - 1) + (h - y - 1) * (h - y - 1)) / (w * h / 8.0))));
    image.data[4 * w * y + 4 * x + 0] = image.data[4 * w * y + 4 * x + 0] / 2 + ((x ^ y) % 256) / 2;
    image.data[4 * w * y + 4 * x + 1] = image.data[4 * w * y + 4 * x + 1] / 2 + ((x ^ y) % 256) / 2;
    image.data[4 * w * y + 4 * x + 2] = image.data[4 * w * y + 4 * x + 2] / 2 + ((x ^ y) % 256) / 2;
    image.data[4 * w * y + 4 * x + 3] = image.data[4 * w * y + 4 * x + 3] / 2 + ((x ^ y) % 256) / 2;
  }

  doCodecTest(image);
}

void testPatternGreyMandel() {
  if(verbose) std::cout << "grey mandelbrot pattern" << std::endl;

  Image image;
  int w = IMGSIZE / 2;
  int h = IMGSIZE / 2;
  image.width = w;
  image.height = h;
  image.colorType = LCT_GREY;
  image.bitDepth = 8;
  image.data.resize(w * h);

  double pr, pi;
  double newRe, newIm, oldRe, oldIm;
  // go to a position in the mandelbrot where there's lots of entropy
  double zoom = 1779.8, moveX = -0.7431533999637661, moveY = -0.1394057861346605;
  int maxIterations = 300;

  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++) {
    pr = 1.5 * (x - w / 2) / (0.5 * zoom * w) + moveX;
    pi = (y - h / 2) / (0.5 * zoom * h) + moveY;
    newRe = newIm = oldRe = oldIm = 0; //these should start at 0,0
    int i;
    for(i = 0; i < maxIterations; i++) {
        oldRe = newRe;
        oldIm = newIm;
        newRe = oldRe * oldRe - oldIm * oldIm + pr;
        newIm = 2 * oldRe * oldIm + pi;
        if((newRe * newRe + newIm * newIm) > 4) break;
    }
    image.data[w * y + x] = i % 256;
  }

  doCodecTest(image);
}

void testPatternGreyMandelSmall() {
  if(verbose) std::cout << "grey mandelbrot pattern" << std::endl;

  Image image;
  int w = IMGSIZE / 8;
  int h = IMGSIZE / 8;
  image.width = w;
  image.height = h;
  image.colorType = LCT_GREY;
  image.bitDepth = 8;
  image.data.resize(w * h);

  double pr, pi;
  double newRe, newIm, oldRe, oldIm;
  // go to a position in the mandelbrot where there's lots of entropy
  double zoom = 1779.8, moveX = -0.7431533999637661, moveY = -0.1394057861346605;
  int maxIterations = 300;

  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++) {
    pr = 1.5 * (x - w / 2) / (0.5 * zoom * w) + moveX;
    pi = (y - h / 2) / (0.5 * zoom * h) + moveY;
    newRe = newIm = oldRe = oldIm = 0; //these should start at 0,0
    int i;
    for(i = 0; i < maxIterations; i++) {
        oldRe = newRe;
        oldIm = newIm;
        newRe = oldRe * oldRe - oldIm * oldIm + pr;
        newIm = 2 * oldRe * oldIm + pi;
        if((newRe * newRe + newIm * newIm) > 4) break;
    }
    image.data[w * y + x] = i % 256;
  }

  doCodecTest(image);
}

void testPatternX() {
  if(verbose) std::cout << "x pattern" << std::endl;

  Image image;
  int w = IMGSIZE;
  int h = IMGSIZE;
  image.width = w;
  image.height = h;
  image.colorType = LCT_GREY;
  image.bitDepth = 8;
  image.data.resize(w * h * 4);
  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++) {
    image.data[w * y + x + 0] = x % 256;
  }

  doCodecTest(image);
}

void testPatternY() {
  if(verbose) std::cout << "y pattern" << std::endl;

  Image image;
  int w = IMGSIZE;
  int h = IMGSIZE;
  image.width = w;
  image.height = h;
  image.colorType = LCT_GREY;
  image.bitDepth = 8;
  image.data.resize(w * h * 4);
  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++) {
    image.data[w * y + x + 0] = y % 256;
  }

  doCodecTest(image);
}

void testPatternDisk(const std::string& filename) {
  if(verbose) std::cout << "file " << filename << std::endl;

  Image image;
  image.colorType = LCT_RGB;
  image.bitDepth = 8;
  lodepng::decode(image.data, image.width, image.height, filename, image.colorType, image.bitDepth);

  doCodecTest(image);
}

int main(int argc, char *argv[]) {
  verbose = false;

  std::vector<std::string> files;

  for(int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if(arg == "-v") verbose = true;
    else files.push_back(arg);
  }

  std::cout << "NUM_DECODE: " << NUM_DECODE << std::endl;

  if(files.empty()) {
    //testPatternDisk("testdata/frymire.png");
    //testPatternGreyMandel();

    testPatternDisk("testdata/Ecce_homo_by_Hieronymus_Bosch.png");
    testPatternDisk("testdata/ephyse_franco-chon-s-butchery.png");
    testPatternDisk("testdata/jwbalsley_subway-rats.png");
    testPatternDisk("testdata/Biomenace_complete.png");
    testPatternDisk("testdata/frymire.png");
    testPatternDisk("testdata/lena.png");
    testPatternDisk("testdata/linedrawing.png");
    //testPatternSine();
    //testPatternSineNoAlpha();
    testPatternXor();
    testPatternPseudoRan();
    //testPatternSineXor();
    testPatternGreyMandel();
    //testPatternX();
    //testPatternY();
    //testPatternDisk("Data/purplesmall.png");

    /*testPatternDisk("testdata/Ecce_homo_by_Hieronymus_Bosch.png");
    testPatternSine();*/
  } else {
    for(size_t i = 0; i < files.size(); i++) {
      testPatternDisk(files[i]);
    }
  }

  std::cout << "Total decoding time: " << total_dec_time/NUM_DECODE << "s (" << ((total_in_size/1024.0/1024.0)/(total_dec_time/NUM_DECODE)) << " MB/s)" << std::endl;
  std::cout << "Total encoding time: " << total_enc_time << "s (" << ((total_in_size/1024.0/1024.0)/(total_enc_time)) << " MB/s)" << std::endl;
  std::cout << "Total uncompressed size  : " << total_in_size << std::endl;
  std::cout << "Total encoded size: " << total_enc_size << " (" << (100.0 * total_enc_size / total_in_size) << "%)" << std::endl;

  if(verbose) std::cout << "benchmark done" << std::endl;
}
