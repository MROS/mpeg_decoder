#include "util.h"
#include <vector>
#include <string>
#include <dirent.h>
#include <algorithm>
#include <math.h>
#include <SFML/Graphics/Color.hpp>

using namespace std;

vector<string> split(const string& str) {
	vector<string> ret;
	string strs = str + " ";
	size_t pos;
	size_t size = strs.size();

	for (int i = 0; i < size; ++i) {
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

double cc(int i, int j) {
	if (i == 0 && j == 0) {
		return 1.0/2.0;
	} else if (i == 0 || j == 0) {
		return 1.0/sqrt(2.0);
	} else {
		return 1.0;
	}
}

// TODO: cos 快取
void idct(double (*dest)[8], int (*source)[8]) {
	// TODO: memset
    for (int i = 0 ; i < 8; i++) {
    	for (int j = 0; j < 8; j++) {
    		dest[i][j] = 0;
    	}
    }

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			for (int x = 0; x < 8; x++) {
				for (int y = 0; y < 8; y++) {
					dest[i][j] += (cc(x, y) * source[x][y] * cos((2*i+1)*M_PI/16.0*x) * cos((2*j+1)*M_PI/16.0*y));
				}
			}
			dest[i][j] /= 4.0;
		}
	}
}

unsigned char chomp(double x) {
	if (x > 255.0) {
		return 255;
	} else if (x < 0) {
		return 0;
	} else {
		return (unsigned char) x;
	}
}

void merge(sf::Color (*dest)[16], double (*y)[8][8], double (*cb)[8], double(*cr)[8]) {
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			double Y = y[(i/8) * 2 + (j/8)][i % 8][j % 8];
			double Cb = cb[i / 2][j / 2];
			double Cr = cr[i / 2][j / 2];
			dest[i][j].r = chomp(Y + 1.402*Cr + 128);
			dest[i][j].g = chomp(Y - 0.34414*Cb - 0.71414*Cr + 128);
			dest[i][j].b = chomp(Y + 1.772*Cb + 128);
		}
	}
}
