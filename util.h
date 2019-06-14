#ifndef MPEG_DECODER_UTIL_H
#define MPEG_DECODER_UTIL_H

#include <vector>
#include <string>
#include <SFML/Graphics/Color.hpp>

std::vector<std::string> split(const std::string& str);
std::vector<std::string> dir_list(std::string dir_name);

void idct(double dest[8][8], int source[8][8]);
void merge_blocks(sf::Color dest[16][16], double source[6][8][8]);

#endif
