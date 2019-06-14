#include "util.h"
#include <vector>
#include <string>
#include <dirent.h>
#include <algorithm>
#include <iostream>
#include <math.h>
#include <SFML/Graphics/Color.hpp>

using namespace std;

vector<string> split(const string& str) {
	vector<string> ret;
	string strs = str + " ";
	size_t pos;
	size_t size = strs.size();

	for (int i = 0; i < (int)size; ++i) {
		pos = strs.find(" ", i);
		if( pos < size) {
			std::string s = strs.substr(i, pos - i);
			ret.push_back(s);
			i = pos;
		}

	}
	return ret;
}
vector<string> dir_list(string dir_name) {
	vector<string> ret;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(dir_name.c_str())) != nullptr) {
		while ((ent = readdir(dir)) != nullptr) {
			ret.emplace_back(ent->d_name);
		}
		closedir(dir);
		sort(ret.begin(), ret.end());
		return ret;
	} else {
		perror("");
		throw "can't open dir"s;
	}
}

double c(int i) {
	static double x = 1.0 / sqrt(2.0);
	if (i == 0) {
		return x;
	}
	else {
		return 1.0;
	}
}

void idct(double dest[8][8], int source[8][8]) {
	static bool init = false;
	static double cos_cache[200] = {};
	if (!init) {
		init = true;
		for (int i = 0; i < 200; i++) {
			cos_cache[i] = cos(i * M_PI / 16.0);
		}
	}
	double tmp[8][8] = {};
	double s[8][8] = {};

	for (int j = 0; j < 8; j++) {
		for (int x = 0; x < 8; x++) {
			for (int y = 0; y < 8; y++) {
				s[j][x] += c (y) * source[x][y] * cos_cache[(j + j + 1) * y];
			}
			s[j][x] = s[j][x] / 2.0;
		}
	}
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			for (int x = 0; x < 8; x++) {
				tmp[i][j] += c(x) * s[j][x] * cos_cache[(i + i + 1) * x];
			}
			tmp[i][j] = tmp[i][j] / 2.0;
		}
	}
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			dest[i][j] = tmp[i][j];
		}
	}
}

unsigned char chomp(double x) {
	if (x > 255.0) {
		return 255;
	} else if (x < 0) {
		return 0;
	} else {
		return (unsigned char) round(x);
	}
}

void merge_blocks(sf::Color dest[16][16], double source[6][8][8]) {
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			double Y = chomp(source[(i/8) * 2 + (j/8)][i % 8][j % 8]);
			double Cb = chomp(source[4][i / 2][j / 2]);
			double Cr = chomp(source[5][i / 2][j / 2]);
			// double R = Y + 1.28033 * Cb;
            // double G = Y - 0.21482 * Cr - 0.38059 * Cb;
            // double B = Y + 2.12798 * Cr;
			// dest[i][j].r = chomp(R);
			// dest[i][j].g = chomp(G);
			// dest[i][j].b = chomp(B);
			// 公式來源： http://softpixel.com/~cwright/programming/colorspace/yuv/
			Cb-=128, Cr-=128;
			dest[i][j].r = chomp(Y + 1.4075*Cr);
			dest[i][j].g = chomp(Y - 0.3455*Cb - 0.7169*Cr);
			dest[i][j].b = chomp(Y + 1.779*Cb);
		}
	}
}
