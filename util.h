#ifndef MPEG_DECODER_UTIL_H
#define MPEG_DECODER_UTIL_H

#include <vector>
#include <string>
#include <SFML/Graphics/Color.hpp>

std::vector<std::string> split(const std::string& str);
std::vector<std::string> dir_list(std::string dir_name);

void idct(double (*dest)[8], int (*source)[8]);
void merge(sf::Color (*dest)[16], double (*y)[8][8], double (*cb)[8], double(*cr)[8]);

#endif
