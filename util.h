#ifndef MPEG_DECODER_UTIL_H
#define MPEG_DECODER_UTIL_H

#include <string>
#include <SFML/Graphics/Color.hpp>
#include "YCbCr.h"

std::vector<std::string> split(const std::string& str);

void idct(double dest[8][8], int source[8][8]);
void merge_blocks(YCbCr dest[16][16], double source[6][8][8]);
unsigned char chomp(double x);

#endif
