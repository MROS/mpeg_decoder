#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <memory.h>
#include <SFML/Graphics/Color.hpp>
#include "decoder.h"
#include "util.h"
#include "YCbCr.h"

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